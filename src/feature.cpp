#include "feature.h"
#include "credential.h"
#include "input.h"
#include "str.h"
#include "user.h"

#include <iostream>

enum confirm {
    yes = 1,
    no,
};

static void print_credential(credential const& credential, int const number) {
    std::cout << "\n=========== " << number << " ===========\n"
              << "Company name : " << credential.company_name << '\n'
              << "Username     : " << credential.username << '\n'
              << "Password     : " << credential.password << "\n\n";
}

static credential get_credential() {
    return credential{
        input::line(str::company_name),
        input::line(str::username),
        input::line(str::password),
    };
}

void feature::add() {
    int const num_of_credentials = input::choice(str::ask_num_of_credentials);
    for (int i = 0; i < num_of_credentials; ++i) {

        std::cout << str::clear_screen << "Enter credential " << i + 1 << " out of "
                  << num_of_credentials << "\n\n";

        credentials.push_back(get_credential());
    }

    modified = num_of_credentials > 0;
}

void feature::edit() {
    int const id_to_edit = input::choice(str::credential_to_edit);
    if (!id_to_edit || id_to_edit > credentials.size()) {
        input::enter(str::credential_not_found);
        return;
    }

    print_credential(credentials[id_to_edit - 1], id_to_edit);
    if (input::choice(str::confirm_edit) == confirm::yes) {
        std::cout << str::clear_screen;
        credentials[id_to_edit - 1] = get_credential();

        modified = true;
    }
}

void feature::remove() {
    int const id_to_remove = input::choice(str::credential_to_remove);

    if (!id_to_remove || id_to_remove > credentials.size()) {
        input::enter(str::credential_not_found);
        return;
    }

    print_credential(credentials[id_to_remove - 1], id_to_remove);
    if (input::choice(str::confirm_remove) == confirm::yes) {
        credentials.erase(credentials.begin() + id_to_remove - 1);

        modified = true;
    }
}

void feature::print() {
    int i = 1;
    for (auto const& credential : credentials) {
        print_credential(credential, i++);
    }

    input::enter();
}

void feature::search() {
    std::string const search_term_company = input::line(str::search_term_company);
    int i = 1;
    for (auto const& credential : credentials) {
        if (credential.company_name.find(search_term_company) != std::string::npos) {
            print_credential(credential, i);
        }
        ++i;
    }

    input::enter();
}

bool feature::is_modified() const { return modified; }

void feature::unset_modified() { modified = false; }

std::vector<credential> const& feature::get_credentials() const {
    return credentials;
}
