// SSVector.hpp
// SSCore
//
// Created by Tim DeBenedictis on 2/24/20.
// Copyright © 2020 Southern Stars. All rights reserved.
//
// Classes for converting points between spherical and rectangular coordinates,
// and for performing vector arithmetic operations.

#ifndef SSVector_hpp
#define SSVector_hpp

#include "SSAngle.hpp"

// Represents a point in a spherical coordinate system (lon/lat, RA/Dec, Az/Alt)

struct SSVector;

struct SSSpherical
{
    SSAngle lon;    // Longitude coordinate, in radians from 0 to kTwoPi.
    SSAngle lat;    // Latitude coordinate, in radians from -kHalfPi to +kHalfPi.
    double  rad;    // Radial distance from origin of coordinate system, in arbitrary units; 1.0 is a unit vector.
    
    SSSpherical ( void );
    SSSpherical ( SSAngle lon, SSAngle lat );
    SSSpherical ( SSAngle lon, SSAngle lat, double rad );
    SSSpherical ( SSVector vec );

    SSAngle angularSeparation ( SSSpherical other );
    SSAngle positionAngle ( SSSpherical other );

    SSVector toVectorPosition ( void );
    SSVector toVectorVelocity ( SSSpherical vsph );

    bool operator == ( SSSpherical other ) { return lon == other.lon && lat == other.lat && rad == other.rad; }
    bool operator != ( SSSpherical other ) { return lon != other.lon || lat != other.lat || rad != other.rad; }
};

// Represents a point in a rectangular (x,y,z) coordinate system.

struct SSVector
{
    double x, y, z;    // Point's distance from origin along X, Y, Z axes, in arbitrary units.

    SSVector ( void );
    SSVector ( double x, double y, double z );
    SSVector ( SSSpherical lbr );
    
    double magnitude ( void );
    SSVector normalize ( void );
    SSVector normalize ( double &magnitude );

    SSVector add ( SSVector other );
    SSVector subtract ( SSVector other );
    SSVector multiplyBy ( double s );
    SSVector divideBy ( double s );
    
    double dotProduct ( SSVector other );
    SSVector crossProduct ( SSVector other );
    
    operator double() { return magnitude(); }
    
    SSVector operator + ( SSVector other ) { return add ( other ); }
    SSVector operator - ( SSVector other ) { return subtract ( other ); }
    double   operator * ( SSVector other ) { return dotProduct ( other ); }
    SSVector operator * ( double scale )  { return multiplyBy ( scale ); }
    SSVector operator / ( double scale )  { return divideBy ( scale ); }

    bool operator == ( SSVector other ) { return x == other.x && y == other.y && z == other.z; }
    bool operator != ( SSVector other ) { return x != other.x || y != other.y || z != other.z; }
    
    void operator += ( SSVector other ) { *this = add ( other ); }
    void operator -= ( SSVector other ) { *this = subtract ( other ); }
    void operator *= ( double scale )  { *this = multiplyBy ( scale ); }
    void operator /= ( double scale )  { *this = divideBy ( scale ); }

    bool isinf ( void ) { return std::isinf ( x ) || std::isinf ( y ) || std::isinf ( z ); }
    bool isnan ( void ) { return std::isnan ( x ) || std::isnan ( y ) || std::isnan ( z ); }

    SSAngle angularSeparation ( SSVector other );
    SSAngle positionAngle ( SSVector other );
    
    double distance ( SSVector other );

    SSSpherical toSpherical ( void );
    SSSpherical toSphericalVelocity ( SSVector vvec );
};

#endif /* SSVector_hpp */
