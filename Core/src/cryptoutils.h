#pragma once
#ifndef CRYPTO_H
#define CRYPTO_H
#include "export.h"

#include <string>
#include <vector>
#include <openssl/x509.h>

class crypt_util
{
    public:
        crypt_util();
        virtual ~crypt_util();
        DLL_PUBLIC int Gen_ECDH_KeyPair( std::string &publickey, std::string &privatekey );
        DLL_PUBLIC static int Gen_Shared_Key( std::string svrpub, std::string &publickey,
                                              std::string &privatekey,
                                              std::string &sharedkey );
        DLL_PUBLIC static int Gen_Shared_Key( EC_KEY *s1, EC_KEY *publickey, EC_KEY *privatekey,
                                              std::string &sharedkey );
        DLL_PUBLIC static void gen_rand( std::vector<char> &buff, int len );
        DLL_PUBLIC static std::string MD5_string( std::string pwd );
        DLL_PUBLIC static std::vector<char> MD5_vector( std::vector<char> buf );
        DLL_PUBLIC static EC_KEY *getPubKeyX509( const char *cert, int len );
        DLL_PUBLIC static EC_KEY *getPrivKeyPKCS8( const char *cert, int len );
        DLL_PUBLIC static std::string getECPubkeyPoint( EC_KEY * );
        DLL_PUBLIC static std::string getECPrivkeyBN( EC_KEY *privkey );
};

#endif
