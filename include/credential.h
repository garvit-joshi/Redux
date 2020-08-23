#ifndef CREDENTIAL_H
#define CREDENTIAL_H

#include <istream>
#include <ostream>
#include <string>

struct credential {
    std::string company_name;
    std::string username;
    std::string password;
};

inline std::ostream& operator<<(std::ostream& os, credential const& credential) {
    return os << credential.company_name << '\n'
              << credential.username << '\n'
              << credential.password << '\n';
}

inline std::istream& operator>>(std::istream& is, credential& credential) {
    getline(is, credential.company_name);
    getline(is, credential.username);
    getline(is, credential.password);

    return is;
}

#endif // CREDENTIAL_H
