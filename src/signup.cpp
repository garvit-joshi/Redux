#include "signup.h"
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

namespace signup {
    static void exceeds_attempt() {
        std::cout << str::clear_screen;
        input::enter(str::exceeds_attempt);
    }

    // Acquired non-blocking right after the username is validated, and held by the caller for
    // the whole session. The lock alone cannot serialize two signups for the same name -- the
    // winner's lock is released when its session ends, and the loser then acquires it freely.
    // It is account::create's under-lock existence recheck that closes that race.
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
        std::string username = input::line(str::username);

        int input_attempt = 0;
        while (!account::valid_username(username) || account::exists(username)) {
            if (++input_attempt == 3) {
                exceeds_attempt();
                return std::make_pair(false, std::string{});
            }

            if (!account::valid_username(username)) {
                std::cout << str::invalid_username;
            } else {
                std::cout << username << str::ac_already_exists;
            }

            username = input::line(str::username);
        }

        return std::make_pair(true, username);
    }

    static std::pair<bool, std::string> valid_password() {
        std::string password = utils::read_password(str::password);

        int input_attempt = 0;
        constexpr auto min_pass_len = 8;
        while (password.size() < min_pass_len) {
            if (++input_attempt == 3) {
                exceeds_attempt();
                return {false, {}};
            }

            std::cout << str::min_pass_len << min_pass_len << "\n\n";
            password = utils::read_password(str::password);
        }

        return {true, password};
    }

    // A password typed once and mistyped creates an account nobody can get back into -- the new
    // vault format has no recovery path. Requiring the same password twice, with its own
    // exceeds-attempt budget on a mismatch, is the only guard against that.
    std::pair<bool, std::string> confirmed_password() {
        int mismatch_attempt = 0;
        while (true) {
            auto [valid, password] = valid_password();
            if (!valid) {
                return {false, {}};
            }

            std::cout << str::password_again << "\n";
            auto [valid_again, password_again] = valid_password();
            if (!valid_again) {
                return {false, {}};
            }

            if (password == password_again) {
                return {true, password};
            }

            if (++mismatch_attempt == 3) {
                exceeds_attempt();
                return {false, {}};
            }

            std::cout << str::password_mismatch;
        }
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

        if (auto [valid, password] = confirmed_password(); valid) {
            user.password = password;

            try {
                account::create(user);
            } catch (account::already_exists const&) {
                // Another instance registered this name while we were prompting. Its vault must
                // not be replaced -- the name is simply taken now.
                std::cout << user.name << str::ac_already_exists;
                input::enter();
                return;
            }

            file::last_user::save(user.name);

            after_signin_services::run(user);
        }
    }
} // namespace signup
