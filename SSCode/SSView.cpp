// SSView.cpp
// SSTest
//
// Created by Tim DeBenedictis on 6/14/20.
// Copyright © 2020 Southern Stars. All rights reserved.

#include "SSView.hpp"

SSView::SSView ( SSProjection projection, float left, float top, float width, float height, SSAngle angle )
{
    _projection = projection;
    setAngle ( angle );
    setBounds ( left, top, width, height );
    setCenter ( 0.0, 0.0, 0.0 );
}

void SSView::setProjection ( SSProjection projection )
{
    SSAngle angle = getWidthAngle();
    _projection = projection;
    setWidthAngle ( angle );
}

void SSView::setBounds ( float left, float top, float width, float height )
{
    SSAngle angle = getWidthAngle();
    
    _left = left;
    _top = top;
    _width = width;
    _height = height;
    _centerX = left + width / 2.0;
    _centerY = top + height / 2.0;
    
    setWidthAngle ( angle );
}

void SSView::setCenter ( SSAngle lon, SSAngle lat, SSAngle rot )
{
    _centerLon = lon;
    _centerLat = lat;
    _centerRot = rot;
    
    _matrix = SSMatrix::rotation ( 3, 2, -lon, 1, -lat, 0, rot );
}

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

void SSView::setAngle ( SSAngle angle )
{
    if ( _width < _height )
        setWidthAngle ( angle );
    else
        setHeightAngle ( angle );
}

SSAngle SSView::maxWidthAngle ( void )
{
    if ( _projection == kGnomonic )
        return SSAngle::fromDegrees ( 120.0 );
    else if ( _projection == kOrthographic )
        return SSAngle::fromDegrees ( 180.0 );
    else if ( _projection == kStereographic )
        return SSAngle::fromDegrees ( 270.0 );
    else // ( _projection == kMercator || _projection == kEquidistant || _projection == kElliptical || _projection == kSinusoidal )
        return SSAngle::fromDegrees ( 360.0 );
}

SSAngle SSView::maxHeightAngle ( void )
{
    if ( _projection == kGnomonic || _projection == kMercator )
        return SSAngle::fromDegrees ( 120.0 );
    else if ( _projection == kOrthographic )
        return SSAngle::fromDegrees ( 180.0 );
    else if ( _projection == kStereographic )
        return SSAngle::fromDegrees ( 270.0 );
    else // ( _projection == kEquidistant || _projection == kElliptical || _projection == kSinusoidal )
        return SSAngle::fromDegrees ( 180.0 );
}

void SSView::setWidthAngle ( SSAngle angle )
{
    if ( angle > maxWidthAngle() )
        angle = maxWidthAngle();
    
    if ( _projection == kGnomonic )
    {
        _scaleX = _scaleY = tan ( angle / 2.0 ) / ( _width / 2.0 );
    }
    else if ( _projection == kOrthographic )
    {
        _scaleX = _scaleY = sin ( angle / 2.0 ) / ( _width / 2.0 );
    }
    else if ( _projection == kStereographic )
    {
        _scaleX = _scaleY = tan ( angle / 4.0 ) / ( _width / 2.0 );
    }
    else // ( _projection == kMercator || _projection == kEquidistant || _projection == kElliptical || _projection == kSinusoidal )
    {
        _scaleX = _scaleY = (double) angle / _width;
    }
}

void SSView::setHeightAngle ( SSAngle angle )
{
    if ( angle > maxHeightAngle() )
        angle = maxHeightAngle();

    if ( _projection == kGnomonic )
    {
        _scaleX = _scaleY = tan ( angle / 2.0 ) / ( _height / 2.0 );
    }
    else if ( _projection == kOrthographic )
    {
        _scaleX = _scaleY = sin ( angle / 2.0 ) / ( _height / 2.0 );
    }
    else if ( _projection == kStereographic )
    {
        _scaleX = _scaleY = tan ( angle / 4.0 ) / ( _height / 2.0 );
    }
    else if ( _projection == kMercator )
    {
        _scaleX = _scaleY = tan ( angle / 2.0 ) / ( _height / 2.0 );
    }
    else if ( _projection == kElliptical )
    {
        _scaleX = _scaleY = M_PI_2 * (double) angle / _height;
    }
    else  // ( _projection == kEquidistant || _projection == kSinusoidal )
    {
        _scaleX = _scaleY = (double) angle / _height;
    }
}

SSAngle SSView::getWidthAngle ( void )
{
    if ( _projection == kGnomonic )
    {
        return SSAngle ( 2.0 * atan ( fabs ( _scaleX ) * _width / 2.0 ) );
    }
    else if ( _projection == kOrthographic )
    {
        double x = fabs ( _scaleX ) * _width / 2.0;
        return SSAngle ( 2.0 * asin ( minimum ( x, 1.0 ) ) );
    }
    else if ( _projection == kStereographic )
    {
        return SSAngle ( 4.0 * atan ( fabs ( _scaleX ) * _width / 2.0 ) );
    }
    else // ( _projection == kEquidistant || _projection == kMercator || _projection == kElliptical || _projection == kSinusoidal )
    {
        return SSAngle ( fabs ( _scaleX ) * _width );
    }
}

SSAngle SSView::getHeightAngle ( void )
{
    if ( _projection == kGnomonic )
    {
        return SSAngle ( 2.0 * atan ( fabs ( _scaleY ) * _height / 2.0 ) );
    }
    else if ( _projection == kOrthographic )
    {
        double y = fabs ( _scaleY ) * _height / 2.0;
        return SSAngle ( 2.0 * asin ( minimum ( y, 1.0 ) ) );
    }
    else if ( _projection == kStereographic )
    {
        return SSAngle ( 4.0 * atan ( fabs ( _scaleY ) * _height / 2.0 ) );
    }
    else if ( _projection == kMercator )
    {
        return SSAngle ( 2.0 * atan ( fabs ( _scaleY ) * _height ) );
    }
    else if ( _projection == kElliptical )
    {
        return SSAngle ( fabs ( _scaleY ) * _height / M_PI_2 );
    }
    else // ( _projection == kEquidistant || _projection == kSinusoidal )
    {
        return SSAngle ( fabs ( _scaleY ) * _height );
    }
}

SSAngle SSView::getDiagonalAngle ( void )
{
    SSAngle angle;
    
    SSVector cvec = unproject ( SSVector ( _left, _top, 0.0 ) );
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
    else if ( _projection == kEquidistant )
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
    else if ( _projection == kElliptical )
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
    else if ( _projection == kEquidistant )
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
    else if ( _projection == kElliptical )
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
