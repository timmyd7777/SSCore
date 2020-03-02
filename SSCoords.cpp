//
//  SSCoords.cpp
//  SSCore
//
//  Created by Tim DeBenedictis on 2/28/20.
//  Copyright © 2020 Southern Stars. All rights reserved.
//

#include "SSCoords.hpp"

SSCoords::SSCoords ( void )
{
    
}

SSCoords::SSCoords ( double jd, bool nutate, double lon, double lat )
{
    this->epoch = SSTime ( jd );
    this->lon = lon;
    this->lat = lat;
    this->lst = epoch.getGreenwichMeanSiderealTime() + dl;
    
    if ( nutate )
        getNutationConstants ( de, dl );
    else
        de = dl = 0;
    
    this->obq = getObliquity() + de;
    
    equatorial = getFundamentalToEquatorialMatrix();
    ecliptic = getEquatorialToEclipticMatrix().multiply ( equatorial );
    horizon = getEquatorialToHorizonMatrix().multiply ( equatorial );
    galactic = getFundamentalToGalacticMatrix();
}

void SSCoords::getPrecessionConstants ( double &zeta, double &z, double &theta )
{
    double t = ( epoch.jd - SSTime::kJ2000 ) / 36525.0;
    double t2 = t * t;
    double t3 = t * t2;

    zeta  = SSAngle::fromArcsec ( 2306.2181 * t + 0.30188 * t2 + 0.017998 * t3 ).a;
    z     = SSAngle::fromArcsec ( 2306.2181 * t + 1.09468 * t2 + 0.018203 * t3 ).a;
    theta = SSAngle::fromArcsec ( 2004.3109 * t - 0.42665 * t2 - 0.041833 * t3 ).a;
}

void SSCoords::getNutationConstants ( double &de, double &dl )
{
    double t, n, l, l1, sn, cn, s2n, c2n, s2l, c2l, s2l1, c2l1;
      
    t = ( epoch.jd - SSTime::kJ2000 ) / 36525.0;
    n  = SSAngle::fromDegrees ( 125.0445 -   1934.1363 * t ).mod2Pi().a;
    l  = SSAngle::fromDegrees ( 280.4665 +  36000.7698 * t ).mod2Pi().a;
    l1 = SSAngle::fromDegrees ( 218.3165 + 481267.8813 * t ).mod2Pi().a * 2.0;
    
    sn   = sin ( n );
    cn   = cos ( n );
    s2n  = 2.0 * sn * cn;
    c2n  = cn * cn - sn * sn;
    s2l  = sin ( l );
    c2l  = cos ( l );
    s2l1 = sin ( l1 );
    c2l1 = cos ( l1 );
    
    dl = SSAngle::fromArcsec ( -17.20 * sn - 1.32 * s2l - 0.23 * s2l1 + 0.21 * s2n ).a;
    de = SSAngle::fromArcsec (   9.20 * cn + 0.57 * c2l + 0.10 * c2l1 - 0.09 * c2n ).a;
}

double SSCoords::getObliquity ( void )
{
    double t, e;

    t = ( epoch.jd - SSTime::kJ2000 ) / 36525.0;
    e = 23.439291 + t * ( -0.0130042 + t * ( -0.00000016 + t * 0.000000504 ) );
      
    return SSAngle::fromDegrees ( e ).a;
}

SSMatrix SSCoords::getFundamentalToEquatorialMatrix ( void )
{
    double zeta = 0.0, z = 0.0, theta = 0.0;
    
    getPrecessionConstants ( zeta, z, theta );
    return SSMatrix::rotation ( 6, 2, zeta, 1, theta, 2, z, 0, -obq, 2, dl, 0, obq + de );
}

SSMatrix SSCoords::getEquatorialToEclipticMatrix ( void )
{
    return SSMatrix::rotation ( 1, 0, -obq );
}

SSMatrix SSCoords::getEquatorialToHorizonMatrix ( void )
{
    SSMatrix m = SSMatrix::rotation ( 2, 2, SSAngle::kPi - lst, 1, lat - SSAngle::kHalfPi );
    
    m.m10 = -m.m10; m.m11 = -m.m11; m.m12 = -m.m12;
    
    return ( m );
}

SSMatrix SSCoords::getFundamentalToGalacticMatrix ( void )
{
    return SSMatrix ( -0.0548755604, -0.8734370902, -0.4838350155,
                       0.4941094279, -0.4448296300,  0.7469822445,
                      -0.8676661490, -0.1980763734,  0.4559837762 );
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

SSCoords::RADec SSCoords::toRADec ( SSVector vec )
{
    RADec radec = { 0 };
    vec.toSpherical ( radec.ra, radec.dec );
    return ( radec );
}

SSCoords::LonLat SSCoords::toLonLat ( SSVector vec )
{
    LonLat lonlat = { 0 };
    vec.toSpherical ( lonlat.lon, lonlat.lat );
    return ( lonlat );
}

SSCoords::AzmAlt SSCoords::toAzmAlt ( SSVector vec )
{
    AzmAlt azmalt = { 0 };
    vec.toSpherical ( azmalt.azm, azmalt.alt );
    return ( azmalt );
}

SSCoords::RADec SSCoords::toEquatorial ( double funRA, double funDec )
{
    SSVector funVec = SSVector::fromSpherical ( funRA, funDec, 1.0 );
    SSVector equVec = equatorial.multiply ( funVec );
    return SSCoords::toRADec ( equVec );
}

SSCoords::LonLat SSCoords::toEcliptic ( double funRA, double funDec )
{
    SSVector funVec = SSVector::fromSpherical ( funRA, funDec, 1.0 );
    SSVector eclVec = ecliptic.multiply ( funVec );
    return SSCoords::toLonLat ( eclVec );
}

SSCoords::LonLat SSCoords::toGalactic ( double funRA, double funDec )
{
    SSVector funVec = SSVector::fromSpherical ( funRA, funDec, 1.0 );
    SSVector galVec = galactic.multiply ( funVec );
    return SSCoords::toLonLat ( galVec );
}

SSCoords::AzmAlt SSCoords::toHorizon ( double funRA, double funDec )
{
    SSVector funVec = SSVector::fromSpherical ( funRA, funDec, 1.0 );
    SSVector horVec = horizon.multiply ( funVec );
    return SSCoords::toAzmAlt ( horVec );
}
