#include "com_southernstars_sscore_JSSRTS.h"
#include "JNIUtilities.h"
#include "SSEvent.hpp"

jobject SSRTSToJSSRTS ( JNIEnv *pEnv, const SSRTS &RTS )
{
    jobject pJRTS = CreateJObject ( pEnv, "com/southernstars/sscore/JSSRTS" );

    if ( pJRTS != nullptr )
    {
        // SetObjectField ( pEnv, pJRTS, "time", SSTimeToJSSTime( pEnv, RTS.time ));
        SetDoubleField ( pEnv, pJRTS, "jd", RTS.time.jd );
        SetDoubleField ( pEnv, pJRTS, "zone", RTS.time.zone );

        SetDoubleField ( pEnv, pJRTS, "azm", RTS.azm );
        SetDoubleField ( pEnv, pJRTS, "alt", RTS.alt );
    }

    return pJRTS;
}

SSRTS JSSRTSToSSRTS ( JNIEnv *pEnv, jobject pJRTS )
{
    SSRTS RTS;

    double jd = GetDoubleField ( pEnv, pJRTS, "jd" );
    double zone = GetDoubleField ( pEnv, pJRTS, "zone" );
    SSTime time = SSTime ( jd, zone );

    RTS.time = time;
    // RTS.time = JSSTimeToSSTime( pEnv, GetObjectField ( pEnv, pJRTS, "time" ));
    RTS.azm = GetDoubleField ( pEnv, pJRTS, "azm" );
    RTS.alt = GetDoubleField ( pEnv, pJRTS, "alt" );

    return RTS;
}
