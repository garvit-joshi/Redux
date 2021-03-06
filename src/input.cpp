#include "input.h"
#include "str.h"

#include <algorithm>
#include <iostream>
#include <iterator>
#include <limits>

namespace input {

    static std::string trim(std::string const& str) {
        std::string result;

        std::string spaces{"\t\v\f "};
        std::copy(str.begin() + str.find_first_not_of(spaces),
                  str.begin() + str.find_last_not_of(spaces) + 1, std::back_inserter(result));

        return result;
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
