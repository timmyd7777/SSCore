#include "com_southernstars_sscore_JSSDate.h"
#include "JNIUtilities.h"
#include "SSTime.hpp"

jobject SSDateToJSSDate ( JNIEnv *pEnv, const SSDate &date )
{
    jobject pJSSDate = CreateJObject ( pEnv, "com/southernstars/sscore/JSSDate" );

    if ( pJSSDate != nullptr )
    {
        SetIntField ( pEnv, pJSSDate, "calendar", date.calendar );
        SetDoubleField ( pEnv, pJSSDate, "zone", date.zone );
        SetIntField ( pEnv, pJSSDate, "year", date.year );
        SetShortField ( pEnv, pJSSDate, "month", date.month );
        SetDoubleField ( pEnv, pJSSDate, "day", date.day );
        SetShortField ( pEnv, pJSSDate, "hour", date.hour );
        SetShortField ( pEnv, pJSSDate, "min", date.min );
        SetDoubleField ( pEnv, pJSSDate, "sec", date.sec );
    }

    return pJSSDate;
}

SSDate JSSDateToSSDate ( JNIEnv *pEnv, jobject pJSSDate )
{
    int calendar = GetIntField ( pEnv, pJSSDate, "calendar" );
    double zone = GetDoubleField ( pEnv, pJSSDate, "zone" );
    int year = GetIntField ( pEnv, pJSSDate, "year" );
    short month = GetShortField ( pEnv, pJSSDate, "month" );
    double day = GetDoubleField ( pEnv, pJSSDate, "day" );
    short hour = GetShortField ( pEnv, pJSSDate, "hour" );
    short min = GetShortField ( pEnv, pJSSDate, "min" );
    double sec = GetDoubleField ( pEnv, pJSSDate, "sec" );

    return SSDate ( (SSCalendar) calendar, zone, year, month, day, hour, min, sec );
}

/*
 * Class:     com_southernstars_sscore_JSSDate
 * Method:    fromJulianDate
 * Signature: (Lcom/southernstars/sscore/JSSTime;)Lcom/southernstars/sscore/JSSDate;
 */

JNIEXPORT jobject JNICALL Java_com_southernstars_sscore_JSSDate_fromJulianDate ( JNIEnv *pEnv, jclass pClass, jobject pJSSTime )
{
    SSTime time = JSSTimeToSSTime ( pEnv, pJSSTime );
    SSDate date = SSDate::fromJulianDate ( time );
    return SSDateToJSSDate ( pEnv, date );
}

/*
 * Class:     com_southernstars_sscore_JSSDate
 * Method:    toJulianDate
 * Signature: ()Lcom/southernstars/sscore/JSSTime;
 */

JNIEXPORT jobject JNICALL Java_com_southernstars_sscore_JSSDate_toJulianDate ( JNIEnv *pEnv, jobject pJSSDate )
{
    SSDate date = JSSDateToSSDate ( pEnv, pJSSDate );
    SSTime time = date.toJulianDate();
    return SSTimeToJSSTime ( pEnv, time );
}
