/**
 * @file config.h
 * @auther Ge Tong
 */

#include "json.hpp"
#include "export.h"

#include <string>
//#include <map>

// The reference to the one and only configure.
class Configure;
DLL_PUBLIC extern Configure *conf;
using json = nlohmann::json;

class Configure
{
    public:
        std::string config_path, server_crt, server_key;
        DLL_PUBLIC Configure( std::string path );
        DLL_PUBLIC virtual ~Configure();
        DLL_PUBLIC void writeConf( std::string path );
        json js;
};
