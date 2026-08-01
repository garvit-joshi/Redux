#ifndef UTILS_H
#define UTILS_H

#include <string>

namespace utils {
    // Turns on ANSI escape sequence interpretation for stdout on Windows 10+ consoles, so
    // str::clear_screen's escape codes clear the screen instead of printing literally. Purely
    // cosmetic: any failure is ignored. Call once, at startup. A no-op on POSIX.
    void enable_vt();

    // Reads a line of input without echoing it. On Windows this restores the exact console mode
    // that was active before the prompt on every exit path -- exceptions included -- rather than
    // unconditionally re-enabling echo, and throws instead of proceeding if echo cannot be turned
    // off on a real console, so a master password is never silently echoed to the screen. Piped
    // or redirected stdin -- no console to mute -- is read through unchanged.
    std::string read_password(char const* prompt);
} // namespace utils

#endif // UTILS_H
