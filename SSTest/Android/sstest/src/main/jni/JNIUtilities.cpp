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

typedef struct ACookie
{
    AAsset *asset;      // pointer to underlying Android asset
    int fd;             // file descriptor for asset; -1 (invalid) if asset is compressed; >=0 (valid) if asset uncompressed.
    FILE *file;         // file pointer to asset; NULL if asset is compressed
    off_t start;        // offset of uncompressed asset from start of APK in bytes; zero if unknown or compressed asset
    off_t length;       // length of uncompressed asset in bytes; zero if unknown or compressed asset
}
ACookie;

static AAssetManager *android_asset_manager = NULL; // must be established by someone else...

void android_fopen_set_asset_manager ( AAssetManager* manager )
{
    android_asset_manager = manager;
}

static int android_read ( void *cookie, char *buf, int size)
{
    FILE *file = ((ACookie *)cookie)->file;
    AAsset *asset = ((ACookie *)cookie)->asset;

    // If we have a valid file pointer, use buffered C FILE * I/O to read the asset directly from the APK.

    if ( file )
    {
        off_t start = ((ACookie *) cookie)->start;
        off_t length = ((ACookie *) cookie)->length;

        // Check for reading past the end of the asset

        if ( ftell ( file ) > start + length )
            return 0;

        int n = fread ( buf, 1, size, file );
        return n < 1 ? -1 : n;
    }

    // Otherwise read using Android asset API.

    return AAsset_read ( asset, buf, size );
}

static int android_write ( void *cookie, const char *buf, int size )
{
    return EACCES; // can't provide write access to the apk
}

static fpos_t android_seek ( void* cookie, fpos_t offset, int whence )
{
    FILE *file = ((ACookie *)cookie)->file;
    AAsset *asset = ((ACookie *)cookie)->asset;

    // If we have a valid file pointer, seek using C FILE * I/O.

    if ( file )
    {
        off_t start = ((ACookie *)cookie)->start;
        off_t length = ((ACookie *)cookie)->length;

        int error = -1;
        if ( whence == SEEK_SET )
            error = fseek ( file, offset + start, SEEK_SET );
        else if ( whence == SEEK_CUR )
            error = fseek ( file, offset, SEEK_CUR );
        else if ( whence == SEEK_END )
            error = fseek ( file, start + length - offset, SEEK_SET );

        if ( error )
            return -1;

        return ftell ( file ) - start;
    }

    // Otherwise seek using Android asset API.

    return AAsset_seek ( asset, offset, whence );
}

static int android_close ( void* cookie )
{
    FILE *file = ((ACookie *)cookie)->file;
    if ( file )
        fclose ( file );

    AAsset *asset = ((ACookie *)cookie)->asset;
    if ( asset )
        AAsset_close ( asset );

    free ( cookie );
    return 0;
}

// Wrapper for fopen() that opens files in assets folder within APK.

#ifdef fopen
#undef fopen
#endif

extern "C" FILE *android_fopen ( const char *name, const char *mode )
{
    // If opening file for writing, android asset manager is
    // not initialized, or path doesn't exist in android asset,
    // just open the file the old fashioned way.

    if ( mode[0] == 'w' || android_asset_manager == NULL )
        return fopen ( name, mode );
    
    AAsset *asset = AAssetManager_open ( android_asset_manager, name, AASSET_MODE_UNKNOWN );
    if ( ! asset )
        return fopen ( name, mode );

    // Create a cookie for file I/O functions. Close asset and return nullptr on failure.

    ACookie *cookie = (ACookie *) calloc ( 1, sizeof ( ACookie ) );
    if ( ! cookie )
    {
        AAsset_close ( asset );
        return nullptr;
    }

    // Try to get a file descriptor, and then a FILE pointer, to the asset.
    // This will fail for compressed assets, but android_read(), etc. will
    // use the native, non-buffered, slow Android asset API instead, in this case.

    cookie->asset = asset;
    cookie->fd = AAsset_openFileDescriptor ( asset, &cookie->start, &cookie->length );
    if ( cookie->fd >= 0 )
    {
        cookie->file = fdopen ( cookie->fd, mode );
        if ( cookie->file && fseek ( cookie->file, cookie->start, SEEK_SET ) != 0 )
        {
            fclose ( cookie->file );
            cookie->file = nullptr;
        }
    }

    return funopen ( cookie, android_read, android_write, android_seek, android_close );
}

// This function implements to the initAssetManager() native method in JSSObjectArray.
// For an example, see comments in this blog post:
// http://www.50ply.com/blog/2013/01/19/loading-compressed-android-assets-with-file-pointer/

bool initAssetManager ( JNIEnv *env, jclass objArrCls, jobject assetManager )
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

void SetObjectField ( JNIEnv *pEnv, jobject pObject, const char *pFieldName, const char *pSignature, jobject value )
{
    jclass pClass = pEnv->GetObjectClass ( pObject );
    jfieldID fid = pEnv->GetFieldID ( pClass, pFieldName, pSignature );
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

jobject GetObjectField ( JNIEnv *pEnv, jobject pObject, const char *pFieldName, const char *pSignature )
{
    jclass pClass = pEnv->GetObjectClass ( pObject );
    jfieldID fid = pEnv->GetFieldID ( pClass, pFieldName, pSignature );
    return pEnv->GetObjectField ( pObject, fid );
}
