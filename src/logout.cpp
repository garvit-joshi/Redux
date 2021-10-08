#include "logout.h"
#include "account.h"
#include "file.h"
#include "input.h"
#include "signup.h"
#include "str.h"
#include "user.h"

#include <filesystem>
#include <iostream>
#include <string>
#include <utility>

void user_logout(user const& user) {
    std::filesystem::remove(file::user_files::returning_user());
    file::crypt::encrypt(file::user_files::data(user.name), user.password);
}

bool change_password(user const& user_) {
    std::cout << "\n";
    auto [valid, new_password] = signup::valid_password();
    if (!valid) {
        return false;
    }
    std::cout << str::password_again << "\n";
    auto [valid_again, new_password_again] = signup::valid_password();
    if (!valid_again) {
        return false;
    }

    if (new_password != new_password_again) {
        std::cout << str::clear_screen;
        std::cout << "Password do not match\n";
        input::enter();
        return false;
    }
    account::change_password(user_, new_password);

    std::cout << str::clear_screen << "Password Changed\n\n";

    input::enter();

    user_logout(user{user_.name, new_password});

    return true;
}
