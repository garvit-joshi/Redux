#ifndef ACCOUNT_H
#define ACCOUNT_H

#include <string>

struct user;

namespace account {
    // A username becomes a filename, so it must not be able to alter the path it builds. Rejects
    // separators, traversal, absolute paths, and names that could collide with Redux's own state
    // files. Check this before any path is derived from a username.
    bool valid_username(std::string const& username);

    bool exists(std::string const& username);

    // Returns false only for a wrong password (indistinguishable from a tampered vault -- see
    // file::vault::read). Any other failure -- I/O errors, an unsupported vault format, a vault
    // whose recorded username does not match -- propagates instead of being reported as a wrong
    // password.
    bool valid_password(user const&);

    void create(user const&);

    // Reads the vault under the old password and writes it once under the new one: a single
    // atomic replace. The former two-file design wrote the account file and the vault file
    // independently, so an interruption between the two could strand the account on mismatched
    // passwords; there is only one file now, so there is nothing left to strand.
    void change_password(user const&, std::string const& password);
} // namespace account

#endif // ACCOUNT_H
