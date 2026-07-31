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
    // The console is shared with the shell that launched Redux, so what comes back on scope exit
    // has to be the exact mode that was active before -- not just echo forced back on, which
    // would clobber any other mode bits (line input, mouse/window input, VT processing) the shell
    // had set and Redux never touched.
    //
    // If GetConsoleMode fails, stdin isn't a console at all -- a pipe or redirected file -- and
    // there is nothing to mute or restore, so the guard stays inactive and reads proceed as-is.
    // If it IS a console and echo cannot be turned off, silently continuing would echo a master
    // password to the screen, so that case fails closed instead.
    class echo_guard {
    public:
        echo_guard() {
            handle_ = GetStdHandle(STD_INPUT_HANDLE);
            if (GetConsoleMode(handle_, &original_mode_) == 0) {
                return;
            }

            active_ = true;

            if (SetConsoleMode(handle_, original_mode_ & ~ENABLE_ECHO_INPUT) == 0) {
                throw std::runtime_error{"could not disable console echo"};
            }
        }

        ~echo_guard() {
            if (active_) {
                SetConsoleMode(handle_, original_mode_);
            }
        }

        echo_guard(echo_guard const&) = delete;
        echo_guard& operator=(echo_guard const&) = delete;

    private:
        HANDLE handle_ = nullptr;
        DWORD original_mode_ = 0;
        bool active_ = false;
    };
#endif
} // namespace

namespace utils {
    void enable_vt() {
#ifdef _WIN32
        // Windows 10+ consoles don't interpret ANSI escape sequences until asked to -- without
        // this, str::clear_screen prints its escape codes literally instead of clearing anything.
        // Cosmetic only: unlike echo, nothing sensitive leaks if this silently no-ops, so any
        // failure (non-console stdout, an older console) is simply ignored.
        HANDLE const stdout_handle = GetStdHandle(STD_OUTPUT_HANDLE);

        DWORD mode = 0;
        if (GetConsoleMode(stdout_handle, &mode) == 0) {
            return;
        }

        SetConsoleMode(stdout_handle, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
#endif
    }

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
