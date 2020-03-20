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

typedef multimap<int,string> HIPMap;
typedef map<int,SSStar> SSStarMap;

SSStarMap importHIC ( const char *filename );
void importHIP ( const char *filename, HIPMap mapHIPtoHR, HIPMap mapHIPtoBF, HIPMap mapHIPtoVar );
HIPMap importHIPtoHRMap ( const char *filename );
HIPMap importHIPtoBayerFlamsteedMap ( const char *filename );
HIPMap importHIPtoVarMap ( const char *filename );
void importHIP2 ( const char *filename );

int main ( int argc, char *argv[] )
{
	SSStarMap mapHIC = importHIC ( "/Users/timmyd/Projects/SouthernStars/Catalogs/Hipparcos Input Catalog/main.dat" );
	HIPMap mapHIPtoHR = importHIPtoHRMap ( "/Users/timmyd/Projects/SouthernStars/Catalogs/Hipparcos/TABLES/IDENT3.DOC" );
	HIPMap mapHIPtoBF = importHIPtoBayerFlamsteedMap ( "/Users/timmyd/Projects/SouthernStars/Catalogs/Hipparcos/TABLES/IDENT4.DOC" );
	HIPMap mapHIPtoVar = importHIPtoVarMap ( "/Users/timmyd/Projects/SouthernStars/Catalogs/Hipparcos/TABLES/IDENT5.DOC" );
	importHIP ( "/Users/timmyd/Projects/SouthernStars/Catalogs/Hipparcos/CATS/HIP_MAIN.DAT", mapHIPtoHR, mapHIPtoBF, mapHIPtoVar );
	importHIP2 ( "/Users/timmyd/Projects/SouthernStars/Catalogs/Hipparcos New Reduction 2007/hip2.dat" );

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

string trim ( string str )
{
    auto start = str.find_first_not_of ( " \t\r\n" );
    auto end = str.find_last_not_of ( " \t\r\n" );

    if ( start == string::npos )
        return string ( "" );
    else
        return str.substr ( start, ( end - start ) + 1 );
}

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
        
		int hip = stoi ( strHIP );
        SSStar star ( kStar, position, velocity, vmag, bmag, strSpec );
		starmap.insert ( { hip, star } );
/*
		cout <<
		
		strHIP << "," <<
		strRA << "," <<
		strDec << "," <<
		strPMRA << "," <<
		strPMDec << "," <<
		strMag << "," <<
		strBmV << "," <<
		strPlx << "," <<
		strRV << "," <<
		strSpec << "," <<
		strHD << "," <<
		strSAO << "," <<
		strBD << "," <<
		strCD << "," <<
		strCP << "," <<
		strVar << "," <<
		strVarType << "," <<
		strVarPer << "," <<
		strVarMax << "," <<
		strVarMin << "," <<

		endl;
*/
        
        
	}
    
    if ( linecount == starmap.size() )
        cout << "Success: " << filename << " linecount " << linecount << " == starmap.size() " << starmap.size() << endl;
    else
        cout << "Failure: " << filename << " linecount " << linecount << " != starmap.size() " << starmap.size() << endl;

    return starmap;
}

void importHIP ( const char *hip_main_filename, HIPMap mapHIPtoHR, HIPMap mapHIPtoBF, HIPMap mapHIPtoVar )
{
	ifstream hip_main_file ( hip_main_filename );
	
	if ( ! hip_main_file )
		return;
	
	string line;
	while ( getline ( hip_main_file, line ) )
	{
		string strHIP = line.substr ( 8, 6 );
		string strRA = line.substr ( 51, 12 );
		string strDec = line.substr ( 64, 12 );
		string strPMRA = line.substr ( 87, 8 );
		string strPMDec = line.substr ( 96, 8 );
		string strMag = line.substr ( 41, 5 );
		string strBmV = line.substr ( 245, 6 );
		string strPlx = line.substr ( 79, 7 );
		string strSpec = line.substr ( 435, 12 );
		string strHD = line.substr ( 390, 6 );
		string strBD = line.substr ( 398, 9 );
		string strCD = line.substr ( 409, 9 );
		string strCP = line.substr ( 420, 9 );

		int hip = stoi ( strHIP );
		
		cout <<
		
		strHIP << "," <<
		strRA << "," <<
		strDec << "," <<
		strPMRA << "," <<
		strPMDec << "," <<
		strMag << "," <<
		strBmV << "," <<
		strPlx << "," <<
		strSpec << "," <<
		strHD << "," <<
		strBD << "," <<
		strCD << "," <<
		strCP << ",";

		auto rangeHR = mapHIPtoHR.equal_range ( hip );
		for ( auto i = rangeHR.first; i != rangeHR.second; i++ )
			cout << i->second << ",";

		auto rangeBF = mapHIPtoBF.equal_range ( hip );
		for ( auto i = rangeBF.first; i != rangeBF.second; i++ )
			cout << i->second << ",";
		
		auto rangeVar = mapHIPtoVar.equal_range ( hip );
		for ( auto i = rangeVar.first; i != rangeVar.second; i++ )
			cout << i->second << ",";

		cout <<	endl;
	}
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
		string strHR = line.substr ( 0, 6 );
		string strHIP = line.substr ( 7, 6 );
		int hip = stoi ( strHIP );
		// cout << strHIP << "," << strHR << "," << endl;
		mapHIPtoHR.insert ( { hip, strHR } );
		linecount++;
	}
	
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
		string strBF = line.substr ( 0, 11 );
		string strHIP = line.substr ( 12, 6 );
		int hip = stoi ( strHIP );

		// cout << strHIP << "," << strHR << "," << endl;
		mapHIPtoBF.insert ( { hip, strBF } );
		linecount++;
	}

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
		string strVar = line.substr ( 0, 11 );
		string strHIP = line.substr ( 12, 6 );
		int hip = stoi ( strHIP );

		// cout << strHIP << "," << strHR << "," << endl;
		mapHIPtoVar.insert ( { hip, strVar } );
		linecount++;
	}

	if ( linecount == mapHIPtoVar.size() )
		cout << "Success: " << filename << " linecount " << linecount << " == mapHIPtoVar.size() " << mapHIPtoVar.size() << endl;
	else
		cout << "Failure: " << filename << " linecount " << linecount << " != mapHIPtoVar.size() " << mapHIPtoVar.size() << endl;
	
	return mapHIPtoVar;
}

void importHIP2 ( const char *hip2_filename )
{
	ifstream hip2_file ( hip2_filename );
	
	if ( ! hip2_file )
		return;
	
	string line;
	while ( getline ( hip2_file, line ) )
	{
		string strHIP = line.substr ( 0, 6 );
		string strRA = line.substr ( 15, 13 );
		string strDec = line.substr ( 29, 13 );
		string strPMRA = line.substr ( 51, 8 );
		string strPMDec = line.substr ( 60, 8 );
		string strMag = line.substr ( 129, 7 );
		string strBmV = line.substr ( 152, 6 );
		string strPlx = line.substr ( 43, 7 );

		cout <<
		
		strHIP << "," <<
		strRA << "," <<
		strDec << "," <<
		strPMRA << "," <<
		strPMDec << "," <<
		strMag << "," <<
		strBmV << "," <<
		strPlx << "," <<

		endl;
	}
}
