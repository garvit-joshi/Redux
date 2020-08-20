#pragma once

#include "menu.h"

#include <iostream>
#include <string>

inline char promt_choice() {
    std::cout << menu::promt_choice;

    std::string choice;
    std::cin >> choice;

    return choice[0];
}

inline void wait_for_enter() {
    std::cout << menu::promt_enter;
    std::cin.ignore();
    std::cin.ignore();
}
