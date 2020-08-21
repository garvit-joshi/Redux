#pragma once

#include "credential.h"
#include "filecrypt.h"
#include "input_util.h"
#include "menu.h"
#include "user.h"

#include <filesystem>
#include <iostream>

inline void print_credentials(user const& user) {
    std::string data_file = user.name + "_data";
    if (!exists(std::filesystem::path{data_file})) {
        wait_for_enter();
        return;
    }

    std::cout << menu::clear_screen;
    int i = 1;
    for (auto const& cre :
         read_credentials(std::ifstream{user.name + "_data"})) {
        std::cout << "=========== " << i++ << " ===========\n"
                  << "Company name : " << cre.company_name << '\n'
                  << "Username     : " << cre.username << '\n'
                  << "Password     : " << cre.password << "\n\n";
    }

    wait_for_enter();
}

inline void add_credentials(user const& user) {
    std::cout << menu::clear_screen << menu::ask_num_of_credentials;
    unsigned number_of_credentials = promt_num();

    if (!number_of_credentials) {
        return;
    }

    std::vector<credential> credentials_to_append;

    for (unsigned i = 0; i < number_of_credentials; ++i) {
        std::cout << menu::clear_screen << "Enter credential " << i + 1
                  << " out of " << number_of_credentials << "\n\n";

        credential credential;
        credential.company_name = promt_msg(menu::promt_company_name);
        credential.username = promt_msg(menu::promt_username);
        credential.password = promt_msg(menu::promt_password);

        credentials_to_append.push_back(credential);
    }

    std::string data_file = user.name + "_data";
    write_credentials(std::ofstream{data_file, std::ios::app},
                      credentials_to_append);
}

inline void search_credential(user const& user) {
    std::cout << menu::clear_screen;

    std::string data_file = user.name + "_data";

    if (!exists(std::filesystem::path{data_file})) {
        std::cout << "Add Some Credential first\n";
        wait_for_enter();
        return;
    }

    std::string const company_name = promt_msg(menu::promt_company_name_search);

    int i = 1;
    for (auto const& cre : read_credentials(std::ifstream{data_file})) {
        if (cre.company_name.find(company_name) != std::string::npos) {
            std::cout << "=========== " << i++ << " ===========\n"
                      << "Company name : " << cre.company_name << '\n'
                      << "Username     : " << cre.username << '\n'
                      << "Password     : " << cre.password << "\n\n";
        }
    }

    if (i == 1) {
        std::cout << "No Credential found for the given search settings\n";
    }

    wait_for_enter();
}

inline void logout(user const& user) {
    remove(std::filesystem::path{"do_not_open"});
    try {
        encrypt(user.name + "_data", user.password);
    } catch (...) {
    }
}
