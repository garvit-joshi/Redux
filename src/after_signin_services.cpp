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

        feature feature{file::credentials::read(file::user_files::data(user.name))};
        auto execute = [&] (auto func) {
            (feature.*func)();
            if (feature.is_modified()) {
                file::credentials::write(file::user_files::data(user.name), feature.get_credentials());
                feature.unset_modified();
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
                execute(&feature::add);
                break;

            case menu::print:
                execute(&feature::print);
                break;

            case menu::search:
                execute(&feature::search);
                break;

            case menu::edit:
                execute(&feature::edit);
                break;

            case menu::remove:
                execute(&feature::remove);
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
