#include "proto.h"
#include "util.h"
#include <fstream>
#include <istream>
#include <limits.h>

ProtoError::ProtoError( const std::string &msg )
    : std::runtime_error( msg )
{
}

void ProtoIn::error( std::string message, int offset )
{
    std::ostringstream err;
    err << message;
    if( !stream->good() ) {
        throw ProtoError( err.str() );
    }
    // also print surrounding few lines of context, if not too large
    err << "\n";
    int current_p = tell();
    std::string strbuf( 30, '\0' );
    if( offset >= 15 ) {
        seek( offset - 15 );
    }
    stream->read( &strbuf[0], 30 );
    char dispbuf[60] = {0};
    util::Buffer2String( ( char * )strbuf.c_str(), 30, dispbuf );
    err << dispbuf << std::endl;
    if( offset >= 15 ) {
        err << std::string( 30, ' ' ) << "^." << offset;
    } else {
        err << "^." << offset;
    }
    seek( current_p );
    throw ProtoError( err.str() );
}

bool ProtoIn::skipToTag( int tag )
{
    ProtoHead head;
    try {
        while( good() ) {
            //peakHead( head );
            head = readHead();
            if( head.type == ProtoSTRUCT_END ) {
                return false;
            }
            if( tag <= head.tag ) {
                if( tag == head.tag ) {
                    return true;
                }
                return false;
            }
            //seek( tell() + head.len );
            skipField( head.type );
        }
    }  catch( const ProtoError & ) {
        return false;
    }
    return false;
}

void ProtoIn::skipToStructEnd()
{
    ProtoHead head;
    do {
        head = this->readHead();
        skipField( head.type );
    } while( head.type != ProtoSTRUCT_END );
}

void ProtoIn::skipField( char type )
{
    int len;
    std::stringstream err;
    switch( type ) {
        case ProtoBYTE:
            seek( tell() + 1 );
            return;
        case ProtoSHORT:
            seek( tell() + 2 );
            return;
        case ProtoINT:
        case ProtoFLOAT:
            seek( tell() + 4 );
            return;
        case ProtoLONG:
        case ProtoDOUBLE:
            seek( tell() + 8 );
            return;
        case ProtoSTRING1:
            len = get_char();
            seek( tell() + len );
            return;
        case ProtoSTRING4:
            len = get_int();
            seek( tell() + len );
            return;
        case ProtoMAP:
            len = get_int();
            for( int i = 0; i < len * 2; i++ ) {
                skipField();
            }
            return;
        case ProtoLIST:
            len = get_int();
            for( int i = 0; i < len; i++ ) {
                skipField();
            }
            return;
        case ProtoSTRUCT_BEGIN:
            //skipToStructEnd();
            return;
        case ProtoSTRUCT_END:
        case ProtoZERO_TAG:
            return;
        case ProtoSIMPLE_LIST:
            ProtoHead head = this->readHead();
            if( head.type != ProtoBYTE ) {
                err << "Proto: skipField with invalid type, type value: " << head.type << " .";
                error( err.str(), tell() ); //invalid type
            }
            read( len );
            seek( tell() + len );
            return;
    }
    err << "Proto: unknown type when skipField, got type " << type << " .";
    error( err.str(), tell() ); //invalid type
}

void ProtoIn::skipField()
{
    ProtoHead head = this->readHead();
    skipField( head.type );
}

void ProtoIn::peakHead( ProtoHead &head )
{
    head = readHead();
    seek( tell() - head.len );
}

struct ProtoIn::ProtoHead ProtoIn::readHead()
{
    char b = 0;
    stream->get( b );
    ProtoHead res;
    res.type = b & 0xF;
    res.tag = 0;
    res.tag = ( b & 0xF0 ) >> 4;
    if( res.tag == 15 ) {
    	stream->get( b );
		res.tag = b;
		res.len = 2;
		return res;
    } else {
    	res.len = 1;
    	return res;
    }
}

int ProtoIn::tell()
{
    return stream->tellg();
}
char ProtoIn::peek()
{
    return ( char )stream->peek();
}
bool ProtoIn::good()
{
    return stream->good();
}

void ProtoIn::seek( int pos )
{
    stream->clear();
    stream->seekg( pos );
}

unsigned char ProtoIn::get_char()
{
    char a;
    stream->get( a );
    return a;
}

unsigned int ProtoIn::get_int()
{
    char a;
    unsigned int res = 0;
    for( int i = 0; i < 4; i++ ) {
        stream->get( a );
        res = ( res << 8 ) | ( unsigned char )a;
    }
    return res;
}

unsigned short ProtoIn::get_short()
{
    char a, b;
    stream->get( a ); //high
    stream->get( b );
    return ( a << 8 ) | ( unsigned char )b;
}

unsigned long ProtoIn::get_long()
{
    char a;
    unsigned long res = 0;
    for( int i = 0; i < 8; i++ ) {
        stream->get( a );
        res = ( res << 8 ) | ( unsigned char )a;
    }
    return res;
}

float ProtoIn::get_float()
{
    float res = 0.0f;
    char a;
    char *p = ( char * )&res;
    for( int i = 3; i >= 0; i-- ) {
        stream->get( a );
        p[i] = a;
    }
    return res;
}

double ProtoIn::get_double()
{
    double res = 0.0f;
    char a;
    char *p = ( char * )&res;
    for( int i = 7; i >= 0; i-- ) {
        stream->get( a );
        p[i] = a;
    }
    return res;
}

std::string ProtoIn::get_string( int length )
{
    std::string str( length, '\0' );
    stream->read( &str[0], length );
    return str;
}

char ProtoIn::read( bool &b )
{
    char a;
    read( a );
    if( a == 0 ) {
        b = false;
    } else {
        b = true;
    }
    return true;
}

char ProtoIn::read( char &c )
{
    ProtoHead head = this->readHead();
    if( head.type == ProtoZERO_TAG ) {
        c = 0;
    } else if( head.type == ProtoBYTE ) {
        c = get_char();
    } else {
        std::stringstream err;
        err << "Proto: unknown type when read char, got type " << head.type << " .";
        error( err.str(), tell() ); //invalid type
    }
    return head.tag;
}

char ProtoIn::read( short unsigned int &s )
{
    ProtoHead head = this->readHead();
    if( head.type == ProtoZERO_TAG ) {
        s = 0;
    } else if( head.type == ProtoBYTE ) {
        s = get_char();
    } else if( head.type == ProtoSHORT ) {
        s = get_short();
    } else {
        std::stringstream err;
        err << "Proto: unknown type when read short, got type " << head.type << " .";
        error( err.str(), tell() ); //invalid type
    }
    return head.tag;
}

char ProtoIn::read( short int &s )
{
    ProtoHead head = this->readHead();
    if( head.type == ProtoZERO_TAG ) {
        s = 0;
    } else if( head.type == ProtoBYTE ) {
        s = get_char();
    } else if( head.type == ProtoSHORT ) {
        s = get_short();
    } else {
        std::stringstream err;
        err << "Proto: unknown type when read short, got type " << head.type << " .";
        error( err.str(), tell() ); //invalid type
    }
    return head.tag;
}

char ProtoIn::read( int &i )
{
    ProtoHead head = this->readHead();
    if( head.type == ProtoZERO_TAG ) {
        i = 0;
    } else if( head.type == ProtoBYTE ) {
        i = get_char();
    } else if( head.type == ProtoSHORT ) {
        i = get_short();
    } else if( head.type == ProtoINT ) {
        i = get_int();
    } else {
        std::stringstream err;
        err << "Proto: unknown type when read int, got type " << head.type << " .";
        error( err.str(), tell() ); //invalid type
    }
    return head.tag;
}

char ProtoIn::read( unsigned int &u )
{
    ProtoHead head = this->readHead();
    if( head.type == ProtoZERO_TAG ) {
        u = 0;
    } else if( head.type == ProtoBYTE ) {
        u = get_char();
    } else if( head.type == ProtoSHORT ) {
        u = get_short();
    } else if( head.type == ProtoINT ) {
        u = get_int();
    } else {
        std::stringstream err;
        err << "Proto: unknown type when read int, got type " << head.type << " .";
        error( err.str(), tell() ); //invalid type
    }
    return head.tag;
}

char ProtoIn::read( long &l )
{
    ProtoHead head = this->readHead();
    if( head.type == ProtoZERO_TAG ) {
        l = 0;
    } else if( head.type == ProtoBYTE ) {
        l = get_char();
    } else if( head.type == ProtoSHORT ) {
        l = get_short();
    } else if( head.type == ProtoINT ) {
        l = get_int();
    } else if( head.type == ProtoLONG ) {
        l = get_long();
    } else {
        std::stringstream err;
        err << "Proto: unknown type when read long, got type " << head.type << ".";
        error( err.str(), tell() ); //invalid type
    }
    return head.tag;
}

//char ProtoIn::read( unsigned long &ul )
//{
//    ProtoHead head = this->readHead();
//    if( head.type == ProtoZERO_TAG ) {
//        ul = 0;
//    } else if( head.type == ProtoBYTE ) {
//        ul = get_char();
//    } else if( head.type == ProtoSHORT ) {
//        ul = get_short();
//    } else if( head.type == ProtoINT ) {
//        ul = get_int();
//    } else if( head.type == ProtoLONG ) {
//        ul = get_long();
//    } else {
//        std::stringstream err;
//        err << "Proto: unknown type when read long, got type " << head.type << " .";
//        error( err.str(), tell() ); //invalid type
//    }
//    return head.tag;
//}

char ProtoIn::read( float &f )
{
    ProtoHead head = this->readHead();
    if( head.type == ProtoZERO_TAG ) {
        f = 0.0f;
    } else if( head.type == ProtoFLOAT ) {
        f = get_float();
    } else {
        std::stringstream err;
        err << "Proto: unknown type when read float, got type " << head.type << " .";
        error( err.str(), tell() ); //invalid type
    }
    return head.tag;
}

char ProtoIn::read( double &d )
{
    ProtoHead head = this->readHead();
    if( head.type == ProtoZERO_TAG ) {
        d = 0.0f;
    } else if( head.type == ProtoFLOAT ) {
        d = get_float();
    } else if( head.type == ProtoDOUBLE ) {
        d = get_double();
    } else {
        std::stringstream err;
        err << "Proto: unknown type when read double, got type " << head.type << " .";
        error( err.str(), tell() ); //invalid type
    }
    return head.tag;
}

char ProtoIn::read( std::string &s )
{
    ProtoHead head = this->readHead();
    int len = 0;
    if( head.type == ProtoZERO_TAG ) {
        s = "";
        return head.tag;
    } else if( head.type == ProtoSTRING1 ) {
        char a;
        stream->get( a );
        len = a;
    } else if( head.type == ProtoSTRING4 ) {
        char *p = ( char * )&len;
        for( int i = 3; i >= 0; i-- ) {
            stream->get( p[i] );
        }
    } else {
        std::stringstream tmp;
        if( head.type == ProtoBYTE ) {
            tmp << get_char();
        } else if( head.type == ProtoSHORT ) {
            tmp << get_short();
        } else if( head.type == ProtoINT ) {
            tmp << get_int();
        } else if( head.type == ProtoLONG ) {
            tmp << get_long();
        } else if( head.type == ProtoFLOAT ) {
            tmp << get_float();
        } else if( head.type == ProtoDOUBLE ) {
            tmp << get_double();
        }
        s = tmp.str();
        return head.tag;
    }
    if( len > Proto_MAX_STRING_LENGTH ) {
        std::stringstream err;
        err << "Proto: String too long:" << len << " .";
        error( err.str(), tell() );
    }
    s = get_string( len );
    return head.tag;
}

char ProtoIn::read( ProtoMap &m )
{
    ProtoHead head = this->readHead();
    if( head.type != ProtoMAP ) {
        std::stringstream err;
        err << "Proto: unknown type when read string map, got type " << head.type << " .";
        error( err.str(), tell() ); //invalid type
    }
    int len;
    read( len );
    if( len < 0 ) {
        std::stringstream err;
        err << "Proto: size invalid when read string map, got size " << len << " .";
        error( err.str(), tell() ); //invalid type
    }
    for( int i = 0; i < len; i++ ) {
        std::string k, v;
        if( read( k ) == 0 )
            if( read( v ) == 1 ) {
                m[std::move( k )] = std::move( v );
            }
    }
    return head.tag;
}

char ProtoIn::read( std::vector<char> &bl )
{
    ProtoHead head = this->readHead();
    if( head.type == ProtoSIMPLE_LIST ) {
        ProtoHead ihead = this->readHead();
        if( ihead.type != ProtoBYTE ) {
            throw ProtoError( "Proto: type mismatch, tag: " + std::to_string( head.tag ) + " type: " +
                              std::to_string( head.type ) + "," + std::to_string( ihead.type ) );
        }
    }
    int len;
    read( len );
    bl.clear();
    char a;
    for( int i = 0; i < len; i++ ) {
        stream->get( a );
        bl.push_back( a );
    }
    return head.tag;
}

void ProtoOut::writeHead( char type, char tag )
{
    if( tag < 15 ) {
        stream->put( ( tag << 4 ) | type );
    } else {
        stream->put( type | 240 );
        stream->put( tag );
    }
}

void ProtoOut::write( bool b, char tag )
{
    write( ( char )( b ? 1 : 0 ), tag );
}

void ProtoOut::write( char c, char tag )
{
    if( c == 0 ) {
        writeHead( ProtoZERO_TAG, tag );
        return;
    }
    writeHead( ProtoBYTE, tag );
    stream->put( c );
}

void ProtoOut::write( short unsigned int s, char tag )
{
    //    if( s > CHAR_MAX ) {
    //        writeHead( ProtoSHORT, tag );
    //        stream->put( ( s & 0xff00 ) >> 8 ); //high
    //        stream->put( s & 0xff );
    //        return;
    //    }
    //    //stream->get( a ); //high
    //    //stream->get( b );
    //    //return ( a << 8 ) | b;
    //    write( ( char )s, tag );
    write( ( int )s, tag );
}

void ProtoOut::write( short int s, char tag )
{
    if( s < SCHAR_MIN || s > SCHAR_MAX ) {
        writeHead( ProtoSHORT, tag );
        stream->put( ( s & 0xff00 ) >> 8 ); //high
        stream->put( s & 0xff );
        return;
    }
    write( ( char )s, tag );
}

void ProtoOut::write( int i, char tag )
{
    if( i < SHRT_MIN || i > SHRT_MAX ) {
        writeHead( ProtoINT, tag );
        char *p = ( char * )&i;
        for( int j = 3; j >= 0; j-- ) {
            stream->put( p[j] );
        }
        return;
    }
    write( ( short )i, tag );
}

void ProtoOut::write( unsigned int u, char tag )
{
    //    if( u > USHRT_MAX ) {
    //        writeHead( ProtoINT, tag );
    //        char *p = ( char * )&u;
    //        for( int i = 3; i >= 0; i-- ) {
    //            stream->put( p[i] );
    //        }
    //        return;
    //    }
    //    write( ( short unsigned int )u, tag );
    write( ( long )u, tag );
}

void ProtoOut::write( long l, char tag )
{
    if( l < INT_MIN || l > INT_MAX ) {
        writeHead( ProtoLONG, tag );
        char *p = ( char * )&l;
        for( int i = 7; i >= 0; i-- ) {
            stream->put( p[i] );
        }
        return;
    }
    write( ( int )l, tag );
}

//void ProtoOut::write( unsigned long ul, char tag )
//{
//    if( ul > UINT_MAX ) {
//        writeHead( ProtoLONG, tag );
//        char *p = ( char * )&ul;
//        for( int i = 7; i >= 0; i-- ) {
//            stream->put( p[i] );
//        }
//        return;
//    }
//    write( ( unsigned int )ul, tag );
//}

void ProtoOut::write( float f, char tag )
{
    writeHead( ProtoFLOAT, tag );
    char *p = ( char * )&f;
    for( int i = 3; i >= 0; i-- ) {
        stream->put( p[i] );
    }
}

void ProtoOut::write( double d, char tag )
{
    writeHead( ProtoDOUBLE, tag );
    char *p = ( char * )&d;
    for( int i = 7; i >= 0; i-- ) {
        stream->put( p[i] );
    }
}

void ProtoOut::write( std::string s, char tag )
{
    int len = s.length();
    if( len > Proto_MAX_STRING_LENGTH ) {
        throw ProtoError( "String length greater than Proto_MAX_STRING_LENGTH, size " + std::to_string(
                              len ) );
    }
    if( len > 255 ) {
        writeHead( ProtoSTRING4, tag );
        char *p = ( char * )&len;
        for( int i = 3; i >= 0; i-- ) {
            stream->put( p[i] );
        }
        stream->write( s.c_str(), len );
        return;
    }
    writeHead( ProtoSTRING1, tag );
    stream->put( ( char )len );
    stream->write( s.c_str(), len );
}

void ProtoOut::write( const char *s, char tag )
{
    write( std::string( s ), tag );
}

void ProtoOut::write( std::vector<char> v, int tag )
{
    writeHead( ProtoSIMPLE_LIST, tag );
    writeHead( ProtoBYTE, 0 );
    write( ( char )v.size(), 0 );
    for( char a : v ) {
        stream->put( a );
    }
}

void ProtoDisplay::do_display()
{
    while( input.good() ) {
        display_Proto();
    }
}

std::string ProtoDisplay::getNamebyType( char type )
{
    switch( type ) {
        case ProtoBYTE:
            return "BYTE";
        case ProtoDOUBLE:
            return "DOUBLE";
        case ProtoFLOAT:
            return "FLOAT";
        case ProtoINT:
            return "INT";
        case ProtoLIST:
            return "LIST";
        case ProtoLONG:
            return "LONG";
        case ProtoMAP:
            return "MAP";
        case ProtoSHORT:
            return "SHORT";
        case ProtoSIMPLE_LIST:
            return "SIMPLE_LIST";
        case ProtoSTRING1:
            return "STRING1";
        case ProtoSTRING4:
            return "STRING4";
        case ProtoSTRUCT_BEGIN:
            return "STRUCT_BEGIN";
        case ProtoSTRUCT_END:
            return "STRUCT_END";
        case ProtoZERO_TAG:
            return "ZERO_TAG";
    }
    return "UNKNOWN";
}

void ProtoDisplay::display_Proto()
{
    ProtoIn::ProtoHead head;
    input.peakHead( head );
    if( !input.good() ) {
        return;
    }
    long a;
    float b;
    double c;
    std::string d;
    std::vector<std::string> e;
    ProtoMap f;
    switch( head.type ) {
        case ProtoZERO_TAG:
        case ProtoBYTE:
        case ProtoSHORT:
        case ProtoINT:
        case ProtoLONG:
            input.read( a );
            display( a, std::to_string( head.tag ) + "," + getNamebyType( head.type ) );
            break;
        case ProtoFLOAT:
            input.read( b );
            display( b, std::to_string( head.tag ) + "," + getNamebyType( head.type ) );
            break;
        case ProtoDOUBLE:
            input.read( c );
            display( c, std::to_string( head.tag ) + "," + getNamebyType( head.type ) );
            break;
        case ProtoSTRING1:
        case ProtoSTRING4:
            input.read( d );
            display( d, std::to_string( head.tag ) + "," + getNamebyType( head.type ) );
            break;
        case ProtoLIST:
            input.read( e );
            display( e, std::to_string( head.tag ) + "," + getNamebyType( head.type ) );
            break;
        case ProtoMAP:
            input.read( f );
            display( f, std::to_string( head.tag ) + "," + getNamebyType( head.type ) );
            break;
        case ProtoSTRUCT_BEGIN:
            input.skipToStructEnd();
            output << std::to_string( head.tag ) << ",[STRUCT]" << std::endl;
            break;
        case ProtoSTRUCT_END:
            break;
        case ProtoSIMPLE_LIST:
            std::vector<char> v;
            input.read( v );
            display( v, std::to_string( head.tag ) + "," + getNamebyType( head.type ) );
            break;
    }
}

void ProtoDisplay::display( bool b, std::string fieldName )
{
    display( std::string( b ? "True" : "False" ), fieldName );
}

void ProtoDisplay::display( char c, std::string fieldName )
{
    char buf[4] = {0};
    util::Buffer2String( &c, 1, buf );
    display( std::string( "\\x" ) + buf, fieldName );
}

void ProtoDisplay::display( std::vector<char> b, std::string fieldName )
{
    bool o_isSimple = isSimple;
    bool o_bSep = bSep;
    isSimple = true;
    bSep = false;
    if( o_isSimple ) {
        for( char a : b ) {
            display( a, "" );
        }
        if( o_bSep ) {
            output << '|';
        }
    } else {
        wrap( fieldName );
        for( char a : b ) {
            display( a, "" );
        }
        output << std::endl;
    }
    isSimple = o_isSimple;
    bSep = o_bSep;
}

void ProtoDisplay::display( std::string s, std::string fieldName )
{
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

void ProtoDisplay::wrap( std::string fieldName )
{
    for( int i = 0; i < _level; i++ ) {
        output << '\t';
    }
    if( fieldName != "" ) {
        output << fieldName << ": ";
    }
}
