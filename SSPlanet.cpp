//
//  SSPlanet.cpp
//  SSCore
//
//  Created by Tim DeBenedictis on 3/15/20.
//  Copyright © 2020 Southern Stars. All rights reserved.
//

#include "SSDynamics.hpp"
#include "SSPlanet.hpp"

SSPlanet::SSPlanet ( void ) : SSPlanet ( kTypePlanet, kSun )
{
	
}

SSPlanet::SSPlanet ( SSObjectType type, SSPlanetID id ) : SSObject ( type )
{
    _id = id;
    _orbit = SSOrbit();
    _position = _velocity = SSVector ( HUGE_VAL, HUGE_VAL, HUGE_VAL );
}

void SSPlanet::computeEphemeris ( SSDynamics &dyn )
{
    double lt = 0.0;
    
    dyn.getPlanetPositionVelocity ( _id, dyn.jde, _position, _velocity );
    lt = ( _position - dyn.obsPos ).magnitude() / dyn.kLightAUPerDay;

    dyn.getPlanetPositionVelocity ( _id, dyn.jde - lt, _position, _velocity );
    _direction = ( _position - dyn.obsPos ).normalize ( _distance );
}
