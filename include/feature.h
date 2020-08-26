#ifndef FEATURE_H
#define FEATURE_H

struct credential;
struct user;

#include <vector>

namespace feature::credentials {
    bool add(std::vector<credential>&);
    bool edit(std::vector<credential>&);
    bool remove(std::vector<credential>&);

    void print(std::vector<credential> const&,int = 1);
    bool search(std::vector<credential>&);
} // namespace feature::credentials

#endif // FEATURE_H
