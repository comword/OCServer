/**
 * @file jni.cpp
 * @auther Ge Tong
 * @brief Implements of native methods exported from JNInterface.java
 */
#include "org_gtdev_oc_server_JNInterface.h"
#include "debug.h"
#include "mfilesystem.h"
#include "config.h"
#include "export.h"
#include "proto.h"
#include "jImpl.h"

JNIEXPORT jint JNICALL Java_org_gtdev_oc_server_JNInterface_println
( JNIEnv *env, jclass, jint priority, jstring tag, jstring msg )
{
    jUtils u( env );
    switch( priority ) {
        case 2:
            DebugLog( "VERBOSE", u.jstr2str( tag ) ) << u.jstr2str( msg );
            break;
        case 3:
            DebugLog( "DEBUG", u.jstr2str( tag ) ) << u.jstr2str( msg );
            break;
        case 4:
            DebugLog( "INFO", u.jstr2str( tag ) ) << u.jstr2str( msg );
            break;
        case 5:
            DebugLog( "WARN", u.jstr2str( tag ) ) << u.jstr2str( msg );
            break;
        case 6:
            DebugLog( "ERROR", u.jstr2str( tag ) ) << u.jstr2str( msg );
            break;
        case 7:
            DebugLog( "ASSERT", u.jstr2str( tag ) ) << u.jstr2str( msg );
            break;
    }
    return 0;
}

JNIEXPORT jboolean JNICALL Java_org_gtdev_oc_server_JNInterface_loadConf
( JNIEnv *env, jclass, jobject m )
{
    if( !conf ) {
        return false;
    }
    jclass mapClass = env->GetObjectClass( m );
    jmethodID put = env->GetMethodID( mapClass, "put",
                                      "(Ljava/lang/Object;Ljava/lang/Object;)Ljava/lang/Object;" );

    return true;
}

JNIEXPORT jstring JNICALL Java_org_gtdev_oc_server_JNInterface_getPath
( JNIEnv *env, jclass, jstring key )
{
    jUtils u( env );
    std::string ckey = u.jstr2str( key );
    if( PATH_CLASS::FILENAMES.count( ckey ) ) {
        return u.str2jstr( PATH_CLASS::FILENAMES[ckey] );
    }
    return ( jstring )env->NewGlobalRef( NULL );
}

JNIEXPORT void JNICALL Java_org_gtdev_oc_server_JNInterface_getAllPaths
( JNIEnv *env, jclass, jobject m )
{
    jclass mapClass = env->GetObjectClass( m );
    jmethodID put = env->GetMethodID( mapClass, "put",
                                      "(Ljava/lang/Object;Ljava/lang/Object;)Ljava/lang/Object;" );
    for( auto const it : PATH_CLASS::FILENAMES ) {
        env->CallObjectMethod( m, put, env->NewStringUTF( it.first.c_str() ),
                               env->NewStringUTF( it.second.c_str() ) );
    }
}

JNIEXPORT jstring JNICALL Java_org_gtdev_oc_server_JNInterface_displayProto
( JNIEnv *env, jclass, jbyteArray d )
{
    jUtils u( env );
    std::stringstream ss( u.jArr2str( d ) );
    ProtoDisplay display( ss );
    display.do_display();
    return u.str2jstr( display.output.str() );
}

JNIEXPORT jboolean JNICALL Java_org_gtdev_oc_server_JNInterface_doTransact
( JNIEnv *env, jclass, jint, jbyteArray, jbyteArray, jint )
{
    jUtils u( env );
    return false;
}

