#include "catch.hpp"

#include "cryptoutils.h"
#include "debug.h"
#include "util.h"
#include <openssl/md5.h>
#include <string.h>

const char *X509_S_PUB_KEY =
    "-----BEGIN PUBLIC KEY-----\n"
    "MEYwEAYHKoZIzj0CAQYFK4EEAB8DMgAEYWrRgPBXe4tmbZz1oJTsEy4tT+5MH386SHdVLfIHjryv\n"
    "tCfm602m6vPoMMfnTtms\n"
    "-----END PUBLIC KEY-----";
const char *X509_S_PRIV_KEY =
    "-----BEGIN EC PRIVATE KEY-----\n"
    "MDYCAQAwEAYHKoZIzj0CAQYFK4EEAB8EHzAdAgEBBBiJS1LyT+kixvABZlDhC4UO1LbkNe6KHDI=\n"
    "-----END EC PRIVATE KEY-----";

TEST_CASE( "MD5 Password" )
{
    std::string pwdmd5hex, res = crypt_util::MD5_string( "1234567890" );
    util::Buffer2String( res, pwdmd5hex );
    CHECK( pwdmd5hex == "e807f1fcf82d132f9bb018ca6738a19f" );
}

//TEST_CASE( "TEA decrypt test" )
//{
//    std::string a( "73FDEA085E722C8EC4E7D8AEB36E24C64A0FFF21BCF8B49D" );
//    std::vector<char> b;
//    uint32_t keys[4] = {0};
//    util::String2Buffer( a, b );
//    std::vector<char> res;
//    crypt_util::TEA_decrypt( b, keys, res );
//    std::stringstream s_res;
//    for( char a : res ) {
//        s_res << a;
//    }
//    std::string tmp = s_res.str();
//    CHECK( tmp == "Test message" );
//}

TEST_CASE( "TEA en/de test" )
{
    std::vector<char> enc;
    std::vector<char> text;
    text.resize( 4 * sizeof( char ) );
    char *p = ( char * )&text.front();
    memcpy( p, "abcd", 4 * sizeof( char ) );
    uint32_t keys[4] = {0};
    std::vector<char> dec;
    std::string b;
    crypt_util::TEA_encrypt( text, keys, enc );
    util::Buffer2String(enc,b);
    DebugLog(D_INFO,D_CRYPT)<<"Encrypt: "<<b;
    crypt_util::TEA_decrypt( enc, keys, dec );
    CHECK( text == dec );
}

TEST_CASE( "ECDH test with public and private keys" )
{
    std::string s1( "020e14a939661cadbdaa0b177b6e8d2b067c310bdeadc09804" );
    std::string pubkey( "021348bccdb2621c2a302bf4d6bbb349c907509b8fd527bd75" );
    std::string prikey( "000000187eaed20dd5d153ed2b0e93bf695f5c6700fd87cbd150a85f" );
    std::string sharekey;
    CHECK( crypt_util::Gen_Shared_Key( s1, pubkey, prikey, sharekey ) == 0 );
    CHECK( sharekey == "2ff60ffdd54a4deab26d0a85e8b9573d" );
}

TEST_CASE( "pwdKey generate" )
{
    std::vector<char> a;
    crypt_util::get_pwdKey( "12345qwert!", 1234567890, a );
    std::string res;
    util::Buffer2String( a, res );
    CHECK( res == "f6e9d2544ee179eeadad328254dbf106" );
}

TEST_CASE( "ECDH sharedkey with java" )
{
	const char ppub[] =
		 "-----BEGIN PUBLIC KEY-----\n"
	"MEYwEAYHKoZIzj0CAQYFK4EEAB8DMgAEKVAh9y8CoUqOVmsuhiSIQATpxmBzKVkTenq/dFShPjYg\n"
	"SYx9WCP568dcdze13DNl\n"
	"-----END PUBLIC KEY-----";
	EC_KEY *mpublic,*mprivate,*ppublic;
	mpublic = crypt_util::getPubKeyX509(X509_S_PUB_KEY, strlen(X509_S_PUB_KEY));
	mprivate = crypt_util::getPrivKeyPKCS8(X509_S_PRIV_KEY, strlen(X509_S_PRIV_KEY));
	ppublic = crypt_util::getPubKeyX509(ppub, sizeof(ppub));
	DebugLog(D_INFO,D_CRYPT)<<"Publickey: "<<crypt_util::getECPubkeyPoint(mpublic);
	DebugLog(D_INFO,D_CRYPT)<<"Privatekey: "<<crypt_util::getECPrivkeyBN(mprivate);
	CHECK( mpublic != nullptr);
	CHECK( mprivate != nullptr);
	CHECK( ppublic != nullptr);
	std::string sharekey;
	CHECK( crypt_util::Gen_Shared_Key( ppublic, mpublic, mprivate, sharekey ) == 0 );
	EC_KEY_free( mpublic );
	EC_KEY_free( mprivate );
	EC_KEY_free( ppublic );
	CHECK( sharekey == "91f4e58e1c16d7f9f04df3b07d5bcb47" );
}
