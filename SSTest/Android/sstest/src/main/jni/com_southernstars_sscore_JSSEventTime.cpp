#include "com_southernstars_sscore_JSSEventTime.h"
#include "JNIUtilities.h"
#include "SSEvent.hpp"

jobject SSEventTimeToJSSEventTime ( JNIEnv *pEnv, const SSEventTime &eventtime )
{
    jobject pJEventTime = CreateJObject ( pEnv, "com/southernstars/sscore/JSSEventTime" );

    if ( pJEventTime != nullptr )
    {
        // SetObjectField ( pEnv, pJEventTime, "time", SSTimeToJSSTime( pEnv, eventtime.time ));
        SetDoubleField ( pEnv, pJEventTime, "jd", eventtime.time.jd );
        SetDoubleField ( pEnv, pJEventTime, "zone", eventtime.time.zone );

        SetDoubleField ( pEnv, pJEventTime, "value", eventtime.value );
    }

    return pJEventTime;
}

SSEventTime JSSEventTimeToSSEventTime ( JNIEnv *pEnv, jobject pJEventTime )
{
    SSEventTime eventtime;

    double jd = GetDoubleField ( pEnv, pJEventTime, "jd" );
    double zone = GetDoubleField ( pEnv, pJEventTime, "zone" );
    SSTime time = SSTime( jd, zone );

    eventtime.time = time;
    // eventtime.time = JSSTimeToSSTime( pEnv, GetObjectField ( pEnv, pJEventTime, "time" ));
    eventtime.value = GetDoubleField ( pEnv, pJEventTime, "value" );

    return eventtime;
}
