#include "com_southernstars_sscore_JSSRTS.h"
#include "JNIUtilities.h"
#include "SSEvent.hpp"

jobject SSRTSToJSSRTS ( JNIEnv *pEnv, const SSRTS &RTS )
{
    jobject pJRTS = CreateJObject ( pEnv, "com/southernstars/sscore/JSSRTS" );

    // Note the last semicolon in the string
    char const *pSignature = "Lcom/southernstars/sscore/JSSTime;";

    if ( pJRTS != nullptr )
    {
        SetObjectField ( pEnv, pJRTS, "time", pSignature, SSTimeToJSSTime( pEnv, RTS.time ));
        SetDoubleField ( pEnv, pJRTS, "azm", RTS.azm );
        SetDoubleField ( pEnv, pJRTS, "alt", RTS.alt );
    }

    return pJRTS;
}

SSRTS JSSRTSToSSRTS ( JNIEnv *pEnv, jobject pJRTS )
{
    SSRTS RTS;

    // Note the last semicolon in the string
    char const *pSignature = "Lcom/southernstars/sscore/JSSTime;";

    RTS.time = JSSTimeToSSTime( pEnv, GetObjectField ( pEnv, pJRTS, pSignature, "time" ));
    RTS.azm = GetDoubleField ( pEnv, pJRTS, "azm" );
    RTS.alt = GetDoubleField ( pEnv, pJRTS, "alt" );

    return RTS;
}
