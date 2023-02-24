// SSObject.cpp
// SSCore
//
// Created by Tim DeBenedictis on 3/15/20.
// Copyright © 2020 Southern Stars. All rights reserved.
//
// Base class for all types of celestial objects (stars, planets, constellations, etc.)
// Also includes SSObjectArray, a class for storing a collection of objects,
// and saving/loading them from CSV files, with built-in memory management.

#include <iostream>
#include <fstream>
#include <map>

#include "SSObject.hpp"
#include "SSPlanet.hpp"
#include "SSStar.hpp"
#include "SSFeature.hpp"
#include "SSConstellation.hpp"

typedef map<SSObjectType,string> SSTypeStringMap;
typedef map<string,SSObjectType> SSStringTypeMap;

static SSTypeStringMap _typeNames =
{
    { kTypeNonexistent, "Nonexistent object" },
    { kTypePlanet, "Planet" },
    { kTypeMoon, "Moon" },
    { kTypeAsteroid, "Asteroid" },
    { kTypeComet, "Comet" },
    { kTypeSatellite, "Satellite" },
    { kTypeSpacecraft, "Spacecraft" },
    { kTypeFeature, "Feature" },
    { kTypeCity, "City" },
    { kTypeStar, "Star" },
    { kTypeDoubleStar, "Double star" },
    { kTypeVariableStar, "Variable star" },
    { kTypeDoubleVariableStar, "Double variable star" },
    { kTypeOpenCluster, "Open cluster" },
    { kTypeGlobularCluster, "Globular cluster" },
    { kTypeBrightNebula, "Bright nebula" },
    { kTypeDarkNebula, "Dark Nebula" },
    { kTypePlanetaryNebula, "Planetary Nebula" },
    { kTypeGalaxy, "Galaxy" },
    { kTypeConstellation, "Constellation" },
    { kTypeAsterism, "Asterism" }
};

static SSTypeStringMap _typeStrings =
{
    { kTypeNonexistent, "NO" },
    { kTypePlanet, "PL" },
    { kTypeMoon, "MN" },
    { kTypeAsteroid, "AS" },
    { kTypeComet, "CM" },
    { kTypeSatellite, "ST" },
    { kTypeSpacecraft, "SC" },
    { kTypeFeature, "FT" },
    { kTypeCity, "CT" },
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
    { "FT", kTypeFeature },
    { "CT", kTypeCity },
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

string SSObject::typeToName ( SSObjectType type )
{
    return _typeNames[ type ];
}

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
    _description = "";
    _direction = SSVector ( INFINITY, INFINITY, INFINITY );
    _distance = INFINITY;
    _magnitude = INFINITY;
}

string SSObject::getName ( int i )
{
    if ( i >= 0 && i < _names.size() )
        return _names[i];
    else
        return string ( "" );
}

string SSObject::getTypeName ( void )
{
    return typeToName ( _type );
}

// Default implementation of getIdentifer(); overridden by subclasses.

SSIdentifier SSObject::getIdentifier ( int i )
{
    return SSIdentifier();
}

SSIdentifier SSObject::getIdentifier ( SSCatalog cat )
{
    return SSIdentifier();
}

// Default implementation of addIdentifer; overridden by subclasses.

bool SSObject::addIdentifier ( SSIdentifier ident )
{
    return false;
}

// Default implementation of getIdentifiers; overridden by subclasses.

vector<SSIdentifier> SSObject::getIdentifiers ( void )
{
    return vector<SSIdentifier> ( 0 );
}

// Default implementation of toCSV; overridden by subclasses.

string SSObject::toCSV ( void )
{
    return "";
}

// Default implementation of computing object's apparent motion in a reference frame
// returns unknown motion. Overridden by sublcasses SSStar and SSPlanet!

SSSpherical SSObject::computeApparentMotion ( SSCoordinates &coords, SSFrame frame )
{
    return SSSpherical ( INFINITY, INFINITY, INFINITY );
}

SSObjectPtr SSObjectArray::set ( size_t index, SSObjectPtr pNew )
{
    if ( index >= 0 && index < size() )
    {
        SSObjectPtr pOld = _objects[index];
        _objects[index] = pNew;
        return pOld;
    }

    return nullptr;
}

// Binary-searches SSObjectArray for objects matching a specific object (key)
// using a caller-supplied comparison function (cmpfunc) that returns true if
// the first object (p1) is less than the second (p2), and false if p1 >= p2.
// SSObjectArray must be sorted prior to search, using same comparison function!!!
// See SSObjectArray::sort() and https://en.cppreference.com/w/cpp/algorithm/sort.
// Results are appended to vector (results); returns number of objects found.

int SSObjectArray::search ( const SSObjectPtr &pKey, bool (cmpfunc) ( const SSObjectPtr &p1, const SSObjectPtr &p2 ), vector<SSObjectPtr> &results )
{
    int nfound = 0;

    auto range = equal_range ( _objects.begin(), _objects.end(), pKey, cmpfunc );
    for ( auto it = range.first; it < range.second; it++ )
    {
        nfound++;
        results.push_back ( *it );
    }

    return nfound;
}

// Searches SSObjectArray for objects passing a caller-supplied test function (testfunc)
// which returns true if the object passes, and false if the object does not.
// SSObjectArray does not need to be sorted prior to search; this search() tests all objects.
// Results are appended to vector (results); returns number of objects found.

int SSObjectArray::search ( bool (*testfunc) ( const SSObjectPtr &pObject ), vector<SSObjectPtr> &results )
{
    int nfound = 0;
    
    for ( const SSObjectPtr &pObject : _objects )
    {
        if ( testfunc ( pObject ) )
        {
            nfound++;
            results.push_back ( pObject );
        }
    }
    
    return nfound;
}

// Searches SSObjectArray for objects appearing within a circle of (radius) radians,
// centered on the celestial sphere at unit direction vector (center) in the fundamental frame.
// Indexes of found objects are appended to vector (results); returns number of objects found.

int SSObjectArray::search ( SSVector center, SSAngle radius, vector<size_t> &results )
{
    int nfound = 0;
    
    for ( size_t index = 0; index < _objects.size(); index++ )
    {
        SSStar *pStar = SSGetStarPtr ( _objects[index] );
        if ( pStar && center.angularSeparation ( pStar->getFundamentalPosition() ) < radius )
        {
            nfound++;
            results.push_back ( index );
        }
    }
    
    return nfound;
}

// Deletes objects in this SSObjectArray appearing within a circle of (radius) radians,
// centered on the celestial sphere at unit direction vector (center) in the fundamental frame.
// Returns number of objects deleted.

int SSObjectArray::erase ( SSVector center, SSAngle radius )
{
    int nfound = 0;
    
    for ( size_t index = 0; index < _objects.size(); index++ )
    {
        SSStar *pStar = SSGetStarPtr ( _objects[index] );
        if ( pStar && center.angularSeparation ( pStar->getFundamentalPosition() ) < radius )
        {
            nfound++;
            // cout << _objects[index]->toCSV() << endl;
            delete _objects[index];
            _objects.erase ( _objects.begin() + index );
        }
    }
    
    return nfound;
}

// Deletes objects in this SSObjectArray appearing within a circle of (radius) radians,
// centered on any star in another SSObjectArray (stars).
// Returns number of objects deleted.

int SSObjectArray::erase ( SSObjectVec &stars, SSAngle radius )
{
    int n = 0;
    for ( SSObjectPtr pObject : stars._objects )
    {
        SSStarPtr pStar = SSGetStarPtr ( pObject );
        if ( pStar )
            n += erase ( pStar->getFundamentalPosition(), radius );
    }
    return n;
}

// Searches SSObjectArray for objects appearing within a circle of (radius) radians,
// centered on the celestial sphere at unit direction vector (center) in the fundamental frame.
// Pointers to found objects are appended to vector (results); returns number of objects found.

int SSObjectArray::search ( SSVector center, SSAngle radius, vector<SSObjectPtr> &results )
{
    vector<size_t> indexes;
    int nfound = search ( center, radius, indexes );
    for ( size_t index : indexes )
        results.push_back ( _objects[index] );
    return nfound;
}

// Given a vector of smart pointers to SSObject, creates a mapping of SSIdentifiers
// in a particular catalog (cat) to index number within the vector.
// Useful for fast object retrieval by identifier (see SSIdentifierToObject()).

SSObjectMap SSMakeObjectMap ( SSObjectVec &objects, SSCatalog cat )
{
    SSObjectMap map;
    
    for ( int i = 0; i < objects.size(); i++ )
    {
        SSObject *ptr = objects[i];
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
    SSTLE tle;
    
    if ( type == kTypeSatellite )
        return new SSSatellite ( tle );
    else if ( type >= kTypePlanet && type <= kTypeSpacecraft )
        return new SSPlanet ( type );
    else if ( type == kTypeFeature )
        return new SSFeature;
    else if ( type == kTypeCity )
        return new SSCity;
    else if ( type == kTypeStar )
        return new SSStar;
    else if ( type == kTypeDoubleStar )
        return new SSDoubleStar;
    else if ( type == kTypeVariableStar )
        return new SSVariableStar;
    else if ( type == kTypeDoubleVariableStar )
        return new SSDoubleVariableStar;
    else if ( type >= kTypeOpenCluster && type <= kTypeGalaxy )
        return new SSDeepSky ( type );
    else if ( type == kTypeNonexistent )
        return new SSDeepSky ( type );
    else if ( type >= kTypeConstellation && type <= kTypeAsterism )
        return new SSConstellation ( type );
    else
        return nullptr;
}

// Allocates a new object which is a complete deep copy of an existing object (pObj)
// Returns pointer to null if pObj is null.

SSObjectPtr SSCloneObject ( SSObject *pObj )
{
    if ( pObj == nullptr )
        return nullptr;
    
    SSObjectType type = pObj->getType();
    if ( type == kTypeSatellite )
        return new SSSatellite ( *SSGetSatellitePtr ( pObj ) );
    else if ( type >= kTypePlanet && type <= kTypeSpacecraft )
        return new SSPlanet ( *SSGetPlanetPtr ( pObj ) );
    else if ( type == kTypeFeature )
        return new SSFeature ( *SSGetFeaturePtr ( pObj ) );
    else if ( type == kTypeCity )
        return new SSCity ( *SSGetCityPtr ( pObj ) );
    else if ( type == kTypeStar )
        return new SSStar ( *SSGetStarPtr ( pObj ) );
    else if ( type == kTypeDoubleStar )
        return new SSDoubleStar ( *SSGetDoubleStarPtr ( pObj ) );
    else if ( type == kTypeVariableStar )
        return new SSVariableStar ( *SSGetVariableStarPtr ( pObj ) );
    else if ( type == kTypeDoubleVariableStar )
        return new SSDoubleVariableStar ( *SSGetDoubleVariableStarPtr ( pObj ) );
    else if ( type >= kTypeOpenCluster && type <= kTypeGalaxy )
        return new SSDeepSky ( *SSGetDeepSkyPtr ( pObj ) );
    else if ( type == kTypeNonexistent )
        return new SSDeepSky ( *SSGetDeepSkyPtr ( pObj ) );
    else if ( type >= kTypeConstellation && type <= kTypeAsterism )
        return new SSConstellation ( *SSGetConstellationPtr ( pObj ) );
    else
        return nullptr;
}

// Exports a vector of objects to a CSV-formatted text file.
// If the filename is an empty string, streams CSV to standard output.
// If a non-null filter function (filter) is provided, objects are exported
// only if they pass the filter; optional data pointer (userData) is passed
// to the filter but not used otherwise.
// Returns the number of objects exported.

int SSExportObjectsToCSV ( const string &filename, SSObjectVec &objects, SSObjectFilter filter, void *userData )
{
    int i = 0;
    
    // If filename is empty, just stream everything to stdout
    
    if ( filename.empty() )
    {
        for ( i = 0; i < objects.size(); i++ )
            cout << objects[i]->toCSV() << endl;
        
        return i;
    }

    // Otherwise open file, overwriting existing content; return on failure.

    ofstream file ( filename, ios::trunc );
    if ( ! file )
        return 0;
    
    // Stream objects which pass filter function to CSV file.
    
    int n = 0;
    for ( i = 0; i < objects.size(); i++ )
    {
        if ( filter == nullptr || filter ( objects[i], userData ) )
        {
            file << objects[i]->toCSV() << endl;
            n++;
        }
    }
    
    // Return exported object count; file will close automatically.
    
    return n;
}

// Imports objects from CSV-formatted text file (filename).
// Imported objects are appended to the input vector of SSObjects (objects).
// If a non-null filter function (filter) is provided, objects are imported
// only if they pass the filter; optional data pointer (userData) is passed
// to the filter but not used otherwise.
// Function returns number of objects successfully imported.

int SSImportObjectsFromCSV ( const string &filename, SSObjectVec &objects, SSObjectFilter filter, void *userData )
{
    // Open file; return on failure.

    FILE *file = fopen ( filename.c_str(), "rb" );
    if ( ! file )
        return 0;

    // Read file line-by-line until we reach end-of-file

    string line = "";
    int numObjects = 0;

    while ( fgetline ( file, line ) )
    {
        // Attempt to create object from CSV file line
        
        SSObjectPtr pObject = SSPlanet::fromCSV ( line );
        if ( pObject == nullptr )
            pObject = SSStar::fromCSV ( line );
        if ( pObject == nullptr )
            pObject = SSFeature::fromCSV ( line );
        if ( pObject == nullptr )
            pObject = SSConstellation::fromCSV ( line );

        // if successful, and object passes filter, add it to object vector.
            
        if ( pObject != nullptr && ( filter == nullptr || filter ( pObject, userData ) ) )
        {
            objects.append ( pObject );
            numObjects++;
        }
    }
    
    // Close file. Return number of objects added to object vector.

    fclose ( file );
    return numObjects;
}
