#include "login.h"
#include "account.h"
#include "after_signin_services.h"
#include "file.h"
#include "input.h"
#include "str.h"
#include "user.h"

#include <iostream>
#include <string>
#include <utility>

namespace login {
    static void exceeds_attempt() {
        std::cout << str::clear_screen;
        input::enter(str::exceeds_attempt);
    }

    static auto valid_username() {
        std::string username = input::line(str::username);

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
        user.password = input::line(str::password);

        int input_attempt = 0;
        while (!account::valid_password(user)) {
            if (++input_attempt == 3) {
                exceeds_attempt();
                return std::make_pair(false, std::string{});
            }

            std::cout << user.name << str::ac_pass_incorrect;
            user.password = input::line(str::password);
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

            file::crypt::decrypt(file::user_files::data(user.name), user.password);

            file::users::write(file::user_files::returning_user(), user);

            after_signin_services::run(user);
        }
    }
} // namespace login
