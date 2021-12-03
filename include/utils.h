#ifndef UTILS_H
#define UTILS_H

#include <string>
#ifndef _WIN32
#include <unistd.h>
#else
#include <Windows.h>
#endif

namespace utils {
    void switchStdinEcho(std::string const& console_mode);
}

#endif // UTILITY FUNCTIONS