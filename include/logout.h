#ifndef LOGOUT_H
#define LOGOUT_H

struct user;

void user_logout(user const&);
bool change_password(user const&);

#endif // LOGOUT_H
