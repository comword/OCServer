#include "jImpl.h"
#include "debug.h"
#include "mfilesystem.h"

std::string jUtils::jArr2str( jbyteArray array )
{
    int len = env->GetArrayLength( array );
    char *buf = new char[len];
    env->GetByteArrayRegion( array, 0, len, reinterpret_cast<jbyte *>( buf ) );
    return std::string( buf, len );
}

jbyteArray jUtils::str2jArr( std::string str )
{
    size_t len = str.length();
    jbyteArray array = env->NewByteArray( len );
    env->SetByteArrayRegion( array, 0, len, reinterpret_cast<jbyte *>( ( signed char * )str.c_str() ) );
    return array;
}

jstring jUtils::str2jstr( std::string str )
{
    jstring res;
    res = env->NewStringUTF( str.c_str() );
    return res;
}

std::string jUtils::jstr2str( jstring jStr )
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

//javabr::javabr()
//{
//    JavaVMOption jvmopt[1];
//    jvmopt[0].optionString = ( char * )std::string( "-Djava.class.path=" +
//                             PATH_CLASS::get_pathname( "javadir" ) ).c_str();
//
//    JavaVMInitArgs vmArgs;
//    vmArgs.version = JNI_VERSION_1_8;
//    vmArgs.nOptions = 1;
//    vmArgs.options = jvmopt;
//    vmArgs.ignoreUnrecognized = JNI_TRUE;
//
//    // Create the JVM
//    long flag = JNI_CreateJavaVM( &jvm, ( void ** )&env, &vmArgs );
//    if( flag == JNI_ERR ) {
//        DebugLog( D_WARNING, D_JAVA ) << "Error creating JVM.";
//        return;
//    }
//}
//
//javabr::~javabr()
//{
//    jvm->DestroyJavaVM();
//}
void JCommChannel::createJVM(std::string classpath)
{
	JavaVMOption jvmopt[1];
    jvmopt[0].optionString = (char*)("-Djava.class.path=" + classpath).c_str();

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

void JCommChannel::loadEnv(JNIEnv *e)
{
	this->env = e;
	e->GetJavaVM(&jvm);
	exloadedJVM = true;
}

void JCommChannel::destroyJVM()
{
	if(!exloadedJVM)
		jvm->DestroyJavaVM();
}
