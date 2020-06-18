// SSView.cpp
// SSTest
//
// Created by Tim DeBenedictis on 6/14/20.
// Copyright © 2020 Southern Stars. All rights reserved.

#include "SSView.hpp"

// Default constructor. Creates SSView with Gnonomic projection, and 90-degree field of view
// spanning 640x480 rectangle centered at (320,240), looking toward celestial coordiantes (0,0).

SSView::SSView ( void )
{
    _projection = kGnomonic;
    _centerX = 320;
    _centerY = 240;
    _width = 640;
    _height = 480;

    setAngularWidth ( SSAngle::kHalfPi );
    setCenter ( 0.0, 0.0, 0.0 );
}

// Constructor with projection, field-of-view width angle, bounding rectangle dimensions and center.
// Celestial coordinates of field-of-view center will be looking toward (0,0).

SSView::SSView ( SSProjection projection, SSAngle angle, float width, float height, float centerX, float centerY )
{
    _projection = projection;
    _centerX = centerX;
    _centerY = centerY;
    _width = width;
    _height = height;
    
    setAngularWidth ( angle );
    setCenter ( 0.0, 0.0, 0.0 );
}

// Changes projection. Attempts to preserve field-of-view width angle,
// which will change scale (but nothing else).

void SSView::setProjection ( SSProjection projection )
{
    if ( projection != _projection )
    {
        SSAngle angle = getAngularWidth();
        _projection = projection;
        setAngularWidth ( angle );
    }
}

// Changes dimensions of bounding rectangle of 2D field of view.
// Attempts to preserve field-of-view width angle,
// which will change scale (but nothing else).

void SSView::setDimensions ( float width, float height )
{
    if ( width != _width || height != _height )
    {
        SSAngle angle = getAngularWidth();
        _width = width;
        _height = height;
        setAngularWidth ( angle );
    }
}

// Sets spherical coordinates of the field of view center,
// i.e. the celestial coordinates that we are looking at.
// Recalculates celestial-to-view rotation matrix.

void SSView::setCenter ( SSAngle lon, SSAngle lat, SSAngle rot )
{
    _centerLon = lon;
    _centerLat = lat;
    _centerRot = rot;
    
    _matrix = SSMatrix::rotation ( 3, 2, -lon, 1, -lat, 0, rot );
}

// Sets celestial-to-view rotation matrix, and recalvulates spherical
// oordinates of the field of view center, i.e. the celestial coordinates
// that we are looking at.

void SSView::setCenterMatrix ( SSMatrix matrix )
{
    _matrix = matrix;
    
    SSVector v0 ( matrix.m00, matrix.m01, matrix.m02 );
    SSVector v2 ( matrix.m20, matrix.m21, matrix.m22 );
    
    SSSpherical cen ( v0 );
    
    _centerLon = cen.lon;
    _centerLat = cen.lat;
    _centerRot = v0.positionAngle ( v2 );
}

// Returns maximum allowable field-of-view width angle in radians
// for the current view projection.

SSAngle SSView::maxAngularWidth ( void )
{
    if ( _projection == kGnomonic )
        return SSAngle::fromDegrees ( 120.0 );
    else if ( _projection == kOrthographic )
        return SSAngle::fromDegrees ( 180.0 );
    else if ( _projection == kStereographic )
        return SSAngle::fromDegrees ( 270.0 );
    else // ( _projection == kMercator || _projection == kEquirectangular || _projection == kMollweide || _projection == kSinusoidal )
        return SSAngle::fromDegrees ( 360.0 );
}

// Returns maximum allowable field-of-view height angle in radians
// for the current view projection.

SSAngle SSView::maxAngularHeight ( void )
{
    if ( _projection == kGnomonic || _projection == kMercator )
        return SSAngle::fromDegrees ( 120.0 );
    else if ( _projection == kOrthographic )
        return SSAngle::fromDegrees ( 180.0 );
    else if ( _projection == kStereographic )
        return SSAngle::fromDegrees ( 270.0 );
    else // ( _projection == kEquirectangular || _projection == kMollweide || _projection == kSinusoidal )
        return SSAngle::fromDegrees ( 180.0 );
}

// Sets angular width of field of view (in radians),
// and reculates the horizontal and vertical scale.

void SSView::setAngularWidth ( SSAngle angle )
{
    if ( angle > maxAngularWidth() )
        angle = maxAngularWidth();
    
    if ( _projection == kGnomonic )
        _scaleX = _scaleY = tan ( angle / 2.0 ) / ( _width / 2.0 );
    else if ( _projection == kOrthographic )
        _scaleX = _scaleY = sin ( angle / 2.0 ) / ( _width / 2.0 );
    else if ( _projection == kStereographic )
        _scaleX = _scaleY = tan ( angle / 4.0 ) / ( _width / 2.0 );
    else // ( _projection == kMercator || _projection == kEquirectangular || _projection == kMollweide || _projection == kSinusoidal )
        _scaleX = _scaleY = (double) angle / _width;
}

// Sets angular height of field of view (in radians),
// and reculates the horizontal and vertical scale.

void SSView::setAngularHeight ( SSAngle angle )
{
    if ( angle > maxAngularHeight() )
        angle = maxAngularHeight();

    if ( _projection == kGnomonic )
        _scaleX = _scaleY = tan ( angle / 2.0 ) / ( _height / 2.0 );
    else if ( _projection == kOrthographic )
        _scaleX = _scaleY = sin ( angle / 2.0 ) / ( _height / 2.0 );
    else if ( _projection == kStereographic )
        _scaleX = _scaleY = tan ( angle / 4.0 ) / ( _height / 2.0 );
    else if ( _projection == kMercator )
        _scaleX = _scaleY = tan ( angle / 2.0 ) / ( _height / 2.0 );
    else if ( _projection == kMollweide )
        _scaleX = _scaleY = M_PI_2 * (double) angle / _height;
    else  // ( _projection == kEquirectangular || _projection == kSinusoidal )
        _scaleX = _scaleY = (double) angle / _height;
}

// Returns angular width of fields of view (in radians)
// from the view's horizontal image scale and width.

SSAngle SSView::getAngularWidth ( void )
{
    if ( _projection == kGnomonic )
    {
        return SSAngle ( 2.0 * atan ( fabs ( _scaleX ) * _width / 2.0 ) );
    }
    else if ( _projection == kOrthographic )
    {
        return SSAngle ( 2.0 * asin ( minimum ( fabs ( _scaleX ) * _width / 2.0, 1.0 ) ) );
    }
    else if ( _projection == kStereographic )
    {
        return SSAngle ( 4.0 * atan ( fabs ( _scaleX ) * _width / 2.0 ) );
    }
    else // ( _projection == kEquirectangular || _projection == kMercator || _projection == kMollweide || _projection == kSinusoidal )
    {
        return SSAngle ( minimum ( fabs ( _scaleX ) * _width, SSAngle::kTwoPi ) );
    }
}

// Returns angular width of fields of view (in radians)
// from the view's horizontal image scale and width.

SSAngle SSView::getAngularHeight ( void )
{
    if ( _projection == kGnomonic )
    {
        return SSAngle ( 2.0 * atan ( fabs ( _scaleY ) * _height / 2.0 ) );
    }
    else if ( _projection == kOrthographic )
    {
        return SSAngle ( 2.0 * asin ( minimum ( fabs ( _scaleY ) * _height / 2.0, 1.0 ) ) );
    }
    else if ( _projection == kStereographic )
    {
        return SSAngle ( 4.0 * atan ( fabs ( _scaleY ) * _height / 2.0 ) );
    }
    else if ( _projection == kMercator )
    {
        return SSAngle ( 2.0 * atan ( fabs ( _scaleY ) * _height ) );
    }
    else if ( _projection == kMollweide )
    {
        return SSAngle ( minimum ( fabs ( _scaleY ) * _height / M_PI_2, SSAngle::kPi ) );
    }
    else // ( _projection == kEquirectangular || _projection == kSinusoidal )
    {
        return SSAngle ( minimum ( fabs ( _scaleY ) * _height, SSAngle::kPi ) );
    }
}

// Returns angular value in radians corresponding to diagonal across retangular
// field of view from (top,left) to (bottom,right).

SSAngle SSView::getAngularDiagonal ( void )
{
    SSAngle angle;
    
    SSVector cvec = unproject ( SSVector ( _centerX - _width / 2, _centerY + _height / 2, 0.0 ) );
    if ( cvec.magnitude() < INFINITY )
    {
        angle = 2.0 * cvec.angularSeparation ( SSVector ( _matrix.m00, _matrix.m01, _matrix.m02 ) );
        if ( angle > SSAngle::kTwoPi )
            angle = SSAngle::kTwoPi;
    }
    else
    {
        if ( _projection == kOrthographic )
            angle = SSAngle::kPi;
        else
            angle = SSAngle::kTwoPi;
    }
    
    return angle;
}

// Projects a vector representing a point on the 3D celestial sphere (cvec)
// to a point on the 2D field of view (x and y fields of the returned vector).
// The z field in the returned vector is the depth coordinate: positive if
// the point on the celestial sphere is "in front of" the viewer, negative
// if the point is behind the viewer. The returned (x,y) may be infinite if
// the point (cvec) is located on part of the celestial sphere that cannot be
// projected onto the rectangular field of view for its current projection.

SSVector SSView::project ( SSVector cvec )
{
    cvec = _matrix * cvec;

    double x = cvec.x;
    double y = cvec.y;
    double z = cvec.z;
    
    SSVector vvec = cvec;

    if ( _projection == kGnomonic )
    {
        if ( cvec.x > 0 )
        {
            vvec.x = _centerX - ( y / x ) / _scaleX;
            vvec.y = _centerY - ( z / x ) / _scaleY;
        }
        else
        {
            vvec.x = cvec.y / _scaleX > 0.0 ? -INFINITY : INFINITY;
            vvec.y = cvec.z / _scaleX > 0.0 ? -INFINITY : INFINITY;
        }
    }
    else if ( _projection == kOrthographic )
    {
        if ( x > 0.0 )
        {
            vvec.x = _centerX - y / _scaleX;
            vvec.y = _centerY - z / _scaleY;
        }
        else
        {
            vvec.x = vvec.y = INFINITY;
        }
    }
    else if ( _projection == kStereographic )
    {
        if ( cvec.x > -0.9 )
        {
            vvec.x = _centerX - ( cvec.y / ( cvec.x + 1.0 ) ) / _scaleX;
            vvec.y = _centerY - ( cvec.z / ( cvec.x + 1.0 ) ) / _scaleY;
        }
        else
        {
            vvec.x = cvec.y / _scaleX > 0.0 ? -INFINITY : INFINITY;
            vvec.y = cvec.z / _scaleY > 0.0 ? -INFINITY : INFINITY;
        }
    }
    else if ( _projection == kEquirectangular )
    {
        vvec.x = _centerX - ( x ? atan2 ( y, x ) : y > 0 ? M_PI_2 : -M_PI_2 ) / _scaleX;
        vvec.y = _centerY - asin ( z ) / _scaleY;
    }
    else if ( _projection == kMercator )
    {
        double r = sqrt ( ( 1.0 - z ) * ( 1.0 + z ) );
        vvec.x = _centerX - ( x ? atan2 ( y, x ) : y > 0 ? M_PI_2 : -M_PI_2 ) / _scaleX;
        vvec.y = r ? _centerY - ( z / r ) / _scaleY : z > 0 ? - INFINITY : INFINITY;
    }
    else if ( _projection == kMollweide )
    {
        double a = x ? atan2 ( y, x ) : y > 0 ? M_PI_2 : -M_PI_2;
        double r = sqrt ( ( 1.0 - z ) * ( 1.0 + z ) );
        vvec.x = _centerX - a * ( r / _scaleX );
        vvec.y = _centerY - M_PI_2 * ( z / _scaleY );
    }
    else if ( _projection == kSinusoidal )
    {
        double a = x ? atan2 ( y, x ) : y > 0 ? M_PI_2 : -M_PI_2;
        double r = sqrt ( ( 1.0 - z ) * ( 1.0 + z ) );
        vvec.x = _centerX - ( a * r ) / _scaleX;
        vvec.y = _centerY - asin ( z ) / _scaleY;
    }
    
    return vvec;
}

// Projects a vector representing a point on the 2D field of view (vvec)
// to a point on the 3D celestial sphere (the returned vector).
// The z field in the input vector (vvec.z) is ignored.
// The returned vector may be infinite if the point (vvec.x,vvec.y) is
// located in part of the the rectangular field of view that cannot be
// projected onto the celestial sphere for the current projection.

SSVector SSView::unproject ( SSVector vvec )
{
    double x = INFINITY, y = INFINITY, z = INFINITY, a = 0.0, b = 0.0;
    SSVector cvec ( INFINITY, INFINITY, INFINITY );
    
    if ( _projection == kGnomonic )
    {
        y = ( _centerX - vvec.x ) * _scaleX;
        z = ( _centerY - vvec.y ) * _scaleY;
        x = sqrt ( 1.0 + y * y + z * z );
        y /= x;
        z /= x;
        x = 1.0 / x;
    }
    else if ( _projection == kOrthographic )
    {
        y = ( _centerX - vvec.x ) * _scaleX;
        z = ( _centerY - vvec.y ) * _scaleY;
        x = 1.0 - y * y - z * z;
        if ( x <= 0.0 )
            return ( cvec );
        
        x = sqrt ( x );
    }
    else if ( _projection == kStereographic )
    {
        y = ( _centerX - vvec.x ) * _scaleX;
        z = ( _centerY - vvec.y ) * _scaleY;
        x = ( 1.0 + y * y + z * z ) / 2.0;
        y /= x;
        z /= x;
        x = 1.0 / x - 1.0;
    }
    else if ( _projection == kEquirectangular )
    {
        a = ( _centerX - vvec.x ) * _scaleX;
        b = ( _centerY - vvec.y ) * _scaleY;
        if ( b > M_PI_2 || b < -M_PI_2 )
            return ( cvec );
        
        x = cos ( a ) * cos ( b );
        y = sin ( a ) * cos ( b );
        z = sin ( b );
    }
    else if ( _projection == kMercator )
    {
        a = ( _centerX - vvec.x ) * _scaleX;
        b = atan ( ( _centerY - vvec.y ) * _scaleY );
        x = cos ( a ) * cos ( b );
        y = sin ( a ) * cos ( b );
        z = sin ( b );
    }
    else if ( _projection == kMollweide )
    {
        b = ( _centerY - vvec.y ) * _scaleY / M_PI_2;
        if ( b > 1.0 || b < -1.0 )
            return ( cvec );
        
        b = asin ( b );
        a = ( _centerX - vvec.x ) * _scaleX / cos ( b );
        if ( a > M_PI || a < -M_PI )
            return ( cvec );
        
        x = cos ( a ) * cos ( b );
        y = sin ( a ) * cos ( b );
        z = sin ( b );
    }
    else if ( _projection == kSinusoidal )
    {
        b = ( _centerY - vvec.y ) * _scaleY;
        if ( b > M_PI_2 || b < -M_PI_2 )
            return ( cvec );
        
        a = ( _centerX - vvec.x ) * _scaleX / cos ( b );
        if ( a > M_PI || a < -M_PI )
            return ( cvec );
        
        x = cos ( a ) * cos ( b );
        y = sin ( a ) * cos ( b );
        z = sin ( b );
    }
    
    vvec = SSVector ( x, y, z );
    cvec = _matrix.transpose() * vvec;
    
    return cvec;
}
