#include "com_southernstars_sscore_JSSEvent.h"
#include "JNIUtilities.h"
#include "SSEvent.hpp"
#include "SSIdentifier.hpp"
#include "SSPlanet.hpp"

jmethodID java_util_ArrayList_add ( JNIEnv *pEnv )
{
    static jclass java_util_ArrayList = static_cast<jclass> ( pEnv->NewGlobalRef ( pEnv->FindClass ( "java/util/ArrayList" )));
    return pEnv->GetMethodID ( java_util_ArrayList, "add", "(Ljava/lang/Object;)Z" );
}

/*
 * Class:     com_southernstars_sscore_JSSEvent
 * Method:    semiDiurnalArc
 * Signature: (DDD)D
 */
JNIEXPORT jdouble JNICALL Java_com_southernstars_sscore_JSSEvent_semiDiurnalArc
    ( JNIEnv *pEnv, jclass pClass, jdouble lat, jdouble dec, jdouble alt )
{
    return SSEvent::semiDiurnalArc( lat, dec, alt );
}

/*
 * Class:     com_southernstars_sscore_JSSEvent
 * Method:    riseTransitSet
 * Signature: (Lcom/southernstars/sscore/JSSTime;DDIDDD)Lcom/southernstars/sscore/JSSTime;
 */
JNIEXPORT jobject JNICALL Java_com_southernstars_sscore_JSSEvent_riseTransitSet__Lcom_southernstars_sscore_JSSTime_2DDIDDD
    ( JNIEnv *pEnv, jclass pClass, jobject pJTime, jdouble ra, jdouble dec, jint sign, jdouble lon, jdouble lat, jdouble alt )
{
    SSTime time = JSSTimeToSSTime ( pEnv, pJTime );
    SSTime ha = SSEvent::riseTransitSet( time, ra, dec, sign, lon, lat, alt );

    return SSTimeToJSSTime ( pEnv, ha );
}

/*
 * Class:     com_southernstars_sscore_JSSEvent
 * Method:    riseTransitSet
 * Signature: (Lcom/southernstars/sscore/JSSTime;Lcom/southernstars/sscore/JSSCoordinates;Lcom/southernstars/sscore/JSSObject;ID)Lcom/southernstars/sscore/JSSTime;
 */
JNIEXPORT jobject JNICALL Java_com_southernstars_sscore_JSSEvent_riseTransitSet__Lcom_southernstars_sscore_JSSTime_2Lcom_southernstars_sscore_JSSCoordinates_2Lcom_southernstars_sscore_JSSObject_2ID
    ( JNIEnv *pEnv, jclass pClass, jobject pJTime, jobject pJCoords, jobject pJObj, jint sign, jdouble alt )
{
    SSTime time = JSSTimeToSSTime ( pEnv, pJTime );
    SSCoordinates *pCoords = (SSCoordinates *) GetLongField ( pEnv, pJCoords, "pCoords" );
    SSObject *pObj = (SSObject *) GetLongField ( pEnv, pJObj, "pObject" );

    if ( pCoords && pObj )
    {
        SSTime ha = SSEvent::riseTransitSet( time, *pCoords, pObj, sign, alt );
        return SSTimeToJSSTime ( pEnv, ha );
    }

    return SSTimeToJSSTime ( pEnv, SSTime ( INFINITY ) );
}

/*
 * Class:     com_southernstars_sscore_JSSEvent
 * Method:    riseTransitSetSearch
 * Signature: (Lcom/southernstars/sscore/JSSTime;Lcom/southernstars/sscore/JSSCoordinates;Lcom/southernstars/sscore/JSSObject;ID)Lcom/southernstars/sscore/JSSTime;
 */
JNIEXPORT jobject JNICALL Java_com_southernstars_sscore_JSSEvent_riseTransitSetSearch
    ( JNIEnv *pEnv, jclass pClass, jobject pJTime, jobject pJCoords, jobject pJObj, jint sign, jdouble alt )
{
    SSTime time = JSSTimeToSSTime ( pEnv, pJTime );
    SSCoordinates *pCoords = (SSCoordinates *) GetLongField ( pEnv, pJCoords, "pCoords" );
    SSObject *pObj = (SSObject *) GetLongField ( pEnv, pJObj, "pObject" );

    if ( pCoords && pObj )
    {
        SSTime ha = SSEvent::riseTransitSetSearch( time, *pCoords, pObj, sign, alt );
        return SSTimeToJSSTime ( pEnv, ha );
    }

    return SSTimeToJSSTime ( pEnv, SSTime ( INFINITY ) );
}

/*
 * Class:     com_southernstars_sscore_JSSEvent
 * Method:    riseTransitSetSearchDay
 * Signature: (Lcom/southernstars/sscore/JSSTime;Lcom/southernstars/sscore/JSSCoordinates;Lcom/southernstars/sscore/JSSObject;ID)Lcom/southernstars/sscore/JSSTime;
 */
JNIEXPORT jobject JNICALL Java_com_southernstars_sscore_JSSEvent_riseTransitSetSearchDay
    ( JNIEnv *pEnv, jclass pClass, jobject pJTime, jobject pJCoords, jobject pJObj, jint sign, jdouble alt )
{
    SSTime today = JSSTimeToSSTime ( pEnv, pJTime );
    SSCoordinates *pCoords = (SSCoordinates *) GetLongField ( pEnv, pJCoords, "pCoords" );
    SSObject *pObj = (SSObject *) GetLongField ( pEnv, pJObj, "pObject" );

    if ( pCoords && pObj )
    {
        SSTime ha = SSEvent::riseTransitSetSearchDay( today, *pCoords, pObj, sign, alt );
        return SSTimeToJSSTime ( pEnv, ha );
    }

    return SSTimeToJSSTime ( pEnv, SSTime ( INFINITY ) );
}

/*
 * Class:     com_southernstars_sscore_JSSEvent
 * Method:    riseTransitSet
 * Signature: (Lcom/southernstars/sscore/JSSTime;Lcom/southernstars/sscore/JSSCoordinates;Lcom/southernstars/sscore/JSSObject;D)Lcom/southernstars/sscore/JSSPass;
 */
JNIEXPORT jobject JNICALL Java_com_southernstars_sscore_JSSEvent_riseTransitSet__Lcom_southernstars_sscore_JSSTime_2Lcom_southernstars_sscore_JSSCoordinates_2Lcom_southernstars_sscore_JSSObject_2D
    ( JNIEnv *pEnv, jclass pClass, jobject pJTime, jobject pJCoords, jobject pJObj, jdouble alt )
{
    SSTime today = JSSTimeToSSTime ( pEnv, pJTime );
    SSCoordinates *pCoords = (SSCoordinates *) GetLongField ( pEnv, pJCoords, "pCoords" );
    SSObject *pObj = (SSObject *) GetLongField ( pEnv, pJObj, "pObject" );

    if ( pCoords && pObj )
    {
        SSPass pass = SSEvent::riseTransitSet( today, *pCoords, pObj, alt );
        return SSPassToJSSPass( pEnv, pass );
    }

    SSPass badpass =
    {
        { { INFINITY, INFINITY }, INFINITY, INFINITY },
        { { INFINITY, INFINITY }, INFINITY, INFINITY },
        { { INFINITY, INFINITY }, INFINITY, INFINITY }
    };

    return SSPassToJSSPass ( pEnv, badpass );
}

/*
 * Class:     com_southernstars_sscore_JSSEvent
 * Method:    findSatellitePasses
 * Signature: (Lcom/southernstars/sscore/JSSCoordinates;Lcom/southernstars/sscore/JSSObject;Lcom/southernstars/sscore/JSSTime;Lcom/southernstars/sscore/JSSTime;DLjava/util/ArrayList;I)I
 */
JNIEXPORT jint JNICALL Java_com_southernstars_sscore_JSSEvent_findSatellitePasses
    ( JNIEnv *pEnv, jclass pClass, jobject pJCoords, jobject pJSat, jobject pJTimeStart, jobject pJTimeStop, jdouble minAlt, jobject pJPasses, jint maxPasses )
{
    SSCoordinates *pCoords = (SSCoordinates *) GetLongField ( pEnv, pJCoords, "pCoords" );
    SSObject *pSat = JSSObjectToSSObject ( pEnv, pJSat );
    SSTime start = JSSTimeToSSTime ( pEnv, pJTimeStart );
    SSTime stop = JSSTimeToSSTime ( pEnv, pJTimeStop );

    vector<SSPass> passes;

    int size = SSEvent::findSatellitePasses ( *pCoords, pSat, start, stop, minAlt, passes, maxPasses );

    for (SSPass pass: passes) {
        jobject jPass = SSPassToJSSPass ( pEnv, pass );
        pEnv->CallBooleanMethod ( pJPasses, java_util_ArrayList_add ( pEnv ), jPass );
    }

    return size;
}

/*
 * Class:     com_southernstars_sscore_JSSEvent
 * Method:    nextMoonPhase
 * Signature: (Lcom/southernstars/sscore/JSSTime;Lcom/southernstars/sscore/JSSObject;Lcom/southernstars/sscore/JSSObject;D)Lcom/southernstars/sscore/JSSTime;
 */
JNIEXPORT jobject JNICALL Java_com_southernstars_sscore_JSSEvent_nextMoonPhase
    ( JNIEnv *pEnv, jclass pClass, jobject pJTime, jobject pJObjSun, jobject pJObjMoon, jdouble phase )
{
    SSTime time = JSSTimeToSSTime ( pEnv, pJTime );
    SSObject *pSun = JSSObjectToSSObject ( pEnv, pJObjSun );
    SSObject *pMoon = JSSObjectToSSObject ( pEnv, pJObjMoon );
    
    SSTime next_time = SSEvent::nextMoonPhase ( time, pSun, pMoon, phase );
    return SSTimeToJSSTime ( pEnv, next_time );
}

/*
 * Class:     com_southernstars_sscore_JSSEvent
 * Method:    findConjunctions
 * Signature: (Lcom/southernstars/sscore/JSSCoordinates;Lcom/southernstars/sscore/JSSObject;Lcom/southernstars/sscore/JSSObject;Lcom/southernstars/sscore/JSSTime;Lcom/southernstars/sscore/JSSTime;Ljava/util/ArrayList;I)V
 */
JNIEXPORT void JNICALL Java_com_southernstars_sscore_JSSEvent_findConjunctions
    ( JNIEnv *pEnv, jclass pClass, jobject pJCoords, jobject pJObj1, jobject pJObj2, jobject pJTimeStart, jobject pJTimeStop, jobject pJEventTimes, jint maxEvents )
{
    SSCoordinates *pCoords = (SSCoordinates *) GetLongField ( pEnv, pJCoords, "pCoords" );
    SSObject *pObj1 = JSSObjectToSSObject ( pEnv, pJObj1 );
    SSObject *pObj2 = JSSObjectToSSObject ( pEnv, pJObj2 );
    SSTime start = JSSTimeToSSTime ( pEnv, pJTimeStart );
    SSTime stop = JSSTimeToSSTime ( pEnv, pJTimeStop );

    vector<SSEventTime> eventtimes;

    SSEvent::findConjunctions( *pCoords, pObj1, pObj2, start, stop, eventtimes, maxEvents );

    for (SSEventTime eventtime: eventtimes) {
        jobject jEventTime = SSEventTimeToJSSEventTime ( pEnv, eventtime );
        pEnv->CallBooleanMethod ( pJEventTimes, java_util_ArrayList_add ( pEnv ), jEventTime );
    }
}

/*
 * Class:     com_southernstars_sscore_JSSEvent
 * Method:    findOppositions
 * Signature: (Lcom/southernstars/sscore/JSSCoordinates;Lcom/southernstars/sscore/JSSObject;Lcom/southernstars/sscore/JSSObject;Lcom/southernstars/sscore/JSSTime;Lcom/southernstars/sscore/JSSTime;Ljava/util/ArrayList;I)V
 */
JNIEXPORT void JNICALL Java_com_southernstars_sscore_JSSEvent_findOppositions
    ( JNIEnv *pEnv, jclass pClass, jobject pJCoords, jobject pJObj1, jobject pJObj2, jobject pJTimeStart, jobject pJTimeStop, jobject pJEventTimes, jint maxEvents )
{
    SSCoordinates *pCoords = (SSCoordinates *) GetLongField ( pEnv, pJCoords, "pCoords" );
    SSObject *pObj1 = JSSObjectToSSObject ( pEnv, pJObj1 );
    SSObject *pObj2 = JSSObjectToSSObject ( pEnv, pJObj2 );
    SSTime start = JSSTimeToSSTime ( pEnv, pJTimeStart );
    SSTime stop = JSSTimeToSSTime ( pEnv, pJTimeStop );

    vector<SSEventTime> eventtimes;

    SSEvent::findOppositions( *pCoords, pObj1, pObj2, start, stop, eventtimes, maxEvents );

    for (SSEventTime eventtime: eventtimes) {
        jobject jEventTime = SSEventTimeToJSSEventTime ( pEnv, eventtime );
        pEnv->CallBooleanMethod ( pJEventTimes, java_util_ArrayList_add ( pEnv ), jEventTime );
    }
}

/*
 * Class:     com_southernstars_sscore_JSSEvent
 * Method:    findNearestDistances
 * Signature: (Lcom/southernstars/sscore/JSSCoordinates;Lcom/southernstars/sscore/JSSObject;Lcom/southernstars/sscore/JSSObject;Lcom/southernstars/sscore/JSSTime;Lcom/southernstars/sscore/JSSTime;Ljava/util/ArrayList;I)V
 */
JNIEXPORT void JNICALL Java_com_southernstars_sscore_JSSEvent_findNearestDistances
    ( JNIEnv *pEnv, jclass pClass, jobject pJCoords, jobject pJObj1, jobject pJObj2, jobject pJTimeStart, jobject pJTimeStop, jobject pJEventTimes, jint maxEvents )
{
    SSCoordinates *pCoords = (SSCoordinates *) GetLongField ( pEnv, pJCoords, "pCoords" );
    SSObject *pObj1 = JSSObjectToSSObject ( pEnv, pJObj1 );
    SSObject *pObj2 = JSSObjectToSSObject ( pEnv, pJObj2 );
    SSTime start = JSSTimeToSSTime ( pEnv, pJTimeStart );
    SSTime stop = JSSTimeToSSTime ( pEnv, pJTimeStop );

    vector<SSEventTime> eventtimes;

    SSEvent::findNearestDistances( *pCoords, pObj1, pObj2, start, stop, eventtimes, maxEvents );

    for (SSEventTime eventtime: eventtimes) {
        jobject jEventTime = SSEventTimeToJSSEventTime ( pEnv, eventtime );
        pEnv->CallBooleanMethod ( pJEventTimes, java_util_ArrayList_add ( pEnv ), jEventTime );
    }
}

/*
 * Class:     com_southernstars_sscore_JSSEvent
 * Method:    findFarthestDistances
 * Signature: (Lcom/southernstars/sscore/JSSCoordinates;Lcom/southernstars/sscore/JSSObject;Lcom/southernstars/sscore/JSSObject;Lcom/southernstars/sscore/JSSTime;Lcom/southernstars/sscore/JSSTime;Ljava/util/ArrayList;I)V
 */
JNIEXPORT void JNICALL Java_com_southernstars_sscore_JSSEvent_findFarthestDistances
    ( JNIEnv *pEnv, jclass pClass, jobject pJCoords, jobject pJObj1, jobject pJObj2, jobject pJTimeStart, jobject pJTimeStop, jobject pJEventTimes, jint maxEvents )
{
    SSCoordinates *pCoords = (SSCoordinates *) GetLongField ( pEnv, pJCoords, "pCoords" );
    SSObject *pObj1 = JSSObjectToSSObject ( pEnv, pJObj1 );
    SSObject *pObj2 = JSSObjectToSSObject ( pEnv, pJObj2 );
    SSTime start = JSSTimeToSSTime ( pEnv, pJTimeStart );
    SSTime stop = JSSTimeToSSTime ( pEnv, pJTimeStop );

    vector<SSEventTime> eventtimes;

    SSEvent::findFarthestDistances( *pCoords, pObj1, pObj2, start, stop, eventtimes, maxEvents );

    for (SSEventTime eventtime: eventtimes) {
        jobject jEventTime = SSEventTimeToJSSEventTime ( pEnv, eventtime );
        pEnv->CallBooleanMethod ( pJEventTimes, java_util_ArrayList_add ( pEnv ), jEventTime );
    }
}
