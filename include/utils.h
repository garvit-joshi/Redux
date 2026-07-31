#ifndef UTILS_H
#define UTILS_H

#include <string>

namespace utils {
    // Reads a line of input without echoing it, restoring the terminal on every exit path --
    // exceptions included. Throwing out of a password prompt used to skip the re-enabling call
    // and leave the invoking Windows shell with echo switched off.
    std::string read_password(char const* prompt);
} // namespace utils

#endif // UTILS_H
