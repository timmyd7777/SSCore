//
//  SSCoords.cpp
//  SSCore
//
//  Created by Tim DeBenedictis on 2/28/20.
//  Copyright © 2020 Southern Stars. All rights reserved.

#include "SSCoords.hpp"

// Constructs a coordinate transformation object for a specific
// Julian Date (jd) and geographic longtiude/latitude (both in radians,
// east and noth are positive), optionally including nutation (nutate).

SSCoords::SSCoords ( double jd, bool nutate, double lon, double lat )
{
    if ( nutate )
        getNutationConstants ( de, dl );
    else
        de = dl = 0;
    
    this->obq = getObliquity();
	this->epoch = jd;
    this->lon = lon;
    this->lat = lat;
    this->lst = SSTime ( jd ).getSiderealTime ( SSAngle ( lon + dl ) ).rad;
    
    equMat = getFundamentalToEquatorialMatrix();
    eclMat = getEquatorialToEclipticMatrix().multiply ( equMat );
    horMat = getEquatorialToHorizonMatrix().multiply ( equMat );
    galMat = getFundamentalToGalacticMatrix();
}

// Computes constants needed to compute precession from J2000 to this coordindate
// transformation object's current Julian date.
// From Jean Meeus, "Astronomical Algorithms", ch 21., p. 134.

void SSCoords::getPrecessionConstants ( double &zeta, double &z, double &theta )
{
    double t = ( epoch - SSTime::kJ2000 ) / 36525.0;
    double t2 = t * t;
    double t3 = t * t2;

    zeta  = SSAngle::fromArcsec ( 2306.2181 * t + 0.30188 * t2 + 0.017998 * t3 ).rad;
    z     = SSAngle::fromArcsec ( 2306.2181 * t + 1.09468 * t2 + 0.018203 * t3 ).rad;
    theta = SSAngle::fromArcsec ( 2004.3109 * t - 0.42665 * t2 - 0.041833 * t3 ).rad;
}

// Computes constants needed to compute nutation from J2000 to this coordindate
// transformation object's current Julian date.
// From Jean Meeus, "Astronomical Algorithms", ch. 22, p. 144.

void SSCoords::getNutationConstants ( double &de, double &dl )
{
    double t, n, l, l1, sn, cn, s2n, c2n, s2l, c2l, s2l1, c2l1;
      
    t = ( epoch - SSTime::kJ2000 ) / 36525.0;
    n  = SSAngle::fromDegrees ( 125.0445 -   1934.1363 * t ).mod2Pi().rad;
    l  = SSAngle::fromDegrees ( 280.4665 +  36000.7698 * t ).mod2Pi().rad * 2.0;
    l1 = SSAngle::fromDegrees ( 218.3165 + 481267.8813 * t ).mod2Pi().rad * 2.0;
    
    sn   = sin ( n );
    cn   = cos ( n );
    s2n  = 2.0 * sn * cn;
    c2n  = cn * cn - sn * sn;
    s2l  = sin ( l );
    c2l  = cos ( l );
    s2l1 = sin ( l1 );
    c2l1 = cos ( l1 );
    
    dl = SSAngle::fromArcsec ( -17.20 * sn - 1.32 * s2l - 0.23 * s2l1 + 0.21 * s2n ).rad;
    de = SSAngle::fromArcsec (   9.20 * cn + 0.57 * c2l + 0.10 * c2l1 - 0.09 * c2n ).rad;
}

// Computes the mean obliquity of the ecliptic (i.e. angle between Earth's equatorial and orbital
// planes) at this coordindate transformation object's current Julian date.  Does not include nutation!

double SSCoords::getObliquity ( void )
{
    double t, e;

    t = ( epoch - SSTime::kJ2000 ) / 36525.0;
    e = 23.439291 + t * ( -0.0130042 + t * ( -0.00000016 + t * 0.000000504 ) );
      
    return SSAngle::fromDegrees ( e ).rad;
}

// Returns a rotation matrix for transforming rectangular coordinates from the
// fundamental J2000 mean equatorial frame to the current equatorial frame.

SSMatrix SSCoords::getFundamentalToEquatorialMatrix ( void )
{
    double zeta = 0.0, z = 0.0, theta = 0.0;
    
    getPrecessionConstants ( zeta, z, theta );
    return SSMatrix::rotation ( 6, 2, zeta, 1, theta, 2, z, 0, -obq, 2, dl, 0, obq + de );
}

// Returns a rotation matrix for transforming rectangular coordinates from the
// fundamental J2000 mean equatorial to the current ecliptic frame.

SSMatrix SSCoords::getEquatorialToEclipticMatrix ( void )
{
    return SSMatrix::rotation ( 1, 0, - obq - de );
}

// Returns a rotation matrix for transforming rectangular coordinates from the
// fundamental J2000 mean equatorial to the current local horizon frame.
// Note we negate the middle row of the matrix because horizon coordinates
// are left-handed!

SSMatrix SSCoords::getEquatorialToHorizonMatrix ( void )
{
    SSMatrix m = SSMatrix::rotation ( 2, 2, SSAngle::kPi - lst, 1, lat - SSAngle::kHalfPi );
    
    m.m10 = -m.m10; m.m11 = -m.m11; m.m12 = -m.m12;
    
    return ( m );
}

// Returns a rotation matrix for transforming rectangular coordinates
// from the fundamental J2000 mean equatorial to the galactic frame.
// From J.C Liu et al, "Reconsidering the Galactic Coordinate System",
// https://www.aanda.org/articles/aa/full_html/2011/02/aa14961-10/aa14961-10.html

SSMatrix SSCoords::getFundamentalToGalacticMatrix ( void )
{
    return SSMatrix ( -0.054875539390, -0.873437104725, -0.483834991775,
                      +0.494109453633, -0.444829594298, +0.746982248696,
                      -0.867666135681, -0.198076389622, +0.455983794523 );
}

// Given a rectangular coordinate vector in the fundamental frame,
// returns a copy of that vector transformed to the current equatorial frame.

SSVector SSCoords::toEquatorial ( SSVector funVec )
{
    return equMat.multiply ( funVec );
}

// Given a rectangular coordinate vector in the fundamental frame,
// returns a copy of that vector transformed to the ecliptic frame.

SSVector SSCoords::toEcliptic ( SSVector funVec )
{
    return eclMat.multiply ( funVec );
}

// Given a rectangular coordinate vector in the fundamental frame,
// returns a copy of that vector transformed to the current local horizon frame.

SSVector SSCoords::toHorizon ( SSVector funVec )
{
    return horMat.multiply ( funVec );
}

// Given a rectangular coordinate vector in the fundamental frame,
// returns a copy of that vector transformed to the galactic frame.

SSVector SSCoords::toGalactic ( SSVector funVec )
{
    return galMat.multiply ( funVec );
}

// Given a rectangular coordinate vector in the current equatorial frame,
// returns a copy of that vector transformed to the fundamental frame.

SSVector SSCoords::fromEquatorial ( SSVector equVec )
{
    return equMat.transpose().multiply ( equVec );
}

// Given a rectangular coordinate vector in the current ecliptic frame,
// returns a copy of that vector transformed to the fundamental frame.

SSVector SSCoords::fromEcliptic ( SSVector eclVec )
{
    return eclMat.transpose().multiply ( eclVec );
}

// Given a rectangular coordinate vector in the galactic frame,
// returns a copy of that vector transformed to the fundamental frame.

SSVector SSCoords::fromGalactic ( SSVector galVec )
{
    return galMat.transpose().multiply ( galVec );
}

// Given a rectangular coordinate vector in the current local horizon frame,
// returns a copy of that vector transformed to the fundamental frame.

SSVector SSCoords::fromHorizon ( SSVector horVec )
{
    return horMat.transpose().multiply ( horVec );
}

// Given a spherical coordinates in the fundamental frame, returns a copy
// of those spherical coords transformed to the current equatorial frame.

SSSpherical SSCoords::toEquatorial ( SSSpherical fun )
{
    return SSSpherical ( toEquatorial ( SSVector ( fun ) ) );
}

// Given a spherical coordinates in the fundamental frame, returns a copy
// of those spherical coords transformed to the current ecliptic frame.

SSSpherical SSCoords::toEcliptic ( SSSpherical fun )
{
    return SSSpherical ( toEcliptic ( SSVector ( fun ) ) );
}

// Given a spherical coordinates in the fundamental frame, returns a copy
// of those spherical coords transformed to the galactic frame.

SSSpherical SSCoords::toGalactic ( SSSpherical fun )
{
    return SSSpherical ( toGalactic ( SSVector ( fun ) ) );
}

// Given a spherical coordinates in the fundamental frame, returns a copy
// of those spherical coords transformed to the current local horizon frame.

SSSpherical SSCoords::toHorizon ( SSSpherical fun )
{
    return SSSpherical ( toHorizon ( SSVector ( fun ) ) );
}

// Given a spherical coordinates in the current equatorial frame,
// returns a copy transformed to the fundamental frame.

SSSpherical SSCoords::fromEquatorial ( SSSpherical equ )
{
    return SSSpherical ( fromEquatorial ( SSVector ( equ ) ) );
}

// Given a spherical coordinates in the current ecliptic frame,
// returns a copy transformed to the fundamental frame.

SSSpherical SSCoords::fromEcliptic ( SSSpherical ecl )
{
    return SSSpherical ( fromEcliptic ( SSVector ( ecl ) ) );
}

// Given a spherical coordinates in the galactic frame,
// returns a copy transformed to the fundamental frame.

SSSpherical SSCoords::fromGalactic ( SSSpherical gal )
{
    return SSSpherical ( fromGalactic ( SSVector ( gal ) ) );
}

// Given a spherical coordinates in the current local horizon frame,
// returns a copy transformed to the fundamental frame.

SSSpherical SSCoords::fromHorizon ( SSSpherical hor )
{
    return SSSpherical ( fromHorizon ( SSVector ( hor ) ) );
}

// Computes atmospheric refraction angle at a particular altitude in radians.
// The boolean a should be true if alt is a true (geometric, un-refracted) altitude,
// and false if alt is an apparent (refracted) altitude.  This formula assumes
// standard atmospheric pressure and temperature of 1010 millibars and +10 deg C.

SSAngle SSCoords::refractionAngle ( SSAngle alt, bool a )
{
	double	h, r;
	
	if ( a == true )
	{
		h = max ( alt.toDegrees(), -1.9 );
		r = 1.02 / tan ( toRadians ( h + ( 10.3 / ( h + 5.11 ) ) ) );
	}
	else
	{
		h = max ( alt.toDegrees(), -1.7 );
		r = 1.0 / tan ( toRadians ( h + ( 7.31 / ( h + 4.4 ) ) ) );
	}
	
	return SSAngle::fromArcmin ( r );
}

// Returns refracted (apparent) altitude from true (geometric) altitude

SSAngle SSCoords::toRefractedAltitude ( SSAngle alt )
{
	return SSAngle ( alt.rad + SSCoords::refractionAngle ( alt, true ).rad );
}

// Returns true (geometric) from refracted (apparent) altitude

SSAngle SSCoords::fromRefractedAltitude ( SSAngle alt )
{
	return SSAngle ( alt.rad - SSCoords::refractionAngle ( alt, false ).rad );
}
