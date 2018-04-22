#pragma once
#include "export.h"

#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
/**
 * If you add an entry, add an entry in that function:
 * std::ostream &operator<<(std::ostream &out, DebugLevel lev)
 */
enum DebugLevel {
    D_DEBUG         = 1,
    D_INFO          = 1 << 2,
    D_WARNING       = 1 << 3,
    D_ERROR         = 1 << 4,
    DL_ALL = ( 1 << 5 ) - 1
};
/**
 * Debugging areas can be enabled for each of those areas separately.
 * If you add an entry, add an entry in that function:
 * std::ostream &operator<<(std::ostream &out, DebugClass cl)
 */
enum DebugClass {
    D_MAIN    = 1,
    D_NETWORK = 1 << 1,
    D_CRYPT   = 1 << 2,
    D_PROTO   = 1 << 3,
    D_PLG     = 1 << 4,
    D_JAVA    = 1 << 5,
    DC_ALL    = ( 1 << 6 ) - 1
};
/** Initializes the debugging system, called exactly once from main() */
extern "C" DLL_PUBLIC void setupDebug( std::string );
/** Opposite of setupDebug, shuts the debugging system down. */
extern "C" DLL_PUBLIC void deinitDebug();

extern "C" DLL_PUBLIC void limitDebugLevel( int );

extern "C" DLL_PUBLIC void limitDebugClass( int );

extern "C" DLL_PUBLIC void DebugLog( int priority, std::string tag, std::string msg );

enum LogDirection {
    D_NONE = 0,
    D_FILE = 1,
    D_COUT = 1 << 1,
    D_CERR = 1 << 2,
    D_THROW = 1 << 3,
    D_ALL = ( 1 << 5 ) - 1
};
// DebugFile OStream Wrapper
struct DebugFile {
    DebugFile();
    ~DebugFile();
    void init( std::string filename );
    void deinit();
    std::ofstream file;
    std::string filename;
    int dir = D_NONE;
    template<typename T>
    void log( const T &v ) {
        if( dir & D_FILE ) {
            file << v;
        }
        if( dir & D_CERR ) {
            std::cerr << v;
        }
        if( dir & D_COUT ) {
            std::cout << v;
        }
        if( dir & D_THROW ) {
            std::stringstream err;
            err << v;
            file.flush();
            std::cout.flush();
            throw std::runtime_error( err.str() );
        }
        file.flush();
        std::cout.flush();
    }
    template<typename T>
    DebugFile &operator<< ( const T &v ) {
        log( v );
        return *this;
    }
    DebugFile &operator<< ( std::ostream & ( *f )( const std::ostream & ) ) {
        if( dir & D_FILE ) {
            f( file );
        }
        if( dir & D_CERR ) {
            f( std::cerr );
        }
        if( dir & D_COUT ) {
            f( std::cout );
        }
        if( dir & D_THROW ) {
            std::stringstream err;
            err << f;
            file.flush();
            std::cout.flush();
            throw std::runtime_error( err.str() );
        }
        return *this;
    }
};

//std::ofstream &currentLevel();
DLL_PUBLIC DebugFile &DebugLog( DebugLevel, DebugClass );
DLL_PUBLIC DebugFile &DebugLog( std::string, std::string );
// OStream operators
template<typename C, typename A>
std::ostream &operator<<( std::ostream &out, const std::vector<C, A> &elm )
{
    bool first = true;
    for( typename std::vector<C>::const_iterator
         it = elm.begin(),
         end = elm.end();
         it != end; ++it ) {
        if( first ) {
            first = false;
        } else {
            out << ",";
        }
        out << *it;
    }
    return out;
}
