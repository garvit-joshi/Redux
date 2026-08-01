#ifndef FILE_H
#define FILE_H

#include <filesystem>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

struct credential;

// One file per account, at config_dir()/<username>, holding every credential the account owns,
// encrypted under the master password. There is no separate account file any more -- the old
// design wrote that file and the vault file independently, so an interruption between the two
// writes (most visibly during change_password) could strand the account on mismatched passwords.
namespace file::vault {
    void write(std::string const& username, std::vector<credential> const& credentials,
               std::string const& password);

    // Throws CryptoPP::HashVerificationFilter::HashVerificationFailed for a wrong password or a
    // tampered ciphertext/header (the two are cryptographically indistinguishable -- see
    // account::valid_password, which relies on that being the only exception with this meaning).
    // Throws std::runtime_error for anything else that makes the file untrustworthy: a missing
    // file, an unrecognized header (not a Redux vault, or scrypt parameters outside the range
    // this build accepts), or a plaintext username that does not match `username` (the file was
    // renamed onto, or substituted for, this account's).
    std::vector<credential> read(std::string const& username, std::string const& password);

    bool exists(std::string const& username);
    std::filesystem::path path(std::string const& username);
} // namespace file::vault

namespace file::users {
    // Builds the whole file in memory and publishes it through the same atomic, owner-only write
    // used by the vault (see write_file_atomic in file.cpp), so a failing export cannot leave a
    // half-written CSV behind. Every field is quoted and OWASP CSV-injection characters are
    // neutralized -- see file.cpp.
    void writeToCSV(std::string const& filename, std::vector<credential> const& credentials);
} // namespace file::users

namespace file::user_files {
    // Where "Export to CSV" writes. The '@' is what keeps this outside the vault namespace:
    // vault files live at config_dir()/<username> with no suffix, usernames may contain dots,
    // and '@' cannot appear in a valid username -- so no account can ever share a path with an
    // export. A plain "<username>.csv" name would be the vault file of an account literally
    // named "alice.csv", and exporting from "alice" would overwrite that vault with plaintext.
    std::string export_path(std::string const& username);
} // namespace file::user_files

// Remembers who logged in last so the login prompt can offer it as a default. Stores the
// username only -- never a password.
namespace file::last_user {
    void save(std::string const& username);
    std::string load();
} // namespace file::last_user

namespace file {

    // Thrown when a second Redux instance already holds the lock on this account.
    struct account_busy : std::runtime_error {
        explicit account_busy(std::string const& username);
    };

    // Held for the duration of a session -- login or signup, through after_signin_services::run
    // -- so a second Redux instance cannot interleave a read-modify-write cycle (most visibly
    // change_password) against the same account. Acquired non-blocking: construction throws
    // account_busy immediately rather than waiting for the other instance to finish. Move-only;
    // the lock file itself is never deleted, only the lock held on it is released, on
    // destruction -- deleting it would reopen the race between the delete and a concurrent
    // acquisition by another instance.
    class account_lock {
    public:
        explicit account_lock(std::string const& username);
        ~account_lock();

        account_lock(account_lock const&) = delete;
        account_lock& operator=(account_lock const&) = delete;
        account_lock(account_lock&&) noexcept;
        account_lock& operator=(account_lock&&) noexcept;

    private:
        struct impl;
        std::unique_ptr<impl> impl_;
    };

} // namespace file

#endif // FILE_H
