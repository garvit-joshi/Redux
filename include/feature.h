#ifndef FEATURE_H
#define FEATURE_H

struct credential;
struct user;

#include <vector>

namespace feature::credentials {
    bool add(std::vector<credential>&);
    bool edit(std::vector<credential>&);
    bool remove(std::vector<credential>&);

    void print(std::vector<credential> const&);
    void search(std::vector<credential> const&);
} // namespace feature::credentials

#endif // FEATURE_H
