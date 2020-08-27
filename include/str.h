#ifndef STR_H
#define STR_H

#ifdef _WIN32
#include <ostream>
#endif

namespace str {
    using str_type = char const* const;

#ifdef _WIN32
    struct clear_screen {};
    constexpr clear_screen clear_screen;

    inline std::ostream& operator<<(std::ostream& os, struct clear_screen const&) {
        system("cls");
        return os;
    }

#else
    str_type clear_screen = "\033[2J\033[1;1H";
#endif // _WIN32

    str_type startup = R"(
              Welcome to Redux
            --------------------
            1. Login
            2. Signup
            3. About
            4. Exit

)";

    str_type const after_signin = R"(
            ------------------------
            1. Add Credential.
            2. Print all Credential.
            3. Search Credential.
            4. Edit Credential.
            5. Delete Credential.
            6. Logout.

)";

    str_type about = R"(
        Redux- A cross-platform Application for storing User-Data.

        Created By-sauravj7 and garvit--joshi.

        Git-Hub: https://github.com/garvit-joshi/Redux

)";

    str_type todo = "Feature not implemented.\n";
    str_type outro = "Thanks for using Redux.\n";
    str_type enter = "Press Enter to continue...";
    str_type exceeds_attempt = "Exceeds attempts going back to the start menu..\n\n";
    str_type ask_num_of_credentials = "How many Credential(s) you want to enter: ";
    str_type welcome = "\n\t\t Welcome ";

    str_type password = "Enter your Password: ";
    str_type username = "Enter your Username: ";

    str_type choice = "Enter your choice: ";
    str_type empty = "Empty.\n";
    str_type add_some_credentials = "Add some Credentials first\n\n";
    str_type search_term_company = "Enter the name of Company you're searching for: ";

    str_type invalid = "That's an Invalid Option\n\n";
    str_type try_again = "Wrong Input, Try Again...\n\n";

    str_type ac_not_exists = "'s account doesn't exists.\n\n";
    str_type ac_pass_incorrect = "'s password is not correct.\n\n";
    str_type ac_already_exists = "'s account is already exists.\n\n";
    str_type min_pass_len = "Minimum password length is ";

    str_type credential_to_remove = "Enter the credential id to remove: ";
    str_type confirm_edit = "Are you sure, you want to edit it.\n\nPress 1 for yes or 2 for no: ";
    str_type confirm_remove =
        "Are you sure, you want to remove it.\n\nPress 1 for yes or 2 for no: ";
    str_type credential_not_found = "Credential not found.\n\n";
    str_type credential_to_edit = "Enter the credential id to edit: ";

    str_type company_name = "Enter the Company's Name: ";
    str_type company_name_search = "Enter the Company's name you want to search: ";
    str_type company_name_edit = "Enter the Company's name you want to edit: ";
    str_type company_name_delete = "Enter the Company's name you want to delete: ";
} // namespace str

#endif // STR_H
