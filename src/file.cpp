#include "file.h"
#include "credential.h"

#include <cryptopp/aes.h>
#include <cryptopp/filters.h>
#include <cryptopp/gcm.h>
#include <cryptopp/osrng.h>
#include <cryptopp/scrypt.h>

#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <sstream>
#include <stdexcept>
#include <system_error>

#ifdef _WIN32
#include <windows.h>

#include <aclapi.h>
#include <userenv.h>
#pragma comment(lib, "Userenv.lib")
#pragma comment(lib, "Advapi32.lib")
#else
#include <fcntl.h>
#include <sys/file.h>
#include <unistd.h>
#endif

namespace {

    // --- vault envelope layout -------------------------------------------------------------
    //
    // offset  0,  8 bytes: magic "REDUXVLT"
    // offset  8,  1 byte : format version
    // offset  9,  1 byte : kdf id (1 = scrypt)
    // offset 10,  1 byte : log2(N)
    // offset 11,  1 byte : r
    // offset 12,  1 byte : p
    // offset 13,  1 byte : salt length
    // offset 14, 16 bytes: salt
    // offset 30,  1 byte : nonce length
    // offset 31, 12 bytes: nonce
    // offset 43, rest    : AES-256-GCM ciphertext, followed by its 16-byte tag
    //
    // Every multibyte field here is a single byte, so the layout has no endianness to get wrong.
    // The whole 43-byte header is authenticated as GCM additional data (AAD): tampering with the
    // salt or the declared parameters is caught by the tag check on read, exactly like tampering
    // with the ciphertext body would be.
    constexpr std::size_t magic_size = 8;
    constexpr std::size_t salt_size = 16;
    constexpr std::size_t nonce_size = 12;
    constexpr std::size_t tag_size = 16;
    constexpr std::size_t key_size = 32; // AES-256
    constexpr std::size_t salt_offset =
        magic_size + 6; // magic + version + kdf + log2n + r + p + saltlen
    constexpr std::size_t nonce_len_offset = salt_offset + salt_size;
    constexpr std::size_t nonce_offset = nonce_len_offset + 1;
    constexpr std::size_t header_size = nonce_offset + nonce_size;
    static_assert(header_size == 43, "vault header layout must match the on-disk spec exactly");

    constexpr std::uint8_t format_version = 0x01;
    constexpr std::uint8_t kdf_scrypt = 0x01;
    constexpr std::uint8_t default_log2n = 15;
    constexpr std::uint8_t default_r = 8;
    constexpr std::uint8_t default_p = 1;

    // Anything below this is not meaningfully hard to brute-force; anything above it risks a
    // multi-gigabyte allocation from a corrupted or hostile header before a single byte of
    // ciphertext is even looked at. r and p need the same treatment: scrypt's memory use scales
    // with N * r (and its work with p), key derivation runs before the GCM tag check, and all
    // three come straight from header bytes -- a single corrupted r byte would otherwise force a
    // pre-authentication allocation in the hundreds of megabytes.
    constexpr std::uint8_t min_log2n = 10;
    constexpr std::uint8_t max_log2n = 24;
    constexpr std::uint8_t min_r = 1;
    constexpr std::uint8_t max_r = 32;
    constexpr std::uint8_t min_p = 1;
    constexpr std::uint8_t max_p = 16;

    // The per-parameter ranges alone still admit combinations whose scrypt working set
    // (128 * N * r bytes) reaches 64 GiB -- enough for one corrupted header to OOM the process
    // before the GCM tag can reject it. The product is what has to be bounded; this ceiling sits
    // comfortably above anything write() produces (32 MiB today) while staying survivable.
    constexpr std::uint64_t max_scrypt_memory = 256ull * 1024 * 1024;

    constexpr char magic[magic_size] = {'R', 'E', 'D', 'U', 'X', 'V', 'L', 'T'};

    // --- owner-only permissions -------------------------------------------------------------

#ifdef _WIN32
    // std::filesystem::permissions() on Windows only toggles the read-only attribute -- it
    // cannot express "only this account may read this file" the way POSIX mode bits can. A
    // protected DACL naming the current user's SID is the actual equivalent: PROTECTED stops the
    // object from inheriting a more permissive ACL from its parent directory.
    void set_owner_only(std::filesystem::path const& path) {
        HANDLE token = nullptr;
        if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &token)) {
            throw std::runtime_error{"could not open the process token"};
        }

        DWORD needed = 0;
        GetTokenInformation(token, TokenUser, nullptr, 0, &needed);
        if (needed == 0) {
            CloseHandle(token);
            throw std::runtime_error{"could not size the process token user information"};
        }

        std::vector<BYTE> user_info(needed);
        if (!GetTokenInformation(token, TokenUser, user_info.data(), needed, &needed)) {
            CloseHandle(token);
            throw std::runtime_error{"could not read the process token user information"};
        }

        PSID const user_sid = reinterpret_cast<TOKEN_USER*>(user_info.data())->User.Sid;
        if (!IsValidSid(user_sid)) {
            CloseHandle(token);
            throw std::runtime_error{"process token did not contain a valid user SID"};
        }

        DWORD const acl_size = static_cast<DWORD>(sizeof(ACL) + sizeof(ACCESS_ALLOWED_ACE) +
                                                  GetLengthSid(user_sid) - sizeof(DWORD));
        std::vector<BYTE> acl_buffer(acl_size);
        PACL const acl = reinterpret_cast<PACL>(acl_buffer.data());

        if (!InitializeAcl(acl, acl_size, ACL_REVISION)) {
            CloseHandle(token);
            throw std::runtime_error{"could not initialize the ACL"};
        }
        if (!AddAccessAllowedAce(acl, ACL_REVISION, FILE_ALL_ACCESS, user_sid)) {
            CloseHandle(token);
            throw std::runtime_error{"could not add the owner ACE"};
        }

        std::wstring const wpath = path.wstring();
        DWORD const result =
            SetNamedSecurityInfoW(const_cast<LPWSTR>(wpath.c_str()), SE_FILE_OBJECT,
                                  DACL_SECURITY_INFORMATION | PROTECTED_DACL_SECURITY_INFORMATION,
                                  nullptr, nullptr, acl, nullptr);

        CloseHandle(token);

        if (result != ERROR_SUCCESS) {
            throw std::runtime_error{"could not restrict permissions on " + path.string()};
        }
    }
#else
    void set_owner_only(std::filesystem::path const& path) {
        bool const is_dir = std::filesystem::is_directory(path);
        std::error_code ec;
        std::filesystem::permissions(
            path,
            is_dir ? std::filesystem::perms::owner_all
                   : (std::filesystem::perms::owner_read | std::filesystem::perms::owner_write),
            ec);
        if (ec) {
            throw std::runtime_error{"could not restrict permissions on " + path.string() + ": " +
                                     ec.message()};
        }
    }
#endif

    // --- config directory --------------------------------------------------------------------

#ifdef _WIN32
    std::filesystem::path windows_profile_dir() {
        HANDLE token = nullptr;
        if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &token)) {
            throw std::runtime_error{"could not open the process token"};
        }

        DWORD size = MAX_PATH;
        std::vector<wchar_t> buf(size);
        BOOL ok = GetUserProfileDirectoryW(token, buf.data(), &size);
        if (!ok && GetLastError() == ERROR_INSUFFICIENT_BUFFER) {
            buf.assign(size, L'\0');
            ok = GetUserProfileDirectoryW(token, buf.data(), &size);
        }

        CloseHandle(token);

        if (!ok) {
            throw std::runtime_error{"could not determine the user profile directory"};
        }

        return std::filesystem::path{buf.data()};
    }
#endif

    // Directory holding every file Redux owns. Never returns an empty or relative-to-nothing
    // path: if it cannot be determined, it throws rather than falling back to a bare name in the
    // working directory -- a machine with an unset HOME (a stripped container, a misconfigured
    // service account) used to make Redux silently read and write secrets next to whatever the
    // current directory happened to be.
    std::filesystem::path config_dir() {
        std::filesystem::path path;

        // Checked on every platform, and first: this is also the hook the regression tests use
        // to relocate storage into a sandbox, since overriding HOME doesn't work on Windows.
        if (char const* dir = std::getenv("REDUX_CONFIG_DIR"); dir != nullptr && *dir != '\0') {
            path = dir;
        } else {
#ifdef _WIN32
            path = windows_profile_dir() / "Redux";
#else
            char const* home = std::getenv("HOME");
            if (home == nullptr || *home == '\0') {
                throw std::runtime_error{
                    "HOME is not set; refusing to guess where the vault lives"};
            }
            path = std::filesystem::path{home} / ".config" / "Redux";
#endif
        }

        std::error_code ec;
        bool const created = std::filesystem::create_directories(path, ec);
        if (ec) {
            throw std::runtime_error{"could not create " + path.string() + ": " + ec.message()};
        }

        // Restricted once, at creation -- not on every call. Re-asserting owner-only permissions
        // on every lookup would silently undo any tightening an operator applied afterwards (or,
        // more mundanely, a directory made temporarily unwritable to simulate a full disk).
        if (created) {
            set_owner_only(path);
        }
        return path;
    }

    // --- atomic, owner-only writes -----------------------------------------------------------

#ifdef _WIN32
    unsigned long current_pid() { return GetCurrentProcessId(); }
#else
    unsigned long current_pid() { return static_cast<unsigned long>(getpid()); }
#endif

    // Dot-prefixed so it cannot collide with a real account: a valid username must begin with an
    // alphanumeric (account::valid_username), so no account is ever named like a staging file.
    // The pid and 8 random hex characters mean two concurrent writes -- from two Redux instances,
    // or two rapid writes from the same process -- can never land on the same staging name and
    // clobber each other.
    std::string staging_name(std::string const& filename) {
        CryptoPP::AutoSeededRandomPool rng;
        CryptoPP::byte random_bytes[4];
        rng.GenerateBlock(random_bytes, sizeof(random_bytes));

        std::ostringstream suffix;
        suffix << std::hex << std::setfill('0');
        for (auto b : random_bytes) {
            suffix << std::setw(2) << static_cast<unsigned>(b);
        }

        return "." + filename + "." + std::to_string(current_pid()) + "." + suffix.str() + ".tmp";
    }

    // Writes `bytes` to a sibling staging file and renames it over `target`. Opening the target
    // directly with trunc would destroy the old contents before the new bytes were known to be
    // good, so a failing write would leave a truncated, unreadable file -- this makes the
    // failure visible instead, with the previous file untouched.
    void write_file_atomic(std::filesystem::path const& target, std::string const& bytes) {
        std::filesystem::path const staging =
            target.parent_path() / staging_name(target.filename().string());

        {
            // Binary: the payload can be ciphertext, and a text-mode stream would translate LF
            // to CRLF inside it on Windows and corrupt it.
            std::ofstream out{staging, std::ios::binary | std::ios::trunc};
            out.write(bytes.data(), static_cast<std::streamsize>(bytes.size()));

            // Close here rather than letting the destructor do it. Some filesystems only report
            // a deferred write error when the file is closed, and a destructor has no way to
            // report it -- the staging file would then be renamed over a good file while holding
            // incomplete bytes. Checking after the close is what makes the rename safe.
            out.close();

            if (!out) {
                std::error_code ec;
                std::filesystem::remove(staging, ec);
                throw std::runtime_error{"could not write " + target.string()};
            }
        }

#ifndef _WIN32
        // The rename below publishes whatever has reached the disk, not whatever was written: a
        // crash shortly after could otherwise publish an empty or partial file over a good one.
        // Windows durability is left to the OS here by design.
        {
            int const fd = open(staging.c_str(), O_RDONLY);
            bool const synced = fd >= 0 && fsync(fd) == 0;
            if (fd >= 0) {
                close(fd);
            }
            if (!synced) {
                std::error_code ec;
                std::filesystem::remove(staging, ec);
                throw std::runtime_error{"could not flush " + target.string() + " to disk"};
            }
        }
#endif

        try {
            // Owner-only before it is published, so the file is never briefly readable by anyone
            // else between the write and the rename.
            set_owner_only(staging);
        } catch (...) {
            std::error_code ec;
            std::filesystem::remove(staging, ec);
            throw;
        }

        std::error_code ec;
        std::filesystem::rename(staging, target, ec);
        if (ec) {
            std::error_code cleanup_ec;
            std::filesystem::remove(staging, cleanup_ec);
            throw std::runtime_error{"could not publish " + target.string() + ": " + ec.message()};
        }

#ifndef _WIN32
        // Make the rename itself durable across a crash. Best-effort: the file is already
        // published and readable, so a directory fsync failure is not worth failing the write
        // over.
        if (int const dir_fd = open(target.parent_path().c_str(), O_RDONLY); dir_fd >= 0) {
            fsync(dir_fd);
            close(dir_fd);
        }
#endif
    }

    // --- key derivation and the header ---------------------------------------------------------

    CryptoPP::SecByteBlock derive_key(std::string const& password, CryptoPP::byte const* salt,
                                      std::uint8_t log2n, std::uint8_t r, std::uint8_t p) {
        CryptoPP::SecByteBlock key(key_size);
        CryptoPP::Scrypt{}.DeriveKey(
            key.data(), key.size(), reinterpret_cast<CryptoPP::byte const*>(password.data()),
            password.size(), salt, salt_size, static_cast<CryptoPP::word64>(1) << log2n, r, p);
        return key;
    }

    std::string build_header(CryptoPP::byte const* salt, CryptoPP::byte const* nonce) {
        std::string header;
        header.reserve(header_size);
        header.append(magic, magic_size);
        header.push_back(static_cast<char>(format_version));
        header.push_back(static_cast<char>(kdf_scrypt));
        header.push_back(static_cast<char>(default_log2n));
        header.push_back(static_cast<char>(default_r));
        header.push_back(static_cast<char>(default_p));
        header.push_back(static_cast<char>(salt_size));
        header.append(reinterpret_cast<char const*>(salt), salt_size);
        header.push_back(static_cast<char>(nonce_size));
        header.append(reinterpret_cast<char const*>(nonce), nonce_size);
        return header;
    }

} // namespace

namespace file::vault {

    std::filesystem::path path(std::string const& username) { return config_dir() / username; }

    bool exists(std::string const& username) { return std::filesystem::exists(path(username)); }

    void write(std::string const& username, std::vector<credential> const& credentials,
               std::string const& password) {
        using namespace CryptoPP;

        // The plaintext identifies the account it belongs to. On read, the first line must match
        // the username being read under -- catching a vault renamed onto, or substituted for,
        // another account's file, which a MAC alone would not: the ciphertext would still be
        // internally valid, just for the wrong account.
        std::string plaintext = username + '\n';
        for (auto const& cred : credentials) {
            std::ostringstream line;
            line << cred;
            plaintext += line.str();
        }

        AutoSeededRandomPool rng;
        byte salt[salt_size];
        rng.GenerateBlock(salt, salt_size);
        byte nonce[nonce_size];
        rng.GenerateBlock(nonce, nonce_size);

        auto const key = derive_key(password, salt, default_log2n, default_r, default_p);
        std::string const header = build_header(salt, nonce);

        GCM<AES>::Encryption enc;
        enc.SetKeyWithIV(key.data(), key.size(), nonce, nonce_size);

        std::string ciphertext;
        AuthenticatedEncryptionFilter ef{enc, new StringSink{ciphertext}, false, tag_size};
        ef.ChannelPut(AAD_CHANNEL, reinterpret_cast<byte const*>(header.data()), header.size());
        ef.ChannelMessageEnd(AAD_CHANNEL);
        ef.ChannelPut(DEFAULT_CHANNEL, reinterpret_cast<byte const*>(plaintext.data()),
                      plaintext.size());
        ef.ChannelMessageEnd(DEFAULT_CHANNEL);

        write_file_atomic(path(username), header + ciphertext);
    }

    std::vector<credential> read(std::string const& username, std::string const& password) {
        using namespace CryptoPP;

        auto const p = path(username);
        if (!std::filesystem::exists(p)) {
            throw std::runtime_error{"no vault for " + username};
        }

        std::ifstream in{p, std::ios::binary};
        if (!in) {
            throw std::runtime_error{"could not open the vault for " + username};
        }
        std::string const bytes{std::istreambuf_iterator<char>{in},
                                std::istreambuf_iterator<char>{}};

        if (bytes.size() < header_size) {
            throw std::runtime_error{"not a Redux vault: file is smaller than the header"};
        }
        if (bytes.compare(0, magic_size, magic, magic_size) != 0) {
            throw std::runtime_error{"not a Redux vault: bad magic"};
        }

        auto const version = static_cast<std::uint8_t>(bytes[8]);
        auto const kdf_id = static_cast<std::uint8_t>(bytes[9]);
        auto const log2n = static_cast<std::uint8_t>(bytes[10]);
        auto const r = static_cast<std::uint8_t>(bytes[11]);
        auto const p_param = static_cast<std::uint8_t>(bytes[12]);
        auto const salt_len = static_cast<std::uint8_t>(bytes[13]);
        auto const nonce_len = static_cast<std::uint8_t>(bytes[nonce_len_offset]);

        if (version != format_version || kdf_id != kdf_scrypt) {
            throw std::runtime_error{"unsupported Redux vault format: unknown version or KDF"};
        }
        // A sanity range on the cost parameters, not the exact values written today: read must
        // still accept a vault written by an earlier build with different (but still sane)
        // scrypt parameters, which is why write always uses the current defaults but read honors
        // whatever the header declares. r and p are bounded for the same reason as log2n: they
        // reach scrypt before the tag check authenticates the header, so an out-of-range value
        // turns one corrupted byte into a huge pre-authentication allocation.
        if (log2n < min_log2n || log2n > max_log2n) {
            throw std::runtime_error{"unsupported Redux vault format: unreasonable scrypt cost"};
        }
        if (r < min_r || r > max_r || p_param < min_p || p_param > max_p) {
            throw std::runtime_error{
                "unsupported Redux vault format: unreasonable scrypt parameters"};
        }
        if ((128ull << log2n) * r > max_scrypt_memory) {
            throw std::runtime_error{
                "unsupported Redux vault format: unreasonable scrypt memory cost"};
        }
        if (salt_len != salt_size || nonce_len != nonce_size) {
            throw std::runtime_error{
                "unsupported Redux vault format: unexpected salt or nonce length"};
        }

        auto const* salt = reinterpret_cast<byte const*>(bytes.data() + salt_offset);
        auto const* nonce = reinterpret_cast<byte const*>(bytes.data() + nonce_offset);

        auto const key = derive_key(password, salt, log2n, r, p_param);

        GCM<AES>::Decryption dec;
        dec.SetKeyWithIV(key.data(), key.size(), nonce, nonce_size);

        std::string plaintext;
        AuthenticatedDecryptionFilter df{dec, new StringSink{plaintext},
                                         AuthenticatedDecryptionFilter::MAC_AT_END |
                                             AuthenticatedDecryptionFilter::THROW_EXCEPTION,
                                         tag_size};

        df.ChannelPut(AAD_CHANNEL, reinterpret_cast<byte const*>(bytes.data()), header_size);
        df.ChannelMessageEnd(AAD_CHANNEL);
        df.ChannelPut(DEFAULT_CHANNEL, reinterpret_cast<byte const*>(bytes.data() + header_size),
                      bytes.size() - header_size);
        df.ChannelMessageEnd(DEFAULT_CHANNEL);
        // A wrong password or any tampering with the header or ciphertext throws
        // CryptoPP::HashVerificationFilter::HashVerificationFailed here -- the two are
        // indistinguishable, which is exactly what account::valid_password relies on.

        std::istringstream plaintext_stream{plaintext};
        std::string stored_username;
        std::getline(plaintext_stream, stored_username);
        if (stored_username != username) {
            throw std::runtime_error{"vault username mismatch: expected " + username};
        }

        std::vector<credential> result;
        credential cred;
        while (plaintext_stream >> cred) {
            result.push_back(cred);
        }
        return result;
    }

} // namespace file::vault

namespace file::users {

    namespace {
        // OWASP CSV-injection neutralization: some spreadsheet programs interpret a cell as a
        // formula when it opens with one of these characters, which turns an exported credential
        // into a code-execution vector the moment someone double-clicks the file. Prefixing a
        // single quote inside the quotes defuses that without changing what the field displays.
        bool starts_with_formula_char(std::string const& field) {
            if (field.empty()) {
                return false;
            }
            switch (field.front()) {
            case '=':
            case '+':
            case '-':
            case '@':
            case '\t':
            case '\r':
                return true;
            default:
                return false;
            }
        }

        // Every field is quoted unconditionally -- it never needs to be inspected further to
        // decide, and it is what keeps a field safe even if a future caller adds one that can
        // contain a comma or a quote itself.
        std::string quote_field(std::string const& field) {
            std::string quoted = "\"";
            if (starts_with_formula_char(field)) {
                quoted += '\'';
            }
            for (char c : field) {
                if (c == '"') {
                    quoted += "\"\"";
                } else {
                    quoted += c;
                }
            }
            quoted += '"';
            return quoted;
        }
    } // namespace

    void writeToCSV(std::string const& filename, std::vector<credential> const& credentials) {
        std::ostringstream out;
        out << "ID,Company Name,Username,Password\n";

        int id = 1;
        for (auto const& cred : credentials) {
            out << id << ',' << quote_field(cred.company_name) << ',' << quote_field(cred.username)
                << ',' << quote_field(cred.password) << "\n";
            ++id;
        }

        write_file_atomic(filename, out.str());
    }

} // namespace file::users

namespace file::user_files {
    std::string filePath(std::string const& username) { return (config_dir() / username).string(); }
} // namespace file::user_files

namespace file::last_user {

    void save(std::string const& username) {
        try {
            auto const path = config_dir() / ".last_user";
            write_file_atomic(path, username);
        } catch (...) {
            // Remembering the username is a convenience. Never let it break signing in.
        }
    }

    std::string load() {
        try {
            auto const path = config_dir() / ".last_user";
            if (!std::filesystem::exists(path)) {
                return {};
            }

            std::ifstream in{path};
            std::string username;
            std::getline(in, username);

            return username;
        } catch (...) {
            return {};
        }
    }
} // namespace file::last_user

namespace file {

    account_busy::account_busy(std::string const& username)
        : std::runtime_error{"account '" + username + "' is locked by another Redux instance"} {}

    // The descriptor is closed by impl's own destructor, not by ~account_lock: move-assignment
    // replaces impl_ wholesale, and a close living in ~account_lock would never run for the impl
    // being replaced -- leaking the descriptor and holding the lock for the rest of the process.
    // Closing releases only the lock; the lock file itself is left in place, so acquiring it
    // again is just opening the same, already-existing file.
    struct account_lock::impl {
#ifdef _WIN32
        HANDLE handle = INVALID_HANDLE_VALUE;

        ~impl() {
            if (handle != INVALID_HANDLE_VALUE) {
                CloseHandle(handle);
            }
        }
#else
        int fd = -1;

        ~impl() {
            if (fd >= 0) {
                close(fd);
            }
        }
#endif
    };

#ifdef _WIN32
    account_lock::account_lock(std::string const& username) : impl_{std::make_unique<impl>()} {
        auto const lock_path = config_dir() / ("." + username + ".lock");

        // The share mode must not be 0: a second instance has to be able to open the file at all
        // so that LockFileEx is what arbitrates exclusivity -- with no sharing, the second open
        // itself fails with ERROR_SHARING_VIOLATION and the account_busy path below is
        // unreachable.
        impl_->handle = CreateFileW(lock_path.wstring().c_str(), GENERIC_READ | GENERIC_WRITE,
                                    FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_ALWAYS,
                                    FILE_ATTRIBUTE_NORMAL, nullptr);
        if (impl_->handle == INVALID_HANDLE_VALUE) {
            // Something else holding the file un-shared still means the account is in use, not
            // that Redux is broken.
            if (GetLastError() == ERROR_SHARING_VIOLATION) {
                throw account_busy{username};
            }
            throw std::runtime_error{"could not open the lock file for " + username};
        }

        OVERLAPPED overlapped{};
        if (!LockFileEx(impl_->handle, LOCKFILE_EXCLUSIVE_LOCK | LOCKFILE_FAIL_IMMEDIATELY, 0,
                        MAXDWORD, MAXDWORD, &overlapped)) {
            DWORD const err = GetLastError();
            CloseHandle(impl_->handle);
            impl_->handle = INVALID_HANDLE_VALUE;
            if (err == ERROR_LOCK_VIOLATION) {
                throw account_busy{username};
            }
            throw std::runtime_error{"could not lock " + lock_path.string()};
        }
    }
#else
    account_lock::account_lock(std::string const& username) : impl_{std::make_unique<impl>()} {
        auto const lock_path = config_dir() / ("." + username + ".lock");

        impl_->fd = open(lock_path.c_str(), O_CREAT | O_RDWR, 0600);
        if (impl_->fd < 0) {
            throw std::runtime_error{"could not open the lock file for " + username};
        }

        if (flock(impl_->fd, LOCK_EX | LOCK_NB) != 0) {
            int const err = errno;
            close(impl_->fd);
            impl_->fd = -1;
            if (err == EWOULDBLOCK) {
                throw account_busy{username};
            }
            throw std::runtime_error{"could not lock " + lock_path.string()};
        }
    }
#endif

    account_lock::~account_lock() = default;
    account_lock::account_lock(account_lock&&) noexcept = default;
    account_lock& account_lock::operator=(account_lock&&) noexcept = default;

} // namespace file
