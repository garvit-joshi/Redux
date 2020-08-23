#include "logout.h"
#include "file.h"
#include "user.h"

void user_logout(user const& user) {
    file::remove(file::user_files::returning_user());
    file::crypt::encrypt(file::user_files::data(user.name), user.password);
}
