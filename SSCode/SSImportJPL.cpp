//
//  SSImportJPL.cpp
//  Nanoverse
//
//  Created by Tim DeBenedictis on 5/11/21.
//  Copyright © 2021 Southern Stars. All rights reserved.
//

#include "SSImportJPL.hpp"

// Converts one line of a JPL DASTCOM export CSV file to an SSPlanet of kTypeAsteroid
// or kTypeComet (type); all other object types fail to create an SSPlanet.
// Returns pointer to newly-allocated SSPlanet if successful or nullptr on failure.

SSPlanetPtr SSImportJPLAstCom ( const string &line, SSObjectType type )
{
    if ( type < kTypeAsteroid || type > kTypeComet )
        return nullptr;
    
    // split string into comma-delimited fields; require at least 14

    vector<string> fields = split_csv ( line );
    if ( fields.size() < 14 )
        return nullptr;
    
    // remove leading & trailing whitespace/line breaks from each field.

    for ( int i = 0; i < fields.size(); i++ )
        fields[i] = trim ( fields[i] );

    SSOrbit orbit;
    
    orbit.e = strtofloat64 ( fields[3] );
    orbit.q = strtofloat64 ( fields[2] );
    orbit.i = degtorad ( strtofloat64 ( fields[4] ) );
    orbit.w = degtorad ( strtofloat64 ( fields[5] ) );
    orbit.n = degtorad ( strtofloat64 ( fields[6] ) );
    orbit.m = degtorad ( strtofloat64 ( fields[7] ) );
    orbit.t = strtofloat64 ( fields[8] );

    // For asteroids, compute perihelion from semimajor axis and eccentricity
    
    if ( type == kTypeAsteroid )
        orbit.q = orbit.q * ( 1.0 - orbit.e );
    
    // Compute mean motion
    
    orbit.mm = SSOrbit::meanMotion ( orbit.e, orbit.q );
    
    // For comets, compute mean anomaly at epoch from perihelion date and mean motion
    
    if ( type == kTypeComet )
        orbit.m = orbit.mm * ( orbit.m - orbit.t );
    
    // Get magnitude parameters, diameter in km, mass in kg, rotation period in days (currenntly ignored)
    
    float h = fields[9].empty() ? INFINITY : strtofloat ( fields[9] );
    float g = fields[10].empty() ? INFINITY : strtofloat ( fields[10] );
    float d = fields[11].empty() ? 0.0 : strtofloat ( fields[11] );
    float m = fields[12].empty() ? 0.0 : strtofloat ( fields[12] ) / SSOrbit::kGravity;
//    float p = fields[13].empty() ? 0.0 : strtofloat ( fields[13] ) / 24.0;

    // Allocate new object; return nullptr on failure
    
    SSPlanetPtr pAstCom = new SSPlanet ( type );
    if ( pAstCom == nullptr )
        return nullptr;
    
//    if ( number )
//        pAsteroid->setIdentifier ( number );
//    pAstCom->setNames ( names );
    
    pAstCom->setOrbit ( orbit );
    pAstCom->setHMagnitude ( h );
    pAstCom->setGMagnitude ( g );
    pAstCom->setRadius ( d / 2.0 );
    pAstCom->setMass ( m / SSCoordinates::kKgPerEarthMass );

    return pAstCom;
}

// Read asteroid or comet data from a JPL DASTCOM export file in CSV format for objects
// of kTypeAsteroid or kTypeComet (type) from https://ssd.jpl.nasa.gov/sbdb_query.cgi
// Imported data is appended to the input vector of SSObjects (objects).
// Returns number of objects successfully imported.

int SSImportJPLDASTCOM ( const string &filename, SSObjectType type, SSObjectVec &objects )
{
    if ( type < kTypeAsteroid || type > kTypeComet )
        return 0;
    
    // Open file; return on failure.

    ifstream file ( filename );
    if ( ! file )
        return ( 0 );

    // Read file line-by-line until we reach end-of-file

    string line = "";
    int numLines = 0, numAstCom = 0;
    while ( getline ( file, line ) )
    {
        numLines++;
        SSPlanetPtr pAstCom = SSImportJPLAstCom ( line, type );
        if ( pAstCom )
        {
            objects.append ( pAstCom );
            numAstCom++;
        }
    }
    
    return numAstCom;
}
