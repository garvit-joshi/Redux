#include "feature.h"
#include "credential.h"
#include "input.h"
#include "str.h"
#include "user.h"

#include <iostream>

namespace feature::credentials {
    enum confirm {
        yes = 1,
        no,
    };

    static void print(credential const& credential, int const number) {
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

    bool add(std::vector<credential>& credentials) {
        int const num_of_credentials = input::choice(str::ask_num_of_credentials);
        for (int i = 0; i < num_of_credentials; ++i) {

            std::cout << str::clear_screen << "Enter credential " << i + 1 << " out of "
                      << num_of_credentials << "\n\n";

            credentials.push_back(get_credential());
        }

        return num_of_credentials > 0;
    }

    bool edit(std::vector<credential>& credentials) {
        int const id_to_edit = input::choice(str::credential_to_edit);
        if (!id_to_edit || id_to_edit > credentials.size()) {
            input::enter(str::credential_not_found);
            return false;
        }

        print(credentials[id_to_edit - 1], id_to_edit);
        if (input::choice(str::confirm_edit) == confirm::yes) {
            std::cout << str::clear_screen;
            credentials[id_to_edit - 1] = get_credential();
            return true;
        }

        return false;
    }

    bool remove(std::vector<credential>& credentials) {
        int const id_to_remove = input::choice(str::credential_to_remove);

        if (!id_to_remove || id_to_remove > credentials.size()) {
            input::enter(str::credential_not_found);
            return false;
        }

        print(credentials[id_to_remove - 1], id_to_remove);
        if (input::choice(str::confirm_remove) == confirm::yes) {
            credentials.erase(credentials.begin() + id_to_remove - 1);
            return true;
        }

        return false;
    }

    void print(std::vector<credential> const& credentials,int operation) {
        int i = 1;
        for (auto const& credential : credentials) {
            print(credential, i++);
        }
        if(operation==1)
            input::enter();
    }

    bool search(std::vector<credential>& credentials) {
        std::string const search_term_company = input::line(str::search_term_company);
        int i = 1;
        for (auto const& credential : credentials) {
            if (credential.company_name.find(search_term_company) != std::string::npos) {
                print(credential, i);
            }
            ++i;
        }

        input::enter();
        return false;
    }
} // namespace feature::credentials
