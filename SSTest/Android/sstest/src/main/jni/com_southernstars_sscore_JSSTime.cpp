#include "com_southernstars_sscore_JSSTime.h"
#include "SSTime.hpp"

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

void SetIntField ( JNIEnv *pEnv, jobject pObject, const char *pFieldName, int value )
{
    jclass pClass = pEnv->GetObjectClass ( pObject );
    jfieldID fid = pEnv->GetFieldID ( pClass, pFieldName, "I" );
    pEnv->SetIntField ( pObject, fid, value );
}

void SetDoubleField ( JNIEnv *pEnv, jobject pObject, const char *pFieldName, double value )
{
    jclass pClass = pEnv->GetObjectClass ( pObject );
    jfieldID fid = pEnv->GetFieldID ( pClass, pFieldName, "D" );
    pEnv->SetDoubleField ( pObject, fid, value );
}

double GetDoubleField ( JNIEnv *pEnv, jobject pObject, const char *pFieldName )
{
    jclass pClass = pEnv->GetObjectClass ( pObject );
    jfieldID fid = pEnv->GetFieldID ( pClass, pFieldName, "D" );
    return pEnv->GetDoubleField ( pObject, fid );
}

int GetIntField ( JNIEnv *pEnv, jobject pObject, const char *pFieldName )
{
    jclass pClass = pEnv->GetObjectClass ( pObject );
    jfieldID fid = pEnv->GetFieldID ( pClass, pFieldName, "I" );
    return pEnv->GetIntField ( pObject, fid );
}

/*
 * Class:     com_southernstars_sscore_JSSTime
 * Method:    fromSystem
 * Signature: ()Lcom/southernstars/sscore/JSSTime;
 */

JNIEXPORT jobject JNICALL Java_com_southernstars_sscore_JSSTime_fromSystem ( JNIEnv *pEnv, jclass pClass )
{
    SSTime time = SSTime::fromSystem();
    jobject pObject = CreateJObject ( pEnv, "com/southernstars/sscore/JSSTime" );

    if ( pObject != nullptr )
    {
        SetDoubleField ( pEnv, pObject, "jd", time.jd );
        SetDoubleField ( pEnv, pObject, "zone", time.zone );
        SetIntField ( pEnv, pObject, "calendar", time.calendar );
    }

    return ( pObject );
}

/*
 * Class:     com_southernstars_sscore_JSSTime
 * Method:    getJulianEphemerisDate
 * Signature: ()D
 */

JNIEXPORT jdouble JNICALL Java_com_southernstars_sscore_JSSTime_getJulianEphemerisDate ( JNIEnv *pEnv, jobject pObject )
{
    double jd = GetDoubleField ( pEnv, pObject, "jd" );
    double zone = GetDoubleField ( pEnv, pObject, "zone" );
    int calendar = GetIntField ( pEnv, pObject, "calendar" );

    SSTime time ( jd, zone, (SSCalendar) calendar );
    return time.getJulianEphemerisDate();
}
