/**
 * @file config.cpp
 * @auther Ge Tong
 * @brief This file implements the loading and saving of Json server configuration data.
 * Json parsing library is json.hpp(nlohmann::json) https://github.com/nlohmann/json version 3.1.2
 * When the game server program starts, it loads the default configuration file according to the input parameters
 * from "userdir" defined in @link PATH_CLASS#FILENAMES
 */
#include "config.h"
#include "debug.h"

#include <fstream>
#include <sstream>
#include <string.h>

Configure *conf;

Configure::Configure( std::string path ):
    config_path( path )
{
    std::ifstream infile( path.c_str(), std::ifstream::in | std::ifstream::binary );
    if( infile.fail() ) {
        DebugLog( D_ERROR, D_MAIN ) << "Error: cannot open configure file: " + path + ": " + std::string(
                                        strerror(
                                            errno ) );
    }
    std::istringstream iss(
        std::string(
            ( std::istreambuf_iterator<char>( infile ) ),
            std::istreambuf_iterator<char>()
        )
    );
    try {
        json jsin = json::parse( iss );
        this->js = jsin;
        infile.close();
    } catch( std::domain_error &e ) {

    }
}

Configure::~Configure() {}

void Configure::writeConf( std::string path )
{
    std::ofstream outfile( path.c_str(), std::ofstream::out | std::ofstream::binary );
    if( outfile.fail() ) {
        DebugLog( D_WARNING, D_MAIN ) << "Error: cannot open configure file: " << ": " << std::string(
                                          strerror(
                                              errno ) );
        return;
    }
    std::string s = js.dump();
    outfile << s;
    return;
}


