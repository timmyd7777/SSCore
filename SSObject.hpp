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
    kUnknown = 0,
    kStar = 1,
    kDoubleStar = 2,
    kVariableStar = 4,
    kDoubleVariableStar = 6,
    kOpenCluster = 10,
    kGlobularCluster = 11,
    kBrightNebula = 12,
    kDarkNebula = 13,
    kPlanetaryNebula = 14,
    kGalaxy = 15,
    kPlanet = 20,
    kMoon = 21,
    kAsteroid = 22,
    kComet = 23,
    kSatellite = 24,
    kSpacecraft = 25
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

    SSObject ( SSObjectType type );       // constuctor: takes type code
    
    SSObjectType    getType ( void ) { return _type; }
    SSVector        getDirection ( void ) { return _direction; }
    double          getDistance ( void ) { return _distance; }
    float           getMagnitude ( void ) { return _magnitude; }
    
    virtual string getName ( int i );  // returns copy of i-th name string
    virtual void computeEphemeris ( class SSDynamics ) = 0;     // computes direction, distance, magnitude for the given dynamical state

};

#pragma pack ( pop )

#endif /* SSObject_hpp */
