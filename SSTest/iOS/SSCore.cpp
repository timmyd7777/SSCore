//
// SSCore.c
// SSTest
//
// Created by Tim DeBenedictis on 4/7/20.
// Copyright © 2020 Southern Stars. All rights reserved.

#include "SSCore.h"

#include "SSTime.hpp"
#include "SSAngle.hpp"
#include "SSCoordinates.hpp"
#include "SSEvent.hpp"
#include "SSIdentifier.hpp"
#include "SSJPLDEphemeris.hpp"
#include "SSVector.hpp"
#include "SSMatrix.hpp"
#include "SSObject.hpp"

// C wrappers for C++ SSTime classes and methods

CSSDate CSSTimeToCSSDate ( CSSTime ctime )
{
    SSDate date ( SSTime ( ctime.jd, ctime.zone ) );
    CSSDate cdate = { date.calendar, date.zone, date.year, date.month, date.day, date.hour, date.min, date.sec };
    return cdate;
}

CSSDate CSSTimeToCSSDate ( CSSTime ctime, int calendar )
{
    SSDate date ( SSTime ( ctime.jd, ctime.zone ), (SSCalendar) calendar );
    CSSDate cdate = { date.calendar, date.zone, date.year, date.month, date.day, date.hour, date.min, date.sec };
    return cdate;
}

CSSTime CSSDateToCSSTime ( CSSDate cdate )
{
    SSTime time ( SSDate ( (SSCalendar) cdate.calendar, cdate.zone, cdate.year, cdate.month, cdate.day, cdate.hour, cdate.min, cdate.sec ) );
    CSSTime ctime = { time.jd, time.zone };
    return ctime;
}

CSSTime CSSTimeFromSSTime ( const SSTime &time )
{
    CSSTime ctime = { time.jd, time.zone };
    return ctime;
}

CSSTime CSSTimeFromSystem ( void )
{
    SSTime now = SSTime::fromSystem();
    return CSSTimeFromSSTime ( now );
}

CSSTime CSSTimeFromCalendarDate ( CSSDate cdate )
{
    SSTime time ( SSDate ( (SSCalendar) cdate.calendar, cdate.zone, cdate.year, cdate.month, cdate.day, cdate.hour, cdate.min, cdate.sec ) );
    return CSSTimeFromSSTime ( time );
}

CSSTime CSSTimeFromUnixTime ( time_t utime )
{
    SSTime time = SSTime::fromUnixTime ( utime );
    return CSSTimeFromSSTime ( time );
}

CSSTime CSSTimeFromJulianYear ( double year )
{
    SSTime time = SSTime::fromJulianYear ( year );
    return CSSTimeFromSSTime ( time );
}

CSSTime CSSTimeFromBesselianYear ( double year )
{
    SSTime time = SSTime::fromBesselianYear ( year );
    return CSSTimeFromSSTime ( time );
}

time_t CSSTimeToUnixTime ( CSSTime ctime )
{
    SSTime time ( ctime.jd );
    return time.toUnixTime();
}

double CSSTimeToJulianYear ( CSSTime ctime )
{
    SSTime time ( ctime.jd );
    return time.toJulianYear();
}

double CSSTimeToBesselianYear ( CSSTime ctime )
{
    SSTime time ( ctime.jd );
    return time.toBesselianYear();
}

double CSSTimeGetDeltaT ( CSSTime ctime )
{
    SSTime time ( ctime.jd );
    return time.getDeltaT();
}

double CSSTimeGetJulianEphemerisDate ( CSSTime ctime )
{
    SSTime time ( ctime.jd );
    return time.getJulianEphemerisDate();
}

double CSSTimeGetSiderealTime ( CSSTime ctime, double lon )
{
    SSTime time ( ctime.jd );
    return time.getSiderealTime ( lon );
}

// C wrappers for C++ SSAngle classes and methods

CSSDegMinSec CSSDegMinSecFromRadians ( CSSAngle rad )
{
    SSDegMinSec dms ( (SSAngle) rad );
    CSSDegMinSec cdms = { dms.sign, dms.deg, dms.min, dms.sec };
    return cdms;
}

CSSDegMinSec CSSDegMinSecFromString ( const char *cstr )
{
    SSDegMinSec dms ( cstr );
    CSSDegMinSec cdms = { dms.sign, dms.deg, dms.min, dms.sec };
    return cdms;
}

CSSAngle CSSDegMinSecToRadians ( CSSDegMinSec cdms )
{
    SSDegMinSec dms ( cdms.sign, cdms.deg, cdms.min, cdms.sec );
    return SSAngle ( dms );
}

const char *CSSDegMinSecToString ( CSSDegMinSec cdms )
{
    static string str = "";
    SSDegMinSec dms ( cdms.sign, cdms.deg, cdms.min, cdms.sec );
    str = dms.toString();
    return str.c_str();
}

CSSHourMinSec CSSHourMinSecFromRadians ( CSSAngle rad )
{
    SSHourMinSec hms ( (SSAngle) rad );
    CSSHourMinSec chms = { hms.hour, hms.min, hms.sec };
    return chms;
}

CSSHourMinSec CSSHourMinSecFromString ( const char *str )
{
    SSHourMinSec hms ( str );
    CSSHourMinSec chms = { hms.hour, hms.min, hms.sec };
    return chms;
}

CSSAngle CSSHourMinSecToRadians ( CSSHourMinSec chms )
{
    SSHourMinSec hms ( chms.hour, chms.min, chms.sec );
    return SSAngle ( hms );
}

const char *CSSHourMinSecToString ( CSSHourMinSec chms )
{
    static string str = "";
    SSHourMinSec hms ( chms.hour, chms.min, chms.sec );
    str = hms.toString();
    return str.c_str();
}

// C wrappers for C++ SSVector classes and methods

CSSSpherical CSSSphericalFromSSSpherical ( SSSpherical &sph )
{
    CSSSpherical csph = { sph.lon, sph.lat, sph.rad };
    return csph;
}

SSSpherical CSSSphericalToSSSpherical ( const CSSSpherical &csph )
{
    return SSSpherical ( csph.lon, csph.lat, csph.rad );
}

CSSVector CSSVectorFromSSVector ( const SSVector &vec )
{
    CSSVector cvec = { vec.x, vec.y, vec.z };
    return cvec;
}

SSVector CSSVectorToSSVector ( const CSSVector &cvec )
{
    return SSVector ( cvec.x, cvec.y, cvec.z );
}

CSSVector CSSVectorFromXYZ ( double x, double y, double z )
{
    CSSVector vec = { x, y, z };
    return vec;
}

CSSSpherical CSSSphericalFromLonLatRad ( double lon, double lat, double rad )
{
    CSSSpherical sph = { lon, lat, rad };
    return sph;
}

double CSSSphericalAngularSeparation ( CSSSpherical csph1, CSSSpherical csph2 )
{
    SSSpherical sph1 ( csph1.lon, csph1.lat, csph1.rad );
    SSSpherical sph2 ( csph2.lon, csph2.lat, csph2.rad );
    return sph1.angularSeparation ( sph2 );
}

double CSSSphericalPositionAngle ( CSSSpherical csph1, CSSSpherical csph2 )
{
    SSSpherical sph1 ( csph1.lon, csph1.lat, csph1.rad );
    SSSpherical sph2 ( csph2.lon, csph2.lat, csph2.rad );
    return sph1.positionAngle ( sph2 );
}

CSSVector CSSSphericalToCSSVector ( CSSSpherical csph )
{
    SSVector vec ( SSSpherical ( csph.lon, csph.lat, csph.rad ) );
    return CSSVectorFromSSVector ( vec );
}

CSSSpherical CSSVectorToCSSSpherical ( CSSVector cvec )
{
    SSSpherical sph ( SSVector ( cvec.x, cvec.y, cvec.z ) );
    return CSSSphericalFromSSSpherical ( sph );
}

CSSVector CSSSphericalCSSVectorVelocity ( CSSSpherical cpos, CSSSpherical cvel )
{
    SSSpherical spos ( cpos.lon, cpos.lat, cpos.rad );
    SSSpherical svel ( cvel.lon, cvel.lat, cvel.rad );
    SSVector vvel = spos.toVectorVelocity ( svel );
    return CSSVectorFromSSVector ( vvel );
}

CSSSpherical CSSVectorToCSSSphericalVelocity ( CSSVector cpos, CSSVector cvel )
{
    SSVector vpos ( cpos.x, cpos.y, cpos.z );
    SSVector vvel ( cvel.x, cvel.y, cvel.z );
    SSSpherical svel = vpos.toSphericalVelocity ( vvel );
    return CSSSphericalFromSSSpherical ( svel );
}

double CSSVectorMagnitude ( CSSVector cvec )
{
    SSVector vec ( cvec.x, cvec.y, cvec.z );
    return vec.magnitude();
}

CSSVector CSSVectorNormalize ( CSSVector cvec )
{
    SSVector vec ( cvec.x, cvec.y, cvec.z );
    vec = vec.normalize();
    return CSSVectorFromSSVector ( vec );
}

CSSVector CSSVectorAdd ( CSSVector cvec1, CSSVector cvec2 )
{
    CSSVector csum = { cvec1.x + cvec2.x, cvec1.y + cvec2.y,  cvec1.z + cvec2.z };
    return csum;
}

CSSVector CSSVectorSubtract ( CSSVector cvec1, CSSVector cvec2 )
{
    CSSVector cdif = { cvec1.x - cvec2.x, cvec1.y - cvec2.y,  cvec1.z - cvec2.z };
    return cdif;
}

CSSVector CSSVectorMultiplyBy ( CSSVector cvec, double s )
{
    CSSVector cvec1 = { cvec.x * s, cvec.y * s, cvec.z * s };
    return cvec1;
}

CSSVector CSSVectorDivideBy ( CSSVector cvec, double s )
{
    CSSVector cvec1 = { cvec.x / s, cvec.y / s, cvec.z / s };
    return cvec1;
}

double CSSVectorDistance ( CSSVector cvec1, CSSVector cvec2 )
{
    return CSSVectorMagnitude ( CSSVectorSubtract ( cvec1, cvec2 ) );
}

double CSSVectorDotProduct ( CSSVector cvec1, CSSVector cvec2 )
{
    return cvec1.x * cvec2.x + cvec1.y * cvec2.y + cvec1.z * cvec2.z;
}

CSSVector CSSVectorCrossProduct ( CSSVector cvec1, CSSVector cvec2 )
{
    SSVector vec1 ( cvec1.x, cvec1.y, cvec1.z );
    SSVector vec2 ( cvec2.x, cvec2.y, cvec2.z );
    SSVector x = vec1.crossProduct ( vec2 );
    return CSSVectorFromSSVector ( x );
}

double CSSVectorAngularSeparation ( CSSVector cvec1, CSSVector cvec2 )
{
    SSVector vec1 ( cvec1.x, cvec1.y, cvec1.z );
    SSVector vec2 ( cvec2.x, cvec2.y, cvec2.z );
    return vec1.angularSeparation ( vec2 );
}

double CSSVectorPositionAngle ( CSSVector cvec1, CSSVector cvec2 )
{
    SSVector vec1 ( cvec1.x, cvec1.y, cvec1.z );
    SSVector vec2 ( cvec2.x, cvec2.y, cvec2.z );
    return vec1.positionAngle ( vec2 );
}

// C wrappers for C++ SSMatrix classes and methods

SSMatrix CSSMatrixToSSMatrix ( const CSSMatrix &cmat )
{
    SSMatrix mat =
    {
        cmat.m00, cmat.m01, cmat.m02,
        cmat.m10, cmat.m11, cmat.m12,
        cmat.m20, cmat.m21, cmat.m22
    };
    
    return mat;
}

CSSMatrix CSSMatrixFromSSMatrix ( const SSMatrix &mat )
{
    CSSMatrix cmat =
    {
        mat.m00, mat.m01, mat.m02,
        mat.m10, mat.m11, mat.m12,
        mat.m20, mat.m21, mat.m22
    };
    
    return cmat;
}

CSSMatrix CSSMatrixIdentity ( void )
{
    CSSMatrix cmat =
    {
        1.0, 0.0, 0.0,
        0.0, 1.0, 0.0,
        0.0, 0.0, 1.0
    };
    
    return cmat;
}

CSSMatrix CSSMatrixTranspose ( CSSMatrix mat )
{
    CSSMatrix matr =
    {
        mat.m00, mat.m10, mat.m20,
        mat.m01, mat.m11, mat.m21,
        mat.m02, mat.m12, mat.m22
    };
    
    return matr;
}

CSSMatrix CSSMatrixInverse ( CSSMatrix cmat )
{
    SSMatrix mat = CSSMatrixToSSMatrix ( cmat );
    mat = mat.inverse();
    return CSSMatrixFromSSMatrix ( mat );
}

double CSSMatrixDeterminant ( CSSMatrix cmat )
{
    SSMatrix mat = CSSMatrixToSSMatrix ( cmat );
    return mat.determinant();
}

CSSMatrix CSSMatrixMultiplyMatrix ( CSSMatrix cmat1, CSSMatrix cmat2 )
{
    SSMatrix mat1 = CSSMatrixToSSMatrix ( cmat1 );
    SSMatrix mat2 = CSSMatrixToSSMatrix ( cmat2 );
    SSMatrix mat3 = mat1 * mat2;
    return CSSMatrixFromSSMatrix ( mat3 );
}

CSSVector CSSMatrixMultiplyVector ( CSSMatrix cmat, CSSVector cvec )
{
    SSMatrix mat = CSSMatrixToSSMatrix ( cmat );
    SSVector vec = { cvec.x, cvec.y, cvec.z };
    vec = mat * vec;
    CSSVector cvec1 = { vec.x, vec.y, vec.z };
    return cvec1;
}

CSSMatrix CSSMatrixRotate ( CSSMatrix cmat, int axis, double angle )
{
    SSMatrix mat = CSSMatrixToSSMatrix ( cmat );
    mat = mat.rotate ( axis, angle );
    return CSSMatrixFromSSMatrix ( mat );
}

// C wrappers for C++ SSCoordinates classes and methods

CSSCoordinates *CSSCoordinatesCreate ( CSSTime ctime, CSSSpherical loc )
{
    SSTime time ( ctime.jd, ctime.zone );
    SSCoordinates *pCoords = new SSCoordinates ( time, CSSSphericalToSSSpherical ( loc ) );
    return (CSSCoordinates *) pCoords;
}

void CSSCoordinatesDestroy ( CSSCoordinates *pCCoords )
{
    SSCoordinates *pCoords = (SSCoordinates *) pCCoords;
    delete pCoords;
}

void CSSCoordinatesSetTime ( CSSCoordinates *pCCoords, CSSTime ctime )
{
    SSTime time ( ctime.jd, ctime.zone );
    SSCoordinates *pCoords = (SSCoordinates *) pCCoords;
    if ( pCoords )
        pCoords->setTime ( time );
}

void CSSCoordinatesSetLocation ( CSSCoordinates *pCCoords, CSSSpherical cloc )
{
    SSSpherical loc ( cloc.lon, cloc.lat, cloc.rad );
    SSCoordinates *pCoords = (SSCoordinates *) pCCoords;
    if ( pCoords )
        pCoords->setLocation ( loc );
}

CSSTime CSSCoordinatesGetTime ( CSSCoordinates *pCCoords )
{
    SSTime time ( HUGE_VAL );
    SSCoordinates *pCoords = (SSCoordinates *) pCCoords;
    if ( pCoords )
        time = pCoords->getTime();
    CSSTime ctime = { time.jd, time.zone };
    return ctime;
}

CSSSpherical CSSCoordinatesGetLocation ( CSSCoordinates *pCCoords )
{
    SSSpherical loc ( HUGE_VAL, HUGE_VAL, HUGE_VAL );
    SSCoordinates *pCoords = (SSCoordinates *) pCCoords;
    if ( pCoords )
        loc = pCoords->getLocation();
    return CSSSphericalFromSSSpherical ( loc );
}

CSSVector CSSCoordinatesGetObserverPosition ( CSSCoordinates *pCCoords )
{
    SSVector pos;
    SSCoordinates *pCoords = (SSCoordinates *) pCCoords;
    if ( pCoords )
        pos = pCoords->getObserverPosition();
    return CSSVectorFromSSVector ( pos );
}

CSSVector CSSCoordinatesGetObserverVelocity ( CSSCoordinates *pCCoords )
{
    SSVector vel;
    SSCoordinates *pCoords = (SSCoordinates *) pCCoords;
    if ( pCoords )
        vel = pCoords->getObserverVelocity();
    return CSSVectorFromSSVector ( vel );
}

bool CSSCoordinatesGetStarParallax ( CSSCoordinates *pCCoords )
{
    SSCoordinates *pCoords = (SSCoordinates *) pCCoords;
    return pCoords ? pCoords->getStarParallax() : false;
}

bool CSSCoordinatesGetStarMotion ( CSSCoordinates *pCCoords )
{
    SSCoordinates *pCoords = (SSCoordinates *) pCCoords;
    return pCoords ? pCoords->getStarMotion() : false;
}

bool CSSCoordinatesGetAberration ( CSSCoordinates *pCCoords )
{
    SSCoordinates *pCoords = (SSCoordinates *) pCCoords;
    return pCoords ? pCoords->getAberration() : false;
}

bool CSSCoordinatesGetLightTime ( CSSCoordinates *pCCoords )
{
    SSCoordinates *pCoords = (SSCoordinates *) pCCoords;
    return pCoords ? pCoords->getLightTime() : false;
}

void CSSCoordinatesSetStarParallax ( CSSCoordinates *pCCoords, bool parallax )
{
    SSCoordinates *pCoords = (SSCoordinates *) pCCoords;
    if ( pCoords )
        pCoords->setStarParallax ( parallax );
}

void CSSCoordinatesSetStarMotion ( CSSCoordinates *pCCoords, bool motion )
{
    SSCoordinates *pCoords = (SSCoordinates *) pCCoords;
    if ( pCoords )
        pCoords->setStarMotion ( motion );
}

void CSSCoordinatesSetAberration ( CSSCoordinates *pCCoords, bool aberration )
{
    SSCoordinates *pCoords = (SSCoordinates *) pCCoords;
    if ( pCoords )
        pCoords->setAberration ( aberration );
}

void CSSCoordinatesSetLightTime ( CSSCoordinates *pCCoords, bool lighttime )
{
    SSCoordinates *pCoords = (SSCoordinates *) pCCoords;
    if ( pCoords )
        pCoords->setLightTime ( lighttime );
}

double CSSCoordinatesGetJED ( CSSCoordinates *pCCoords )
{
    SSCoordinates *pCoords = (SSCoordinates *) pCCoords;
    return pCoords ? pCoords->getJED() : HUGE_VAL;
}

double CSSCoordinatesGetLST ( CSSCoordinates *pCCoords )
{
    SSCoordinates *pCoords = (SSCoordinates *) pCCoords;
    return pCoords ? pCoords->getLST() : HUGE_VAL;
}

CSSVector CSSCoordinatesTransformVector ( CSSCoordinates *pCCoords, int from, int to, CSSVector cvec )
{
    SSVector vec ( cvec.x, cvec.y, cvec.z );
    SSCoordinates *pCoords = (SSCoordinates *) pCCoords;
    if ( pCoords )
        vec = pCoords->transform ( (SSFrame) from, (SSFrame) to, vec );
    return CSSVectorFromSSVector ( vec );
}

CSSSpherical CSSCoordinatesTransformSpherical ( CSSCoordinates *pCCoords, int from, int to, CSSSpherical csph )
{
    SSSpherical sph ( csph.lon, csph.lat, csph.rad );
    SSCoordinates *pCoords = (SSCoordinates *) pCCoords;
    if ( pCoords )
        sph = pCoords->transform ( (SSFrame) from, (SSFrame) to, sph );
    return CSSSphericalFromSSSpherical ( sph );
}

double CSSCoordinatesGetObliquity ( double jd )
{
    return SSCoordinates::getObliquity ( jd );
}

void CSSCoordinatesGetNutationConstants ( double jd, double *de, double *dl )
{
    SSCoordinates::getNutationConstants ( jd, *de, *dl );
}

void CSSCoordinatesGetPrecessionConstants ( double jd, double *zeta, double *z, double *theta )
{
    SSCoordinates::getPrecessionConstants ( jd, *zeta, *z, *theta );
}

CSSMatrix CSSCoordinatesGetPrecessionMatrix ( double jd )
{
    SSMatrix mat = SSCoordinates::getPrecessionMatrix ( jd );
    return CSSMatrixFromSSMatrix ( mat );
}

CSSMatrix CSSCoordinatesGetNutationMatrix ( double obliquity, double dl, double de )
{
    SSMatrix mat = SSCoordinates::getNutationMatrix ( obliquity, dl, de );
    return CSSMatrixFromSSMatrix ( mat );
}

CSSMatrix CSSCoordinatesGetEclipticMatrix ( double obliquity )
{
    SSMatrix mat = SSCoordinates::getEclipticMatrix ( obliquity );
    return CSSMatrixFromSSMatrix ( mat );
}

CSSMatrix CSSCoordinatesGetHorizonMatrix ( double lst, double lat )
{
    SSMatrix mat = SSCoordinates::getHorizonMatrix ( lst, lat );
    return CSSMatrixFromSSMatrix ( mat );
}

CSSMatrix CSSCoordinatesGetGalacticMatrix ( void )
{
    SSMatrix mat = SSCoordinates::getGalacticMatrix();
    return CSSMatrixFromSSMatrix ( mat );
}

CSSVector CSSCoordinatessApplyAberration ( CSSCoordinates *pCCoords, CSSVector cdir )
{
    SSVector dir ( cdir.x, cdir.y, cdir.z );
    SSCoordinates *pCoords = (SSCoordinates *) pCCoords;
    if ( pCoords )
        dir = pCoords->applyAberration ( dir );
    return CSSVectorFromSSVector ( dir );
}

CSSVector CSSCoordinatesRemoveAberration ( CSSCoordinates *pCCoords, CSSVector cdir )
{
    SSVector dir ( cdir.x, cdir.y, cdir.z );
    SSCoordinates *pCoords = (SSCoordinates *) pCCoords;
    if ( pCoords )
        dir = pCoords->removeAberration ( dir );
    return CSSVectorFromSSVector ( dir );
}

double CSSCoordinatesRedShiftToRadVel ( double z )
{
    return SSCoordinates::redShiftToRadVel ( z );
}

double CSSCoordinatesRadVelToRedShift ( double rv )
{
    return SSCoordinates::radVelToRedShift ( rv );
}

CSSVector CSSCoordinatesToGeocentric ( CSSSpherical csph, double re, double f )
{
    SSSpherical sph = CSSSphericalToSSSpherical ( csph );
    SSVector vec = SSCoordinates::toGeocentric ( sph, re, f );
    return CSSVectorFromSSVector ( vec );
}

CSSSpherical CSSCoordinatesToGeodetic ( CSSVector cvec, double re, double f )
{
    SSVector vec = CSSVectorToSSVector ( cvec );
    SSSpherical sph = SSCoordinates::toGeodetic ( vec, re, f );
    return CSSSphericalFromSSSpherical ( sph );
}

double CSSCoordinatesRefractionAngle ( double alt, bool a )
{
    return SSCoordinates::refractionAngle ( alt, a );
}

double CSSCoordinatesApplyRefraction ( double alt )
{
    return SSCoordinates::applyRefraction ( alt );
}

double CSSCoordinatesRemoveRefraction ( double alt )
{
    return SSCoordinates::removeRefraction ( alt );
}

// C wrappers for C++ SSIdentifier classes and methods

CSSIdentifier CSSIdentifierFromString ( const char *str )
{
    SSIdentifier ident = SSIdentifier::fromString ( str );
    return ident;
}

const char *CSSIdentifierToString ( CSSIdentifier cident )
{
    static string str = "";
    SSIdentifier ident ( cident );
    str = ident.toString();
    return str.c_str();
}

CSSIdentifier CSSIdentifierFromCatalogNumber ( int catalog, int64_t number )
{
    SSIdentifier ident ( (SSCatalog) catalog, number );
    return ident;
}

char CSSIdentifierGetCatalog ( CSSIdentifier cident )
{
    SSIdentifier ident ( cident );
    return ident.catalog();
}

int64_t CSSIdentifierGetNumber ( CSSIdentifier cident )
{
    SSIdentifier ident ( cident );
    return ident.identifier();
}

// C wrappers for C++ SSJPLDEphemeris classes and methods

static SSJPLDEphemeris _jpldeph;

bool CSSJPLDEphemerisOpen ( const char *filename )
{
    return _jpldeph.open ( string ( filename ) );
}

bool CSSJPLDEphemerisIsOpen ( void )
{
    return _jpldeph.isOpen();
}

void CSSJPLDEphemerisClose ( void )
{
    _jpldeph.close();
}

int CSSJPLDEphemerisGetConstantCount ( void )
{
    return SSJPLDEphemeris::getConstantNumber();
}

const char *CSSJPLDEphemerisGetConstantName ( int i )
{
    static string name = "";
    name = SSJPLDEphemeris::getConstantName ( i );
    return name.c_str();
}

double CSSJPLDEphemerisGetConstantValue ( int i )
{
    return SSJPLDEphemeris::getConstantValue ( i );
}

double CSSJPLDEphemerisGetStartJED ( void )
{
    return _jpldeph.getStartJED();
}

double CSSJPLDEphemerisGetStopJED ( void )
{
    return _jpldeph.getStopJED();
}

double CSSJPLDEphemerisGetStep ( void )
{
    return _jpldeph.getStep();
}

bool CSSJPLDEphemerisCompute ( int planet, double jd, bool bary, CSSVector *cpos, CSSVector *cvel )
{
    SSVector pos ( HUGE_VAL, HUGE_VAL, HUGE_VAL );
    SSVector vel ( HUGE_VAL, HUGE_VAL, HUGE_VAL );

    bool result = _jpldeph.compute ( planet, jd, bary, pos, vel );
    
    memcpy ( cpos, &pos, sizeof ( pos ) );
    memcpy ( cvel, &vel, sizeof ( vel ) );
    
    return result;
}

// C wrappers for C++ SSObject definitions, classes and methods

const char *CSSObjectTypeToCode ( int type )
{
    static string code = "";
    code = SSObject::typeToCode ( (SSObjectType) type );
    return code.c_str();
}

int CSSObjectTypeFromCode ( const char *cstr )
{
    return SSObject::codeToType ( string ( cstr ) );
}

int CSSObjectGetType ( CSSObjectPtr pObject )
{
    SSObject *pObj = (SSObject *) pObject;
    return pObj ? pObj->getType() : kTypeNonexistent;
}

const char *CSSObjectGetName ( CSSObjectPtr pObject, int i )
{
    static string name = "";
    SSObject *pObj = (SSObject *) pObject;
    name = pObj ? pObj->getName ( i ) : "";
    return name.c_str();
}

CSSIdentifier CSSObjectGetIdentifier ( CSSObjectPtr pObject, int cat )
{
    SSObject *pObj = (SSObject *) pObject;
    return pObj ? pObj->getIdentifier ( (SSCatalog) cat ) : SSIdentifier ( 0 );
}

CSSVector CSSObjectGetDirection  ( CSSObjectPtr pObject )
{
    SSObject *pObj = (SSObject *) pObject;
    SSVector dir = pObj ? pObj->getDirection() : SSVector ( HUGE_VAL, HUGE_VAL, HUGE_VAL );
    CSSVector direction = { dir.x, dir.y, dir.z };
    return direction;
}

double CSSObjectGetDistance ( CSSObjectPtr pObject )
{
    SSObject *pObj = (SSObject *) pObject;
    return pObj ? pObj->getDistance() : HUGE_VAL;
}

float CSSObjectGetMagnitude ( CSSObjectPtr pObject )
{
    SSObject *pObj = (SSObject *) pObject;
    return pObj ? pObj->getMagnitude() : HUGE_VAL;
}

void CSSObjectSetDirection  ( CSSObjectPtr pObject, CSSVector dir )
{
    SSVector direction = { dir.x, dir.y, dir.z };
    SSObject *pObj = (SSObject *) pObject;
    if ( pObj )
        pObj->setDirection ( direction );
}

void CSSObjectSetDirection  ( CSSObjectPtr pObject, double distance )
{
    SSObject *pObj = (SSObject *) pObject;
    if ( pObj )
        pObj->setDistance ( distance );
}

void CSSObjectSetMagnitude  ( CSSObjectPtr pObject, float magnitude )
{
    SSObject *pObj = (SSObject *) pObject;
    if ( pObj )
        pObj->setMagnitude ( magnitude );
}

CSSObjectArray *CSSObjectArrayCreate ( void )
{
    SSObjectVec *pObjVec = new SSObjectVec;
    return (CSSObjectArray *) pObjVec;
}

void CSSObjectArrayDestroy ( CSSObjectArray *pObjArr )
{
    SSObjectVec *pObjVec = (SSObjectVec *) pObjArr;
    delete pObjVec;
}

int CSSImportObjectsFromCSV ( const char *filename, CSSObjectArray *pObjArr )
{
    SSObjectVec *pObjVec = (SSObjectVec *) pObjArr;
    return SSImportObjectsFromCSV ( string ( filename ), *pObjVec );
}

size_t CSSObjectArraySize ( CSSObjectArray *pObjArr )
{
    SSObjectVec *pObjVec = (SSObjectVec *) pObjArr;
    return pObjVec->size();
}

CSSObjectPtr CSSObjectGetFromArray ( CSSObjectArray *pObjArr, int i )
{
    SSObjectVec *pObjVec = (SSObjectVec *) pObjArr;
    return (CSSObject *) ( pObjVec && i >= 0 && i < pObjVec->size() ? pObjVec->at(i).get() : nullptr );
}

// C wrappers for C++ SSEvent definitions, classes and methods

CSSAngle CSSEventSemiDiurnalArc ( CSSAngle lat, CSSAngle dec, CSSAngle alt )
{
    return SSEvent::semiDiurnalArc ( lat, dec, alt );
}

CSSTime CSSEventRiseTransitSet ( CSSTime ctime, CSSAngle ra, CSSAngle dec, int sign, CSSAngle lon, CSSAngle lat, CSSAngle alt )
{
    SSTime time ( ctime.jd, ctime.zone );
    time = SSEvent::riseTransitSet ( time, ra, dec, sign, lon, lat, alt );
    return CSSTimeFromSSTime ( time );
}

CSSTime CSSEventRiseTransitSet2 ( CSSTime ctime, CSSCoordinates *pCCoords, CSSObjectPtr pCObj, int sign, CSSAngle alt )
{
    SSTime time ( ctime.jd, ctime.zone );
    SSCoordinates *pCoords = (SSCoordinates *) pCCoords;
    SSObject *pObj = (SSObject *) pCObj;
    if ( pCoords && pObj )
        time = SSEvent::riseTransitSet ( time, *pCoords, SSObjectPtr ( pObj ), sign, alt );
    else
        time.jd = INFINITY;
    return CSSTimeFromSSTime ( time );
}

CSSTime CSSEventRiseTransitSetSearch ( CSSTime ctime, CSSCoordinates *pCCoords, CSSObjectPtr pCObj, int sign, CSSAngle alt )
{
    SSTime time ( ctime.jd, ctime.zone );
    SSCoordinates *pCoords = (SSCoordinates *) pCCoords;
    SSObject *pObj = (SSObject *) pCObj;
    if ( pCoords && pObj )
        time = SSEvent::riseTransitSetSearch ( time, *pCoords, SSObjectPtr ( pObj ), sign, alt );
    else
        time.jd = INFINITY;
    return CSSTimeFromSSTime ( time );
}

CSSTime CSSEventRiseTransitSetSearchDay ( CSSTime ctime, CSSCoordinates *pCCoords, CSSObjectPtr pCObj, int sign, CSSAngle alt )
{
    SSTime time ( ctime.jd, ctime.zone );
    SSCoordinates *pCoords = (SSCoordinates *) pCCoords;
    SSObject *pObj = (SSObject *) pCObj;
    if ( pCoords && pObj )
        time = SSEvent::riseTransitSetSearchDay ( time, *pCoords, SSObjectPtr ( pObj ), sign, alt );
    else
        time.jd = INFINITY;
    return CSSTimeFromSSTime ( time );
}

CSSPass CSSPassFromSSPass ( SSPass pass )
{
    CSSPass cpass =
    {
        { { pass.rising.time.jd,  pass.rising.time.zone  }, pass.rising.azm,  pass.rising.alt },
        { { pass.transit.time.jd, pass.transit.time.zone }, pass.transit.azm, pass.transit.alt },
        { { pass.setting.time.jd, pass.setting.time.zone }, pass.setting.azm, pass.setting.alt }
    };
    
    return cpass;
}

CSSPass CSSEventRiseTransitSet3 ( CSSTime ctime, CSSCoordinates *pCCoords, CSSObjectPtr pCObj, CSSAngle alt )
{
    SSTime time ( ctime.jd, ctime.zone );
    SSCoordinates *pCoords = (SSCoordinates *) pCCoords;
    SSObject *pObj = (SSObject *) pCObj;
    SSPass pass = { { INFINITY, INFINITY, INFINITY }, { INFINITY, INFINITY, INFINITY }, { INFINITY, INFINITY, INFINITY } };
    if ( pCoords && pObj )
        pass = SSEvent::riseTransitSet ( time, *pCoords, SSObjectPtr ( pObj ), alt );
    return CSSPassFromSSPass ( pass );
}

int CSSEventFindSatellitePasses ( CSSCoordinates *pCCoords, CSSObjectPtr pCSat, CSSTime cstart, CSSTime cstop, CSSAngle minAlt, CSSPass cpasses[], int maxPasses )
{
    SSTime start ( cstart.jd, cstart.zone );
    SSTime stop ( cstop.jd, cstop.zone );
    SSCoordinates *pCoords = (SSCoordinates *) pCCoords;
    SSObject *pSat = (SSObject *) pCSat;
    vector<SSPass> passes;
    int nPasses = SSEvent::findSatellitePasses ( *pCoords, SSObjectPtr ( pSat ), start, stop, minAlt, passes, maxPasses );
    for ( int i = 0; i < nPasses; i++ )
        cpasses[i] = CSSPassFromSSPass ( passes[i] );
    return nPasses;
}

CSSTime CSSEventNextMoonPhase ( CSSTime ctime, CSSObjectPtr pCSun, CSSObjectPtr pCMoon, double phase )
{
    SSTime time ( ctime.jd, ctime.zone );
    SSObject *pSun = (SSObject *) pCSun;
    SSObject *pMoon = (SSObject *) pCMoon;
    if ( pSun && pMoon )
        time = SSEvent::nextMoonPhase ( time, SSObjectPtr ( pSun ), SSObjectPtr ( pMoon ), phase );
    else
        time.jd = INFINITY;
    return CSSTimeFromSSTime ( time );
}

/*
void CSSEventFindConjunctions ( CSSCoordinates *pCCoords, CSSObjectPtr pObj1, CSSObjectPtr pObj2, CSSTime start, CSSTime stop, CSSEventTime events[], int maxEvents );
void CSSEventFindOppositions ( CSSCoordinates *pCCoords, CSSObjectPtr pObj1, CSSObjectPtr pObj2, CSSTime start, CSSTime stop, CSSEventTime events[], int maxEvents );
void CSSEventFindNearestDistances ( CSSCoordinates *pCCoords, CSSObjectPtr pObj1, CSSObjectPtr pObj2, CSSTime start, CSSTime stop, CSSEventTime events[], int maxEvents );
void CSSEventFindFarthestDistances ( CSSCoordinates *pCCoords, CSSObjectPtr pObj1, CSSObjectPtr pObj2, CSSTime start, CSSTime stop, CSSEventTime events[], int maxEvents );
*/
