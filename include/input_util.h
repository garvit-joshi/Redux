#pragma once

#include "menu.h"

#include <ios>
#include <iostream>
#include <limits>
#include <string>

inline std::string promt_msg(char const* const msg) {
    std::cout << msg;

    std::string result;
    getline(std::cin, result);

    return result;
}

inline char promt_choice() { return promt_msg(menu::promt_choice)[0]; }

inline unsigned promt_num() {
    std::cout << menu::promt_num;

    int result;
    while (!(std::cin >> result)) {
        std::cout << "Wrong Input Try Again...\n" << menu::promt_num;
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }

    std::cin.ignore();

    return result;
}

inline void wait_for_enter() {
    std::cout << menu::promt_enter;
    std::cin.ignore();
}
