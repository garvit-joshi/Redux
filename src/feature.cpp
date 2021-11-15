#include "feature.h"
#include "credential.h"
#include "file.h"
#include "input.h"
#include "str.h"
#include "user.h"

#include <algorithm>
#include <iostream>
#include <vector>

enum confirm {
    yes = 1,
    no,
};

void feature::save() const {
    file::credentials::write(file::user_files::data(username), credentials);
}

feature::feature(std::string username_)
    : username{std::move(username_)}, credentials{file::credentials::read(
                                          file::user_files::data(username))} {}

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

    if (num_of_credentials > 0) {
        save();
    }
}

static bool invalid_id(std::vector<credential> const& credentials, int const id) {
    return id < 1 || id > credentials.size();
}

void feature::edit() {
    if (credentials.empty()) {
        input::enter(str::add_some_credentials);
        return;
    }

    int const id_to_edit = input::choice(str::credential_to_edit);

    if (invalid_id(credentials, id_to_edit)) {
        input::enter(str::credential_not_found);
        return;
    }

    print_credential(credentials[id_to_edit - 1], id_to_edit);
    if (input::choice(str::confirm_edit) == confirm::yes) {
        std::cout << str::clear_screen;
        credentials[id_to_edit - 1] = get_credential();

        save();
    }
}

void feature::remove() {
    if (credentials.empty()) {
        input::enter(str::add_some_credentials);
        return;
    }

    int const id_to_remove = input::choice(str::credential_to_remove);

    if (invalid_id(credentials, id_to_remove)) {
        input::enter(str::credential_not_found);
        return;
    }

    print_credential(credentials[id_to_remove - 1], id_to_remove);
    if (input::choice(str::confirm_remove) == confirm::yes) {
        credentials[id_to_remove - 1] = std::move(credentials.back());
        credentials.pop_back();

        save();
    }
}

void feature::print() {
    if (credentials.empty()) {
        input::enter(str::add_some_credentials);
        return;
    }

    for_each(cbegin(credentials), cend(credentials), [n = 1](auto const& credential) mutable {
        print_credential(credential, n);
        ++n;
    });

    input::enter();
}

void feature::export_to_csv(user const& user_) {
    if (credentials.empty()) {
        input::enter(str::add_some_credentials);
        return;
    }

    std::string file_name = file::user_files::filePath(user_.name) + ".csv";

    for_each(cbegin(credentials), cend(credentials),
             [n = 1, file_name](auto const& credential) mutable {
                 // ToDO: add a check for the file existence
                 // ToDO: Make unique file name(with username)
                 file::users::writeToCSV(file_name, credential, n);
                 ++n;
             });

    std::cout << "\nData Saved to "<<file_name<<"\n\n\n";

    input::enter();
}

void feature::search() {
    bool cred_found = false;
    if (credentials.empty()) {
        input::enter(str::add_some_credentials);
        return;
    }

    std::string const search_term_company = input::line(str::search_term_company);

    for_each(cbegin(credentials), cend(credentials), [&, n = 1](auto const& credential) mutable {
        if (credential.company_name.find(search_term_company) != std::string::npos) {
            print_credential(credential, n);
            cred_found = true;
        }
        ++n;
    });
    if (cred_found == false) {
        std::cout << str::clear_screen << "No credential found in the database.\n\n";
    }
    input::enter();
}
