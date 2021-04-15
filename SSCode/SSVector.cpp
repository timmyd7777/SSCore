//  SSVector.hpp
//  SSCore
//
//  Created by Tim DeBenedictis on 2/24/20.
//  Copyright © 2020 Southern Stars. All rights reserved.

#include <math.h>
#include "SSVector.hpp"

// Constructs spherical coordinates with default values of zero.

SSSpherical::SSSpherical ( void )
{
    lon = lat = rad = 0.0;
}

// Constructs spherical coordinates from longitude and latitude, both in radians.
// Since we often don't care about the radial coordinate, set it to 1.0 to make a unit vector.

SSSpherical::SSSpherical ( SSAngle lon, SSAngle lat )
{
    this->lon = lon;
    this->lat = lat;
    this->rad = 1.0;
}

// Constructs spherical coordinates from longitude and latitude, both in radians,
// and radial distance from the origin in arbitrary units.

SSSpherical::SSSpherical ( SSAngle lon, SSAngle lat, double rad )
{
    this->lon = lon;
    this->lat = lat;
    this->rad = rad;
}

// Constructs spherical coordinates from a rectangular coordinate vector.

SSSpherical::SSSpherical ( SSVector vec )
{
    rad = vec.magnitude();
    lat = SSAngle ( asin ( vec.z / rad ) );
    lon = SSAngle ( atan2 ( vec.y, vec.x ) ).mod2Pi();
}

// Converts spherical coordinates to rectanguler x,y,z position vector.
// Inputs are this spherical coordinate object whose (lon,lat) are in radians
// are radial distance from the origin (rad) is in arbitrary distance units.
// Returns x,y,z position vector in same units as input radial distance.
// Based on formulae from http://www.astrosurf.com/jephem/library/li110spherCart_en.htm

SSVector SSSpherical::toVectorPosition ( void )
{
    double x = rad * cos ( lat ) * cos ( lon );
    double y = rad * cos ( lat ) * sin ( lon );
    double z = rad * sin ( lat );
    
    return SSVector ( x, y, z );
}

// Converts spherical position and velocity to rectanguler position and velocity vectors.
// Inputs are spherical coordinates (this object) and velocity (vsph) whose (lon,lat)
// are motion in radians/time unit and whose (rad) is radial velocity in distance/time unit.
// Returns x,y,z velocity vector in same distance/time unit as input radial velocity.
// Based on formulae from http://www.astrosurf.com/jephem/library/li110spherCart_en.htm
// (although note sign errors in formulae on that page) and "Sky Catalog 2000.0" vol. 1.

SSVector SSSpherical::toVectorVelocity ( SSSpherical vel )
{
    double coslon = cos ( lon );
    double sinlon = sin ( lon );
    double coslat = cos ( lat );
    double sinlat = sin ( lat );

    double x = rad * coslat * coslon;
    double y = rad * coslat * sinlon;
    double z = rad * sinlat;
    
    double vx = vel.rad * x / rad - y * vel.lon - z * vel.lat * coslon;
    double vy = vel.rad * y / rad + x * vel.lon - z * vel.lat * sinlon;
    double vz = vel.rad * z / rad + rad * vel.lat * coslat;
    
    return SSVector ( vx, vy, vz );
}

// Returns angular separation in radians from this point in a spherical coordinate system
// to another point in the same spherical coordinate system. This hversine formula, from
// https://en.wikipedia.org/wiki/Haversine_formula
// is accurate for all angles from 0 to kPi radians.
// We ignores both points' radial distances from the origin.

double haversin ( double x )
{
    double s = sin ( x * 0.5 );
    return ( s * s );
}

SSAngle SSSpherical::angularSeparation ( SSSpherical other )
{
    double s = haversin ( other.lat - lat ) + cos ( lat ) * cos ( other.lat ) * haversin ( other.lon - lon );
    s = s < 0.0 ? 0.0 : s > 1.0 ? 1.0 : s;
    return SSAngle ( 2.0 * asin ( sqrt ( s ) ) );
}

// Returns position angle in radians from this point in a spherical coordinate system
// to another point in the same spherical coordinate system.

SSAngle SSSpherical::positionAngle ( SSSpherical other )
{
    double eta = cos ( other.lat ) * sin ( other.lon - lon );
    double xi = cos ( lat ) * sin ( other.lat ) - sin ( lat ) * cos ( other.lat ) * cos ( other.lon - lon );
    return SSAngle ( atan2pi ( eta, xi ) );
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
    x = sph.rad * cos ( sph.lat ) * cos ( sph.lon );
    y = sph.rad * cos ( sph.lat ) * sin ( sph.lon );
    z = sph.rad * sin ( sph.lat );
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

// Returns the angular separation in radians from this vector in a rectangular coordinate system
// to another vector (v) in the same rectangular system, as seen from the origin of the coordinate system.
// Both vectors must be unit vectors. Formula is accurate for all angles from 0 to kPi radians.

SSAngle SSVector::angularSeparation ( SSVector v )
{
    double d = subtract ( v ).magnitude();
    return SSAngle ( 2.0 * asin ( d / 2.0 ) );
}

// Returns the position in radians from this vector in a rectangular coordinate system
// to another vector (v) in the same rectangular system, as seen from the origin of the coordinate system.
// Both vectors must be unit vectors. Formula is accurate for all position angles from 0 to kTwoPi radians.
// Position angle is measured eastward (counterclockwise) from north:
// north = 0, east = kHalfPi, south = kPi, west = kThreeHalvesPi.

SSAngle SSVector::positionAngle ( SSVector v )
{
    double nz = sqrt ( 1.0 - z * z );
    if ( nz == 0.0 )
        return ( 0.0 );

    double nx = -x * z / nz;
    double ny = -y * z / nz;

    double ex = -y / nz;
    double ey =  x / nz;

    double edotv = ex * v.x + ey * v.y;
    double ndotv = nx * v.x + ny * v.y + nz * v.z;

    double pa = ( edotv == 0.0 && ndotv == 0.0 ) ? 0.0 : atan2pi ( edotv, ndotv );
    return SSAngle ( pa );
}

// Returns the distance from this point in a rectangular coordinate system
// to another point in the same rectangular system, in the same arbitrary
// units that X,Y,Z coordinates of both systems are measured in.

double SSVector::distance ( SSVector other )
{
    return subtract ( other ).magnitude();
}

// Rotates vector counterclockwise around an axis unit vector (u) by an
// angle in radians (a). Does not modify this vector; returns rotated copy!

SSVector SSVector::rotate ( SSVector u, SSAngle a )
{
    double sina = sin ( a );
    double cosa = cos ( a );
    double omca = 1.0 - cosa;
    SSVector r;
    
    r.x = x * ( u.x * u.x * omca + cosa )
        + y * ( u.x * u.y * omca - u.z * sina )
        + z * ( u.x * u.z * omca + u.y * sina );
    
    r.y = x * ( u.x * u.y * omca + u.z * sina )
        + y * ( u.y * u.y * omca + cosa )
        + z * ( u.y * u.z * omca - u.x * sina );
    
    r.z = x * ( u.x * u.z * omca - u.y * sina )
        + y * ( u.y * u.z * omca + u.x * sina )
        + z * ( u.z * u.z * omca + cosa );
    
    return r;
}

// Converts this rectangular vectors to spherical coordinates (lon,lat,rad).
// Returns coordinates (lon,lat) in radians and radial distance in same
// unit as input x,y,z vector.

SSSpherical SSVector::toSpherical ( void )
{
    double r = magnitude();
    double lat = asin ( z / r );
    double lon = atan2 ( y, x );
    
    return SSSpherical ( SSAngle ( lon ).mod2Pi(), SSAngle ( lat ), r );
}

// Converts rectangular position and velocity vectors to spherical position and velocity.
// Inputs are position (this vector) and velocity (vvec) as x,y,z vectors.
// Returns spherical velocity whose (lon,lat) are in radians per time unit
// and whose rad is radial distance per time unit; Distance & time units
// are the same in returned spherical coordinates as for input x,y,z vectors.
// Based on formulae from http://www.astrosurf.com/jephem/library/li110spherCart_en.htm
// although note sign errors in formulae on that page!

SSSpherical SSVector::toSphericalVelocity ( SSVector vvec )
{
    double r = magnitude();
    if ( r == 0 || ( x == 0.0 && y == 0.0 ) )
    {
        return SSSpherical ( 0.0, 0.0, 0.0 );
    }
    else
    {
        double x2 = x * x;
        double y2 = y * y;
        double vx = vvec.x;
        double vy = vvec.y;
        double vz = vvec.z;

        double vrad = ( x * vx + y * vy + z * vz ) / r;
        double vlon = ( x * vy - y * vx ) / ( x2 + y2 );
        double vlat = ( r * vz - z * vrad ) / ( sqrt ( x2 + y2 ) * r );

        return SSSpherical ( vlon, vlat, vrad );
    }
}

