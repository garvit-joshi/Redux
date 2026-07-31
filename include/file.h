#ifndef FILE_H
#define FILE_H

#include <string>
#include <vector>

struct credential;

namespace file::credentials {
    void write(std::string const& filename, std::vector<credential> const&,
               std::string const& password);
    std::vector<credential> read(std::string const& filename, std::string const& password);
} // namespace file::credentials

namespace file::users {
    void writeToCSV(std::string filename, credential const& user_credentials, int const number);
} // namespace file::users

namespace file::user_files {
    std::string filePath(std::string const& username);
    std::string data(std::string const& username);
    std::string account(std::string const& username);
} // namespace file::user_files

// Remembers who logged in last so the login prompt can offer it as a default. Stores the
// username only -- never a password.
namespace file::last_user {
    void save(std::string const& username);
    std::string load();
} // namespace file::last_user

namespace file::crypt {
    // Encrypts `plaintext` under `password` and writes the ciphertext to `filename`.
    void write_encrypted(std::string const& filename, std::string const& plaintext,
                         std::string const& password);

    // Returns the plaintext of `filename`. Throws if the file is missing or unreadable, or if
    // `password` is wrong -- the MAC makes a wrong password indistinguishable from corruption.
    std::string read_decrypted(std::string const& filename, std::string const& password);
} // namespace file::crypt

#endif // FILE_H
