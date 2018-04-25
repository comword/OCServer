#pragma once
#ifndef Proto_H
#define Proto_H

#include <sstream>
#include <string>
#include <vector>
#include <map>
#include "export.h"

#define ProtoBYTE 0
#define ProtoDOUBLE 5
#define ProtoFLOAT 4
#define ProtoINT 2
#define Proto_MAX_STRING_LENGTH 104857600
#define ProtoLIST 9
#define ProtoLONG 3
#define ProtoMAP 8
#define ProtoSHORT 1
#define ProtoSIMPLE_LIST 13
#define ProtoSTRING1 6
#define ProtoSTRING4 7
#define ProtoSTRUCT_BEGIN 10
#define ProtoSTRUCT_END 11
#define ProtoZERO_TAG 12

using ProtoMap = std::map<std::string, std::string>;

template <typename T>
using is_vector = std::is_same<T, std::vector< typename T::value_type,
      typename T::allocator_type > >;
template <typename T>
using is_map = std::is_same<T, std::map< typename T::key_type,
      typename T::mapped_type> >;

class ProtoIn;
class ProtoOut;
class ProtoDisplay;
struct DLL_PUBLIC ProtoStructBase {
    protected:
        ProtoStructBase() {}
        ~ProtoStructBase() {}
};

class DLL_PUBLIC ProtoError : public std::runtime_error
{
    public:
        ProtoError( const std::string &msg );
        const char *c_str() const noexcept {
            return what();
        }
};

class DLL_PUBLIC ProtoIn
{
    public:
        struct ProtoHead {
            short type;
            char tag;
            char len;
            void clear() {
                type = 0;
                tag = 0;
                len = 0;
            }
        };
    private:
        std::istream *stream;
    public:
        ProtoIn( std::istream &s ) : stream( &s ) {}
        ProtoIn( std::istream &s, int position ) : stream( &s ) {
            seek( position );
        }
        virtual ~ProtoIn() {}
        std::istream &getStream();
        struct ProtoHead readHead();
        void peakHead( ProtoHead &head );
        bool skipToTag( int tag );
        unsigned char get_char();
        unsigned int get_int();
        unsigned short get_short();
        unsigned long get_long();
        float get_float();
        double get_double();
        std::string get_string( int length );
        int tell(); // get current stream position
        void seek( int pos ); // seek to specified stream position
        char peek(); // what's the next char gonna be?
        bool good();
        void skipToStructEnd();
        char read( bool &b );
        char read( char &c );
        char read( short unsigned int &s );
        char read( short int &s );
        char read( int &i );
        char read( unsigned int &u );
        char read( long &l );
        //char read( unsigned long &ul );
        char read( float &f );
        char read( double &d );
        char read( std::string &s );
        char read( ProtoMap &m );
        char read( std::vector<char> &bl );

        template<typename T, typename std::enable_if<std::is_convertible<T *, ProtoStructBase *>::value>::type * = nullptr>
        void read( T &v ) {
            ProtoHead head = this->readHead();
            if( head.type != ProtoSTRUCT_BEGIN ) {
                char s[100];
                snprintf( s, sizeof( s ), "Proto: read 'struct' type mismatch, get type: %d.", head.type );
                error( s, tell() ); //invalid type
            }
            v.readFrom( *this );
            skipToStructEnd();
        }

        template < typename T, typename std::enable_if <
                       !std::is_same<char, typename T::value_type>::value >::type * = nullptr
                   >
        auto read( T &v ) -> decltype( v.front(), true ) {
            ProtoHead head = this->readHead();
            bool res = true;
            if( head.type != ProtoLIST ) {
                res = false;
                error( "Proto: unknown type when read array struct, got type " + std::to_string( head.type ),
                       tell() ); //invalid type
            }
            v.clear();
            int len;
            read( len );
            if( len < 0 ) {
                error( "Proto: size invalid when read array, got size " + std::to_string( len ),
                       tell() ); //invalid type
            }
            int i;
            for( i = 0; i < len; i++ ) {
                typename T::value_type element;
                read( element );
                v.push_back( std::move( element ) );
            }
            return res;
        }

        template < typename T, typename std::enable_if <
                       !std::is_same<typename T::key_type, typename T::value_type>::value >::type * = nullptr
                   >
        char read( T &m ) {
            ProtoHead head = this->readHead();
            if( head.type != ProtoMAP ) {
                error( "Proto: unknown type when read map, got type " + std::to_string( head.type ),
                       tell() ); //invalid type
            }
            m.clear();
            int len;
            read( len );
            for( int i = 0; i < len; i++ ) {
                typename T::key_type k;
                typename T::value_type v;
                if( read( k ) == 0 )
                    if( read( v ) == 1 ) {
                        m[std::move( k )] = std::move( v );
                    }
            }
            return head.tag;
        }

    private:
        void error( std::string message, int offset = 0 );
        void skipField( char type );
        void skipField();
};

class DLL_PUBLIC ProtoOut
{
    private:
        std::ostream *stream;
    public:
        ProtoOut( std::ostream &s ) : stream( &s ) {}
        virtual ~ProtoOut() {}
        void writeHead( char type, char tag );
        void write( bool b, char tag );
        void write( char c, char tag );
        void write( short unsigned int s, char tag );
        void write( short int s, char tag );
        void write( int i, char tag );
        void write( unsigned int u, char tag );
        void write( long l, char tag );
        //void write( unsigned long ul, char tag );
        void write( float f, char tag );
        void write( double d, char tag );
        void write( std::string s, char tag );
        void write( const char *s, char tag );
        template <typename T, typename std::enable_if<
                      is_map<T>::value >::type * = nullptr
                  >
        void write( T const &m, int tag ) {
            writeHead( ProtoMAP, tag );
            write( ( int )( m.size() ), 0 );
            if( m.size() != 0 ) {
                for( std::pair<typename T::key_type, typename T::mapped_type> const &it : m ) {
                    write( it.first, 0 );
                    write( it.second, 1 );
                }
            }
        }
        //not include char type
        template < typename T, typename std::enable_if < is_vector<T>::value &&
                   !std::is_same<char, typename T::value_type>::value >::type * = nullptr
                   >
        void write( T const &v, int tag ) {
            writeHead( ProtoLIST, tag );
            write( ( int )( v.size() ), 0 );
            for( auto it : v ) {
                write( it, 0 );
            }
        }

        void write( std::vector<char> v, int tag );

        template<typename T>
        void write( const T &v, uint8_t tag,
                    typename std::enable_if<std::is_convertible<T *, ProtoStructBase *>::value>::type * = nullptr ) {
            writeHead( ProtoSTRUCT_BEGIN, tag );
            v.writeTo( *this );
            writeHead( ProtoSTRUCT_END, tag );
        }
};

class DLL_PUBLIC ProtoDisplay
{
    private:
        ProtoIn input;
    public:
        bool isSimple = false;
        bool bSep = true;
        int _level = 0;
        std::stringstream output;
        ProtoDisplay( std::istream &s ) : input( s ) {}
        virtual ~ProtoDisplay() {}
        void do_display();
        bool display_Proto();
    private:
        void wrap( std::string fieldName );
        std::string getNamebyType( char tpye );
        void display( bool b, std::string fieldName );
        void display( char c, std::string fieldName );
        void display( std::vector<char> b, std::string fieldName );
        void display( std::string s, std::string fieldName );
        template < typename T, typename std::enable_if < std::is_fundamental<T>::value >::type * = nullptr >
        void display( T s, std::string fieldName ) {
            if( isSimple ) {
                output << s;
                if( bSep ) {
                    output << '|';
                }
            } else {
                wrap( fieldName );
                output << s << std::endl;
            }
        }
        template < typename T, typename std::enable_if < is_vector<T>::value &&
                   !std::is_same<char, typename T::value_type>::value >::type * = nullptr
                   >
        void display( T s, std::string fieldName ) {
            std::stringstream res;
            if( s.size() == 0 ) {
                res << "[]";
                display( res.str(), fieldName );
                return;
            }
            bool b = false;
            res << "[";
            for( unsigned int i = 0; i < s.size(); i++ ) {
                if( b ) {
                    res << ",";
                } else {
                    b = true;
                }
                res << s[i];
            }
            res << "]";
            display( res.str(), fieldName );
            return;
        }
        template <typename T, typename std::enable_if<
                      is_map<T>::value >::type * = nullptr
                  >
        void display( T m, std::string fieldName ) {
            if( m.size() == 0 ) {
                display( "{}", fieldName );
                return;
            }
            bool o_isSimple = isSimple;
            bool o_bSep = bSep;
            isSimple = true;
            bSep = false;
            bool b = false;
            if( o_isSimple ) {
                output << "{";
                for( auto const &it : m ) {
                    if( b ) {
                        output << ",";
                    } else {
                        b = true;
                    }
                    display( it.first, "" );
                    output << ":";
                    display( it.second, "" );
                }
                output << "}";
                if( o_bSep ) {
                    output << '|';
                }
            } else {
                wrap( fieldName );
                output << "{";
                for( auto const &it : m ) {
                    if( b ) {
                        output << ",";
                    } else {
                        b = true;
                    }
                    display( it.first, "" );
                    output << ":";
                    display( it.second, "" );
                }
                output << "}" << std::endl;
            }
            isSimple = o_isSimple;
            bSep = o_bSep;
            return;
        }
        void displayStruct( std::string fieldName );
};

#endif
