#include "input.h"
#include "startup_services.h"

#include <exception>
#include <iostream>

int main() {
    try {
        startup_services::run();
    } catch (input::end_of_input const&) {
        // Ctrl-D or an exhausted redirected stdin. A normal way to quit, not a failure.
        std::cout << '\n';
        return 0;
    } catch (std::exception const& e) {
        // Anything else -- an unreadable or corrupt vault, a failed write, a filesystem error.
        // Report it instead of letting it abort the process.
        std::cerr << "\nRedux stopped: " << e.what() << '\n';
        return 1;
    } catch (...) {
        std::cerr << "\nRedux stopped: unknown error\n";
        return 1;
    }
}
