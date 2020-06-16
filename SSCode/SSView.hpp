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
    kGnomonic = 1,
    kOrthographic = 2,
    kStereographic = 3,
    kEquidistant = 4,
    kMercator = 5,
    kElliptical = 6,
    kSinusoidal = 7
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
    SSAngle         _widthAngle;              // angular width of field of view [radians]
    SSAngle         _heightAngle;             // angular height of field of view [radians]
    float           _centerX, _centerY;       // coordinates on 2D field of view which correspond to field of view center
    float           _left, _top;              // origin of 2D bounding rectangle of field of view
    float           _width, _height;          // dimensions of 2D bounding rectangle of field of view
    float           _scaleX, _scaleY;         // radians per pixel at field of view center in horizontal and vertical direction.
    
public:
    
    SSView ( SSProjection projection, float left, float top, float width, float height, SSAngle angle );
    
    SSProjection getProjection ( void ) { return _projection; }
    void setProjection ( SSProjection proj );
    
    void setBounds ( float left, float top, float width, float height );
    float getLeft ( void ) { return _left; }
    float getTop ( void ) { return _top; }
    float getWidth ( void ) { return _width; }
    float getHeight ( void ) { return _height; }

    void setScale ( float scaleX, float scaleY ) { _scaleX = scaleX; _scaleY = scaleY; }
    float getScaleX ( void ) { return _scaleX; }
    float getScaleY ( void ) { return _scaleY; }
    
    void setAngle ( SSAngle angle );
    void setWidthAngle ( SSAngle angle );
    void setHeightAngle ( SSAngle angle );
    
    SSAngle maxWidthAngle ( void );
    SSAngle maxHeightAngle ( void );

    SSAngle getWidthAngle ( void );
    SSAngle getHeightAngle ( void );
    SSAngle getDiagonalAngle ( void );
    
    void setCenterMatrix ( SSMatrix matrix );
    SSMatrix getCenterMatrix ( void ) { return _matrix; }

    void setCenter ( SSAngle lon, SSAngle lat, SSAngle rot );
    SSAngle getCenterLongitude ( void ) { return _centerLon; }
    SSAngle getCenterLatitude ( void ) { return _centerLat; }
    SSAngle getCenterRotation ( void ) { return _centerRot; }

    void setCenter ( float x, float y );
    void getCenter ( float &x, float &y ) { x = _centerX; y = _centerY; }
    
    SSVector project ( SSVector cvec );
    SSVector unproject ( SSVector vvec );
};

#endif /* SSView_hpp */
