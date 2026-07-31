#ifndef SIGNUP_H
#define SIGNUP_H

#include <string>
#include <utility>

namespace signup {
    auto promt() -> void;

    // Prompts for a password twice and requires both entries to match, retrying the whole pair
    // up to 3 times on a mismatch. Shared by signup itself and by change_password, since both
    // create a master password with no recovery path if it's mistyped.
    auto confirmed_password() -> std::pair<bool, std::string>;
} // namespace signup

#endif // SIGNUP_H
