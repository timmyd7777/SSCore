//
// SSCore.c
// SSTest
//
// Created by Tim DeBenedictis on 4/7/20.
// Copyright © 2020 Southern Stars. All rights reserved.

#include "SSCore.h"

#include "SSTime.hpp"
#include "SSAngle.hpp"
#include "SSIdentifier.hpp"
#include "SSJPLDEphemeris.hpp"
#include "SSVector.hpp"
#include "SSMatrix.hpp"
#include "SSObject.hpp"

// C wrappers for C++ SSTime classes and methods

CSSDate CSSTimeToCSSDate ( CSSTime ctime )
{
    SSDate date ( SSTime ( ctime.jd, ctime.zone, (SSCalendar) ctime.calendar ) );
    CSSDate cdate = { date.calendar, date.zone, date.year, date.month, date.day, date.hour, date.min, date.sec };
    return cdate;
}

CSSTime CSSDateToCSSTime ( CSSDate cdate )
{
    SSTime time ( SSDate ( (SSCalendar) cdate.calendar, cdate.zone, cdate.year, cdate.month, cdate.day, cdate.hour, cdate.min, cdate.sec ) );
    CSSTime ctime = { time.jd, time.zone, time.calendar };
    return ctime;
}

CSSTime CSSTimeFromSystem ( void )
{
    SSTime now = SSTime::fromSystem();
    CSSTime cnow = { now.jd, now.zone, now.calendar };
    return cnow;
}

CSSTime CSSTimeFromCalendarDate ( CSSDate cdate )
{
    SSTime time ( SSDate ( (SSCalendar) cdate.calendar, cdate.zone, cdate.year, cdate.month, cdate.day, cdate.hour, cdate.min, cdate.sec ) );
    CSSTime ctime = { time.jd, time.zone, time.calendar };
    return ctime;
}

CSSTime CSSTimeFromUnixTime ( time_t utime )
{
    SSTime time = SSTime::fromUnixTime ( utime );
    CSSTime ctime = { time.jd, time.zone, time.calendar };
    return ctime;
}

CSSTime CSSTimeFromJulianYear ( double year )
{
    SSTime time = SSTime::fromJulianYear ( year );
    CSSTime ctime = { time.jd, time.zone, time.calendar };
    return ctime;
}

CSSTime CSSTimeFromBesselianYear ( double year )
{
    SSTime time = SSTime::fromBesselianYear ( year );
    CSSTime ctime = { time.jd, time.zone, time.calendar };
    return ctime;
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
    CSSHourMinSec chms = { hms.sign, hms.hour, hms.min, hms.sec };
    return chms;
}

CSSHourMinSec CSSHourMinSecFromString ( const char *str )
{
    SSHourMinSec hms ( str );
    CSSHourMinSec chms = { hms.sign, hms.hour, hms.min, hms.sec };
    return chms;
}

CSSAngle CSSHourMinSecToRadians ( CSSHourMinSec chms )
{
    SSHourMinSec hms ( chms.sign, chms.hour, chms.min, chms.sec );
    return SSAngle ( hms );
}

const char *CSSHourMinSecToString ( CSSHourMinSec chms )
{
    static string str = "";
    SSHourMinSec hms ( chms.sign, chms.hour, chms.min, chms.sec );
    str = hms.toString();
    return str.c_str();
}

// C wrappers for C++ SSVector classes and methods

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
    CSSVector cvec = { vec.x, vec.y, vec.z };
    return cvec;
}

CSSSpherical CSSVectorToCSSSpherical ( CSSVector cvec )
{
    SSSpherical sph ( SSVector ( cvec.x, cvec.y, cvec.z ) );
    CSSSpherical csph = { sph.lon, sph.lat, sph.rad };
    return csph;
}

CSSVector CSSSphericalCSSVectorVelocity ( CSSSpherical cpos, CSSSpherical cvel )
{
    SSSpherical spos ( cpos.lon, cpos.lat, cpos.rad );
    SSSpherical svel ( cvel.lon, cvel.lat, cvel.rad );
    SSVector vvel = spos.toVectorVelocity ( svel );
    CSSVector cvec = { vvel.x, vvel.y, vvel.z };
    return cvec;
}

CSSSpherical CSSVectorToCSSSphericalVelocity ( CSSVector cpos, CSSVector cvel )
{
    SSVector vpos ( cpos.x, cpos.y, cpos.z );
    SSVector vvel ( cvel.x, cvel.y, cvel.z );
    SSSpherical svel = vpos.toSphericalVelocity ( vvel );
    CSSSpherical csph = { svel.lon, svel.lat, svel.rad };
    return csph;
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
    cvec.x = vec.x; cvec.y = vec.y; cvec.z = vec.z;
    return cvec;
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
    CSSVector cx = { x.x, x.y, x.z };
    return cx;
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
    SSMatrix mat =
    {
        cmat.m00, cmat.m01, cmat.m02,
        cmat.m10, cmat.m11, cmat.m12,
        cmat.m20, cmat.m21, cmat.m22
    };
    
    mat = mat.inverse();
    
    CSSMatrix cminv =
    {
        mat.m00, mat.m01, mat.m02,
        mat.m10, mat.m11, mat.m12,
        mat.m20, mat.m21, mat.m22
    };
    
    return cminv;
}

double CSSMatrixDeterminant ( CSSMatrix cmat )
{
    SSMatrix mat =
    {
        cmat.m00, cmat.m01, cmat.m02,
        cmat.m10, cmat.m11, cmat.m12,
        cmat.m20, cmat.m21, cmat.m22
    };
    
    return mat.determinant();
}

CSSMatrix CSSMatrixMultiplyMatrix ( CSSMatrix cmat1, CSSMatrix cmat2 )
{
    SSMatrix mat1 =
    {
        cmat1.m00, cmat1.m01, cmat1.m02,
        cmat1.m10, cmat1.m11, cmat1.m12,
        cmat1.m20, cmat1.m21, cmat1.m22
    };
    
    SSMatrix mat2 =
    {
        cmat2.m00, cmat2.m01, cmat2.m02,
        cmat2.m10, cmat2.m11, cmat2.m12,
        cmat2.m20, cmat2.m21, cmat2.m22
    };

    SSMatrix mat3 = mat1 * mat2;
    
    CSSMatrix cmat3 =
    {
        mat3.m00, mat3.m01, mat3.m02,
        mat3.m10, mat3.m11, mat3.m12,
        mat3.m20, mat3.m21, mat3.m22
    };
    
    return cmat3;
}

CSSVector CSSMatrixMultiplyVector ( CSSMatrix cmat, CSSVector cvec )
{
    SSMatrix mat =
    {
        cmat.m00, cmat.m01, cmat.m02,
        cmat.m10, cmat.m11, cmat.m12,
        cmat.m20, cmat.m21, cmat.m22
    };
    
    SSVector vec = { cvec.x, cvec.y, cvec.z };
    vec = mat * vec;
    
    CSSVector cvec1 = { vec.x, vec.y, vec.z };
    return cvec1;
}

CSSMatrix CSSMatrixRotate ( CSSMatrix cmat, int axis, double angle )
{
    SSMatrix mat =
    {
        cmat.m00, cmat.m01, cmat.m02,
        cmat.m10, cmat.m11, cmat.m12,
        cmat.m20, cmat.m21, cmat.m22
    };
    
    mat = mat.rotate ( axis, angle );
    
    CSSMatrix cmatr =
    {
        mat.m00, mat.m01, mat.m02,
        mat.m10, mat.m11, mat.m12,
        mat.m20, mat.m21, mat.m22
    };
    
    return cmatr;
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
