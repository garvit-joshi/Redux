#pragma once

#include "account.h"
#include "input_util.h"
#include "menu.h"
#include "services.h"
#include "user.h"

#include <iostream>

inline void exceeds_attempt_msg() {
    std::cout << menu::clear_screen << menu::exceeds_attempt;
    wait_for_enter();
}

inline void save_current_user(user const& user) {
    std::ofstream{"do_not_open", std::ios::trunc} << user.name << '\n'
                                                  << user.password << '\n';
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

    decrypt(result.name + "_data", result.password);
    save_current_user(result);
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
    save_current_user(result);
    services(result);
}

inline bool someone_already_loggedin() {
    return exists(std::filesystem::path{"do_not_open"});
}

inline user get_loggedin_user() {
    user result;

    std::ifstream loggedin_user_file{"do_not_open"};
    getline(loggedin_user_file, result.name);
    getline(loggedin_user_file, result.password);

    return result;
}
