#include "com_southernstars_sscore_JSSEvent.h"
#include "JNIUtilities.h"
#include "SSEvent.hpp"

/*
 * Class:     com_southernstars_sscore_JSSEvent
 * Method:    semiDiurnalArc
 * Signature: (Lcom/southernstars/sscore/JSSAngle;Lcom/southernstars/sscore/JSSAngle;Lcom/southernstars/sscore/JSSAngle;)Lcom/southernstars/sscore/JSSAngle;
 */
JNIEXPORT jobject JNICALL Java_com_southernstars_sscore_JSSEvent_semiDiurnalArc (JNIEnv *pEnv, jclass pClass, jobject pJSSAngleLat, jobject pJSSAngleDec, jobject pJSSAngleAlt)
{
    double lat_rad = GetDoubleField ( pEnv, pJSSAngleLat, "rad" );  // I guess rad is not protected in the jobject
    double dec_rad = GetDoubleField ( pEnv, pJSSAngleDec, "rad" );
    double alt_rad = GetDoubleField ( pEnv, pJSSAngleAlt, "rad" );

    SSAngle lat = SSAngle( lat_rad );
    SSAngle dec = SSAngle( dec_rad );
    SSAngle alt = SSAngle( alt_rad );

    SSAngle ha = SSEvent::semiDiurnalArc( lat, dec, alt );
    
    jobject pJSSAngle = CreateJObject ( pEnv, "com/southernstars/sscore/JSSAngle" );

    double rad = ha.toHours() / ha.kHourPerRad;     // ha._rad is protected... or can we actually access _rad from line 94 in SSAngle.hpp?

    if ( pJSSAngle != nullptr )     // why is this check necessary?
    {
        SetDoubleField ( pEnv, pJSSAngle, "rad", rad );
    }

    return pJSSAngle;
}

/*
 * Class:     com_southernstars_sscore_JSSEvent
 * Method:    riseTransitSet
 * Signature: (Lcom/southernstars/sscore/JSSTime;Lcom/southernstars/sscore/JSSAngle;Lcom/southernstars/sscore/JSSAngle;ILcom/southernstars/sscore/JSSAngle;Lcom/southernstars/sscore/JSSAngle;Lcom/southernstars/sscore/JSSAngle;)Lcom/southernstars/sscore/JSSTime;
 */
JNIEXPORT jobject JNICALL Java_com_southernstars_sscore_JSSEvent_riseTransitSet__Lcom_southernstars_sscore_JSSTime_2Lcom_southernstars_sscore_JSSAngle_2Lcom_southernstars_sscore_JSSAngle_2ILcom_southernstars_sscore_JSSAngle_2Lcom_southernstars_sscore_JSSAngle_2Lcom_southernstars_sscore_JSSAngle_2 (JNIEnv *pEnv, jclass pClass, jobject pJSSTimeJd, jobject pJSSAngleRa, jobject pJSSAngleDec, jint sign, jobject pJSSAngleLon, jobject pJSSAngleLat, jobject pJSSAngleAlt)
{
    double jd_jd = GetDoubleField ( pEnv, pJSSTimeJd, "jd" );
    double jd_zone = GetDoubleField ( pEnv, pJSSTimeJd, "zone" );
    SSTime jd = SSTime( jd_jd, jd_zone );

    SSAngle ra = SSAngle( GetDoubleField ( pEnv, pJSSAngleRa, "rad" ) );
    SSAngle dec = SSAngle( GetDoubleField ( pEnv, pJSSAngleDec, "rad" ) );
    SSAngle lon = SSAngle( GetDoubleField ( pEnv, pJSSAngleLon, "rad" ) );
    SSAngle lat = SSAngle( GetDoubleField ( pEnv, pJSSAngleLat, "rad" ) );
    SSAngle alt = SSAngle( GetDoubleField ( pEnv, pJSSAngleAlt, "rad" ) );

    SSTime ha = SSEvent::riseTransitSet( jd, ra, dec, sign, lon, lat, alt );

    jobject pJSSTime = CreateJObject ( pEnv, "com/southernstars/sscore/JSSTime" );

    if ( pJSSTime != nullptr )
    {
        SetDoubleField ( pEnv, pJSSTime, "jd", ha.jd );
        SetDoubleField ( pEnv, pJSSTime, "zone", ha.zone );
    }

    return pJSSTime;
}

/*
 * Class:     com_southernstars_sscore_JSSEvent
 * Method:    riseTransitSet
 * Signature: (Lcom/southernstars/sscore/JSSTime;Lcom/southernstars/sscore/JSSCoordinates;Lcom/southernstars/sscore/JSSObject;ILcom/southernstars/sscore/JSSAngle;)Lcom/southernstars/sscore/JSSTime;
 */
JNIEXPORT jobject JNICALL Java_com_southernstars_sscore_JSSEvent_riseTransitSet__Lcom_southernstars_sscore_JSSTime_2Lcom_southernstars_sscore_JSSCoordinates_2Lcom_southernstars_sscore_JSSObject_2ILcom_southernstars_sscore_JSSAngle_2  (JNIEnv *pEnv, jclass pClass, jobject, jobject, jobject, jint, jobject);

/*
 * Class:     com_southernstars_sscore_JSSEvent
 * Method:    riseTransitSetSearch
 * Signature: (Lcom/southernstars/sscore/JSSTime;Lcom/southernstars/sscore/JSSCoordinates;Lcom/southernstars/sscore/JSSObject;ILcom/southernstars/sscore/JSSAngle;)Lcom/southernstars/sscore/JSSTime;
 */
JNIEXPORT jobject JNICALL Java_com_southernstars_sscore_JSSEvent_riseTransitSetSearch  (JNIEnv *pEnv, jclass pClass, jobject, jobject, jobject, jint, jobject);

/*
 * Class:     com_southernstars_sscore_JSSEvent
 * Method:    riseTransitSetSearchDay
 * Signature: (Lcom/southernstars/sscore/JSSTime;Lcom/southernstars/sscore/JSSCoordinates;Lcom/southernstars/sscore/JSSObject;ILcom/southernstars/sscore/JSSAngle;)Lcom/southernstars/sscore/JSSTime;
 */
JNIEXPORT jobject JNICALL Java_com_southernstars_sscore_JSSEvent_riseTransitSetSearchDay  (JNIEnv *pEnv, jclass pClass, jobject, jobject, jobject, jint, jobject);

/*
 * Class:     com_southernstars_sscore_JSSEvent
 * Method:    riseTransitSet
 * Signature: (Lcom/southernstars/sscore/JSSTime;Lcom/southernstars/sscore/JSSCoordinates;Lcom/southernstars/sscore/JSSObject;Lcom/southernstars/sscore/JSSAngle;)Lcom/southernstars/sscore/JSSPass;
 */
JNIEXPORT jobject JNICALL Java_com_southernstars_sscore_JSSEvent_riseTransitSet__Lcom_southernstars_sscore_JSSTime_2Lcom_southernstars_sscore_JSSCoordinates_2Lcom_southernstars_sscore_JSSObject_2Lcom_southernstars_sscore_JSSAngle_2  (JNIEnv *pEnv, jclass pClass, jobject, jobject, jobject, jobject);

/*
 * Class:     com_southernstars_sscore_JSSEvent
 * Method:    findSatellitePaJsses
 * Signature: (Lcom/southernstars/sscore/JSSCoordinates;Lcom/southernstars/sscore/JSSObject;Lcom/southernstars/sscore/JSSTime;Lcom/southernstars/sscore/JSSTime;DLjava/util/ArrayList;I)I
 */
JNIEXPORT jint JNICALL Java_com_southernstars_sscore_JSSEvent_findSatellitePaJsses  (JNIEnv *pEnv, jclass pClass, jobject, jobject, jobject, jobject, jdouble, jobject, jint);

/*
 * Class:     com_southernstars_sscore_JSSEvent
 * Method:    nextMoonPhase
 * Signature: (Lcom/southernstars/sscore/JSSTime;Lcom/southernstars/sscore/JSSObject;Lcom/southernstars/sscore/JSSObject;D)Lcom/southernstars/sscore/JSSTime;
 */
JNIEXPORT jobject JNICALL Java_com_southernstars_sscore_JSSEvent_nextMoonPhase  (JNIEnv *pEnv, jclass pClass, jobject, jobject, jobject, jdouble);

/*
 * Class:     com_southernstars_sscore_JSSEvent
 * Method:    findConjunctions
 * Signature: (Lcom/southernstars/sscore/JSSCoordinates;Lcom/southernstars/sscore/JSSObject;Lcom/southernstars/sscore/JSSObject;Lcom/southernstars/sscore/JSSTime;Lcom/southernstars/sscore/JSSTime;Ljava/util/ArrayList;I)V
 */
JNIEXPORT void JNICALL Java_com_southernstars_sscore_JSSEvent_findConjunctions  (JNIEnv *pEnv, jclass pClass, jobject, jobject, jobject, jobject, jobject, jobject, jint);

/*
 * Class:     com_southernstars_sscore_JSSEvent
 * Method:    findOppositions
 * Signature: (Lcom/southernstars/sscore/JSSCoordinates;Lcom/southernstars/sscore/JSSObject;Lcom/southernstars/sscore/JSSObject;Lcom/southernstars/sscore/JSSTime;Lcom/southernstars/sscore/JSSTime;Ljava/util/ArrayList;I)V
 */
JNIEXPORT void JNICALL Java_com_southernstars_sscore_JSSEvent_findOppositions  (JNIEnv *pEnv, jclass pClass, jobject, jobject, jobject, jobject, jobject, jobject, jint);

/*
 * Class:     com_southernstars_sscore_JSSEvent
 * Method:    findNearestDistances
 * Signature: (Lcom/southernstars/sscore/JSSCoordinates;Lcom/southernstars/sscore/JSSObject;Lcom/southernstars/sscore/JSSObject;Lcom/southernstars/sscore/JSSTime;Lcom/southernstars/sscore/JSSTime;Ljava/util/ArrayList;I)V
 */
JNIEXPORT void JNICALL Java_com_southernstars_sscore_JSSEvent_findNearestDistances  (JNIEnv *pEnv, jclass pClass, jobject, jobject, jobject, jobject, jobject, jobject, jint);

/*
 * Class:     com_southernstars_sscore_JSSEvent
 * Method:    findFarthestDistances
 * Signature: (Lcom/southernstars/sscore/JSSCoordinates;Lcom/southernstars/sscore/JSSObject;Lcom/southernstars/sscore/JSSObject;Lcom/southernstars/sscore/JSSTime;Lcom/southernstars/sscore/JSSTime;Ljava/util/ArrayList;I)V
 */
JNIEXPORT void JNICALL Java_com_southernstars_sscore_JSSEvent_findFarthestDistances  (JNIEnv *pEnv, jclass pClass, jobject, jobject, jobject, jobject, jobject, jobject, jint);
