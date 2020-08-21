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
    char c{services_menu_options::Add};
    while (c != services_menu_options::Back) {
        
        std::cout << menu::clear_screen << menu::welcome_msg 
                  << user.name << '\n'<< menu::features_menu;
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
    decrypt(user.name + "_data", user.password);
    
    std::cout << menu::clear_screen;
    for (auto const& cre : read_credentials(std::ifstream{user.name + "_data"})) {
        std::cout << "Company name: " << cre.company_name << '\n'
                  << "Username    : " << cre.username << '\n'
                  << "Password    : " << cre.password << "\n\n";
    }
    
    encrypt(user.name + "_data", user.password);
    
    wait_for_enter();
}

void add_credentials(user const& user) {
    std::cout << menu::clear_screen
              << menu::ask_num_of_credentials;
    unsigned number_of_credentials = promt_num();

    std::vector<credential> credentials_to_append;
    
    for (unsigned i = 0; i < number_of_credentials; ++i) {
        credential credential;
    
        credential.company_name = promt_msg(menu::promt_company_name);
        credential.username = promt_msg(menu::promt_username);
        credential.password = promt_msg(menu::promt_password);
        
        credentials_to_append.push_back(credential);
    }
    
    decrypt(user.name + "_data", user.password);
    write_credentials(std::ofstream{user.name + "_data", std::ios::app}, credentials_to_append);
    encrypt(user.name + "_data", user.password);
}
