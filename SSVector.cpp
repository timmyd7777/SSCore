//  SSVector.hpp
//  SSCore
//
//  Created by Tim DeBenedictis on 2/24/20.
//  Copyright © 2020 Southern Stars. All rights reserved.

#include <math.h>
#include "SSVector.hpp"

// Constructs spberical coordinates from longitude and latitude, both in radians.
// Since we often don't care about the radial coordinate, set it to 1.0 to make a unit vector.

SSSpherical::SSSpherical ( SSAngle lon, SSAngle lat )
{
	this->lon = lon;
	this->lat = lat;
	this->rad = 1.0;
}

// Constructs spberical coordinates from longitude and latitude, both in radians,
// and radial distance from the origin in arbitrary units.

SSSpherical::SSSpherical ( SSAngle lon, SSAngle lat, double rad )
{
	this->lon = lon;
	this->lat = lat;
	this->rad = rad;
}

// Constructs spberical coordinates from a rectangular coordinate vector.

SSSpherical::SSSpherical ( SSVector vec )
{
    rad = vec.magnitude();
    lat = SSAngle ( asin ( vec.z / rad ) );
    lon = SSAngle ( atan2 ( vec.y, vec.x ) ).mod2Pi();
}

// Returns angular separation in radians from this point in a spherical coordinate system
// to another point in the same spherical coordinate system.  Both points must have radial
// distance from the origin set to 1.0, or the returned value will be invalid.

SSAngle SSSpherical::angularSeparation ( SSSpherical other )
{
    return SSVector ( *this ).angularSeparation ( SSVector ( other ) );
}

// Constructs a rectangular coordinate vector at the origin of the coordinate system

SSVector::SSVector ( void )
{
    x = y = z = 0.0;
}

// Constructs a rectangular coordinate vector with X, Y, Z positions relative to
// the origin of the coordinate system specified in arbitrary units.

SSVector::SSVector ( double x, double y, double z )
{
    this->x = x;
    this->y = y;
    this->z = z;
}

// Constructs a rectangular coordinate vector from spherical coordinates.
// The origin of longitude is along the +X axis, and X/Y plane is the "equator"
// The origin of latitude is the X/Y plane, and latitude increases with Z.
// The +Z axis runs though the "north pole" of the spherical coordinate system.
// The origin of both systems is at (0,0,0).

SSVector::SSVector ( SSSpherical sph )
{
	x = sph.rad * cos ( sph.lat.rad ) * cos ( sph.lon.rad );
	y = sph.rad * cos ( sph.lat.rad ) * sin ( sph.lon.rad );
	z = sph.rad * sin ( sph.lat.rad );
}

// Returns this vector's magnitude (length) measured from the origin.

double SSVector::magnitude ( void )
{
    return ( sqrt ( x * x + y * y + z * z ) );
}

// Returns a copy of this vector normalized to unit length, and returns its original magnitude.
// If the original vector was a zero-length vector, the returned vector will also be zero length.
// Does not modify this vector!

SSVector SSVector::normalize ( double &mag )
{
    mag = magnitude();
    if ( mag > 0.0 )
        return divideBy ( mag );
    else
        return SSVector ( 0.0, 0.0, 0.0 );
}

// Returns a copy of this vector normalized to unit length. Does not modify this vector!

SSVector SSVector::normalize ( void )
{
    double mag;
    return normalize ( mag );
}


// Returns a vector which is the sum of this vector with another.
// This vector is not affected by the addition.

SSVector SSVector::add ( SSVector other )
{
    return ( SSVector ( x + other.x, y + other.y, z + other.z ) );
}

// Returns a vector which is the difference of this vector with another.
// This vector is not affected by the subtraction.

SSVector SSVector::subtract ( SSVector other )
{
    return ( SSVector ( x - other.x, y - other.y, z - other.z ) );
}

// Returns a copy of this vector multiplied by a scale factor.
// This vector is not modified by the scaling.

SSVector SSVector::multiplyBy ( double s )
{
    return ( SSVector ( x * s, y * s, z * s ) );
}

// Returns a copy of this vector divided by a scale factor.
// This vector is not modified by the scaling.

SSVector SSVector::divideBy ( double s )
{
    return ( SSVector ( x / s, y / s, z / s ) );
}

// Returns the dot product of this vector with another vector.

double SSVector::dotProduct ( SSVector other )
{
    return ( x * other.x + y * other.y + z * other.z );
}

// Returns the vector cross product of this vector with another vector.
// This vector is not modified by the cross product operation.

SSVector SSVector::crossProduct ( SSVector other )
{
    double u = y * other.z - z * other.y;
    double v = z * other.x - x * other.z;
    double w = x * other.y - y * other.x;
    
    return ( SSVector ( u, v, w ) );
}

// Returns the angular separation in radians from this point in a rectangular coordinate system
// to another point in the same rectangular system, as seen from the origin of the coordinate system.

SSAngle SSVector::angularSeparation ( SSVector other )
{
    return SSAngle ( asin ( dotProduct ( other ) ) );
}

// Returns the distance from this point in a rectangular coordinate system
// to another point in the same rectangular system, in the same arbitrary
// units that X,Y,Z coordinates of both systems are measured in.

double SSVector::distance ( SSVector other )
{
    return subtract ( other ).magnitude();
}
