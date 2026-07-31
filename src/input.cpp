#include "input.h"
#include "str.h"

#include <iostream>
#include <limits>

namespace input {

    static std::string trim(std::string const& str) {
        std::string spaces{"\t\v\f "};
        auto const first = str.find_first_not_of(spaces);
        if (first == std::string::npos) {
            return {};
        }

        auto const last = str.find_last_not_of(spaces);
        return str.substr(first, last - first + 1);
    }

    static bool valid(std::string const& str) { return !str.empty() && !trim(str).empty(); }

    std::string line(char const* const msg) {
        std::cout << msg;

        std::string str;
        while (getline(std::cin, str)) {
            if (valid(str)) {
                return trim(str);
            }

            std::cout << str::try_again << msg;
        }

        throw end_of_input{};
    }

    std::string line_or(char const* const msg, std::string const& fallback) {
        std::cout << msg;

        std::string str;
        if (!getline(std::cin, str)) {
            throw end_of_input{};
        }

        auto trimmed = trim(str);

        return trimmed.empty() ? fallback : trimmed;
    }

    int choice(char const* const msg) {
        std::cout << msg;

        int result = 0;

        while (!(std::cin >> result)) {
            // Without this, clear() followed by ignore() re-sets eofbit immediately and the loop
            // spins forever printing the retry message.
            if (std::cin.eof()) {
                throw end_of_input{};
            }

            std::cout << str::try_again << msg;
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        }

        std::cin.ignore();

        return result;
    }

    void enter(char const* const msg) {
        if (msg != nullptr) {
            std::cout << msg;
        }

        std::cout << str::enter;
        std::cin.ignore();
    }

} // namespace input
