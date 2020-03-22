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
typedef map<SSIdentifier,string> SSStarNameMap;
typedef map<int,SSStar> SSStarMap;

SSStarMap importHIC ( const char *filename );
SSStarMap importHIP ( const char *filename, HIPMap mapHIPtoHR, HIPMap mapHIPtoBF, HIPMap mapHIPtoVar, SSStarMap mapHIC, SSStarMap mapHIP2, SSStarNameMap mapNames );
HIPMap importHIPtoHRMap ( const char *filename );
HIPMap importHIPtoBayerFlamsteedMap ( const char *filename );
HIPMap importHIPtoVarMap ( const char *filename );
HIPNameMap importHIPNameMap ( const char *filename );
SSStarNameMap importIAUStarNames ( const char *filename );
SSStarMap importHIP2 ( const char *filename );
vector<SSStar> importSKY2000 ( const char *filename, SSStarNameMap &nameMap );

int main ( int argc, char *argv[] )
{
	SSStarNameMap nameMap = importIAUStarNames ( "/Users/timmyd/Projects/SouthernStars/Projects/Star Names/IAU-CSN.txt" );
	HIPNameMap mapHIPtoHIPName = importHIPNameMap ( "/Users/timmyd/Projects/SouthernStars/Catalogs/Hipparcos/TABLES/IDENT6.DOC" );

	HIPMap mapHIPtoHR = importHIPtoHRMap ( "/Users/timmyd/Projects/SouthernStars/Catalogs/Hipparcos/TABLES/IDENT3.DOC" );
	HIPMap mapHIPtoBF = importHIPtoBayerFlamsteedMap ( "/Users/timmyd/Projects/SouthernStars/Catalogs/Hipparcos/TABLES/IDENT4.DOC" );
	HIPMap mapHIPtoVar = importHIPtoVarMap ( "/Users/timmyd/Projects/SouthernStars/Catalogs/Hipparcos/TABLES/IDENT5.DOC" );
	SSStarMap mapHIC = importHIC ( "/Users/timmyd/Projects/SouthernStars/Catalogs/Hipparcos Input Catalog/main.dat" );
	SSStarMap mapHIP2 = importHIP2 ( "/Users/timmyd/Projects/SouthernStars/Catalogs/Hipparcos New Reduction 2007/hip2.dat" );
	SSStarMap mapHIP = importHIP ( "/Users/timmyd/Projects/SouthernStars/Catalogs/Hipparcos/CATS/HIP_MAIN.DAT", mapHIPtoHR, mapHIPtoBF, mapHIPtoVar, mapHIC, mapHIP2, nameMap );

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

// Adds a new identifier to a vector of identifiers,
// if the new identifier is valid and not already present in the vector.

void addIdentifier ( vector<SSIdentifier> &identVec, SSIdentifier ident )
{
	if ( ident && find ( identVec.begin(), identVec.end(), ident ) == identVec.end() )
		identVec.push_back ( ident );
}

// Imports IAU official star name table from Working Group on Star Names
// from http://www.pas.rochester.edu/~emamajek/WGSN/IAU-CSN.txt
// Assumes names are unique (i.e. only one name per identifier);
// discards additional names beyond the first for any given identifier.
// Returns map of name strings indexed by identifier.

SSStarNameMap importIAUStarNames ( const char *filename )
{
	SSStarNameMap nameMap;
	
	// Open file; report error and return empty map on failure.

	ifstream file ( filename );
	if ( ! file )
	{
		cout << "Failure: can't open " << filename << endl;
		return nameMap;
	}

	// Read file line-by-line until we reach end-of-file

	string line = "";
	int linecount = 0;

	while ( getline ( file, line ) )
	{
		linecount++;
		if ( line.length() < 96 )
			continue;
		
		// Extract main identifier, Hipparcos number, and name
		
		string strIdent = trim ( line.substr ( 36, 13 ) );
		string strHIP = trim ( line.substr ( 91, 6 ) );
		string strName = trim ( line.substr ( 0, 18 ) );
		
		// Construct identifier from main ident string, or HIP number if that fails.
		
		SSIdentifier ident = SSIdentifier::fromString ( strIdent );
		if ( ! ident )
		{
			int hip = strtoint ( strHIP );
			if ( hip )
				ident = SSIdentifier ( kCatHIP, hip );
		}
		
		// If successful, iInsert identifier and name into map; display warning on failure.
		
		if ( ident )
			nameMap.insert ( { ident, strName } );
		else
			cout << "Warning: can't convert " << strIdent << " for " << strName << endl;
	}

	// Return fully-imported name map.
	
	return nameMap;
}

// Given a vector of identifiers, returns vector of all corresponding name strings
// from the input star name map.  If no names correspond to any identifiers,
// returns a zero-length vector.

vector<string> getStarNames ( vector<SSIdentifier> &idents, SSStarNameMap nameMap )
{
	vector<string> names ( 0 );

	for ( SSIdentifier ident : idents )
	{
		string name = nameMap[ ident ];
		if ( name.length() > 0 )
			names.push_back ( name );
	}

	return names;
}

// Cleans up some oddball conventions in the Hipparcos star name identification tables
// for Bayer, Flamsteed, and variable star names so SSIdentifier understands them.
// Returns cleaned-up name string, does not modify input string.

string cleanHIPNameString ( string str )
{
	// Change abbreviation for "alpha" from "alf" to "alp"
	
	if ( str.find ( "alf" ) == 0 )
		str.replace ( 0, 3, "alp" );
	
	// Change abbreviation for "xi" from "ksi"
	
	if ( str.find ( "ksi" ) == 0 )
		str.replace ( 0, 3, "xi." );
	
	// Remove "." after "mu", "nu", "xi"
	
	size_t idx = str.find ( "." );
	if ( idx != string::npos )
		str.erase ( idx, 1 );
	
	// Remove multiple star designations "_A", "_B", "_C" etc. after constellation
	
	size_t len = str.length();
	if ( str[ len - 2 ] == '_' )
		str.erase ( len - 2, 2 );
	
	// Convert remaining underscores to whitespace.
	
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
		
		float plx = strPlx.empty() ? 0.0 : strtofloat ( strPlx );
		if ( plx > 0.0 )
        	position.rad = 1000.0 / plx;
        
		if ( ! strPMRA.empty() )
        	velocity.lon = SSAngle::fromArcsec ( strtofloat ( strPMRA ) ) / cos ( position.lat );
		
		if ( ! strPMDec.empty() )
        	velocity.lat = SSAngle::fromArcsec ( strtofloat ( strPMDec ) );
		
		if ( ! strRV.empty() )
        	velocity.rad = strtofloat ( strRV ) / SSDynamics::kLightKmPerSec;
        
        float vmag = strMag.empty() ? HUGE_VAL : strtofloat ( strMag );
        float bmag = strBmV.empty() ? HUGE_VAL : vmag - strtofloat ( strBmV );
        
		vector<SSIdentifier> idents ( 0 );
		vector<string> names ( 0 );
		
		if ( ! strHD.empty() )
			addIdentifier ( idents, SSIdentifier ( kCatHD, strtoint ( strHD ) ) );

		if ( ! strSAO.empty() )
			addIdentifier ( idents, SSIdentifier ( kCatSAO, strtoint ( strSAO ) ) );

		if ( ! strHIP.empty() )
			addIdentifier ( idents, SSIdentifier ( kCatHIP, strtoint ( strHIP ) ) );
		
		int hip = strtoint ( strHIP );
        SSStar star ( kStar, names, idents, position, velocity, vmag, bmag, strSpec );
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
// Adds star name strings from a mapping of identifiers to names (nameMap).
// Returns map of SSStar objects indexed by HIP number, which should contain 118218 entries if successful.

SSStarMap importHIP ( const char *filename, HIPMap mapHIPtoHR, HIPMap mapHIPtoBF, HIPMap mapHIPtoVar, SSStarMap mapHIC, SSStarMap mapHIP2, SSStarNameMap nameMap )
{
	SSStarMap starmap;
	ifstream file ( filename );
	
	if ( ! file )
	{
		cout << "Failure: can't open " << filename << endl;
		return starmap;
	}

	// Read file line-by-line until we reach end-of-file

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
			position.lon = SSAngle::fromDegrees ( strtofloat ( strRA ) );
		else
			position.lon = SSHourMinSec ( trim ( line.substr ( 17, 11 ) ) );
		
		// Get declination and convert to radians
		
		if ( ! strDec.empty() )
			position.lat = SSAngle::fromDegrees ( strtofloat ( strDec ) );
		else
			position.lat = SSDegMinSec ( trim ( line.substr ( 29, 11 ) ) );
		
		// Get proper motion in RA and convert to radians per year
		
		if ( ! strPMRA.empty() )
        	velocity.lon = SSAngle::fromArcsec ( strtofloat ( strPMRA ) / 1000.0 ) / cos ( position.lat );
		
		// Get proper motion in Dec and convert to radians per year
		
		if ( ! strPMDec.empty() )
        	velocity.lat = SSAngle::fromArcsec ( strtofloat ( strPMDec ) / 1000.0 );
		
		// If proper motion is valid, use it to update position and proper motion from J1991.25 to J2000.
		
		if ( ! isinf ( velocity.lon ) && ! isinf ( velocity.lat ) )
			updateHIPStarPositionVelocity ( position, velocity );
		
		// Get Johnson V magnitude, and (if present) get B-V color index then compute Johnson B magnitude.
		
        float vmag = strMag.empty() ? HUGE_VAL : strtofloat ( strMag );
        float bmag = strBmV.empty() ? HUGE_VAL : vmag - strtofloat ( strBmV );

		// If we have a parallax > 1 milliarcsec, use it to compute distance in parsecs.
		
		float plx = strPlx.empty() ? 0.0 : strtofloat ( strPlx );
		if ( plx > 0.0 )
        	position.rad = 1000.0 / plx;
        
		// Set up name and identifier vectors.

		vector<SSIdentifier> idents ( 0 );
		vector<string> names ( 0 );

		// Parse HIP catalog number and add Hipparcos identifier.

		int hip = strtoint ( strHIP );
		SSIdentifier hipID = SSIdentifier ( kCatHIP, hip );
		addIdentifier ( idents, hipID );

		// Add Henry Draper and Durchmusterung identifiers.
		
		if ( ! strHD.empty() )
			addIdentifier ( idents, SSIdentifier ( kCatHD, strtoint ( strHD ) ) );

		if ( ! strBD.empty() )
			addIdentifier ( idents, SSIdentifier::fromString ( "BD " + strBD ) );
		
		if ( ! strCD.empty() )
			addIdentifier ( idents, SSIdentifier::fromString ( "CD " + strCD ) );

		if ( ! strCP.empty() )
			addIdentifier ( idents, SSIdentifier::fromString ( "CP " + strCP ) );

		// Add HR identification (if present) from Bright Star identification table.
		
		auto rangeHR = mapHIPtoHR.equal_range ( hip );
		for ( auto i = rangeHR.first; i != rangeHR.second; i++ )
			addIdentifier ( idents, i->second );

		// Add Bayer and Flamsteed identifier(s) (if present) from Bayer identification table.

		auto rangeBF = mapHIPtoBF.equal_range ( hip );
		for ( auto i = rangeBF.first; i != rangeBF.second; i++ )
			addIdentifier ( idents, i->second );
		
		// Add GCVS identifier(s) from the variable star ident table.
		// Don't add GCVS identifiers which are Bayer/Flamsteed letters!

		auto rangeVar = mapHIPtoVar.equal_range ( hip );
		for ( auto i = rangeVar.first; i != rangeVar.second; i++ )
			if ( i->second.catalog() == kCatGCVS )
				addIdentifier ( idents, i->second );

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
				addIdentifier ( idents, saoID );
			
			velocity.rad = hicStar.getRadVel();
		}

		// Sert identifier vector.  Add names(s) from identifier-to-name map.
		
		sort ( idents.begin(), idents.end() ); // , compareSSIdentifiers );
		names = getStarNames ( idents, nameMap );
		
		// Construct star and insert into star map object.

		SSStar star ( kStar, names, idents, position, velocity, vmag, bmag, strSpec );
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

// Imports Hipparcos HR (Bright Star) identifier table (IDENT3.DOC).
// Returns map of HR identifiers indexed by HIP number,
// which should contain 9077 entries if successful.

HIPMap importHIPtoHRMap ( const char *filename )
{
	HIPMap mapHIPtoHR;
	
	// Open file; report error and return empty map on failure.
	
	ifstream file ( filename );
	if ( ! file )
	{
		cout << "Failure: can't open " << filename << endl;
		return mapHIPtoHR;
	}
	
	// Read file line-by-line until we reach end-of-file

	string line ( "" );
	int linecount = 0;
	
	while ( getline ( file, line ) )
	{
		linecount++;
		string strHR = trim ( line.substr ( 0, 6 ) );
		string strHIP = trim ( line.substr ( 7, 6 ) );
		int hip = strtoint ( strHIP );

		SSIdentifier id = SSIdentifier ( kCatHR, strtoint ( strHR ) );
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

// Imports Hipparcos Bayer/Flamsteed identifier table (IDENT4.DOC).
// Returns map of Bayer/Flamsteed identifiers indexed by HIP number,
// which should contain 4440 entries if successful.

HIPMap importHIPtoBayerFlamsteedMap ( const char *filename )
{
	HIPMap mapHIPtoBF;
	
	// Open file; report error and return empty map on failure.

	ifstream file ( filename );
	if ( ! file )
	{
		cout << "Failure: can't open " << filename << endl;
		return mapHIPtoBF;
	}
	
	// Read file line-by-line until we reach end-of-file

	string line = "";
	int linecount = 0;

	while ( getline ( file, line ) )
	{
		linecount++;
		string strBF = trim ( line.substr ( 0, 11 ) );
		string strHIP = trim ( line.substr ( 12, 6 ) );
		int hip = strtoint ( strHIP );

		strBF = cleanHIPNameString ( strBF );
		SSIdentifier id = SSIdentifier::fromString ( strBF );
		
		// cout << hip << "," << id.toString() << endl;
		if ( id )
			mapHIPtoBF.insert ( { hip, id } );
		else
			cout << "Warning: con't convert " << strBF << " for HIP " << hip << endl;
	}

	// Report success or failure.  Return identifier map object.

	if ( linecount == mapHIPtoBF.size() )
		cout << "Success: " << filename << " linecount " << linecount << " == mapHIPtoBF.size() " << mapHIPtoBF.size() << endl;
	else
		cout << "Failure: " << filename << " linecount " << linecount << " != mapHIPtoBF.size() " << mapHIPtoBF.size() << endl;

	return mapHIPtoBF;
}

// Imports Hipparcos variable star identifier table (IDENT5.DOC).
// Returns map of GCVS identifiers indexed by HIP number,
// which should contain 6390 entries if successful.

HIPMap importHIPtoVarMap ( const char *filename )
{
	HIPMap mapHIPtoVar;

	// Open file; report error and return empty map on failure.

	ifstream file ( filename );
	if ( ! file )
	{
		cout << "Failure: can't open " << filename << endl;
		return mapHIPtoVar;
	}
	
	// Read file line-by-line until we reach end-of-file

	string line = "";
	int linecount = 0;
	
	while ( getline ( file, line ) )
	{
		linecount++;
		string strVar = trim ( line.substr ( 0, 11 ) );
		string strHIP = trim ( line.substr ( 12, 6 ) );
		int hip = strtoint ( strHIP );

		strVar = cleanHIPNameString ( strVar );
		SSIdentifier id = SSIdentifier::fromString ( strVar );
		// cout << hip << "," << id.toString() << endl;
		
		if ( id )
			mapHIPtoVar.insert ( { hip, id } );
		else
			cout << "Warning: con't convert " << strVar << " for HIP " << hip << endl;
	}

	// Report success or failure.  Return identifier map object.

	if ( linecount == mapHIPtoVar.size() )
		cout << "Success: " << filename << " linecount " << linecount << " == mapHIPtoVar.size() " << mapHIPtoVar.size() << endl;
	else
		cout << "Failure: " << filename << " linecount " << linecount << " != mapHIPtoVar.size() " << mapHIPtoVar.size() << endl;
	
	return mapHIPtoVar;
}

// Imports Hipparcos New Reduction 2007 star catalog.
// Returns map of SSStar objects indexed by Hipparcos number.
// If successful, map should contain 117955 entries.

SSStarMap importHIP2 ( const char *filename )
{
	SSStarMap mapHIP2;
	
	// Open file; report error and return empty map on failure.

	ifstream file ( filename );
	if ( ! file )
	{
		cout << "Failure: can't open " << filename << endl;
		return mapHIP2;
	}

	// Read file line-by-line until we reach end-of-file

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
		
		vector<SSIdentifier> idents ( 0 );
		vector<string> names ( 0 );

		int hip = strtoint ( strHIP );
		addIdentifier ( idents, SSIdentifier ( kCatHIP, hip ) );
		
		// Construct star and insert into map.
		
        SSStar star ( kStar, names, idents, position, velocity, vmag, bmag, "" );
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

// Imports Hipparcos star name identification table (IDENT6.DOC).
// Returns map of name strings identifiers indexed by HIP number,
// which should contain 96 entries if successful.

HIPNameMap importHIPNameMap ( const char *filename )
{
	HIPNameMap hipNameMap;
	
	// Open file; report error and return empty map on failure.

	ifstream file ( filename );
	if ( ! file )
	{
		cout << "Failure: can't open " << filename << endl;
		return hipNameMap;
	}

	// Read file line-by-line until we reach end-of-file

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

// Imports SKY2000 Master Star Catalog v5.
// Inserts name strings from nameNap;
// Returns vector of SSStar objects which should contain 299460 entries if successful.

vector<SSStar> importSKY2000 ( const char *filename, SSStarNameMap &nameMap )
{
	vector<SSStar> starVec;
	
	// Open file; report error and return empty map on failure.

	ifstream file ( filename );
	if ( ! file )
	{
		cout << "Failure: can't open " << filename << endl;
		return starVec;
	}

	// Read file line-by-line until we reach end-of-file

	string line = "";
	int linecount = 0;

	while ( getline ( file, line ) )
	{
		linecount++;
		if ( line.length() < 96 )
			continue;
		
		string strHD = trim ( line.substr ( 35, 6 ) );
		string strSAO = trim ( line.substr ( 43, 6 ) );
		string strDM = trim ( line.substr ( 50, 5 ) ) + " " + trim ( line.substr ( 55, 5 ) );
		string strHR = trim ( line.substr ( 63, 4 ) );
		string strWDS = trim ( line.substr ( 67, 12 ) );
		
		// Extract Bayer/Flamsteed names like "21alp And" and "kap1Scl"; ignore AG catalog numbers.
		
		string strBay = "";
		string strFlm = "";
		string strName = line.substr ( 98, 10 );
		if ( strName.find ( "AG" ) != 0 )
		{
			string strCon = trim ( line.substr ( 105, 3 ) );
			if ( ! strCon.empty() )
			{
				strFlm = trim ( line.substr ( 98, 3 ) );
				strBay = trim ( line.substr ( 101, 4 ) );
				
				if ( ! strFlm.empty() )
					strFlm = strFlm + " " + strCon;
				
				if ( ! strBay.empty() )
					strBay = strBay + " " + strCon;
			}
		}
		
		string strVar = trim ( line.substr ( 108, 10 ) );

		// Extract RA and Dec, adding whitespace to separate hour/deg, min, sec
		
		string strRA = trim ( line.substr ( 118, 2 ) ) + " "
                     + trim ( line.substr ( 120, 2 ) ) + " "
                     + trim ( line.substr ( 122, 7 ) );
		
        string strDec = trim ( line.substr ( 129, 1 ) )
		              + trim ( line.substr ( 130, 2 ) ) + " "
                      + trim ( line.substr ( 132, 2 ) ) + " "
		              + trim ( line.substr ( 134, 6 ) );

		// Extract proper motion, removing whitepace after sign of PM in Dec.
		
		string strPMRA = trim ( line.substr ( 149, 8 ) );
		string strPMDec = trim ( line.substr ( 157, 1 ) )
                        + trim ( line.substr ( 158, 7 ) );
		
		// Extract radial velocity, removing whitespace after sign
		
		string strRV = trim ( line.substr ( 167, 1 ) )
                     + trim ( line.substr ( 168, 5 ) );
        
		string strPlx = trim ( line.substr ( 175, 8 ) );
		string strPlxErr = trim ( line.substr ( 183, 8 ) );
		
		// Extract Johnson V magnitude.  Get observed V if present; otherwise get derived V.
		
		string strMag = trim ( line.substr ( 232, 6 ) );
		if ( strMag.empty() )
			strMag = trim ( line.substr ( 238, 5 ) );
		
		string strBmV = trim ( line.substr ( 258, 6 ) );

		string strSpec = trim ( line.substr ( 304, 30 ) );
		if ( strSpec.empty() )
			strSpec = trim ( line.substr ( 336, 3 ) );
		
		// Extract separation and magnitude difference between components,
		// position angle, year of measurement, and component identifiers.

		string strDblSep = trim ( line.substr ( 341, 7 ) );
		string strDblMag = trim ( line.substr ( 348, 5 ) );
		string strDblPA = trim ( line.substr ( 360, 3 ) );
		string strDblPAyr = trim ( line.substr ( 363, 7 ) );
		string strDblComp = trim ( line.substr ( 77, 5 ) );

		string strVarMax = trim ( line.substr ( 411, 5 ) );
		string strVarMin = trim ( line.substr ( 416, 5 ) );
		string strVarPer = trim ( line.substr ( 427, 8 ) );
		string strVarEpoch = trim ( line.substr ( 435, 8 ) );
		string strVarType = trim ( line.substr ( 443, 3 ) );

        SSHourMinSec ra ( strRA );
        SSDegMinSec dec ( strDec );
        
        double pmRA = HUGE_VAL;
        if ( ! strPMRA.empty() )
            pmRA = SSAngle::fromArcsec ( strtofloat ( strPMRA ) );
        
        double pmDec = HUGE_VAL;
        if ( ! strPMDec.empty() )
            pmDec = SSAngle::fromArcsec ( strtofloat ( strPMDec ) );
        
        SSSpherical position ( ra, dec, HUGE_VAL );
        SSSpherical velocity ( pmRA, pmDec, HUGE_VAL );
        
        double plx = strtofloat ( strPlx );
        if ( plx > 0.001 )
            position.rad = 1.0 / plx;
        
        if ( ! strRV.empty() )
            velocity.rad = strtofloat ( strRV ) / SSDynamics::kLightKmPerSec;
        
        float vmag = HUGE_VAL;
        if ( ! strMag.empty() )
            vmag = strtofloat ( strMag );
        
		// Get Johnson B magnitude from color index
		
        float bmag = HUGE_VAL;
        if ( ! strBmV.empty() )
            bmag = strtofloat ( strBmV ) + vmag;
        
		// Set up name and identifier vectors.

        vector<SSIdentifier> idents ( 0 );
        vector<string> names ( 0 );
        
		if ( ! strBay.empty() )
			addIdentifier ( idents, SSIdentifier::fromString ( strBay ) );
		
		if ( ! strFlm.empty() )
			addIdentifier ( idents, SSIdentifier::fromString ( strFlm ) );
		
		if ( ! strVar.empty() )
			addIdentifier ( idents, SSIdentifier::fromString ( strVar ) );

		if ( ! strHR.empty() )
			addIdentifier ( idents, SSIdentifier ( kCatHR, strtoint ( strHR ) ) );

		if ( ! strHD.empty() )
			addIdentifier ( idents, SSIdentifier ( kCatHD, strtoint ( strHD ) ) );
		
		if ( ! strSAO.empty() )
			addIdentifier ( idents, SSIdentifier ( kCatSAO, strtoint ( strSAO ) ) );

		if ( ! strDM.empty() )
			addIdentifier ( idents, SSIdentifier::fromString ( strDM ) );
		
		// Sert identifier vector.  Get name string(s) corresponding to identifier(s).
		// Construct star and insert into star vector.
		
		sort ( idents.begin(), idents.end(), compareSSIdentifiers );
		names = getStarNames ( idents, nameMap );
		
		SSStar star ( kStar, names, idents, position, velocity, vmag, bmag, strSpec );
		cout << star.toCSV() << endl;
		starVec.push_back ( star );
	}

	return starVec;
}
