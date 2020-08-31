#include "feature.h"
#include "credential.h"
#include "input.h"
#include "str.h"
#include "user.h"

#include <algorithm>
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
    generate_n(back_inserter(credentials), num_of_credentials, [&, n = 1]() mutable {
        std::cout << str::clear_screen << "Enter credential " << n << " out of "
                  << num_of_credentials << "\n\n";
        ++n;
        return get_credential();
    });

    modified = num_of_credentials > 0;
}

static bool invalid_id(std::vector<credential> const& credentials, int const id) {
    return id < 1 || id > credentials.size();
}

void feature::edit() {
    int const id_to_edit = input::choice(str::credential_to_edit);

    if (invalid_id(credentials, id_to_edit)) {
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

    if (invalid_id(credentials, id_to_remove)) {
        input::enter(str::credential_not_found);
        return;
    }

    print_credential(credentials[id_to_remove - 1], id_to_remove);
    if (input::choice(str::confirm_remove) == confirm::yes) {
        credentials[id_to_remove - 1] = std::move(credentials.back());
        credentials.pop_back();

        modified = true;
    }
}

void feature::print() {
    for_each(cbegin(credentials), cend(credentials), [n = 1](auto const& credential) mutable {
        print_credential(credential, n);
        ++n;
    });

    input::enter();
}

void feature::search() {
    std::string const search_term_company = input::line(str::search_term_company);

    for_each(cbegin(credentials), cend(credentials), [&, n = 1](auto const& credential) mutable {
        if (credential.company_name.find(search_term_company) != std::string::npos) {
            print_credential(credential, n);
        }
        ++n;
    });

    input::enter();
}

bool feature::is_modified() const { return modified; }

void feature::unset_modified() { modified = false; }

std::vector<credential> const& feature::get_credentials() const { return credentials; }
