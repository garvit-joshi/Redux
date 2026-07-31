#include "file.h"
#include "credential.h"

#include <cryptopp/default.h>
#include <cryptopp/filters.h>

#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <sstream>
#ifdef _WIN32
#include <userenv.h>
#pragma comment(lib, "Userenv.lib")
#endif

namespace {
    // Directory holding every file Redux owns. Empty when it cannot be determined, which makes
    // the callers below fall back to bare relative names in the working directory.
    std::filesystem::path config_dir() {
#ifdef _WIN32
        TCHAR szHomeDirBuf[MAX_PATH] = {0};

        // We need a process with query permission set
        HANDLE hToken = 0;
        OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken);
        DWORD BufSize = MAX_PATH;
        GetUserProfileDirectory(hToken, szHomeDirBuf, &BufSize);
        // Close handle opened via OpenProcessToken
        CloseHandle(hToken);
        std::filesystem::path path = std::filesystem::path{szHomeDirBuf} / "Redux";
#else
        char const* home = std::getenv("HOME");
        if (home == nullptr || std::string{home}.empty()) {
            std::cerr << "HOME environment variable is not set\n";
            return {};
        }
        std::filesystem::path path = std::filesystem::path{home} / ".config" / "Redux";
#endif
        std::filesystem::create_directories(path);
        return path;
    }
} // namespace

namespace file::user_files {

    std::string filePath(std::string const& username) {
        return (config_dir() / username).string();
    }
    std::string data(std::string const& username) { return filePath(username) + "_data"; }
    std::string account(std::string const& username) { return filePath(username); }
} // namespace file::user_files

namespace file::last_user {

    void save(std::string const& username) {
        try {
            auto const dir = config_dir();
            if (dir.empty()) {
                return;
            }
            std::filesystem::permissions(dir, std::filesystem::perms::owner_all);

            auto const path = dir / ".last_user";
            std::ofstream{path} << username;
            std::filesystem::permissions(path, std::filesystem::perms::owner_read |
                                                   std::filesystem::perms::owner_write);
        } catch (...) {
            // Remembering the username is a convenience. Never let it break signing in.
        }
    }

    std::string load() {
        try {
            auto const path = config_dir() / ".last_user";
            if (!std::filesystem::exists(path)) {
                return {};
            }

            std::ifstream in{path};
            std::string username;
            std::getline(in, username);

            return username;
        } catch (...) {
            return {};
        }
    }
} // namespace file::last_user

namespace file::users {

    void writeToCSV(std::string filename, credential const& user_credentials, int const number) {
        if (number == 1) {
            std::ofstream{filename, std::ios::out} << "ID,Company Name,Username,Password\n";
        }
        std::ofstream{filename, std::ios::app} << number << "," << user_credentials.company_name
                                               << "," << user_credentials.username << ","
                                               << user_credentials.password << "\n";
    }
} // namespace file::users

namespace file::credentials {
    void write(std::string const& filename, std::vector<credential> const& credentials,
               std::string const& password) {
        std::ostringstream plaintext;

        for (auto const& cre : credentials) {
            plaintext << cre;
        }

        file::crypt::write_encrypted(filename, plaintext.str(), password);
    }

    std::vector<credential> read(std::string const& filename, std::string const& password) {
        std::vector<credential> result;

        if (!std::filesystem::exists(filename)) {
            return result;
        }

        std::istringstream plaintext{file::crypt::read_decrypted(filename, password)};

        credential credential;
        while (plaintext >> credential) {
            result.push_back(credential);
        }

        return result;
    }
} // namespace file::credentials

namespace file::crypt {

    void write_encrypted(std::string const& filename, std::string const& plaintext,
                         std::string const& password) {
        using namespace CryptoPP;

        std::string ciphertext;
        StringSource{plaintext, true,
                     new DefaultEncryptorWithMAC{reinterpret_cast<byte const*>(password.data()),
                                                 password.size(), new StringSink{ciphertext}}};

        // Binary: the payload is ciphertext, and a text-mode stream would translate LF to CRLF
        // inside it on Windows and corrupt it.
        std::ofstream out{filename, std::ios::binary | std::ios::trunc};
        out.write(ciphertext.data(), static_cast<std::streamsize>(ciphertext.size()));
    }

    std::string read_decrypted(std::string const& filename, std::string const& password) {
        using namespace CryptoPP;

        std::ifstream in{filename, std::ios::binary};
        std::string const ciphertext{std::istreambuf_iterator<char>{in},
                                     std::istreambuf_iterator<char>{}};

        std::string plaintext;
        StringSource{ciphertext, true,
                     new DefaultDecryptorWithMAC{reinterpret_cast<byte const*>(password.data()),
                                                 password.size(), new StringSink{plaintext}}};

        return plaintext;
    }
} // namespace file::crypt
