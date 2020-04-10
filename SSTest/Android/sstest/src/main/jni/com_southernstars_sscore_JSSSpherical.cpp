#include "com_southernstars_sscore_JSSSpherical.h"
#include "JNIUtilities.h"
#include "SSVector.hpp"

jobject SSSphericalToJSSSpherical ( JNIEnv *pEnv, SSSpherical &spherical )
{
    jobject pJSSSpherical = CreateJObject ( pEnv, "com/southernstars/sscore/JSSSpherical" );

    if ( pJSSSpherical != nullptr )
    {
        SetDoubleField ( pEnv, pJSSSpherical, "lon", spherical.lon );
        SetDoubleField ( pEnv, pJSSSpherical, "lat", spherical.lat );
        SetDoubleField ( pEnv, pJSSSpherical, "rad", spherical.rad );
    }

    return pJSSSpherical;
}

SSSpherical JSSSphericalToSSSpherical ( JNIEnv *pEnv, jobject pJSSSpherical )
{
    double lon = GetDoubleField ( pEnv, pJSSSpherical, "lon" );
    double lat = GetDoubleField ( pEnv, pJSSSpherical, "lat" );
    double rad = GetDoubleField ( pEnv, pJSSSpherical, "rad" );

    return SSSpherical ( lon, lat, rad );
}

/*
 * Class:     com_southernstars_sscore_JSSSpherical
 * Method:    fromVector
 * Signature: (Lcom/southernstars/sscore/JSSVector;)Lcom/southernstars/sscore/JSSSpherical;
 */

JNIEXPORT jobject JNICALL Java_com_southernstars_sscore_JSSSpherical_fromVector ( JNIEnv *pEnv, jclass pClass, jobject pJSSVector )
{
    SSSpherical sph ( JSSVectorToSSVector ( pEnv, pJSSVector ) );
    return SSSphericalToJSSSpherical ( pEnv, sph );
}

/*
 * Class:     com_southernstars_sscore_JSSSpherical
 * Method:    toVector
 * Signature: ()Lcom/southernstars/sscore/JSSVector;
 */

JNIEXPORT jobject JNICALL Java_com_southernstars_sscore_JSSSpherical_toVector ( JNIEnv *pEnv, jobject pJSSSpherical )
{
    SSVector vec ( JSSSphericalToSSSpherical ( pEnv, pJSSSpherical ) );
    return SSVectorToJSSVector ( pEnv, vec );
}


/*
 * Class:     com_southernstars_sscore_JSSSpherical
 * Method:    toVectorVelocity
 * Signature: (Lcom/southernstars/sscore/JSSSpherical;)Lcom/southernstars/sscore/JSSVector;
 */

JNIEXPORT jobject JNICALL Java_com_southernstars_sscore_JSSSpherical_toVectorVelocity ( JNIEnv *pEnv, jobject pJCoords, jobject pJMotion )
{
    SSSpherical coords = JSSSphericalToSSSpherical ( pEnv, pJCoords );
    SSSpherical motion = JSSSphericalToSSSpherical ( pEnv, pJMotion );
    SSVector vel = coords.toVectorVelocity ( motion );
    return SSVectorToJSSVector ( pEnv, vel );
}

/*
 * Class:     com_southernstars_sscore_JSSSpherical
 * Method:    angularSeparation
 * Signature: (Lcom/southernstars/sscore/JSSSpherical;)D
 */

JNIEXPORT jdouble JNICALL Java_com_southernstars_sscore_JSSSpherical_angularSeparation ( JNIEnv *pEnv, jobject pJThisSph, jobject pJThatSph )
{
    SSSpherical thisSph = JSSSphericalToSSSpherical ( pEnv, pJThisSph );
    SSSpherical thatSph = JSSSphericalToSSSpherical ( pEnv, pJThatSph );
    return thisSph.angularSeparation ( thatSph );
}

/*
 * Class:     com_southernstars_sscore_JSSSpherical
 * Method:    positionAngle
 * Signature: (Lcom/southernstars/sscore/JSSSpherical;)D
 */

JNIEXPORT jdouble JNICALL Java_com_southernstars_sscore_JSSSpherical_positionAngle ( JNIEnv *pEnv, jobject pJThisSph, jobject pJThatSph )
{
    SSSpherical thisSph = JSSSphericalToSSSpherical ( pEnv, pJThisSph );
    SSSpherical thatSph = JSSSphericalToSSSpherical ( pEnv, pJThatSph );
    return thisSph.angularSeparation ( thatSph );
}
