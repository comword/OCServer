#define CATCH_CONFIG_RUNNER
#include "catch.hpp"
#include "mfilesystem.h"
#include "debug.h"
#include "config.h"
#include <stdio.h>
#include <vector>

int main( int argc, const char *argv[] )
{
    Catch::Session session;

    std::vector<const char *> arg_vec( argv, argv + argc );
    int result = session.applyCommandLine( arg_vec.size(), &arg_vec[0] );
    if( result != 0 || session.configData().showHelp ) {
        printf( "OCServer specific options:\n" );
        return result;
    }
    PATH_CLASS::init_user_dir( "" );
    PATH_CLASS::update_datadir();
    //setupDebug( "test.log" );
    limitDebugLevel( D_ALL );
    limitDebugClass( DC_ALL );
    DebugLog( D_INFO, D_MAIN ) << "Version: " << VERSION;
    conf = new Configure( PATH_CLASS::FILENAMES["userdir"] + "config.json" );
    result = session.run();
    deinitDebug();
    return result;
}
