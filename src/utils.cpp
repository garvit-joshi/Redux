#include "utils.h"

#include "input.h"

#include <stdexcept>
#include <string>
#ifdef _WIN32
#include <Windows.h>
#else
#include <unistd.h>
#endif

namespace {
#ifdef _WIN32
    void set_console_echo(bool const enabled) {
        HANDLE const stdin_handle = GetStdHandle(STD_INPUT_HANDLE);

        DWORD mode = 0;
        if (GetConsoleMode(stdin_handle, &mode) == 0) {
            return;
        }

        if (enabled) {
            mode |= ENABLE_ECHO_INPUT;
        } else {
            mode &= ~ENABLE_ECHO_INPUT;
        }

        SetConsoleMode(stdin_handle, mode);
    }

    // The console is shared with the shell that launched Redux, so echo has to come back however
    // the scope is left.
    class echo_guard {
    public:
        echo_guard() { set_console_echo(false); }
        ~echo_guard() { set_console_echo(true); }

        echo_guard(echo_guard const&) = delete;
        echo_guard& operator=(echo_guard const&) = delete;
    };
#endif
} // namespace

namespace utils {
    std::string read_password(char const* const prompt) {
#ifdef _WIN32
        echo_guard const guard;

        return input::line(prompt);
#else
        // getpass manages the terminal itself, and returns null when it cannot open /dev/tty --
        // constructing a std::string from that would be undefined behaviour.
        char const* const entered = getpass(prompt);
        if (entered == nullptr) {
            throw std::runtime_error{"could not read a password from the terminal"};
        }

        return entered;
#endif
    }
} // namespace utils
