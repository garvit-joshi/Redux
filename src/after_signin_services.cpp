#include "after_signin_services.h"
#include "credential.h"
#include "feature.h"
#include "file.h"
#include "input.h"
#include "logout.h"
#include "str.h"
#include "user.h"

#include <iomanip>
#include <iostream>

namespace after_signin_services {
    enum menu {
        add = 1,
        print,
        search,
        edit,
        remove,
        logout,
    };

    void run(user const& user) {
        namespace fic = file::credentials;
        namespace fec = feature::credentials;

        auto credentials = fic::read(file::user_files::data(user.name));

        auto execute = [&credentials](auto operation) {
            if (credentials.empty()) {
                input::enter(str::add_some_credentials);
                return false;
            }

            return operation(credentials);
        };

        auto save = [&credentials, &user](bool should_save, auto operation) {
            if (should_save) {
                operation(file::user_files::data(user.name), credentials);
            }
        };

        unsigned choice = menu::add; // making sure choice is not 'logout'.
        while (choice != menu::logout) {
            std::cout << str::clear_screen << str::welcome << std::quoted(user.name)
                      << str::after_signin;

            choice = input::choice();

            std::cout << str::clear_screen;

            switch (choice) {
            case menu::add:
                save(fec::add(credentials), fic::append);
                break;

            case menu::print:
                fec::print(credentials);
                break;

            case menu::search:
                execute(fec::search);
                break;

            case menu::edit:
                save(execute(fec::edit), fic::write);
                break;

            case menu::remove:
                save(execute(fec::remove), fic::write);
                break;

            case logout:
                user_logout(user);
                return;

            default:
                input::enter(str::invalid);
                break;
            }
        }
    }
} // namespace after_signin_services
