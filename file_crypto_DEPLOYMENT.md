# file_crypto — Deployment & Key-Management Controls

Operational controls for `file_crypto.cpp` that live **outside** the binary.
The code enforces the host-side half (TLS-verified fetch, RAM-only key, SMBIOS
UUID binding, fail-closed on any error). Everything below is your
responsibility to configure and document in the HIPAA risk analysis.

Context: PRIMAL is an air-gapped (no internet) DICOM archive handling PHI
(~10 MB studies). Servers are VMware VMs in **separate vCenters** per site.
VMware-level settings are owned by a different team, so the controls here are
chosen to NOT depend on that team where possible.

---

## 1. How the encryption works (recap)

Envelope encryption:

- Each file gets a random **AES-256 data key (DEK)**; the DICOM is encrypted
  with AES-256-GCM under the DEK.
- The DEK is wrapped (AES-256-GCM) under the **master key (MK)**.
- The wrapped DEK is stored **inside** the `.enc` container, so decryption
  needs only the MK — there is no per-file key to manage.
- The MK lives on **dc2primal02** (a different physical site, different
  vCenter): database `primal`, table `config`, row `conf_name = 'MK'`, key
  material in `conf_value`.

The PRIMAL host holds the MK **only in RAM**, fetched at startup. A stolen
PRIMAL VM image / snapshot / backup therefore contains only ciphertext.

---

## 2. Generate and store the master key (one time)

Generate a 32-byte AES-256 key as hex (recommended) or base64:

```bash
openssl rand -hex 32
```

On **dc2primal02**, insert it (run as an admin account, NOT the app account):

```sql
INSERT INTO primal.config (conf_name, conf_value) VALUES ('MK', '<paste-hex>');
```

`conf_value` accepts 64 hex chars or standard base64; the loader verifies it
decodes to exactly 32 bytes.

### Escrow (REQUIRED)

Keep **one offline copy** of the MK — printed and sealed in a safe, or on an
encrypted USB stored in a locked location at a different site. Without escrow,
loss of dc2primal02 (or its data) means permanent, unrecoverable loss of every
encrypted study. Record who holds it and the retrieval procedure.

---

## 3. dc2primal02 — lock down the key host

The MK sits in the clear in a DB row, so this host must be hardened. A
compromise or backup of dc2primal02 alone leaks the MK.

- **Dedicated least-privilege account.** Create a MySQL user used ONLY to read
  the MK row — no access to the PHI database, no writes:

  ```sql
  CREATE USER 'primal_mk'@'<PRIMAL-host-IP>' IDENTIFIED BY '<strong-random>'
      REQUIRE SSL;                                  -- forces TLS (see §4)
  GRANT SELECT (conf_value) ON primal.config TO 'primal_mk'@'<PRIMAL-host-IP>';
  -- Consider a dedicated table/view exposing only the MK row if `config`
  -- holds other sensitive values.
  ```

- **Source-IP allow-listing.** The `'primal_mk'@'<PRIMAL-host-IP>'` host part
  above restricts logins to the PRIMAL host. Back it with a firewall rule on
  dc2primal02 allowing port 3306 only from the PRIMAL host(s). A cloned VM
  booted elsewhere (different IP) is refused the key.

- **At-rest encryption on dc2primal02.** Your IBM/Pure array covers drive
  theft. If the `config` table may be included in filesystem/DB backups, also
  enable MySQL InnoDB tablespace encryption (keyring) or ensure backups land on
  encrypted media, so a backup of dc2primal02 does not leak the MK in cleartext.

- **Audit logging.** Enable the MySQL audit log (or equivalent) and alert on
  reads of the MK row. Under HIPAA 45 CFR 164.312(b) you must be able to show
  who/what accessed the key and when. Expect exactly one fetch per PRIMAL
  service start; anything else is suspicious.

---

## 4. TLS between PRIMAL and dc2primal02 (REQUIRED)

The MK crosses the inter-site network, so the connection must be encrypted.
Enforcement is **server-side** (version-independent, fully in your control):

1. Create the account `REQUIRE SSL` (or `REQUIRE X509` for mutual auth) as in
   §3 — the server then rejects any cleartext connection.
2. Configure the client (PRIMAL host) to trust dc2primal02's CA, e.g. in the
   client `my.cnf`:

   ```ini
   [client]
   ssl-ca=/etc/primal/dc2primal02-ca.pem
   # for REQUIRE X509, also:
   ssl-cert=/etc/primal/primal-client-cert.pem
   ssl-key=/etc/primal/primal-client-key.pem
   ```

The code **verifies** TLS is actually in effect after connecting (checks
`Ssl_cipher`) and **fails closed** if the session is not encrypted — it will
not transmit or use the MK over cleartext.

---

## 5. Host identity binding (clone/copy protection)

Set `MasterKeyDbConfig.expectedHostUuid` to the PRIMAL VM's SMBIOS UUID. Read
the current value on the PRIMAL host:

```bash
cat /sys/class/dmi/id/product_uuid     # or: dmidecode -s system-uuid
```

Behavior:

- **vMotion** preserves the UUID → the service keeps running across routine DRS
  moves. (No VMware-team action needed — this is the reason we bind to UUID
  rather than trying to detect migration, which is not reliably visible to the
  guest.)
- **Clone / export / template deploy** gets a *new* UUID → the service fails
  closed and refuses to fetch or use the MK.

Notes:

- `/sys/class/dmi/id/product_uuid` is root-readable only, so the PRIMAL service
  must run as root (or be granted read access to it). If it cannot be read while
  `expectedHostUuid` is set, the service fails closed.
- Optional hardening: also register the UUID on dc2primal02 and release the MK
  only for the registered UUID, so the key host independently enforces the bind.
- If you re-platform the PRIMAL VM legitimately (new UUID), update
  `expectedHostUuid` as a deliberate, documented step.

---

## 6. Fail-closed contract (for the integrating code)

`loadMasterKey()`, `encryptFile()`, `decryptFile()`, and the
`MasterKeySession` constructor **throw** on any failure (wrong host, no TLS, MK
missing, auth failure, tamper). The DICOM receiver **must**:

- Treat a thrown exception as a hard stop for that file — **never** write the
  plaintext study to disk unencrypted as a fallback.
- Prefer `MasterKeySession`: construct it once at startup so the service fails
  to start (loudly) if the host/TLS/MK checks fail, rather than discovering it
  per file. The MK is then cached in RAM for the service lifetime and scrubbed
  on shutdown — this also rides out brief inter-site link outages.

---

## 7. Residual plaintext exposure (track in the risk analysis)

Application-level encryption protects the DICOM **once encrypted**. These gaps
remain and are NOT closed by this module, the storage-array encryption, or
host/LUKS encryption on a *running* system:

- **Inbound/processing dirs.** The receiver writes incoming studies to
  `/home/dicom/inbound` (then `processing`, `outbound`) in the clear before they
  are encrypted. Encrypt as early in the pipeline as feasible, restrict
  directory permissions to the service user, and confine with SELinux.
- **PRIMAL metadata database.** If the PRIMAL DB stores patient
  names/MRNs/study info, that is PHI and is plaintext to the running host.
  Consider InnoDB tablespace encryption; the array still covers drive theft.
- **VM memory.** The MK and in-flight DEKs live in the PRIMAL VM's RAM. A VMware
  **memory snapshot** or suspended-VM `.vmem`/`.vmsn` captures them. You cannot
  set VMware RBAC yourself, so **file a request with the VMware team** to:
  restrict snapshot/suspend/clone/console privileges on the PRIMAL VM, avoid
  memory snapshots, and enable **Encrypted vMotion** (DRS moves VM memory across
  the network). Document the request and their response for the risk analysis.

---

## 8. Key rotation

Because only the small wrapped DEK is encrypted under the MK (not the file),
rotating the MK re-wraps DEKs **without** re-encrypting the DICOM: for each
container, unwrap the DEK with the old MK and re-wrap it with the new MK,
rewriting only the fixed-size header. Plan a rotation schedule; a helper for
this is not yet implemented.

---

## 9. Honest limits

- A **live, fully-rooted PRIMAL host** can fetch/hold the MK exactly as the
  service does, then decrypt. No same-host scheme — and no KMS/HSM — removes
  this; the controls above raise the bar and specifically defeat the realistic
  "exfiltrate a copy of the VM and decrypt it elsewhere" path.
- Defense rests on compromising **both** sites/vCenters, TLS in transit, strict
  least privilege on dc2primal02, and the fail-closed host binding. Document
  each control and its owner.

---

## 10. Build

```bash
# With MySQL-backed MK lookup (RHEL 10, OpenSSL 3.x + Connector/C++):
g++ -std=c++20 -DPRIMAL_WITH_MYSQL file_crypto.cpp -c -I/usr/include/cppconn
#   link: -lssl -lcrypto -lmysqlcppconn

# Crypto-core self-test, no DB (proves round-trip):
g++ -std=c++20 -DFILE_CRYPTO_DEMO file_crypto.cpp -o file_crypto -lssl -lcrypto
KEY=$(openssl rand -hex 32)
./file_crypto enc sample.dcm "$KEY"
./file_crypto dec sample.dcm.enc "$KEY" out.dcm
cmp sample.dcm out.dcm && echo "round-trip OK"
```
