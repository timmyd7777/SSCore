//
//  SSImportJPL.cpp
//  Nanoverse
//
//  Created by Tim DeBenedictis on 5/11/21.
//  Copyright © 2021 Southern Stars. All rights reserved.
//

#include <unordered_set>

#include "SSImportJPL.hpp"
#include "SSImportMPC.hpp"

// Converts one line of a JPL DASTCOM export CSV file to an SSPlanet of kTypeAsteroid
// or kTypeComet (type); all other object types fail to create an SSPlanet.
// Expected CSV format for asteroids is:
// full_name,equinox,a,e,i,w,om,ma,epoch,H,G,diameter,GM,rot_per,albedo,BV,spec_T
// Expected CSV format for comets is:
// full_name,equinox,q,e,i,w,om,tp,epoch,M1,K1,diameter,GM,rot_per,albedo,BV,spec_T
// Returns pointer to newly-allocated SSPlanet if successful or nullptr on failure.

SSPlanetPtr SSImportJPLAstCom ( const string &line, SSObjectType type )
{
    if ( type < kTypeAsteroid || type > kTypeComet )
        return nullptr;
    
    // split string into comma-delimited fields; require at least 14

    vector<string> fields = split_csv ( line );
    if ( fields.size() < 17 )
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

    // For asteroids, compute perihelion from semimajor axis and eccentricity
    
    if ( type == kTypeAsteroid )
        orbit.q = orbit.q * ( 1.0 - orbit.e );
    
    // reject invalid orbits
    
    if ( orbit.q <= 0.0 || orbit.t <= 0.0 )
        return nullptr;
    
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
    float p = fields[13].empty() ? INFINITY : strtofloat ( fields[13] ) / 24.0;
    float a = fields[14].empty() ? INFINITY : strtofloat ( fields[14] );
    float c = fields[15].empty() ? INFINITY : strtofloat ( fields[15] );

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
            if ( p1 != string::npos )
            {
                string desig = p2 != string::npos ? trim ( fields[0].substr ( p1 + 1, p2 - p1 - 1 ) ) : "";
                if ( ! desig.empty() )
                    names.push_back ( desig );
                
                string name = trim ( fields[0].substr ( 0, p1 ) );
                if ( ! name.empty() )
                    names.push_back ( name );
            }
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
    pAstCom->setGMagnitude ( type == kTypeComet ? g / 2.5 : g );
    pAstCom->setColorIndex ( c );
    pAstCom->setRadius ( d / 2.0 );
    pAstCom->setMass ( m / SSCoordinates::kKgPerEarthMass );
    pAstCom->setRotationPeriod ( p );
    pAstCom->setAlbedo ( a );
    pAstCom->setTaxonomy ( fields[16] );

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
        else
        {
            // cerr << line << endl;
        }
    }
    
    return numAstCom;
}

// Comparison function for soring imported comets.
// Numbered periodic comets and their fragments are sorted first by periodic comet number, then by designation.
// Unnumbered comets follow, sorted by designation, and numbered interstellar comets are sorted last by designation.
// Returns true if the first comet should precede the second.

static bool CometCmp ( const SSObjectPtr &pObj1, const SSObjectPtr &pObj2 )
{
    SSPlanetPtr p1 = SSGetPlanetPtr ( pObj1 );
    SSPlanetPtr p2 = SSGetPlanetPtr ( pObj2 );

    if ( p1 == nullptr || p2 == nullptr )
        return p1 != nullptr;

    string d1 = p1->getName ( 0 );
    string d2 = p2->getName ( 0 );

    bool i1 = d1.length() > 1 && d1.back() == 'I' && strtoint ( d1 ) > 0;
    bool i2 = d2.length() > 1 && d2.back() == 'I' && strtoint ( d2 ) > 0;

    SSIdentifier id1 = p1->getIdentifier ( kCatComNum );
    SSIdentifier id2 = p2->getIdentifier ( kCatComNum );

    bool p1num = id1.identifier() > 0;
    bool p2num = id2.identifier() > 0;

    // Numbered periodic comets and fragments first.

    if ( p1num != p2num )
        return p1num;

    if ( p1num )
    {
        if ( id1.identifier() != id2.identifier() )
            return id1.identifier() < id2.identifier();

        return d1 < d2;
    }

    // Interstellar objects last.

    if ( i1 != i2 )
        return ! i1;

    // Unnumbered comets alphabetically; interstellar numerically/alphabetically
    // also comes out correctly as 1I, 2I, 3I.

    return d1 < d2;
}

// Imports comet orbital data from both JPL DASTCOM and the Minor Planet Center comet database.
// JPL data is preferred; MPC comets are appended only if not present in the JPL data.
// Returns total number of comets successfully appended to output object vector (comets).

int SSImportComets ( const string &jplFilename, const string &mpcFilename, SSObjectVec &comets )
{
    // Import JPL comets first.
    
    int start = (int) comets.size();
    int n = SSImportJPLDASTCOM ( jplFilename, kTypeComet, comets );
    if ( n < 1 )
        return 0;
    
    // Build set of all comet designations found in JPL.

    unordered_set<string> designations;
    for ( int i = start; i < comets.size(); i++ )
    {
        SSPlanetPtr pComet = SSGetPlanetPtr ( comets[i] );
        if ( pComet )
            designations.insert ( pComet->getName ( 0 ) );
    }

    // Append MPC comets not found in JPL designations.

    ifstream file ( mpcFilename );
    if ( ! file )
        return (int) comets.size() - start;

    string line;
    while ( getline ( file, line ) )
    {
        SSPlanetPtr pComet = SSImportMPCComet ( line );
        if ( pComet == nullptr )
            continue;

        string desig = pComet->getName ( 0 );
        if ( ! desig.empty() && designations.find ( desig ) == designations.end() )
        {
            comets.append ( pComet );
            designations.insert ( desig );
        }
        else
        {
            delete pComet;
        }
    }

    comets.sort ( CometCmp );
    return (int) comets.size() - start;
}

// Compares two asteroids for sorting.
// Returns true if the first asteroid should precede the second.

static bool CompareAsteroids ( const SSObjectPtr &pObj1, const SSObjectPtr &pObj2 )
{
    SSPlanetPtr p1 = SSGetPlanetPtr ( pObj1 );
    SSPlanetPtr p2 = SSGetPlanetPtr ( pObj2 );

    if ( p1 == nullptr || p2 == nullptr )
        return p1 != nullptr;

    SSIdentifier id1 = p1->getIdentifier ( kCatAstNum );
    SSIdentifier id2 = p2->getIdentifier ( kCatAstNum );

    // Numbered asteroids precede unnumbered asteroids.

    if ( bool ( id1 ) != bool ( id2 ) )
        return bool ( id1 );

    // Sort numbered asteroids by asteroid number.

    if ( id1 )
        return id1.identifier() < id2.identifier();

    // Sort unnumbered asteroids by designation alphabetically.

    return p1->getName ( 0 ) < p2->getName ( 0 );
}

// Imports combined asteroid data from JPL DASTCOM and Minor Planet Center MPCORB.
// JPL asteroids are imported preferentially and retain their original data.
// MPC asteroids are appended only if their asteroid number or provisional designation is not present in JPL.
// Imported data is appended to the input vector of SSObjects (asteroids).
// Returns total number of asteroids successfully imported.

int SSImportAsteroids ( const string &jplFilename, const string &mpcFilename, SSObjectVec &asteroids )
{
    // Import JPL asteroids first.

    int start = (int) asteroids.size();
    int n = SSImportJPLDASTCOM ( jplFilename, kTypeAsteroid, asteroids );
    if ( n < 1 )
        return 0;

    // Build sets of all numbered and unnumbered asteroid designations found in JPL.

    unordered_set<uint32_t> numbers;
    unordered_set<string> designations;

    numbers.reserve ( n );
    designations.reserve ( n );

    for ( int i = start; i < asteroids.size(); i++ )
    {
        SSPlanetPtr pAsteroid = SSGetPlanetPtr ( asteroids[i] );
        if ( pAsteroid == nullptr )
            continue;

        SSIdentifier ident = pAsteroid->getIdentifier ( kCatAstNum );
        if ( ident )
        {
            numbers.insert ( (uint32_t) ident.identifier() );
        }
        else
        {
            string desig = pAsteroid->getName ( 0 );
            if ( ! desig.empty() )
                designations.insert ( desig );
        }
    }

    // Read MPC asteroids and append only those not found in JPL DASTCOM.

    ifstream file ( mpcFilename );
    if ( ! file )
        return (int) asteroids.size() - start;

    string line;
    while ( getline ( file, line ) )
    {
        SSPlanetPtr pAsteroid = SSImportMPCAsteroid ( line );
        if ( pAsteroid == nullptr )
        {
            // cerr << line << endl;
            continue;
        }

        bool found = false;
        SSIdentifier ident = pAsteroid->getIdentifier ( kCatAstNum );
        if ( ident )
        {
            uint32_t number = (uint32_t) ident.identifier();
            if ( numbers.find ( number ) != numbers.end() )
                found = true;
            else
                numbers.insert ( number );
        }
        else
        {
            string desig = pAsteroid->getName ( 0 );
            if ( desig.empty() || designations.find ( desig ) != designations.end() )
                found = true;
            else
                designations.insert ( desig );
        }

        if ( found )
            delete pAsteroid;
        else
            asteroids.append ( pAsteroid );
    }

    asteroids.sort ( CompareAsteroids );
    return (int) asteroids.size() - start;
}
