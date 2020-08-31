#ifndef INPUT_H
#define INPUT_H

#include "str.h"

#include <string>

namespace input {
    auto line(char const* msg) -> std::string;
    auto choice(char const* msg = str::choice) -> int;
    auto enter(char const* msg = nullptr) -> void;
} // namespace input

#endif // INPUT_H
