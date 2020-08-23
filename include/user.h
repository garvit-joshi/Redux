#ifndef USER_H
#define USER_H

#include <istream>
#include <ostream>
#include <string>

struct user {
    std::string name;
    std::string password;
};

inline std::ostream& operator<<(std::ostream& os, user const& user) {
    return os << user.name << '\n' << user.password << '\n';
}

inline std::istream& operator>>(std::istream& is, user& user) {
    getline(is, user.name);
    getline(is, user.password);

    return is;
}

#endif // USER_H
