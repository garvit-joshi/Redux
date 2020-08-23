#ifndef FILE_H
#define FILE_H

struct user;
struct credential;

#include <string>
#include <vector>

namespace file {
    bool exists(std::string const& filename);
    void remove(std::string const& filename);
    bool empty(std::string const& filename);
} // namespace file

namespace file::credentials {
    void write(std::string const& filename, std::vector<credential> const&);
    void append(std::string const&, std::vector<credential> const&);
    std::vector<credential> read(std::string const& filename);
} // namespace file::credentials

namespace file::users {
    user read(std::string const& filename);
    void write(std::string const& filename, user const&);
} // namespace file::users

namespace file::user_files {
    std::string data(std::string const& username);
    std::string account(std::string const& username);
    std::string returning_user();
} // namespace file::user_files

namespace file::crypt {
    void encrypt(std::string const& filename, std::string const& password);
    void decrypt(std::string const& filename, std::string const& password);
} // namespace file::crypt

#endif // FILECRYPT_H
