#include "utils.h"

#include <iostream>
#include <string>
#ifdef _WIN32
#include <Windows.h>
#endif

namespace utils {
    void switchStdinEcho(std::string const& console_mode) {
        HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE);
        DWORD mode = 0;
        GetConsoleMode(hStdin, &mode);
        if (console_mode == "ECHO_ON") {
            mode &= ~ENABLE_ECHO_INPUT;
        } else {
            mode |= ENABLE_ECHO_INPUT;
        }
        SetConsoleMode(hStdin, mode);
    }
} // namespace utils
