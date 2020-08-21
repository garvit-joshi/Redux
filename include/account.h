#pragma once

#include "filecrypt.h"
#include "user.h"

#include <filesystem>
#include <fstream>

namespace account {
    inline bool valid_password(user const& user) {
        try {
            decrypt(user.name, user.password);
        } catch (...) {
            return false;
        }

        encrypt(user.name, user.password);
        return true;
    }

    inline bool exists(user const& user) {
        return exists(std::filesystem::path{user.name});
    }

    inline void create(user const& user) {
        std::ofstream{user.name} << user.name;

        encrypt(user.name, user.password);
    }
} // namespace account
