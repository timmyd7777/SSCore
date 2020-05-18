#include "com_southernstars_sscore_JSSPass.h"
#include "JNIUtilities.h"
#include "SSEvent.hpp"

jobject SSPassToJSSPass ( JNIEnv *pEnv, const SSPass &pass )
{
    jobject pJPass = CreateJObject ( pEnv, "com/southernstars/sscore/JSSPass" );

    if ( pJPass != nullptr )
    {
        SetObjectField ( pEnv, pJPass, "rising", SSRTSToJSSRTS( pEnv, pass.rising ));
        SetObjectField ( pEnv, pJPass, "transit", SSRTSToJSSRTS( pEnv, pass.transit ));
        SetObjectField ( pEnv, pJPass, "setting", SSRTSToJSSRTS( pEnv, pass.setting ));
    }

    return pJPass;
}

SSPass JSSPassToSSPass ( JNIEnv *pEnv, jobject pJPass )
{
    SSRTS rising = JSSRTSToSSRTS( pEnv, GetObjectField ( pEnv, pJPass, "rising" ));
    SSRTS transit = JSSRTSToSSRTS( pEnv, GetObjectField ( pEnv, pJPass, "transit" ));
    SSRTS setting = JSSRTSToSSRTS( pEnv, GetObjectField ( pEnv, pJPass, "setting" ));

    SSPass pass;

    pass.rising = rising;
    pass.transit = transit;
    pass.setting = setting;

    return pass;
}
