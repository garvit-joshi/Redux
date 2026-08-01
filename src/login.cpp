#include "login.h"
#include "account.h"
#include "after_signin_services.h"
#include "file.h"
#include "input.h"
#include "str.h"
#include "user.h"
#include "utils.h"

#include <iostream>
#include <optional>
#include <string>
#include <utility>

namespace login {
    static void exceeds_attempt() {
        std::cout << str::clear_screen;
        input::enter(str::exceeds_attempt);
    }

    // Acquired non-blocking right after the username is validated, and held by the caller for
    // the whole session: a second Redux instance signed into the same account could otherwise
    // race a read-modify-write cycle (most visibly change_password) against this one.
    static bool acquire_lock(std::string const& username, std::optional<file::account_lock>& lock) {
        try {
            lock.emplace(username);
            return true;
        } catch (file::account_busy const&) {
            std::cout << username << str::ac_in_use;
            input::enter();
            return false;
        }
    }

    static auto valid_username() {
        // Offer the last signed-in username as a default. Only the name is remembered; the
        // password is always required below.
        std::string const remembered = file::last_user::load();

        std::string username;
        if (remembered.empty()) {
            username = input::line(str::username);
        } else {
            std::string const msg =
                std::string{str::username_prefix} + remembered + str::username_suffix;
            username = input::line_or(msg.c_str(), remembered);
        }

        // Checking validity first means a rejected name never reaches the filesystem, so a
        // crafted username cannot be used to probe for files outside the config directory.
        int input_attempt = 0;
        while (!account::valid_username(username) || !account::exists(username)) {
            if (++input_attempt == 3) {
                exceeds_attempt();
                return std::make_pair(false, std::string{});
            }

            std::cout << username << str::ac_not_exists;
            username = input::line(str::username);
        }

        return std::make_pair(true, username);
    }

    static auto valid_password(user const& other) {
        user user{other.name, utils::read_password(str::password)};

        int input_attempt = 0;
        while (!account::valid_password(user)) {
            if (++input_attempt == 3) {
                exceeds_attempt();
                return std::make_pair(false, std::string{});
            }

            std::cout << user.name << str::ac_pass_incorrect;
            user.password = utils::read_password(str::password);
        }

        return std::make_pair(true, user.password);
    }

    void promt() {
        user user;

        if (auto [valid, username] = valid_username(); valid) {
            user.name = username;
        } else {
            return;
        }

        std::optional<file::account_lock> lock;
        if (!acquire_lock(user.name, lock)) {
            return;
        }

        if (auto [valid, password] = valid_password(user); valid) {
            user.password = password;

            file::last_user::save(user.name);

            after_signin_services::run(user);
        }
    }
} // namespace login
