#include "catch.hpp"

#include "proto.h"
#include "util.h"
#include <string.h>
#include <limits.h>

struct TestUserInfo : public ProtoStructBase {
    public:
        void writeTo( ProtoOut &_os ) const {
            _os.write( uin, 1 );
            _os.write( nick, 2 );
            _os.write( birthday, 3 );
        }
        void readFrom( ProtoIn &_is ) {
            _is.read( uin );
            _is.read( nick );
            _is.read( birthday );
        }
    public:
        int uin;
        std::string nick;
        std::vector<char> birthday;
};
/*
  100120ff31010041010151ff7f61ff7e7200010000820001000192ffff7fffa2ffff7ffeb300-
  00000100000000c30000000100000001d3ffffffff7fffffffe3ffffffff7ffffffef40f3f8c-
  cccdf410c00ccccdf5113ff19999a0000000f512c0019999a0000000f6134041424344454647-
  48494a4b4c4d4e4f505145535455565758595a6162636465666768696a6b6c6d6e6f70717265-
  737475767778797a313233343536373839302ef8140002060531323334351605363738393006-
  05616263646516054142434445fd1500000a30313233343536373839f916000a020000ffff02-
  0001000002000100010200010002020001000302000100040200010005020001000602000100-
  070200010008fa171200bc614e260a48656c6c6f576f726c643d000004077d020dfb17f61803-
  454e44
 */
std::stringstream proto_test_data()
{
    TestUserInfo tui;
    tui.uin = 12345678;
    tui.nick = "HelloWorld";
    tui.birthday = {0x07, 0x7d, 0x02, 0x0d};
    std::stringstream res;
    ProtoOut jout( res );
    std::map<std::string, std::string> m_map;
    std::vector<char> sBuffer;
    char buff[512] = {0};
    jout.write( 1, 1 ); //char
    jout.write( -1, 2 ); //char
    jout.write( UCHAR_MAX + 1, 3 ); //short
    jout.write( UCHAR_MAX + 2, 4 ); //short
    jout.write( SCHAR_MIN - 1, 5 ); //short
    jout.write( SCHAR_MIN - 2, 6 ); //short
    jout.write( USHRT_MAX + 1, 7 ); //int
    jout.write( USHRT_MAX + 2, 8 ); //int
    jout.write( ( int )SHRT_MIN - 1, 9 ); //int
    jout.write( ( int )SHRT_MIN - 2, 10 ); //int
    jout.write( ( long )UINT_MAX + 1, 11 ); //long
    jout.write( ( long )UINT_MAX + 2, 12 ); //long
    jout.write( ( long )INT_MIN - 1, 13 ); //long
    jout.write( ( long )INT_MIN - 2, 14 ); //long
    jout.write( 1.1f, 15 ); //float
    jout.write( -2.2f, 16 ); //float
    jout.write( ( double )1.1f, 17 ); //double
    jout.write( ( double ) - 2.2f, 18 ); //double
    jout.write( std::string( "ABCDEFGHIJKLMNOPQESTUVWXYZabcdefghijklmnopqrestuvwxyz1234567890." ), 19 );
    m_map[std::string( "12345" )] = std::string( "67890" );
    m_map[std::string( "abcde" )] = std::string( "ABCDE" );
    jout.write( m_map, 20 );
    for( int i = 0; i < 10; i++ ) {
        sBuffer.push_back( i + '0' );
    }
    jout.write( sBuffer, 21 );
    std::vector<int> intBuffer;
    for( int i = USHRT_MAX; i < USHRT_MAX + 10; i++ ) {
        intBuffer.push_back( i );
    }
    jout.write( intBuffer, 22 );
    jout.write( tui, 23 );
    jout.write( std::string( "END" ), 24 );
    return res;
}

TEST_CASE( "ProtoOut r/w full test" )
{
    std::stringstream res = proto_test_data();
    std::string dat = res.str();
    std::string dathex;
    util::Buffer2String( dat, dathex );
    INFO( dathex );
    ProtoIn jin( res );
    int a;
    jin.read( a ); //1
    CHECK( a == 1 );
    jin.read( a ); //2
    CHECK( ( signed char )a == -1 );
    jin.read( a ); //3
    CHECK( a == ( UCHAR_MAX + 1 ) );
    jin.read( a ); //4
    jin.read( a ); //5
    CHECK( ( signed short )a == ( SCHAR_MIN - 1 ) );
    jin.read( a ); //6
    jin.read( a ); //7
    CHECK( jin.read( a ) == 8 );
    CHECK( a == ( USHRT_MAX + 2 ) );
    CHECK( jin.read( a ) == 9 );
    CHECK( a == ( SHRT_MIN - 1 ) );
    jin.read( a ); //10
    long b;
    jin.read( b ); //11
    CHECK( b == ( ( long ) UINT_MAX + 1 ) );
    jin.read( b ); //12
    jin.read( b ); //13
    CHECK( b == ( ( long )INT_MIN - 1 ) );
    jin.read( b ); //14
    float c;
    double d;
    jin.read( c ); //15
    //CHECK( c == 1.1f );
    jin.read( c ); //16
    CHECK( c == -2.2f );
    jin.read( d ); //17
    //CHECK( d == 1.1f );
    jin.read( d ); //18
    CHECK( d == -2.2f );
    std::string e;
    jin.read( e ); //19
    CHECK( e == "ABCDEFGHIJKLMNOPQESTUVWXYZabcdefghijklmnopqrestuvwxyz1234567890." );
    std::map<std::string, std::string> r_map;
    jin.read( r_map ); //20
    CHECK( r_map["12345"] == "67890" );
    CHECK( r_map["abcde"] == "ABCDE" );
    std::vector<char> r_sBuffer;
    jin.read( r_sBuffer ); //21
    CHECK( r_sBuffer[0] == '0' );
    CHECK( r_sBuffer[1] == '1' );
    std::vector<int> r_intBuffer;
    jin.read( r_intBuffer ); //22
    CHECK( r_intBuffer[0] == USHRT_MAX );
    CHECK( r_intBuffer[1] == USHRT_MAX + 1 );
    TestUserInfo otui;
    jin.read( otui ); //23
    CHECK( otui.uin == 12345678 );
    CHECK( otui.nick == "HelloWorld" );
    std::vector<char> corr = {0x07, 0x7d, 0x02, 0x0d};
    CHECK( otui.birthday == corr );
    jin.read( e ); //24
    CHECK( e == "END" );
}
TEST_CASE( "ProtoDisplay test" )
{
    std::stringstream res = proto_test_data();
    ProtoDisplay d( res );
    d.do_display();
    WARN( d.output.str() );
}
