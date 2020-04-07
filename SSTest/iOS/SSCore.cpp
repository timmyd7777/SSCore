//
// SSCore.c
// SSTest
//
// Created by Tim DeBenedictis on 4/7/20.
// Copyright © 2020 Southern Stars. All rights reserved.

#include "SSCore.h"

#include "SSTime.hpp"
#include "SSAngle.hpp"
#include "SSVector.hpp"

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
    SSDegMinSec dms ( cdms.sign, cdms.deg, cdms.min, cdms.sec );
    static string str = dms.toString();
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
    static string str = hms.toString();
    return SSAngle ( hms );
}

const char *CSSHourMinSecToString ( CSSHourMinSec chms )
{
    SSHourMinSec hms ( chms.sign, chms.hour, chms.min, chms.sec );
    static string str = hms.toString();
    return str.c_str();
}

// C wrappers for C++ SSVector classes and methods

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
    cvec.x *= s; cvec.y *= s; cvec.z *= s;
    return cvec;
}

CSSVector CSSVectorDivideBy ( CSSVector cvec, double s )
{
    cvec.x /= s; cvec.y /= s; cvec.z /= s;
    return cvec;
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
