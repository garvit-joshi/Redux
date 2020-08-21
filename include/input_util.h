#pragma once

#include "menu.h"

#include <ios>
#include <iostream>
#include <string>
#include <limits>

inline char promt_choice() {
    std::cout << menu::promt_choice;

    std::string choice;
    std::cin >> choice;

    return choice[0];
}

inline unsigned promt_num() {
    std::cout << menu::promt_num;

    int result;
    while (!(std::cin >> result)) {
        std::cout << "Wrong Input Try Again...\n" << menu::promt_num;
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }

    return result;
}

inline void wait_for_enter() {
    std::cout << menu::promt_enter;
    std::cin.ignore();
    std::cin.ignore();
}
