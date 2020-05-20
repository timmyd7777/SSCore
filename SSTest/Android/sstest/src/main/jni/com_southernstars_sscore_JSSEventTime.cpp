#include "com_southernstars_sscore_JSSEventTime.h"
#include "JNIUtilities.h"
#include "SSEvent.hpp"

jobject SSEventTimeToJSSEventTime ( JNIEnv *pEnv, const SSEventTime &eventtime )
{
    jobject pJEventTime = CreateJObject ( pEnv, "com/southernstars/sscore/JSSEventTime" );

    // Note the last semicolon in the string
    char const *pSignature = "Lcom/southernstars/sscore/JSSTime;";

    if ( pJEventTime != nullptr )
    {
        SetObjectField ( pEnv, pJEventTime, "time", pSignature, SSTimeToJSSTime( pEnv, eventtime.time ));
        SetDoubleField ( pEnv, pJEventTime, "value", eventtime.value );
    }

    return pJEventTime;
}

SSEventTime JSSEventTimeToSSEventTime ( JNIEnv *pEnv, jobject pJEventTime )
{
    SSEventTime eventtime;

    // Note the last semicolon in the string
    char const *pSignature = "Lcom/southernstars/sscore/JSSTime;";

    eventtime.time = JSSTimeToSSTime( pEnv, GetObjectField ( pEnv, pJEventTime, pSignature, "time" ));
    eventtime.value = GetDoubleField ( pEnv, pJEventTime, "value" );

    return eventtime;
}
