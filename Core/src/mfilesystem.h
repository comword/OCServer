#pragma once
#ifndef FILESYS_H
#define FILESYS_H
#include <string>
#include <vector>
#include <map>
#include "export.h"

#ifdef _MSC_VER
#include <Windows.h>
#include "wdirent.h"
#include <direct.h>
#else
#include <dirent.h>
#include <unistd.h>
#endif

#ifndef FILE_SEP
#if (defined _WIN32 || defined WINDOW)
#define FILE_SEP '\\'
#else
#define FILE_SEP '/'
#endif // if
#define is_filesep(ch) (ch == '/' || ch == '\\')
#endif // ifndef


typedef std::map<std::string, std::string> FILEMAPCL;

namespace PATH_CLASS
{
extern FILEMAPCL FILENAMES;
unsigned long get_file_size( const char *path );
bool do_mkdir( std::string const &path, int const mode );
bool assure_dir_exist( std::string const &path );
bool file_exist( const std::string &path );
bool remove_file( const std::string &path );
bool rename_file( const std::string &old_path, const std::string &new_path );
bool is_directory( dirent const &entry, std::string const &full_path );
DLL_PUBLIC void init_user_dir( const char *ud = "" );
DLL_PUBLIC void update_datadir( void );
DLL_PUBLIC void update_pathname( std::string name, std::string path );
DLL_PUBLIC std::string get_pathname( std::string name );
DLL_PUBLIC void check_logs();
void GetFilesInDirectory( std::vector<std::string> &out, const std::string &directory );
std::string find_translated_file( const std::string &pathid, const std::string &extension,
                                  const std::string &defaultid );
}
#endif
