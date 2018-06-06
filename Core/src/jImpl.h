/**
 * @file jImpl.h
 * @auther Ge Tong
 */
#ifndef SRC_JUTILS_H_
#define SRC_JUTILS_H_

#include <jni.h>
#include <string>
#include <vector>
#include <unordered_map>

#include "export.h"

#define BASE_SVRS_CLASSPATH "org/gtdev/oc/server/services/"

class DLL_PUBLIC jUtils
{
    private:
        JNIEnv *env;
    public:
        jUtils( JNIEnv *env ): env( env ) {}
        virtual ~jUtils() {}
        std::string jstr2str( jstring str );
        jstring str2jstr( std::string str );
        std::string jArr2str( jbyteArray array );
        jbyteArray str2jArr( std::string str );
};

class DLL_PUBLIC JCommChannel
{
    private:
        JNIEnv *env = nullptr;
        JavaVM *jvm = nullptr;
        bool exloadedJVM = false;
        jUtils *util = nullptr;
        std::vector<std::string> ava_srvs;
        std::unordered_map<std::string, jobject> reg_srvs;
    public:
        JCommChannel() {}
        virtual ~JCommChannel() {
            destroyJVM();
            if( util != nullptr ) {
                delete util;
            }
        }
        void destroyJVM();
        void createJVM( std::string classpath, std::string nlibrary );
        void loadEnv( JNIEnv *e );
        std::string pingJava();
        void loadJavaSrvConf();
        bool loadJavaSrv( std::string classpath );
        bool load_allAvaJavaSrvs();
        bool execTransact( jobject obj, std::string tosrv, std::string &fromsrv, int flags );
        jobject getService( std::string cmdName );
        bool InitDB( std::string addr, std::string uname, std::string pwd);
};

extern JCommChannel *jc;

#endif /* SRC_JUTILS_H_ */
