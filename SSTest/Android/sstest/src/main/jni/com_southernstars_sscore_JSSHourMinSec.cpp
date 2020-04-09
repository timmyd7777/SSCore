#include "com_southernstars_sscore_JSSHourMinSec.h"
#include "JNIUtilities.h"
#include "SSAngle.hpp"

jobject SSHourMinSecToJSSHourMinSec ( JNIEnv *pEnv, SSHourMinSec &hms )
{
    jobject pJSSHourMinSec = CreateJObject ( pEnv, "com/southernstars/sscore/JSSHourMinSec" );

    if ( pJSSHourMinSec != nullptr )
    {
        SetCharField ( pEnv, pJSSHourMinSec, "sign", hms.sign );
        SetShortField ( pEnv, pJSSHourMinSec, "hour", hms.hour );
        SetShortField ( pEnv, pJSSHourMinSec, "min", hms.min );
        SetDoubleField ( pEnv, pJSSHourMinSec, "sec", hms.sec );
    }

    return pJSSHourMinSec;
}

SSHourMinSec JSSHourMinSecToSSHourMinSec ( JNIEnv *pEnv, jobject pJSSHourMinSec )
{
    char sign = GetCharField ( pEnv, pJSSHourMinSec, "sign" );
    short hour = GetShortField ( pEnv, pJSSHourMinSec, "hour" );
    short min = GetShortField ( pEnv, pJSSHourMinSec, "min" );
    double sec = GetDoubleField ( pEnv, pJSSHourMinSec, "sec" );

    return SSHourMinSec ( sign, hour, min, sec );
}

/*
 * Class:     com_southernstars_sscore_JSSHourMinSec
 * Method:    fromString
 * Signature: (Ljava/lang/String;)Lcom/southernstars/sscore/JSSHourMinSec;
 */

JNIEXPORT jobject JNICALL Java_com_southernstars_sscore_JSSHourMinSec_fromString ( JNIEnv *pEnv, jclass pClass, jstring pJString )
{
    const char *pCString = pEnv->GetStringUTFChars ( pJString, nullptr );
    SSHourMinSec hms ( pCString );
    pEnv->ReleaseStringUTFChars ( pJString, pCString );
    return SSHourMinSecToJSSHourMinSec ( pEnv, hms );
}

/*
 * Class:     com_southernstars_sscore_JSSHourMinSec
 * Method:    toString
 * Signature: ()Ljava/lang/String;
 */

JNIEXPORT jstring JNICALL Java_com_southernstars_sscore_JSSHourMinSec_toString ( JNIEnv *pEnv, jobject pJSSHourMinSec )
{
    SSHourMinSec hms = JSSHourMinSecToSSHourMinSec ( pEnv, pJSSHourMinSec );
    return pEnv->NewStringUTF ( hms.toString().c_str() );
}
