// SSMPC.cpp
// SSCore
//
// Created by Tim DeBenedictis on 3/24/20.
// Copyright © 2020 Southern Stars. All rights reserved.

#include <iostream>
#include <fstream>

#include "SSTime.hpp"
#include "SSMPC.hpp"

// Reads comet data from a Minor Planet Center comet orbit export file:
// https://www.minorplanetcenter.net/iau/MPCORB/CometEls.txt
// Returns number of comets successfully imported. Imported comet data
// is appended to the input vector of SSObjects (comets).

int importMPCComets ( const char *filename, SSObjectVec &comets )
{
    // Open file; report error and return empty map on failure.

    ifstream file ( filename );
    if ( ! file )
    {
        cout << "Failure: can't open " << filename << endl;
        return ( 0 );
    }

    // Read file line-by-line until we reach end-of-file

    string line = "";
    int linecount = 0, numcomets = 0;

    while ( getline ( file, line ) )
    {
        linecount++;
        if ( line.length() < 160 )
            continue;
		
		// col 1-4: periodic or interstellar comet number,
		// denoted with 'P' or 'I' in column 5.
		
		string field = line.substr ( 0, 4 );
		int number = line[5] == 'P' ? strtoint ( field ) : 0;
		
		// col 6-12: provisional designation
		
		field = line.substr ( 5, 7 );
		
		// col 15-18: year/month/day of perihelion passage (TDT)

		int year = strtoint ( line.substr ( 14, 4 ) );
		int month = strtoint ( line.substr ( 19, 2 ) );
		double day = strtofloat64 ( line.substr ( 22, 7 ) );
		double peridate = year && month && day ? SSTime ( SSDate ( kGregorian, 0.0, year, month, day, 0, 0, 0 ) ).jd : 0.0;
		if ( peridate == 0.0 )
			continue;
				
		// col 31-39: perihelion distance (AU)
		
		field = line.substr ( 30, 9 );
		double q = strtofloat64 ( field );
		if ( q <= 0.0 )
			continue;
		
		// col 42-49: orbital eccentricity
		
		field = line.substr ( 41, 8 );
		double e = strtofloat64 ( field );
		if ( e <= 0.0 )
			continue;
		
		// col 52-59: argument of perihelion, J2000.0 (degrees)
		
		field = line.substr ( 51, 8 );
		double w = SSAngle::fromDegrees ( strtofloat64 ( field ) );
		if ( w <= 0.0 || w > SSAngle::kTwoPi )
			continue;
		
		// col 62-69: longitude of ascending node, J2000.0 (degrees)
		
		field = line.substr ( 61, 8 );
		double n = SSAngle::fromDegrees ( strtofloat64 ( field ) );
		if ( n <= 0.0 || n > SSAngle::kTwoPi )
			continue;
		
		// col 72-79: inclination, J2000.0 (degrees)
		
		field = line.substr ( 71, 8 );
		double i = SSAngle::fromDegrees ( strtofloat64 ( field ) );
		if ( i <= 0.0 || i > SSAngle::kPi )
			continue;
		
		// col 82-85: epoch for perturbed solution - may be blank

		year = strtoint ( line.substr ( 81, 4 ) );
		month = strtoint ( line.substr ( 85, 2 ) );
		day = strtofloat64 ( line.substr ( 87, 2 ) );
		double epoch = year && month && day ? SSTime ( SSDate ( kGregorian, 0.0, year, month, day, 0, 0, 0 ) ).jd : 0.0;
		
		// col 92-95: absolute magnitude
		
		field = trim ( line.substr ( 91, 4 ) );
		float hmag = field.empty() ? HUGE_VAL : strtofloat ( field );
		
		// col 97-100: magnitude slope parameter
		
		field = trim ( line.substr ( 96, 5 ) );
		float gmag = field.empty() ? HUGE_VAL : strtofloat ( field );

		// col 103 - 159: name
		
		vector<string> names;
		field = trim ( line.substr ( 102, 56 ) );
		if ( ! field.empty() )
			names.push_back ( field );

		SSPlanetPtr pComet = new SSPlanet ( kTypeComet );
		if ( pComet == nullptr )
			continue;
		
		// Compute mean motion from semimajor axis.
		// If we have an epoch, compute mean anomaly at epoch.
		// Otherwise, use perihelion date as epoch and set mean anomaly to zero.
		
		double mdm = SSOrbit::meanMotion ( e, q );
		double t = epoch == 0.0 ? peridate : epoch;
		double m = epoch == 0.0 ? 0.0 : mdm * ( epoch - peridate );
		SSOrbit orbit ( t, q, e, i, w, n, m, mdm );
		
		if ( number )
			pComet->setIdentifier ( SSIdentifier ( kCatComNum, number ) );

		pComet->setNames ( names );
		pComet->setOrbit ( orbit );
		pComet->setHMagnitude ( hmag );
		pComet->setGMagnitude ( gmag );
		
		cout << pComet->toCSV() << endl;
		comets.push_back ( shared_ptr<SSObject> ( pComet ) );
		numcomets++;
	}
	
	return numcomets;
}

// Read asteroid data from a Minor Planet Center asteroid orbit export file:
// https://minorplanetcenter.net/iau/Ephemerides/Bright/2018/Soft00Bright.txt (bright asteroids at opposition in 2018)
// https://minorplanetcenter.net/iau/Ephemerides/Bright/2018/Soft00Bright.txt (bright asteroids at opposition in 2018)
// https://minorplanetcenter.net/iau/Ephemerides/Unusual/Soft00Unusual.txt (unusual objects including NEOs)
// https://minorplanetcenter.net/iau/Ephemerides/Distant/Soft00Distant.txt (distant objects including KBOs)
// Returned comet data is appended to the input vector of SSObjects (asteroids).

int importMPCAsteroids ( const char *filename, SSObjectVec &asteroids )
{
	return 0;
}
