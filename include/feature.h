#ifndef FEATURE_H
#define FEATURE_H

#include <vector>

struct credential;
struct user;

class feature {
public:
    explicit feature(std::vector<credential>&& credentials_)
        : credentials{std::move(credentials_)} {}

    void add();
    void edit();
    void remove();
    void search();
    void print();

    [[nodiscard]] bool is_modified() const;
    void unset_modified();

    [[nodiscard]] std::vector<credential> const& get_credentials() const;

private:
    std::vector<credential> credentials;
    bool modified = false;
};

#endif // FEATURE_H
