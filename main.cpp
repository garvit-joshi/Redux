#include "login_signup.h"
#include "menu.h"
#include "input_util.h"

#include <iostream>
#include <string>

enum startup_menu_option {
    Login = '1',
    Signup,
    About,
    Exit,
};

int main() {
    std::cout << menu::clear_screen << menu::startup_menu;

    char c{startup_menu_option::About};
    while (c != startup_menu_option::Exit) {
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
