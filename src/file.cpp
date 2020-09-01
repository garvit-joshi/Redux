#include "file.h"
#include "credential.h"
#include "user.h"

#include <cryptopp/default.h>
#include <cryptopp/files.h>
#include <cryptopp/filters.h>

#include <filesystem>
#include <fstream>
#include <string>
#include <iostream>
#include <ios>

namespace file {
    bool exists(std::string const& filename) { return exists(std::filesystem::path{filename}); }
    void remove(std::string const& filename) { remove(std::filesystem::path{filename}); }
    bool empty(std::string const& filename) { return is_empty(std::filesystem::path{filename}); }
} // namespace file

namespace file::user_files {
    std::string data(std::string const& username) { return username + "_data"; }
    std::string account(std::string const& username) { return username; }
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

    static void save(std::string const& filename, std::vector<credential> const& credentials,
                     std::ios_base::openmode const mode) {
    }

    void write(std::string const& filename, std::vector<credential> const& credentials) {
        std::ofstream file{filename};

        for (auto const& cre : credentials) {
            file << cre;
        }
    }

    std::vector<credential> read(std::string const& filename) {
        std::vector<credential> result;

        std::ifstream file{filename, std::ios::app};

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

        if (file::empty(filename)) {
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

        if (file::empty(filename)) {
            return;
        }

        std::string decrypted_content;
        FileSource{filename.c_str(), true,
                   new DefaultDecryptorWithMAC{(byte*)password.data(), password.size(),
                                               new StringSink{decrypted_content}}};

        std::ofstream{filename, std::ios::trunc} << decrypted_content;
    }
} // namespace file::crypt
