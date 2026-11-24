// file_crypto.h
//
// Public interface for the PRIMAL PHI file-encryption module (file_crypto.cpp).
// See file_crypto.cpp and file_crypto_DEPLOYMENT.md for the full design.
//
// The crypto core (the *WithKey functions) needs only OpenSSL. The MySQL-backed
// master-key layer (MasterKeySession etc.) is declared only when PRIMAL_WITH_MYSQL
// is defined, and requires the MySQL C API (libmysqlclient / MariaDB Connector/C).

#ifndef PRIMAL_FILE_CRYPTO_H
#define PRIMAL_FILE_CRYPTO_H

#include <string>
#include <vector>

// Where the master key lives. Defaults match the separate-host PRIMAL design;
// supply credentials from configuration -- do NOT hardcode them.
struct MasterKeyDbConfig {
    std::string  host        = "dc2primal02";
    unsigned     port        = 3306;
    std::string  user;                       // dedicated, least-privilege account
    std::string  password;
    std::string  schema      = "primal";
    std::string  table       = "config";
    std::string  nameColumn  = "conf_name";
    std::string  valueColumn = "conf_value";
    std::string  keyName     = "MK";
    bool         requireTls  = true;          // verified after connect (see loadMasterKey)

    // Host identity binding. If expectedHostUuid is non-empty, the service reads
    // its own SMBIOS/BIOS UUID and REFUSES to run (fails closed) unless it
    // matches. The SMBIOS UUID survives a vMotion but is reassigned when a VM is
    // cloned/exported, so this tolerates live migration while blocking a copied
    // VM booted elsewhere. Empty disables the check (not recommended for PHI).
    std::string  expectedHostUuid;            // e.g. "42151c0a-....". Case-insensitive.
    std::string  uuidPath = "/sys/class/dmi/id/product_uuid";  // root-readable on RHEL
};

// ---- Crypto core (OpenSSL only; master key supplied as 32 raw bytes) -------

// Encrypt `fileName` under `masterKey` (must be 32 bytes). Writes
// "<fileName>.enc" and returns its path. Throws std::runtime_error on failure.
std::string encryptFileWithKey(const std::string& fileName,
                               const std::vector<unsigned char>& masterKey);

// Decrypt `encFileName` under `masterKey`. If `outFileName` is empty, a trailing
// ".enc" is stripped (or ".dec" appended) to form the output path. Throws on
// failure, including authentication failure if the file or its wrapped key was
// tampered with, or the wrong master key was supplied.
std::string decryptFileWithKey(const std::string& encFileName,
                               const std::vector<unsigned char>& masterKey,
                               const std::string& outFileName = "");

// Decode a conf_value string (hex or base64) into a 32-byte master key.
std::vector<unsigned char> decodeMasterKey(const std::string& encoded);

// Returns true if `path` is a file_crypto container (identified by its magic
// header), i.e. an encrypted file that must be decrypted before use. Returns
// false for plaintext files, missing files, or anything too short to be one.
// This reads only the header, not the whole file.
bool isFileCryptoContainer(const std::string& path);

#ifdef PRIMAL_WITH_MYSQL
// ---- MySQL-backed layer (requires the MySQL C API / libmysqlclient) --------

// Verify host identity (if db.expectedHostUuid is set). Throws (fails closed)
// on mismatch or if the UUID cannot be read when a match is required.
void verifyHostIdentity(const MasterKeyDbConfig& db);

// Verify host identity, connect to the MK host over a TLS-verified session, and
// fetch+decode the master key. Throws on any failure -- callers MUST NOT fall
// back to writing plaintext.
std::vector<unsigned char> loadMasterKey(const MasterKeyDbConfig& db);

// Stateless convenience wrappers: fetch MK from MySQL, then encrypt / decrypt.
// Each call re-fetches the key. For a long-running service, prefer
// MasterKeySession below, which fetches once and holds the key in RAM only.
std::string encryptFile(const std::string& fileName, const MasterKeyDbConfig& db);
std::string decryptFile(const std::string& encFileName, const MasterKeyDbConfig& db,
                        const std::string& outFileName = "");

// RAM-only, lifetime-cached master key for an unattended service. Construct once
// at startup: it verifies host identity and fetches the MK over TLS, failing
// closed if either step fails. The key lives only in this object's memory and is
// scrubbed on destruction; it is never written to disk.
class MasterKeySession {
public:
    explicit MasterKeySession(MasterKeyDbConfig db);
    ~MasterKeySession();
    MasterKeySession(const MasterKeySession&) = delete;
    MasterKeySession& operator=(const MasterKeySession&) = delete;

    std::string encryptFile(const std::string& fileName) const;
    std::string decryptFile(const std::string& encFileName,
                            const std::string& outFileName = "") const;
private:
    std::vector<unsigned char> mk_;
};
#endif  // PRIMAL_WITH_MYSQL

#endif  // PRIMAL_FILE_CRYPTO_H
