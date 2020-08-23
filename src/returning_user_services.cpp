#include "returning_user_services.h"
#include "user.h"
#include "after_signin_services.h"
#include "file.h"

namespace returning_user_services {
    static bool returning_user() {
        return file::exists(file::user_files::returning_user());
    }

    static user get_returning_user() {
        return file::users::read(file::user_files::returning_user());
    }

    void run() {
        if (returning_user()) {
            after_signin_services::run(get_returning_user());
        }
    }
}
