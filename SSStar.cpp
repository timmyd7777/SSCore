//  SSStar.cpp
//  SSCore
//
//  Created by Tim DeBenedictis on 3/15/20.
//  Copyright © 2020 Southern Stars. All rights reserved.

#include "SSDynamics.hpp"
#include "SSStar.hpp"

std::string format ( const char *fmt, ... )
{
	char buf[1024] = { 0 };

    va_list args;
    va_start ( args, fmt );
    vsnprintf ( buf, sizeof buf, fmt, args );
    va_end ( args );

	return string ( buf );
}

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
	
	_parallax = position.rad;
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

string SSStar::toCSV ( void )
{
	SSSpherical pos = _position.toSpherical();
	SSSpherical vel = _position.toSphericalVelocity( _velocity );
	
	SSHourMinSec ra = pos.lon;
	SSDegMinSec dec = pos.lat;
	
	string csv = "S,";
	
	csv += ra.toString() + ",";
	csv += dec.toString() + ",";
	
	csv += format ( "%+.4f,", ( vel.lon / 15.0 ).toArcsec() );
	csv += format ( "%+.3f,", vel.lat.toArcsec() );
	
	csv += isinf ( _Vmag ) ? "," : format ( "%+.2f,", _Vmag );
	csv += isinf ( _Bmag )  ? "," : format ( "%+.2f,", _Bmag );
	
	csv += isinf ( _parallax ) ? "," : format ( "%.4f,", _parallax );
	csv += isinf ( _radvel ) ? "," : format ( "%+.1f,", _radvel * SSDynamics::kLightKmPerSec );
	
	for ( int i = 0; i < _ids.size(); i++ )
		csv += _ids[i].toString() + ",";
		
	return csv;
}
