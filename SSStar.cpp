//  SSStar.cpp
//  SSCore
//
//  Created by Tim DeBenedictis on 3/15/20.
//  Copyright © 2020 Southern Stars. All rights reserved.

#include "SSDynamics.hpp"
#include "SSStar.hpp"

SSStar::SSStar ( void ) : SSObject ( kStar )
{
	_names = vector<string> ( 0 );
	_ids = vector<SSIdentifier> ( 0 );

	_parallax = HUGE_VAL;
	_radvel = HUGE_VAL;
    _position = _velocity = SSVector ( HUGE_VAL, HUGE_VAL, HUGE_VAL );
    _Vmag = HUGE_VAL;
    _Bmag = HUGE_VAL;
	
	_spectrum = "";
}

SSStar::SSStar ( SSObjectType type ) : SSObject ( type )
{
	_names = vector<string> ( 0 );
	_ids = vector<SSIdentifier> ( 0 );

	_parallax = HUGE_VAL;
	_radvel = HUGE_VAL;
	_position = _velocity = SSVector ( HUGE_VAL, HUGE_VAL, HUGE_VAL );
    _Vmag = HUGE_VAL;
    _Bmag = HUGE_VAL;

	_spectrum = "";
}

SSStar::SSStar ( SSObjectType type, vector<string> names, vector<SSIdentifier> ids, SSSpherical position, SSSpherical motion, float vmag, float bmag, string spectrum ) : SSObject ( type )
{
	_names = names;
	_ids = ids;
	
	_parallax = 1.0 / position.rad;
	_radvel = motion.rad;

	if ( _parallax <= 0.0 || isinf ( position.rad ) )
	{
		position.rad = 1.0;
		motion.rad = 0.0;
	}
	
	if ( isinf ( motion.rad ) )
		motion.rad = 0.0;
	
	_position = position.toVectorPosition();
	_velocity = position.toVectorVelocity ( motion );
	
	_Vmag = vmag;
	_Bmag = bmag;
	
	_spectrum = spectrum;
}

vector<SSIdentifier> SSStar::getIdentifiers ( void )
{
	return _ids;
}

SSIdentifier SSStar::getIdentifier ( SSCatalog cat )
{
	for ( int i = 0; i < _ids.size(); i++ )
		if ( _ids[i].catalog() == cat )
			return _ids[i];
	
	return SSIdentifier();
}

vector<SSIdentifier> ids ( void );

void SSStar::computeEphemeris ( SSDynamics dyn )
{
    if ( _parallax > 0.0 )
    {
        _direction = _position + _velocity * ( dyn.jde - SSTime::kJ2000 );
        _distance = _direction.magnitude();
        _direction /= _distance;
        _magnitude = _Vmag + 5.0 * log10 ( _distance * _parallax );
    }
    else
    {
        _direction = _position;
        _distance = HUGE_VAL;
        _magnitude = _Vmag;
    }
}

SSSpherical SSStar::getFundamentalPosition ( void )
{
	SSSpherical pos = _position.toSpherical();
	pos.rad = isinf ( _parallax ) || _parallax == 0.0 ? HUGE_VAL : 1.0 / _parallax;
	return pos;
}

SSSpherical SSStar::getFundamentalProperMotion ( void )
{
	SSSpherical vel = _position.toSphericalVelocity ( _velocity );
	vel.rad = _radvel;
	return vel;
}

string SSStar::toCSV ( void )
{
	SSSpherical pos = _position.toSpherical();
	SSSpherical vel = _position.toSphericalVelocity( _velocity );
	
	SSHourMinSec ra = pos.lon;
	SSDegMinSec dec = pos.lat;
	
	string csv = "S,";
	
	csv += ra.toString() + ",";
	csv += dec.toString() + ",";
	
	csv += isnan ( vel.lon ) ? "        ," : format ( "%+8.5f,", ( vel.lon / 15.0 ).toArcsec() );
	csv += isnan ( vel.lat ) ? "        ," : format ( "%+7.4f,", vel.lat.toArcsec() );
	
	csv += isinf ( _Vmag ) ? "      ," : format ( "%+6.2f,", _Vmag );
	csv += isinf ( _Bmag ) ? "      ," : format ( "%+6.2f,", _Bmag );
	
	csv += isinf ( _parallax ) ? "      ," : format ( "%6.4f,", _parallax );
	csv += isinf ( _radvel )   ? "      ," : format ( "%+6.1f,", _radvel * SSDynamics::kLightKmPerSec );
	
	csv += _spectrum + ",";
	
	for ( int i = 0; i < _names.size(); i++ )
		csv += _names[i] + ",";

	for ( int i = 0; i < _ids.size(); i++ )
		csv += _ids[i].toString() + ",";
		
	return csv;
}
