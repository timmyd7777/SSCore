//
//  SSDynamics.cpp
//  SSCore
//
//  Created by Tim DeBenedictis on 3/10/20.
//  Copyright © 2020 Southern Stars. All rights reserved.
//

#include <map>
#include "SSDynamics.hpp"

SSDynamics::SSDynamics ( double jd, double lon, double lat ) : coords ( jd, lon, lat )
{
    jde = SSTime ( jd ).getJulianEphemerisDate();
    orbMat = SSCoords::getEclipticMatrix ( SSCoords::getObliquity ( SSTime::kJ2000 ) );
    getPlanetPositionVelocity ( kEarth, obsPos, obsVel );
	
	SSSpherical geodetic ( coords.lst, coords.lat, 0.0 );
	SSVector geocentric = SSDynamics::toGeocentric ( geodetic, kKMPerEarthRadii / kKMPerAU, kEarthFlattening );
	
	obsPos = obsPos.add ( coords.fromEquatorial ( geocentric ) );
}

SSVector SSDynamics::toGeocentric ( SSSpherical geodetic, double a, double f )
{
	double c, s, cp = cos ( geodetic.lat.rad ), sp = sin ( geodetic.lat.rad );

	f = ( 1.0 - f ) * ( 1.0 - f );
	c = 1.0 / sqrt ( cp * cp + f * sp * sp );
	s = f * c;
	
	double x = ( a * c + geodetic.rad ) * cp * cos ( geodetic.lon.rad );
	double y = ( a * c + geodetic.rad ) * cp * sin ( geodetic.lon.rad );
	double z = ( a * s + geodetic.rad ) * sp;
	
	return SSVector ( x, y, z );
}

SSSpherical SSDynamics::toGeodetic ( SSVector geocentric, double a, double f )
{
	double x = geocentric.x, y = geocentric.y, z = geocentric.z;
	double r = sqrt ( x * x + y * y );
	double e2 = 2.0 * f - f * f, s, c;
	double lon = atan2Pi ( y, x );
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

void SSDynamics::getPlanetPositionVelocity ( SSPlanetID id, SSVector &pos, SSVector &vel )
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

void SSDynamics::getMoonPositionVelocity ( SSMoonID id, SSVector &pos, SSVector &vel )
{
	double d = jde - SSTime::kJ2000 + 1.5;
	double a = 60.2666; // Earth radii
	double e = 0.054900;
	double i = toRadians ( 5.1454 );
	double Ms = mod2Pi ( toRadians ( 356.0470 +  0.9856002585 * d ) );
	double Mm = mod2Pi ( toRadians ( 115.3654 + 13.0649929509 * d ) );
	double Nm = mod2Pi ( toRadians ( 125.1228 -  0.0529538083 * d ) ), N = Nm;
	double ws = mod2Pi ( toRadians ( 282.9404 +  4.70935e-5 * d ) );
	double wm = mod2Pi ( toRadians ( 318.0634 +  0.1643573223 * d ) ), w = wm;
	double Ls = mod2Pi ( Ms + ws );
	double Lm = mod2Pi ( Mm + wm + Nm );
	double D = mod2Pi ( Lm - Ls );
	double F = mod2Pi ( Lm - Nm );
	
	double M = Mm;
	double E = M + e * sin ( M ) * ( 1.0 + e * cos ( M ) ), E1 = E, E0 = E + 1.0;

	while ( fabs ( E1 - E0 ) > toRadians ( 0.0001 ) )
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
	double lon_corr = 3.82394E-5 * ( /* 365.2422 * ( epoch - 2000.0 ) */ - d );	// precession, degrees
	
	lonecl += toRadians ( lon_corr +
            -1.274 * sin ( Mm - 2 * D )          // (the Evection)
            +0.658 * sin ( 2 * D )               // (the Variation)
            -0.186 * sin ( Ms )                // (the Yearly Equation)
            -0.059 * sin (2 * Mm - 2 * D )
            -0.057 * sin ( Mm - 2 * D + Ms )
            +0.053 * sin ( Mm + 2 * D )
            +0.046 * sin ( 2 * D - Ms )
            +0.041 * sin ( Mm - Ms)
            -0.035 * sin ( D )                 // (the Parallactic Equation)
            -0.031 * sin ( Mm + Ms )
            -0.015 * sin ( 2 * F - 2 * D )
            +0.011 * sin ( Mm - 4 * D ) );

	latecl += toRadians (
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

	double ecl = toRadians ( 23.4393 - 3.563e-7 * d );
	
	pos.x = xh;
	pos.y = yh * cos ( ecl ) - zh * sin ( ecl );
	pos.z = yh * sin ( ecl ) + zh * cos ( ecl );
	
	pos = pos.multiplyBy ( kKMPerEarthRadii / kKMPerAU );
	
	SSVector earthPos, earthVel;
	
	getPlanetPositionVelocity ( kEarth, earthPos, earthVel );
	pos = earthPos.add ( pos );
}


SSPlanet::SSPlanet ( SSPlanetID id )
{
    this->id = id;
    this->pos = SSVector();
    this->vel = SSVector();
    this->dir = SSVector();
    this->dist = 0.0;
    this->mag = 0.0;
}

void SSPlanet::computeEphemeris ( SSDynamics dyn )
{
    dyn.getPlanetPositionVelocity ( id, pos, vel );
    
    dir = pos.subtract ( dyn.obsPos );
    dist = dir.normalize();
}
