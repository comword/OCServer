#pragma once
#ifndef CRYPTO_H
#define CRYPTO_H
#include "export.h"

#include <string>
#include <vector>
#include <openssl/x509.h>

class DLL_PUBLIC crypt_util
{
    public:
        crypt_util();
        virtual ~crypt_util();
        int Gen_ECDH_KeyPair( std::string &publickey, std::string &privatekey );
        static int Gen_Shared_Key( std::string svrpub, std::string &publickey,
                                              std::string &privatekey,
                                              std::string &sharedkey );
        static int Gen_Shared_Key( EC_KEY *s1, EC_KEY *publickey, EC_KEY *privatekey,
                                              std::string &sharedkey );
        static void gen_rand( std::vector<char> &buff, int len );
        static std::string MD5_string( std::string pwd );
        static std::vector<char> MD5_vector( std::vector<char> buf );
        static EC_KEY *getPubKeyX509( const char *cert, int len );
        static EC_KEY *getPrivKeyPKCS8( const char *cert, int len );
        static std::string getECPubkeyPoint( EC_KEY * );
        static std::string getECPrivkeyBN( EC_KEY *privkey );
};

#endif
