#ifndef _JAVASOFT_JNI_MD_H_
#define _JAVASOFT_JNI_MD_H_

#include <stdint.h>

#define JNIEXPORT __declspec(dllexport)
#define JNIIMPORT __declspec(dllimport)
#define JNICALL __stdcall

typedef int32_t jint;
typedef int64_t jlong;
typedef signed char jbyte;

#endif /* !_JAVASOFT_JNI_MD_H_ */

#include <jni.h>
