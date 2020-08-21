#include "input_util.h"
#include "login_signup.h"
#include "menu.h"

#include <iostream>
#include <string>

enum startup_menu_option {
    Login = '1',
    Signup,
    About,
    Exit,
};

int main() {
    char c{startup_menu_option::About};
    while (c != startup_menu_option::Exit) {
        if (someone_already_loggedin()) {
            services(get_loggedin_user());
        }

        std::cout << menu::clear_screen << menu::startup_menu;
        c = promt_choice();
        std::cout << menu::clear_screen;

        switch (c) {
        case startup_menu_option::Login:
            promt_login();
            break;

        case startup_menu_option::Signup:
            promt_signup();
            break;

        case startup_menu_option::About:
            std::cout << menu::about;
            wait_for_enter();
            break;

        case startup_menu_option::Exit:
            std::cout << menu::outro;
            return EXIT_SUCCESS;

        default:
            std::cout << menu::promt_invalid;
            wait_for_enter();
            break;
        }
    }
}
