#include "com_southernstars_sscore_JSSCoordinates.h"
#include "JNIUtilities.h"
#include "SSCoordinates.hpp"

/*
 * Class:     com_southernstars_sscore_JSSCoordinates
 * Method:    create
 * Signature: (Lcom/southernstars/sscore/JSSTime;Lcom/southernstars/sscore/JSSSpherical;)V
 */

JNIEXPORT void JNICALL Java_com_southernstars_sscore_JSSCoordinates_create ( JNIEnv *pEnv, jobject pJCoords, jobject pJTime, jobject pJLocation )
{
    SSTime time = JSSTimeToSSTime ( pEnv, pJTime );
    SSSpherical loc = JSSSphericalToSSSpherical ( pEnv, pJLocation );
    SSCoordinates *pCoords = new SSCoordinates ( time, loc );
    SetLongField ( pEnv, pJCoords, "pCoords", (long) pCoords );
}

/*
 * Class:     com_southernstars_sscore_JSSCoordinates
 * Method:    destroy
 * Signature: ()V
 */

JNIEXPORT void JNICALL Java_com_southernstars_sscore_JSSCoordinates_destroy ( JNIEnv *pEnv, jobject pJCoords )
{
    SSCoordinates *pCoords = (SSCoordinates *) GetLongField ( pEnv, pJCoords, "pCoords" );
    SetLongField ( pEnv, pJCoords, "pCoords", 0 );
    delete pCoords;
}

/*
 * Class:     com_southernstars_sscore_JSSCoordinates
 * Method:    setTime
 * Signature: (Lcom/southernstars/sscore/JSSTime;)V
 */

JNIEXPORT void JNICALL Java_com_southernstars_sscore_JSSCoordinates_setTime ( JNIEnv *pEnv, jobject pJCoords, jobject pJTime )
{
    SSTime time = JSSTimeToSSTime ( pEnv, pJTime );
    SSCoordinates *pCoords = (SSCoordinates *) GetLongField ( pEnv, pJCoords, "pCoords" );
    if ( pCoords ) pCoords->setTime ( time );
}

/*
 * Class:     com_southernstars_sscore_JSSCoordinates
 * Method:    setLocation
 * Signature: (Lcom/southernstars/sscore/JSSSpherical;)V
 */

JNIEXPORT void JNICALL Java_com_southernstars_sscore_JSSCoordinates_setLocation ( JNIEnv *pEnv, jobject pJCoords, jobject pJLocation )
{
    SSSpherical loc = JSSSphericalToSSSpherical ( pEnv, pJLocation );
    SSCoordinates *pCoords = (SSCoordinates *) GetLongField ( pEnv, pJCoords, "pCoords" );
    if ( pCoords ) pCoords->setLocation ( loc );
}

/*
 * Class:     com_southernstars_sscore_JSSCoordinates
 * Method:    getTime
 * Signature: ()Lcom/southernstars/sscore/JSSTime;
 */

JNIEXPORT jobject JNICALL Java_com_southernstars_sscore_JSSCoordinates_getTime ( JNIEnv *pEnv, jobject pJCoords )
{
    SSCoordinates *pCoords = (SSCoordinates *) GetLongField ( pEnv, pJCoords, "pCoords" );
    return pCoords ? SSTimeToJSSTime ( pEnv, pCoords->getTime() ) : nullptr;
}

/*
 * Class:     com_southernstars_sscore_JSSCoordinates
 * Method:    getLocation
 * Signature: ()Lcom/southernstars/sscore/JSSSpherical;
 */

JNIEXPORT jobject JNICALL Java_com_southernstars_sscore_JSSCoordinates_getLocation ( JNIEnv *pEnv, jobject pJCoords )
{
    SSCoordinates *pCoords = (SSCoordinates *) GetLongField ( pEnv, pJCoords, "pCoords" );
    return pCoords ? SSSphericalToJSSSpherical ( pEnv, pCoords->getLocation() ) : nullptr;
}

/*
 * Class:     com_southernstars_sscore_JSSCoordinates
 * Method:    getObserverPosition
 * Signature: ()Lcom/southernstars/sscore/JSSVector;
 */

JNIEXPORT jobject JNICALL Java_com_southernstars_sscore_JSSCoordinates_getObserverPosition ( JNIEnv *pEnv, jobject pJCoords )
{
    SSCoordinates *pCoords = (SSCoordinates *) GetLongField ( pEnv, pJCoords, "pCoords" );
    return pCoords ? SSVectorToJSSVector ( pEnv, pCoords->getObserverPosition() ) : nullptr;
}

/*
 * Class:     com_southernstars_sscore_JSSCoordinates
 * Method:    getObserverVelocity
 * Signature: ()Lcom/southernstars/sscore/JSSVector;
 */

JNIEXPORT jobject JNICALL Java_com_southernstars_sscore_JSSCoordinates_getObserverVelocity ( JNIEnv *pEnv, jobject pJCoords )
{
    SSCoordinates *pCoords = (SSCoordinates *) GetLongField ( pEnv, pJCoords, "pCoords" );
    return pCoords ? SSVectorToJSSVector ( pEnv, pCoords->getObserverVelocity() ) : nullptr;
}

/*
 * Class:     com_southernstars_sscore_JSSCoordinates
 * Method:    getStarParallax
 * Signature: ()Z
 */

JNIEXPORT jboolean JNICALL Java_com_southernstars_sscore_JSSCoordinates_getStarParallax ( JNIEnv *pEnv, jobject pJCoords )
{
    SSCoordinates *pCoords = (SSCoordinates *) GetLongField ( pEnv, pJCoords, "pCoords" );
    return pCoords ? pCoords->getStarParallax() : false;
}

/*
 * Class:     com_southernstars_sscore_JSSCoordinates
 * Method:    getStarMotion
 * Signature: ()Z
 */

JNIEXPORT jboolean JNICALL Java_com_southernstars_sscore_JSSCoordinates_getStarMotion ( JNIEnv *pEnv, jobject pJCoords )
{
    SSCoordinates *pCoords = (SSCoordinates *) GetLongField ( pEnv, pJCoords, "pCoords" );
    return pCoords ? pCoords->getStarMotion() : false;
}

/*
 * Class:     com_southernstars_sscore_JSSCoordinates
 * Method:    getAberration
 * Signature: ()Z
 */

JNIEXPORT jboolean JNICALL Java_com_southernstars_sscore_JSSCoordinates_getAberration ( JNIEnv *pEnv, jobject pJCoords )
{
    SSCoordinates *pCoords = (SSCoordinates *) GetLongField ( pEnv, pJCoords, "pCoords" );
    return pCoords ? pCoords->getAberration() : false;
}

/*
 * Class:     com_southernstars_sscore_JSSCoordinates
 * Method:    getLightTime
 * Signature: ()Z
 */

JNIEXPORT jboolean JNICALL Java_com_southernstars_sscore_JSSCoordinates_getLightTime ( JNIEnv *pEnv, jobject pJCoords )
{
    SSCoordinates *pCoords = (SSCoordinates *) GetLongField ( pEnv, pJCoords, "pCoords" );
    return pCoords ? pCoords->getLightTime() : false;
}

/*
 * Class:     com_southernstars_sscore_JSSCoordinates
 * Method:    setStarParallax
 * Signature: (Z)V
 */

JNIEXPORT void JNICALL Java_com_southernstars_sscore_JSSCoordinates_setStarParallax ( JNIEnv *pEnv, jobject pJCoords, jboolean parallax )
{
    SSCoordinates *pCoords = (SSCoordinates *) GetLongField ( pEnv, pJCoords, "pCoords" );
    if ( pCoords )
        pCoords->setStarParallax ( parallax );
}

/*
 * Class:     com_southernstars_sscore_JSSCoordinates
 * Method:    setStarMotion
 * Signature: (Z)V
 */

JNIEXPORT void JNICALL Java_com_southernstars_sscore_JSSCoordinates_setStarMotion ( JNIEnv *pEnv, jobject pJCoords, jboolean motion )
{
    SSCoordinates *pCoords = (SSCoordinates *) GetLongField ( pEnv, pJCoords, "pCoords" );
    if ( pCoords )
        pCoords->setStarMotion ( motion );
}

/*
 * Class:     com_southernstars_sscore_JSSCoordinates
 * Method:    setAberration
 * Signature: (Z)V
 */

JNIEXPORT void JNICALL Java_com_southernstars_sscore_JSSCoordinates_setAberration ( JNIEnv *pEnv, jobject pJCoords, jboolean aberration )
{
    SSCoordinates *pCoords = (SSCoordinates *) GetLongField ( pEnv, pJCoords, "pCoords" );
    if ( pCoords )
        pCoords->setAberration ( aberration );
}

/*
 * Class:     com_southernstars_sscore_JSSCoordinates
 * Method:    setLightTime
 * Signature: (Z)V
 */

JNIEXPORT void JNICALL Java_com_southernstars_sscore_JSSCoordinates_setLightTime ( JNIEnv *pEnv, jobject pJCoords, jboolean lighttime )
{
    SSCoordinates *pCoords = (SSCoordinates *) GetLongField ( pEnv, pJCoords, "pCoords" );
    if ( pCoords )
        pCoords->setLightTime ( lighttime );
}

/*
 * Class:     com_southernstars_sscore_JSSCoordinates
 * Method:    getJED
 * Signature: ()D
 */

JNIEXPORT jdouble JNICALL Java_com_southernstars_sscore_JSSCoordinates_getJED ( JNIEnv *pEnv, jobject pJCoords )
{
    SSCoordinates *pCoords = (SSCoordinates *) GetLongField ( pEnv, pJCoords, "pCoords" );
    return pCoords ? pCoords->getJED() : HUGE_VAL;
}

/*
 * Class:     com_southernstars_sscore_JSSCoordinates
 * Method:    getLST
 * Signature: ()D
 */

JNIEXPORT jdouble JNICALL Java_com_southernstars_sscore_JSSCoordinates_getLST ( JNIEnv *pEnv, jobject pJCoords )
{
    SSCoordinates *pCoords = (SSCoordinates *) GetLongField ( pEnv, pJCoords, "pCoords" );
    return pCoords ? pCoords->getLST() : HUGE_VAL;
}

/*
 * Class:     com_southernstars_sscore_JSSCoordinates
 * Method:    getObliquity
 * Signature: (D)D
 */

JNIEXPORT jdouble JNICALL Java_com_southernstars_sscore_JSSCoordinates_getObliquity ( JNIEnv *pEnv, jclass pJCoords, jdouble jd )
{
    return SSCoordinates::getObliquity ( jd );
}

/*
 * Class:     com_southernstars_sscore_JSSCoordinates
 * Method:    getNutationConstants
 * Signature: (DLcom/southernstars/sscore/JSSDouble;Lcom/southernstars/sscore/JSSDouble;)V
 */

JNIEXPORT void JNICALL Java_com_southernstars_sscore_JSSCoordinates_getNutationConstants ( JNIEnv *pEnv, jclass pJCoords, jdouble jd, jobject pJde, jobject pJdl )
{
    double dl = 0.0, de = 0.0;
    SSCoordinates::getNutationConstants ( jd, de, dl );
    SetDoubleField ( pEnv, pJde, "value", de );
    SetDoubleField ( pEnv, pJdl, "value", dl );
}

/*
 * Class:     com_southernstars_sscore_JSSCoordinates
 * Method:    getPrecessionConstants
 * Signature: (DLcom/southernstars/sscore/JSSDouble;Lcom/southernstars/sscore/JSSDouble;Lcom/southernstars/sscore/JSSDouble;)V
 */

JNIEXPORT void JNICALL Java_com_southernstars_sscore_JSSCoordinates_getPrecessionConstants ( JNIEnv *pEnv, jclass pJCoords, jdouble jd, jobject pJzeta, jobject pJz, jobject pJtheta )
{
    double zeta = 0.0, z = 0.0, theta = 0.0;
    SSCoordinates::getPrecessionConstants ( jd, zeta, z, theta );
    SetDoubleField ( pEnv, pJzeta, "value", zeta );
    SetDoubleField ( pEnv, pJz, "value", z );
    SetDoubleField ( pEnv, pJtheta, "value", theta );
}

/*
 * Class:     com_southernstars_sscore_JSSCoordinates
 * Method:    getPrecessionMatrix
 * Signature: (D)Lcom/southernstars/sscore/JSSMatrix;
 */

JNIEXPORT jobject JNICALL Java_com_southernstars_sscore_JSSCoordinates_getPrecessionMatrix ( JNIEnv *pEnv, jclass pJCoords, jdouble jd )
{
    SSMatrix matrix = SSCoordinates::getPrecessionMatrix ( jd );
    return SSMatrixToJSSMatrix ( pEnv, matrix );
}

/*
 * Class:     com_southernstars_sscore_JSSCoordinates
 * Method:    getNutationMatrix
 * Signature: (DDD)Lcom/southernstars/sscore/JSSMatrix;
 */

JNIEXPORT jobject JNICALL Java_com_southernstars_sscore_JSSCoordinates_getNutationMatrix ( JNIEnv *pEnv, jclass pJCoords, jdouble obq, jdouble dl, jdouble de )
{
    SSMatrix matrix = SSCoordinates::getNutationMatrix ( obq, dl, de );
    return SSMatrixToJSSMatrix ( pEnv, matrix );
}

/*
 * Class:     com_southernstars_sscore_JSSCoordinates
 * Method:    getEclipticMatrix
 * Signature: (D)Lcom/southernstars/sscore/JSSMatrix;
 */

JNIEXPORT jobject JNICALL Java_com_southernstars_sscore_JSSCoordinates_getEclipticMatrix ( JNIEnv *pEnv, jclass pJCoords, jdouble obq )
{
    SSMatrix matrix = SSCoordinates::getEclipticMatrix ( obq );
    return SSMatrixToJSSMatrix ( pEnv, matrix );
}

/*
 * Class:     com_southernstars_sscore_JSSCoordinates
 * Method:    getHorizonMatrix
 * Signature: (DD)Lcom/southernstars/sscore/JSSMatrix;
 */

JNIEXPORT jobject JNICALL Java_com_southernstars_sscore_JSSCoordinates_getHorizonMatrix ( JNIEnv *pEnv, jclass pJCoords, jdouble lst, jdouble lat )
{
    SSMatrix matrix = SSCoordinates::getHorizonMatrix ( lst, lat );
    return SSMatrixToJSSMatrix ( pEnv, matrix );
}

/*
 * Class:     com_southernstars_sscore_JSSCoordinates
 * Method:    getGalacticMatrix
 * Signature: ()Lcom/southernstars/sscore/JSSMatrix;
 */

JNIEXPORT jobject JNICALL Java_com_southernstars_sscore_JSSCoordinates_getGalacticMatrix ( JNIEnv *pEnv, jclass pJCoords )
{
    SSMatrix matrix = SSCoordinates::getGalacticMatrix();
    return SSMatrixToJSSMatrix ( pEnv, matrix );
}

/*
 * Class:     com_southernstars_sscore_JSSCoordinates
 * Method:    transform
 * Signature: (IILcom/southernstars/sscore/JSSVector;)Lcom/southernstars/sscore/JSSVector;
 */

JNIEXPORT jobject JNICALL Java_com_southernstars_sscore_JSSCoordinates_transform__IILcom_southernstars_sscore_JSSVector_2 ( JNIEnv *pEnv, jobject pJCoords, jint from, jint to, jobject pJVec )
{
    SSVector vec = JSSVectorToSSVector ( pEnv, pJVec );
    SSCoordinates *pCoords = (SSCoordinates *) GetLongField ( pEnv, pJCoords, "pCoords" );
    if ( pCoords ) vec = pCoords->transform ( (SSFrame) from, (SSFrame) to, vec );
    return SSVectorToJSSVector ( pEnv, vec );
}

/*
 * Class:     com_southernstars_sscore_JSSCoordinates
 * Method:    transform
 * Signature: (IILcom/southernstars/sscore/JSSSpherical;)Lcom/southernstars/sscore/JSSSpherical;
 */

JNIEXPORT jobject JNICALL Java_com_southernstars_sscore_JSSCoordinates_transform__IILcom_southernstars_sscore_JSSSpherical_2 ( JNIEnv *pEnv, jobject pJCoords, jint from, jint to, jobject pJSph )
{
    SSSpherical sph = JSSSphericalToSSSpherical ( pEnv, pJSph );
    SSCoordinates *pCoords = (SSCoordinates *) GetLongField ( pEnv, pJCoords, "pCoords" );
    if ( pCoords ) sph = pCoords->transform ( (SSFrame) from, (SSFrame) to, sph );
    return SSSphericalToJSSSpherical ( pEnv, sph );
}

/*
 * Class:     com_southernstars_sscore_JSSCoordinates
 * Method:    applyAberration
 * Signature: (Lcom/southernstars/sscore/JSSVector;)Lcom/southernstars/sscore/JSSVector;
 */

JNIEXPORT jobject JNICALL Java_com_southernstars_sscore_JSSCoordinates_applyAberration ( JNIEnv *pEnv, jobject pJCoords, jobject pJVec )
{
    SSVector vec = JSSVectorToSSVector ( pEnv, pJVec );
    SSCoordinates *pCoords = (SSCoordinates *) GetLongField ( pEnv, pJCoords, "pCoords" );
    if ( pCoords ) vec = pCoords->applyAberration ( vec );
    return SSVectorToJSSVector ( pEnv, vec );
}

/*
 * Class:     com_southernstars_sscore_JSSCoordinates
 * Method:    removeAberration
 * Signature: (Lcom/southernstars/sscore/JSSVector;)Lcom/southernstars/sscore/JSSVector;
 */

JNIEXPORT jobject JNICALL Java_com_southernstars_sscore_JSSCoordinates_removeAberration ( JNIEnv *pEnv, jobject pJCoords, jobject pJVec )
{
    SSVector vec = JSSVectorToSSVector ( pEnv, pJVec );
    SSCoordinates *pCoords = (SSCoordinates *) GetLongField ( pEnv, pJCoords, "pCoords" );
    if ( pCoords ) vec = pCoords->removeAberration ( vec );
    return SSVectorToJSSVector ( pEnv, vec );
}

/*
 * Class:     com_southernstars_sscore_JSSCoordinates
 * Method:    redShiftToRadVel
 * Signature: (D)D
 */

JNIEXPORT jdouble JNICALL Java_com_southernstars_sscore_JSSCoordinates_redShiftToRadVel ( JNIEnv *pEnv, jclass pJCoords, jdouble z )
{
    return SSCoordinates::redShiftToRadVel ( z );
}

/*
 * Class:     com_southernstars_sscore_JSSCoordinates
 * Method:    radVelToRedShift
 * Signature: (D)D
 */

JNIEXPORT jdouble JNICALL Java_com_southernstars_sscore_JSSCoordinates_radVelToRedShift ( JNIEnv *pEnv, jclass pJCoords, jdouble rv )
{
    return SSCoordinates::radVelToRedShift ( rv );
}

/*
 * Class:     com_southernstars_sscore_JSSCoordinates
 * Method:    toGeocentric
 * Signature: (Lcom/southernstars/sscore/JSSSpherical;DD)Lcom/southernstars/sscore/JSSVector;
 */

JNIEXPORT jobject JNICALL Java_com_southernstars_sscore_JSSCoordinates_toGeocentric ( JNIEnv *pEnv, jclass pJCoords, jobject pJSph, jdouble re, jdouble f )
{
    SSSpherical sph = JSSSphericalToSSSpherical ( pEnv, pJSph );
    SSVector vec = SSCoordinates::toGeocentric ( sph, re, f );
    return SSVectorToJSSVector ( pEnv, vec );
}

/*
 * Class:     com_southernstars_sscore_JSSCoordinates
 * Method:    toGeodetic
 * Signature: (Lcom/southernstars/sscore/JSSVector;DD)Lcom/southernstars/sscore/JSSSpherical;
 */

JNIEXPORT jobject JNICALL Java_com_southernstars_sscore_JSSCoordinates_toGeodetic ( JNIEnv *pEnv, jclass pJCoords, jobject pJVec, jdouble re, jdouble f )
{
    SSVector vec = JSSVectorToSSVector ( pEnv, pJVec );
    SSSpherical sph = SSCoordinates::toGeodetic ( vec, re, f );
    return SSSphericalToJSSSpherical ( pEnv, sph );
}

/*
 * Class:     com_southernstars_sscore_JSSCoordinates
 * Method:    refractionAngle
 * Signature: (DZ)D
 */

JNIEXPORT jdouble JNICALL Java_com_southernstars_sscore_JSSCoordinates_refractionAngle ( JNIEnv *pEnv, jclass pJCoords, jdouble alt, jboolean a )
{
    return SSCoordinates::refractionAngle ( alt, a );
}

/*
 * Class:     com_southernstars_sscore_JSSCoordinates
 * Method:    applyRefraction
 * Signature: (D)D
 */

JNIEXPORT jdouble JNICALL Java_com_southernstars_sscore_JSSCoordinates_applyRefraction ( JNIEnv *pEnv, jclass pJCoords, jdouble alt )
{
    return SSCoordinates::applyRefraction ( alt );
}

/*
 * Class:     com_southernstars_sscore_JSSCoordinates
 * Method:    removeRefraction
 * Signature: (D)D
 */

JNIEXPORT jdouble JNICALL Java_com_southernstars_sscore_JSSCoordinates_removeRefraction ( JNIEnv *pEnv, jclass pJCoords, jdouble alt )
{
    return SSCoordinates::removeRefraction ( alt );
}
