#include "after_signin_services.h"
#include "credential.h"
#include "feature.h"
#include "file.h"
#include "input.h"
#include "logout.h"
#include "str.h"
#include "user.h"

#include <iostream>

namespace after_signin_services {
    enum menu {
        add = 1U,
        print,
        search,
        edit,
        remove,
        logout,
    };

    void run(user const& user) {
        namespace fc = file::credentials;

        auto credentials = file::credentials::read(file::user_files::data(user.name));
        auto execute = [&credentials, &user](
                           bool run,
                           void (*operation)(std::string const&, std::vector<credential> const&)) {
            if (run) {
                operation(file::user_files::data(user.name), credentials);
            }
        };

        unsigned choice = menu::add; // making sure choice is not 'logout'.
        while (choice != menu::logout) {
            std::cout << str::clear_screen << str::after_signin;

            choice = input::choice();

            std::cout << str::clear_screen;

            switch (choice) {
            case menu::add:
                execute(feature::credentials::add(credentials), fc::append);
                break;

            case menu::print:
                feature::credentials::print(credentials);
                break;

            case menu::search:
                feature::credentials::search(credentials);
                break;

            case menu::edit:
                execute(feature::credentials::edit(credentials), fc::write);
                break;

            case menu::remove:
                execute(feature::credentials::remove(credentials), fc::write);
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
