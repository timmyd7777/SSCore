//  test.cpp
//  SSCore
//
//  Created by Tim DeBenedictis on 2/24/20.
//  Copyright © 2020 Southern Stars. All rights reserved.

#include <stdio.h>
#include "AstroLib.h"
#include "SSCoords.hpp"

int main ( int argc, char *argv[] )
{
    SSTime now = SSTime::fromSystem();

	SSCoords::LonLat here = { SSAngle::fromDegMinSec ( '-', 122, 25, 55.3 ), SSAngle::fromDegMinSec ( '+', 37, 46, 09.7 ) };
    SSCoords coords ( now.jd, true, here.lon.a, here.lat.a );

    SSTime::CalendarDate date = now.toCalendarDate ( kSSCalendarGregorian );
    
    printf ( "Julian Date: %f\n", now.jd );
    printf ( "Time Zone: %.1f\n", now.zone );
    printf ( "Local Date: %04d-%02hd-%02.0f\n", date.year, date.month, floor ( date.day ) );
    printf ( "Local Time: %02d:%02d:%04.1f\n", date.hour, date.min, date.sec );

    SSCoords::RADec siriusFun = { SSAngle::fromHourMinSec ( '+', 06, 45, 08.92 ), SSAngle::fromDegMinSec  ( '-', 16, 42, 58.0 ) };
    SSCoords::RADec siriusEqu = coords.toEquatorial ( siriusFun.ra.a, siriusFun.dec.a );
    SSCoords::LonLat siriusEcl = coords.toEcliptic ( siriusFun.ra.a, siriusFun.dec.a );
    SSCoords::LonLat siriusGal = coords.toGalactic ( siriusFun.ra.a, siriusFun.dec.a );
    SSCoords::AzmAlt siriusHor = coords.toHorizon ( siriusFun.ra.a, siriusFun.dec.a );

    SSAngle::HMS ra = siriusFun.ra.toHMS();
    SSAngle::DMS dec = siriusFun.dec.toDMS();
    
    printf ( "Fundamental RA  = %02hd %02hd %05.2f\n", ra.hour, ra.min, ra.sec );
    printf ( "Fundamental Dec = %c%02hd %02hd %04.1f\n", dec.sign, dec.deg, dec.min, dec.sec );

    ra = siriusEqu.ra.toHMS();
    dec = siriusEqu.dec.toDMS();
    
    printf ( "Equatorial RA  = %02hd %02hd %05.2f\n", ra.hour, ra.min, ra.sec );
    printf ( "Equatorial Dec = %c%02hd %02hd %04.1f\n", dec.sign, dec.deg, dec.min, dec.sec );

    SSAngle::DMS eclon = siriusEcl.lon.toDMS();
    SSAngle::DMS eclat = siriusEcl.lat.toDMS();

    printf ( "Ecliptic Lon = %03hd %02hd %04.1f\n", eclon.deg, eclon.min, eclon.sec );
    printf ( "Ecliptic Lat = %c%02hd %02hd %04.1f\n", eclat.sign, eclat.deg, eclat.min, eclat.sec );

    SSAngle::DMS galon = siriusGal.lon.toDMS();
    SSAngle::DMS galat = siriusGal.lat.toDMS();

    printf ( "Galactic Lon = %03hd %02hd %04.1f\n", galon.deg, galon.min, galon.sec );
    printf ( "Galactic Lat = %c%02hd %02hd %04.1f\n", galat.sign, galat.deg, galat.min, galat.sec );

    SSAngle::DMS azm = siriusHor.azm.toDMS();
    SSAngle::DMS alt = siriusHor.alt.toDMS();

    printf ( "Azimuth  = %03hd %02hd %04.1f\n", azm.deg, azm.min,azm.sec );
    printf ( "Altitude = %c%02hd %02hd %04.1f\n", alt.sign, alt.deg, alt.min, alt.sec );

	SSCoords::RADec galCen = coords.fromGalactic ( 0.0, 0.0 );
	ra = galCen.ra.toHMS();
	dec = galCen.dec.toDMS();
    printf ( "Gal Cen RA  = %02hd %02hd %05.2f\n", ra.hour, ra.min, ra.sec );
    printf ( "Gal Cen Dec = %c%02hd %02hd %04.1f\n", dec.sign, dec.deg, dec.min, dec.sec );

	SSCoords::RADec ngp = coords.fromGalactic ( 0.0, SSAngle::fromDegrees ( 90.0 ).a );
	ra = ngp.ra.toHMS();
	dec = ngp.dec.toDMS();
    printf ( "NGP RA  = %02hd %02hd %05.2f\n", ra.hour, ra.min, ra.sec );
    printf ( "NGP Dec = %c%02hd %02hd %04.1f\n", dec.sign, dec.deg, dec.min, dec.sec );

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
