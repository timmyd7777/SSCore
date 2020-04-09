//
// Created by Tim DeBenedictis on 4/9/20.
//

#ifndef JNIUTILITIES_H
#define JNIUTILITIES_H

#include <jni.h>

#include "SSTime.hpp"

jobject CreateJObject ( JNIEnv *pEnv, const char *pClassName );
void SetShortField ( JNIEnv *pEnv, jobject pObject, const char *pFieldName, short value );
void SetIntField ( JNIEnv *pEnv, jobject pObject, const char *pFieldName, int value );
void SetLongField ( JNIEnv *pEnv, jobject pObject, const char *pFieldName, int64_t value );
void SetFloatField ( JNIEnv *pEnv, jobject pObject, const char *pFieldName, float value );
void SetDoubleField ( JNIEnv *pEnv, jobject pObject, const char *pFieldName, double value );

short GetShortField ( JNIEnv *pEnv, jobject pObject, const char *pFieldName );
int GetIntField ( JNIEnv *pEnv, jobject pObject, const char *pFieldName );
int64_t GetLongField ( JNIEnv *pEnv, jobject pObject, const char *pFieldName );
float GetFloatField ( JNIEnv *pEnv, jobject pObject, const char *pFieldName );
double GetDoubleField ( JNIEnv *pEnv, jobject pObject, const char *pFieldName );

jobject SSTimeToJSSTime ( JNIEnv *pEnv, SSTime &time );
SSTime JSSTimeToSSTime ( JNIEnv *pEnv, jobject pJSSTime );

jobject SSDateToJSSDate ( JNIEnv *pEnv, SSDate &date );
SSDate JSSDateToSSDate ( JNIEnv *pEnv, jobject pJSSDate );

#endif // JNIUTILITIES_H
