//  test.cpp
//  SSCore
//
//  Created by Tim DeBenedictis on 2/24/20.
//  Copyright © 2020 Southern Stars. All rights reserved.

#include <stdio.h>
#include "AstroLib.h"
#include "SSCoords.hpp"
#include "SSOrbit.hpp"
#include "SSDynamics.hpp"

int main ( int argc, char *argv[] )
{
    SSTime now = SSTime::fromSystem();

	SSSpherical here = { SSAngle ( SSDegMinSec ( '-', 122, 25, 55.3 ) ), SSAngle ( SSDegMinSec ( '+', 37, 46, 09.7 ) ) };
    SSCoords coords ( now.jd, here.lon.rad, here.lat.rad );

//	SSTime now ( SSDate ( kSSGregorian, -5.0, 1971, 12, 28, 11, 44, 0.0 ) );
	SSDate date = ( now );
    
    printf ( "Julian Date: %f\n", now.jd );
    printf ( "Time Zone: %.1f\n", now.zone );
    printf ( "Local Date: %04d-%02hd-%02.0f\n", date.year, date.month, floor ( date.day ) );
    printf ( "Local Time: %02d:%02d:%04.1f\n", date.hour, date.min, date.sec );

    SSSpherical siriusFun = { SSAngle ( SSHourMinSec ( '+', 06, 45, 08.92 ) ).rad, SSAngle ( SSDegMinSec  ( '-', 16, 42, 58.0 ) ).rad };
    SSSpherical siriusEqu = coords.toEquatorial ( siriusFun );
    SSSpherical siriusEcl = coords.toEcliptic ( siriusFun );
    SSSpherical siriusGal = coords.toGalactic ( siriusFun );
    SSSpherical siriusHor = coords.toHorizon ( siriusFun );

    SSHourMinSec ra ( siriusFun.lon );
    SSDegMinSec dec ( siriusFun.lat );
    
    printf ( "Fundamental RA  = %02hd %02hd %05.2f\n", ra.hour, ra.min, ra.sec );
    printf ( "Fundamental Dec = %c%02hd %02hd %04.1f\n", dec.sign, dec.deg, dec.min, dec.sec );

    ra = SSHourMinSec ( siriusEqu.lon );
    dec = SSDegMinSec ( siriusEqu.lat );
    
    printf ( "Equatorial RA  = %02hd %02hd %05.2f\n", ra.hour, ra.min, ra.sec );
    printf ( "Equatorial Dec = %c%02hd %02hd %04.1f\n", dec.sign, dec.deg, dec.min, dec.sec );

    SSDegMinSec eclon ( siriusEcl.lon );
    SSDegMinSec eclat ( siriusEcl.lat );

    printf ( "Ecliptic Lon = %03hd %02hd %04.1f\n", eclon.deg, eclon.min, eclon.sec );
    printf ( "Ecliptic Lat = %c%02hd %02hd %04.1f\n", eclat.sign, eclat.deg, eclat.min, eclat.sec );

    SSDegMinSec galon ( siriusGal.lon );
    SSDegMinSec galat ( siriusGal.lat );

    printf ( "Galactic Lon = %03hd %02hd %04.1f\n", galon.deg, galon.min, galon.sec );
    printf ( "Galactic Lat = %c%02hd %02hd %04.1f\n", galat.sign, galat.deg, galat.min, galat.sec );

    SSDegMinSec azm ( siriusHor.lon );
    SSDegMinSec alt ( siriusHor.lat );

    printf ( "Azimuth  = %03hd %02hd %04.1f\n", azm.deg, azm.min,azm.sec );
    printf ( "Altitude = %c%02hd %02hd %04.1f\n", alt.sign, alt.deg, alt.min, alt.sec );

	// Print J2000 RA/Dec of north galactic pole
	
	SSSpherical galCen = coords.fromGalactic ( SSSpherical ( 0.0, 0.0 ) );
	ra = SSHourMinSec ( galCen.lon );
	dec = SSDegMinSec ( galCen.lat );
	
    printf ( "Gal Cen RA  = %02hd %02hd %05.2f\n", ra.hour, ra.min, ra.sec );
    printf ( "Gal Cen Dec = %c%02hd %02hd %04.1f\n", dec.sign, dec.deg, dec.min, dec.sec );

	// Print J2000 RA/Dec of galactic center
	
	SSSpherical ngp = coords.fromGalactic ( SSSpherical ( 0.0, SSAngle::fromDegrees ( 90.0 ) ) );
	ra = SSHourMinSec ( ngp.lon );
	dec = SSDegMinSec ( ngp.lat );
    printf ( "NGP RA  = %02hd %02hd %05.2f\n", ra.hour, ra.min, ra.sec );
    printf ( "NGP Dec = %c%02hd %02hd %04.1f\n", dec.sign, dec.deg, dec.min, dec.sec );

	double jde = now.getJulianEphemerisDate();
	SSOrbit orb = SSOrbit::getEarthOrbit ( jde );
	SSMatrix orbMat = SSCoords::getEclipticMatrix ( SSCoords::getObliquity ( SSTime::kJ2000 ) );
	
	SSVector pos, vel;
	orb.toPositionVelocity ( jde, pos, vel );
	
	pos = pos.multiplyBy ( -1.0 );
	pos = orbMat.multiply ( pos );
	vel = orbMat.multiply ( vel );
	
	pos = coords.toEquatorial ( pos );
	SSSpherical equ ( pos );
	ra = SSHourMinSec ( equ.lon );
	dec = SSDegMinSec ( equ.lat );

    printf ( "Sun RA   = %02hd %02hd %05.2f\n", ra.hour, ra.min, ra.sec );
    printf ( "Sun Dec  = %c%02hd %02hd %04.1f\n", dec.sign, dec.deg, dec.min, dec.sec );
    printf ( "Sun Dist = %f AU\n", equ.rad );

    SSDynamics dyn ( now.jd, here.lon.rad, here.lat.rad );
    
    SSPlanetID planetIDs[10] = { kSun, kMercury, kVenus, kEarth, kMars, kJupiter, kSaturn, kUranus, kNeptune, kPluto };
    
    for ( SSPlanetID id : planetIDs )
    {
        SSPlanet planet ( id );
        planet.computeEphemeris ( dyn );
        
        SSSpherical equ ( dyn.coords.toEquatorial ( planet.dir ) );
        ra = SSHourMinSec ( equ.lon );
        dec = SSDegMinSec ( equ.lat );

        printf ( "%d RA   = %02hd %02hd %05.2f\n", id, ra.hour, ra.min, ra.sec );
        printf ( "%d Dec  = %c%02hd %02hd %04.1f\n", id, dec.sign, dec.deg, dec.min, dec.sec );
        printf ( "%d Dist = %f AU\n", id, equ.rad );
    }
	
	dyn.getMoonPositionVelocity ( kLuna, pos, vel );
	equ = SSSpherical ( dyn.coords.toEquatorial ( pos ) );
	ra = SSHourMinSec ( equ.lon );
	dec = SSDegMinSec ( equ.lat );
	
    printf ( "Moon RA   = %02hd %02hd %05.2f\n", ra.hour, ra.min, ra.sec );
    printf ( "Moon Dec  = %c%02hd %02hd %04.1f\n", dec.sign, dec.deg, dec.min, dec.sec );
    printf ( "Moon Dist = %f ER\n", equ.rad );

/*
	SSVector v1 ( 1.0, 2.0, 3.0 );
    SSVector v2 ( 4.0, 5.0, 6.0 );
    SSVector v3;
    
    v1.add ( v2 );
    
    
    v3 = v1.add ( v2 );
    
    printf ( "%lf %lf %lf\n", v1.x, v1.y, v1.z );
    printf ( "%lf %lf %lf\n", v2.x, v2.y, v2.z );
    printf ( "%lf %lf %lf\n", v3.x, v3.y, v3.z );
    
    v1 = v1.subtract ( v2 );
    printf ( "%lf %lf %lf\n", v1.x, v1.y, v1.z );
    
    SSMatrix i = SSMatrix::identity();
    
    printf ( "%lf %lf %lf\n", i.m00, i.m01, i.m02 );
    printf ( "%lf %lf %lf\n", i.m10, i.m11, i.m12 );
    printf ( "%lf %lf %lf\n", i.m20, i.m21, i.m22 );
*/
	
}
