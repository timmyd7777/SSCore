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
// If width is negative, view is inverted horizontally; if height is negative, view is inverted vertically.
// Celestial coordinates of field-of-view center will be looking toward (0,0).

SSView::SSView ( SSProjection projection, SSAngle angle, double width, double height, double centerX, double centerY )
{
    _projection = projection;
    _centerX = centerX;
    _centerY = centerY;
    _width = width;
    _height = height;
    
    setAngularWidth ( angle );
    setCenter ( 0.0, 0.0, 0.0 );
}

// Constructor with projection, field-of-view width and height angles, and pixel scale in radians per pixel.
// If width is negative, view is inverted horizontally; if height is negative, view is inverted vertically.
// Celestial coordinates of field-of-view center will be looking toward (0,0).

SSView::SSView ( SSProjection projection, SSAngle width, SSAngle height, SSAngle scale )
{
    _projection = projection;
    _width = width / scale;
    _height = height / scale;
    _centerX = _width / 2;
    _centerY = _height / 2;
    
    setAngularWidth ( width );
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
// If width is negative, view is inverted horizontally;
// if height is negative, view is inverted vertically.
// Attempts to preserve field-of-view width angle,
// which will change scale (but nothing else).

void SSView::setDimensions ( double width, double height )
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

// Sets celestial-to-view rotation matrix, and recalculates spherical
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

// Returns celestial coordinates of point at center of field of view
// as a unit (x,y,z) vector to that point on the celestial sphere.

SSVector SSView::getCenterVector ( void )
{
    return SSVector ( _matrix.m00, _matrix.m01, _matrix.m02 );
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

// Sets angular width of field of view (in radians, always positive),
// and recalulates the horizontal and vertical scale.

void SSView::setAngularWidth ( SSAngle angle )
{
    if ( angle == 0.0 || isinf ( angle ) || isnan ( angle ) )
        return;
    
    angle = fabs ( angle );
    if ( angle > maxAngularWidth() )
        angle = maxAngularWidth();
    
    float width = fabs ( _width );
    if ( _projection == kGnomonic )
        _scaleX = tan ( angle / 2.0 ) / ( width / 2.0 );
    else if ( _projection == kOrthographic )
        _scaleX = sin ( angle / 2.0 ) / ( width / 2.0 );
    else if ( _projection == kStereographic )
        _scaleX = tan ( angle / 4.0 ) / ( width / 2.0 );
    else // kMercator, kEquirectangular, kMollweide, kSinusoidal
        _scaleX = (double) angle / width;
    
    _scaleY = _height > 0 ? _scaleX : -_scaleX;
    _scaleX = _width > 0 ? _scaleX : -_scaleX;
}

// Sets angular height of field of view (in radians, always positive),
// and reculates the horizontal and vertical scale.

void SSView::setAngularHeight ( SSAngle angle )
{
    if ( angle == 0.0 || isinf ( angle ) || isnan ( angle ) )
        return;

    angle = fabs ( angle );
    if ( angle > maxAngularHeight() )
        angle = maxAngularHeight();

    float height = fabs ( _height );
    if ( _projection == kGnomonic )
        _scaleY = tan ( angle / 2.0 ) / ( height / 2.0 );
    else if ( _projection == kOrthographic )
        _scaleY = sin ( angle / 2.0 ) / ( height / 2.0 );
    else if ( _projection == kStereographic )
        _scaleY = tan ( angle / 4.0 ) / ( height / 2.0 );
    else if ( _projection == kMercator )
        _scaleY = tan ( angle / 2.0 ) / ( height / 2.0 );
    else if ( _projection == kMollweide )
        _scaleY = SSAngle::kHalfPi * (double) angle / height;
    else  // ( _projection == kEquirectangular || _projection == kSinusoidal )
        _scaleY = (double) angle / height;
    
    _scaleX = _width > 0 ? _scaleY : -_scaleY;
    _scaleY = _height > 0 ? _scaleY : -_scaleY;
}

// Returns angular width of fields of view (in radians, always positive)
// from the view's horizontal image scale and width.

SSAngle SSView::getAngularWidth ( void )
{
    if ( _projection == kGnomonic )
        return SSAngle ( 2.0 * atan ( _scaleX * _width / 2.0 ) );
    else if ( _projection == kOrthographic )
        return SSAngle ( 2.0 * asin ( min ( _scaleX * _width / 2.0, 1.0 ) ) );
    else if ( _projection == kStereographic )
        return SSAngle ( 4.0 * atan ( _scaleX * _width / 2.0 ) );
    else // ( _projection == kEquirectangular || _projection == kMercator || _projection == kMollweide || _projection == kSinusoidal )
        return SSAngle ( min ( _scaleX * _width, (double) SSAngle::kTwoPi ) );
}

// Returns angular width of fields of view (in radians, always positive)
// from the view's horizontal image scale and width.

SSAngle SSView::getAngularHeight ( void )
{
    if ( _projection == kGnomonic )
        return SSAngle ( 2.0 * atan ( _scaleY * _height / 2.0 ) );
    else if ( _projection == kOrthographic )
        return SSAngle ( 2.0 * asin ( min ( _scaleY * _height / 2.0, 1.0 ) ) );
    else if ( _projection == kStereographic )
        return SSAngle ( 4.0 * atan ( _scaleY * _height / 2.0 ) );
    else if ( _projection == kMercator )
        return SSAngle ( 2.0 * atan ( _scaleY * _height ) );
    else if ( _projection == kMollweide )
        return SSAngle ( min ( _scaleY * _height / SSAngle::kHalfPi, (double) SSAngle::kPi ) );
    else // ( _projection == kEquirectangular || _projection == kSinusoidal )
        return SSAngle ( min ( _scaleY * _height, (double) SSAngle::kPi ) );
}

// Returns angular value in radians (always positive) corresponding to diagonal
// across retangular field of view from (top,left) to (bottom,right).

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
    cvec = transform ( cvec );

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
        vvec.x = _centerX - ( x ? atan2 ( y, x ) : y > 0 ? SSAngle::kHalfPi : -SSAngle::kHalfPi ) / _scaleX;
        vvec.y = _centerY - asin ( z ) / _scaleY;
    }
    else if ( _projection == kMercator )
    {
        double r = sqrt ( ( 1.0 - z ) * ( 1.0 + z ) );
        vvec.x = _centerX - ( x ? atan2 ( y, x ) : y > 0 ? SSAngle::kHalfPi : -SSAngle::kHalfPi ) / _scaleX;
        vvec.y = r ? _centerY - ( z / r ) / _scaleY : z > 0 ? - INFINITY : INFINITY;
    }
    else if ( _projection == kMollweide )
    {
        double a = x ? atan2 ( y, x ) : y > 0 ? SSAngle::kHalfPi : -SSAngle::kHalfPi;
        double r = sqrt ( ( 1.0 - z ) * ( 1.0 + z ) );
        vvec.x = _centerX - a * ( r / _scaleX );
        vvec.y = _centerY - SSAngle::kHalfPi * ( z / _scaleY );
    }
    else if ( _projection == kSinusoidal )
    {
        double a = x ? atan2 ( y, x ) : y > 0 ? SSAngle::kHalfPi : -SSAngle::kHalfPi;
        double r = sqrt ( ( 1.0 - z ) * ( 1.0 + z ) );
        vvec.x = _centerX - ( a * r ) / _scaleX;
        vvec.y = _centerY - asin ( z ) / _scaleY;
    }
    
    vvec.z = x;
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
        if ( fabs ( a ) > SSAngle::kPi || fabs ( b ) > SSAngle::kHalfPi )
            return ( cvec );
        
        x = cos ( a ) * cos ( b );
        y = sin ( a ) * cos ( b );
        z = sin ( b );
    }
    else if ( _projection == kMercator )
    {
        a = ( _centerX - vvec.x ) * _scaleX;
        if ( fabs ( a ) > SSAngle::kPi )
            return ( cvec );

        b = atan ( ( _centerY - vvec.y ) * _scaleY );
        x = cos ( a ) * cos ( b );
        y = sin ( a ) * cos ( b );
        z = sin ( b );
    }
    else if ( _projection == kMollweide )
    {
        b = ( _centerY - vvec.y ) * _scaleY / SSAngle::kHalfPi;
        if ( fabs ( b ) > 1.0 )
            return ( cvec );
        
        b = asin ( b );
        a = ( _centerX - vvec.x ) * _scaleX / cos ( b );
        if ( fabs ( a ) > SSAngle::kPi )
            return ( cvec );
        
        x = cos ( a ) * cos ( b );
        y = sin ( a ) * cos ( b );
        z = sin ( b );
    }
    else if ( _projection == kSinusoidal )
    {
        b = ( _centerY - vvec.y ) * _scaleY;
        if ( fabs ( b ) > SSAngle::kHalfPi )
            return ( cvec );
        
        a = ( _centerX - vvec.x ) * _scaleX / cos ( b );
        if ( fabs ( a ) > SSAngle::kPi )
            return ( cvec );
        
        x = cos ( a ) * cos ( b );
        y = sin ( a ) * cos ( b );
        z = sin ( b );
    }
    
    vvec = SSVector ( x, y, z );
    cvec = untransform ( vvec );
    
    return cvec;
}

// tests whether point (x,y) is within view's 2D bounding rectangle

bool SSView::inBoundRect ( double x, double y )
{
    if ( x > getLeft() && x < getRight() && y > getTop() && y < getBottom() )
        return true;
    else
        return false;
}

// tests whether point (x,y) is within a circle centered at (xc,yc) with radius (r).

bool point_in_circle ( float x, float y, float xc, float yc, float r )
{
    float dx = x - xc;
    float dy = y - yc;
    
    return ( dx * dx + dy * dy < r * r );
}

// tests whether point (x,y) is within a rectangle bounded by (left,top) and (right,bottom).

bool point_in_rectangle ( float x, float y, float left, float top, float right, float bottom )
{
    return ( x > left && x < right && y > top && y < bottom );
}

// This "perpendicular dot product" method for determining whether a point (x,y) is
// inside a triangle with vertices (x1,y1), (x2,y2), (x3,y3) comes from:
// https://www.gamedev.net/forums/topic.asp?topic_id=295943

float perp_dot ( float x1, float y1, float x2, float y2, float x3, float y3 )
{
    return ( x1 - x3 ) * ( y2 - y3 ) - ( x2 - x3 ) * ( y1 - y3 );
}

bool point_in_triangle ( float x, float y, float x1, float y1, float x2, float y2, float x3, float y3 )
{
    bool b1 = perp_dot ( x, y, x1, y1, x2, y2 ) < 0.0;
    bool b2 = perp_dot ( x, y, x2, y2, x3, y3 ) < 0.0;
    bool b3 = perp_dot ( x, y, x3, y3, x1, y1 ) < 0.0;
    
    return ( b1 == b2 ) && ( b2 == b3 );
}

// This simplification of the Liang-Barsky line-clipping algorithm is adapted from
// https://stackoverflow.com/questions/11194876/clip-line-to-screen-coordinates
// Returns a boolean which indicates whether a line from (x0,y0) to (x1,y1)
// is inside or intersects the bounding rectangle defined by (left,top,right,bottom).

bool line_in_rectangle ( float x0, float y0, float x1, float y1, float left, float top, float right, float bottom )
{
    double t0 = 0.0, t1 = 1.0;
    double xdelta = x1 - x0;
    double ydelta = y1 - y0;
    double p = 0.0, q = 0.0, r = 0.0;

    // Traverse through left, right, bottom, top edges.
    
    for ( int edge = 0; edge < 4; edge++ )
    {
        if ( edge == 0 )
        {
            p = -xdelta;
            q = x0 - left;
        }
        
        if ( edge == 1 )
        {
            p = xdelta;
            q = right - x0;
        }
        
        if ( edge == 2 )
        {
            p = -ydelta;
            q = y0 - top;
        }
        
        if ( edge == 3 )
        {
            p = ydelta;
            q = bottom - y0;
        }
        
        r = q / p;
        
        if ( p == 0 && q < 0 )
            return false;

        if ( p < 0 )
        {
            if ( r > t1 )
                return false;
            else if ( r > t0 )
                t0 = r;
        }
        else if ( p > 0 )
        {
            if ( r < t0 )
                return false;
            else if ( r < t1 )
                t1 = r;
        }
    }

    return true;
}

bool rectangle_in_rectangle ( float xmin, float ymin, float xmax, float ymax, float left, float top, float right, float bottom )
{
    return ( xmax > left && xmin < right && ymax > top && ymin < bottom );
}

// tests whether rectangle bounded by (xmin,ymin), (xmax,ymax)
// intersects view's 2D bouning rectangle.

bool SSView::inBoundRect ( double xmin, double ymin, double xmax, double ymax )
{
    return ( xmax > getLeft() && xmin < getRight() && ymax > getTop() && ymin < getBottom() );
}

// tests whether circle centered at (x,y) with radius (r)
// intersects view's 2D bounding rectangle

bool SSView::inBoundRect ( double x, double y, double r )
{
    float xmin = getLeft();
    float ymin = getTop();
    float xmax = getRight();
    float ymax = getBottom();
    
    // no intersection if circle's bounding box outside view bounding rectangle
    
    if ( ! point_in_rectangle ( x, y, xmin - r, ymin - r, xmax + r, ymax + r ) )
        return false;
    
    // intersection if circle center inside bounding rectangle, +/- radius vertically or horizontally
    
    if ( point_in_rectangle ( x, y, xmin, ymin - r, xmax, ymax + r ) )
        return true;
    
    if ( point_in_rectangle ( x, y, xmin - r, ymin, xmax + r, ymax ) )
        return true;
    
    // intersection if any bounding rectangle corner inside circle

    if ( point_in_circle ( x, y, xmin, ymin, r ) )
        return true;
    
    if ( point_in_circle ( x, y, xmax, ymin, r ) )
        return true;

    if ( point_in_circle ( x, y, xmax, ymax, r ) )
        return true;

    if ( point_in_circle ( x, y, xmin, ymax, r ) )
        return true;
    
    return false;
}

// tests whether triangle with vertices (x1,y1), (x2,y2), (x3,y3)
// intersects view's 2D bounding rectangle.  For discussion, see:
// https://stackoverflow.com/questions/13790208/triangle-square-intersection-test-in-2d
// https://seblee.me/2009/05/super-fast-trianglerectangle-intersection-test/

bool SSView::inBoundRect ( double x1, double y1, double x2, double y2, double x3, double y3 )
{
    double l = getLeft();
    double t = getTop();
    double r = getRight();
    double b = getBottom();

    // No intersection if triangle's bounding box outside bounding rectangle.
    
    double xmin = min ( min ( x1, x2 ), x3 );
    double xmax = max ( max ( x1, x2 ), x3 );
    double ymin = min ( min ( y1, y2 ), y3 );
    double ymax = max ( max ( y1, y2 ), y3 );

    if ( ! rectangle_in_rectangle ( xmin, ymin, xmax, ymax, l, t, r, b ) )
        return false;

    // intersection if any triangle vertex inside bounding rectangle
    
    if ( point_in_rectangle ( x1, y1, l, t, r, b ) )
        return true;
    
    if ( point_in_rectangle ( x2, y2, l, t, r, b ) )
        return true;

    if ( point_in_rectangle ( x3, y3, l, t, r, b ) )
        return true;

    // intersection if any bounding rectangle corner inside triangle
    
    if ( point_in_triangle ( l, t, x1, y1, x2, y2, x3, y3 ) )
        return true;
    
    if ( point_in_triangle ( r, t, x1, y1, x2, y2, x3, y3 ) )
        return true;

    if ( point_in_triangle ( r, b, x1, y1, x2, y2, x3, y3 ) )
        return true;

    if ( point_in_triangle ( l, b, x1, y1, x2, y2, x3, y3 ) )
        return true;

    // intersection if any triangle edge intersects any bounding rectangle edge.

    if ( line_in_rectangle ( x1, y1, x2, y2, l, t, r, b ) )
        return true;
    
    if ( line_in_rectangle ( x2, y2, x3, y3, l, t, r, b ) )
        return true;

    if ( line_in_rectangle ( x1, y1, x3, y3, l, t, r, b ) )
        return true;

    return false;
}

// tests whether the view's bounding rectangle is entirely inside a circle
// centered at (xc, yc) with radius r.

bool SSView::inCircle ( double xc, double yc, double r )
{
    double left = getLeft();
    double top = getTop();
    double right = getRight();
    double bottom = getBottom();
    
    if ( ! point_in_circle ( left, top, xc, yc, r ) )
        return false;
    
    if ( ! point_in_circle ( right, top, xc, yc, r ) )
        return false;

    if ( ! point_in_circle ( right, bottom, xc, yc, r ) )
        return false;

    if ( ! point_in_circle ( left, bottom, xc, yc, r ) )
        return false;

    return true;
}

// Given a horizontal angular distance in radians from the view center,
// returns the corresponding horiztonal distance in pixels. If radians
// are negative, the returned value in pixels will also be hegative.

double SSView::radiansToPixelsX ( SSAngle radians )
{
    float scale = fabs ( _scaleX );

    if ( _projection == kGnomonic )
        return ( radians < SSAngle::kHalfPi ? tan ( radians ) / scale : INFINITY );
    else if ( _projection == kOrthographic )
        return ( ( radians < SSAngle::kHalfPi ? sin ( radians ) : 1.0 ) / scale );
    else if ( _projection == kStereographic )
        return ( radians < SSAngle::kPi ? tan ( radians / 2.0 ) / scale : INFINITY );
    else // kMercator, kElliptical, kEquirectangular, kSinusoidal:
        return ( radians < SSAngle::kPi ? (double) radians / scale : SSAngle::kPi / scale );
}

// Given a vertical angular distance in radians from the view center,
// returns the corresponding vertical distance in pixels. If radians
// are negative, the returned value in pixels will also be hegative.

double SSView::radiansToPixelsY ( SSAngle radians )
{
    float scale = fabs ( _scaleY );

    if ( _projection == kGnomonic || _projection == kMercator )
        return ( radians < SSAngle::kHalfPi ? tan ( radians ) / scale : INFINITY );
    else if ( _projection == kOrthographic )
        return ( ( radians < SSAngle::kHalfPi ? sin ( radians ) : 1.0 ) / scale );
    else if ( _projection == kStereographic )
        return ( radians < SSAngle::kPi ? tan ( radians / 2.0 ) / scale : INFINITY );
    else if ( _projection == kMollweide )
        return ( radians < SSAngle::kPi ? SSAngle::kHalfPi * sin ( radians ) / scale : INFINITY );
    else // kEquirectangular, kSinusoidal:
        return ( radians < SSAngle::kPi ? (double) radians / scale : SSAngle::kPi / scale );
}

// Given a horizontal distance in pixels from the view center,
// returns the corresponding horizontal angular value in radians.
// If pixels are negative, the returned radians are also hegative.

SSAngle SSView::pixelsToRadiansX ( double pixels )
{
    float scale = fabs ( _scaleX );    // horizontal scale is negative if chart is flipped
    
    if ( _projection == kGnomonic )
        return SSAngle ( atan ( pixels * scale ) );
    else if ( _projection == kOrthographic )
        return SSAngle ( asin ( clamp ( pixels * scale, -1.0, 1.0 ) ) );
    else if ( _projection == kStereographic )
        return SSAngle ( atan ( pixels * scale ) * 2.0 );
    else // kMercator, kElliptical, kEquidistant, kSinusoidal:
        return SSAngle ( min ( pixels * scale, (double) SSAngle::kPi ) );
}

// Given a vertical distance in pixels from the view center,
// returns the corresponding vertical angular value in radians.
// If pixels are negative, the returned radians are also hegative.

SSAngle SSView::pixelsToRadiansY ( double pixels )
{
    float scale = fabs ( _scaleY );    // horizontal scale is negative if chart is flipped
    
    if ( _projection == kGnomonic || _projection == kMercator )
        return SSAngle ( atan ( pixels * scale ) );
    else if ( _projection == kOrthographic )
        return SSAngle ( asin ( clamp ( pixels * scale, -1.0, 1.0 ) ) );
    else if ( _projection == kStereographic )
        return SSAngle ( atan ( pixels * scale ) * 2.0 );
    else if ( _projection == kMollweide )
        return SSAngle ( asin ( min ( pixels * scale / SSAngle::kHalfPi, 1.0 ) ) );
    else // kEquidistant, kSinusoidal:
        return SSAngle ( min ( pixels * scale, (double) SSAngle::kHalfPi ) );
}

// Returns angle in radians corresponding to north on celestial sphere at 2D view coordinates (x,y).
// Toward +X axis ("right" in view) is zero radians (0 degrees);
// Toward +Y axis ("down" in view) is i / 2 radians (90 degrees);
// Toward -X axis ("left" in view) is pi radians (180 degrees)
// Toward -Y axis ("up" in view) is 3 * pi / 2 radians (270 degrees)

SSAngle SSView::north ( double x, double y )
{
    SSSpherical coords = unproject ( SSVector ( x, y, 0.0 ) );

    coords.lat += SSAngle::fromDegrees ( 1.0 );
    if ( coords.lat > SSAngle::kHalfPi )
        coords.lat = SSAngle::kHalfPi;
    
    SSVector n = project ( coords );
    
    double dx = n.x - x;
    double dy = n.y - y;
    
    return SSAngle::atan2Pi ( _scaleX < 0 ? -dy : dy, _scaleX < 0 ? -dx : dx );
}

// This adaptation of the Liang-Barsky line-clipping algorithm is derived from
// https://stackoverflow.com/questions/11194876/clip-line-to-screen-coordinates
// Vectors v0 and v1 define the start and end points of the line; their z coordinates are ignored.
// On return, these vectors are modified to receive clipped endpoint coordinate values.
// Returns a boolean which indicates whether any part of the clipped line is inside
// the view's 2D bounding rectangle, i.e. whether it should be drawn at all!

bool SSView::clipLine ( SSVector &v0, SSVector &v1 )
{
    double edgeLeft = getLeft();
    double edgeTop = getTop();
    double edgeRight = getRight();
    double edgeBottom = getBottom();
    double t0 = 0.0, t1 = 1.0;
    double xdelta = v1.x - v0.x;
    double ydelta = v1.y - v0.y;
    double p = 0.0, q = 0.0, r = 0.0;

    if ( v0.isinf() || v1.isinf() )
        return false;

    // Traverse through left, right, bottom, top edges.
    
    for ( int edge = 0; edge < 4; edge++ )
    {
        if ( edge == 0 )
        {
            p = -xdelta;
            q = v0.x - edgeLeft;
        }
        
        if ( edge == 1 )
        {
            p = xdelta;
            q = edgeRight - v0.x;
        }
        
        if ( edge == 2 )
        {
            p = -ydelta;
            q = v0.y - edgeTop;
        }
        
        if ( edge == 3 )
        {
            p = ydelta;
            q = edgeBottom - v0.y;
        }
        
        r = q / p;
        
        if ( p == 0 && q < 0 )
            return false;          // Don't draw line at all. (parallel line outside bound rect)

        if ( p < 0 )
        {
            if ( r > t1 )
                return false;      // Don't draw line at all.
            else if ( r > t0 )
                t0 = r;            // Line is clipped!
        }
        else if ( p > 0 )
        {
            if ( r < t0 )
                return false;      // Don't draw line at all.
            else if ( r < t1 )
                t1 = r;            // Line is clipped!
        }
    }

    double x0clip = v0.x + t0 * xdelta;
    double y0clip = v0.y + t0 * ydelta;
    double x1clip = v0.x + t1 * xdelta;
    double y1clip = v0.y + t1 * ydelta;

    v0.x = x0clip;
    v0.y = y0clip;
    v1.x = x1clip;
    v1.y = y1clip;
    
    return true;        // should draw (clipped) line
}

bool SSView::lineWrap ( SSVector &v0, SSVector &v1 )
{
    if ( _projection >= kEquirectangular && ( v0.z < 0.0 || v1.z < 0.0 ) )
    {
        if ( v0.x > _centerX && v1.x < _centerX )
            return true;
        else if ( v0.x < _centerX && v1.x > _centerX )
            return true;
    }

    return false;
}

// Given three unit vectors (v1, v2, v3) which define a triangle on the celestial sphere,
// returns an integer (1, 2, 3) if any vector lies across the edges of the sky from the other two;
// or returns zero of all points in the triangle lie on the same edge of the sky.
// Nonzero values are only possible in 360-degree sky projections which unwrap
// the entire celestial sphere onto a flat plane (Equirectangular, Mercator, etc.)

int SSView::triangleWrap ( const SSVector &v1, const SSVector &v2, const SSVector &v3 )
{
    if ( _projection >= kEquirectangular && ( v1.z < 0.0 || v2.z < 0.0 || v3.z < 0.0 ) )
    {
        if ( v1.x < _centerX && v2.x > _centerX && v3.x > _centerX )
            return 1;
        if ( v1.x > _centerX && v2.x < _centerX && v3.x < _centerX )
            return 1;

        if ( v1.x < _centerX && v2.x > _centerX && v3.x < _centerX )
            return 2;
        if ( v1.x > _centerX && v2.x < _centerX && v3.x > _centerX )
            return 2;

        if ( v1.x > _centerX && v2.x > _centerX && v3.x < _centerX )
            return 3;
        if ( v1.x < _centerX && v2.x < _centerX && v3.x > _centerX )
            return 3;
    }

    return 0;
}

// Given a point (x,y), returns the x coordinate of that point reflected across
// the edge of the sky in 360-degree projections. If the current projection does
// not allow 360-degree fields of view, returned value equals input x.

double SSView::edgeReflect ( double x, double y )
{
    if ( _projection < kEquirectangular )
        return x;
    
    double left, right;
    edges ( y, left, right );
    if ( x > _centerX )
        x = left - ( right - x );
    else
        x = right + ( x - left );
    
    return x;
}

void SSView::edgeWrap ( SSVector &v0, SSVector &v1 )
{
    if ( _projection == kMercator || _projection == kEquirectangular )
    {
        v0.y = v1.y = ( v0.y + v1.y ) / 2;

        if ( v0.x > _centerX )
            v0.x = _centerX + SSAngle::kPi / _scaleX;

        if ( v0.x < _centerX )
            v0.x = _centerX - SSAngle::kPi / _scaleX;

        if ( v1.x > _centerX )
            v1.x = _centerX + SSAngle::kPi / _scaleX;

        if ( v1.x < _centerX )
            v1.x = _centerX - SSAngle::kPi / _scaleX;
    }
    else if ( _projection == kMollweide || _projection == kSinusoidal )
    {
        v0.y = v1.y = ( v0.y + v1.y ) / 2;
        double coslat = cos ( pixelsToRadiansY ( v0.y - _centerY ) );
        
        if ( v0.x > _centerX )
            v0.x = _centerX + coslat * SSAngle::kPi / _scaleX;

        if ( v0.x < _centerX )
            v0.x = _centerX - coslat * SSAngle::kPi / _scaleX;

        if ( v1.x > _centerX )
            v1.x = _centerX + coslat * SSAngle::kPi / _scaleX;

        if ( v1.x < _centerX )
            v1.x = _centerX - coslat * SSAngle::kPi / _scaleX;
    }
}

// Determines the bounding rectangle of the current sky projection.
// This may be larger or smaller than the view bounding rectangle!

void SSView::skyBounds ( double &left, double &top, double &right, double &bottom )
{
    double sx = fabs ( _scaleX );
    double sy = fabs ( _scaleY );
    
    if ( _projection == kGnomonic || _projection == kStereographic )
    {
        left = top = -INFINITY;
        right = bottom = INFINITY;
    }
    else if ( _projection == kOrthographic )
    {
        left = _centerX - 1.0 / sx;
        right = _centerX + 1.0 / sx;
        top = _centerY - 1.0 / sy;
        bottom = _centerY + 1.0 / sy;
    }
    else if ( _projection == kMercator )
    {
        left = _centerX - SSAngle::kPi / sx;
        right = _centerX + SSAngle::kPi / sx;
        top = -INFINITY;
        bottom = INFINITY;
    }
    else // if ( _projection == kEquirectangular || _projection == kMollweide || _projection == kSinusoidal )
    {
        left = _centerX - SSAngle::kPi / sx;
        right = _centerX + SSAngle::kPi / sx;
        top = _centerY - SSAngle::kHalfPi / sy;
        bottom = _centerY + SSAngle::kHalfPi / sy;
    }
}

// Determines the left and right edges of the sky area at a particular vertical coordinate (y).
// Given the current sky projection, these may be outside the left and right edges of the view!

void SSView::edges ( double y, double &xleft, double &xright )
{
    double sx = fabs ( _scaleX );
    double sy = fabs ( _scaleY );
    
    if ( _projection == kGnomonic || _projection == kStereographic )
    {
        xleft = -INFINITY;
        xright = INFINITY;
    }
    else if ( _projection == kOrthographic )
    {
        xleft = xright = _centerX;
        y = ( y - _centerY ) * sy;
        if ( fabs ( y ) < 1.0 )
        {
            double x = sqrt ( 1.0 - y * y );
            xleft = _centerX - x / sx;
            xright = _centerX + x / sx;
        }
    }
    else if ( _projection == kMercator || _projection == kEquirectangular )
    {
        xright = _centerX + SSAngle::kPi / sx;
        xleft  = _centerX - SSAngle::kPi / sx;
    }
    else if ( _projection == kMollweide || _projection == kSinusoidal )
    {
        xright = xleft = _centerX;
        double coslat = cos ( pixelsToRadiansY ( y - _centerY ) );
        if ( ! isnan ( coslat ) )
        {
            xright = _centerX + coslat * SSAngle::kPi / sx;
            xleft = _centerX - coslat * SSAngle::kPi / sx;
        }
    }
}

