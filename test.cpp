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
#include "SSStar.hpp"

#include <iostream>
#include <fstream>
#include <map>

typedef multimap<int,SSIdentifier> HIPMap;
typedef multimap<int,string> HIPNameMap;
typedef map<int,SSStar> SSStarMap;

SSStarMap importHIC ( const char *filename );
SSStarMap importHIP ( const char *filename, HIPMap mapHIPtoHR, HIPMap mapHIPtoBF, HIPMap mapHIPtoVar, SSStarMap mapHIC, SSStarMap mapHIP2, HIPNameMap mapNames );
HIPMap importHIPtoHRMap ( const char *filename );
HIPMap importHIPtoBayerFlamsteedMap ( const char *filename );
HIPMap importHIPtoVarMap ( const char *filename );
HIPNameMap importHIPNameMap ( const char *filename );
HIPNameMap importIAUNameMap ( const char *filename );
SSStarMap importHIP2 ( const char *filename );

int main ( int argc, char *argv[] )
{
	HIPNameMap mapHIPtoIAUName = importIAUNameMap ( "/Users/timmyd/Projects/SouthernStars/Projects/Star Names/IAU-CSN.txt" );
	HIPNameMap mapHIPtoHIPName = importHIPNameMap ( "/Users/timmyd/Projects/SouthernStars/Catalogs/Hipparcos/TABLES/IDENT6.DOC" );
	HIPMap mapHIPtoHR = importHIPtoHRMap ( "/Users/timmyd/Projects/SouthernStars/Catalogs/Hipparcos/TABLES/IDENT3.DOC" );
	HIPMap mapHIPtoBF = importHIPtoBayerFlamsteedMap ( "/Users/timmyd/Projects/SouthernStars/Catalogs/Hipparcos/TABLES/IDENT4.DOC" );
	HIPMap mapHIPtoVar = importHIPtoVarMap ( "/Users/timmyd/Projects/SouthernStars/Catalogs/Hipparcos/TABLES/IDENT5.DOC" );
	SSStarMap mapHIC = importHIC ( "/Users/timmyd/Projects/SouthernStars/Catalogs/Hipparcos Input Catalog/main.dat" );
	SSStarMap mapHIP2 = importHIP2 ( "/Users/timmyd/Projects/SouthernStars/Catalogs/Hipparcos New Reduction 2007/hip2.dat" );
	SSStarMap mapHIP = importHIP ( "/Users/timmyd/Projects/SouthernStars/Catalogs/Hipparcos/CATS/HIP_MAIN.DAT", mapHIPtoHR, mapHIPtoBF, mapHIPtoVar, mapHIC, mapHIP2, mapHIPtoIAUName );

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
        SSPlanet planet ( kPlanet, id );
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

// Cleans up some oddball conventions in the Hipparcos star name identification tables
// for Bayer, Flamsteed, and variable star names so SSIdentifier understands them.
// Returns cleaned-up name string, does not modify input string.

string cleanHIPNameString ( string str )
{
	if ( str.find ( "alf" ) == 0 )
		str.replace ( 0, 3, "alp" );
	
	if ( str.find ( "ksi" ) == 0 )
		str.replace ( 0, 3, "xi." );
	
	size_t idx = str.find ( "." );
	if ( idx != string::npos )
		str.erase ( idx, 1 );
	
	idx = str.find ( "_" );
	if ( idx != string::npos )
		str.replace ( idx, 1, " " );
	
	return str;
}

// Updates star position and proper motion from the Hipparcos epoch J1991.25 to J2000.
// Uses a rigorous transformation which is accurate in all parts of the sky.

void updateHIPStarPositionVelocity ( SSSpherical &position, SSSpherical &velocity )
{
	double rad = position.rad;
	double radvel = velocity.rad;
	
	position.rad = 1.0;
	velocity.rad = 0.0;
	
	SSVector pos = position.toVectorPosition();
	SSVector vel = position.toVectorVelocity ( velocity );
	
	pos += vel * ( 2000.0 - 1991.25 );
	
	position = pos.toSpherical();
	velocity = pos.toSphericalVelocity ( vel );
	
	position.rad = rad;
	velocity.rad = radvel;
}

// Imports the Hipparcos Input Catalog, version 2.
// Still useful for SAO and variable star identifiers
// and radial velocities, all omitted from the final Hipparcos catalog.

SSStarMap importHIC ( const char *filename )
{
    SSStarMap starmap;
	ifstream hic_file ( filename );
	
	if ( ! hic_file )
		return starmap;
	
	string line;
    int linecount = 0;
	while ( getline ( hic_file, line ) )
	{
		string strHIP = trim ( line.substr ( 0, 6 ) );
		string strRA = trim ( line.substr ( 13, 12 ) );
		string strDec = trim ( line.substr ( 26, 12 ) );
		string strPMRA = trim ( line.substr ( 155, 6 ) );
		string strPMDec = trim ( line.substr ( 162, 6 ) );
		string strMag = trim ( line.substr ( 190, 6 ) );
		string strBmV = trim ( line.substr ( 202, 6 ) );
		string strSpec = trim ( line.substr ( 216, 11 ) );
		string strPlx = trim ( line.substr ( 230, 6 ) );
		string strRV = trim ( line.substr ( 241, 6 ) );
		string strVar = trim ( line.substr ( 251, 9 ) );
		string strVarType = trim ( line.substr ( 261, 3 ) );
		string strVarPer = trim ( line.substr ( 265, 6 ) );
		string strVarMax = trim ( line.substr ( 272, 4 ) );
		string strVarMin = trim ( line.substr ( 277, 4 ) );
		string strHD = trim ( line.substr ( 359, 6 ) );
		string strBD = trim ( line.substr ( 320, 10 ) );
		string strCD = trim ( line.substr ( 334, 10 ) );
		string strCP = trim ( line.substr ( 348, 10 ) );
		string strSAO = trim ( line.substr ( 385, 6 ) );

        linecount++;
        
		SSSpherical position ( HUGE_VAL, HUGE_VAL, HUGE_VAL );
		SSSpherical velocity ( HUGE_VAL, HUGE_VAL, HUGE_VAL );
        
        position.lon = SSHourMinSec ( strRA );
        position.lat = SSDegMinSec ( strDec );
		
		float plx = strPlx.empty() ? 0.0 : stof ( strPlx );
		if ( plx > 0.0 )
        	position.rad = 1000.0 / plx;
        
		if ( ! strPMRA.empty() )
        	velocity.lon = SSAngle::fromArcsec ( stof ( strPMRA ) ) / cos ( position.lat );
		
		if ( ! strPMDec.empty() )
        	velocity.lat = SSAngle::fromArcsec ( stof ( strPMDec ) );
		
		if ( ! strRV.empty() )
        	velocity.rad = stof ( strRV ) / SSDynamics::kLightKmPerSec;
        
        float vmag = strMag.empty() ? HUGE_VAL : stof ( strMag );
        float bmag = strBmV.empty() ? HUGE_VAL : vmag - stof ( strBmV );
        
		vector<SSIdentifier> ids ( 0 );
		vector<string> names ( 0 );
		
		if ( ! strHD.empty() )
			ids.push_back ( SSIdentifier ( kCatHD, stoi ( strHD ) ) );

		if ( ! strSAO.empty() )
			ids.push_back ( SSIdentifier ( kCatSAO, stoi ( strSAO ) ) );

		if ( ! strHIP.empty() )
			ids.push_back ( SSIdentifier ( kCatHIP, stoi ( strHIP ) ) );
		
		int hip = stoi ( strHIP );
        SSStar star ( kStar, names, ids, position, velocity, vmag, bmag, strSpec );
		// cout << star.toCSV() << endl;
		starmap.insert ( { hip, star } );
	}
    
	// Report success or failure.  Return star map object.

	if ( linecount == starmap.size() )
        cout << "Success: " << filename << " linecount " << linecount << " == starmap.size() " << starmap.size() << endl;
    else
        cout << "Failure: " << filename << " linecount " << linecount << " != starmap.size() " << starmap.size() << endl;

    return starmap;
}

// Imports the main Hipparcos star catalog.
// Adds HR, Bayer/Flamsteed, and GCVS identifiers from auxiliary identification tables (mapHIPtoHR, mapHIPtoBF, mapHIPtoVar).
// Adds SAO identifiers and radial velocity from Hipparcos Input Catalog (mapHIC).
// Uses position and proper motion with values from Hippacos New Reduction (mapHIP2) if possible.
// Adds star name strings from a mapping of HIP numbers to names (mapHIPNames).

SSStarMap importHIP ( const char *filename, HIPMap mapHIPtoHR, HIPMap mapHIPtoBF, HIPMap mapHIPtoVar, SSStarMap mapHIC, SSStarMap mapHIP2, HIPNameMap mapHIPNames )
{
	SSStarMap starmap;
	ifstream file ( filename );
	
	if ( ! file )
	{
		cout << "Failure: can't open " << filename << endl;
		return starmap;
	}

	string line = "";
	int linecount = 0;
	
	while ( getline ( file, line ) )
	{
		linecount++;
		
		string strHIP = trim ( line.substr ( 8, 6 ) );
		string strRA = trim ( line.substr ( 51, 12 ) );
		string strDec = trim ( line.substr ( 64, 12 ) );
		string strPMRA = trim ( line.substr ( 87, 8 ) );
		string strPMDec = trim ( line.substr ( 96, 8 ) );
		string strMag = trim ( line.substr ( 41, 5 ) );
		string strBmV = trim ( line.substr ( 245, 6 ) );
		string strPlx = trim ( line.substr ( 79, 7 ) );
		string strSpec = trim ( line.substr ( 435, 12 ) );
		string strHD = trim ( line.substr ( 390, 6 ) );
		string strBD = trim ( line.substr ( 398, 9 ) );
		string strCD = trim ( line.substr ( 409, 9 ) );
		string strCP = trim ( line.substr ( 420, 9 ) );

		SSSpherical position ( HUGE_VAL, HUGE_VAL, HUGE_VAL );
		SSSpherical velocity ( HUGE_VAL, HUGE_VAL, HUGE_VAL );
        
		// Get right ascension and convert to radians
		
		if ( ! strRA.empty() )
			position.lon = SSAngle::fromDegrees ( stof ( strRA ) );
		else
			position.lon = SSHourMinSec ( trim ( line.substr ( 17, 11 ) ) );
		
		// Get declination and convert to radians
		
		if ( ! strDec.empty() )
			position.lat = SSAngle::fromDegrees ( stof ( strDec ) );
		else
			position.lat = SSDegMinSec ( trim ( line.substr ( 29, 11 ) ) );
		
		// Get proper motion in RA and convert to radians per year
		
		if ( ! strPMRA.empty() )
        	velocity.lon = SSAngle::fromArcsec ( stof ( strPMRA ) / 1000.0 ) / cos ( position.lat );
		
		// Get proper motion in Dec and convert to radians per year
		
		if ( ! strPMDec.empty() )
        	velocity.lat = SSAngle::fromArcsec ( stof ( strPMDec ) / 1000.0 );
		
		// If proper motion is valid, use it to update position and proper motion from J1991.25 to J2000.
		
		if ( ! isinf ( velocity.lon ) && ! isinf ( velocity.lat ) )
			updateHIPStarPositionVelocity ( position, velocity );
		
		// Get Johnson V magnitude, and (if present) get B-V color index then compute Johnson B magnitude.
		
        float vmag = strMag.empty() ? HUGE_VAL : stof ( strMag );
        float bmag = strBmV.empty() ? HUGE_VAL : vmag - stof ( strBmV );

		// If we have a parallax > 1 milliarcsec, use it to compute distance in parsecs.
		
		float plx = strPlx.empty() ? 0.0 : stof ( strPlx );
		if ( plx > 0.0 )
        	position.rad = 1000.0 / plx;
        
		// Set up name and identifier vectors.

		vector<SSIdentifier> ids ( 0 );
		vector<string> names ( 0 );

		// Parse HIP catalog number and add Hipparcos identifier.

		int hip = strtoint ( strHIP );
		SSIdentifier hipID = SSIdentifier ( kCatHIP, hip );
		ids.push_back ( hipID );

		// Add Henry Draper and Durchmusterung identifiers.
		
		if ( ! strHD.empty() )
			ids.push_back ( SSIdentifier ( kCatHD, stoi ( strHD ) ) );

		if ( ! strBD.empty() )
			ids.push_back ( SSIdentifier::fromString ( "BD " + strBD ) );
		
		if ( ! strCD.empty() )
			ids.push_back ( SSIdentifier::fromString ( "CD " + strCD ) );

		if ( ! strCP.empty() )
			ids.push_back ( SSIdentifier::fromString ( "CP " + strCP ) );

		// Add HR identification (if present) from Bright Star ident table.
		
		auto rangeHR = mapHIPtoHR.equal_range ( hip );
		for ( auto i = rangeHR.first; i != rangeHR.second; i++ )
			ids.push_back ( i->second );

		// Add Bayer and Flamsteed identifier(s) (if present) from Bayer ident table.

		auto rangeBF = mapHIPtoBF.equal_range ( hip );
		for ( auto i = rangeBF.first; i != rangeBF.second; i++ )
			ids.push_back ( i->second );
		
		// Add GCVS identifier(s) from the variable star ident table.
		// Don't add GCVS identifiers which are Bayer/Flamsteed letters!

		auto rangeVar = mapHIPtoVar.equal_range ( hip );
		for ( auto i = rangeVar.first; i != rangeVar.second; i++ )
			if ( i->second.catalog() == kCatGCVS )
				ids.push_back ( i->second );

		// If we found a matching Hipparcos New Reduction star,
		// replace position and velocity with newer values.
		
		SSStar hip2Star = mapHIP2[ hip ];
		if ( hip2Star.getIdentifier ( kCatHIP ) == hipID )
		{
			position = hip2Star.getFundamentalPosition();
			velocity = hip2Star.getFundamentalProperMotion();
		}

		// If we found a matching Hipparcos Input Catalog star,
		// splice in SAO identifier and radial velocity.
		
		SSStar hicStar = mapHIC[ hip ];
		if ( hicStar.getIdentifier ( kCatHIP ) == hipID )
		{
			SSIdentifier saoID = hicStar.getIdentifier ( kCatSAO );
			if ( saoID )
				ids.push_back ( saoID );
			
			velocity.rad = hicStar.getRadVel();
		}
		
		// Add names(s) from HIP number to name map.

		auto rangeNames = mapHIPNames.equal_range ( hip );
		for ( auto i = rangeNames.first; i != rangeNames.second; i++ )
			names.push_back ( i->second );

		// Sert identifier vector.  Construct star and insert into star map object.
		
		sort ( ids.begin(), ids.end(), compareSSIdentifiers );
		SSStar star ( kStar, names, ids, position, velocity, vmag, bmag, strSpec );
		cout << star.toCSV() << endl;
		starmap.insert ( { hip, star } );
	}

	// Report success or failure.  Return star map object.

	if ( linecount == starmap.size() )
		cout << "Success: " << filename << " linecount " << linecount << " == starmap.size() " << starmap.size() << endl;
	else
		cout << "Failure: " << filename << " linecount " << linecount << " != starmap.size() " << starmap.size() << endl;

	return starmap;
}

HIPMap importHIPtoHRMap ( const char *filename )
{
	HIPMap mapHIPtoHR;
	
	ifstream file ( filename );
	
	if ( ! file )
	{
		cout << "Failure: can't open " << filename << endl;
		return mapHIPtoHR;
	}
	
	string line ( "" );
	int linecount = 0;
	
	while ( getline ( file, line ) )
	{
		linecount++;
		string strHR = trim ( line.substr ( 0, 6 ) );
		string strHIP = trim ( line.substr ( 7, 6 ) );
		int hip = stoi ( strHIP );

		SSIdentifier id = SSIdentifier ( kCatHR, stoi ( strHR ) );
		// cout << hip << "," << id.toString() << "," << endl;
		mapHIPtoHR.insert ( { hip, id } );
	}
	
	// Report success or failure.  Return identifier map object.

	if ( linecount == mapHIPtoHR.size() )
		cout << "Success: " << filename << " linecount " << linecount << " == mapHIPtoHR.size() " << mapHIPtoHR.size() << endl;
	else
		cout << "Failure: " << filename << " linecount " << linecount << " != mapHIPtoHR.size() " << mapHIPtoHR.size() << endl;

	return mapHIPtoHR;
}


HIPMap importHIPtoBayerFlamsteedMap ( const char *filename )
{
	HIPMap mapHIPtoBF;
	ifstream file ( filename );
	if ( ! file )
	{
		cout << "Failure: can't open " << filename << endl;
		return mapHIPtoBF;
	}
	
	string line = "";
	int linecount = 0;

	while ( getline ( file, line ) )
	{
		linecount++;
		string strBF = trim ( line.substr ( 0, 11 ) );
		string strHIP = trim ( line.substr ( 12, 6 ) );
		int hip = stoi ( strHIP );

		strBF = cleanHIPNameString ( strBF );
		SSIdentifier id = SSIdentifier::fromString ( strBF );
		
		// cout << hip << "," << id.toString() << endl;
		mapHIPtoBF.insert ( { hip, id } );
	}

	// Report success or failure.  Return identifier map object.

	if ( linecount == mapHIPtoBF.size() )
		cout << "Success: " << filename << " linecount " << linecount << " == mapHIPtoBF.size() " << mapHIPtoBF.size() << endl;
	else
		cout << "Failure: " << filename << " linecount " << linecount << " != mapHIPtoBF.size() " << mapHIPtoBF.size() << endl;

	return mapHIPtoBF;
}

HIPMap importHIPtoVarMap ( const char *filename )
{
	HIPMap mapHIPtoVar;
	ifstream file ( filename );
	if ( ! file )
	{
		cout << "Failure: can't open " << filename << endl;
		return mapHIPtoVar;
	}
	
	string line = "";
	int linecount = 0;
	
	while ( getline ( file, line ) )
	{
		linecount++;
		string strVar = trim ( line.substr ( 0, 11 ) );
		string strHIP = trim ( line.substr ( 12, 6 ) );
		int hip = stoi ( strHIP );

		strVar = cleanHIPNameString ( strVar );
		SSIdentifier id = SSIdentifier::fromString ( strVar );
		// cout << hip << "," << id.toString() << endl;
		mapHIPtoVar.insert ( { hip, id } );
	}

	// Report success or failure.  Return identifier map object.

	if ( linecount == mapHIPtoVar.size() )
		cout << "Success: " << filename << " linecount " << linecount << " == mapHIPtoVar.size() " << mapHIPtoVar.size() << endl;
	else
		cout << "Failure: " << filename << " linecount " << linecount << " != mapHIPtoVar.size() " << mapHIPtoVar.size() << endl;
	
	return mapHIPtoVar;
}

// Imports Hipparcos New Reduction 2007 star catalog.

SSStarMap importHIP2 ( const char *filename )
{
	SSStarMap mapHIP2;
	
	ifstream file ( filename );
	if ( ! file )
	{
		cout << "Failure: can't open " << filename << endl;
		return mapHIP2;
	}

	string line = "";
	int linecount = 0;
	
	while ( getline ( file, line ) )
	{
		linecount++;
		
		string strHIP = trim ( line.substr ( 0, 6 ) );
		string strRA = trim ( line.substr ( 15, 13 ) );
		string strDec = trim ( line.substr ( 29, 13 ) );
		string strPMRA = trim ( line.substr ( 51, 8 ) );
		string strPMDec = trim ( line.substr ( 60, 8 ) );
		string strMag = trim ( line.substr ( 129, 7 ) );
		string strBmV = trim ( line.substr ( 152, 6 ) );
		string strPlx = trim ( line.substr ( 43, 7 ) );

		if ( strRA.empty() || strDec.empty() )
			continue;
		
		SSSpherical position ( HUGE_VAL, HUGE_VAL, HUGE_VAL );
		SSSpherical velocity ( HUGE_VAL, HUGE_VAL, HUGE_VAL );

		// Get right ascension and declination in radians
		
		position.lon = strtofloat ( strRA );
		position.lat = strtofloat ( strDec );
		
		// Get proper motion in RA and Dec and convert to radians per year
		
		if ( ! strPMRA.empty() )
			velocity.lon = SSAngle::fromArcsec ( strtofloat ( strPMRA ) / 1000.0 ) / cos ( position.lat );
		
		if ( ! strPMDec.empty() )
			velocity.lat = SSAngle::fromArcsec ( strtofloat ( strPMDec ) / 1000.0 );
		
		// If proper motion is valid, use it to bring position from J1991.25 to J2000
		
		if ( ! isinf ( velocity.lon ) && ! isinf ( velocity.lat ) )
			updateHIPStarPositionVelocity ( position, velocity );
		
		// Get Hipparcos magnitude
		
		float vmag = HUGE_VAL;
		if ( ! strMag.empty() )
			vmag = strtofloat ( strMag );
		
		// Get B-V color index and use it to convert Hipparcos magnitude to Johnson B and V
		
		float bmv = HUGE_VAL, bmag = HUGE_VAL;
		if ( ! strBmV.empty() )
		{
			bmv = strtofloat ( strBmV );
			vmag += -0.2964 * bmv + 0.1110 * bmv * bmv;
			bmag = vmag + bmv;
		}
		
		// If we have a parallax greater than 1 milliarcec, use it to compute distance in parsecs
		
		if ( ! strPlx.empty() )
		{
			float plx = strtofloat ( strPlx );
			if ( plx > 1.0 )
				position.rad = 1000.0 / plx;
		}
		
		// Add single Hipparcos identifier and empty name string.
		
		vector<SSIdentifier> ids ( 0 );
		vector<string> names ( 0 );

		int hip = stoi ( strHIP );
		ids.push_back ( SSIdentifier ( kCatHIP, hip ) );
		
		// Construct star and insert into map.
		
        SSStar star ( kStar, names, ids, position, velocity, vmag, bmag, "" );
		// cout << star.toCSV() << endl;
		mapHIP2.insert ( { hip, star } );
	}
	
	// Report success or failure.  Return star map object.
	
	if ( linecount == mapHIP2.size() )
		cout << "Success: " << filename << " linecount " << linecount << " == mapHIP2.size() " << mapHIP2.size() << endl;
	else
		cout << "Failure: " << filename << " linecount " << linecount << " != mapHIP2.size() " << mapHIP2.size() << endl;

	return mapHIP2;
}

HIPNameMap importHIPNameMap ( const char *filename )
{
	HIPNameMap hipNameMap;
	
	ifstream file ( filename );
	if ( ! file )
	{
		cout << "Failure: can't open " << filename << endl;
		return hipNameMap;
	}

	string line = "";
	int linecount = 0;

	while ( getline ( file, line ) )
	{
		linecount++;
		string strHIP = trim ( line.substr ( 17, 6 ) );
		string strName = trim ( line.substr ( 0, 16 ) );
		
		int hip = strtoint ( strHIP );
		if ( ! hip )
			continue;
		
		// cout << hip << "," << strName << endl;
		hipNameMap.insert ( { hip, strName } );
	}

	return hipNameMap;
}

HIPNameMap importIAUNameMap ( const char *filename )
{
	HIPNameMap hipNameMap;
	
	ifstream file ( filename );
	if ( ! file )
	{
		cout << "Failure: can't open " << filename << endl;
		return hipNameMap;
	}

	string line = "";
	int linecount = 0;

	while ( getline ( file, line ) )
	{
		linecount++;
		if ( line.length() < 96 )
			continue;
		
		string strHIP = trim ( line.substr ( 91, 6 ) );
		string strName = trim ( line.substr ( 0, 18 ) );
		
		int hip = strtoint ( strHIP );
		if ( ! hip )
			continue;
		
		// cout << hip << "," << strName << endl;
		hipNameMap.insert ( { hip, strName } );
	}

	return hipNameMap;
}
