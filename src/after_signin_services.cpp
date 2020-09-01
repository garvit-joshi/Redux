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
        change_pass,
        logout,
    };

    void run(user const& user) {
        feature feature{user.name};

        unsigned choice = menu::add; // making sure choice is not 'logout'.
        while (choice != menu::logout) {
            std::cout << str::clear_screen << str::welcome << std::quoted(user.name)
                      << str::after_signin;

            choice = input::choice();

            std::cout << str::clear_screen;

            switch (choice) {
            case menu::add:
                feature.add();
                break;

            case menu::print:
                feature.print();
                break;

            case menu::search:
                feature.search();
                break;

            case menu::edit:
                feature.edit();
                break;

            case menu::remove:
                feature.remove();
                break;

            case change_pass:
                if (change_password(user)) {
                    return;
                }
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
