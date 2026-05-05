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
        while (getline(std::cin, str) && !valid(str)) {
            std::cout << str::try_again << msg;
        }

        return trim(str);
    }

    int choice(char const* const msg) {
        std::cout << msg;

        int result = 0;

        while (!(std::cin >> result)) {
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
