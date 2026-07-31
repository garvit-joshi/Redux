#ifndef INPUT_H
#define INPUT_H

#include "str.h"

#include <stdexcept>
#include <string>

namespace input {
    // Thrown when stdin is exhausted: Ctrl-D, or a redirected stdin that ran out. Quitting that
    // way is a normal user action, so main treats it as a clean exit rather than an error.
    struct end_of_input : std::runtime_error {
        end_of_input() : std::runtime_error{"end of input"} {}
    };

    auto line(char const* msg) -> std::string;
    // As `line`, but empty input yields `fallback` instead of re-prompting.
    auto line_or(char const* msg, std::string const& fallback) -> std::string;
    auto choice(char const* msg = str::choice) -> int;
    auto enter(char const* msg = nullptr) -> void;
} // namespace input

#endif // INPUT_H
