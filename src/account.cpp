#include "account.h"
#include "file.h"
#include "user.h"

#include <filesystem>
#include <string>

namespace account {
    bool exists(std::string const& username) {
        return std::filesystem::exists(file::user_files::filePath(username));
    }

    bool valid_password(user const& user) {
        // The account file's plaintext is exactly the username, so decrypting it both
        // authenticates (via the MAC) and sanity-checks. Nothing is written, so a failed attempt
        // cannot leave the file in a state that locks the account out.
        try {
            return file::crypt::read_decrypted(file::user_files::account(user.name),
                                               user.password) == user.name;
        } catch (...) {
            return false;
        }
    }

    void create(user const& user) {
        namespace uf = file::user_files;

        file::crypt::write_encrypted(uf::account(user.name), user.name, user.password);
        file::crypt::write_encrypted(uf::data(user.name), "", user.password);
    }

    void change_password(user const& user, std::string const& password) {
        namespace uf = file::user_files;

        // Re-encrypt the vault itself under the new password. Nothing else does this: the files
        // are always encrypted at rest, so there is no logout step left to pick it up.
        // The two writes are not atomic with respect to each other; see the audit's C3.
        auto const data = file::crypt::read_decrypted(uf::data(user.name), user.password);
        file::crypt::write_encrypted(uf::data(user.name), data, password);

        file::crypt::write_encrypted(uf::account(user.name), user.name, password);
    }
} // namespace account
