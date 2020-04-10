//
// Created by Tim DeBenedictis on 4/9/20.
//

#ifndef JNIUTILITIES_H
#define JNIUTILITIES_H

#include <jni.h>

#include "SSTime.hpp"
#include "SSVector.hpp"

jobject CreateJObject ( JNIEnv *pEnv, const char *pClassName );
void SetCharField ( JNIEnv *pEnv, jobject pObject, const char *pFieldName, jchar value );
void SetShortField ( JNIEnv *pEnv, jobject pObject, const char *pFieldName, jshort value );
void SetIntField ( JNIEnv *pEnv, jobject pObject, const char *pFieldName, jint value );
void SetLongField ( JNIEnv *pEnv, jobject pObject, const char *pFieldName, jlong value );
void SetFloatField ( JNIEnv *pEnv, jobject pObject, const char *pFieldName, jfloat value );
void SetDoubleField ( JNIEnv *pEnv, jobject pObject, const char *pFieldName, jdouble value );

jchar GetCharField ( JNIEnv *pEnv, jobject pObject, const char *pFieldName );
jshort GetShortField ( JNIEnv *pEnv, jobject pObject, const char *pFieldName );
jint GetIntField ( JNIEnv *pEnv, jobject pObject, const char *pFieldName );
jlong GetLongField ( JNIEnv *pEnv, jobject pObject, const char *pFieldName );
jfloat GetFloatField ( JNIEnv *pEnv, jobject pObject, const char *pFieldName );
jdouble GetDoubleField ( JNIEnv *pEnv, jobject pObject, const char *pFieldName );

jobject SSTimeToJSSTime ( JNIEnv *pEnv, SSTime &time );
SSTime JSSTimeToSSTime ( JNIEnv *pEnv, jobject pJSSTime );

jobject SSDateToJSSDate ( JNIEnv *pEnv, SSDate &date );
SSDate JSSDateToSSDate ( JNIEnv *pEnv, jobject pJSSDate );

jobject SSDegMinSecToJSSDegMinSec ( JNIEnv *pEnv, SSDegMinSec &time );
SSDegMinSec JSSDegMinSecToSSDegMinSec ( JNIEnv *pEnv, jobject pJSSDegMinSec );

jobject SSHourMinSecToJSSHourMinSec ( JNIEnv *pEnv, SSHourMinSec &time );
SSHourMinSec JSSHourMinSecToSSHourMinSec ( JNIEnv *pEnv, jobject pJSSHourMinSec );

jobject SSSphericalToJSSSpherical ( JNIEnv *pEnv, SSSpherical &spherical );
SSSpherical JSSSphericalToSSSpherical ( JNIEnv *pEnv, jobject pJSSSpherical );

jobject SSVectorToJSSVector ( JNIEnv *pEnv, SSVector &vector );
SSVector JSSVectorToSSVector ( JNIEnv *pEnv, jobject pJSSVector );

#endif // JNIUTILITIES_H
