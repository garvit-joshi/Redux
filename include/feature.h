#ifndef FEATURE_H
#define FEATURE_H

#include <string>
#include <vector>

struct credential;
struct user;

class feature {
public:
    feature(std::string username, std::string password);

    void add();
    void edit();
    void remove();
    void search();
    void print();
    void export_to_csv(user const& user_);

private:
    void save() const;

    std::string username;
    std::string password;
    std::vector<credential> credentials;
};

#endif // FEATURE_H
