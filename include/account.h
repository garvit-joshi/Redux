#ifndef ACCOUNT_H
#define ACCOUNT_H

#include <string>

struct user;


namespace account {
    bool exists(std::string const& username);
    bool valid_password(user const&);
    void create(user const&);
    void change_password(user const&, std::string const& password);
} // namespace account

#endif // ACCOUNT_H
