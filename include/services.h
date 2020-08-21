#pragma once

#include "feature.h"
#include "input_util.h"
#include "menu.h"
#include "user.h"

#include <iostream>

enum services_menu_options {
    Add = '1',
    Print,
    Search,
    Edit,
    Logout,
};

inline void services(user const& user) {
    char c{services_menu_options::Add};
    while (c != services_menu_options::Logout) {
        std::cout << menu::clear_screen << menu::welcome_msg << user.name
                  << '\n'
                  << menu::features_menu;
        c = promt_choice();
        std::cout << menu::clear_screen;

        switch (c) {
        case services_menu_options::Add:
            add_credentials(user);
            break;

        case services_menu_options::Print:
            print_credentials(user);
            break;

        case services_menu_options::Search:
            search_credential(user);
            break;

        case services_menu_options::Edit:
            std::cout << menu::todo;
            wait_for_enter();
            break;

        case services_menu_options::Logout:
            logout(user);
            return;

        default:
            std::cout << menu::promt_invalid;
            wait_for_enter();
            break;
        }
    }
}
