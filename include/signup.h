#ifndef SIGNUP_H
#define SIGNUP_H

#include <string>

namespace signup {
    auto promt() -> void;
    auto valid_password() -> std::pair<bool, std::string>;
}

#endif // SIGNUP_H
