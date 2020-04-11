#include "com_southernstars_sscore_JSSVector.h"
#include "JNIUtilities.h"
#include "SSVector.hpp"

jobject SSVectorToJSSVector ( JNIEnv *pEnv, SSVector &vector )
{
    jobject pJSSVector = CreateJObject ( pEnv, "com/southernstars/sscore/JSSVector" );

    if ( pJSSVector != nullptr )
    {
        SetDoubleField ( pEnv, pJSSVector, "x", vector.x );
        SetDoubleField ( pEnv, pJSSVector, "y", vector.y );
        SetDoubleField ( pEnv, pJSSVector, "z", vector.z );
    }

    return pJSSVector;
}

SSVector JSSVectorToSSVector ( JNIEnv *pEnv, jobject pJSSVector )
{
    double x = GetDoubleField ( pEnv, pJSSVector, "x" );
    double y = GetDoubleField ( pEnv, pJSSVector, "y" );
    double z = GetDoubleField ( pEnv, pJSSVector, "z" );

    return SSVector ( x, y, z );
}

/*
 * Class:     com_southernstars_sscore_JSSVector
 * Method:    fromSpherical
 * Signature: (Lcom/southernstars/sscore/JSSSpherical;)Lcom/southernstars/sscore/JSSVector;
 */

JNIEXPORT jobject JNICALL Java_com_southernstars_sscore_JSSVector_fromSpherical ( JNIEnv *pEnv, jclass pClass, jobject pJSSSpherical )
{
    SSVector vec ( JSSSphericalToSSSpherical ( pEnv, pJSSSpherical ) );
    return SSVectorToJSSVector ( pEnv, vec );
}

/*
 * Class:     com_southernstars_sscore_JSSVector
 * Method:    toSpherical
 * Signature: ()Lcom/southernstars/sscore/JSSSpherical;
 */

JNIEXPORT jobject JNICALL Java_com_southernstars_sscore_JSSVector_toSpherical  ( JNIEnv *pEnv, jobject pJSSVector )
{
    SSVector vec = JSSVectorToSSVector ( pEnv, pJSSVector );
    SSSpherical sph ( vec );
    return SSSphericalToJSSSpherical ( pEnv, sph );
}

/*
 * Class:     com_southernstars_sscore_JSSVector
 * Method:    toSphericalVelocity
 * Signature: (Lcom/southernstars/sscore/JSSVector;)Lcom/southernstars/sscore/JSSSpherical;
 */

JNIEXPORT jobject JNICALL Java_com_southernstars_sscore_JSSVector_toSphericalVelocity ( JNIEnv *pEnv, jobject pJSSPosVec, jobject pJSSVelVec )
{
    SSVector pos = JSSVectorToSSVector ( pEnv, pJSSPosVec );
    SSVector vel = JSSVectorToSSVector ( pEnv, pJSSVelVec );
    SSSpherical sph = pos.toSphericalVelocity ( vel );
    return SSSphericalToJSSSpherical ( pEnv, sph );
}

/*
 * Class:     com_southernstars_sscore_JSSVector
 * Method:    angularSeparation
 * Signature: (Lcom/southernstars/sscore/JSSVector;)D
 */

JNIEXPORT jdouble JNICALL Java_com_southernstars_sscore_JSSVector_angularSeparation ( JNIEnv *pEnv, jobject pJSSThisVec, jobject pJSSThatVec )
{
    SSVector thisVec = JSSVectorToSSVector ( pEnv, pJSSThisVec );
    SSVector thatVec = JSSVectorToSSVector ( pEnv, pJSSThatVec );
    return thisVec.angularSeparation ( thatVec );
}

/*
 * Class:     com_southernstars_sscore_JSSVector
 * Method:    positionAngle
 * Signature: (Lcom/southernstars/sscore/JSSVector;)D
 */

JNIEXPORT jdouble JNICALL Java_com_southernstars_sscore_JSSVector_positionAngle ( JNIEnv *pEnv, jobject pJSSThisVec, jobject pJSSThatVec )
{
    SSVector thisVec = JSSVectorToSSVector ( pEnv, pJSSThisVec );
    SSVector thatVec = JSSVectorToSSVector ( pEnv, pJSSThatVec );

    return thisVec.positionAngle ( thatVec );
}
