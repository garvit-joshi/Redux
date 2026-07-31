#include "account.h"
#include "credential.h"
#include "file.h"
#include "user.h"

#include <cryptopp/filters.h>

#include <algorithm>
#include <cctype>
#include <string>
#include <vector>

namespace account {
    bool valid_username(std::string const& username) {
        constexpr std::string::size_type max_len = 64;

        if (username.empty() || username.size() > max_len) {
            return false;
        }

        auto is_alnum = [](char c) { return std::isalnum(static_cast<unsigned char>(c)) != 0; };

        // Requiring an alphanumeric first character rules out ".", "..", and Redux's own
        // dot-prefixed state files (lock files, staging files, ".last_user") in one go.
        if (!is_alnum(username.front())) {
            return false;
        }

        return std::all_of(username.begin(), username.end(),
                           [&](char c) { return is_alnum(c) || c == '_' || c == '-' || c == '.'; });
    }

    bool exists(std::string const& username) { return file::vault::exists(username); }

    bool valid_password(user const& user) {
        // A wrong password and a tampered vault are cryptographically indistinguishable, and
        // HashVerificationFailed is the only exception that means one of those two things.
        // Everything else -- an I/O failure, an unrecognized header, a vault whose recorded
        // username does not match `user.name` -- is a different problem, and must not be
        // reported as "wrong password": it propagates so the caller's handler can report what
        // actually happened.
        try {
            file::vault::read(user.name, user.password);
            return true;
        } catch (CryptoPP::HashVerificationFilter::HashVerificationFailed const&) {
            return false;
        }
    }

    void create(user const& user) { file::vault::write(user.name, {}, user.password); }

    void change_password(user const& user, std::string const& password) {
        auto const credentials = file::vault::read(user.name, user.password);
        file::vault::write(user.name, credentials, password);
    }
} // namespace account
