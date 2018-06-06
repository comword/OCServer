#define CATCH_CONFIG_RUNNER
#include "catch.hpp"
#include "mfilesystem.h"
#include "debug.h"
#include "config.h"
#include "jImpl.h"
#include <stdio.h>
#include <vector>

JCommChannel *jc;

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
    std::string javadir = PATH_CLASS::get_pathname( "javadir" );
    std::string basedir = PATH_CLASS::get_pathname( "basedir" );
    DebugLog( D_INFO, D_JAVA ) << javadir;
    jc = new JCommChannel();
    jc->createJVM( javadir + "JavaP.jar:" + javadir + "lib/mysql-connector-java-5.1.46.jar",
                   basedir + "Core" );
    jc->load_allAvaJavaSrvs();
    jc->InitDB(conf->js["db_addr"], conf->js["db_username"], conf->js["db_password"]);
    result = session.run();
    deinitDebug();
    return result;
}
