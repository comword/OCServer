#include "org_gtdev_oc_server_JNInterface.h"
#include "debug.h"
#include "mfilesystem.h"
#include "export.h"
/**
 * Priority constant for the println method; use Log.v.
public static final int VERBOSE = 2;
 * Priority constant for the println method; use Log.d.
public static final int DEBUG = 3;
 * Priority constant for the println method; use Log.i.
public static final int INFO = 4;
 * Priority constant for the println method; use Log.w.
public static final int WARN = 5;
 * Priority constant for the println method; use Log.e.
public static final int ERROR = 6;
 * Priority constant for the println method.
public static final int ASSERT = 7;
 */

std::string jstr2str( JNIEnv *env, jstring str );

JNIEXPORT jint JNICALL Java_org_gtdev_oc_server_JNInterface_println
( JNIEnv *env, jclass, jint priority, jstring tag, jstring msg )
{
    switch( priority ) {
        case 2:
            DebugLog( "VERBOSE", jstr2str( env, tag ) ) << jstr2str( env, msg );
            break;
        case 3:
            DebugLog( "DEBUG", jstr2str( env, tag ) ) << jstr2str( env, msg );
            break;
        case 4:
            DebugLog( "INFO", jstr2str( env, tag ) ) << jstr2str( env, msg );
            break;
        case 5:
            DebugLog( "WARN", jstr2str( env, tag ) ) << jstr2str( env, msg );
            break;
        case 6:
            DebugLog( "ERROR", jstr2str( env, tag ) ) << jstr2str( env, msg );
            break;
        case 7:
            DebugLog( "ASSERT", jstr2str( env, tag ) ) << jstr2str( env, msg );
            break;
    }
    return 0;
}

std::string jstr2str( JNIEnv *env, jstring jStr )
{
    /*if (!jStr)
            return "";
    const jclass stringClass = env->GetObjectClass(jStr);
    const jmethodID getBytes = env->GetMethodID(stringClass, "getBytes", "(Ljava/lang/String;)[B");
    const jbyteArray stringJbytes = (jbyteArray) env->CallObjectMethod(jStr, getBytes, env->NewStringUTF("UTF-8"));

    size_t length = (size_t) env->GetArrayLength(stringJbytes);
    jbyte* pBytes = env->GetByteArrayElements(stringJbytes, NULL);

    std::string ret = std::string((char *)pBytes, length);
    env->ReleaseByteArrayElements(stringJbytes, pBytes, JNI_ABORT);

    env->DeleteLocalRef(stringJbytes);
    env->DeleteLocalRef(stringClass);
    return ret;*/
    const char *cstr = env->GetStringUTFChars( jStr, JNI_FALSE );
    std::string str = std::string( cstr );
    env->ReleaseStringUTFChars( jStr, cstr );
    return str;
}

class javabr
{
    private:
        JNIEnv *env;
        JavaVM *jvm;
    public:
        DLL_PUBLIC javabr();
        DLL_PUBLIC ~javabr();

};

javabr::javabr()
{
    JavaVMOption jvmopt[1];
    jvmopt[0].optionString = ( char * )std::string( "-Djava.class.path=" +
                             PATH_CLASS::get_pathname( "javadir" ) ).c_str();

    JavaVMInitArgs vmArgs;
    vmArgs.version = JNI_VERSION_1_8;
    vmArgs.nOptions = 1;
    vmArgs.options = jvmopt;
    vmArgs.ignoreUnrecognized = JNI_TRUE;

    // Create the JVM
    long flag = JNI_CreateJavaVM( &jvm, ( void ** )&env, &vmArgs );
    if( flag == JNI_ERR ) {
        DebugLog( D_WARNING, D_JAVA ) << "Error creating JVM.";
        return;
    }
}

javabr::~javabr()
{
    jvm->DestroyJavaVM();
}

