#pragma once

#include <fstream>
#include <ostream>
#include <sstream>
#include <string>

inline void encrypt(std::string const& filename, std::string const& key) {
    std::stringstream ss;
    ss << std::ifstream{filename}.rdbuf();
    std::string content = ss.str();

    for (unsigned i = 0; i < content.size(); i++) {
        content[i] ^= key[i % key.size()];
    }

    std::ofstream{filename, std::ios::trunc} << content;
}

inline void decrypt(std::string const& filename, std::string const& key) {
    std::stringstream ss;
    ss << std::ifstream{filename}.rdbuf();
    std::string content = ss.str();

    for (unsigned i = 0; i < content.size(); i++) {
        content[i] ^= key[i % key.size()];
    }

    std::ofstream{filename, std::ios::trunc} << content;
}
