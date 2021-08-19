#include "account.h"
#include "file.h"
#include "user.h"

#include <filesystem>
#include <fstream>

namespace account {
    bool exists(std::string const& username) {
        return std::filesystem::exists(file::user_files::filePath(username));
    }

    bool valid_password(user const& user) {
        try {
            file::crypt::decrypt(file::user_files::filePath(user.name), user.password);
        } catch (...) {
            return false;
        }

        file::crypt::encrypt(file::user_files::filePath(user.name), user.password);
        return true;
    }

    void create(user const& user) {
        namespace uf = file::user_files;

        std::ofstream{uf::account(user.name)} << user.name;
        std::ofstream{uf::data(user.name)};

        file::crypt::encrypt(uf::account(user.name), user.password);
    }

    void change_password(user const& user, std::string const& password) {
        namespace uf = file::user_files;

        std::filesystem::remove(uf::account(user.name));

        std::ofstream{uf::account(user.name)} << user.name;
        file::crypt::encrypt(uf::account(user.name), password);
    }
} // namespace account
