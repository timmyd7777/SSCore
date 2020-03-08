//
//  SSCoords.hpp
//  SSCore
//
//  Created by Tim DeBenedictis on 2/28/20.
//  Copyright © 2020 Southern Stars. All rights reserved.
//	Classes for converting rectangular and spherical coordinates between different astronomical reference frames.

#ifndef SSCoords_hpp
#define SSCoords_hpp

#include <stdio.h>
#include "SSAngle.hpp"
#include "SSTime.hpp"
#include "SSMatrix.hpp"

// Identifiers for the principal astronomical reference frames.

enum SSFrame
{
	kFundamental = 0,	// ICRS: the mean equatorial reference frame at epoch J2000 (excludes nutation); X/Y plane is Earth's equatorial plane, +X toward vernal equinox, +Z toward north pole; spherical coords are RA/Dec
	kEquatorial = 1,	// equatorial frame at a specific epoch (including nutation); X/Y plane is Earth's equatorial plane, +X toward vernal equinox, +Z toward north pole; spherical coords are RA/Dec
	kEcliptic = 2,		// ecliptic frame at a specific epoch (includes nutation); X/Y plane is Earth's orbital plane; +X toward vernal equinox, +Z toward north ecliptic pole; spherical coords are ccliptic lon/lat
	kGalactic = 3,		// galactic frame; fixed relative to ICRS; X/Y plane is galactic equator; +X toward galactic center, +Z toward north galactic pole; spherical coords are galactic lon/lat
	kHorizon = 4,		// local horizon frame; X/Y plane is local horizon, +X is north, +Z is zenith; ; spherical coords are azimuth/altitude
};

// This class converts coordinates between the principal astronomical reference frames.

class SSCoords
{
private:
    double      epoch;          // precession epoch [Julian Date]
    double      lon;            // observer's longitude [radians, east positive]
    double      lat;            // obseerver's latitude [radians, north positive]
    double      lst;            // local apparent sidereal time [radians]
    double      obq;            // obliquity of ecliptic [radians]
    double      de;             // nutation in obliquity [radians]
    double      dl;             // nutation in longitude [radians]
    
    SSMatrix    equMat;     	// transforms coords from fundamental to equatorial frame
    SSMatrix    eclMat;       	// transforms coords from fundamental to ecliptic frame
    SSMatrix    horMat;       	// transforms coords from fundamental to horizon frame
    SSMatrix    galMat;       	// transforms coords from fundamental to galactic frame
    
public:

    SSCoords ( double epoch, bool nutate, double lon, double lat );
    
    double  getObliquity ( void );
    void    getNutationConstants ( double &de, double &dl );
    void    getPrecessionConstants ( double &zeta, double &z, double &theta );
    double  getLocalSiderealTime ( void );
    
    SSMatrix getFundamentalToEquatorialMatrix ( void );
    SSMatrix getEquatorialToEclipticMatrix ( void );
    SSMatrix getEquatorialToHorizonMatrix ( void );
    SSMatrix getFundamentalToGalacticMatrix ( void );

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

    SSSpherical fromEquatorial ( SSSpherical equSph );
    SSSpherical fromEcliptic ( SSSpherical eclSph );
    SSSpherical fromGalactic ( SSSpherical galSph );
    SSSpherical fromHorizon ( SSSpherical horSph );

    SSVector  	transform ( SSFrame from, SSFrame to, SSVector vec );
    SSSpherical transform ( SSFrame from, SSFrame to, SSSpherical sph );
};

#endif /* SSCoords_hpp */
