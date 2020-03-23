//  SSObject.hpp
//  SSCore
//
//  Created by Tim DeBenedictis on 3/15/20.
//  Copyright © 2020 Southern Stars. All rights reserved.
//  Base class for all objects in the database (stars, planets, etc.)

#ifndef SSObject_hpp
#define SSObject_hpp

#include <stdio.h>
#include <math.h>

#include <string>
#include <vector>

#include "SSVector.hpp"
#include "SSIdentifier.hpp"

using namespace std;

// Recognized object type codes

enum SSObjectType
{
    kTypeUnknown = 0,
    kTypeStar = 1,
    kTypeDoubleStar = 2,
    kTypeVariableStar = 4,
    kTypeDoubleVariableStar = 6,
    kTypeOpenCluster = 10,
    kTypeGlobularCluster = 11,
    kTypeBrightNebula = 12,
    kTypeDarkNebula = 13,
    kTypePlanetaryNebula = 14,
    kTypeGalaxy = 15,
    kTypePlanet = 20,
    kTypeMoon = 21,
    kTypeAsteroid = 22,
    kTypeComet = 23,
    kTypeSatellite = 24,
    kTypeSpacecraft = 25
};

#pragma pack ( push, 1 )

class SSObject
{
protected:
    
    SSObjectType     _type;          // object type code
    vector<string>  _names;          // vector of name string(s)
    SSVector    _direction;          // apparent direction to object as unit vector in fundamental reference frame; infinite if unknown
    double      _distance;           // distance to object in AU; infinite if unknown
    float       _magnitude;          // visual magnitude; infinite if unknown
    
public:

	SSObject ( void );
    SSObject ( SSObjectType type );       // constuctor: takes type code
    
    SSObjectType    getType ( void ) { return _type; }
    SSVector        getDirection ( void ) { return _direction; }
    double          getDistance ( void ) { return _distance; }
    float           getMagnitude ( void ) { return _magnitude; }
    
    virtual string getName ( int i );  // returns copy of i-th name string
    virtual void computeEphemeris ( class SSDynamics &dyn );     // computes direction, distance, magnitude for the given dynamical state
};

typedef shared_ptr<SSObject> SSObjectPtr;
typedef vector<SSObjectPtr> SSObjectVec;

SSObjectPtr SSNewObject ( SSObjectType type );

#pragma pack ( pop )

#endif /* SSObject_hpp */
