#include "logout.h"
#include "account.h"
#include "input.h"
#include "signup.h"
#include "str.h"
#include "user.h"

#include <iostream>
#include <string>
#include <utility>

bool change_password(user const& user_) {
    std::cout << "\n";
    auto [valid, new_password] = signup::confirmed_password();
    if (!valid) {
        return false;
    }

    account::change_password(user_, new_password);

    std::cout << str::clear_screen << "Password Changed\n\n";

    input::enter();

    // Returning true sends the caller back to the startup menu, which is the re-authentication
    // we want. There is no logout-time file work left: the vault is always encrypted at rest.
    return true;
}
