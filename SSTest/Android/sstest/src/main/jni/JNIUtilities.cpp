//
// Created by Tim DeBenedictis on 4/9/20.
//

#include <stdio.h>
#include <errno.h>

#include "JNIUtilities.h"

// Code to read files from Android assets based on this solution:
// http://www.50ply.com/blog/2013/01/19/loading-compressed-android-assets-with-file-pointer/
// https://github.com/netguy204/gambit-game-lib/blob/dk94/android_fopen.c
// https://github.com/netguy204/gambit-game-lib/blob/dk94/android_fopen.h

static AAssetManager *android_asset_manager = NULL; // must be established by someone else...

void android_fopen_set_asset_manager ( AAssetManager* manager )
{
    android_asset_manager = manager;
}

static int android_read ( void *cookie, char *buf, int size)
{
    return AAsset_read ( (AAsset*) cookie, buf, size );
}

static int android_write ( void *cookie, const char *buf, int size )
{
    return EACCES; // can't provide write access to the apk
}

static fpos_t android_seek ( void* cookie, fpos_t offset, int whence )
{
    return AAsset_seek ( (AAsset*)cookie, offset, whence );
}

static int android_close ( void* cookie )
{
    AAsset_close ( (AAsset *) cookie );
    return 0;
}

// Wrapper for fopen() that opens files in assets folder within APK.

FILE *android_fopen ( const char *name, const char *mode )
{
    // If opening file for writing, or if android asset manager is
    // not initialized, just open the file the old fashioned way.

    if ( mode[0] == 'w' || android_asset_manager == NULL )
        return fopen ( name, mode );

    AAsset *asset = AAssetManager_open ( android_asset_manager, name, 0 );
    if ( ! asset )
        return NULL;

    return funopen ( asset, android_read, android_write, android_seek, android_close );
}

// This JNI function correspond to an initAssetManager() method present in your MainActivity's
// java or kotlin code. For an example, see comments in this blog post:
// http://www.50ply.com/blog/2013/01/19/loading-compressed-android-assets-with-file-pointer/

extern "C" JNIEXPORT jboolean JNICALL
Java_com_southernstars_sstest_MainActivity_initAssetManager ( JNIEnv *env, jobject obj, jobject assetManager )
{
    AAssetManager *mgr = AAssetManager_fromJava ( env, assetManager );
    if ( mgr == NULL )
    {
        __android_log_print ( ANDROID_LOG_ERROR, "initAssetManager", "Failed to initialize asset manager" );
        return false;
    }
    else
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "initAssetManager",
                            "Successfully initialized asset manager");
        android_fopen_set_asset_manager(mgr);
        return true;
    }
}

jobject CreateJObject ( JNIEnv *pEnv, const char *pClassName )
{
    jclass pClass = pEnv->FindClass ( pClassName );
    if ( pClass == NULL )
        return NULL;

    jmethodID mid = pEnv->GetMethodID ( pClass, "<init>", "()V" );
    if ( mid == NULL )
        return NULL;

    jobject pObject = pEnv->NewObject ( pClass, mid );
    if ( pObject == NULL )
        return NULL;

    return pObject;
}

void SetCharField ( JNIEnv *pEnv, jobject pObject, const char *pFieldName, jchar value )
{
    jclass pClass = pEnv->GetObjectClass ( pObject );
    jfieldID fid = pEnv->GetFieldID ( pClass, pFieldName, "C" );
    pEnv->SetCharField ( pObject, fid, value );
}

void SetShortField ( JNIEnv *pEnv, jobject pObject, const char *pFieldName, jshort value )
{
    jclass pClass = pEnv->GetObjectClass ( pObject );
    jfieldID fid = pEnv->GetFieldID ( pClass, pFieldName, "S" );
    pEnv->SetShortField ( pObject, fid, value );
}

void SetIntField ( JNIEnv *pEnv, jobject pObject, const char *pFieldName, jint value )
{
    jclass pClass = pEnv->GetObjectClass ( pObject );
    jfieldID fid = pEnv->GetFieldID ( pClass, pFieldName, "I" );
    pEnv->SetIntField ( pObject, fid, value );
}

void SetLongField ( JNIEnv *pEnv, jobject pObject, const char *pFieldName, jlong value )
{
    jclass pClass = pEnv->GetObjectClass ( pObject );
    jfieldID fid = pEnv->GetFieldID ( pClass, pFieldName, "J" );
    pEnv->SetLongField ( pObject, fid, value );
}

void SetFloatField ( JNIEnv *pEnv, jobject pObject, const char *pFieldName, jfloat value )
{
    jclass pClass = pEnv->GetObjectClass ( pObject );
    jfieldID fid = pEnv->GetFieldID ( pClass, pFieldName, "F" );
    pEnv->SetFloatField ( pObject, fid, value );
}

void SetDoubleField ( JNIEnv *pEnv, jobject pObject, const char *pFieldName, jdouble value )
{
    jclass pClass = pEnv->GetObjectClass ( pObject );
    jfieldID fid = pEnv->GetFieldID ( pClass, pFieldName, "D" );
    pEnv->SetDoubleField ( pObject, fid, value );
}

void SetObjectField ( JNIEnv *pEnv, jobject pObject, const char *pFieldName, jobject value )
{
    jclass pClass = pEnv->GetObjectClass ( pObject );
    jfieldID fid = pEnv->GetFieldID ( pClass, pFieldName, "Ljava/lang/Object;" );
    pEnv->SetObjectField ( pObject, fid, value );
}

jchar GetCharField ( JNIEnv *pEnv, jobject pObject, const char *pFieldName )
{
    jclass pClass = pEnv->GetObjectClass ( pObject );
    jfieldID fid = pEnv->GetFieldID ( pClass, pFieldName, "C" );
    return pEnv->GetCharField ( pObject, fid );
}

jshort GetShortField ( JNIEnv *pEnv, jobject pObject, const char *pFieldName )
{
    jclass pClass = pEnv->GetObjectClass ( pObject );
    jfieldID fid = pEnv->GetFieldID ( pClass, pFieldName, "S" );
    return pEnv->GetShortField ( pObject, fid );
}

jint GetIntField ( JNIEnv *pEnv, jobject pObject, const char *pFieldName )
{
    jclass pClass = pEnv->GetObjectClass ( pObject );
    jfieldID fid = pEnv->GetFieldID ( pClass, pFieldName, "I" );
    return pEnv->GetIntField ( pObject, fid );
}

jlong GetLongField ( JNIEnv *pEnv, jobject pObject, const char *pFieldName )
{
    jclass pClass = pEnv->GetObjectClass ( pObject );
    jfieldID fid = pEnv->GetFieldID ( pClass, pFieldName, "J" );
    return pEnv->GetLongField ( pObject, fid );
}

jfloat GetFloatField ( JNIEnv *pEnv, jobject pObject, const char *pFieldName )
{
    jclass pClass = pEnv->GetObjectClass ( pObject );
    jfieldID fid = pEnv->GetFieldID ( pClass, pFieldName, "F" );
    return pEnv->GetFloatField ( pObject, fid );
}

jdouble GetDoubleField ( JNIEnv *pEnv, jobject pObject, const char *pFieldName )
{
    jclass pClass = pEnv->GetObjectClass ( pObject );
    jfieldID fid = pEnv->GetFieldID ( pClass, pFieldName, "D" );
    return pEnv->GetDoubleField ( pObject, fid );
}

jobject GetObjectField ( JNIEnv *pEnv, jobject pObject, const char *pFieldName )
{
    jclass pClass = pEnv->GetObjectClass ( pObject );
    jfieldID fid = pEnv->GetFieldID ( pClass, pFieldName, "Ljava/lang/Object;" );
    return pEnv->GetObjectField ( pObject, fid );
}
