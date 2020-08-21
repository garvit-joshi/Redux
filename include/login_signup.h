#pragma once

#include "account.h"
#include "input_util.h"
#include "menu.h"
#include "services.h"
#include "user.h"

#include <iostream>


inline std::string user_data_file(user const& user) {
    return user.name + "_data";
}

inline void exceeds_attempt_msg() {
    std::cout << menu::clear_screen << menu::exceeds_attempt;
    wait_for_enter();
}

inline void save_current_user(user const& user) {
    std::ofstream{"do_not_open", std::ios::trunc} << user.name << '\n'
                                                  << user.password << '\n';
}

inline void promt_login() {
    user user;

    user.name = promt_msg(menu::promt_username);

    int username_attempt = 1;
    while (!account::exists(user)) {
        std::cerr << user.name << "' account not exists\n";

        user.name = promt_msg(menu::promt_username);

        if (++username_attempt == 3) {
            exceeds_attempt_msg();
            return;
        }
    }

    user.password = promt_msg(menu::promt_password);

    int password_attempt = 1;
    while (!account::valid_password(user)) {
        std::cout << "password not correct\n";

        user.password = promt_msg(menu::promt_password);

        if (++password_attempt == 3) {
            exceeds_attempt_msg();
            return;
        }
    }

    decrypt(user_data_file(user), user.password);
    save_current_user(user);
    services(user);
}

inline void promt_signup() {
    user user;

    user.name = promt_msg(menu::promt_username);

    int username_attempt = 1;
    while (account::exists(user)) {
        std::cout << user.name << " already exists.\n";

        user.name = promt_msg(menu::promt_username);

        if (++username_attempt == 3) {
            exceeds_attempt_msg();
            return;
        }
    }

    user.password = promt_msg(menu::promt_password);

    int password_attempt = 1;
    constexpr int min_pass_len = 8;
    while (user.password.size() < min_pass_len) {
        std::cerr << "Minimum password length is " << min_pass_len << '\n';

        user.password = promt_msg(menu::promt_password);

        if (++password_attempt == 3) {
            exceeds_attempt_msg();
            return;
        }
    }

    account::create(user);
    save_current_user(user);
    services(user);
}

inline user get_loggedin_user() {
    user result;

    std::ifstream loggedin_user_file{"do_not_open"};
    getline(loggedin_user_file, result.name);
    getline(loggedin_user_file, result.password);

    return result;
}

inline void returning_user() {
    if (!exists(std::filesystem::path{"do_not_open"})) {
        return;
    }

    services(get_loggedin_user());
}
