//
//  SSCoords.cpp
//  SSCore
//
//  Created by Tim DeBenedictis on 2/28/20.
//  Copyright © 2020 Southern Stars. All rights reserved.
//

#include "SSCoords.hpp"

SSCoords::SSCoords ( double jd, bool nutate, double lon, double lat )
{
    if ( nutate )
        getNutationConstants ( de, dl );
    else
        de = dl = 0;
    
    this->obq = getObliquity();
	this->epoch = jd;
    this->lon = lon;
    this->lat = lat;
    this->lst = SSTime ( jd ).getMeanSiderealTime ( SSAngle ( lon + dl ) ).rad;
    
    equatorial = getFundamentalToEquatorialMatrix();
    ecliptic = getEquatorialToEclipticMatrix().multiply ( equatorial );
    horizon = getEquatorialToHorizonMatrix().multiply ( equatorial );
    galactic = getFundamentalToGalacticMatrix();
}

void SSCoords::getPrecessionConstants ( double &zeta, double &z, double &theta )
{
    double t = ( epoch - SSTime::kJ2000 ) / 36525.0;
    double t2 = t * t;
    double t3 = t * t2;

    zeta  = SSAngle::fromArcsec ( 2306.2181 * t + 0.30188 * t2 + 0.017998 * t3 ).rad;
    z     = SSAngle::fromArcsec ( 2306.2181 * t + 1.09468 * t2 + 0.018203 * t3 ).rad;
    theta = SSAngle::fromArcsec ( 2004.3109 * t - 0.42665 * t2 - 0.041833 * t3 ).rad;
}

void SSCoords::getNutationConstants ( double &de, double &dl )
{
    double t, n, l, l1, sn, cn, s2n, c2n, s2l, c2l, s2l1, c2l1;
      
    t = ( epoch - SSTime::kJ2000 ) / 36525.0;
    n  = SSAngle::fromDegrees ( 125.0445 -   1934.1363 * t ).mod2Pi().rad;
    l  = SSAngle::fromDegrees ( 280.4665 +  36000.7698 * t ).mod2Pi().rad * 2.0;
    l1 = SSAngle::fromDegrees ( 218.3165 + 481267.8813 * t ).mod2Pi().rad * 2.0;
    
    sn   = sin ( n );
    cn   = cos ( n );
    s2n  = 2.0 * sn * cn;
    c2n  = cn * cn - sn * sn;
    s2l  = sin ( l );
    c2l  = cos ( l );
    s2l1 = sin ( l1 );
    c2l1 = cos ( l1 );
    
    dl = SSAngle::fromArcsec ( -17.20 * sn - 1.32 * s2l - 0.23 * s2l1 + 0.21 * s2n ).rad;
    de = SSAngle::fromArcsec (   9.20 * cn + 0.57 * c2l + 0.10 * c2l1 - 0.09 * c2n ).rad;
}

double SSCoords::getObliquity ( void )
{
    double t, e;

    t = ( epoch - SSTime::kJ2000 ) / 36525.0;
    e = 23.439291 + t * ( -0.0130042 + t * ( -0.00000016 + t * 0.000000504 ) );
      
    return SSAngle::fromDegrees ( e ).rad;
}

SSMatrix SSCoords::getFundamentalToEquatorialMatrix ( void )
{
    double zeta = 0.0, z = 0.0, theta = 0.0;
    
    getPrecessionConstants ( zeta, z, theta );
    return SSMatrix::rotation ( 6, 2, zeta, 1, theta, 2, z, 0, -obq, 2, dl, 0, obq + de );
}

SSMatrix SSCoords::getEquatorialToEclipticMatrix ( void )
{
    return SSMatrix::rotation ( 1, 0, - obq - de );
}

SSMatrix SSCoords::getEquatorialToHorizonMatrix ( void )
{
    SSMatrix m = SSMatrix::rotation ( 2, 2, SSAngle::kPi - lst, 1, lat - SSAngle::kHalfPi );
    
    m.m10 = -m.m10; m.m11 = -m.m11; m.m12 = -m.m12;
    
    return ( m );
}

SSMatrix SSCoords::getFundamentalToGalacticMatrix ( void )
{
    return SSMatrix ( -0.054875539390, -0.873437104725, -0.483834991775,
                      +0.494109453633, -0.444829594298, +0.746982248696,
                      -0.867666135681, -0.198076389622, +0.455983794523 );
}

SSVector SSCoords::toEquatorial ( SSVector funVec )
{
    return equatorial.multiply ( funVec );
}

SSVector SSCoords::toEcliptic ( SSVector funVec )
{
    return ecliptic.multiply ( funVec );
}

SSVector SSCoords::toHorizon ( SSVector funVec )
{
    return horizon.multiply ( funVec );
}

SSVector SSCoords::toGalactic ( SSVector funVec )
{
    return galactic.multiply ( funVec );
}

SSVector SSCoords::fromEquatorial ( SSVector funVec )
{
    return equatorial.transpose().multiply ( funVec );
}

SSVector SSCoords::fromEcliptic ( SSVector funVec )
{
    return ecliptic.transpose().multiply ( funVec );
}

SSVector SSCoords::fromHorizon ( SSVector funVec )
{
    return horizon.transpose().multiply ( funVec );
}

SSVector SSCoords::fromGalactic ( SSVector funVec )
{
    return galactic.transpose().multiply ( funVec );
}

SSSpherical SSCoords::toEquatorial ( SSSpherical fun )
{
    return SSSpherical ( toEquatorial ( SSVector ( fun ) ) );
}

SSSpherical SSCoords::toEcliptic ( SSSpherical fun )
{
    return SSSpherical ( toEcliptic ( SSVector ( fun ) ) );
}

SSSpherical SSCoords::toGalactic ( SSSpherical fun )
{
    return SSSpherical ( toGalactic ( SSVector ( fun ) ) );
}

SSSpherical SSCoords::toHorizon ( SSSpherical fun )
{
    return SSSpherical ( toHorizon ( SSVector ( fun ) ) );
}

SSSpherical SSCoords::fromEquatorial ( SSSpherical equ )
{
    return SSSpherical ( fromEquatorial ( SSVector ( equ ) ) );
}

SSSpherical SSCoords::fromEcliptic ( SSSpherical ecl )
{
    return SSSpherical ( fromEcliptic ( SSVector ( ecl ) ) );
}

SSSpherical SSCoords::fromGalactic ( SSSpherical gal )
{
    return SSSpherical ( fromGalactic ( SSVector ( gal ) ) );
}

SSSpherical SSCoords::fromHorizon ( SSSpherical hor )
{
    return SSSpherical ( fromHorizon ( SSVector ( hor ) ) );
}
