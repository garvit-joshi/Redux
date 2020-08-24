#include "startup_services.h"
#include "input.h"
#include "login.h"
#include "returning_user_services.h"
#include "signup.h"
#include "str.h"

#include <iostream>

namespace startup_services {
    enum menu {
        login = 1,
        signup,
        about,
        exit,
    };

    void run() {
        returning_user_services::run();

        unsigned choice = menu::login; // making sure choice is not 'exit'.

        while (choice != menu::exit) {
            std::cout << str::clear_screen << str::startup;

            choice = input::choice();

            std::cout << str::clear_screen;

            switch (choice) {
            case menu::login:
                login::promt();
                break;

            case menu::signup:
                signup::promt();
                break;

            case menu::about:
                input::enter(str::about);
                break;

            case menu::exit:
                std::cout << str::outro;
                return;

            default:
                input::enter(str::invalid);
                break;
            }
        }
    }
} // namespace startup_services
