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
// Returned comet data is appended to the input vector of SSObjects (comets).

void importMPCComets ( const char *filename, SSObjectVec &comets )
{
    // Open file; report error and return empty map on failure.

    ifstream file ( filename );
    if ( ! file )
    {
        cout << "Failure: can't open " << filename << endl;
        return;
    }

    // Read file line-by-line until we reach end-of-file

    string line = "";
    int linecount = 0;

    while ( getline ( file, line ) )
    {
        linecount++;
        if ( line.length() < 160 )
            continue;
		
		//  col 1-4 : periodic comet number
		string field = line.substr ( 0, 4 );
		int number = strtoint ( field );
		
		// col 6-12 : provisional designation
		field = line.substr ( 5, 7 );
		
		// col 15-18 : year of perihelion passage
		field = line.substr ( 14, 4 );
		int year = strtoint ( field );
		if ( year < 1 )
			continue;
		
		// col 20-21 : month of perihelion passage
		field = line.substr ( 19, 2 );
		int month = strtoint ( field );
		if ( month < 1 || month > 12 )
			continue;
		
		// col 23-29 : day of perihelion passage
		field = line.substr ( 22, 7 );
		double day = strtofloat64 ( field );
		if ( day < 1.0 || day >= 32.0 )
			continue;
		
		// compute perihelion date as JD
		double peridate = SSTime ( SSDate ( kGregorian, 0.0, year, month, day, 0, 0, 0 ) );
		
		// col 31-39 : perihelion distance (AU)
		field = line.substr ( 30, 9 );
		double q = strtofloat64 ( field );
		if ( q < 0.0 )
			continue;
		
		// col 42-49 : orbital eccentricity
		field = line.substr ( 41, 8 );
		double e = strtofloat64 ( field );
		if ( e < 0.0 || e > 2.0 )
			continue;
		
		// col 52-59 : argument of perihelion, J2000.0 (degrees)
		field = line.substr ( 51, 8 );
		double w = strtofloat64 ( field );
		if ( w < 0.0 || w > 360.0 )
			continue;
		
		// col 62-69 : longitude of ascending node, J2000.0 (degrees)
		field = line.substr ( 61, 8 );
		double n = strtofloat64 ( field );
		if ( n < 0.0 || n > 360.0 )
			continue;
		
		// col 72-79 : inclination, J2000.0 (degrees)
		field = line.substr ( 71, 8 );
		double i = strtofloat64 ( field );
		if ( i < 0.0 || i > 180.0 )
			continue;
		
		// col 82-85 : epoch for perturbed solution - may be blank
		field = line.substr ( 81, 8 );
		year = strtoint ( line.substr ( 81, 4 ) );
		month = strtoint ( line.substr ( 85, 2 ) );
		month = strtofloat64 ( line.substr ( 87, 2 ) );
		double epoch = year && month && day ? SSTime ( SSDate ( kGregorian, 0.0, year, month, day, 0, 0, 0 ) ).jd : 0.0;
		
		// col 92-95 : absolute magnitude
		field = trim ( line.substr ( 91, 4 ) );
		float hmag = field.empty() ? HUGE_VAL : strtofloat ( field );
		
		// col 97-100 : magnitude slope parameter
		field = trim ( line.substr ( 96, 5 ) );
		float gmag = field.empty() ? HUGE_VAL : strtofloat ( field );

		// col 103 - 159 : name
		field = trim ( line.substr ( 102, 56 ) );
/*
		mdm = AAMeanMotion ( HELIO_GAUSS_CONST * HELIO_GAUSS_CONST, q, e );
		
		comet->number = number;
		comet->t = epoch == 0.0 ? peridate : epoch;
		comet->q = q;
		comet->e = e;
		comet->m = RAD_TO_DEG ( epoch == 0.0 ? 0.0 : mdm * ( epoch - peridate ) );
		comet->i = i;
		comet->w = w;
		comet->n = n;
		comet->h = hmag;
		comet->g = gmag;
		
		return ( TRUE );
*/
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
		
		pComet->setOrbit ( orbit );
		pComet->setHMagnitude ( hmag );
		pComet->setGMagnitude ( gmag );
		
		comets.push_back ( shared_ptr<SSObject> ( pComet ) );
	}
}

// Read asteroid data from a Minor Planet Center asteroid orbit export file:
// https://minorplanetcenter.net/iau/Ephemerides/Bright/2018/Soft00Bright.txt (bright asteroids at opposition in 2018)
// https://minorplanetcenter.net/iau/Ephemerides/Bright/2018/Soft00Bright.txt (bright asteroids at opposition in 2018)
// https://minorplanetcenter.net/iau/Ephemerides/Unusual/Soft00Unusual.txt (unusual objects including NEOs)
// https://minorplanetcenter.net/iau/Ephemerides/Distant/Soft00Distant.txt (distant objects including KBOs)
// Returned comet data is appended to the input vector of SSObjects (asteroids).

void importMPCAsteroids ( const char *filename, SSObjectVec &asteroids )
{
	
}
