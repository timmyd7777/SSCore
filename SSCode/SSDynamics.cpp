//
//  SSDynamics.cpp
//  SSCore
//
//  Created by Tim DeBenedictis on 3/10/20.
//  Copyright © 2020 Southern Stars. All rights reserved.
//

#include <map>
#include "SSDynamics.hpp"
#include "SSPlanet.hpp"

SSDynamics::SSDynamics ( double jd, double lon, double lat, double alt ) : coords ( jd, lon, lat, alt )
{
    jde = SSTime ( jd ).getJulianEphemerisDate();
    orbMat = SSCoords::getEclipticMatrix ( SSCoords::getObliquity ( SSTime::kJ2000 ) );
    SSPlanet::computeMajorPlanetPositionVelocity ( kEarth, jde, 0.0, obsPos, obsVel );
    
    SSSpherical geodetic ( coords.lst, coords.lat, coords.alt );
    SSVector geocentric = SSDynamics::toGeocentric ( geodetic, kKmPerEarthRadii, kEarthFlattening );
    
    geocentric = coords.fromEquatorial ( geocentric );
    obsPos = obsPos.add ( geocentric / kKmPerAU );
}

// Converts geodetic longitude, latitude, altitude to geocentric X, Y, Z vector.
// geodetic.lon and .lat are in radians; geo.rad is altitude above geoid in same
// units as equatorial radius of geoid ellipse (a). Geoid flattening (f) is ratio
// (a - b)/(a), where b is polar radius of geoid ellipse. Rectangular XYZ vector
// is returned in same units as (a).
// Formula from "The Astronomical Almanac for the Year 1990", pp. K11-K13.

SSVector SSDynamics::toGeocentric ( SSSpherical geodetic, double a, double f )
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

SSSpherical SSDynamics::toGeodetic ( SSVector geocentric, double a, double f )
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

// Adds aberration of light to an apparent direction unit vector (p)
// in the fundamental J2000 equatorial frame. Returns the "aberrated"
// vector; p itself is not modified. Uses relativatic formula from
// The Explanatory Supplement to the Astronomical Almanac, p. 129.
// Observer's heliocentric velocity must have been calcualted previously!

SSVector SSDynamics::addAberration ( SSVector p )
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

SSVector SSDynamics::subtractAberration ( SSVector p )
{
    return ( p - obsVel / kLightAUPerDay ).normalize();
}

// Given a positive or negative red shift (z), returns the equivalent radial velocity
// as a fraction of light speed (rv) using relativistic formula.
// Redshifts > 1.0 return radial velocities < 1.0.

double SSDynamics::redShiftToRadVel ( double z )
{
    double z12 = ( z + 1.0 ) * ( z + 1.0 );
    double rv = ( z12 - 1.0 ) / ( z12 + 1.0 );
    
    return ( rv );
}

// Given positive or negative radial velocity as a fraction of light speed (rv),
// returns the equivalent red shift (z) using relativistic formula.
// Radial velocity must be < 1.0 but may return redshift > 1.0.

double SSDynamics::radVelToRedShift ( double rv )
{
    return sqrt ( ( 1.0 + rv ) / ( 1.0 - rv ) ) - 1.0;
}

void SSDynamics::getPlanetPositionVelocity ( SSPlanetID id, double jde, SSVector &pos, SSVector &vel )
{
    SSOrbit orb;
    
    if ( id == kMercury )
        orb = SSOrbit::getMercuryOrbit ( jde );
    else if ( id == kVenus )
        orb = SSOrbit::getVenusOrbit ( jde );
    else if ( id == kEarth )
        orb = SSOrbit::getEarthOrbit ( jde );
    else if ( id == kMars )
        orb = SSOrbit::getMarsOrbit ( jde );
    else if ( id == kJupiter )
        orb = SSOrbit::getJupiterOrbit ( jde );
    else if ( id == kSaturn )
        orb = SSOrbit::getSaturnOrbit ( jde );
    else if ( id == kUranus )
        orb = SSOrbit::getUranusOrbit ( jde );
    else if ( id == kNeptune )
        orb = SSOrbit::getNeptuneOrbit ( jde );
    else if ( id == kPluto )
        orb = SSOrbit::getPlutoOrbit ( jde );

    orb.toPositionVelocity ( jde, pos, vel );
    pos = orbMat.multiply ( pos );
    vel = orbMat.multiply ( vel );
}

void SSDynamics::getMoonPositionVelocity ( SSPlanetID id, double jde, SSVector &pos, SSVector &vel )
{
    double d = jde - SSTime::kJ2000 + 1.5;
    double a = 60.2666; // Earth radii
    double e = 0.054900;
    double i = SSAngle::fromDegrees ( 5.1454 );
    double Ms = SSAngle::fromDegrees ( 356.0470 +  0.9856002585 * d ).mod2Pi();
    double Mm = SSAngle::fromDegrees ( 115.3654 + 13.0649929509 * d ).mod2Pi();
    double Nm = SSAngle::fromDegrees ( 125.1228 -  0.0529538083 * d ).mod2Pi(), N = Nm;
    double ws = SSAngle::fromDegrees ( 282.9404 +  4.70935e-5 * d ).mod2Pi();
    double wm = SSAngle::fromDegrees ( 318.0634 +  0.1643573223 * d ).mod2Pi(), w = wm;
    double Ls = SSAngle ( Ms + ws ).mod2Pi();
    double Lm = SSAngle ( Mm + wm + Nm ).mod2Pi();
    double D = SSAngle ( Lm - Ls ).mod2Pi();
    double F = SSAngle ( Lm - Nm ).mod2Pi();
    
    double M = Mm;
    double E = M + e * sin ( M ) * ( 1.0 + e * cos ( M ) ), E1 = E, E0 = E + 1.0;

    while ( fabs ( E1 - E0 ) > SSAngle::fromDegrees ( 0.0001 ) )
    {
        E0 = E1;
        E1 = E0 - ( E0 - e * sin ( E0 ) - M ) / ( 1 - e * cos ( E0 ) );
    }
    
    E = E1;

    double xv = a * ( cos( E ) - e );
    double yv = a * ( sqrt ( 1.0 - e * e ) * sin ( E ) );

    double v = atan2 ( yv, xv );
    double r = sqrt ( xv * xv + yv * yv );

    double xh = r * ( cos ( N ) * cos ( v + w ) - sin ( N ) * sin ( v + w ) * cos ( i ) );
    double yh = r * ( sin ( N ) * cos ( v + w ) + cos ( N ) * sin ( v + w ) * cos ( i ) );
    double zh = r * ( sin ( v + w ) * sin ( i ) );

    double lonecl = atan2 ( yh, xh );
    double latecl = atan2 ( zh, sqrt ( xh * xh + yh * yh ) );
    double lon_corr = 3.82394E-5 * ( /* 365.2422 * ( epoch - 2000.0 ) */ - d );    // precession, degrees
    
    lonecl += SSAngle::fromDegrees ( lon_corr +
            -1.274 * sin ( Mm - 2 * D )          // (the Evection)
            +0.658 * sin ( 2 * D )               // (the Variation)
            -0.186 * sin ( Ms )                  // (the Yearly Equation)
            -0.059 * sin ( 2 * Mm - 2 * D )
            -0.057 * sin ( Mm - 2 * D + Ms )
            +0.053 * sin ( Mm + 2 * D )
            +0.046 * sin ( 2 * D - Ms )
            +0.041 * sin ( Mm - Ms)
            -0.035 * sin ( D )                   // (the Parallactic Equation)
            -0.031 * sin ( Mm + Ms )
            -0.015 * sin ( 2 * F - 2 * D )
            +0.011 * sin ( Mm - 4 * D ) );

    latecl += SSAngle::fromDegrees (
            -0.173 * sin ( F - 2 * D )
            -0.055 * sin ( Mm - F - 2 * D )
            -0.046 * sin ( Mm + F - 2 * D )
            +0.033 * sin ( F + 2 * D )
            +0.017 * sin ( 2 * Mm + F ) );

    r += -0.58 * cos ( Mm - 2 * D )
         -0.46 * cos ( 2 * D );

    xh = r * cos ( lonecl ) * cos ( latecl );
    yh = r * sin ( lonecl ) * cos ( latecl );
    zh = r * sin ( latecl );

    double ecl = SSAngle::fromDegrees ( 23.4393 - 3.563e-7 * d );
    
    pos.x = xh;
    pos.y = yh * cos ( ecl ) - zh * sin ( ecl );
    pos.z = yh * sin ( ecl ) + zh * cos ( ecl );
    
    pos *= kKmPerEarthRadii / kKmPerAU;
    
    SSVector earthPos, earthVel;
    
    getPlanetPositionVelocity ( kEarth, jde, earthPos, earthVel );
    pos += earthPos;
}


