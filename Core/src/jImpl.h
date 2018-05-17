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
        JNIEnv *env;
        JavaVM *jvm;
        bool exloadedJVM = false;
        jUtils *util = nullptr;
        std::vector<std::string> ava_apps;
        std::unordered_map<std::string,jobject> reg_apps;
    public:
        JCommChannel(){}
        virtual ~JCommChannel(){ if(util!=nullptr) delete util; }
        void destroyJVM();
        void createJVM(std::string classpath, std::string nlibrary);
        void loadEnv(JNIEnv *e);
        std::string pingJava();
        void loadJavaAppsConf();
        bool loadJavaApp(std::string classpath);
        bool execTransact(jobject obj, int code, std::string toapp, std::string& fromapp, int flags);
};

#endif /* SRC_JUTILS_H_ */
