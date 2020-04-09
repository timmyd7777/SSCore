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
