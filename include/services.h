#pragma once

#include "filecrypt.h"
#include "input_util.h"
#include "menu.h"
#include "credential.h"
#include "user.h"

#include <algorithm>
#include <fstream>
#include <iostream>
#include <iterator>

enum services_menu_options {
    Add = '1',
    Print,
    Back,
};

inline void add_credentials(user const& user);
inline void print_credentials(user const& user);

inline void services(user const& user) {
    std::cout << menu::clear_screen 
              << menu::welcome_msg << user.name << '\n';

    char c{services_menu_options::Add};
    while (c != services_menu_options::Back) {
        
        std::cout << menu::features_menu;
        c = promt_choice();
        std::cout << menu::clear_screen;

        switch (c) {
        case services_menu_options::Add:
            add_credentials(user);
            break;

        case services_menu_options::Print:
            print_credentials(user);
            break;

        case services_menu_options::Back:
            return;

        default:
            std::cout << menu::promt_invalid;
            wait_for_enter();
            break;
        }
    }
}

void print_credentials(user const& user) {
    for (auto const& cre : read_credentials(std::ifstream{user.name + "_data"})) {
        std::cout << menu::clear_screen
                  << "Company name: " << cre.company_name << '\n'
                  << "Username    : " << cre.username << '\n'
                  << "Password    : " << cre.password << '\n';
    }
}

void add_credentials(user const& user) {
    std::cout << menu::clear_screen
              << menu::ask_num_of_credentials;

    unsigned number_of_credentials = promt_num();

    for (unsigned i = 0; i < number_of_credentials; ++i) {
        
    }
}
