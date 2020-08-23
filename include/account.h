#ifndef ACCOUNT_H
#define ACCOUNT_H

struct user;

#include <string>

namespace account {
    bool exists(std::string const& username);
    bool valid_password(user const&);
    void create(user const&);
} // namespace account

#endif // ACCOUNT_H
