#pragma once

#include "user.h"
#include "filecrypt.h"

#include <filesystem>
#include <fstream>

namespace account {
    inline bool valid_password(user const& user) {
        decrypt(user.name, user.password);

        std::string decrypted_username;
        getline(std::ifstream{user.name}, decrypted_username);
        
        encrypt(user.name, user.password);

        return user.name == decrypted_username;
    }

    inline bool exists(user const& user) {
        return exists(std::filesystem::path{user.name});
    }

    inline void create(user const& user) {
        std::ofstream{user.name} << user.name;

        encrypt(user.name, user.password);
    }
} // namespace account
