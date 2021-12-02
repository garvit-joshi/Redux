#ifndef UTILS_H
#define UTILS_H

#include <string>
#ifdef _WIN32
#include <Windows.h>
#endif

namespace utils {
    void switchStdinEcho(std::string const& console_mode);
}

#endif // UTILITY FUNCTIONS