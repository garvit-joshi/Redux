#pragma once

#include <fstream>
#include <ostream>
#include <sstream>
#include <string>

#include <cryptopp/default.h>
#include <cryptopp/files.h>
#include <cryptopp/filters.h>
#include <cryptopp/hex.h>

inline void encrypt(std::string const& filename, std::string const& key) {
    using namespace CryptoPP;

    std::string encrypted;

    /**********************************************************
     * As FileSource take filename in wchar_t*
     * The below two lines convert string to wchar_t*
     **********************************************************/
    auto wide_string = std::wstring(filename.begin(), filename.end());
    const wchar_t* wfileName = wide_string.c_str();

    FileSource ss1(
        wfileName, true,
        new DefaultEncryptorWithMAC((CryptoPP::byte*)key.data(), key.size(),
                                    new HexEncoder(new StringSink(encrypted))));

    std::ofstream{filename, std::ios::trunc} << encrypted;
}

inline void decrypt(std::string const& filename, std::string const& key) {
    using namespace CryptoPP;

    std::string encrypted, decrypted;

    /**********************************************************
     * Encrypted data is fetched frm Fileand kept in a string
     **********************************************************/
    std::ifstream{filename} >> encrypted;

    /********************************************************
     * Data being Decrypted
     ********************************************************/
    StringSource ss2(encrypted, true,
                     new HexDecoder(new DefaultDecryptorWithMAC(
                         (CryptoPP::byte*)key.data(), key.size(),
                         new StringSink(decrypted))));

    /*******************************************************
     * If Password Is right the Encrypted Data from the file
     * will be deleted and the Original data will be kept.
     *******************************************************/
    std::ofstream{filename, std::ios::trunc} << decrypted;
}
