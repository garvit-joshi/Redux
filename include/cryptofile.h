#pragma once

#include <string>
using std::string;
using std::wstring;

#include <cryptopp/default.h>
using CryptoPP::DefaultEncryptorWithMAC;
using CryptoPP::DefaultDecryptorWithMAC;

#include <cryptopp/filters.h>
using CryptoPP::StringSource;
using CryptoPP::StringSink;

#include <cryptopp/files.h>
using CryptoPP::FileSource;

#include <cryptopp/hex.h>
using CryptoPP::HexEncoder;
using CryptoPP::HexDecoder;

#include<fstream>


int Encryption(string password,string fileName)
{
    /**********************************************************
     * Encrypted Data will be stored  in Encrypted string and 
     * then will be tranfered to filename given in Parameter
    *********************************************************/
    string encrypted;


    /**********************************************************
     * As FileSource take filename in wchar_t* 
     * The below two lines convert string to wchar_t*
    **********************************************************/
    wstring wide_string = wstring(fileName.begin(), fileName.end());
    const wchar_t* wfileName = wide_string.c_str();


    FileSource ss1(wfileName, true,
		new DefaultEncryptorWithMAC(
			(CryptoPP::byte*)password.data(), password.size(),
			new HexEncoder(
				new StringSink(encrypted)
			)
		)
	);


    std::ofstream{fileName,std::ios::trunc}<<encrypted;

    return 0;

}


int Decryption(string password,string fileName)
{
    string encrypted,decrypted;

    /**********************************************************
     * Encrypted data is fetched frm Fileand kept in a string
    **********************************************************/
    std::ifstream{fileName}>>encrypted;

    /********************************************************
     * Data being Decrypted
    ********************************************************/
    StringSource ss2(encrypted, true,
		new HexDecoder(
			new DefaultDecryptorWithMAC(
			(CryptoPP::byte*)password.data(), password.size(),
				new StringSink(decrypted)
			)
		)
	);


    /*******************************************************
     * If Password Is right the Encrypted Data from the file
     * will be deleted and the Original data will be kept.
    *******************************************************/
    std::ofstream{fileName,std::ios::trunc}<<decrypted;


    return 0;


}