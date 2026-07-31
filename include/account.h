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
    bool valid_password(user const&);
    void create(user const&);
    void change_password(user const&, std::string const& password);
} // namespace account

#endif // ACCOUNT_H
