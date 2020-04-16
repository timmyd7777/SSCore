//  SSCoordinates.cpp
//  SSCore
//
//  Created by Tim DeBenedictis on 2/28/20.
//  Copyright © 2020 Southern Stars. All rights reserved.

#include "SSCoordinates.hpp"
#include "SSPlanet.hpp"

#ifndef max
#define max(x,y) (x>y?x:y)
#endif

// Constructs a coordinate transformation object for a specific Julian Date (jd),
// geographic longitude (lon), latitude (lat), and altitude (alt).
// Longitude and latitude are both in radians; east and noth are positive.
// Altitude is in kilometers above the Earth's ellipsoid.

SSCoordinates::SSCoordinates ( double jd, double lon, double lat, double alt )
{
    jed = SSTime ( jd ).getJulianEphemerisDate();

    getNutationConstants ( jd, de, dl );
    this->obq = getObliquity ( jd );
    this->epoch = jd;
    this->lon = lon;
    this->lat = lat;
    this->alt = alt;
    this->lst = SSTime ( jd ).getSiderealTime ( SSAngle ( lon + dl * cos ( obq + de ) ) );
    
    preMat = getPrecessionMatrix ( jd );
    nutMat = getNutationMatrix ( obq, dl, de );
    equMat = nutMat.multiply ( preMat );
    eclMat = getEclipticMatrix ( - obq - de ).multiply ( equMat );
    horMat = getHorizonMatrix ( lst, lat ).multiply ( equMat );
    galMat = getGalacticMatrix();

    SSPlanet::computeMajorPlanetPositionVelocity ( kEarth, jed, 0.0, obsPos, obsVel );
    
    SSSpherical geodetic ( lst, lat, alt );
    SSVector geocentric = toGeocentric ( geodetic, kKmPerEarthRadii, kEarthFlattening );
    
    geocentric = fromEquatorial ( geocentric );
    obsPos = obsPos.add ( geocentric / kKmPerAU );
    
    starParallax = true;
    starMotion = true;
    aberration = true;
    lighttime = true;
}

// Computes constants needed to compute precession from J2000 to a specific Julian Date (jd).
// From Jean Meeus, "Astronomical Algorithms", ch 21., p. 134.

void SSCoordinates::getPrecessionConstants ( double jd, double &zeta, double &z, double &theta )
{
    double t = ( jd - SSTime::kJ2000 ) / 36525.0;
    double t2 = t * t;
    double t3 = t * t2;

    zeta  = SSAngle::fromArcsec ( 2306.2181 * t + 0.30188 * t2 + 0.017998 * t3 );
    z     = SSAngle::fromArcsec ( 2306.2181 * t + 1.09468 * t2 + 0.018203 * t3 );
    theta = SSAngle::fromArcsec ( 2004.3109 * t - 0.42665 * t2 - 0.041833 * t3 );
}

// Computes constants needed to compute nutation from J2000 to a specific Julian date (jd).
// From Jean Meeus, "Astronomical Algorithms", ch. 22, p. 144.

void SSCoordinates::getNutationConstants ( double jd, double &de, double &dl )
{
    double t, n, l, l1, sn, cn, s2n, c2n, s2l, c2l, s2l1, c2l1;
      
    t = ( jd - SSTime::kJ2000 ) / 36525.0;
    n  = SSAngle::fromDegrees ( 125.0445 -   1934.1363 * t ).mod2Pi();
    l  = SSAngle::fromDegrees ( 280.4665 +  36000.7698 * t ).mod2Pi() * 2.0;
    l1 = SSAngle::fromDegrees ( 218.3165 + 481267.8813 * t ).mod2Pi() * 2.0;
    
    sn   = sin ( n );
    cn   = cos ( n );
    s2n  = 2.0 * sn * cn;
    c2n  = cn * cn - sn * sn;
    s2l  = sin ( l );
    c2l  = cos ( l );
    s2l1 = sin ( l1 );
    c2l1 = cos ( l1 );
    
    dl = SSAngle::fromArcsec ( -17.20 * sn - 1.32 * s2l - 0.23 * s2l1 + 0.21 * s2n );
    de = SSAngle::fromArcsec (   9.20 * cn + 0.57 * c2l + 0.10 * c2l1 - 0.09 * c2n );
}

// Computes the mean obliquity of the ecliptic (i.e. angle between Earth's equatorial and orbital
// planes) at any epoch (expressed as a Julian Date) from 1600 to 2100.  Does not include nutation!

double SSCoordinates::getObliquity ( double jd )
{
    double t = ( jd - SSTime::kJ2000 ) / 36525.0;
    double e = 23.439291 + t * ( -0.0130042 + t * ( -0.00000016 + t * 0.000000504 ) );
      
    return SSAngle::fromDegrees ( e );
}

// Returns a rotation matrix for transforming rectangular coordinates from the
// fundamental J2000 mean equatorial frame to the precessed equatorial frame
// at the specified epoch (expressed as a Julian Date, jd). Does not include nutation!

SSMatrix SSCoordinates::getPrecessionMatrix ( double jd )
{
    double zeta = 0.0, z = 0.0, theta = 0.0;
    getPrecessionConstants ( jd, zeta, z, theta );
    return SSMatrix::rotation ( 3, 2, zeta, 1, theta, 2, z );
}

// Returns a rotation matrix which corrects equatorial coordinates for nutation,
// i.e. transforming rectangular coordinates from the mean to the true equatorial frame.
// The mean obliquity of the ecliptic is obq; the nutation in longitude and obliquity
//  are nutLon and nutObj, all in radians.

SSMatrix SSCoordinates::getNutationMatrix ( double obq, double nutLon, double nutObq )
{
    return SSMatrix::rotation ( 3, 0, -obq, 2, nutLon, 0, obq + nutObq );
}

// Returns a rotation matrix for transforming rectangular coordinates from the
// ecliptic to the equatorial frame, where oliquity is the angle in radians between
// the ecliptic and equatorial planes (i.e., the Earth's orbital and equatorial planes).
// Pass negative obliquity to get matrix for transforming equatorial -> ecliptic.

SSMatrix SSCoordinates::getEclipticMatrix ( double obliquity )
{
    return SSMatrix::rotation ( 1, 0, obliquity );
}

// Returns a rotation matrix for transforming rectangular coordinates from the
// current true equatorial frame to the local horizon frame, given the local sidereal
// time (lst) and latitude (lat), both in radians. Note we negate the middle row
// of the matrix because horizon coordinates are left-handed!

SSMatrix SSCoordinates::getHorizonMatrix ( double lst, double lat )
{
    SSMatrix m = SSMatrix::rotation ( 2, 2, SSAngle::kPi - lst, 1, lat - SSAngle::kHalfPi );
    
    m.m10 = -m.m10; m.m11 = -m.m11; m.m12 = -m.m12;
    
    return ( m );
}

// Returns a rotation matrix for transforming rectangular coordinates
// from the fundamental J2000 mean equatorial to the galactic frame.
// From J.C Liu et al, "Reconsidering the Galactic Coordinate System",
// https://www.aanda.org/articles/aa/full_html/2011/02/aa14961-10/aa14961-10.html

SSMatrix SSCoordinates::getGalacticMatrix ( void )
{
    return SSMatrix ( -0.054875539390, -0.873437104725, -0.483834991775,
                      +0.494109453633, -0.444829594298, +0.746982248696,
                      -0.867666135681, -0.198076389622, +0.455983794523 );
}

// Given a rectangular coordinate vector in the fundamental frame,
// returns a copy of that vector transformed to the current equatorial frame.

SSVector SSCoordinates::toEquatorial ( SSVector funVec )
{
    return equMat * funVec;
}

// Given a rectangular coordinate vector in the fundamental frame,
// returns a copy of that vector transformed to the ecliptic frame.

SSVector SSCoordinates::toEcliptic ( SSVector funVec )
{
    return eclMat * funVec;
}

// Given a rectangular coordinate vector in the fundamental frame,
// returns a copy of that vector transformed to the current local horizon frame.

SSVector SSCoordinates::toHorizon ( SSVector funVec )
{
    return horMat * funVec;
}

// Given a rectangular coordinate vector in the fundamental frame,
// returns a copy of that vector transformed to the galactic frame.

SSVector SSCoordinates::toGalactic ( SSVector funVec )
{
    return galMat * funVec;
}

// Given a rectangular coordinate vector in the current equatorial frame,
// returns a copy of that vector transformed to the fundamental frame.

SSVector SSCoordinates::fromEquatorial ( SSVector equVec )
{
    return equMat.transpose() * equVec;
}

// Given a rectangular coordinate vector in the current ecliptic frame,
// returns a copy of that vector transformed to the fundamental frame.

SSVector SSCoordinates::fromEcliptic ( SSVector eclVec )
{
    return eclMat.transpose() * eclVec;
}

// Given a rectangular coordinate vector in the galactic frame,
// returns a copy of that vector transformed to the fundamental frame.

SSVector SSCoordinates::fromGalactic ( SSVector galVec )
{
    return galMat.transpose() * galVec;
}

// Given a rectangular coordinate vector in the current local horizon frame,
// returns a copy of that vector transformed to the fundamental frame.

SSVector SSCoordinates::fromHorizon ( SSVector horVec )
{
    return horMat.transpose() * horVec;
}

// Given a spherical coordinates in the fundamental frame, returns a copy
// of those spherical coords transformed to the current equatorial frame.

SSSpherical SSCoordinates::toEquatorial ( SSSpherical fun )
{
    return SSSpherical ( toEquatorial ( SSVector ( fun ) ) );
}

// Given a spherical coordinates in the fundamental frame, returns a copy
// of those spherical coords transformed to the current ecliptic frame.

SSSpherical SSCoordinates::toEcliptic ( SSSpherical fun )
{
    return SSSpherical ( toEcliptic ( SSVector ( fun ) ) );
}

// Given a spherical coordinates in the fundamental frame, returns a copy
// of those spherical coords transformed to the galactic frame.

SSSpherical SSCoordinates::toGalactic ( SSSpherical fun )
{
    return SSSpherical ( toGalactic ( SSVector ( fun ) ) );
}

// Given a spherical coordinates in the fundamental frame, returns a copy
// of those spherical coords transformed to the current local horizon frame.

SSSpherical SSCoordinates::toHorizon ( SSSpherical fun )
{
    return SSSpherical ( toHorizon ( SSVector ( fun ) ) );
}

// Given a spherical coordinates in the current equatorial frame,
// returns a copy transformed to the fundamental frame.

SSSpherical SSCoordinates::fromEquatorial ( SSSpherical equ )
{
    return SSSpherical ( fromEquatorial ( SSVector ( equ ) ) );
}

// Given a spherical coordinates in the current ecliptic frame,
// returns a copy transformed to the fundamental frame.

SSSpherical SSCoordinates::fromEcliptic ( SSSpherical ecl )
{
    return SSSpherical ( fromEcliptic ( SSVector ( ecl ) ) );
}

// Given a spherical coordinates in the galactic frame,
// returns a copy transformed to the fundamental frame.

SSSpherical SSCoordinates::fromGalactic ( SSSpherical gal )
{
    return SSSpherical ( fromGalactic ( SSVector ( gal ) ) );
}

// Given a spherical coordinates in the current local horizon frame,
// returns a copy transformed to the fundamental frame.

SSSpherical SSCoordinates::fromHorizon ( SSSpherical hor )
{
    return SSSpherical ( fromHorizon ( SSVector ( hor ) ) );
}


// Converts geodetic longitude, latitude, altitude to geocentric X, Y, Z vector.
// geodetic.lon and .lat are in radians; geo.rad is altitude above geoid in same
// units as equatorial radius of geoid ellipse (a). Geoid flattening (f) is ratio
// (a - b)/(a), where b is polar radius of geoid ellipse. Rectangular XYZ vector
// is returned in same units as (a).
// Formula from "The Astronomical Almanac for the Year 1990", pp. K11-K13.

SSVector SSCoordinates::toGeocentric ( SSSpherical geodetic, double a, double f )
{
    double c, s, cp = cos ( geodetic.lat ), sp = sin ( geodetic.lat );

    f = ( 1.0 - f ) * ( 1.0 - f );
    c = 1.0 / sqrt ( cp * cp + f * sp * sp );
    s = f * c;
    
    double x = ( a * c + geodetic.rad ) * cp * cos ( geodetic.lon );
    double y = ( a * c + geodetic.rad ) * cp * sin ( geodetic.lon );
    double z = ( a * s + geodetic.rad ) * sp;
    
    return SSVector ( x, y, z );
}

// Converts geocentric X,Y,Z vector to geodetic longitude, latitude altitude.
// Geoid equatorial radius (a) and flattening (f) are as for SSDynamis::toGeocentric().
// Algorithm is from "The Astronomical Almanac for the Year 1990", pp. K11-K13.

SSSpherical SSCoordinates::toGeodetic ( SSVector geocentric, double a, double f )
{
    double x = geocentric.x, y = geocentric.y, z = geocentric.z;
    double r = sqrt ( x * x + y * y );
    double e2 = 2.0 * f - f * f, s, c;
    double lon = SSAngle::atan2Pi ( y, x );
    double lat = atan2 ( z, r ), lat1 = lat;

    if ( r > 0.0 )
    {
        do
        {
            lat1 = lat;
            s = sin ( lat1 );
            c = 1.0 / sqrt ( 1.0 - e2 * s * s );
            lat = atan ( ( z + a * c * e2 * s ) / r );
        }
        while ( fabs ( lat1 - lat ) > 1.0e-8 );
    }
      else
    {
        lat = z == 0.0 ? 0.0 : z > 0.0 ? SSAngle::kHalfPi : -SSAngle::kHalfPi;
        c = 1.0 / ( 1.0 - f );
    }
    
    double h = r / cos ( lat ) - a * c;
    return SSSpherical ( lon, lat, h );
}

// Applies aberration of light to an apparent direction unit vector (p)
// in the fundamental J2000 equatorial frame. Returns the "aberrated"
// vector; p itself is not modified. Uses relativatic formula from
// The Explanatory Supplement to the Astronomical Almanac, p. 129.
// Observer's heliocentric velocity must have been calcualted previously!

SSVector SSCoordinates::applyAberration ( SSVector p )
{
    SSVector v = obsVel / kLightAUPerDay;
    
    double beta = sqrt ( 1.0 - v * v );
    double dot = v * p;
    double s = 1.0 + dot / ( 1.0 + beta );
    double n = 1.0 + dot;
    double px = ( p.x * beta + v.x * s ) / n;
    double py = ( p.y * beta + v.y * s ) / n;
    double pz = ( p.z * beta + v.z * s ) / n;

    return SSVector ( px, py, pz );
}

// Removes aberration from an apparent unit direction vector (p)
// in the fundamental J2000 equatorial frame. Returns the "un-aberrated"
// vector; p itself is not modified. Uses non-relativistic approximation.

SSVector SSCoordinates::removeAberration ( SSVector p )
{
    return ( p - obsVel / kLightAUPerDay ).normalize();
}

// Given a positive or negative red shift (z), returns the equivalent radial velocity
// as a fraction of light speed (rv) using relativistic formula.
// Redshifts > 1.0 return radial velocities < 1.0.

double SSCoordinates::redShiftToRadVel ( double z )
{
    double z12 = ( z + 1.0 ) * ( z + 1.0 );
    double rv = ( z12 - 1.0 ) / ( z12 + 1.0 );
    
    return ( rv );
}

// Given positive or negative radial velocity as a fraction of light speed (rv),
// returns the equivalent red shift (z) using relativistic formula.
// Radial velocity must be < 1.0 but may return redshift > 1.0.

double SSCoordinates::radVelToRedShift ( double rv )
{
    return sqrt ( ( 1.0 + rv ) / ( 1.0 - rv ) ) - 1.0;
}

// Computes atmospheric refraction angle at a particular altitude in radians.
// The boolean a should be true if alt is a true (geometric, un-refracted) altitude,
// and false if alt is an apparent (refracted) altitude.  This formula assumes
// standard atmospheric pressure and temperature of 1010 millibars and +10 deg C.

SSAngle SSCoordinates::refractionAngle ( SSAngle alt, bool a )
{
    double    h, r;
    
    if ( a == true )
    {
        h = max ( alt.toDegrees(), -1.9 );
        r = 1.02 / tan ( SSAngle::fromDegrees ( h + ( 10.3 / ( h + 5.11 ) ) ) );
    }
    else
    {
        h = max ( alt.toDegrees(), -1.7 );
        r = 1.0 / tan ( SSAngle::fromDegrees ( h + ( 7.31 / ( h + 4.4 ) ) ) );
    }
    
    return SSAngle::fromArcmin ( r );
}

// Returns refracted (apparent) altitude from true (geometric) altitude

SSAngle SSCoordinates::applyRefraction ( SSAngle alt )
{
    return alt + SSCoordinates::refractionAngle ( alt, true );
}

// Returns true (geometric) from refracted (apparent) altitude

SSAngle SSCoordinates::removeRefraction ( SSAngle alt )
{
    return alt - SSCoordinates::refractionAngle ( alt, false );
}
