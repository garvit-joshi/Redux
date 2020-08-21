#pragma once

#include <string>

struct user {
    std::string name;
    std::string password;
};

inline std::string user_data_file(user const& user) {
    return user.name + "_data";
}
