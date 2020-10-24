// SSCoordinates.hpp
// SSCore
//
// Created by Tim DeBenedictis on 2/28/20.
// Copyright © 2020 Southern Stars. All rights reserved.
//
// This class converts rectangular and spherical coordinates between different astronomical reference frames
// at a particular time and geographic location. It also handles precession, nutation, aberration, refraction,
// and other coordinate-related issues.
// Includes new expressions for precession, valid for +/- 200,000 years from the present time,
// translated from Fortran code in "New precession expressions, valid for long time intervals",
// by J. Vondrak, N. Capitaine, and P. Wallace, Astronomy & Astrophysics 534, A22 (2011)

#ifndef SSCoordinates_hpp
#define SSCoordinates_hpp

#include <stdio.h>
#include "SSAngle.hpp"
#include "SSTime.hpp"
#include "SSMatrix.hpp"

// Identifiers for the principal astronomical reference frames.

enum SSFrame
{
    kFundamental = 0,   // ICRS: the mean equatorial reference frame at epoch J2000 (excludes nutation); X/Y plane is Earth's equatorial plane, +X toward vernal equinox, +Z toward north pole; spherical coords are RA/Dec
    kEquatorial = 1,    // equatorial frame at a specific epoch (including nutation); X/Y plane is Earth's equatorial plane, +X toward vernal equinox, +Z toward north pole; spherical coords are RA/Dec
    kEcliptic = 2,      // ecliptic frame at a specific epoch (includes nutation); X/Y plane is Earth's orbital plane; +X toward vernal equinox, +Z toward north ecliptic pole; spherical coords are ccliptic lon/lat
    kGalactic = 3,      // galactic frame; fixed relative to ICRS; X/Y plane is galactic equator; +X toward galactic center, +Z toward north galactic pole; spherical coords are galactic lon/lat
    kHorizon = 4,       // local horizon frame; X/Y plane is local horizon, +X is north, +Z is zenith; ; spherical coords are azimuth/altitude
};

// This class converts coordinates between the principal astronomical reference frames at a particular time and geographic location.
// It also handles precession, nutation, aberration, refraction, and other coordinate-related issues; and is used in ephemeris computation.

class SSCoordinates
{
protected:
    
    SSTime      _jd;             // Julian (Civil) Date, i.e. Julian Date in UTC, and local time zone in hours east of UTC
    double      _jed;            // Julian Ephemeris Date, i.e. Julian Date with Delta-T added (UTC to TDT)
    double      _lon;            // observer's longitude [radians, east positive]
    double      _lat;            // observer's latitude [radians, north positive]
    double      _alt;            // observer's altitude above geoid [kilometers]
    double      _lst;            // local apparent sidereal time [radians]
    double      _obq;            // mean obliquity of ecliptic at current epoch [radians]
    double      _de;             // nutation in obliquity [radians]
    double      _dl;             // nutation in longitude [radians]
    
    SSMatrix    _preMat;         // transforms from fundamental to mean precessed equatorial frame, not including nutation.
    SSMatrix    _nutMat;         // transforms from mean precessed equatorial frame to true equatorial frame, i.e. corrects for nutation.
    SSMatrix    _equMat;         // transforms from fundamental to current true equatorial frame.
    SSMatrix    _eclMat;         // transforms from fundamental to current true ecliptic frame (includes nutation).
    SSMatrix    _horMat;         // transforms from fundamental to current local horizon frame.
    SSMatrix    _galMat;         // transforms from fundamental to galactic frame

    SSVector    _obsPos;         // observer's heliocentric position in fundamental J2000 equatorial frame (ICRS) [AU]
    SSVector    _obsVel;         // observer's heliocentric velocity in fundamental J2000 equatorial frame (ICRS) [AU/day]

    bool        _starParallax;   // flag to apply helioecntric parallax when computing star apparent directions; default true.
    bool        _starMotion;     // flag to apply stellar space motion when computing star apparent directions; default true.
    bool        _aberration;     // flag to apply aberration of light when computing all object's apparent directions; default true.
    bool        _lighttime;      // flag to apply light time correction when computing solar system object's apparent directions; default true.

public:
    
    static constexpr double kKmPerAU = 149597870.700;                               // kilometers per Astronomical Unit (IAU 2012)
    static constexpr double kKmPerEarthRadii = 6378.137;                            // kilometers per equatorial Earth radius (WGS84)
    static constexpr double kEarthFlattening = 1 / 298.257;                         // Earth ellipsoid flattening factor (WGS84)
    static constexpr double kLightKmPerSec = 299792.458;                            // Speed of light in kilometers per second
    static constexpr double kLightAUPerDay = kLightKmPerSec * 86400.0 / kKmPerAU;   // Speed of lignt in astronomical units per day = 173.144
    static constexpr double kAUPerParsec = SSAngle::kArcsecPerRad;                  // Astronomical units per parsec = 206264.806247
    static constexpr double kParsecPerAU = 1.0 / kAUPerParsec;                      // Parsecs per astronomical unit
    static constexpr double kAUPerLY = kLightAUPerDay * 365.25;                     // Astronomical units per light year = 63241.077084 (assuming Julian year of exactly 365.25 days)
    static constexpr double kLYPerAU = 1.0 / kAUPerLY;                              // Light years per astronomical unit
    static constexpr double kLYPerParsec = kAUPerParsec / kAUPerLY;                 // Light years per parsec = 3.261563777179643
    static constexpr double kParsecPerLY = kAUPerLY / kAUPerParsec;                 // Parsecs per light year

    SSCoordinates ( SSTime time, SSSpherical location );
    
    void setTime ( SSTime time );
    void setLocation ( SSSpherical location );

    SSTime getTime ( void ) { return SSTime ( _jd ); }
    SSSpherical getLocation ( void ) { return SSSpherical ( _lon, _lat, _alt ); }
    double getJED ( void ) { return _jed; }
    double getLST ( void ) { return _lst; }
    
    SSVector getObserverPosition ( void ) { return _obsPos; }
    SSVector getObserverVelocity ( void ) { return _obsVel; }
    
    void setObserverPosition ( SSVector pos ) { _obsPos = pos; }
    void setObserverVelocity ( SSVector vel ) { _obsVel = vel; }

    bool getStarParallax ( void ) { return _starParallax; }
    bool getStarMotion ( void ) { return _starMotion; }
    bool getAberration ( void ) { return _aberration; }
    bool getLightTime ( void ) { return _lighttime; }
    
    void setStarParallax ( bool parallax ) { _starParallax = parallax; }
    void setStarMotion ( bool motion ) { _starMotion = motion; }
    void setAberration ( bool aberration ) { _aberration = aberration; }
    void setLightTime ( bool lighttime ) { _lighttime = lighttime; }
    
    static double getObliquity ( double jd );
    static void   getNutationConstants ( double jd, double &de, double &dl );
    static void   getPrecessionConstants ( double jd, double &zeta, double &z, double &theta );

    static SSMatrix getPrecessionMatrix ( double jd );
    static SSMatrix getNutationMatrix ( double obliquity, double nutLon, double nutObq );
    static SSMatrix getEclipticMatrix ( double obliquity );
    static SSMatrix getHorizonMatrix ( double lst, double lat );
    static SSMatrix getGalacticMatrix ( void );

    SSVector    transform ( SSFrame from, SSFrame to, SSVector vec );
    SSMatrix    transform ( SSFrame from, SSFrame to, SSMatrix mat );

    SSVector applyAberration ( SSVector direction );
    SSVector removeAberration ( SSVector direction );
    
    static double redShiftToRadVel ( double z );
    static double radVelToRedShift ( double rv );
    
    static SSVector toGeocentric ( SSSpherical geodetic, double re, double f );
    static SSSpherical toGeodetic ( SSVector geocentric, double re, double f );

    static SSAngle refractionAngle ( SSAngle alt, bool a );
    static SSAngle applyRefraction ( SSAngle alt );
    static SSAngle removeRefraction ( SSAngle alt );
    
    SSVector apparentDirection ( SSVector position, double &distance );
};

#endif /* SSCoordinates_hpp */
