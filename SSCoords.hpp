//
//  SSCoords.hpp
//  SSCore
//
//  Created by Tim DeBenedictis on 2/28/20.
//  Copyright © 2020 Southern Stars. All rights reserved.
//

#ifndef SSCoords_hpp
#define SSCoords_hpp

#include <stdio.h>
#include "SSAngle.hpp"
#include "SSTime.hpp"
#include "SSMatrix.hpp"

class SSCoords
{
private:
    SSTime      epoch;          // precession epoch [Julian Date]
    double      lon;            // observer's longitude [radians, east positive
    double      lat;            // obseerver's latitude [radians, north positive]
    double      lst;            // local apparent sidereal time [radians]
    double      obq;            // obliquity of ecliptic [radians]
    double      de;             // nutation in obliquity [radians]
    double      dl;             // nutation in longitude [radians]
    
    SSMatrix    equatorial;     // transforms coords from fundamental to equatorial frame
    SSMatrix    ecliptic;       // transforms coords from fundamental to ecliptic frame
    SSMatrix    horizon;        // transforms coords from fundamental to horizon frame
    SSMatrix    galactic;       // transforms coords from fundamental to galactic frame
    
public:

    struct LonLat
    {
        SSAngle lon;
        SSAngle lat;
    };

    struct RADec
    {
        SSAngle ra;
        SSAngle dec;
    };

    struct AzmAlt
    {
        SSAngle azm;
        SSAngle alt;
    };

    enum Frame
    {
        kFundamental = 0,
        kEquatorial = 1,
        kEcliptic = 2,
        kGalactic = 3,
        kHorizon = 4,
    };
    
    SSCoords ( void );
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

    RADec    toEquatorial ( double funRA, double funDec );
    LonLat   toEcliptic ( double funRA, double funDec );
    LonLat   toGalactic ( double funRA, double funDec );
    AzmAlt   toHorizon ( double funRA, double funDec );

    RADec     fromEquatorial ( double equRA, double equDec );
    LonLat    fromEcliptic ( double eclLon, double eclLat );
    LonLat    fromGalactic ( double galLon, double galLat );
    AzmAlt    fromHorizon ( double azm, double alt );

    static RADec toRADec ( SSVector vec );
    static LonLat toLonLat ( SSVector vec );
    static AzmAlt toAzmAlt ( SSVector vec );
    
    SSVector    transform ( Frame from, Frame to, SSVector vec );
    LonLat    transform ( Frame from, Frame to, double lon, double lat );
};

#endif /* SSCoords_hpp */
