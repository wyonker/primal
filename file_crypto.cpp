// file_crypto.cpp
//
// Envelope file encryption / decryption for PRIMAL.
//
// Intended for protecting PHI (e.g. ~10 MB DICOM objects) at rest.
//
// Scheme (envelope / hybrid encryption):
//   * Per file: a random 256-bit data key (DEK) encrypts the file contents
//     with AES-256-GCM (confidentiality + integrity / tamper detection).
//   * The DEK is wrapped (encrypted) with AES-256-GCM under a master key (MK,
//     a key-encryption key). The wrapped DEK is stored inside the output file,
//     so decryption needs only the MK -- there is no per-file key to transport.
//   * The MK lives in MySQL on a separate host (see MasterKeyDbConfig): in the
//     `primal` database, `config` table, row where conf_name = 'MK', with the
//     key material in conf_value.
//
// MK encoding in conf_value: the 32-byte AES-256 master key, encoded as either
//   * 64 lowercase hex characters (recommended), e.g. `openssl rand -hex 32`
//   * or standard base64 (e.g. `openssl rand -base64 32`).
// The loader accepts either and verifies it decodes to exactly 32 bytes.
//
// === PHI / HIPAA key-handling notes (READ THIS) =============================
//   The master key protects every file it wraps; its compromise is total.
//   Storing MK in a MySQL row keeps it on a different host from the ciphertext,
//   which is better than colocation, but the row holds the key in the CLEAR --
//   a DB dump, backup, or stolen credential exposes it (unlike a KMS/HSM, which
//   never releases key material). To keep this defensible, the deployment MUST:
//     * connect to the MK host over TLS only;
//     * use a dedicated MySQL account allowed to SELECT only the MK row, with
//       no access to the PHI database;
//     * network-restrict the MK host (dc2primal02) to the app hosts;
//     * enable MySQL at-rest / keyring encryption so backups don't leak MK;
//     * audit-log every read of the MK row (HIPAA 45 CFR 164.312(b));
//     * rotate the MK on a schedule. Because only the small wrapped DEK is
//       encrypted under MK (not the file), rotation re-wraps DEKs without
//       re-encrypting the DICOM: decrypt the wrapped DEK with the old MK and
//       re-wrap it with the new MK. (Not implemented here.)
//   This module scrubs MK/DEK/plaintext from its own memory after use.
//
//   AES-256-GCM is a FIPS-approved primitive suitable for PHI. For FIPS-mode
//   deployments, build against the OpenSSL FIPS provider; this code uses only
//   the default EVP interfaces.
//
//   This module enforces what the host itself can enforce: the master key is
//   fetched over a TLS-verified session, held in RAM only (never written to
//   disk), and gated on this VM's SMBIOS UUID so a cloned/exported VM fails
//   closed. The controls that live OUTSIDE the binary -- the dc2primal02
//   account/REQUIRE SSL, source-IP allow-listing, audit logging, UUID
//   registration, MK generation and escrow -- are documented in the companion
//   file: file_crypto_DEPLOYMENT.md. Read it before deploying.
// ============================================================================
//
// Requires OpenSSL 3.x (the RHEL 10 system OpenSSL); also builds on 1.1.1.
// The MySQL-backed layer requires MySQL Connector/C++ (JDBC-style cppconn, as
// used elsewhere in PRIMAL) and is compiled only when PRIMAL_WITH_MYSQL is
// defined. The crypto core (the *WithKey functions) needs only OpenSSL.
//
// Build (core self-test, no DB):
//   g++ -std=c++20 -DFILE_CRYPTO_DEMO file_crypto.cpp -o file_crypto -lssl -lcrypto
// Build (with MySQL-backed MK lookup):
//   g++ -std=c++20 -DPRIMAL_WITH_MYSQL file_crypto.cpp -c \
//       -I/usr/include/cppconn -lssl -lcrypto -lmysqlcppconn

#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/err.h>

#include <algorithm>
#include <array>
#include <cctype>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

// ============================================================================
// Public interface
// ============================================================================

// Where the master key lives. Defaults match the PRIMAL deployment; supply
// credentials from /etc/primal/primal.conf -- do NOT hardcode them.
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

    // Host identity binding. If expectedHostUuid is non-empty, the service
    // reads its own SMBIOS/BIOS UUID and REFUSES to run (fails closed) unless
    // it matches. The SMBIOS UUID is preserved across a vMotion but is
    // reassigned when a VM is cloned/exported, so this tolerates legitimate
    // live migration while blocking a copied VM booted elsewhere. Leaving it
    // empty disables the check (not recommended for production PHI).
    std::string  expectedHostUuid;            // e.g. "42151c0a-....". Case-insensitive.
    std::string  uuidPath = "/sys/class/dmi/id/product_uuid";  // root-readable on RHEL
};

// ---- Crypto core (OpenSSL only; master key supplied as 32 raw bytes) -------

// Encrypt `fileName` under `masterKey` (must be 32 bytes). Writes
// "<fileName>.enc" and returns its path. Throws std::runtime_error on failure.
std::string encryptFileWithKey(const std::string& fileName,
                               const std::vector<unsigned char>& masterKey);

// Decrypt `encFileName` under `masterKey`. If `outFileName` is empty, a
// trailing ".enc" is stripped (or ".dec" appended) to form the output path.
// Throws on failure, including authentication failure if the file or its
// wrapped key was tampered with, or the wrong master key was supplied.
std::string decryptFileWithKey(const std::string& encFileName,
                               const std::vector<unsigned char>& masterKey,
                               const std::string& outFileName = "");

// Decode a conf_value string (hex or base64) into a 32-byte master key.
std::vector<unsigned char> decodeMasterKey(const std::string& encoded);

#ifdef PRIMAL_WITH_MYSQL
// ---- MySQL-backed layer (requires MySQL Connector/C++) ---------------------

// Verify host identity (if db.expectedHostUuid is set). Throws (fails closed)
// on mismatch or if the UUID cannot be read when a match is required.
void verifyHostIdentity(const MasterKeyDbConfig& db);

// Verify host identity, connect to the MK host over a TLS-verified session,
// and fetch+decode the master key. Throws on any failure -- callers MUST NOT
// fall back to writing plaintext.
std::vector<unsigned char> loadMasterKey(const MasterKeyDbConfig& db);

// Stateless convenience wrappers: fetch MK from MySQL, then encrypt / decrypt.
// Each call re-fetches the key. For a long-running receiver, prefer
// MasterKeySession below, which fetches once and holds the key in RAM only.
std::string encryptFile(const std::string& fileName, const MasterKeyDbConfig& db);
std::string decryptFile(const std::string& encFileName, const MasterKeyDbConfig& db,
                        const std::string& outFileName = "");

// RAM-only, lifetime-cached master key for an unattended service. Construct
// once at startup: it verifies host identity and fetches the MK over TLS,
// failing closed if either step fails (so a cloned/misplaced VM never starts
// processing). The key lives only in this object's memory and is scrubbed on
// destruction; it is never written to disk.
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
#endif

// ============================================================================
// Implementation
// ============================================================================

namespace {

// ---- On-disk container format (fixed offsets) ------------------------------
//   magic[8]       = "PRMLENC2"
//   wrapIv[12]     = GCM nonce used to wrap the DEK
//   wrapTag[16]    = GCM tag over the wrapped DEK
//   wrappedDek[32] = AES-256-GCM(DEK) under the master key
//   fileIv[12]     = GCM nonce used to encrypt the file
//   fileTag[16]    = GCM tag over the file ciphertext
//   ciphertext[]   = AES-256-GCM(file contents) under the DEK
constexpr std::array<unsigned char, 8> kMagic =
    {'P', 'R', 'M', 'L', 'E', 'N', 'C', '2'};
constexpr int kKeyLen = 32;  // AES-256 (both DEK and MK)
constexpr int kIvLen  = 12;  // 96-bit GCM nonce
constexpr int kTagLen = 16;  // 128-bit GCM tag
constexpr size_t kHeaderLen =
    kMagic.size() + kIvLen + kTagLen + kKeyLen + kIvLen + kTagLen;  // 96

struct CipherDeleter { void operator()(EVP_CIPHER_CTX* p) const { EVP_CIPHER_CTX_free(p); } };
using CipherPtr = std::unique_ptr<EVP_CIPHER_CTX, CipherDeleter>;

[[noreturn]] void fail(const std::string& what) {
    unsigned long e = ERR_get_error();
    char buf[256] = {0};
    if (e) ERR_error_string_n(e, buf, sizeof(buf));
    throw std::runtime_error(what + (e ? (": " + std::string(buf)) : ""));
}

template <typename Container>
void scrub(Container& c) {
    if (!c.empty())
        OPENSSL_cleanse(c.data(), c.size() * sizeof(typename Container::value_type));
}

std::vector<unsigned char> readWholeFile(const std::string& path) {
    std::ifstream in(path, std::ios::binary);
    if (!in) throw std::runtime_error("cannot open input file: " + path);
    return std::vector<unsigned char>((std::istreambuf_iterator<char>(in)),
                                       std::istreambuf_iterator<char>());
}

void writeAll(std::ostream& os, const unsigned char* p, size_t n) {
    os.write(reinterpret_cast<const char*>(p), static_cast<std::streamsize>(n));
}

// AES-256-GCM encrypt. Returns ciphertext; fills tag (kTagLen bytes).
std::vector<unsigned char> gcmEncrypt(const std::vector<unsigned char>& key,
                                      const unsigned char* iv,
                                      const unsigned char* plain, size_t plainLen,
                                      unsigned char* tagOut) {
    CipherPtr ctx(EVP_CIPHER_CTX_new());
    if (!ctx) fail("EVP_CIPHER_CTX_new");
    if (EVP_EncryptInit_ex(ctx.get(), EVP_aes_256_gcm(), nullptr, nullptr, nullptr) != 1)
        fail("EncryptInit (cipher)");
    if (EVP_CIPHER_CTX_ctrl(ctx.get(), EVP_CTRL_GCM_SET_IVLEN, kIvLen, nullptr) != 1)
        fail("set IV len");
    if (EVP_EncryptInit_ex(ctx.get(), nullptr, nullptr, key.data(), iv) != 1)
        fail("EncryptInit (key/iv)");

    std::vector<unsigned char> out(plainLen + EVP_MAX_BLOCK_LENGTH);
    int len = 0, total = 0;
    if (plainLen > 0) {
        if (EVP_EncryptUpdate(ctx.get(), out.data(), &len, plain,
                              static_cast<int>(plainLen)) != 1)
            fail("EncryptUpdate");
        total = len;
    }
    if (EVP_EncryptFinal_ex(ctx.get(), out.data() + total, &len) != 1)
        fail("EncryptFinal");
    total += len;
    out.resize(total);
    if (EVP_CIPHER_CTX_ctrl(ctx.get(), EVP_CTRL_GCM_GET_TAG, kTagLen, tagOut) != 1)
        fail("get GCM tag");
    return out;
}

// AES-256-GCM decrypt with authentication. Throws on tag mismatch.
std::vector<unsigned char> gcmDecrypt(const std::vector<unsigned char>& key,
                                      const unsigned char* iv,
                                      const unsigned char* cipher, size_t cipherLen,
                                      const unsigned char* tag) {
    CipherPtr ctx(EVP_CIPHER_CTX_new());
    if (!ctx) fail("EVP_CIPHER_CTX_new");
    if (EVP_DecryptInit_ex(ctx.get(), EVP_aes_256_gcm(), nullptr, nullptr, nullptr) != 1)
        fail("DecryptInit (cipher)");
    if (EVP_CIPHER_CTX_ctrl(ctx.get(), EVP_CTRL_GCM_SET_IVLEN, kIvLen, nullptr) != 1)
        fail("set IV len");
    if (EVP_DecryptInit_ex(ctx.get(), nullptr, nullptr, key.data(), iv) != 1)
        fail("DecryptInit (key/iv)");

    std::vector<unsigned char> out(cipherLen + EVP_MAX_BLOCK_LENGTH);
    int len = 0, total = 0;
    if (cipherLen > 0) {
        if (EVP_DecryptUpdate(ctx.get(), out.data(), &len, cipher,
                              static_cast<int>(cipherLen)) != 1)
            fail("DecryptUpdate");
        total = len;
    }
    if (EVP_CIPHER_CTX_ctrl(ctx.get(), EVP_CTRL_GCM_SET_TAG, kTagLen,
                            const_cast<unsigned char*>(tag)) != 1)
        fail("set GCM tag");
    if (EVP_DecryptFinal_ex(ctx.get(), out.data() + total, &len) != 1)
        throw std::runtime_error(
            "authentication failed: file is corrupt, truncated, tampered with, "
            "or the wrong master key was supplied");
    total += len;
    out.resize(total);
    return out;
}

std::string trim(const std::string& s) {
    size_t a = 0, b = s.size();
    while (a < b && std::isspace(static_cast<unsigned char>(s[a]))) ++a;
    while (b > a && std::isspace(static_cast<unsigned char>(s[b - 1]))) --b;
    return s.substr(a, b - a);
}

std::vector<unsigned char> fromHex(const std::string& s) {
    auto nib = [](char c) -> int {
        if (c >= '0' && c <= '9') return c - '0';
        if (c >= 'a' && c <= 'f') return c - 'a' + 10;
        if (c >= 'A' && c <= 'F') return c - 'A' + 10;
        return -1;
    };
    std::vector<unsigned char> out(s.size() / 2);
    for (size_t i = 0; i < out.size(); ++i) {
        int hi = nib(s[2 * i]), lo = nib(s[2 * i + 1]);
        if (hi < 0 || lo < 0) throw std::runtime_error("invalid hex in master key");
        out[i] = static_cast<unsigned char>((hi << 4) | lo);
    }
    return out;
}

std::vector<unsigned char> fromBase64(const std::string& s) {
    if (s.empty() || s.size() % 4 != 0)
        throw std::runtime_error("invalid base64 length in master key");
    std::vector<unsigned char> out(s.size() / 4 * 3);
    int n = EVP_DecodeBlock(out.data(),
                            reinterpret_cast<const unsigned char*>(s.data()),
                            static_cast<int>(s.size()));
    if (n < 0) throw std::runtime_error("invalid base64 in master key");
    size_t pad = 0;
    if (s[s.size() - 1] == '=') ++pad;
    if (s[s.size() - 2] == '=') ++pad;
    out.resize(static_cast<size_t>(n) - pad);
    return out;
}

} // namespace

std::vector<unsigned char> decodeMasterKey(const std::string& encoded) {
    const std::string s = trim(encoded);
    std::vector<unsigned char> key;
    bool looksHex = (s.size() == 64) &&
        std::all_of(s.begin(), s.end(),
                    [](unsigned char c) { return std::isxdigit(c); });
    key = looksHex ? fromHex(s) : fromBase64(s);
    if (key.size() != static_cast<size_t>(kKeyLen)) {
        scrub(key);
        throw std::runtime_error(
            "master key must decode to 32 bytes (AES-256); got " +
            std::to_string(key.size()));
    }
    return key;
}

std::string encryptFileWithKey(const std::string& fileName,
                               const std::vector<unsigned char>& masterKey) {
    if (masterKey.size() != static_cast<size_t>(kKeyLen))
        throw std::runtime_error("master key must be 32 bytes (AES-256)");

    std::vector<unsigned char> plaintext = readWholeFile(fileName);

    // Random per-file data key (DEK) and nonces.
    std::vector<unsigned char> dek(kKeyLen);
    unsigned char fileIv[kIvLen], wrapIv[kIvLen];
    if (RAND_bytes(dek.data(), kKeyLen) != 1) fail("RAND_bytes (dek)");
    if (RAND_bytes(fileIv, kIvLen) != 1)      fail("RAND_bytes (fileIv)");
    if (RAND_bytes(wrapIv, kIvLen) != 1)      fail("RAND_bytes (wrapIv)");

    // Encrypt the file under the DEK.
    unsigned char fileTag[kTagLen];
    std::vector<unsigned char> ciphertext =
        gcmEncrypt(dek, fileIv, plaintext.data(), plaintext.size(), fileTag);
    scrub(plaintext);

    // Wrap the DEK under the master key.
    unsigned char wrapTag[kTagLen];
    std::vector<unsigned char> wrappedDek =
        gcmEncrypt(masterKey, wrapIv, dek.data(), dek.size(), wrapTag);
    scrub(dek);
    if (wrappedDek.size() != static_cast<size_t>(kKeyLen))
        throw std::runtime_error("unexpected wrapped-key length");

    const std::string outName = fileName + ".enc";
    std::ofstream out(outName, std::ios::binary | std::ios::trunc);
    if (!out) throw std::runtime_error("cannot open output file: " + outName);
    writeAll(out, kMagic.data(), kMagic.size());
    writeAll(out, wrapIv, kIvLen);
    writeAll(out, wrapTag, kTagLen);
    writeAll(out, wrappedDek.data(), wrappedDek.size());
    writeAll(out, fileIv, kIvLen);
    writeAll(out, fileTag, kTagLen);
    writeAll(out, ciphertext.data(), ciphertext.size());
    out.flush();
    if (!out) throw std::runtime_error("write failed: " + outName);
    return outName;
}

std::string decryptFileWithKey(const std::string& encFileName,
                               const std::vector<unsigned char>& masterKey,
                               const std::string& outFileName) {
    if (masterKey.size() != static_cast<size_t>(kKeyLen))
        throw std::runtime_error("master key must be 32 bytes (AES-256)");

    std::vector<unsigned char> blob = readWholeFile(encFileName);
    if (blob.size() < kHeaderLen)
        throw std::runtime_error("file too small to be a valid container: " + encFileName);
    if (std::memcmp(blob.data(), kMagic.data(), kMagic.size()) != 0)
        throw std::runtime_error("bad magic (not a PRMLENC2 file): " + encFileName);

    size_t pos = kMagic.size();
    const unsigned char* wrapIv     = blob.data() + pos; pos += kIvLen;
    const unsigned char* wrapTag    = blob.data() + pos; pos += kTagLen;
    const unsigned char* wrappedDek = blob.data() + pos; pos += kKeyLen;
    const unsigned char* fileIv     = blob.data() + pos; pos += kIvLen;
    const unsigned char* fileTag    = blob.data() + pos; pos += kTagLen;
    const unsigned char* ciphertext = blob.data() + pos;
    const size_t ciphertextLen      = blob.size() - pos;

    // Unwrap the DEK under the master key (authenticated).
    std::vector<unsigned char> dek =
        gcmDecrypt(masterKey, wrapIv, wrappedDek, kKeyLen, wrapTag);
    if (dek.size() != static_cast<size_t>(kKeyLen)) {
        scrub(dek);
        throw std::runtime_error("unwrapped DEK has unexpected length");
    }

    // Decrypt the file under the DEK (authenticated).
    std::vector<unsigned char> plaintext;
    try {
        plaintext = gcmDecrypt(dek, fileIv, ciphertext, ciphertextLen, fileTag);
    } catch (...) {
        scrub(dek);
        throw;
    }
    scrub(dek);

    std::string outName = outFileName;
    if (outName.empty()) {
        const std::string suffix = ".enc";
        if (encFileName.size() > suffix.size() &&
            encFileName.compare(encFileName.size() - suffix.size(),
                                suffix.size(), suffix) == 0)
            outName = encFileName.substr(0, encFileName.size() - suffix.size());
        else
            outName = encFileName + ".dec";
    }

    std::ofstream out(outName, std::ios::binary | std::ios::trunc);
    if (!out) { scrub(plaintext); throw std::runtime_error("cannot open output file: " + outName); }
    writeAll(out, plaintext.data(), plaintext.size());
    out.flush();
    bool ok = static_cast<bool>(out);
    scrub(plaintext);
    if (!ok) throw std::runtime_error("write failed: " + outName);
    return outName;
}

// ============================================================================
// MySQL-backed layer: fetch MK from primal.config on the configured host.
// Matches PRIMAL's JDBC-style MySQL Connector/C++ usage (get_driver_instance,
// connect(url,user,pass), setSchema, prepareStatement).
// ============================================================================
#ifdef PRIMAL_WITH_MYSQL
#include <cppconn/driver.h>
#include <cppconn/connection.h>
#include <cppconn/statement.h>
#include <cppconn/prepared_statement.h>
#include <cppconn/resultset.h>
#include <cppconn/exception.h>

namespace {

std::string toLowerTrim(std::string s) {
    s = trim(s);
    std::transform(s.begin(), s.end(), s.begin(),
                   [](unsigned char c) { return std::tolower(c); });
    return s;
}

} // namespace

void verifyHostIdentity(const MasterKeyDbConfig& db) {
    if (db.expectedHostUuid.empty())
        return;  // binding disabled (see MasterKeyDbConfig docs / deployment file)

    std::ifstream f(db.uuidPath);
    if (!f)
        throw std::runtime_error(
            "host identity check: cannot read " + db.uuidPath +
            " (needs read access; service likely must run as root). Failing "
            "closed because expectedHostUuid is set.");
    std::string actual;
    std::getline(f, actual);

    if (toLowerTrim(actual) != toLowerTrim(db.expectedHostUuid))
        throw std::runtime_error(
            "host identity mismatch: this VM's SMBIOS UUID does not match the "
            "expected value. Refusing to run (a vMotion preserves the UUID; a "
            "clone/export does not). Encryption/decryption aborted.");
}

std::vector<unsigned char> loadMasterKey(const MasterKeyDbConfig& db) {
    if (db.user.empty())
        throw std::runtime_error("MasterKeyDbConfig.user is empty; supply a "
                                 "dedicated least-privilege account");

    // Fail closed if this is not the host we are bound to.
    verifyHostIdentity(db);

    try {
        sql::Driver* driver = get_driver_instance();
        const std::string url = "tcp://" + db.host + ":" + std::to_string(db.port);

        // TLS: the hard enforcement is server-side -- the MK account MUST be
        // created WITH `REQUIRE SSL` (or REQUIRE X509) so the server rejects any
        // cleartext connection, and the client must trust the server CA (set
        // ssl-ca in the client my.cnf / connector config). Connector/C++ then
        // negotiates TLS automatically. Rather than depend on version-specific
        // client SSL options, we VERIFY below that the live session is actually
        // encrypted and fail closed if it is not. See the deployment file.
        std::unique_ptr<sql::Connection> con(
            driver->connect(url, db.user, db.password));
        con->setSchema(db.schema);

        if (db.requireTls) {
            std::unique_ptr<sql::Statement> st(con->createStatement());
            std::unique_ptr<sql::ResultSet> srs(
                st->executeQuery("SHOW SESSION STATUS LIKE 'Ssl_cipher'"));
            std::string cipher;
            if (srs->next()) cipher = srs->getString(2);  // Value column
            if (cipher.empty())
                throw std::runtime_error(
                    "TLS required but the MySQL session is NOT encrypted "
                    "(Ssl_cipher is empty). Refusing to transmit the master "
                    "key in cleartext. Create the account WITH REQUIRE SSL and "
                    "configure the client CA.");
        }

        std::unique_ptr<sql::PreparedStatement> ps(con->prepareStatement(
            "SELECT " + db.valueColumn + " FROM " + db.table +
            " WHERE " + db.nameColumn + " = ?"));
        ps->setString(1, db.keyName);
        std::unique_ptr<sql::ResultSet> rs(ps->executeQuery());

        if (!rs->next())
            throw std::runtime_error("master key row not found: " + db.nameColumn +
                                     " = '" + db.keyName + "'");
        std::string encoded = rs->getString(1);
        if (rs->next())
            throw std::runtime_error("multiple master key rows found for " +
                                     db.keyName + "; refusing to guess");

        std::vector<unsigned char> key = decodeMasterKey(encoded);
        scrub(encoded);  // best effort; std::string from the driver
        return key;
    } catch (const sql::SQLException& e) {
        throw std::runtime_error(std::string("MySQL error loading master key: ") +
                                 e.what());
    }
}

std::string encryptFile(const std::string& fileName, const MasterKeyDbConfig& db) {
    std::vector<unsigned char> mk = loadMasterKey(db);
    try {
        std::string out = encryptFileWithKey(fileName, mk);
        scrub(mk);
        return out;
    } catch (...) { scrub(mk); throw; }
}

std::string decryptFile(const std::string& encFileName, const MasterKeyDbConfig& db,
                        const std::string& outFileName) {
    std::vector<unsigned char> mk = loadMasterKey(db);
    try {
        std::string out = decryptFileWithKey(encFileName, mk, outFileName);
        scrub(mk);
        return out;
    } catch (...) { scrub(mk); throw; }
}

// ---- MasterKeySession: fetch once at startup, hold in RAM only -------------
MasterKeySession::MasterKeySession(MasterKeyDbConfig db) {
    // Verifies identity + TLS and fetches the MK; throws (fails closed) if the
    // host is wrong, the session is not encrypted, or the key is unavailable.
    mk_ = loadMasterKey(db);
}

MasterKeySession::~MasterKeySession() {
    scrub(mk_);
}

std::string MasterKeySession::encryptFile(const std::string& fileName) const {
    return encryptFileWithKey(fileName, mk_);
}

std::string MasterKeySession::decryptFile(const std::string& encFileName,
                                          const std::string& outFileName) const {
    return decryptFileWithKey(encFileName, mk_, outFileName);
}
#endif // PRIMAL_WITH_MYSQL

// ============================================================================
// Optional self-test for the crypto core (no DB): build with -DFILE_CRYPTO_DEMO
//   file_crypto enc <file>      <hexkey>          -> writes <file>.enc
//   file_crypto dec <file.enc>  <hexkey> [out]    -> recovers the file
// Generate a test key with:  openssl rand -hex 32
// ============================================================================
#ifdef FILE_CRYPTO_DEMO
#include <iostream>

int main(int argc, char** argv) {
    try {
        if (argc >= 4 && std::string(argv[1]) == "enc") {
            std::vector<unsigned char> mk = decodeMasterKey(argv[3]);
            std::string out = encryptFileWithKey(argv[2], mk);
            scrub(mk);
            std::cout << "Encrypted -> " << out << "\n";
        } else if (argc >= 4 && std::string(argv[1]) == "dec") {
            std::vector<unsigned char> mk = decodeMasterKey(argv[3]);
            std::string out = decryptFileWithKey(argv[2], mk, argc >= 5 ? argv[4] : "");
            scrub(mk);
            std::cout << "Decrypted -> " << out << "\n";
        } else {
            std::cerr << "usage:\n"
                      << "  " << argv[0] << " enc <file> <hexkey>\n"
                      << "  " << argv[0] << " dec <file.enc> <hexkey> [out]\n";
            return 2;
        }
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }
    return 0;
}
#endif
