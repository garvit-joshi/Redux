#ifndef USER_H
#define USER_H

#include <string>

// Deliberately not streamable. `user` holds a plaintext password, so giving it operator<< /
// operator>> is what previously let the master password be written to disk in the clear.
struct user {
    std::string name;
    std::string password;
};

#endif // USER_H
