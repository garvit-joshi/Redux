#include "login.h"
#include "account.h"
#include "after_signin_services.h"
#include "file.h"
#include "input.h"
#include "str.h"
#include "user.h"
#include "utils.h"

#include <iostream>
#include <string>
#include <utility>
#ifndef _WIN32
#include <unistd.h>
#else
#include <Windows.h>
#endif

namespace login {
    static void exceeds_attempt() {
        std::cout << str::clear_screen;
        input::enter(str::exceeds_attempt);
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

        int input_attempt = 0;
        while (!account::exists(username)) {
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
        user user{other.name};
#ifdef _WIN32
        utils::switchStdinEcho("ECHO_ON");
        user.password = input::line(str::password);
        utils::switchStdinEcho("ECHO_OFF");
#else
        user.password = getpass(str::password);
#endif
        int input_attempt = 0;
        while (!account::valid_password(user)) {
            if (++input_attempt == 3) {
                exceeds_attempt();
                return std::make_pair(false, std::string{});
            }

            std::cout << user.name << str::ac_pass_incorrect;
#ifdef _WIN32
            utils::switchStdinEcho("ECHO_ON");
            user.password = input::line(str::password);
            utils::switchStdinEcho("ECHO_OFF");
#else
            user.password = getpass(str::password);
#endif
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

        if (auto [valid, password] = valid_password(user); valid) {
            user.password = password;

            file::last_user::save(user.name);

            after_signin_services::run(user);
        }
    }
} // namespace login
