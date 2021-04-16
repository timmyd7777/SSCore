// SSObject.hpp
// SSCore
//
// Created by Tim DeBenedictis on 3/15/20.
// Copyright © 2020 Southern Stars. All rights reserved.
// Base class for all objects in the database (stars, planets, etc.)
// Also includes SSObjectArray, a class for storing a collection of objects,
// and saving/loading them from CSV files, with built-in memory management.

#ifndef SSObject_hpp
#define SSObject_hpp

#include <stdio.h>
#include <math.h>

#include <algorithm>
#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <map>

#include "SSCoordinates.hpp"
#include "SSIdentifier.hpp"

using namespace std;

#pragma pack ( push, 1 )

// This is the base class for all astronomical objects (planets, stars, deep sky objects, constellations, etc.)

class SSObject
{
protected:
    
    SSObjectType    _type;          // object type code
    vector<string>  _names;         // vector of name string(s)
    SSVector        _direction;     // apparent direction to object as unit vector in fundamental reference frame; infinite if unknown
    double          _distance;      // distance to object in AU; infinite if unknown
    float           _magnitude;     // visual magnitude; infinite if unknown
    
public:

    // constructors
    
    SSObject ( void );
    SSObject ( SSObjectType type );
    virtual ~SSObject ( void ) {}   // test code: { cout << "~SSObject" << endl; }

    // accessors
    
    SSObjectType getType ( void ) { return _type; }
    vector<string> getNames ( void ) { return _names; }
    SSVector getDirection ( void ) { return _direction; }
    double getDistance ( void ) { return _distance; }
    float getMagnitude ( void ) { return _magnitude; }
    
    // modifiers. Type cannot be changed after object construction!
    
    void setNames ( vector<string> names ) { _names = names; }
    void setDirection ( SSVector dir ) { _direction = dir; }
    void setDistance ( double dist ) { _distance = dist; }
    void setMagnitude ( float mag ) { _magnitude = mag; }
    
    // Converts integer object types to two-character type codes and vice-versa.
    
    static string typeToName ( SSObjectType type );
    static string typeToCode ( SSObjectType type );
    static SSObjectType codeToType ( string );

    virtual string getName ( int i );                           // returns copy of i-th name string
    virtual SSIdentifier getIdentifier ( int i );               // returns copy of i-th catalog identifier
    virtual SSIdentifier getIdentifier ( SSCatalog cat );       // returns identifier in the specified catalog, or null identifier if object has none in that catalog.
    virtual bool addIdentifier ( SSIdentifier ident );          // adds the specified identifier to the object, only if the ident is valid and not already present.
    virtual vector<SSIdentifier> getIdentifiers ( void );       // returns vector of all object identifiers
    SSAngle angularSeparation ( SSObject &other ) { return _direction.angularSeparation ( other._direction ); }
    
    // Default empty implementations of position/velocity/ephemeris/motion computations are overridden by subclasses!
    
    virtual void computeEphemeris ( SSCoordinates &coords ) {};                                         // computes apparent direction, distance, magnitude seen from the given observer coordinates and time.
    virtual void computePositionVelocity ( SSCoordinates &coords, SSVector &pos, SSVector &vel ) {};    // computes heliocentric position and velocity at the time in the given coordinates object.
    virtual SSSpherical computeApparentMotion ( SSCoordinates &coords, SSFrame frame = kFundamental );  // computes object's apparent motion in the specified reference frame.
    
    virtual string toCSV ( void );
};

typedef SSObject *SSObjectPtr;

// This class stores a vector of pointers to SSObject, and deletes them when class instance is destroyed.

class SSObjectArray
{
protected:
    vector<SSObjectPtr> _objects;

public:
    ~SSObjectArray ( void ) { for ( SSObjectPtr pObj : _objects ) delete pObj; }
    SSObjectPtr at ( size_t index ) { return index >= 0 && index < size() ? _objects.at ( index ) : nullptr; }
    SSObjectPtr operator [] ( size_t index ) { return at ( index ); }
    void push_back ( SSObjectPtr pObj ) { _objects.push_back ( pObj ); }
    size_t size ( void ) { return _objects.size(); }
    void clear ( void ) { _objects.clear(); }   // empties object vector but DOES NOT delete individual objects!!!
    void sort ( bool (*cmpfunc) ( const SSObjectPtr &p1, const SSObjectPtr &p2 ) ) { std::sort ( _objects.begin(), _objects.end(), cmpfunc ); }
};

typedef SSObjectArray SSObjectVec;          // legacy declaration was typedef vector<SSObjectPtr> SSObjectVec; now we use SSObjectArray class
typedef map<SSIdentifier,int> SSObjectMap;

SSObjectPtr SSNewObject ( SSObjectType type );
SSObjectPtr SSCloneObject ( SSObject *pObj );
SSObjectMap SSMakeObjectMap ( SSObjectVec &objects, SSCatalog cat );
SSObjectPtr SSIdentifierToObject ( SSIdentifier ident, SSObjectMap &map, SSObjectVec &objects );

int SSImportObjectsFromCSV ( const string &filename, SSObjectVec &objects );
int SSExportObjectsToCSV ( const string &filename, SSObjectVec &objects );

#pragma pack ( pop )

#endif /* SSObject_hpp */
