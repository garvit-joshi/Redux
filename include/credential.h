#pragma once

#include <istream>
#include <ostream>
#include <string>
#include <vector>

struct credential {
    std::string company_name;
    std::string username;
    std::string password;
};

inline std::istream& operator>>(std::istream& is, credential& credential) {

    getline(is, credential.company_name);
    getline(is, credential.username);
    getline(is, credential.password);

    return is;
}

inline std::vector<credential> read_credentials(std::istream&& is) {
    std::vector<credential> result;

    credential credential;
    while (is >> credential) {
        result.push_back(credential);
    }

    return result;
}

inline std::ostream& operator<<(std::ostream& os,
                                credential const& credential) {
    return os << credential.company_name << '\n'
              << credential.username << '\n'
              << credential.password << '\n';
}

inline void write_credentials(std::ostream&& os,
                              std::vector<credential> const& credentials) {
    for (auto const& cre : credentials) {
        os << cre;
    }
}
