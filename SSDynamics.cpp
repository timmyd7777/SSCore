//
//  SSDynamics.cpp
//  SSCore
//
//  Created by Tim DeBenedictis on 3/10/20.
//  Copyright © 2020 Southern Stars. All rights reserved.
//

#include <map>
#include "SSDynamics.hpp"

SSDynamics::SSDynamics ( double jd, double lon, double lat ) : coords ( jd, lon, lat )
{
    jde = SSTime ( jd ).getJulianEphemerisDate();
    orbMat = SSCoords::getEclipticMatrix ( SSCoords::getObliquity ( SSTime::kJ2000 ) );
    getPlanetPositionVelocity ( kEarth, obsPos, obsVel );
}

void SSDynamics::getPlanetPositionVelocity ( SSPlanetID id, SSVector &pos, SSVector &vel )
{
    SSOrbit orb;
    
    if ( id == kMercury )
        orb = SSOrbit::getMercuryOrbit ( jde );
    else if ( id == kVenus )
        orb = SSOrbit::getVenusOrbit ( jde );
    else if ( id == kEarth )
        orb = SSOrbit::getEarthOrbit ( jde );
    else if ( id == kMars )
        orb = SSOrbit::getMarsOrbit ( jde );
    else if ( id == kJupiter )
        orb = SSOrbit::getJupiterOrbit ( jde );
    else if ( id == kSaturn )
        orb = SSOrbit::getSaturnOrbit ( jde );
    else if ( id == kUranus )
        orb = SSOrbit::getUranusOrbit ( jde );
    else if ( id == kNeptune )
        orb = SSOrbit::getNeptuneOrbit ( jde );
    else if ( id == kPluto )
        orb = SSOrbit::getPlutoOrbit ( jde );

    orb.toPositionVelocity ( jde, pos, vel );
    pos = orbMat.multiply ( pos );
    vel = orbMat.multiply ( vel );
}

void SSDynamics::getMoonPositionVelocity ( SSMoonID id, SSVector &pos, SSVector &vel )
{

}

SSPlanet::SSPlanet ( SSPlanetID id )
{
    this->id = id;
    this->pos = SSVector();
    this->vel = SSVector();
    this->dir = SSVector();
    this->dist = 0.0;
    this->mag = 0.0;
}

void SSPlanet::computeEphemeris ( SSDynamics dyn )
{
    dyn.getPlanetPositionVelocity ( id, pos, vel );
    
    dir = pos.subtract ( dyn.obsPos );
    dist = dir.normalize();
}
