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
#include "SSPlanet.hpp"
#include "SSStar.hpp"
#include "SSHipparcos.hpp"
#include "SSSKY2000.hpp"
#include "SSNGCIC.hpp"

int main ( int argc, char *argv[] )
{
	SSObjectVec objects;
	SSIdentifierNameMap ngcicNameMap;
	
	importNGCICNameMap ( "/Users/timmyd/Projects/SouthernStars/Catalogs/Revised NGC-IC 2019/NINames.csv", ngcicNameMap );
	importNGCIC ( "/Users/timmyd/Projects/SouthernStars/Catalogs/Revised NGC-IC 2019/NI2019.txt", ngcicNameMap, objects );
	
	HIPMap mapHIPtoHR = importHIPtoHRMap ( "/Users/timmyd/Projects/SouthernStars/Catalogs/Hipparcos/TABLES/IDENT3.DOC" );
	HIPMap mapHIPtoBF = importHIPtoBayerFlamsteedMap ( "/Users/timmyd/Projects/SouthernStars/Catalogs/Hipparcos/TABLES/IDENT4.DOC" );
	HIPMap mapHIPtoVar = importHIPtoVarMap ( "/Users/timmyd/Projects/SouthernStars/Catalogs/Hipparcos/TABLES/IDENT5.DOC" );
    HIPNameMap mapHIPNames = importHIPNameMap ( "/Users/timmyd/Projects/SouthernStars/Catalogs/Hipparcos/TABLES/IDENT6.DOC" );
	SSStarMap mapHIC = importHIC ( "/Users/timmyd/Projects/SouthernStars/Catalogs/Hipparcos Input Catalog/main.dat" );
	SSStarMap mapHIP2 = importHIP2 ( "/Users/timmyd/Projects/SouthernStars/Catalogs/Hipparcos New Reduction 2007/hip2.dat" );
	SSStarMap mapHIP = importHIP ( "/Users/timmyd/Projects/SouthernStars/Catalogs/Hipparcos/CATS/HIP_MAIN.DAT", mapHIPtoHR, mapHIPtoBF, mapHIPtoVar, mapHIC, mapHIP2, mapHIPNames );

    SSStarNameMap nameMap = importIAUStarNames ( "/Users/timmyd/Projects/SouthernStars/Projects/Star Names/IAU-CSN.txt" );
    vector<SSStar> skymap = importSKY2000 ( "/Users/timmyd/Projects/SouthernStars/Catalogs/SKY2000 Master Star Catalog/ATT_sky2kv5.cat", nameMap );
	
    SSAngle zero = 0.0;
    SSAngle one ( 1.0 );
    SSAngle two ( 2.0 );
    
    SSAngle sum = zero + one - 1.5;
    sum /= two;
    
    printf ( "%f\n", (double) sum );
            
    SSTime now = SSTime::fromSystem();
	SSSpherical here = { SSAngle ( SSDegMinSec ( '-', 122, 25, 55.3 ) ), SSAngle ( SSDegMinSec ( '+', 37, 46, 09.7 ) ) };
    SSDynamics dyn ( now.jd, here.lon, here.lat );
    
//	SSTime now ( SSDate ( kSSGregorian, -5.0, 1971, 12, 28, 11, 44, 0.0 ) );
	SSDate date = ( now );
    
    printf ( "Julian Date: %f\n", now.jd );
    printf ( "Time Zone: %.1f\n", now.zone );
    printf ( "Local Date: %04d-%02hd-%02.0f\n", date.year, date.month, floor ( date.day ) );
    printf ( "Local Time: %02d:%02d:%04.1f\n", date.hour, date.min, date.sec );

    SSSpherical siriusFun = { SSAngle ( SSHourMinSec ( '+', 06, 45, 08.92 ) ), SSAngle ( SSDegMinSec  ( '-', 16, 42, 58.0 ) ) };
    siriusFun = dyn.addAberration ( siriusFun );
    
    SSSpherical siriusEqu = dyn.coords.toEquatorial ( siriusFun );
    SSSpherical siriusEcl = dyn.coords.toEcliptic ( siriusFun );
    SSSpherical siriusGal = dyn.coords.toGalactic ( siriusFun );
    SSSpherical siriusHor = dyn.coords.toHorizon ( siriusFun );

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
	
	SSSpherical galCen = dyn.coords.fromGalactic ( SSSpherical ( 0.0, 0.0 ) );
	ra = SSHourMinSec ( galCen.lon );
	dec = SSDegMinSec ( galCen.lat );
	
    printf ( "Gal Cen RA  = %02hd %02hd %05.2f\n", ra.hour, ra.min, ra.sec );
    printf ( "Gal Cen Dec = %c%02hd %02hd %04.1f\n", dec.sign, dec.deg, dec.min, dec.sec );

	// Print J2000 RA/Dec of galactic center
	
	SSSpherical ngp = dyn.coords.fromGalactic ( SSSpherical ( 0.0, SSAngle::fromDegrees ( 90.0 ) ) );
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
	
	pos = dyn.coords.toEquatorial ( pos );
	SSSpherical equ ( pos );
	ra = SSHourMinSec ( equ.lon );
	dec = SSDegMinSec ( equ.lat );

    printf ( "Sun RA   = %02hd %02hd %05.2f\n", ra.hour, ra.min, ra.sec );
    printf ( "Sun Dec  = %c%02hd %02hd %04.1f\n", dec.sign, dec.deg, dec.min, dec.sec );
    printf ( "Sun Dist = %f AU\n", equ.rad );

    SSPlanetID planetIDs[10] = { kSun, kMercury, kVenus, kEarth, kMars, kJupiter, kSaturn, kUranus, kNeptune, kPluto };
    
    for ( SSPlanetID id : planetIDs )
    {
        SSPlanet planet ( kTypePlanet, id );
        planet.computeEphemeris ( dyn );
        
        SSSpherical equ ( dyn.coords.toEquatorial ( planet.getDirection() ) );
        ra = SSHourMinSec ( equ.lon );
        dec = SSDegMinSec ( equ.lat );

        printf ( "%d RA   = %02hd %02hd %05.2f\n", id, ra.hour, ra.min, ra.sec );
        printf ( "%d Dec  = %c%02hd %02hd %04.1f\n", id, dec.sign, dec.deg, dec.min, dec.sec );
        printf ( "%d Dist = %f AU\n", id, planet.getDistance() );
    }
	
	dyn.getMoonPositionVelocity ( kLuna, dyn.jde, pos, vel );
	pos = pos.subtract ( dyn.obsPos );
	equ = SSSpherical ( dyn.coords.toEquatorial ( pos ) );
	ra = SSHourMinSec ( equ.lon );
	dec = SSDegMinSec ( equ.lat );
	
    printf ( "Moon RA   = %02hd %02hd %05.2f\n", ra.hour, ra.min, ra.sec );
    printf ( "Moon Dec  = %c%02hd %02hd %04.1f\n", dec.sign, dec.deg, dec.min, dec.sec );
    printf ( "Moon Dist = %f ER\n", equ.rad * SSDynamics::kKmPerAU );

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

SSObjectPtr SSNewObject ( SSObjectType type )
{
	if ( type == kTypeStar )
		return shared_ptr<SSStar> ( new SSStar );
	else if ( type == kTypeDoubleStar )
		return shared_ptr<SSDoubleStar> ( new SSDoubleStar );
	else if ( type == kTypeVariableStar )
		return shared_ptr<SSVariableStar> ( new SSVariableStar );
	else if ( type == kTypeDoubleVariableStar )
		return shared_ptr<SSDoubleVariableStar> ( new SSDoubleVariableStar );
    else if ( type >= kTypeOpenCluster && type <= kTypeGalaxy )
        return shared_ptr<SSDeepSky> ( new SSDeepSky ( type ) );
	else if ( type >= kTypePlanet && type <= kTypeSpacecraft )
		return shared_ptr<SSPlanet> ( new SSPlanet ( type ) );
	else
		return shared_ptr<class SSObject> ( nullptr );
}
