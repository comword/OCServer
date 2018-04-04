#include "mfilesystem.h"
#include "debug.h"
#include "uv_net.h"
#include "javabr.h"

#include <functional>
#include <signal.h>
#include <stdlib.h>
#include <stdexcept>
#include <map>
#include <string.h>
#include <string>

#if !(defined _WIN32 || defined WINDOWS || defined __CYGWIN__)
#include <unistd.h>
#include <sys/stat.h>
#else
#include <windows.h>
#endif

uvnet *net;
javabr* jbr;
void exit_handler( int s );

namespace
{

struct arg_handler {
    //! This is flag handler structure to define a function to be called. This function will be
    //! invoked with the number of arguments after the functional flag and the array of them.
    typedef std::function<int( int, const char ** )> handler_method;

    const char *flag;  //! This is the cmd line parameter to handle ("--server").
    const char *param_documentation;  //! This is the parameter documentation to show ("<IP Address>").
    const char *documentation;  //! This is the easy readable documentation to print.
    const char *help_group; //!< Section of the help message in which to include this argument.
    handler_method handler;  //!< The callback to be invoked when this argument is encountered.
};

void printHelpMessage( const arg_handler *arguments, size_t num_arguments );
}

int main( int argc, char *argv[] )
{
    bool if_exit = false;
    bool is_daemon = false;
    std::string configure_file = "";
    int port = 9001;
    //  const char *help_section_default = nullptr;
    PATH_CLASS::init_user_dir( "" );
    PATH_CLASS::update_datadir();
    const char *help_section_system = "System";
    const char *help_section_network = "Network";
    const arg_handler arg_proc[] = {
        {
            "--daemon", nullptr, "Start program in background.",
            help_section_system,
            [&is_daemon]( int, const char ** ) -> int {
                is_daemon = true;
                return 0;
            }
        },
        {
            "--config", "<path to configure file>", "Import server settings from a configure file. Settings by parameters will override settings from configure file.",
            help_section_system,
            [&configure_file]( int, const char **params ) -> int {
                configure_file = std::string( params[0] );
                return 1;
            }
        },
        {
            "--port", "<port numbers>", "Bind specific port. Default to 9002.",
            help_section_network,
            [&port]( int, const char **params ) -> int {
                port = atoi( params[0] );
                return 1;
            }
        },
        {
            "--log", "<string to log file path>", "Specify log file path. Set to NULL to drop all logs. Default to (UserDirectory)/logs/",
            help_section_system,
            []( int, const char **params ) -> int {
                PATH_CLASS::update_pathname( "logdir", params[0] );
                return 1;
            }
        },
        {
            "--userdir", "<path to userdir>", "Specify server (UserDirectory). Default to ($HOME)/.OCServer/",
            help_section_system,
            []( int, const char **params ) -> int {
                PATH_CLASS::update_pathname( "userdir", params[0] );
                PATH_CLASS::update_datadir();
                return 1;
            }
        },
        {
            "--loglevel", "<level of sever log>", "Set debug levels that should be logged. Available values are 0 to 7. D_ERROR is always logged. D_INFO=1, D_WARNING=2, D_ERROR=4.",
            help_section_system,
            []( int, const char **params ) -> int {
                int bitmask = atoi( params[0] );
                if( bitmask >= 0 && bitmask <= 7 )
                {
                    bitmask |= D_ERROR;
                    limitDebugLevel( bitmask );
                } else
                {
                    throw std::runtime_error( std::string( "--loglevel available values are 0 to 7.\n" ) );
                }
                return 1;
            }
        }
    };
    // Process CLI arguments.
    const size_t num_arguments =
        sizeof( arg_proc ) / sizeof( arg_proc[0] );
    //int saved_argc = --argc; // skip program name
    //const char **saved_argv = (const char **)++argv;
    --argc;
    ++argv;
    while( argc ) {
        if( !strcmp( argv[0], "--help" ) ) {
            printHelpMessage( arg_proc, num_arguments );
            return 0;
        } else {
            bool arg_handled = false;
            for( size_t i = 0; i < num_arguments; ++i ) {
                auto &arg_handler = arg_proc[i];
                if( !strcmp( argv[0], arg_handler.flag ) ) {
                    argc--;
                    argv++;
                    int args_consumed = arg_handler.handler( argc, ( const char ** )argv );
                    if( args_consumed < 0 ) {
                        printf( "Failed parsing parameter '%s'\n", *( argv - 1 ) );
                        exit( 1 );
                    }
                    argc -= args_consumed;
                    argv += args_consumed;
                    arg_handled = true;
                    break;
                }
            }
            // Skip other options.
            if( !arg_handled ) {
                --argc;
                ++argv;
            }
        }
    }
    //Start INIT
    if( is_daemon == true ) {
#if !(defined _WIN32 || defined WINDOWS || defined __CYGWIN__)
        pid_t pid, sid;
        pid = fork();
        if( pid < 0 ) {
            exit( EXIT_FAILURE );
        }
        if( pid > 0 ) {
            exit( EXIT_SUCCESS );
        }
        umask( 0 );
        sid = setsid();
        if( sid < 0 ) {
            exit( EXIT_FAILURE );
        }
#else
        ShowWindow(GetConsoleWindow(), SW_HIDE);
#endif
        PATH_CLASS::check_logs();
        //setupDebug();
    } else {
#if !(defined _WIN32 || defined WINDOWS || defined __CYGWIN__)
        int res = std::system( "clear" ); // Clear screen
#else
        int res = std::system( "cls" );
#endif
        (void) res;
        PATH_CLASS::check_logs();
        //redirect_out_to_log(true);
        setupDebug("Server.log");
    }
#if !(defined _WIN32 || defined WINDOWS || defined __CYGWIN__)
    struct sigaction sigHandler;
    sigHandler.sa_handler = exit_handler;
    sigemptyset( &sigHandler.sa_mask );
    sigHandler.sa_flags = 0;
    sigaction( SIGINT, &sigHandler, NULL );
#endif
    if( setlocale( LC_ALL, "" ) == NULL ) {
        DebugLog( D_WARNING, D_MAIN ) << "main.cpp:setlocale(LC_ALL, '') == NULL.\n";
    }
    //net = new uvnet("0.0.0.0",9001,false);
    jbr = new javabr();

    net = new uvnet();
    net->bind_net();
    net->Start();
    while(!if_exit){
    	sleep(1);
    }
}

namespace
{
void printHelpMessage( const arg_handler *arguments, size_t num_arguments )
{
    // Group all arguments by help_group.
    std::multimap<std::string, const arg_handler *> help_map;
    for( size_t i = 0; i < num_arguments; ++i ) {
        std::string help_group;
        if( arguments[i].help_group ) {
            help_group = arguments[i].help_group;
        }
        help_map.insert( std::make_pair( help_group, &arguments[i] ) );
    }
    printf( "Command line paramters:\n" );
    std::string current_help_group;
    auto it = help_map.begin();
    auto it_end = help_map.end();
    for( ; it != it_end; ++it ) {
        if( it->first != current_help_group ) {
            current_help_group = it->first;
            printf( "\n%s\n", current_help_group.c_str() );
        }
        const arg_handler *handler = it->second;
        printf( "%s", handler->flag );
        if( handler->param_documentation ) {
            printf( " %s", handler->param_documentation );
        }
        printf( "\n" );
        if( handler->documentation ) {
            printf( "\t%s\n", handler->documentation );
        }
    }
}
}  // namespace
void exit_handler( int n )
{
    if( n == 2 ) {
        //    std::system("clear");
        int status = 0;
        std::exit( status );
    } else {
        std::exit( n );
    }
}
