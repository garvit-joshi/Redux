#include "file.h"
#include "credential.h"
#include "user.h"

#include <cryptopp/default.h>
#include <cryptopp/files.h>
#include <cryptopp/filters.h>

#include <filesystem>
#include <fstream>
#include <iostream>
#ifndef _WIN32
#include <unistd.h>
#endif

namespace file::user_files {

    std::string filePath(std::string const& username) {
#ifdef _WIN32
        return username;
#else
        char* user;
        if ((user = getlogin()) == NULL) {
            perror("getlogin() error");
            return username;
        }
        std::string path = "/home/" + std::string(user) + "/.config/Redux/";
        std::filesystem::create_directories(path);
        return path + username;
#endif
    }
    std::string data(std::string const& username) { return filePath(username) + "_data"; }
    std::string account(std::string const& username) { return filePath(username); }
    std::string returning_user() { return "do_not_open"; }
} // namespace file::user_files

namespace file::users {

    user read(std::string const& filename) {
        user result;

        std::ifstream{filename} >> result;

        return result;
    }

    void write(std::string const& filename, user const& user) { std::ofstream{filename} << user; }
} // namespace file::users

namespace file::credentials {
    void write(std::string const& filename, std::vector<credential> const& credentials) {
        std::ofstream file{filename};

        for (auto const& cre : credentials) {
            file << cre;
        }
    }

    std::vector<credential> read(std::string const& filename) {
        std::vector<credential> result;

        std::ifstream file{filename};

        credential credential;
        while (file >> credential) {
            result.push_back(credential);
        }

        return result;
    }
} // namespace file::credentials

namespace file::crypt {

    void encrypt(std::string const& filename, std::string const& password) {
        using namespace CryptoPP;

        if (std::filesystem::is_empty(filename)) {
            return;
        }

        std::string encrypted_content;
        FileSource{filename.c_str(), true,
                   new DefaultEncryptorWithMAC{(byte*)password.data(), password.size(),
                                               new StringSink{encrypted_content}}};

        std::ofstream{filename, std::ios::trunc} << encrypted_content;
    }

    void decrypt(std::string const& filename, std::string const& password) {
        using namespace CryptoPP;

        if (std::filesystem::is_empty(filename)) {
            return;
        }

        std::string decrypted_content;
        FileSource{filename.c_str(), true,
                   new DefaultDecryptorWithMAC{(byte*)password.data(), password.size(),
                                               new StringSink{decrypted_content}}};

        std::ofstream{filename, std::ios::trunc} << decrypted_content;
    }
} // namespace file::crypt
