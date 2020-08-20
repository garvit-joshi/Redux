#pragma once 

#include <ostream>
#include <istream>
#include <string>

struct credentials {
    std::string company_name;
    std::string username;
    std::string password;
};

inline std::istream& operator>>(std::istream& is, credentials& credentials) {
    getline(is, credentials.company_name);
    getline(is, credentials.username);
    getline(is, credentials.password);

    return is;
}

inline std::ostream& operator<<(std::ostream& os, credentials const& credentials) {
    return os << credentials.company_name << '\n'
              << credentials.username << '\n'
              << credentials.password << '\n';
}

inline void write_credentials_to_file(std::ostream&& os, credentials const& credentials) {
    os << credentials;
}

inline void read_credentials_from_file(std::istream&& is, credentials& credentials) {
    is >> credentials;
}