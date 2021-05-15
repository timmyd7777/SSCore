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
    string          _description;   // plain-text object description (may be empty)
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
    string getDescription ( void ) { return _description; }
    SSVector getDirection ( void ) { return _direction; }
    double getDistance ( void ) { return _distance; }
    float getMagnitude ( void ) { return _magnitude; }
    
    // modifiers. setType() with caution; _type is used to determine object class.
    
    void setType ( SSObjectType type ) { _type = type; }
    void setNames ( const vector<string> &names ) { _names = names; }
    void setDescription ( const string &desc ) { _description = desc; }
    void setDirection ( SSVector dir ) { _direction = dir; }
    void setDistance ( double dist ) { _distance = dist; }
    void setMagnitude ( float mag ) { _magnitude = mag; }
    
    // Converts integer object types to two-character type codes and vice-versa.
    
    static string typeToName ( SSObjectType type );
    static string typeToCode ( SSObjectType type );
    static SSObjectType codeToType ( string );

    virtual string getName ( int i );                           // returns copy of i-th name string
    virtual string getTypeName ( void );                        // returns string form of object type code
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
    SSObjectPtr get ( size_t index ) { return index >= 0 && index < size() ? _objects.at ( index ) : nullptr; }
    SSObjectPtr set ( size_t index, SSObjectPtr pObj );
    SSObjectPtr operator [] ( size_t index ) { return get ( index ); }
    void append ( SSObjectPtr pObj ) { _objects.push_back ( pObj ); }
    void insert ( SSObjectPtr pObj, size_t index ) { _objects.insert ( _objects.begin() + index, pObj ); }
    void remove ( size_t index ) { _objects.erase ( _objects.begin() + index ); }   // DOES NOT actually delete object!!!
    size_t size ( void ) { return _objects.size(); }
    void clear ( void ) { _objects.clear(); }   // empties object vector but DOES NOT delete individual objects!!!
    void erase ( void ) { for ( SSObjectPtr pObj : _objects ) delete pObj; clear(); }   // deletes all objects AND clears vector.
    void sort ( bool (*cmpfunc) ( const SSObjectPtr &p1, const SSObjectPtr &p2 ) ) { std::sort ( _objects.begin(), _objects.end(), cmpfunc ); }
    int search ( const SSObjectPtr &pKey, bool (*cmpfunc) ( const SSObjectPtr &p1, const SSObjectPtr &p2 ), vector<SSObjectPtr> &results );
    int search ( bool (*testfunc) ( const SSObjectPtr &pObject ), vector<SSObjectPtr> &results );
    int search ( SSVector center, SSAngle rad, vector<SSObjectPtr> &results );
    int search ( SSVector center, SSAngle rad, vector<size_t> &results );
    int erase ( SSVector center, SSAngle rad );
    int erase ( SSObjectArray &stars, SSAngle rad );
};

typedef SSObjectArray SSObjectVec;          // legacy declaration was typedef vector<SSObjectPtr> SSObjectVec; now we use SSObjectArray class
typedef map<SSIdentifier,int> SSObjectMap;

SSObjectPtr SSNewObject ( SSObjectType type );
SSObjectPtr SSCloneObject ( SSObject *pObj );
SSObjectMap SSMakeObjectMap ( SSObjectVec &objects, SSCatalog cat );
SSObjectPtr SSIdentifierToObject ( SSIdentifier ident, SSObjectMap &map, SSObjectVec &objects );

typedef bool (*SSObjectFilter) ( SSObjectPtr pObject, void *userData );
int SSImportObjectsFromCSV ( const string &filename, SSObjectVec &objects, SSObjectFilter filter = nullptr, void *userData = nullptr );
int SSExportObjectsToCSV ( const string &filename, SSObjectVec &objects, SSObjectFilter filter = nullptr, void *userData = nullptr );

#pragma pack ( pop )

#endif /* SSObject_hpp */
