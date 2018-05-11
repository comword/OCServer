#include "catch.hpp"

#include "mfilesystem.h"
#include "jImpl.h"
#include "debug.h"

JCommChannel jcc;

TEST_CASE( "Load JVM" )
{
	std::string javadir = PATH_CLASS::get_pathname("javadir");
	std::string basedir = PATH_CLASS::get_pathname("basedir");
	DebugLog(D_INFO, D_JAVA)<<javadir;
	jcc.createJVM(javadir+"JavaP.jar:"+javadir+"lib/mysql-connector-java-5.1.46.jar",basedir+"Core");
	std::string res = jcc.pingJava();
	DebugLog(D_INFO, D_JAVA) << res;
}
