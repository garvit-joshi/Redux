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
#include <stdexcept>
#include <system_error>
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
    // '@' cannot appear in a valid username, which is what keeps this suffix from colliding with
    // another account: with the old "_data" suffix, registering "alice_data" silently took over
    // the vault belonging to "alice".
    std::string data(std::string const& username) { return filePath(username) + "@vault"; }
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

        // Write to a sibling temp file and rename it over the target. Opening the target with
        // trunc would destroy the old contents before the new bytes were known to be on disk, so
        // a failing write would leave a 0-byte vault -- unreadable, and unrecoverable.
        // The staging name is dot-prefixed so it cannot collide with a real user's file: a valid
        // username must begin with an alphanumeric, so no account file is ever named ".<x>.new".
        // Naming it "<filename>.new" would mean saving user "alice" clobbers user "alice.new".
        std::filesystem::path const target{filename};
        std::filesystem::path const staging =
            target.parent_path() / ('.' + target.filename().string() + ".new");

        {
            // Binary: the payload is ciphertext, and a text-mode stream would translate LF to
            // CRLF inside it on Windows and corrupt it.
            std::ofstream out{staging, std::ios::binary | std::ios::trunc};
            out.write(ciphertext.data(), static_cast<std::streamsize>(ciphertext.size()));
            out.flush();

            if (!out) {
                std::error_code ec;
                std::filesystem::remove(staging, ec);
                throw std::runtime_error{"could not write " + target.string()};
            }
        }

        // Owner-only before it is published, so the file is never briefly world-readable.
        std::filesystem::permissions(staging, std::filesystem::perms::owner_read |
                                                  std::filesystem::perms::owner_write);

        std::filesystem::rename(staging, target);
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
