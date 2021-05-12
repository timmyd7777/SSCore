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
// Expected CSV format for asteroids is:
// full_name,equinox,a,e,i,w,om,ma,epoch,H,G,diameter,GM,rot_per
// Expected CSV format for comets is:
// full_name,equinox,q,e,i,w,om,tp,epoch,M1,K1,diameter,GM,rot_per
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
    orbit.m = strtofloat64 ( fields[7] );
    orbit.t = strtofloat64 ( fields[8] );

    // reject invalid orbits
    
    if ( orbit.q <= 0.0 || orbit.t <= 0.0 )
        return nullptr;
    
    // For asteroids, compute perihelion from semimajor axis and eccentricity
    
    if ( type == kTypeAsteroid )
        orbit.q = orbit.q * ( 1.0 - orbit.e );
    
    // Compute mean motion
    
    orbit.mm = SSOrbit::meanMotion ( orbit.e, orbit.q );
    
    // For asteroids, convert mean anomaly to radians.
    // For comets, compute mean anomaly at epoch from perihelion date and mean motion.
    
    if ( type == kTypeAsteroid )
        orbit.m = degtorad ( orbit.m );
    else
        orbit.m = orbit.mm * ( orbit.t - orbit.m );
    
    // Get magnitude parameters, diameter in km, mass in kg, rotation period in days (currenntly ignored)
    
    float h = fields[9].empty() ? INFINITY : strtofloat ( fields[9] );
    float g = fields[10].empty() ? INFINITY : strtofloat ( fields[10] );
    float d = fields[11].empty() ? INFINITY : strtofloat ( fields[11] );
    float m = fields[12].empty() ? INFINITY : strtofloat ( fields[12] ) / SSOrbit::kGravity;
    float p = fields[13].empty() ? 0.0 : strtofloat ( fields[13] ) / 24.0; p = p;   // supress unused variable warning

    // parse number, name, designation
    
    int number = 0;
    vector<string> names;
    string name = "", desig = "";
    if ( type == kTypeAsteroid )
    {
        number = strtoint ( fields[0] );
        if ( number > 0 )
        {
            size_t p0 = fields[0].find ( ' ' );
            size_t p1 = fields[0].find ( '(' );
            size_t p2 = fields[0].find ( ')' );
            if ( p1 - p0 > 2 )
                names.push_back ( fields[0].substr ( p0 + 1, p1 - p0 - 2 ) );   // name
            if ( p2 - p1 > 1 )
                names.push_back ( fields[0].substr ( p1 + 1, p2 - p1 - 1 ) );   // designation
        }
        else
        {
            size_t p1 = fields[0].find ( '(' );
            size_t p2 = fields[0].find ( ')' );
            if ( p2 - p1 > 1 )
                names.push_back ( fields[0].substr ( p1 + 1, p2 - p1 - 1 ) );   // designation
        }
    }
    else
    {
        number = strtoint ( fields[0] );
        if ( number > 0 )
        {
            size_t p0 = fields[0].find ( '/' );
            names.push_back ( fields[0].substr ( 0, p0 ) );                     // designation
            names.push_back ( fields[0].substr ( p0 + 1, string::npos ) );      // name
        }
        else
        {
            size_t p1 = fields[0].find ( '(' );
            size_t p2 = fields[0].find ( ')' );
            names.push_back ( fields[0].substr ( 0, p1 - 1 ) );                 // designation
            names.push_back ( fields[0].substr ( p1 + 1, p2 - p1 - 1 ) );       // name
        }
    }
    
    // Allocate new object; return nullptr on failure
    
    SSPlanetPtr pAstCom = new SSPlanet ( type );
    if ( pAstCom == nullptr )
        return nullptr;
    
    pAstCom->setNames ( names );
    if ( number )
        pAstCom->setIdentifier ( SSIdentifier ( type == kTypeAsteroid ? kCatAstNum : kCatComNum, number ) );

    pAstCom->setOrbit ( orbit );
    pAstCom->setHMagnitude ( h );
    pAstCom->setGMagnitude ( g );
    pAstCom->setRadius ( d / 2.0 );
    pAstCom->setMass ( m / SSCoordinates::kKgPerEarthMass );

    // cout << pAstCom->toCSV() << endl;
    return pAstCom;
}

// Read asteroid or comet data from a JPL DASTCOM export file in CSV format for objects
// of kTypeAsteroid or kTypeComet (type) from https://ssd.jpl.nasa.gov/sbdb_query.cgi
// Imported data is appended to the input vector of SSObjects (objects).
// If a non-null filter function (filter) is provided, objects are imported
// only if they pass the filter; optional data pointer (userData) is passed
// to the filter but not used otherwise.
// Returns number of objects successfully imported.

int SSImportJPLDASTCOM ( const string &filename, SSObjectType type, SSObjectVec &objects, SSObjectFilter filter, void *userData )
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
            if ( filter == nullptr || filter ( pAstCom, userData ) )
            {
                objects.append ( pAstCom );
                numAstCom++;
            }
        }
    }
    
    return numAstCom;
}
