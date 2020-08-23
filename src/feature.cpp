#include "feature.h"
#include "credential.h"
#include "input.h"
#include "str.h"
#include "user.h"

#include <iostream>

namespace feature::credentials {
    enum confirm {
        yes = 1U,
        no = 2,
    };

    static void print(credential const& credential, unsigned const number) {
        std::cout << "=========== " << number << " ===========\n"
                  << "Company name : " << credential.company_name << '\n'
                  << "Username     : " << credential.username << '\n'
                  << "Password     : " << credential.password << "\n\n";
    }

    static credential get_credential() {
        return credential{
            input::line(str::company_name),
            input::line(str::username),
            input::line(str::username),
        };
    }

    bool add(std::vector<credential>& credentials) {
        std::cout << str::clear_screen << str::ask_num_of_credentials;

        unsigned const num_of_credentials = input::choice();
        credential credential;
        for (unsigned i = 0; i < num_of_credentials; ++i) {

            std::cout << str::clear_screen << "Enter credential " << i + 1 << " out of "
                      << num_of_credentials << "\n\n";

            credentials.push_back(get_credential());
        }

        return num_of_credentials;
    }

    bool edit(std::vector<credential>& credentials) {
        std::cout << str::credential_to_edit;
        unsigned const id = input::choice() - 1U;

        if (id > credentials.size()) {
            input::enter(str::credential_not_found);
        }

        print(credentials[id], id + 1U);
        std::cout << str::confirm_edit;
        if (input::choice() == confirm::yes) {
            credentials[id] = get_credential();
            return true;
        }

        return false;
    }

    bool remove(std::vector<credential>& credentials) {
        std::cout << str::credential_to_remove;
        unsigned const id = input::choice() - 1U;

        if (id > credentials.size()) {
            input::enter(str::credential_not_found);
        }

        print(credentials[id], id + 1U);
        std::cout << str::confirm_remove;
        if (input::choice() == confirm::yes) {
            credentials.erase(credentials.begin() + id);
            return true;
        }

        return false;
    }

    void print(std::vector<credential> const& credentials) {
        unsigned i = 1;
        for (auto const& credential : credentials) {
            print(credential, i++);
        }

        input::enter();
    }

    void search(std::vector<credential> const& credentials) {
        if (credentials.empty()) {
            input::enter(str::add_some_credentials);
            return;
        }

        std::string const search_term_company = input::line(str::search_term_company);
        unsigned i = 1;
        for (auto const& credential : credentials) {
            if (credential.company_name.find(search_term_company) != std::string::npos) {
                print(credential, i);
            }
            ++i;
        }

        input::enter();
    }
} // namespace feature::credentials
