//  SSCoords.hpp
//  SSCore
//
//  Created by Tim DeBenedictis on 2/28/20.
//  Copyright © 2020 Southern Stars. All rights reserved.
//    Classes for converting rectangular and spherical coordinates between different astronomical reference frames
//  at a particular time and geographic location.

#ifndef SSCoords_hpp
#define SSCoords_hpp

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

// This class converts coordinates between the principal astronomical reference frames.

class SSCoords
{
public:
    double      epoch;          // precession epoch [Julian Date]
    double      lon;            // observer's longitude [radians, east positive]
    double      lat;            // obseerver's latitude [radians, north positive]
    double      lst;            // local apparent sidereal time [radians]
    double      obq;            // mean obliquity of ecliptic at current epoch [radians]
    double      de;             // nutation in obliquity [radians]
    double      dl;             // nutation in longitude [radians]
    
    SSMatrix    preMat;         // transforms from fundamental to mean precessed equatorial frame, not including nutation.
    SSMatrix    nutMat;         // transforms from mean precessed equatorial frame to true equatorial frame, i.e. corrects for nutation.
    SSMatrix    equMat;         // transforms from fundamental to current true equatorial frame.
    SSMatrix    eclMat;         // transforms from fundamental to current true ecliptic frame (includes nutation).
    SSMatrix    horMat;         // transforms from fundamental to current local horizon frame.
    SSMatrix    galMat;         // transforms from fundamental to galactic frame
    
    SSCoords ( double epoch, double lon, double lat );
    
    static double getObliquity ( double epoch );
    static void   getNutationConstants ( double jd, double &de, double &dl );
    static void   getPrecessionConstants ( double jd, double &zeta, double &z, double &theta );

    static SSMatrix getPrecessionMatrix ( double epoch );
    static SSMatrix getNutationMatrix ( double onliquity, double nutLon, double nutObq );
    static SSMatrix getEclipticMatrix ( double obliquity );
    static SSMatrix getHorizonMatrix ( double lst, double lat );
    static SSMatrix getGalacticMatrix ( void );

    SSVector toEquatorial ( SSVector funVec );
    SSVector toEcliptic ( SSVector funVec );
    SSVector toHorizon ( SSVector funVec );
    SSVector toGalactic ( SSVector funVec );

    SSVector fromEquatorial ( SSVector equVec );
    SSVector fromEcliptic ( SSVector eclVec );
    SSVector fromHorizon ( SSVector horVec );
    SSVector fromGalactic ( SSVector galVec );

    SSSpherical toEquatorial ( SSSpherical funSph );
    SSSpherical toEcliptic ( SSSpherical funSph );
    SSSpherical toGalactic ( SSSpherical funSph );
    SSSpherical toHorizon ( SSSpherical funSph );
    SSSpherical toHorizon ( SSSpherical funSph, bool refract );

    SSSpherical fromEquatorial ( SSSpherical equSph );
    SSSpherical fromEcliptic ( SSSpherical eclSph );
    SSSpherical fromGalactic ( SSSpherical galSph );
    SSSpherical fromHorizon ( SSSpherical horSph );
    SSSpherical fromHorizon ( SSSpherical horSph, bool refract );

    SSVector      transform ( SSFrame from, SSFrame to, SSVector vec );
    SSSpherical transform ( SSFrame from, SSFrame to, SSSpherical sph );
    
    static SSAngle refractionAngle ( SSAngle alt, bool a );
    static SSAngle toRefractedAltitude ( SSAngle alt );
    static SSAngle fromRefractedAltitude ( SSAngle alt );
};

#endif /* SSCoords_hpp */
