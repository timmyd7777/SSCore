#include "com_southernstars_sscore_JSSTime.h"
#include "JNIUtilities.h"
#include "SSTime.hpp"

jobject SSTimeToJSSTime ( JNIEnv *pEnv, const SSTime &time )
{
    jobject pJSSTime = CreateJObject ( pEnv, "com/southernstars/sscore/JSSTime" );

    if ( pJSSTime != nullptr )
    {
        SetDoubleField ( pEnv, pJSSTime, "jd", time.jd );
        SetDoubleField ( pEnv, pJSSTime, "zone", time.zone );
        SetIntField ( pEnv, pJSSTime, "calendar", time.calendar );
    }

    return pJSSTime;
}

SSTime JSSTimeToSSTime ( JNIEnv *pEnv, jobject pJSSTime )
{
    double jd = GetDoubleField ( pEnv, pJSSTime, "jd" );
    double zone = GetDoubleField ( pEnv, pJSSTime, "zone" );
    int calendar = GetIntField ( pEnv, pJSSTime, "calendar" );

    return SSTime ( jd, zone, (SSCalendar) calendar );
}


/*
 * Class:     com_southernstars_sscore_JSSTime
 * Method:    fromCalendarDate
 * Signature: (Lcom/southernstars/sscore/JSSDate;)Lcom/southernstars/sscore/JSSTime;
 */

JNIEXPORT jobject JNICALL Java_com_southernstars_sscore_JSSTime_fromCalendarDate ( JNIEnv *pEnv, jclass pClass, jobject pJSSDate )
{
    SSTime time ( JSSDateToSSDate ( pEnv, pJSSDate ) );
    return SSTimeToJSSTime ( pEnv, time );
}

/*
 * Class:     com_southernstars_sscore_JSSTime
 * Method:    fromSystem
 * Signature: ()Lcom/southernstars/sscore/JSSTime;
 */

JNIEXPORT jobject JNICALL Java_com_southernstars_sscore_JSSTime_fromSystem ( JNIEnv *pEnv, jclass pClass )
{
    SSTime time = SSTime::fromSystem();
    return SSTimeToJSSTime ( pEnv, time );
}

/*
 * Class:     com_southernstars_sscore_JSSTime
 * Method:    toCalendarDate
 * Signature: ()Lcom/southernstars/sscore/JSSDate;
 */

JNIEXPORT jobject JNICALL Java_com_southernstars_sscore_JSSTime_toCalendarDate ( JNIEnv *pEnv, jobject pJSSTime )
{
    SSTime time = JSSTimeToSSTime ( pEnv, pJSSTime );
    SSDate date ( time );
    return SSDateToJSSDate( pEnv, date );
}

/*
 * Class:     com_southernstars_sscore_JSSTime
 * Method:    getWeekday
 * Signature: ()I
 */

JNIEXPORT jint JNICALL Java_com_southernstars_sscore_JSSTime_getWeekday ( JNIEnv *pEnv, jobject pJSSTime )
{
    SSTime time = JSSTimeToSSTime ( pEnv, pJSSTime );
    return time.getWeekday();
}

/*
 * Class:     com_southernstars_sscore_JSSTime
 * Method:    getJulianEphemerisDate
 * Signature: ()D
 */

JNIEXPORT jdouble JNICALL Java_com_southernstars_sscore_JSSTime_getJulianEphemerisDate ( JNIEnv *pEnv, jobject pJSSTime )
{
    SSTime time = JSSTimeToSSTime ( pEnv, pJSSTime );
    return time.getJulianEphemerisDate();
}

/*
 * Class:     com_southernstars_sscore_JSSTime
 * Method:    getDeltaT
 * Signature: ()D
 */

JNIEXPORT jdouble JNICALL Java_com_southernstars_sscore_JSSTime_getDeltaT ( JNIEnv *pEnv, jobject pJSSTime )
{
    SSTime time = JSSTimeToSSTime ( pEnv, pJSSTime );
    return time.getDeltaT();
}

/*
 * Class:     com_southernstars_sscore_JSSTime
 * Method:    getSiderealTime
 * Signature: (D)D
 */

JNIEXPORT jdouble JNICALL Java_com_southernstars_sscore_JSSTime_getSiderealTime ( JNIEnv *pEnv, jobject pJSSTime, jdouble lon )
{
    SSTime time = JSSTimeToSSTime ( pEnv, pJSSTime );
    return time.getSiderealTime ( lon );
}
