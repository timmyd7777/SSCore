// SSImportMPC.cpp
// SSCore
//
// Created by Tim DeBenedictis on 3/24/20.
// Copyright © 2020 Southern Stars. All rights reserved.

#include <iostream>
#include <fstream>

#include "SSTime.hpp"
#include "SSImportMPC.hpp"

// Reads comet data from a Minor Planet Center comet orbit export file:
// https://www.minorplanetcenter.net/iau/MPCORB/CometEls.txt
// Imported data is appended to the input vector of SSObjects (comets).
// Returns number of comets successfully imported.

int SSImportMPCComets ( const string &filename, SSObjectVec &comets )
{
    // Open file; return on failure.

    ifstream file ( filename );
    if ( ! file )
        return ( 0 );

    // Read file line-by-line until we reach end-of-file

    string line = "";
    int numLines = 0, numComets = 0;

    while ( getline ( file, line ) )
    {
        numLines++;
        if ( line.length() < 160 )
            continue;
        
        // col 1-4: periodic or interstellar comet number, denoted with 'P' or 'I' in column 5.
        // col 6-12: provisional designation. Both currently unused.

        string field = line.substr ( 0, 4 );
        field = line.substr ( 5, 7 );
        
        // col 15-18: year/month/day of perihelion passage (TDT)

        int year = strtoint ( line.substr ( 14, 4 ) );
        int month = strtoint ( line.substr ( 19, 2 ) );
        double day = strtofloat64 ( line.substr ( 22, 7 ) );
        double peridate = year && month && day ? SSTime ( SSDate ( kGregorian, 0.0, year, month, day, 0, 0, 0 ) ).jd : 0.0;
        if ( peridate == 0.0 )
            continue;
                
        // col 31-39: perihelion distance (AU)
        
        field = trim ( line.substr ( 30, 9 ) );
        double q = field.empty() ? INFINITY : strtofloat64 ( field );
        
        // col 42-49: orbital eccentricity
        
        field = trim ( line.substr ( 41, 8 ) );
        double e = field.empty() ? INFINITY : strtofloat64 ( field );
        
        // col 52-59: argument of perihelion, J2000.0 (degrees)
        
        field = trim ( line.substr ( 51, 8 ) );
        double w = field.empty() ? INFINITY : degtorad ( strtofloat64 ( field ) );
        
        // col 62-69: longitude of ascending node, J2000.0 (degrees)
        
        field = line.substr ( 61, 8 );
        double n = field.empty() ? INFINITY : degtorad ( strtofloat64 ( field ) );
        
        // col 72-79: inclination, J2000.0 (degrees)
        
        field = line.substr ( 71, 8 );
        double i = field.empty() ? INFINITY : degtorad ( strtofloat64 ( field ) );
        
        // col 82-85: epoch for perturbed solution - may be blank

        year = strtoint ( line.substr ( 81, 4 ) );
        month = strtoint ( line.substr ( 85, 2 ) );
        day = strtofloat64 ( line.substr ( 87, 2 ) );
        double epoch = year && month && day ? SSTime ( SSDate ( kGregorian, 0.0, year, month, day, 0, 0, 0 ) ).jd : 0.0;
        
        // col 92-95: absolute magnitude
        
        field = trim ( line.substr ( 91, 4 ) );
        float hmag = field.empty() ? INFINITY : strtofloat ( field );
        
        // col 97-100: magnitude slope parameter
        
        field = trim ( line.substr ( 96, 5 ) );
        float gmag = field.empty() ? INFINITY : strtofloat ( field );

        // col 103 - 159: name including provisional desingation and/or periodic comet number
        
        vector<string> names;
        field = trim ( line.substr ( 102, 56 ) );
        SSIdentifier number = SSIdentifier::fromString ( field );
        
        // for numbered periodic comets, extract name following slash.
        
        if ( number )
        {
            size_t pos = field.find ( "P/" );
            string name = field.substr ( 0, pos + 1 );
            if ( ! name.empty() )
                names.push_back ( name );

            name = field.substr ( pos + 2, string::npos );
            if ( ! name.empty() )
                names.push_back ( name );
        }
        else
        {
            // extract name in parantheses (if any), preceded by provisional designation
            
            size_t pos1 = field.find ( "(" );
            size_t pos2 = field.find ( ")" );
            
            if ( pos1 == string::npos || pos2 == string::npos )
            {
                names.push_back ( field );
            }
            else
            {
                string name1 = trim ( field.substr ( 0, pos1 - 1 ) );
                string name2 = trim ( field.substr ( pos1 + 1, pos2 - pos1 - 1 ) );
                
                if ( ! name1.empty() )
                    names.push_back ( name1 );
                
                if ( ! name2.empty() )
                    names.push_back ( name2 );
            }
        }

        // Allocate new comet object with default values
        
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
            pComet->setIdentifier ( number );

        pComet->setNames ( names );
        pComet->setOrbit ( orbit );
        pComet->setHMagnitude ( hmag );
        pComet->setGMagnitude ( gmag );
        
        // cout << pComet->toCSV() << endl;
        comets.push_back ( pComet );
        numComets++;
    }
    
    return numComets;
}

// Read asteroid data from a Minor Planet Center asteroid orbit export file:
// https://www.minorplanetcenter.net/iau/MPCORB/MPCORB.DAT
// Imported data is appended to the input vector of SSObjects (asteroids).
// Returns number of asteroids successfully imported.

int SSImportMPCAsteroids ( const string &filename, SSObjectVec &asteroids )
{
    // Open file; return on failure.

    ifstream file ( filename );
    if ( ! file )
        return ( 0 );

    // Read file line-by-line until we reach end-of-file

    string line = "";
    int numLines = 0, numAsteroids = 0;

    while ( getline ( file, line ) )
    {
        numLines++;
        if ( line.length() < 195 )
            continue;

        // col 9-13: absolute magnitude
        
        string field = trim ( line.substr ( 8, 5 ) );
        float hmag = field.empty() ? INFINITY : strtofloat ( field );
        
        // col 15-19: magnitude slope parameter
        
        field = trim ( line.substr ( 14, 5 ) );
        float gmag = field.empty() ? INFINITY : strtofloat ( field );
        
        // col 21-25: epoch in packed form
        
        field = line.substr ( 20, 5 );
        int year = 100 * ( 20 + toupper ( field[0] ) - 'K' );  // century
        year += strtoint ( field.substr ( 1, 2 ) );
        
        int month = 0;
        if ( field[3] >= '1' && field[3] <= '9' )
            month = 1 + field[3] - '1';
        else if ( toupper( field[3] ) >= 'A' && toupper( field[3] ) <= 'C' )
            month = 10 + toupper ( field[3] ) - 'A';
        else
            continue;
        
        double day = 0.0;
        if ( field[4] >= '1' && field[4] <= '9' )
            day = 1 + field[4] - '1';
        else if ( toupper ( field[4] ) >= 'A' && toupper ( field[4] ) <= 'V' )
            day = 10 + toupper ( field[4] ) - 'A';
        else
            continue;
        
        double epoch = year && month && day ? SSTime ( SSDate ( kGregorian, 0.0, year, month, day, 0, 0, 0 ) ).jd : 0.0;
        
        // col 27-35: Mean anomaly in degrees
        
        field = trim ( line.substr ( 26, 9 ) );
        double m = field.empty() ? INFINITY : degtorad ( strtofloat64 ( field ) );
        
        // col 38-46: Argument of perihelion in degrees
        
        field = trim ( line.substr ( 37, 9 ) );
        double w = field.empty() ? INFINITY : degtorad ( strtofloat64 ( field ) );
        
        // col 49-57: Longitude of ascending node in degrees
        
        field = trim ( line.substr ( 48, 9 ) );
        double n = field.empty() ? INFINITY : degtorad ( strtofloat64 ( field ) );
        
        // col 60-68: Inclination in degrees
        
        field = trim ( line.substr ( 59, 9 ) );
        double i = field.empty() ? INFINITY : degtorad ( strtofloat64 ( field ) );
        
        // col 71-79: Eccentricity
        
        field = trim ( line.substr ( 70, 9 ) );
        double e = field.empty() ? INFINITY : strtofloat64 ( field );
        
        // col 81-91: Mean motion in degrees per day
        
        field = trim ( line.substr ( 80, 11 ) );
        double mm = field.empty() ? INFINITY : degtorad ( strtofloat64 ( field ) );
        
        // col 93-103: Semimajor axis in AU.  If not found, compute from mean motion.
        
        field = trim ( line.substr ( 92, 11 ) );
        double a = strtofloat64 ( field );
        if ( a <= 0.0 )
            a = pow ( SSOrbit::kGaussGravHelio / ( mm * mm ), 1.0 / 3.0 );
        
        // col 167-254: asteroid number (may be blank)
        
        field = trim ( line.substr ( 166, 8 ) );
        SSIdentifier number = SSIdentifier::fromString ( field );
        
        // col 167-254: Name or provisional designation

        vector<string> names;
        field = trim ( line.substr ( 175, 19 ) );
        if ( ! field.empty() )
            names.push_back ( field );
        
        // Allocate new asteroid object with default values
        
        SSPlanetPtr pAsteroid = new SSPlanet ( kTypeAsteroid );
        if ( pAsteroid == nullptr )
            continue;
        
        SSOrbit orbit ( epoch, a * ( 1.0 - e ), e, i, w, n, m, mm );

        if ( number )
            pAsteroid->setIdentifier ( number );
        
        pAsteroid->setNames ( names );
        pAsteroid->setOrbit ( orbit );
        pAsteroid->setHMagnitude ( hmag );
        pAsteroid->setGMagnitude ( gmag );

        // cout << pAsteroid->toCSV() << endl;
        asteroids.push_back ( pAsteroid );
        numAsteroids++;
    }

    return numAsteroids;
}
