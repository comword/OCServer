#pragma once
#ifndef CHARSETUTILS_H
#define CHARSETUTILS_H
#include <stdint.h>
#include <string>
#include <vector>

#include "export.h"

DLL_PUBLIC std::string base64_encode( std::string str );
DLL_PUBLIC std::string base64_encode( unsigned char const *, unsigned int len );
DLL_PUBLIC std::string base64_decode( std::string const &s );

DLL_PUBLIC std::wstring utf8_to_wstr( const std::string &str );
DLL_PUBLIC std::string wstr_to_utf8( const std::wstring &wstr );

DLL_PUBLIC std::vector<std::string> str_split( const std::string &input, const std::string &regex );

#endif
