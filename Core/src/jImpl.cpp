/**
 * @file jImpl.cpp
 * @auther Ge Tong
 * @brief This file contains the utilities communicating to Java parts.
 * This class initializes the JVM and implements several useful type casting
 * functions are defined, including jstring<->std::string, jbyteArray<->std::string
 */
#include "jImpl.h"
#include "debug.h"
#include "mfilesystem.h"
#include "config.h"
#include "util.h"

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

void JCommChannel::createJVM( std::string classpath, std::string nlibrary )
{
    JavaVMOption jvmopt[2];
    classpath = "-Djava.class.path=" + classpath;
    nlibrary = "-Djava.library.path=" + nlibrary;
    DebugLog( D_INFO, D_JAVA ) << classpath;
    DebugLog( D_INFO, D_JAVA ) << nlibrary;
    jvmopt[0].optionString = ( char * )classpath.c_str();
    jvmopt[1].optionString = ( char * )nlibrary.c_str();

    JavaVMInitArgs vmArgs;
    vmArgs.version = JNI_VERSION_1_8;
    vmArgs.nOptions = 2;
    vmArgs.options = jvmopt;
    vmArgs.ignoreUnrecognized = false;

    // Create the JVM
    long flag = JNI_CreateJavaVM( &jvm, ( void ** )&env, &vmArgs );
    if( flag != JNI_OK ) {
        DebugLog( D_WARNING, D_JAVA ) << "Error creating JVM: " << flag;
        return;
    }
    DebugLog( D_INFO, D_JAVA ) << "JVM loaded successfully.";
    assert( env );
    util = new jUtils( env );
    loadJavaSrvConf();
}

void JCommChannel::loadEnv( JNIEnv *e )
{
    this->env = e;
    e->GetJavaVM( &jvm );
    exloadedJVM = true;
}

void JCommChannel::destroyJVM()
{
    if( !exloadedJVM ) {
        jvm->DestroyJavaVM();
    }
}

std::string JCommChannel::pingJava()
{
    jclass JNIcls = env->FindClass( "org/gtdev/oc/server/JNInterface" );
    if( JNIcls == nullptr ) {
        DebugLog( D_ERROR, D_JAVA ) << "ERROR: JNInterface class not found.";
    } else {
        jmethodID mid = env->GetStaticMethodID( JNIcls, "PingJava", "()Ljava/lang/String;" );
        if( mid == nullptr ) {
            DebugLog( D_ERROR, D_JAVA ) << "ERROR: static method String PingJava() not found !";
        } else {
            jstring returnString = ( jstring ) env->CallStaticObjectMethod( JNIcls, mid );
            std::string res = util->jstr2str( returnString );
            env->DeleteLocalRef( returnString );
            return res;
        }
    }
    return std::string();
}

void JCommChannel::loadJavaSrvConf()
{
    json c = conf->js;
    ava_srvs = c["server_apps"].get<std::vector<std::string>>();
    std::string pr = "Available Java apps:";
    for( auto a : ava_srvs ) {
        pr.push_back( ' ' );
        pr.append( a );
    }
    DebugLog( D_INFO, D_JAVA ) << pr;
}

bool JCommChannel::loadJavaSrv( std::string classpath )
{
    jclass cls = env->FindClass( std::string( BASE_SVRS_CLASSPATH + classpath ).c_str() );
    if( cls == nullptr ) {
        return false;
    }
    jmethodID mid = env->GetMethodID( cls, "<init>", "()V" );
    jobject app = env->NewObject( cls, mid );
    reg_srvs.insert( std::make_pair( classpath, app ) );
    return false;
}

bool JCommChannel::load_allAvaJavaSrvs()
{
    bool res;
    for( auto a : ava_srvs ) {
        res = loadJavaSrv( a );
        if( !res ) {
            return false;
        }
    }
    return true;
}

bool JCommChannel::execTransact( jobject obj, std::string tosrv, std::string &fromsrv, int flags )
{
    jclass cls = env->GetObjectClass( obj );
    assert( cls );
    jmethodID mid = env->GetMethodID( cls, "execTransact",
                                      "(Ljava/nio/ByteBuffer;Ljava/nio/ByteBuffer;I)Z" );
    jobject bt = env->NewDirectByteBuffer( ( void * ) tosrv.c_str(), tosrv.length() );

//	jclass bbcls = env->FindClass("java/nio/ByteBuffer");
//	jmethodID bbinit = env->GetMethodID(bbcls, "allocate", "void(I)");
//	jobject bf = env->NewObject(bbcls, bbinit, 128); //default length is 255.
    char jbuff[512] = {0};
    jobject bf = env->NewDirectByteBuffer( jbuff, 512 );

    bool result = env->CallBooleanMethod( obj, mid, bt, bf, flags );
	char *buf = ( char * )env->GetDirectBufferAddress( bf );
	if(buf != nullptr) {
		size_t len = env->GetDirectBufferCapacity( bf );
		//DebugLog(D_WARNING,D_JAVA)<<len;
		fromsrv = std::string( buf, len );
		while( !fromsrv.empty() && fromsrv.back() == '\0' ) {
				fromsrv.pop_back();
		}
	}
    env->DeleteLocalRef( bt );
    env->DeleteLocalRef( bf );
    return result;
}

jobject JCommChannel::getService( std::string cmdName )
{
    if( reg_srvs.find( cmdName ) == reg_srvs.end() ) {
        return nullptr;
    } else {
        return reg_srvs.at( cmdName );
    }
}

bool JCommChannel::InitDB( std::string addr, std::string uname, std::string pwd)
{
	bool result = false;
	jclass JNIcls = env->FindClass( "org/gtdev/oc/server/JNInterface" );
	if( JNIcls == nullptr ) {
		DebugLog( D_ERROR, D_JAVA ) << "ERROR: JNInterface class not found.";
	} else {
		jmethodID mid = env->GetStaticMethodID( JNIcls, "InitDB", "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)Z" );
		jstring jaddr = util->str2jstr(addr);
		jstring juname = util->str2jstr(uname);
		jstring jpwd = util->str2jstr(pwd);
		result = env->CallStaticBooleanMethod(JNIcls, mid, jaddr, juname, jpwd);
		env->DeleteLocalRef( jaddr );
		env->DeleteLocalRef( juname );
		env->DeleteLocalRef( jpwd );
	}
	return result;
}
