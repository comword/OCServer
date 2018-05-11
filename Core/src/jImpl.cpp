#include "jImpl.h"
#include "debug.h"
#include "mfilesystem.h"
#include "config.h"

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

//Classpath Java/JavaP.jar;Java/lib/mysql-connector-java-5.1.46.jar
void JCommChannel::createJVM(std::string classpath, std::string nlibrary)
{
	JavaVMOption jvmopt[2];
	classpath = "-Djava.class.path=" + classpath;
	nlibrary = "-Djava.library.path=" + nlibrary;
	DebugLog( D_INFO, D_JAVA ) << classpath;
	DebugLog( D_INFO, D_JAVA ) << nlibrary;
    jvmopt[0].optionString = (char*)classpath.c_str();
    jvmopt[1].optionString = (char*)nlibrary.c_str();

    JavaVMInitArgs vmArgs;
    vmArgs.version = JNI_VERSION_1_8;
    vmArgs.nOptions = 2;
    vmArgs.options = jvmopt;
    vmArgs.ignoreUnrecognized = false;

    // Create the JVM
    long flag = JNI_CreateJavaVM( &jvm, ( void ** )&env, &vmArgs );
    if( flag != JNI_OK ) {
        DebugLog( D_WARNING, D_JAVA ) << "Error creating JVM: "<<flag;
        return;
    }
    DebugLog( D_INFO, D_JAVA ) << "JVM loaded successfully.";
    assert(env);
    util = new jUtils(env);
    loadJavaApps();
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

std::string JCommChannel::pingJava()
{
	jclass JNIcls = env->FindClass("org/gtdev/oc/server/JNInterface");
	if(JNIcls == nullptr) {
	    DebugLog(D_ERROR, D_JAVA) << "ERROR: JNInterface class not found.";
	}
	else {
	    jmethodID mid = env->GetStaticMethodID(JNIcls, "PingJava", "()Ljava/lang/String;");
	    if(mid == nullptr)
	    	DebugLog(D_ERROR, D_JAVA) << "ERROR: static method String PingJava() not found !";
	    else {
	    	jstring returnString = (jstring) env->CallStaticObjectMethod(JNIcls, mid);
	    	std::string res = util->jstr2str(returnString);
	    	env->DeleteLocalRef(returnString);
	    	return res;
	    }
	}
	return std::string();
}

void JCommChannel::loadJavaApps()
{
	json c = conf->js;
	ava_apps = c["server_apps"].get<std::vector<std::string>>();
	std::string pr = "Enabled Java apps:\n";
	for(auto a:ava_apps){
		pr.append(a);
		pr.push_back('\n');
	}
	DebugLog(D_INFO, D_JAVA) << pr;
}
