#pragma once

#include "account.h"
#include "user.h"
#include "menu.h"
#include "input_util.h"

#include <iostream>

void after_login_services(user const& user) {
    std::cout << menu::clear_screen << menu::features_menu;
    wait_for_enter();
}

inline void exceeds_attempt_msg() {
    std::cout << menu::clear_screen << menu::exceeds_attempt;
    wait_for_enter();
}

inline void promt_login() {
    user result;

    std::cout << menu::promt_username;
    std::cin >> result.name;

    int username_attempt = 1;
    while (!account::exists(result)) {
        std::cerr << result.name << "' account not exists\n";
        std::cout << menu::promt_username;
        std::cin >> result.name;

        if (++username_attempt == 3) {
            exceeds_attempt_msg();
            return;
        }
    }

    std::cout << menu::promt_password;
    std::cin >> result.password;

    int password_attempt = 1;
    while (!account::valid_password(result)) {
        std::cout << "password not correct\n";
        std::cout << menu::promt_password;
        std::cin >> result.password;

        if (++password_attempt == 3) {
            exceeds_attempt_msg();
            return;
        }
    }

    after_login_services(result);
}

inline void promt_signup() {
    user result;

    std::cout << menu::promt_username;
    std::cin >> result.name;

    int username_attempt = 1;
    while (account::exists(result)) {
        std::cout << result.name << " already exists.\n";
        std::cout << menu::promt_username;
        std::cin >> result.name;

        if (++username_attempt == 3) {
            exceeds_attempt_msg();
            return;
        }
    }

    std::cout << menu::promt_password;
    std::cin >> result.password;

    int password_attempt = 1;
    const int min_pass_len = 8;
    while (result.password.size() < min_pass_len) {
        std::cerr << "Minimum password length is " << min_pass_len << '\n';
        std::cout << menu::promt_password;
        std::cin >> result.password;

        if (++password_attempt == 3) {
            exceeds_attempt_msg();
            return;
        }
    }

    account::create(result);
    after_login_services(result);
}
