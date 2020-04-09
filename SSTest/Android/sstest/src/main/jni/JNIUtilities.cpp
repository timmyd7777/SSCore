//
// Created by Tim DeBenedictis on 4/9/20.
//

#include "JNIUtilities.h"

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

void SetShortField ( JNIEnv *pEnv, jobject pObject, const char *pFieldName, short value )
{
    jclass pClass = pEnv->GetObjectClass ( pObject );
    jfieldID fid = pEnv->GetFieldID ( pClass, pFieldName, "S" );
    pEnv->SetShortField ( pObject, fid, value );
}

void SetIntField ( JNIEnv *pEnv, jobject pObject, const char *pFieldName, int value )
{
    jclass pClass = pEnv->GetObjectClass ( pObject );
    jfieldID fid = pEnv->GetFieldID ( pClass, pFieldName, "I" );
    pEnv->SetIntField ( pObject, fid, value );
}

void SetLongField ( JNIEnv *pEnv, jobject pObject, const char *pFieldName, int64_t value )
{
    jclass pClass = pEnv->GetObjectClass ( pObject );
    jfieldID fid = pEnv->GetFieldID ( pClass, pFieldName, "J" );
    pEnv->SetLongField ( pObject, fid, value );
}

void SetFloatField ( JNIEnv *pEnv, jobject pObject, const char *pFieldName, float value )
{
    jclass pClass = pEnv->GetObjectClass ( pObject );
    jfieldID fid = pEnv->GetFieldID ( pClass, pFieldName, "F" );
    pEnv->SetFloatField ( pObject, fid, value );
}

void SetDoubleField ( JNIEnv *pEnv, jobject pObject, const char *pFieldName, double value )
{
    jclass pClass = pEnv->GetObjectClass ( pObject );
    jfieldID fid = pEnv->GetFieldID ( pClass, pFieldName, "D" );
    pEnv->SetDoubleField ( pObject, fid, value );
}

short GetShortField ( JNIEnv *pEnv, jobject pObject, const char *pFieldName )
{
    jclass pClass = pEnv->GetObjectClass ( pObject );
    jfieldID fid = pEnv->GetFieldID ( pClass, pFieldName, "S" );
    return pEnv->GetShortField ( pObject, fid );
}

int GetIntField ( JNIEnv *pEnv, jobject pObject, const char *pFieldName )
{
    jclass pClass = pEnv->GetObjectClass ( pObject );
    jfieldID fid = pEnv->GetFieldID ( pClass, pFieldName, "I" );
    return pEnv->GetIntField ( pObject, fid );
}

int64_t GetLongField ( JNIEnv *pEnv, jobject pObject, const char *pFieldName )
{
    jclass pClass = pEnv->GetObjectClass ( pObject );
    jfieldID fid = pEnv->GetFieldID ( pClass, pFieldName, "J" );
    return pEnv->GetLongField ( pObject, fid );
}

float GetFloatField ( JNIEnv *pEnv, jobject pObject, const char *pFieldName )
{
    jclass pClass = pEnv->GetObjectClass ( pObject );
    jfieldID fid = pEnv->GetFieldID ( pClass, pFieldName, "F" );
    return pEnv->GetFloatField ( pObject, fid );
}

double GetDoubleField ( JNIEnv *pEnv, jobject pObject, const char *pFieldName )
{
    jclass pClass = pEnv->GetObjectClass ( pObject );
    jfieldID fid = pEnv->GetFieldID ( pClass, pFieldName, "D" );
    return pEnv->GetDoubleField ( pObject, fid );
}
