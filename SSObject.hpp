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

// Recognized astronomical object types

enum SSObjectType
{
    kTypeUnknown = 0,
    kTypePlanet = 1,
    kTypeMoon = 2,
    kTypeAsteroid = 3,
    kTypeComet = 4,
    kTypeSatellite = 5,
    kTypeSpacecraft = 6,
    kTypeStar = 10,
    kTypeDoubleStar = 12,
    kTypeVariableStar = 13,
    kTypeDoubleVariableStar = 14,
    kTypeOpenCluster = 20,
    kTypeGlobularCluster = 21,
    kTypeBrightNebula = 22,
    kTypeDarkNebula = 23,
    kTypePlanetaryNebula = 24,
    kTypeGalaxy = 25,
    kTypeConstellation = 30,
    kTypeAsterism = 31,
};

#pragma pack ( push, 1 )

// This is the base class for all astronomical objects (planets, stars, deep sky objects, constellations, etc.)

class SSObject
{
protected:
    
    SSObjectType    _type;      	// object type code
    vector<string>  _names;     	// vector of name string(s)
    SSVector    	_direction; 	// apparent direction to object as unit vector in fundamental reference frame; infinite if unknown
    double      	_distance;  	// distance to object in AU; infinite if unknown
    float       	_magnitude;		// visual magnitude; infinite if unknown
    
public:

	// constructors
	
	SSObject ( void );
    SSObject ( SSObjectType type );

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
	
	static string typeToCode ( SSObjectType type );
	static SSObjectType codeToType ( string );

	virtual string getName ( int i );  // returns copy of i-th name string
    virtual void computeEphemeris ( class SSDynamics &dyn );     // computes direction, distance, magnitude for the given dynamical state
};

typedef shared_ptr<SSObject> SSObjectPtr;
typedef vector<SSObjectPtr> SSObjectVec;

SSObjectPtr SSNewObject ( SSObjectType type );

#pragma pack ( pop )

#endif /* SSObject_hpp */
