#include "com_southernstars_sscore_JSSDegMinSec.h"
#include "JNIUtilities.h"
#include "SSAngle.hpp"

jobject SSDegMinSecToJSSDegMinSec ( JNIEnv *pEnv, SSDegMinSec &dms )
{
    jobject pJSSDegMinSec = CreateJObject ( pEnv, "com/southernstars/sscore/JSSDegMinSec" );

    if ( pJSSDegMinSec != nullptr )
    {
        SetCharField ( pEnv, pJSSDegMinSec, "sign", dms.sign );
        SetShortField ( pEnv, pJSSDegMinSec, "deg", dms.deg );
        SetShortField ( pEnv, pJSSDegMinSec, "min", dms.min );
        SetDoubleField ( pEnv, pJSSDegMinSec, "sec", dms.sec );
    }

    return pJSSDegMinSec;
}

SSDegMinSec JSSDegMinSecToSSDegMinSec ( JNIEnv *pEnv, jobject pJSSDegMinSec )
{
    char sign = GetCharField ( pEnv, pJSSDegMinSec, "sign" );
    short deg = GetShortField ( pEnv, pJSSDegMinSec, "deg" );
    short min = GetShortField ( pEnv, pJSSDegMinSec, "min" );
    double sec = GetDoubleField ( pEnv, pJSSDegMinSec, "sec" );

    return SSDegMinSec ( sign, deg, min, sec );
}

/*
 * Class:     com_southernstars_sscore_JSSDegMinSec
 * Method:    fromString
 * Signature: (Ljava/lang/String;)Lcom/southernstars/sscore/JSSDegMinSec;
 */

JNIEXPORT jobject JNICALL Java_com_southernstars_sscore_JSSDegMinSec_fromString ( JNIEnv *pEnv, jclass pClass, jstring pJString )
{
    const char *pCString = pEnv->GetStringUTFChars ( pJString, nullptr );
    SSDegMinSec dms ( pCString );
    pEnv->ReleaseStringUTFChars ( pJString, pCString );
    return SSDegMinSecToJSSDegMinSec ( pEnv, dms );
}

/*
 * Class:     com_southernstars_sscore_JSSDegMinSec
 * Method:    toString
 * Signature: ()Ljava/lang/String;
 */

JNIEXPORT jstring JNICALL Java_com_southernstars_sscore_JSSDegMinSec_toString ( JNIEnv *pEnv, jobject pJSSDegMinSec )
{
    SSDegMinSec dms = JSSDegMinSecToSSDegMinSec ( pEnv, pJSSDegMinSec );
    return pEnv->NewStringUTF ( dms.toString().c_str() );
}
