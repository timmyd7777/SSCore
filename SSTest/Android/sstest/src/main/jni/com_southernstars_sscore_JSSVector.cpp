#include "com_southernstars_sscore_JSSVector.h"
#include "JNIUtilities.h"
#include "SSVector.hpp"

/*
 * Class:     com_southernstars_sscore_JSSVector
 * Method:    fromSpherical
 * Signature: (Lcom/southernstars/sscore/JSSSpherical;)Lcom/southernstars/sscore/JSSVector;
 */

JNIEXPORT jobject JNICALL Java_com_southernstars_sscore_JSSVector_fromSpherical ( JNIEnv *pEnv, jclass pClass, jobject pJSSSpherical )
{

}

/*
 * Class:     com_southernstars_sscore_JSSVector
 * Method:    toSpherical
 * Signature: ()Lcom/southernstars/sscore/JSSSpherical;
 */
JNIEXPORT jobject JNICALL Java_com_southernstars_sscore_JSSVector_toSpherical  ( JNIEnv *pEnv, jobject pJSSVector )
{

}

/*
 * Class:     com_southernstars_sscore_JSSVector
 * Method:    toSphericalVelocity
 * Signature: (Lcom/southernstars/sscore/JSSVector;)Lcom/southernstars/sscore/JSSSpherical;
 */

JNIEXPORT jobject JNICALL Java_com_southernstars_sscore_JSSVector_toSphericalVelocity ( JNIEnv *pEnv, jobject pJSSPosVector, jobject pJSSVelVector )
{

}

/*
 * Class:     com_southernstars_sscore_JSSVector
 * Method:    angularSeparation
 * Signature: (Lcom/southernstars/sscore/JSSVector;)D
 */

JNIEXPORT jdouble JNICALL Java_com_southernstars_sscore_JSSVector_angularSeparation ( JNIEnv *pEnv, jobject pJSSThisVector, jobject pJSSThatVector )
{

}

/*
 * Class:     com_southernstars_sscore_JSSVector
 * Method:    positionAngle
 * Signature: (Lcom/southernstars/sscore/JSSVector;)D
 */
JNIEXPORT jdouble JNICALL Java_com_southernstars_sscore_JSSVector_positionAngle ( JNIEnv *pEnv, jobject pJSSThisVector, jobject pJSSThatVector )
{

}
