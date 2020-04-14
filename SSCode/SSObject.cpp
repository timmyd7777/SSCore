// SSObject.cpp
// SSCore
//
// Created by Tim DeBenedictis on 3/15/20.
// Copyright © 2020 Southern Stars. All rights reserved.
//
// Base class for all types of celestial objects (stars, planets, constellations, etc.)

#include <iostream>
#include <fstream>
#include <map>

#include "SSObject.hpp"
#include "SSPlanet.hpp"
#include "SSStar.hpp"
#include "SSConstellation.hpp"
#include "SSCoordinates.hpp"

typedef map<SSObjectType,string> SSTypeStringMap;
typedef map<string,SSObjectType> SSStringTypeMap;

static SSTypeStringMap _typeStrings =
{
    { kTypeNonexistent, "NO" },
    { kTypePlanet, "PL" },
    { kTypeMoon, "MN" },
    { kTypeAsteroid, "AS" },
    { kTypeComet, "CM" },
    { kTypeSatellite, "ST" },
    { kTypeSpacecraft, "SC" },
    { kTypeStar, "SS" },
    { kTypeDoubleStar, "DS" },
    { kTypeVariableStar, "VS" },
    { kTypeDoubleVariableStar, "DV" },
    { kTypeOpenCluster, "OC" },
    { kTypeGlobularCluster, "GC" },
    { kTypeBrightNebula, "BN" },
    { kTypeDarkNebula, "DN" },
    { kTypePlanetaryNebula, "PN" },
    { kTypeGalaxy, "GX" },
    { kTypeConstellation, "CN" },
    { kTypeAsterism, "AM" }
};

static SSStringTypeMap _stringTypes =
{
    { "NO", kTypeNonexistent },
    { "PL", kTypePlanet },
    { "MN", kTypeMoon },
    { "AS", kTypeAsteroid },
    { "CM", kTypeComet },
    { "ST", kTypeSatellite },
    { "SC", kTypeSpacecraft },
    { "SS", kTypeStar },
    { "DS", kTypeDoubleStar },
    { "VS", kTypeVariableStar },
    { "DV", kTypeDoubleVariableStar },
    { "OC", kTypeOpenCluster },
    { "GC", kTypeGlobularCluster },
    { "BN", kTypeBrightNebula },
    { "DN", kTypeDarkNebula },
    { "PN", kTypePlanetaryNebula },
    { "GX", kTypeGalaxy },
    { "CN", kTypeConstellation },
    { "AM", kTypeAsterism }
};

string SSObject::typeToCode ( SSObjectType type )
{
    return _typeStrings[ type ];
}

SSObjectType SSObject::codeToType ( string code )
{
    return _stringTypes[ code ];
}

SSObject::SSObject ( void ) : SSObject ( kTypeNonexistent )
{

}

SSObject::SSObject ( SSObjectType type )
{
    _type = type;
    _names = vector<string> ( 0 );
    _direction = SSVector ( HUGE_VAL, HUGE_VAL, HUGE_VAL );
    _distance = HUGE_VAL;
    _magnitude = HUGE_VAL;
}

string SSObject::getName ( int i )
{
    if ( i >= 0 && i < _names.size() )
        return _names[i];
    else
        return string ( "" );
}

// Default implementation of getIdentifer; overridden by subclasses.

SSIdentifier SSObject::getIdentifier ( SSCatalog cat )
{
    return SSIdentifier();
}

// Default implementation of addIdentifer; overridden by subclasses.

bool SSObject::addIdentifier ( SSIdentifier ident )
{
    return false;
}

// Default implementation of toCSV; overridden by subclasses.

string SSObject::toCSV ( void )
{
    return "";
}

// Default implementation of compteEphemeris; overridden by subclasses.

void SSObject::computeEphemeris ( SSCoordinates &coords )
{
    
}

// Given a vector of smart pointers to SSObject, creates a mapping of SSIdentifiers
// in a particular catalog (cat) to index number within the vector.
// Useful for fast object retrieval by identifier (see SSIdentifierToObject()).

SSObjectMap SSMakeObjectMap ( SSObjectVec &objects, SSCatalog cat )
{
    SSObjectMap map;
    
    for ( int i = 0; i < objects.size(); i++ )
    {
        SSObject *ptr = objects[i].get();
        if ( ptr == nullptr )
            continue;
        
        SSIdentifier ident = ptr->getIdentifier ( cat );
        if ( ! ident )
            continue;
        
        map.insert ( { ident, i + 1 } );
    }
    
    return map;
}

// Given a catalog identifier (ident), a mapping of identifiers to object indices (map),
// and a vector of smart pointers to objects (objects), this function returns a smart pointer
// to the first object in the vector which matches ident.  If the identifier does not map to
// any object in the vector, this function returns a smart pointer to null!

SSObjectPtr SSIdentifierToObject ( SSIdentifier ident, SSObjectMap &map, SSObjectVec &objects )
{
    int k = map[ ident ];

    if ( k > 0 )
        return objects[ k - 1 ];
    else
        return SSObjectPtr ( nullptr );
}

// Allocates a new object of the specific object type and returns a shared pointer to it.
// On failure, returns a shared pointer to null.

SSObjectPtr SSNewObject ( SSObjectType type )
{
    if ( type >= kTypePlanet && type <= kTypeSpacecraft )
        return shared_ptr<SSPlanet> ( new SSPlanet ( type ) );
    else if ( type == kTypeStar )
        return shared_ptr<SSStar> ( new SSStar );
    else if ( type == kTypeDoubleStar )
        return shared_ptr<SSDoubleStar> ( new SSDoubleStar );
    else if ( type == kTypeVariableStar )
        return shared_ptr<SSVariableStar> ( new SSVariableStar );
    else if ( type == kTypeDoubleVariableStar )
        return shared_ptr<SSDoubleVariableStar> ( new SSDoubleVariableStar );
    else if ( type >= kTypeOpenCluster && type <= kTypeGalaxy )
        return shared_ptr<SSDeepSky> ( new SSDeepSky ( type ) );
    else if ( type >= kTypeConstellation && type <= kTypeAsterism )
        return shared_ptr<SSConstellation> ( new SSConstellation ( type ) );
    else
        return shared_ptr<class SSObject> ( nullptr );
}

// Exports a vector of objects to a CSV-formatted text file.
// If the filename is an empty string, streams CSV to standard output.
// Returns the number of objects exported.

int SSExportObjectsToCSV ( const string &filename, SSObjectVec &objects )
{
    int i = 0;
    
    // If filename is empty, just stream everything to stdout
    
    if ( filename.empty() )
    {
        for ( i = 0; i < objects.size(); i++ )
            cout << objects[i].get()->toCSV() << endl;
        
        return i;
    }

    // Otherwise open file; return on failure.

    ofstream file ( filename );
    if ( ! file )
        return 0;
    
    // Stream everything to file.
    
    for ( i = 0; i < objects.size(); i++ )
        file << objects[i].get()->toCSV() << endl;

    // Return object count; file will close automatically.
    
    return i;
}

// Imports objects from CSV-formatted text file (filename).
// Imported objects are appended to the input vector of SSObjects (objects).
// Returns number of objects successfully imported.

int SSImportObjectsFromCSV ( const string &filename, SSObjectVec &objects )
{
    // Open file; return on failure.

    FILE *file = fopen ( filename.c_str(), "r" );
    if ( ! file )
        return 0;

    // Read file line-by-line until we reach end-of-file

    string line = "";
    int numObjects = 0;

    while ( fgetline ( file, line ) )
    {
        // Attempt to create solar system object from CSV file line; if successful add to object vector.
        
        SSObjectPtr pObject = SSPlanet::fromCSV ( line );
        if ( pObject )
        {
            objects.push_back ( pObject );
            numObjects++;
            continue;
        }

        // Attempt to create star from CSV file line; if successful add to object vector.

        pObject = SSStar::fromCSV ( line );
        if ( pObject )
        {
            objects.push_back ( pObject );
            numObjects++;
            continue;
        }
        
        // Attempt to create constellation from CSV file line; if successful add to object vector.

        pObject = SSConstellation::fromCSV ( line );
        if ( pObject )
        {
            objects.push_back ( pObject );
            numObjects++;
            continue;
        }
    }
    
    // Close file. Return number of objects added to object vector.

    fclose ( file );
    return numObjects;
}
