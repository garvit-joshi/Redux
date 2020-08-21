#pragma once

#include "account.h"
#include "input_util.h"
#include "menu.h"
#include "user.h"
#include "services.h"

#include <iostream>

inline void exceeds_attempt_msg() {
    std::cout << menu::clear_screen << menu::exceeds_attempt;
    wait_for_enter();
}

inline void promt_login() {
    user result;

    result.name = promt_msg(menu::promt_username);

    int username_attempt = 1;
    while (!account::exists(result)) {
        std::cerr << result.name << "' account not exists\n";
        
        result.name = promt_msg(menu::promt_username);

        if (++username_attempt == 3) {
            exceeds_attempt_msg();
            return;
        }
    }

    result.password = promt_msg(menu::promt_password);

    int password_attempt = 1;
    while (!account::valid_password(result)) {
        std::cout << "password not correct\n";
        
       result.password = promt_msg(menu::promt_password);

        if (++password_attempt == 3) {
            exceeds_attempt_msg();
            return;
        }
    }

    services(result);
}

inline void promt_signup() {
    user result;

    result.name = promt_msg(menu::promt_username);

    int username_attempt = 1;
    while (account::exists(result)) {
        std::cout << result.name << " already exists.\n";
        
        result.name = promt_msg(menu::promt_username);

        if (++username_attempt == 3) {
            exceeds_attempt_msg();
            return;
        }
    }

    result.password = promt_msg(menu::promt_password);

    int password_attempt = 1;
    const int min_pass_len = 8;
    while (result.password.size() < min_pass_len) {
        std::cerr << "Minimum password length is " << min_pass_len << '\n';
        
        result.password = promt_msg(menu::promt_password);

        if (++password_attempt == 3) {
            exceeds_attempt_msg();
            return;
        }
    }

    account::create(result);
    services(result);
}
