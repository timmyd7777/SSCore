//  SSStar.cpp
//  SSCore
//
//  Created by Tim DeBenedictis on 3/15/20.
//  Copyright © 2020 Southern Stars. All rights reserved.

#include "SSDynamics.hpp"
#include "SSStar.hpp"

SSStar::SSStar ( SSObjectType type ) : SSObject ( type )
{
    _position = _velocity = SSVector ( HUGE_VAL, HUGE_VAL, HUGE_VAL );
    _Vmag = HUGE_VAL;
    _Bmag = HUGE_VAL;
}

SSStar::SSStar ( SSObjectType type, SSSpherical position, SSSpherical motion, float vmag, float bmag, string spectrum ) : SSObject ( type )
{
    
}

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
