#include "com_southernstars_sscore_JSSPass.h"
#include "JNIUtilities.h"
#include "SSEvent.hpp"

jobject SSPassToJSSPass ( JNIEnv *pEnv, const SSPass &pass )
{
    jobject pJPass = CreateJObject ( pEnv, "com/southernstars/sscore/JSSPass" );
    
    // Note the last semicolon in the string
    char const *pSignature = "Lcom/southernstars/sscore/JSSRTS;";

    if ( pJPass != nullptr )
    {
        SetObjectField ( pEnv, pJPass, "rising", pSignature, SSRTSToJSSRTS( pEnv, pass.rising ));
        SetObjectField ( pEnv, pJPass, "transit", pSignature, SSRTSToJSSRTS( pEnv, pass.transit ));
        SetObjectField ( pEnv, pJPass, "setting", pSignature, SSRTSToJSSRTS( pEnv, pass.setting ));
    }

    return pJPass;
}

SSPass JSSPassToSSPass ( JNIEnv *pEnv, jobject pJPass )
{
    // Note the last semicolon in the string
    char const *pSignature = "Lcom/southernstars/sscore/JSSRTS;";

    SSRTS rising = JSSRTSToSSRTS( pEnv, GetObjectField ( pEnv, pJPass, "rising", pSignature ));
    SSRTS transit = JSSRTSToSSRTS( pEnv, GetObjectField ( pEnv, pJPass, "transit", pSignature ));
    SSRTS setting = JSSRTSToSSRTS( pEnv, GetObjectField ( pEnv, pJPass, "setting", pSignature ));

    SSPass pass;

    pass.rising = rising;
    pass.transit = transit;
    pass.setting = setting;

    return pass;
}
