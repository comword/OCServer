#include "charutils.h"

#include <regex>
#include <sstream>
/*
   base64 encoding and decoding with C++.
   Version: 1.01.00
   Copyright (C) 2004-2017 René Nyffenegger
   This source code is provided 'as-is', without any express or implied
   warranty. In no event will the author be held liable for any damages
   arising from the use of this software.
   Permission is granted to anyone to use this software for any purpose,
   including commercial applications, and to alter it and redistribute it
   freely, subject to the following restrictions:
   1. The origin of this source code must not be misrepresented; you must not
      claim that you wrote the original source code. If you use this source code
      in a product, an acknowledgment in the product documentation would be
      appreciated but is not required.
   2. Altered source versions must be plainly marked as such, and must not be
      misrepresented as being the original source code.
   3. This notice may not be removed or altered from any source distribution.
   René Nyffenegger rene.nyffenegger@adp-gmbh.ch
*/
static const std::string base64_chars =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "abcdefghijklmnopqrstuvwxyz"
    "0123456789+/";


static inline bool is_base64( unsigned char c )
{
    return ( isalnum( c ) || ( c == '+' ) || ( c == '/' ) );
}

std::string base64_encode( std::string str )
{
    return base64_encode( ( unsigned char * )str.c_str(), str.length() );
}

std::string base64_encode( unsigned char const *bytes_to_encode, unsigned int in_len )
{
    std::string ret;
    int i = 0;
    int j = 0;
    unsigned char char_array_3[3];
    unsigned char char_array_4[4];

    while( in_len-- ) {
        char_array_3[i++] = *( bytes_to_encode++ );
        if( i == 3 ) {
            char_array_4[0] = ( char_array_3[0] & 0xfc ) >> 2;
            char_array_4[1] = ( ( char_array_3[0] & 0x03 ) << 4 ) + ( ( char_array_3[1] & 0xf0 ) >> 4 );
            char_array_4[2] = ( ( char_array_3[1] & 0x0f ) << 2 ) + ( ( char_array_3[2] & 0xc0 ) >> 6 );
            char_array_4[3] = char_array_3[2] & 0x3f;

            for( i = 0; ( i < 4 ) ; i++ ) {
                ret += base64_chars[char_array_4[i]];
            }
            i = 0;
        }
    }

    if( i ) {
        for( j = i; j < 3; j++ ) {
            char_array_3[j] = '\0';
        }

        char_array_4[0] = ( char_array_3[0] & 0xfc ) >> 2;
        char_array_4[1] = ( ( char_array_3[0] & 0x03 ) << 4 ) + ( ( char_array_3[1] & 0xf0 ) >> 4 );
        char_array_4[2] = ( ( char_array_3[1] & 0x0f ) << 2 ) + ( ( char_array_3[2] & 0xc0 ) >> 6 );

        for( j = 0; ( j < i + 1 ); j++ ) {
            ret += base64_chars[char_array_4[j]];
        }

        while( ( i++ < 3 ) ) {
            ret += '=';
        }

    }

    return ret;

}

std::string base64_decode( std::string const &encoded_string )
{
    int in_len = encoded_string.size();
    int i = 0;
    int j = 0;
    int in_ = 0;
    unsigned char char_array_4[4], char_array_3[3];
    std::string ret;

    while( in_len-- && ( encoded_string[in_] != '=' ) && is_base64( encoded_string[in_] ) ) {
        char_array_4[i++] = encoded_string[in_];
        in_++;
        if( i == 4 ) {
            for( i = 0; i < 4; i++ ) {
                char_array_4[i] = base64_chars.find( char_array_4[i] );
            }

            char_array_3[0] = ( char_array_4[0] << 2 ) + ( ( char_array_4[1] & 0x30 ) >> 4 );
            char_array_3[1] = ( ( char_array_4[1] & 0xf ) << 4 ) + ( ( char_array_4[2] & 0x3c ) >> 2 );
            char_array_3[2] = ( ( char_array_4[2] & 0x3 ) << 6 ) +   char_array_4[3];

            for( i = 0; ( i < 3 ); i++ ) {
                ret += char_array_3[i];
            }
            i = 0;
        }
    }

    if( i ) {
        for( j = 0; j < i; j++ ) {
            char_array_4[j] = base64_chars.find( char_array_4[j] );
        }

        char_array_3[0] = ( char_array_4[0] << 2 ) + ( ( char_array_4[1] & 0x30 ) >> 4 );
        char_array_3[1] = ( ( char_array_4[1] & 0xf ) << 4 ) + ( ( char_array_4[2] & 0x3c ) >> 2 );

        for( j = 0; ( j < i - 1 ); j++ ) {
            ret += char_array_3[j];
        }
    }

    return ret;
}

static inline void strip_trailing_nulls( std::wstring &str )
{
    while( !str.empty() && str.back() == '\0' ) {
        str.pop_back();
    }
}

static inline void strip_trailing_nulls( std::string &str )
{
    while( !str.empty() && str.back() == '\0' ) {
        str.pop_back();
    }
}

std::wstring utf8_to_wstr( const std::string &str )
{
#if defined(_WIN32) || defined(WINDOWS)
    int sz = MultiByteToWideChar( CP_UTF8, 0, str.c_str(), -1, NULL, 0 ) + 1;
    std::wstring wstr( sz, '\0' );
    MultiByteToWideChar( CP_UTF8, 0, str.c_str(), -1, &wstr[0], sz );
    strip_trailing_nulls( wstr );
    return wstr;
#else
    std::size_t sz = std::mbstowcs( NULL, str.c_str(), 0 ) + 1;
    std::wstring wstr( sz, '\0' );
    std::mbstowcs( &wstr[0], str.c_str(), sz );
    strip_trailing_nulls( wstr );
    return wstr;
#endif
}

std::string wstr_to_utf8( const std::wstring &wstr )
{
#if defined(_WIN32) || defined(WINDOWS)
    int sz = WideCharToMultiByte( CP_UTF8, 0, wstr.c_str(), -1, NULL, 0, NULL, NULL );
    std::string str( sz, '\0' );
    WideCharToMultiByte( CP_UTF8, 0, wstr.c_str(), -1, &str[0], sz, NULL, NULL );
    strip_trailing_nulls( str );
    return str;
#else
    std::size_t sz = std::wcstombs( NULL, wstr.c_str(), 0 ) + 1;
    std::string str( sz, '\0' );
    std::wcstombs( &str[0], wstr.c_str(), sz );
    strip_trailing_nulls( str );
    return str;
#endif
}

std::vector<std::string> str_split( const std::string &input, const std::string &regex )
{
    // passing -1 as the submatch index parameter performs splitting
    std::regex re( regex );
    std::sregex_token_iterator
    first{input.begin(), input.end(), re, -1},
          last;
    return {first, last};
}
