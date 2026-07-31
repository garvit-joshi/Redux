#include "account.h"
#include "credential.h"
#include "file.h"
#include "user.h"

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <string>
#include <vector>

namespace account {
    bool valid_username(std::string const& username) {
        constexpr std::string::size_type max_len = 64;

        if (username.empty() || username.size() > max_len) {
            return false;
        }

        auto is_alnum = [](char c) {
            return std::isalnum(static_cast<unsigned char>(c)) != 0;
        };

        // Requiring an alphanumeric first character rules out ".", "..", and Redux's own
        // dot-prefixed state files in one go.
        if (!is_alnum(username.front())) {
            return false;
        }

        return std::all_of(username.begin(), username.end(), [&](char c) {
            return is_alnum(c) || c == '_' || c == '-' || c == '.';
        });
    }

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
        //
        // Going through file::credentials rather than crypt directly means an absent vault is
        // handled here exactly as it is everywhere else, instead of throwing from a code path
        // that menu item 7 reaches directly.
        auto const credentials = file::credentials::read(uf::data(user.name), user.password);
        file::credentials::write(uf::data(user.name), credentials, password);

        // Each write is individually atomic, but the pair is not: an interruption between them
        // leaves the vault on the new password and the account file on the old one.
        file::crypt::write_encrypted(uf::account(user.name), user.name, password);
    }
} // namespace account
