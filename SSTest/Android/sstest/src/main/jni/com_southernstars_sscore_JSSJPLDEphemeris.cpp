#include "com_southernstars_sscore_JSSJPLDEphemeris.h"
#include "JNIUtilities.h"
#include "SSJPLDEphemeris.hpp"

/*
 * Class:     com_southernstars_sscore_JSSJPLDEphemeris
 * Method:    open
 * Signature: (Ljava/lang/String;)Z
 */

JNIEXPORT jboolean JNICALL Java_com_southernstars_sscore_JSSJPLDEphemeris_open ( JNIEnv *pEnv, jclass pJPLDEphem, jstring pJFileName )
{
    const char *pFileName = pEnv->GetStringUTFChars ( pJFileName, nullptr );
    bool result = SSJPLDEphemeris::open ( string ( pFileName ) );
    pEnv->ReleaseStringUTFChars ( pJFileName, pFileName );
    return result;
}

/*
 * Class:     com_southernstars_sscore_JSSJPLDEphemeris
 * Method:    isOpen
 * Signature: ()Z
 */

JNIEXPORT jboolean JNICALL Java_com_southernstars_sscore_JSSJPLDEphemeris_isOpen ( JNIEnv *pEnv, jclass pJPLDEphem )
{
    return SSJPLDEphemeris::isOpen();
}

/*
 * Class:     com_southernstars_sscore_JSSJPLDEphemeris
 * Method:    close
 * Signature: ()V
 */

JNIEXPORT void JNICALL Java_com_southernstars_sscore_JSSJPLDEphemeris_close  ( JNIEnv *pEnv, jclass pJPLDEphem )
{
    return SSJPLDEphemeris::close();
}


/*
 * Class:     com_southernstars_sscore_JSSJPLDEphemeris
 * Method:    getConstantNumber
 * Signature: ()I
 */

JNIEXPORT jint JNICALL Java_com_southernstars_sscore_JSSJPLDEphemeris_getConstantNumber ( JNIEnv *pEnv, jclass pJPLDEphem )
{
    return SSJPLDEphemeris::getConstantNumber();
}

/*
 * Class:     com_southernstars_sscore_JSSJPLDEphemeris
 * Method:    getConstantName
 * Signature: (I)Ljava/lang/String;
 */

JNIEXPORT jstring JNICALL Java_com_southernstars_sscore_JSSJPLDEphemeris_getConstantName ( JNIEnv *pEnv, jclass pJPLDEpheme, jint index )
{
    string name = SSJPLDEphemeris::getConstantName ( index );
    return pEnv->NewStringUTF ( name.c_str() );
}

/*
 * Class:     com_southernstars_sscore_JSSJPLDEphemeris
 * Method:    getConstantValue
 * Signature: (I)D
 */

JNIEXPORT jdouble JNICALL Java_com_southernstars_sscore_JSSJPLDEphemeris_getConstantValue ( JNIEnv *pEnv, jclass pJPLDEphem, jint index )
{
    return SSJPLDEphemeris::getConstantValue ( index );
}

/*
 * Class:     com_southernstars_sscore_JSSJPLDEphemeris
 * Method:    getStartJED
 * Signature: ()D
 */
JNIEXPORT jdouble JNICALL Java_com_southernstars_sscore_JSSJPLDEphemeris_getStartJED ( JNIEnv *pEnv, jclass pJPLDEphem )
{
    return SSJPLDEphemeris::getStartJED();
}

/*
 * Class:     com_southernstars_sscore_JSSJPLDEphemeris
 * Method:    getStopJED
 * Signature: ()D
 */

JNIEXPORT jdouble JNICALL Java_com_southernstars_sscore_JSSJPLDEphemeris_getStopJED ( JNIEnv *pEnv, jclass pJPLDEphem )
{
    return SSJPLDEphemeris::getStopJED();
}

/*
 * Class:     com_southernstars_sscore_JSSJPLDEphemeris
 * Method:    getStep
 * Signature: ()D
 */

JNIEXPORT jdouble JNICALL Java_com_southernstars_sscore_JSSJPLDEphemeris_getStep ( JNIEnv *pEnv, jclass pJPLDEphem )
{
    return SSJPLDEphemeris::getStep();
}


/*
 * Class:     com_southernstars_sscore_JSSJPLDEphemeris
 * Method:    compute
 * Signature: (IDZLcom/southernstars/sscore/JSSVector;Lcom/southernstars/sscore/JSSVector;)Z
 */

JNIEXPORT jboolean JNICALL Java_com_southernstars_sscore_JSSJPLDEphemeris_compute ( JNIEnv *pEnv, jclass pJPLDEphem, jint planet, jdouble jed, jboolean bary, jobject pJPosVec, jobject pJVelVec )
{
    SSVector pos, vel;
    bool result = SSJPLDEphemeris::compute ( planet, jed, bary, pos, vel );
    SSVectorToJSSVector ( pEnv, pos, pJPosVec );
    SSVectorToJSSVector ( pEnv, vel, pJVelVec );
    return result;
}
