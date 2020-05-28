//
// Created by Tim DeBenedictis on 4/9/20.
//

#ifndef JNIUTILITIES_H
#define JNIUTILITIES_H

#include <jni.h>
#include <android/log.h>
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>

#include "SSTime.hpp"
#include "SSVector.hpp"
#include "SSMatrix.hpp"
#include "SSIdentifier.hpp"
#include "SSObject.hpp"

void android_fopen_set_asset_manager ( AAssetManager* manager );
FILE *android_fopen ( const char* fname, const char* mode );

jobject CreateJObject ( JNIEnv *pEnv, const char *pClassName );
void SetCharField ( JNIEnv *pEnv, jobject pObject, const char *pFieldName, jchar value );
void SetShortField ( JNIEnv *pEnv, jobject pObject, const char *pFieldName, jshort value );
void SetIntField ( JNIEnv *pEnv, jobject pObject, const char *pFieldName, jint value );
void SetLongField ( JNIEnv *pEnv, jobject pObject, const char *pFieldName, jlong value );
void SetFloatField ( JNIEnv *pEnv, jobject pObject, const char *pFieldName, jfloat value );
void SetDoubleField ( JNIEnv *pEnv, jobject pObject, const char *pFieldName, jdouble value );
void SetObjectField ( JNIEnv *pEnv, jobject pObject, const char *pFieldName, const char *pSignature, jobject value );

jchar GetCharField ( JNIEnv *pEnv, jobject pObject, const char *pFieldName );
jshort GetShortField ( JNIEnv *pEnv, jobject pObject, const char *pFieldName );
jint GetIntField ( JNIEnv *pEnv, jobject pObject, const char *pFieldName );
jlong GetLongField ( JNIEnv *pEnv, jobject pObject, const char *pFieldName );
jfloat GetFloatField ( JNIEnv *pEnv, jobject pObject, const char *pFieldName );
jdouble GetDoubleField ( JNIEnv *pEnv, jobject pObject, const char *pFieldName );
jobject GetObjectField ( JNIEnv *pEnv, jobject pObject, const char *pFieldName, const char *pSignature );

jobject SSTimeToJSSTime ( JNIEnv *pEnv, const SSTime &time );
SSTime JSSTimeToSSTime ( JNIEnv *pEnv, jobject pJSSTime );

jobject SSDateToJSSDate ( JNIEnv *pEnv, const SSDate &date );
SSDate JSSDateToSSDate ( JNIEnv *pEnv, jobject pJSSDate );

jobject SSDegMinSecToJSSDegMinSec ( JNIEnv *pEnv, const SSDegMinSec &dms );
SSDegMinSec JSSDegMinSecToSSDegMinSec ( JNIEnv *pEnv, jobject pJSSDegMinSec );

jobject SSHourMinSecToJSSHourMinSec ( JNIEnv *pEnv, const SSHourMinSec &hms );
SSHourMinSec JSSHourMinSecToSSHourMinSec ( JNIEnv *pEnv, jobject pJSSHourMinSec );

jobject SSSphericalToJSSSpherical ( JNIEnv *pEnv, const SSSpherical &spherical );
SSSpherical JSSSphericalToSSSpherical ( JNIEnv *pEnv, jobject pJSSSpherical );

jobject SSVectorToJSSVector ( JNIEnv *pEnv, const SSVector &vector );
void SSVectorToJSSVector ( JNIEnv *pEnv, const SSVector &vector, jobject pJSSVector );
SSVector JSSVectorToSSVector ( JNIEnv *pEnv, jobject pJSSVector );

jobject SSMatrixToJSSMatrix ( JNIEnv *pEnv, const SSMatrix &matrix );
SSVector JSSVectorToSSVector ( JNIEnv *pEnv, jobject pJSSVector );

jobject SSIdentifierToJSSIdentifier ( JNIEnv *pEnv, const SSIdentifier &ident );
SSIdentifier JSSIdentifierToIdentifier (JNIEnv *pEnv, jobject pJSSIdentifier );

jobject SSEventTimeToJSSEventTime ( JNIEnv *pEnv, const SSEventTime &eventtime );
SSEventTime JSSEventTimeToSSEventTime (JNIEnv *pEnv, jobject pJSSEventTime );

jobject SSPassToJSSPass ( JNIEnv *pEnv, const SSPass &pass );
SSPass JSSPassToSSPass (JNIEnv *pEnv, jobject pJSSPass );

jobject SSRTSToJSSRTS ( JNIEnv *pEnv, const SSRTS &rts );
SSRTS JSSRTSToSSRTS (JNIEnv *pEnv, jobject pJSSRTS );

jobject SSObjectToJSSObject ( JNIEnv *pEnv, SSObject *pObject );
SSObject *JSSObjectToSSObject (JNIEnv *pEnv, jobject pJSSObject );

#endif // JNIUTILITIES_H
