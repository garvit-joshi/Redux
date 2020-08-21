#pragma once

#include "user.h"
#include "services.h"

#include <fstream>
#include <filesystem>
#include <string>

inline std::string returning_user_file() {
    return "do_not_open";
}

inline user get_loggedin_user() {
    user result;

    std::ifstream loggedin_user_file{returning_user_file()};
    getline(loggedin_user_file, result.name);
    getline(loggedin_user_file, result.password);

    return result;
}

inline void returning_user() {
    if (!exists(std::filesystem::path{returning_user_file()})) {
        return;
    }

    services(get_loggedin_user());
}

inline void save_current_user(user const& user) {
    std::ofstream{returning_user_file(), std::ios::trunc} << user.name << '\n'
                                                  << user.password << '\n';
}
