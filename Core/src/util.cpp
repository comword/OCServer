#include "util.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

void util::int8_to_buf( char *buf, int i, int value )
{
    buf[i] = ( char ) value;
}
void util::int16_to_buf( char *buf, int i, int value )
{
    buf[i + 1] = ( char ) value;
    buf[i + 0] = ( char )( value >> 8 );
}
void util::int32_to_buf( char *buf, int i, int value )
{
    buf[i + 3] = ( char ) value ;
    buf[i + 2] = ( char )( value >> 8 );
    buf[i + 1] = ( char )( value >> 16 );
    buf[i + 0] = ( char )( value >> 24 );
}
void util::int64_to_buf( char *buf, int i, long value )
{
    buf[i + 7] = ( char ) value;
    buf[i + 6] = ( char )( value >> 8 );
    buf[i + 5] = ( char )( value >> 16 );
    buf[i + 4] = ( char )( value >> 24 );
    buf[i + 3] = ( char )( value >> 32 );
    buf[i + 2] = ( char )( value >> 40 );
    buf[i + 1] = ( char )( value >> 48 );
    buf[i + 0] = ( char )( value >> 56 );
}

/*hextobin*/
int util::String2Buffer( char *src, int srclen, char *dest )
{
    int i = 0;
    if( srclen % 2 != 0 ) {
        return 0;
    }
    for( i = 0; i < srclen / 2; i++ ) {
        char tmp[3];
        tmp[0] = *( src + 2 * i );
        tmp[1] = *( src + 2 * i + 1 );
        tmp[2] = 0;
        unsigned int out = 0;
        sscanf( tmp, "%x", &out );
        unsigned char ch = out & 0xff;
        *( dest + i ) = ch;
    }
    return i;
}

/*bintohex*/
int util::Buffer2String( char *src, int srclen, char *dest )
{
    int i;
    for( i = 0; i < srclen; i++ ) {
        char tmp[3] = { 0 };
        sprintf( tmp, "%x", *( src + i ) & 0xff );
        if( strlen( tmp ) == 1 ) {
            strcat( ( char * )dest, "0" );
            strncat( ( char * )dest, tmp, 1 );
        } else if( strlen( tmp ) == 2 ) {
            strncat( ( char * )dest, tmp, 2 );
        } else {
            strcat( ( char * )dest, "00" );
        }
    }
    return i * 2;
}

int util::Buffer2String( char *src, int srclen, std::string &dest )
{
    char *buff = ( char * )malloc( 2 * srclen * sizeof( char ) + 1 );
    memset( buff, 0, 2 * srclen * sizeof( char ) + 1 );
    int res = Buffer2String( src, srclen, buff );
    dest = std::string( buff );
    free( buff );
    return res;
}

int util::String2Buffer( std::string src, std::string &dest )
{
    int len = src.length();
    char *buff = ( char * )malloc( len * sizeof( char ) + 1 );
    memset( buff, 0, len * sizeof( char ) + 1 );
    int res = String2Buffer( ( char * )src.c_str(), len, buff );
    dest = std::string( buff );
    free( buff );
    return res;
}

int util::Buffer2String( std::string src, std::string &dest )
{
    int len = src.length();
    char *buff = ( char * )malloc( len * sizeof( char ) * 2 + 1 );
    memset( buff, 0, len * sizeof( char ) * 2 + 1 );
    int res = Buffer2String( ( char * )src.c_str(), len, buff );
    dest = std::string( buff );
    free( buff );
    return res;
}

int util::String2Buffer( std::string src, std::vector<char> &dest )
{
    int len = src.length();
    char *buff = ( char * )malloc( len * sizeof( char ) + 1 );
    memset( buff, 0, len * sizeof( char ) + 1 );
    int res = String2Buffer( ( char * )src.c_str(), len, buff );
    dest.clear();
    for( int i = 0; i < res; i++ ) {
        dest.push_back( buff[i] );
    }
    free( buff );
    return res;
}

int util::Buffer2String( std::vector<char> src, std::string &dest )
{
    int len = src.size();
    char *buff = ( char * )malloc( sizeof( char ) * len );
    int i = 0;
    for( char a : src ) {
        buff[i] = a;
        i++;
    }
    char *result = ( char * )malloc( len * sizeof( char ) * 2 + 1 );
    memset( result, 0, len * sizeof( char ) * 2 + 1 );
    int res = Buffer2String( buff, len, result );
    dest = std::string( result );
    free( buff );
    free( result );
    return res;
}
