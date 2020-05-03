//  SSCoordinates.cpp
//  SSCore
//
//  Created by Tim DeBenedictis on 2/28/20.
//  Copyright © 2020 Southern Stars. All rights reserved.

#include "SSCoordinates.hpp"
#include "SSPlanet.hpp"

#define NEW_PRECESSION 1    // 1 to use new long-term precession, 0 to use IAU 1976 precession.

// Constructs a coordinate transformation object for a specific Julian Date (time),
// geographic longitude (loc.lon), latitude (loc.lat), and altitude (loc.rad).
// Longitude and latitude are both in radians; east and noth are positive.
// Altitude is in kilometers above the Earth's ellipsoid.

SSCoordinates::SSCoordinates ( SSTime time, SSSpherical loc )
{
    _lon = loc.lon;
    _lat = loc.lat;
    _alt = loc.rad;
    
    setTime ( time );
    
    starParallax = true;
    starMotion = true;
    aberration = true;
    lighttime = true;
}

// Changes this coordinate transformation object's Julian Date (time) and recomputes
// all of its time-dependent quantites and matrices, without changing the observer's
// longitude, latitude, or altitude.

void SSCoordinates::setTime ( SSTime time )
{
    _jd = time.jd;
    _jed = time.getJulianEphemerisDate();

    getNutationConstants ( _jd, _de, _dl );
    _obq = getObliquity ( _jd );
    _lst = time.getSiderealTime ( SSAngle ( _lon + _dl * cos ( _obq + _de ) ) );
    
    _preMat = getPrecessionMatrix ( _jd );
    _nutMat = getNutationMatrix ( _obq, _dl, _de );
    _equMat = _nutMat * ( _preMat );
    _eclMat = getEclipticMatrix ( - _obq - _de ) * _equMat;
    _galMat = getGalacticMatrix();

    setLocation ( SSSpherical ( _lon, _lat, _alt ) );
}

// Changes this coordinate transformation object's observer longitude (loc.lon), latitude (loc.lat),
// and altitude (loc.rad); and recomputes all of its location-dependent quantites and matrices,
// without changing the time.  Longitude and latitude in radians; altitude in kilometers.

void SSCoordinates::setLocation ( SSSpherical loc )
{
    _lon = loc.lon;
    _lat = loc.lat;
    _alt = loc.rad;
    _lst = SSTime ( _jd ).getSiderealTime ( SSAngle ( _lon + _dl * cos ( _obq + _de ) ) );
    
    _horMat = getHorizonMatrix ( _lst, _lat ).multiply ( _equMat );

    SSPlanet::computeMajorPlanetPositionVelocity ( kEarth, _jed, 0.0, obsPos, obsVel );
    
    SSSpherical geodetic ( _lst, _lat, _alt );
    SSVector geocentric = toGeocentric ( geodetic, kKmPerEarthRadii, kEarthFlattening );
    
    geocentric = transform ( kEquatorial, kFundamental, geocentric );
    obsPos = obsPos.add ( geocentric / kKmPerAU );
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

// Computes nutation in longitude (dl) and in obliquity (de), both in radians,
// at a given Julian Date (jd) using the IAU 1980 theory of nutation.
// This approximation uses linear terms in the fundamental arguments, and the
// four largest periodic terms, giving an accuracy of 0.5 arcsec (for dl) and
// 0.1 arcsec (for de). From Jean Meeus, "Astronomical Algorithms", ch. 22, p. 144.

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

#if NEW_PRECESSION

SSVector getEclipticPoleVector ( double jed )
{
    // Polynomials
    
    static double pqpol[2][4] =
    {
        { +5851.607687, -0.1189000, -0.00028913, +0.000000101 },
        { -1600.886300, +1.1689818, -0.00000020, -0.000000437 }
    };

    // Periodics
    
    static double pqper[8][5] =
    {
        {  708.15, -5486.751211, -684.661560,   667.666730, -5523.863691 },
        { 2309.00,   -17.127623, 2446.283880, -2354.886252,  -549.747450 },
        { 1620.00,  -617.517403,  399.671049,  -428.152441,  -310.998056 },
        {  492.20,   413.442940, -356.652376,   376.202861,   421.535876 },
        { 1183.00,    78.614193, -186.387003,   184.778874,   -36.776172 },
        {  622.00,  -180.732815, -316.800070,   335.321713,  -145.278396 },
        {  882.00,   -87.676083,  198.296071,  -185.138669,   -34.744450 },
        {  547.00,    46.140315,  101.135679,  -120.972830,    22.885731 }
    };
    
    // Obliquity at J2000.0 (radians)
    
    static double eps0 = 84381.406 * SSAngle::kRadPerArcsec;

    // centuries since J2000
    
    double t = ( jed - SSTime::kJ2000 ) / 36525.0;
    
    // initialize P_A and Q_A accumulators
    
    double p = 0.0, q = 0.0;
    
    // Periodic terms.
    
    for ( int i = 0; i < 8; i++ )
    {
        double w = SSAngle::kTwoPi * t;
        double a = w / pqper[i][0];
        double s = sin ( a );
        double c = cos ( a );
        p += c * pqper[i][1] + s * pqper[i][3];
        q += c * pqper[i][2] + s * pqper[i][4];
    }
    
    // Polynomial terms.
    
    double w = 1.0;
    for ( int i = 0; i < 4; i++ )
    {
        p += pqpol[0][i] * w;
        q += pqpol[1][i] * w;
        w *= t;
    }
    
    // P_A and Q_A (radians).
    
    p *= SSAngle::kRadPerArcsec;
    q *= SSAngle::kRadPerArcsec;
    
    // Form the ecliptic pole vector.
    
    double z = 1.0 - p * p - q * q;
    z = z > 0.0 ? sqrt ( z ) : 0.0;

    double s = sin ( eps0 );
    double c = cos ( eps0 );
    
    return SSVector ( p, - q * c - z * s, -q * s + z * c );
}

SSVector getEquatorPoleVector ( double jed )
{
    // Polynomials
    
    static double xypol[2][4] =
    {
        {  +5453.282155, +0.4252841, -0.00037173, -0.000000152 },
        { -73750.930350, -0.7675452, -0.00018725, +0.000000231 }
    };

    // Periodics
    
    static double xyper[14][5] =
    {
        {  256.75,  -819.940624, 75004.344875,  81491.287984,   1558.515853 },
        {  708.15, -8444.676815,   624.033993,    787.163481,   7774.939698 },
        {  274.20,  2600.009459,  1251.136893,   1251.296102,  -2219.534038 },
        {  241.45,  2755.175630, -1102.212834,  -1257.950837,  -2523.969396 },
        { 2309.00,  -167.659835, -2660.664980,  -2966.799730,    247.850422 },
        {  492.20,   871.855056,   699.291817,    639.744522,   -846.485643 },
        {  396.10,    44.769698,   153.167220,    131.600209,  -1393.124055 },
        {  288.90,  -512.313065,  -950.865637,   -445.040117,    368.526116 },
        {  231.10,  -819.415595,   499.754645,    584.522874,    749.045012 },
        { 1610.00,  -538.071099,  -145.188210,    -89.756563,    444.704518 },
        {  620.00,  -189.793622,   558.116553,    524.429630,    235.934465 },
        {  157.87,  -402.922932,   -23.923029,    -13.549067,    374.049623 },
        {  220.30,   179.516345,  -165.405086,   -210.157124,   -171.330180 },
        { 1200.00,    -9.814756,     9.344131,    -44.919798,    -22.899655 }
    };
    
    // centuries since J2000
    
    double t = ( jed - SSTime::kJ2000 ) / 36525.0;
    
    // initialize X and Y accumulators
    
    double x = 0.0, y = 0.0;
    
    // Periodic terms.
    
    for ( int i = 0; i < 14; i++ )
    {
        double w = SSAngle::kTwoPi * t;
        double a = w / xyper[i][0];
        double s = sin ( a );
        double c = cos ( a );
        x += c * xyper[i][1] + s * xyper[i][3];
        y += c * xyper[i][2] + s * xyper[i][4];
    }
    
    // Polynomial terms.
    
    double w = 1.0;
    for ( int i = 0; i < 4; i++ )
    {
        x += xypol[0][i] * w;
        y += xypol[1][i] * w;
        w *= t;
    }
    
    // X and Y (direction cosines).
    
    x *= SSAngle::kRadPerArcsec;
    y *= SSAngle::kRadPerArcsec;
    
    // Form the equator pole vector.
    
    double z = x * x + y * y;
    z = z < 1.0 ? sqrt ( 1.0 - z ) : 0.0;

    return SSVector ( x, y, z );
}

// From "New precession expressions, valid for long time intervals",
// J. Vondrak, N. Capitaine, and P. Wallace, Astronomy & Astrophysics 534, A22 (2011)
// Accuracy comparable to IAU 2006 precession model around the central epoch J2000.0,
// a few arcseconds throughout the historical period, and a few tenths of a degree
// at the ends of the +/-200 millennia time span.

SSMatrix SSCoordinates::getPrecessionMatrix ( double jed )
{
    SSVector vec = getEclipticPoleVector ( jed );
    SSVector veq = getEquatorPoleVector ( jed );
    
    SSVector eqx = veq.crossProduct ( vec ).normalize();
    SSVector mid = veq.crossProduct ( eqx );
    
    return SSMatrix ( eqx.x, eqx.y, eqx.z,
                      mid.x, mid.y, mid.z,
                      veq.x, veq.y, veq.z );
}

#else

// IAU 1976 expression for precession, from Jean Meeus,
// "Astronomical Algorithms", ch. 21, p 134.
// Valid only for a few centuries around the basic epoch, J2000.0.

SSMatrix SSCoordinates::getPrecessionMatrix ( double jd )
{
    double zeta = 0.0, z = 0.0, theta = 0.0;
    getPrecessionConstants ( jd, zeta, z, theta );
    return SSMatrix::rotation ( 3, 2, zeta, 1, theta, 2, z );
}

#endif

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

// Transforms a rectangular coordinate vector from one reference frame to another.
// Returns transformed vector; does not modify input vector.

SSVector SSCoordinates::transform ( SSFrame from, SSFrame to, SSVector vec )
{
    if ( from != to )
    {
        if ( from == kEquatorial )
            vec = _equMat.transpose() * vec;
        else if ( from == kEcliptic )
            vec = _eclMat.transpose() * vec;
        else if ( from == kGalactic )
            vec = _galMat.transpose() * vec;
        else if ( from == kHorizon )
            vec = _horMat.transpose() * vec;
        
        if ( to == kEquatorial )
            vec = _equMat * vec;
        else if ( to == kEcliptic )
            vec = _eclMat * vec;
        else if ( to == kGalactic )
            vec = _galMat * vec;
        else if ( to == kHorizon )
            vec = _horMat * vec;
    }
    
    return vec;
}

// Transforms spherical coordinates from one reference frame to another.
// Returns transformed spherical coordinates; does not modify input coords.

SSSpherical SSCoordinates::transform ( SSFrame from, SSFrame to, SSSpherical sph )
{
    SSVector vec ( sph );
    vec = transform ( from, to, vec );
    return SSSpherical ( vec );
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
        h = maximum ( alt.toDegrees(), -1.9 );
        r = 1.02 / tan ( SSAngle::fromDegrees ( h + ( 10.3 / ( h + 5.11 ) ) ) );
    }
    else
    {
        h = maximum ( alt.toDegrees(), -1.7 );
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
