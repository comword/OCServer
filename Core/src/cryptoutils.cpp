#include "cryptoutils.h"
#include "debug.h"
#include "util.h"
#include <stdio.h>
#include <openssl/md5.h>
#include <openssl/ssl.h>
#include <openssl/bn.h>
#include <openssl/bio.h>
#include <openssl/x509.h>
#include <openssl/pem.h>
#include <ctime>

int crypt_util::Gen_ECDH_KeyPair( std::string &publickey, std::string &privatekey )
{
    EC_KEY *eckey = EC_KEY_new_by_curve_name( NID_secp192k1 ); //711
    if( !eckey ) {
        DebugLog( D_WARNING, D_CRYPT ) << "EC_KEY_new_by_curve_name(NID_secp192k1) failed.";
        return -0x7;
    }
    int r5 = EC_KEY_generate_key( eckey );
    if( r5 != 1 ) {
        DebugLog( D_WARNING, D_CRYPT ) << "EC_KEY_generate_key failed return = " + std::to_string( r5 );
        return -0x55;
    }
    const EC_GROUP *group1 = EC_KEY_get0_group( eckey );
    const EC_POINT *point1 = EC_KEY_get0_public_key( eckey );

    //publickey
    char _pubkey[1024] = {0};
    int publen = EC_POINT_point2oct( group1, point1, POINT_CONVERSION_COMPRESSED,
                                     ( unsigned char * )_pubkey, 67, nullptr );
    std::string pubhex;
    util::Buffer2String( _pubkey, publen, pubhex );
    //DebugLog( D_INFO, D_CRYPT ) << "pubkey = " << pubhex;
    publickey = std::move( pubhex );

    //privatekey
    const BIGNUM *pribig = EC_KEY_get0_private_key( eckey );
    char pout[1024] = {0};
    int privlen = BN_bn2mpi( pribig, ( unsigned char * )pout );
    std::string privhex;
    util::Buffer2String( pout, privlen, privhex );
    //DebugLog( D_INFO, D_CRYPT ) << "privkey = " << privhex;
    privatekey = std::move( privhex );
    EC_KEY_free( eckey );
    return 0;
}

int crypt_util::Gen_Shared_Key( std::string peerpub, std::string &publickey,
                                std::string &privatekey,
                                std::string &sharedkey )
{
    if( peerpub.length() == 0 ) {
        DebugLog( D_WARNING, D_CRYPT ) << "svrpub can't be empty.";
        return -0x10;
    }
    if( privatekey.length() == 0 ) {
        DebugLog( D_WARNING, D_CRYPT ) << "privatekey can't be empty.";
        return -0x55;
    }
    EC_KEY *eckey = EC_KEY_new_by_curve_name( NID_secp192k1 ); //711
    if( !eckey ) {
        DebugLog( D_WARNING, D_CRYPT ) << "EC_KEY_new_by_curve_name(NID_secp192k1) failed.";
        return -0x7;
    }

    BIGNUM *bignum = BN_new();
    char buff[1024] = {0};
    int privkey_len = util::String2Buffer( ( char * )privatekey.c_str(), privatekey.length(),
                                           buff );
    BN_mpi2bn( ( unsigned char * )buff, privkey_len, bignum );
    memset( buff, 0, 1024 );
    if( !bignum ) {
        DebugLog( D_WARNING, D_CRYPT ) << "big number null.";
        return -0x5;
    }
    int r = EC_KEY_set_private_key( eckey, bignum );
    BN_free( bignum );
    if( r != 1 ) {
        DebugLog( D_WARNING, D_CRYPT ) << "EC_KEY_set_private_key failed.";
        return -0x1;
    }
    const EC_GROUP *group = EC_KEY_get0_group( eckey );
    EC_POINT *point = EC_POINT_new( group );

    if( publickey.length() > 0 ) { //provided pubkey
        int pubkey_len = util::String2Buffer( ( char * )publickey.c_str(), publickey.length(), buff );
        int r1 = EC_POINT_oct2point( group, point, ( unsigned char * )buff, pubkey_len, nullptr );
        if( r1 != 1 ) {
            DebugLog( D_WARNING, D_CRYPT ) << "EC_POINT_oct2point ret = " + std::to_string( r1 );
            return -0x11;
        }
    } else {
        int r2 = EC_POINT_mul( group, point, NULL, NULL, NULL, NULL );
        if( r2 != 1 ) {
            DebugLog( D_WARNING, D_CRYPT ) << "EC_POINT_mul failed = " + std::to_string( r2 );
            return -0x2;
        }
    }
    int r3 = EC_KEY_set_public_key( eckey, point );
    if( r3 != 1 ) {
        DebugLog( D_WARNING, D_CRYPT ) << "EC_KEY_set_public_key ret = " + std::to_string( r3 );
        return -0x3;
    }
    EC_POINT_free( point );

    const EC_GROUP *group1 = EC_KEY_get0_group( eckey );
    const EC_POINT *point1 = EC_KEY_get0_public_key( eckey );

    //publickey
    char _pubkey[1024] = {0};
    int publen = EC_POINT_point2oct( group1, point1, POINT_CONVERSION_COMPRESSED,
                                     ( unsigned char * )_pubkey, 67, nullptr );
    std::string pubhex;
    util::Buffer2String( _pubkey, publen, pubhex );
    //DebugLog( D_INFO, D_CRYPT ) << "pubkey = " << pubhex;
    publickey = std::move( pubhex );

    //privatekey
    const BIGNUM *pribig = EC_KEY_get0_private_key( eckey );
    char pout[1024] = {0};
    int privlen = BN_bn2mpi( pribig, ( unsigned char * )pout );
    std::string privhex;
    util::Buffer2String( pout, privlen, privhex );
    //DebugLog( D_INFO, D_CRYPT ) << "privkey = " << privhex;
    privatekey = std::move( privhex );

    //load server ecdh public key
    char _svrpub[1024] = {0};
    int svrpublen = util::String2Buffer( ( char * )peerpub.c_str(), peerpub.length(), _svrpub );
    EC_POINT *point2 = EC_POINT_new( group1 );
    int r4 = EC_POINT_oct2point( group1, point2, ( unsigned char * )_svrpub, svrpublen, nullptr );
    if( r4 != 1 ) {
        DebugLog( D_WARNING, D_CRYPT ) << "EC_POINT_oct2point ret = " + std::to_string( r4 );
        return -0x4;
    }

    //sharedkey
    char _sharedkey[1024] = {0};
    int sklen = ECDH_compute_key( _sharedkey, 512, point2, eckey, nullptr );
    if( sklen <= 0 ) {
        DebugLog( D_WARNING, D_CRYPT ) << "ECDH_compute_key ret = " + std::to_string( sklen );
        return -0x9;
    }
    std::string sharehex;
    util::Buffer2String( _sharedkey, sklen, sharehex );
    DebugLog( D_INFO, D_CRYPT ) << "sharedkey = " << sharehex;
    EC_POINT_free( point2 );

    unsigned char md5share[MD5_DIGEST_LENGTH];
    MD5( ( unsigned char * )_sharedkey, sklen, md5share );
    std::string share_md5hex;
    util::Buffer2String( ( char * )md5share, MD5_DIGEST_LENGTH, share_md5hex );
    DebugLog( D_INFO, D_CRYPT ) << "sharedkey_md5 = " << share_md5hex ;
    sharedkey = std::move( share_md5hex );

    EC_KEY_free( eckey );
    return 0;
}

int crypt_util::Gen_Shared_Key( EC_KEY *s1, EC_KEY *publickey, EC_KEY *privatekey,
                                std::string &sharedkey )
{
    EC_KEY *eckey = EC_KEY_new_by_curve_name( NID_secp192k1 );
    const EC_POINT *point1 = EC_KEY_get0_public_key( publickey );
    int r1 = EC_KEY_set_public_key( eckey, point1 );
    if( r1 != 1 ) {
        DebugLog( D_WARNING, D_CRYPT ) << "EC_KEY_set_public_key ret = " + std::to_string( r1 );
        return -0x1;
    }

    const BIGNUM *pribig = EC_KEY_get0_private_key( privatekey );
    int r2 = EC_KEY_set_private_key( eckey, pribig );
    if( r2 != 1 ) {
        DebugLog( D_WARNING, D_CRYPT ) << "EC_KEY_set_private_key ret = " + std::to_string( r2 );
        return -0x2;
    }
    const EC_GROUP *group1 = EC_KEY_get0_group( eckey );

    //load server ecdh public key
    const EC_POINT *ps1 = EC_KEY_get0_public_key( s1 );
    char _pubkey[1024] = {0};
    int svrpublen = EC_POINT_point2oct( group1, ps1, POINT_CONVERSION_COMPRESSED,
                                        ( unsigned char * )_pubkey, 67, nullptr );
    EC_POINT *point2 = EC_POINT_new( group1 );
    int r4 = EC_POINT_oct2point( group1, point2, ( unsigned char * )_pubkey, svrpublen, nullptr );
    if( r4 != 1 ) {
        DebugLog( D_WARNING, D_CRYPT ) << "EC_POINT_oct2point ret = " + std::to_string( r4 );
        return -0x4;
    }
    //sharedkey
    char _sharedkey[1024] = {0};
    int sklen = ECDH_compute_key( _sharedkey, 512, point2, eckey, nullptr );
    if( sklen <= 0 ) {
        DebugLog( D_WARNING, D_CRYPT ) << "ECDH_compute_key ret = " + std::to_string( sklen );
        return -0x9;
    }
    std::string sharehex;
    util::Buffer2String( _sharedkey, sklen, sharehex );
    DebugLog( D_INFO, D_CRYPT ) << "sharedkey = " << sharehex;
    EC_POINT_free( point2 );

    unsigned char md5share[MD5_DIGEST_LENGTH];
    MD5( ( unsigned char * )_sharedkey, sklen, md5share );
    std::string share_md5hex;
    util::Buffer2String( ( char * )md5share, MD5_DIGEST_LENGTH, share_md5hex );
    DebugLog( D_INFO, D_CRYPT ) << "sharedkey_md5 = " << share_md5hex ;
    sharedkey = std::move( share_md5hex );

    EC_KEY_free( eckey );
    return 0;
}

std::string crypt_util::MD5_string( std::string pwd )
{
    unsigned char pwdmd5[MD5_DIGEST_LENGTH + 1] = {0};
    MD5( ( unsigned char * )pwd.c_str(), pwd.length(), pwdmd5 );
    return std::string( ( char * )pwdmd5 );
}

std::vector<char> crypt_util::MD5_vector( std::vector<char> buf )
{
    unsigned char md5[MD5_DIGEST_LENGTH + 1] = {0};
    MD5( ( unsigned char * )&buf.front(), buf.size(), md5 );
    std::vector<char> res;
    for( int i = 0; i < MD5_DIGEST_LENGTH; i++ ) {
        res.push_back( *( md5 + i ) );
    }
    return res;
}

EC_KEY *crypt_util::getPubKeyX509( const char *cert, int len )
{
    BIO *bio;
    bio = BIO_new( BIO_s_mem() );
    // create BIO structure which deals with memory
    BIO_write( bio, cert, len );
    EVP_PKEY *pKey = nullptr;
    if( !PEM_read_bio_PUBKEY( bio, &pKey, 0, NULL ) ) {
        BIO_free( bio );
        DebugLog( D_WARNING, D_CRYPT ) << "Error while loading X509 public EC key.";
        return nullptr;
    }
    BIO_free( bio );
    EC_KEY *eckey = EVP_PKEY_get1_EC_KEY( pKey );
    EVP_PKEY_free( pKey );
    return eckey;
}

EC_KEY *crypt_util::getPrivKeyPKCS8( const char *cert, int len )
{
    BIO *bio;
    bio = BIO_new( BIO_s_mem() );
    // create BIO structure which deals with memory
    BIO_write( bio, cert, len );
    EVP_PKEY *pKey = nullptr;
    if( !PEM_read_bio_PrivateKey( bio, &pKey, 0, NULL ) ) {
        BIO_free( bio );
        DebugLog( D_WARNING, D_CRYPT ) << "Error while loading X509 private EC key.";
        return nullptr;
    }
    BIO_free( bio );
    EC_KEY *eckey = EVP_PKEY_get1_EC_KEY( pKey );
    EVP_PKEY_free( pKey );
    return eckey;
}

std::string crypt_util::getECPubkeyPoint( EC_KEY *pubkey )
{
    const EC_GROUP *group1 = EC_KEY_get0_group( pubkey );
    const EC_POINT *point1 = EC_KEY_get0_public_key( pubkey );

    //publickey
    char _pubkey[1024] = {0};
    int publen = EC_POINT_point2oct( group1, point1, POINT_CONVERSION_COMPRESSED,
                                     ( unsigned char * )_pubkey, 67, nullptr );
    std::string pubhex;
    util::Buffer2String( _pubkey, publen, pubhex );
    return std::string( pubhex );
}

std::string crypt_util::getECPrivkeyBN( EC_KEY *privkey )
{
    const BIGNUM *pribig = EC_KEY_get0_private_key( privkey );
    char pout[1024] = {0};
    int privlen = BN_bn2mpi( pribig, ( unsigned char * )pout );
    std::string privhex;
    util::Buffer2String( pout, privlen, privhex );
    return privhex;
}
