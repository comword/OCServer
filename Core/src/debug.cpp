/**
 * @file debug.cpp
 * @auther Ge Tong
 * @brief This is an advanced debugging system with logging support.
 * Use DebugLog(debugLevel, debugClass)<<"Text";
 *
 */
#include "export.h"
#include "debug.h"
#include "mfilesystem.h"

#include <time.h>
#include <cassert>
#include <cstdlib>
#include <cstdarg>
#include <iosfwd>
#include <streambuf>
#include <sys/stat.h>
#include <exception>

#ifndef _MSC_VER
#include <sys/time.h>
#endif

#ifdef BACKTRACE
#include <execinfo.h>
#include <stdlib.h>
#endif

#if (defined(DEBUG) || defined(_DEBUG)) && !defined(NDEBUG)
static int debugLevel = DL_ALL;
static int debugClass = DC_ALL;
#else
static int debugLevel = D_ERROR;
static int debugClass = DC_ALL;
#endif
// Static defines
bool test_dirty = false;
bool debug_mode = false;

#define TRACE_SIZE 20

void *tracePtrs[TRACE_SIZE];

void limitDebugLevel( int level_bitmask )
{
    DebugLog( D_INFO, D_MAIN ) << "Set debug level to: " << level_bitmask;
    debugLevel = level_bitmask;
}

void limitDebugClass( int class_bitmask )
{
    DebugLog( D_INFO, D_MAIN ) << "Set debug class to: " << class_bitmask;
    debugClass = class_bitmask;
}

std::string currentTime()
{
    time_t t = time( 0 );
    char tmp[25];
    strftime( tmp, sizeof( tmp ), "[%Y-%m-%d %H:%M:%S]", localtime( &t ) );
    return std::string( tmp );
}

// Null OStream

struct NullBuf : public std::streambuf {
    NullBuf() {}
    int overflow( int c ) override {
        return c;
    }
};

static DebugFile debugFile;

DebugFile::DebugFile()
{
}

DebugFile::~DebugFile()
{
    if( file.is_open() ) {
        deinit();
    }
}
void DebugFile::deinit()
{
    file << "\n";
    file << currentTime() << " : Log shutdown.\n";
    file << "-----------------------------------------\n\n";
    file.close();
}

void DebugFile::init( std::string filename )
{
    this->filename = filename;
    const std::string oldfile = filename + ".prev";
    bool rename_failed = false;
    struct stat buffer;
    if( stat( filename.c_str(), &buffer ) == 0 ) {
        // Continue with the old log file if it's smaller than 1 MiB
        if( buffer.st_size >= 1024 * 1024 ) {
            rename_failed = !PATH_CLASS::rename_file( filename, oldfile );
        }
    }
    file.open( filename.c_str(), std::ios::out | std::ios::app );
    file << "\n-----------------------------------------\n";
    file << currentTime() << " : Starting log.";
    if( rename_failed ) {
        DebugLog( D_WARNING, D_MAIN ) << "Moving the previous log file to " + oldfile + " failed.\n" +
                                      "Check the file permissions. This program will continue to use the previous log file.";
    }
}

static NullBuf nullBuf;
static std::ostream nullStream( &nullBuf );

std::ostream &operator<<( std::ostream &out, DebugLevel lev )
{
    if( lev != DL_ALL ) {
        if( lev & D_DEBUG ) {
            out << "DEBUG";
        }
        if( lev & D_INFO ) {
            out << "INFO";
        }
        if( lev & D_WARNING ) {
            out << "WARNING";
        }
        if( lev & D_ERROR ) {
            out << "ERROR";
        }
    }
    return out;
}

std::ostream &operator<<( std::ostream &out, DebugClass cl )
{
    if( cl != DC_ALL ) {
        if( cl & D_MAIN ) {
            out << "MAIN";
        }
        if( cl & D_NETWORK ) {
            out << "NETWORK";
        }
        if( cl & D_CRYPT ) {
            out << "CRYPT";
        }
        if( cl & D_PROTO ) {
            out << "PROTO";
        }
        if( cl & D_PLG ) {
            out << "PLUGIN";
        }
        if( cl & D_JAVA ) {
            out << "JAVA";
        }
    }
    return out;
}


void setupDebug( std::string logpath )
{
    /*
    int level = 0;
    level |= D_DEBUG;
    level |= D_INFO;
    level |= D_WARNING;
    level |= D_ERROR;
    if( level != 0 ) {
        limitDebugLevel( level );
    }
    int cl = 0;
    cl |= D_MAIN;
    cl |= D_NETWORK;
    cl |= D_CRYPT;
    cl |= D_PROTO;
    cl |= D_PLG;
    if( cl != 0 ) {
        limitDebugClass( cl );
    }
    */
    debugFile.init( logpath );
}

void deinitDebug()
{
    debugFile.deinit();
}

struct time_info {
    int hours;
    int minutes;
    int seconds;
    int mseconds;

    template <typename Stream>
    friend Stream &operator<<( Stream &out, time_info const &t ) {
        using char_t = typename Stream::char_type;
        using base   = std::basic_ostream<char_t>;

        static_assert( std::is_base_of<base, Stream>::value, "" );

        out << t.hours << ':' << t.minutes << ':' << t.seconds << '.' << t.mseconds;

        return out;
    }
};

#ifdef _MSC_VER
time_info get_time() noexcept
{
    SYSTEMTIME time {};

    GetLocalTime( &time );

    return time_info { static_cast<int>( time.wHour ), static_cast<int>( time.wMinute ),
                       static_cast<int>( time.wSecond ), static_cast<int>( time.wMilliseconds )
                     };
}
#else
time_info get_time() noexcept
{
    timeval tv;
    gettimeofday( &tv, nullptr );

    auto const tt      = time_t {tv.tv_sec};
    auto const current = localtime( &tt );

    return time_info { current->tm_hour, current->tm_min, current->tm_sec,
                       static_cast<int>( tv.tv_usec / 1000.0 + 0.5 )
                     };
}
#endif

DebugFile &DebugLog( DebugLevel lev, DebugClass cl )
{
    // Error are always logged, because they are important.
    if( ( ( lev & debugLevel ) && ( cl & debugClass ) ) || lev & D_ERROR || cl & D_MAIN ) {
        if( lev == D_ERROR ) {
            debugFile.dir = D_FILE | D_CERR;
            debugFile << '\n';
            debugFile << currentTime() << " ";
            debugFile << "[" << lev << "] ";
            debugFile << "[" << cl << "]";
            debugFile << ": ";
#if !(defined _WIN32 || defined WINDOWS || defined __CYGWIN__)
            int count = backtrace( tracePtrs, TRACE_SIZE );
            char **funcNames = backtrace_symbols( tracePtrs, count );
            for( int i = 0; i < count; ++i ) {
                debugFile << "\n\t(" << funcNames[i] << "), ";
            }
            debugFile << "\n\t";
            free( funcNames );
#endif
            debugFile.dir |= D_THROW;
            return debugFile;
        } else {
            debugFile.dir &= ~D_THROW;
            debugFile.dir = D_FILE | D_COUT;
            debugFile << '\n';
            debugFile << currentTime() << " ";
            debugFile << "[" << lev << "] ";
            debugFile << "[" << cl << "]";
            debugFile << ": ";
            return debugFile;
        }
    }
    debugFile.dir = D_NONE;
    return debugFile;
}

DebugFile &DebugLog( std::string priority, std::string tag )
{
    debugFile.dir &= ~D_THROW;
    debugFile.dir = D_FILE | D_COUT;
    debugFile << '\n';
    debugFile << currentTime() << " ";
    debugFile << "[" << priority << "] ";
    debugFile << "[" << tag << "]";
    debugFile << ": ";
    return debugFile;
}

void DebugLog( int priority, std::string tag, std::string msg )
{
    debugFile.dir &= ~D_THROW;
    debugFile.dir = D_FILE | D_COUT;
    debugFile << '\n';
    debugFile << currentTime() << " ";
    debugFile << "[" << priority << "] ";
    debugFile << "[" << tag << "]";
    debugFile << ": ";
    debugFile << msg;
}
