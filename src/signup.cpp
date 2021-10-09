#include "signup.h"
#include "account.h"
#include "after_signin_services.h"
#include "file.h"
#include "input.h"
#include "str.h"
#include "user.h"

#include <iostream>
#include <string>
#include <utility>
#ifndef _WIN32
#include <unistd.h>
#else
#include <Windows.h>
#endif

namespace signup {
    static void exceeds_attempt() {
        std::cout << str::clear_screen;
        input::enter(str::exceeds_attempt);
    }

    static auto valid_username() {
        std::string username = input::line(str::username);

        int input_attempt = 0;
        while (account::exists(username)) {
            if (++input_attempt == 3) {
                exceeds_attempt();
                return std::make_pair(false, std::string{});
            }

            std::cout << username << str::ac_already_exists;
            username = input::line(str::username);
        }

        return std::make_pair(true, username);
    }

    std::pair<bool, std::string> valid_password() {
#ifdef _WIN32
        HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE);
        DWORD mode = 0;
        GetConsoleMode(hStdin, &mode);
        SetConsoleMode(hStdin, mode & (~ENABLE_ECHO_INPUT));
        std::string password = input::line(str::password);
#else
        std::string password = getpass(str::password);
#endif

        int input_attempt = 0;
        constexpr auto min_pass_len = 8;
        while (password.size() < min_pass_len) {
            if (++input_attempt == 3) {
                exceeds_attempt();
                return {false, {}};
            }

            std::cout << str::min_pass_len << min_pass_len << "\n\n";
#ifdef _WIN32
            HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE);
            DWORD mode = 0;
            GetConsoleMode(hStdin, &mode);
            SetConsoleMode(hStdin, mode & (~ENABLE_ECHO_INPUT));
            password = input::line(str::password);
#else
            password = getpass(str::password);
#endif
        }

        return {true, password};
    }

    void promt() {
        user user;

        if (auto [valid, username] = valid_username(); valid) {
            user.name = username;
        } else {
            return;
        }

        if (auto [valid, password] = valid_password(); valid) {
            user.password = password;

            account::create(user);

            file::users::write(file::user_files::returning_user(), user);

            after_signin_services::run(user);
        }
    }
} // namespace signup
