#ifndef INPUT_H
#define INPUT_H

#include <string>

namespace input {
    auto line(char const* msg) -> std::string;
    auto choice() -> unsigned;
    auto enter(char const* msg) -> void;
    auto enter() -> void;
} // namespace input

#endif // INPUT_H
