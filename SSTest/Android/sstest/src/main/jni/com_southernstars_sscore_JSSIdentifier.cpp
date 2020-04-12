#include "com_southernstars_sscore_JSSIdentifier.h"
#include "JNIUtilities.h"
#include "SSIdentifier.hpp"

/*
 * Class:     com_southernstars_sscore_JSSIdentifier
 * Method:    fromString
 * Signature: (Ljava/lang/String;)Lcom/southernstars/sscore/JSSIdentifier;
 */

JNIEXPORT jobject JNICALL Java_com_southernstars_sscore_JSSIdentifier_fromString ( JNIEnv *pEnv, jclass pClassJSSIdentifier, jstring pJString )
{
    const char *pCString = pEnv->GetStringUTFChars ( pJString, nullptr );
    SSIdentifier ident = SSIdentifier::fromString ( pCString );
    pEnv->ReleaseStringUTFChars ( pJString, pCString );
    jobject pJSSIdentifier = CreateJObject ( pEnv, "com/southernstars/sscore/JSSIdentifier" );
    if ( pJSSIdentifier )
        SetLongField ( pEnv, pJSSIdentifier, "id", ident );
    return pJSSIdentifier;
}

/*
 * Class:     com_southernstars_sscore_JSSIdentifier
 * Method:    toString
 * Signature: ()Ljava/lang/String;
 */

JNIEXPORT jstring JNICALL Java_com_southernstars_sscore_JSSIdentifier_toString ( JNIEnv *pEnv, jobject pJSSIdentifier )
{
    SSIdentifier ident ( GetLongField ( pEnv, pJSSIdentifier, "id" ) );
    return pEnv->NewStringUTF ( ident.toString().c_str() );
}
