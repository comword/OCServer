#ifndef SRC_JUTILS_H_
#define SRC_JUTILS_H_

#include <jni.h>
#include <string>

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
    public:
        JCommChannel(){}
        virtual ~JCommChannel(){}
        void destroyJVM();
        void createJVM(std::string classpath);
        void loadEnv(JNIEnv *e);
};

#endif /* SRC_JUTILS_H_ */
