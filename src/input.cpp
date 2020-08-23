#include "input.h"
#include "str.h"

#include <iostream>
#include <limits>

namespace input {
    std::string line(char const* const msg) {
        std::string result;

        std::cout << msg;
        getline(std::cin, result);

        return result;
    }

    unsigned choice() {
        std::cout << str::choice;

        unsigned result = 0U;

        while (!(std::cin >> result)) {
            std::cout << str::try_again << str::choice;
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        }

        std::cin.ignore();

        return result;
    }

    void enter(char const* const msg) {
        std::cout << msg << str::enter;
        std::cin.ignore();
    }

    void enter() {
        std::cout << str::enter;
        std::cin.ignore();
    }
} // namespace input
