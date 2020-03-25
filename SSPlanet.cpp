//
//  SSPlanet.cpp
//  SSCore
//
//  Created by Tim DeBenedictis on 3/15/20.
//  Copyright © 2020 Southern Stars. All rights reserved.
//

#include "SSDynamics.hpp"
#include "SSPlanet.hpp"

SSPlanet::SSPlanet ( SSObjectType type ) : SSObject ( type )
{
    _id = SSIdentifier();
    _orbit = SSOrbit();
	_Hmag = _Gmag = HUGE_VAL;
    _position = _velocity = SSVector ( HUGE_VAL, HUGE_VAL, HUGE_VAL );
}

SSPlanet::SSPlanet ( SSObjectType type, SSPlanetID id ) : SSPlanet ( type )
{
    _id = SSIdentifier ( kCatJPLanet, id );
}

void SSPlanet::computeEphemeris ( SSDynamics &dyn )
{
    double lt = 0.0;
	SSPlanetID planetID = static_cast<SSPlanetID> ( _id.identifier() );
    
    dyn.getPlanetPositionVelocity ( planetID, dyn.jde, _position, _velocity );
    lt = ( _position - dyn.obsPos ).magnitude() / dyn.kLightAUPerDay;

    dyn.getPlanetPositionVelocity ( planetID, dyn.jde - lt, _position, _velocity );
    _direction = ( _position - dyn.obsPos ).normalize ( _distance );
}

// Downcasts generic SSObject pointer to SSPlanet pointer.
// Returns nullptr if input pointer is not an instance of SSPlanet!

SSPlanetPtr SSGetPlanetPtr ( SSObjectPtr ptr )
{
    return dynamic_cast<SSPlanet *> ( ptr.get() );
}

// Returns CSV string from planet data, including identifier and names.

string SSPlanet::toCSV ( void )
{
	string csv = SSObject::typeToCode ( _type ) + ",";
	
	csv += isinf ( _orbit.q ) ? "," : format ( "%.6f,", _orbit.q );
	csv += isinf ( _orbit.e ) ? "," : format ( "%.6f,", _orbit.e );
	csv += isinf ( _orbit.i ) ? "," : format ( "%.6f,", _orbit.i * SSAngle::kDegPerRad );
	csv += isinf ( _orbit.w ) ? "," : format ( "%.6f,", _orbit.w * SSAngle::kDegPerRad );
	csv += isinf ( _orbit.n ) ? "," : format ( "%.6f,", _orbit.n * SSAngle::kDegPerRad );
	csv += isinf ( _orbit.m ) ? "," : format ( "%.6f,", _orbit.m * SSAngle::kDegPerRad );
	csv += isinf ( _orbit.t ) ? "," : format ( "%.6f,", _orbit.t );
	
	csv += isinf ( _Hmag ) ? "," : format ( "%+.2f,", _Hmag );
	csv += isinf ( _Gmag ) ? "," : format ( "%+.2f,", _Gmag );
	
	for ( int i = 0; i < _names.size(); i++ )
		csv += _names[i] + ",";

	return csv;
}
