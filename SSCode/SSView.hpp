// SSView.hpp
// SSCore
//
// Created by Tim DeBenedictis on 6/14/20.
// Copyright © 2020 Southern Stars. All rights reserved.

#ifndef SSView_hpp
#define SSView_hpp

#include "SSMatrix.hpp"

// system of mathematical equations used for 3D spherical <-> 2D map projection conversion

enum SSProjection
{
    kGnomonic = 1,          // https://en.wikipedia.org/wiki/Gnomonic_projection
    kOrthographic = 2,      // https://en.wikipedia.org/wiki/Orthographic_projection
    kStereographic = 3,     // https://en.wikipedia.org/wiki/Stereographic_projection
    kEquirectangular = 4,   // https://en.wikipedia.org/wiki/Equirectangular_projection
    kMercator = 5,          // https://en.wikipedia.org/wiki/Mercator_projection
    kMollweide = 6,         // https://en.wikipedia.org/wiki/Mollweide_projection
    kSinusoidal = 7         // https://en.wikipedia.org/wiki/Sinusoidal_projection
};

// Represents a rectangular field of view of a part of the celestial sphere, and converts
// converts 3D positions on the celestrial sphere to 2D coordinates in the field of view,
// and vice versa.

class SSView
{
protected:
    
    SSProjection    _projection;              // system of mathematical equations used for 3D spherical <-> 2D map projection conversion
    SSMatrix        _matrix;                  // rotation matrix for transforming from celestial reference frame to view reference frame
    SSAngle         _centerLon, _centerLat;   // longitude, latitude, of field of view center in 3D celestial sphere reference frame
    SSAngle         _centerRot;               // rotation angle at field of view center in celestial sphere 3D reference frame
    float           _centerX, _centerY;       // coordinates on 2D field of view which correspond to field of view center
    float           _scaleX, _scaleY;         // radians per pixel at field of view center in horizontal and vertical direction; if ngative, view is inverted horizontally/vertically
    float           _width, _height;          // dimensions of bounding rectangle of 2D field of view; if negative, inverts view horizontally/vertically

public:
    
    // constructors
    
    SSView ( void );
    SSView ( SSProjection projection, SSAngle widthAngle, float width, float height, float centerX, float centerY );
    
    // changes projection; attempts to preserve angular field of view width
    
    void setProjection ( SSProjection proj );
    SSProjection getProjection ( void ) { return _projection; }
    
    // changes dimensions; attempts to preserve angular field of view width

    void setDimensions ( float width, float height );
    float getWidth ( void ) { return _width; }
    float getHeight ( void ) { return _height; }

    // changes center of rectangular field of view without changing dimensions, scale, etc.

    void setCenter ( float centerX, float centerY ) { _centerX = centerX; _centerY = centerY; }
    float getCenterX ( void ) { return _centerX; }
    float getCenterY ( void ) { return _centerY; }

    // changing scale will also change angular field of view width x height
    
    void setScale ( float scaleX, float scaleY ) { _scaleX = scaleX; _scaleY = scaleY; }
    float getScaleX ( void ) { return _scaleX; }
    float getScaleY ( void ) { return _scaleY; }

    // maximum angular field of view width x height allowed in current projection

    SSAngle maxAngularWidth ( void );
    SSAngle maxAngularHeight ( void );

    // sets angular field of view width x height; will also change scale
    
    void setAngularWidth ( SSAngle angle );
    void setAngularHeight ( SSAngle angle );

    // returns current angular width, height, diagonal field of view
    
    SSAngle getAngularWidth ( void );
    SSAngle getAngularHeight ( void );
    SSAngle getAngularDiagonal ( void );
    
    // sets celestial spherical coordinates of field of view center
    
    void setCenter ( SSAngle lon, SSAngle lat, SSAngle rot );
    SSAngle getCenterLongitude ( void ) { return _centerLon; }
    SSAngle getCenterLatitude ( void ) { return _centerLat; }
    SSAngle getCenterRotation ( void ) { return _centerRot; }
    SSVector getCenterVector ( void );
    
    // sets rotation matrix corresponding to field of view center
    
    void setCenterMatrix ( SSMatrix matrix );
    SSMatrix getCenterMatrix ( void ) { return _matrix; }

    // projects point from celestial sphere onto rectangular field of view, and vice-versa
    
    SSVector project ( SSVector cvec );
    SSVector unproject ( SSVector vvec );
    
    // gets 2D bounding rectangle; tests whether point is within view's 2D bounding rectangle

    float getLeft ( void ) { return _centerX - fabs ( _width ) / 2.0; }
    float getTop ( void ) { return _centerY - fabs ( _height ) / 2.0; }
    float getRight ( void ) { return _centerX + fabs ( _width ) / 2.0; }
    float getBottom ( void ) { return _centerY + fabs ( _height ) / 2.0; }
    bool inBoundRect ( float x, float y );

    // converts horizontal/vertical distance from chart center in radians to pixels, and vice-versa
    
    float radiansToPixelsX ( SSAngle radians );
    float radiansToPixelsY ( SSAngle radians );
    
    SSAngle pixelsToRadiansX ( float pixels );
    SSAngle pixelsToRadiansY ( float pixels );

    // clips line defined by two endpoints to view's 2D bounding rectangle
    
    bool clipLine ( SSVector &v0, SSVector &v1 );
    bool lineWrap ( SSVector &v0, SSVector &v1 );
    float wrapX ( float x );
};

#endif /* SSView_hpp */
