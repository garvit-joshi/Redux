#ifndef FEATURE_H
#define FEATURE_H

#include <string>
#include <vector>

struct credential;
struct user;

class feature {
public:
    explicit feature(std::string username);

    void add();
    void edit();
    void remove();
    void search();
    void print();

private:
    void save() const;

    std::string username;
    std::vector<credential> credentials;
};

#endif // FEATURE_H
