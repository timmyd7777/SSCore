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

JNIEXPORT jobject JNICALL Java_com_southernstars_sscore_JSSEvent_getISS
  (JNIEnv *pEnv, jclass pClass)
{
    //
    // VERSION 1: RECONSTRUCT JUST THE ISS OBJECT
    //
    // From visual.txt
    string visual_line_1 = "ISS (ZARYA)             ";
    string visual_line_2 = "1 25544U 98067A   20095.06923611  .00000951  00000-0  25699-4 0  9994";
    string visual_line_3 = "2 25544  51.6468 355.2267 0003903  85.7249 119.8680 15.48678336220527";

    // From SSPlanet::SSImportSatellitesFromTLE & SSTLE::read
    // Code is slimmed down since we know the input
    SSTLE tle;

    double xmnpda = 1.44e3;

    int    year = 0, number = 0, iexp = 0, ibexp = 0;
    double xm0 = 0.0, xnode0 = 0.0, omega0 = 0.0;
    double e0 = 0.0, xn0 = 1.0, xndt20 = 0.0, xndd60 = 0.0;
    double day = 0.0, epoch = 0.0;
    double temp = M_2PI / xmnpda / xmnpda;

    tle.name = trim ( visual_line_1 );

    number = strtoint ( visual_line_2.substr ( 2, 5 ) );
    tle.desig = trim ( visual_line_2.substr ( 9, 6 ) );
    epoch = strtofloat64 ( visual_line_2.substr ( 18, 14 ) );
    xndt20 = strtofloat64 ( visual_line_2.substr ( 33, 10 ) );
    xndd60 = strtofloat64 ( visual_line_2.substr ( 44, 6 ) );
    iexp = strtoint ( visual_line_2.substr ( 50, 2 ) );
    double bstar = strtofloat64 ( visual_line_2.substr ( 53, 6 ) );
    ibexp = strtoint ( visual_line_2.substr ( 59, 2 ) );

    year = epoch / 1000.0;
    day = epoch - year * 1000.0;
    if ( year > 56 )
        year += 1900;
    else
        year += 2000;

    tle.norad = number;
    tle.jdepoch = SSTime ( SSDate ( kGregorian, 0.0, year, 1, day, 0, 0, 0.0 ) );
    tle.xndt2o = xndt20 * temp;
    tle.xndd6o = xndd60 * 1.0e-5 * pow ( 10.0, iexp );
    tle.bstar = bstar * 1.0e-5 * pow ( 10.0, ibexp );

    number = strtoint ( visual_line_3.substr ( 2, 5 ) );
    double xincl = strtofloat64 ( visual_line_3.substr ( 8, 8 ) );
    xnode0 = strtofloat64 ( visual_line_3.substr ( 17, 8 ) );
    e0 = strtofloat64 ( visual_line_3.substr ( 26, 7 ) );
    omega0 = strtofloat64 ( visual_line_3.substr ( 34, 8 ) );
    xm0 = strtofloat64 ( visual_line_3.substr ( 43, 8 ) );
    xn0 = strtofloat64 ( visual_line_3.substr ( 52, 11 ) );
    
    tle.xincl = degtorad ( xincl );
    tle.xnodeo = degtorad ( xnode0 );
    tle.eo = e0 * 1.0e-7;
    tle.omegao = degtorad ( omega0 );
    tle.xmo = degtorad ( xm0 );
    tle.xno = xn0 * M_2PI / xmnpda;

    tle.delargs();
    tle.deep = tle.isdeep();

    SSSatellite *pSat = new SSSatellite ( tle );

    // From mcnames.txt
    string line = "25544 ISS             30.0 20.0  0.0 -0.5 v  399";

    McName mcname = { 0, "", 0.0, 0.0, 0.0, 0.0 };
    mcname.norad = strtoint ( line.substr ( 0, 5 ) );
    mcname.name = trim ( line.substr ( 6, 17 ) );
    mcname.len = strtofloat ( line.substr ( 22, 4 ) );
    mcname.wid = strtofloat ( line.substr ( 27, 4 ) );
    mcname.dep = strtofloat ( line.substr ( 32, 4 ) );
    mcname.mag = strtofloat ( line.substr ( 37, 4 ) );

    pSat->setHMagnitude ( mcname.mag );
    pSat->setRadius ( mcname.len / 1000.0 );

    // return SSObjectToJSSObject ( pEnv, pSat );


    //
    // VERSION 2: USE SSImportSatellitesFromTLE & SSImportMcNames
    //
    SSObjectVec solsys;

    int nsat = SSImportSatellitesFromTLE ( "SSData/SolarSystem/Satellites/visual.txt", solsys );
    cout << "Imported " << nsat << " artificial satellites." << endl;

    int nnames = SSImportMcNames ( "SSData/SolarSystem/Satellites/mcnames.txt", solsys );
    cout << "Imported " << nnames << " McCants satellite names." << endl;

    // Find the ISS
    
    int i = 0;
    for ( i = 0; i < solsys.size(); i++ )
    {
        SSPlanet *p = SSGetPlanetPtr ( solsys[i] );
        if ( p->getType() == kTypeSatellite )
            if ( p->getIdentifier() == SSIdentifier ( kCatNORADSat, 25544 ) )
                break;
    }
    
    // // If we found it, return the ISS
    
    // if ( i < solsys.size() )
    // {
    //     return SSObjectToJSSObject ( pEnv, solsys[i] );
    // }

    SSObject *z = solsys[i];
    jobject jgood = SSObjectToJSSObject ( pEnv, pSat );
    jobject jbad = SSObjectToJSSObject ( pEnv, solsys[i] );
    SSObject *ssgood = JSSObjectToSSObject ( pEnv, jgood );
    SSObject *ssbad = JSSObjectToSSObject ( pEnv, jbad );
    // SSSatellite *_bad = SSGetSatellitePtr ( solsys[i] );
    // SSTLE _tle = _bad->getTLE();
    // SSSatellite *__bad = new SSSatellite ( _tle  );
    return jgood;
}