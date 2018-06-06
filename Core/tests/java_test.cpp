#include "catch.hpp"

#include "mfilesystem.h"
#include "jImpl.h"
#include "debug.h"
#include "proto.h"
#include "cryptoutils.h"
#include "charutils.h"
#include "util.h"

#include <sstream>
#include <string>
#include <algorithm>

TEST_CASE( "PingJava" )
{
    std::string res = jc->pingJava();
    DebugLog( D_INFO, D_JAVA ) << res;
}

std::stringstream login_test_data()
{
    std::stringstream ss;
    ProtoOut jout( ss );
    jout.write( 0, 0 ); //unknown UID
    jout.write( "LoginSvc/Login", 1 ); //serviceCmd
    jout.write( 100, 2 ); //packSeq
    //no extra attribute
    jout.write( 0, 4 ); //PROTO_APP_FIRST_TAG (login by UID)
    jout.write( 1, 5 ); //login uid;
    std::string pwdMD5 = crypt_util::MD5_string( "12345qwert" );
    std::string pwdMD5hex;
    util::Buffer2String(pwdMD5, pwdMD5hex);
    std::transform(pwdMD5hex.begin(), pwdMD5hex.end(), pwdMD5hex.begin(), ::toupper);
    jout.write(pwdMD5hex, 6 ); //pwdMD5
    return ss;
}

TEST_CASE( "DummyLogin" )
{
    std::stringstream ss = login_test_data();
    ProtoIn jin( ss );
    std::string cmd;
    jin.skipToTag( 1 );
    jin.read( cmd ); //serviceCmd
    std::string cpath = str_split( cmd, "/" )[0];
    CHECK( cpath.length() != 0 );
    jobject cls = jc->getService( cpath );
    CHECK( cls != nullptr );
    if( cls != nullptr ) {
        std::string fromsrv, hex;
        bool res = jc->execTransact( cls, ss.str(), fromsrv, 0 );
        WARN( res );
        util::Buffer2String(fromsrv, hex);
        WARN( hex );
    }
}



