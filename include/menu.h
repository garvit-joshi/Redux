#pragma once

namespace menu {
    constexpr static char const* const clear_screen = "\033[2J\033[1;1H";
    constexpr static char const* const startup_menu = R"(
              Welcome
            ---------------
            1. Login
            2. Signup
            3. About
            4. Exit

    )";
    
    constexpr static char const* const features_menu = R"(
            ------------------------
            1. Add Credential(s)
            2. Print Credential(s)
            3. Back

    )";

    constexpr static char const* const about = R"(
            Program to encrypt your credentials.

            created by {apna} {apna} {naam}.
    )";
    
    constexpr static char const* const outro = "Thanks for using pwdstore.\n";
    constexpr static char const* const promt_enter = "\nPress Enter to continue...";
    constexpr static char const* const exceeds_attempt = "Exceeds attempts going back to start menu\n";
    constexpr static char const* const welcome_msg = "\n\n\t\t Welcome ";

    constexpr static char const* const promt_password = "Enter Your Password: ";
    constexpr static char const* const promt_username = "Enter Your Username: ";
    constexpr static char const* const promt_choice   = "Choose Your Option: ";
    constexpr static char const* const promt_num      = "Enter a number: ";
    constexpr static char const* const promt_invalid  = "That's a invalid Option: ";
} // namespace menu
