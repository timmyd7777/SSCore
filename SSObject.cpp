//  SSObject.cpp
//  SSCore
//
//  Created by Tim DeBenedictis on 3/15/20.
//  Copyright © 2020 Southern Stars. All rights reserved.
//  Base class for all objects in the database (stars, planets, etc.)

#include <map>

#include "SSObject.hpp"
#include "SSDynamics.hpp"

typedef map<SSObjectType,string> SSTypeStringMap;
typedef map<string,SSObjectType> SSStringTypeMap;

SSTypeStringMap _typeStrings =
{
	{ kTypeStar, "SS" },
	{ kTypeDoubleStar, "DS" },
	{ kTypeVariableStar, "VS" },
	{ kTypeDoubleVariableStar, "DV" },
	{ kTypeOpenCluster, "OC" },
	{ kTypeGlobularCluster, "GC" },
	{ kTypeBrightNebula, "BN" },
	{ kTypeDarkNebula, "DN" },
	{ kTypePlanetaryNebula, "PN" },
	{ kTypeGalaxy, "GX" },
	{ kTypePlanet, "PL" },
	{ kTypeMoon, "MN" },
	{ kTypeAsteroid, "MP" },
	{ kTypeComet, "CM" },
	{ kTypeSatellite, "SA" },
	{ kTypeSpacecraft, "SC" }
};

SSStringTypeMap _stringTypes =
{
	{ "SS", kTypeStar },
	{ "DS", kTypeDoubleStar },
	{ "VS", kTypeVariableStar },
	{ "DV", kTypeDoubleVariableStar },
	{ "OC", kTypeOpenCluster },
	{ "GC", kTypeGlobularCluster },
	{ "BN", kTypeBrightNebula },
	{ "DN", kTypeDarkNebula },
	{ "PN", kTypePlanetaryNebula },
	{ "GX", kTypeGalaxy },
	{ "PL", kTypePlanet },
	{ "MN", kTypeMoon },
	{ "MP", kTypeAsteroid },
	{ "CM", kTypeComet },
	{ "SA", kTypeSatellite },
	{ "SC", kTypeSpacecraft }
};

string SSObject::typeToCode ( SSObjectType type )
{
	return _typeStrings[ type ];
}

SSObjectType SSObject::codeToType ( string code )
{
	return _stringTypes[ code ];
}

SSObject::SSObject ( void ) : SSObject ( kTypeUnknown )
{

}

SSObject::SSObject ( SSObjectType type )
{
    _type = type;
    _names = vector<string> ( 0 );
    _direction = SSVector ( HUGE_VAL, HUGE_VAL, HUGE_VAL );
    _distance = HUGE_VAL;
    _magnitude = HUGE_VAL;
}

string SSObject::getName ( int i )
{
    if ( i >= 0 && i < _names.size() )
        return _names[i];
    else
        return string ( "" );
}

void SSObject::computeEphemeris ( SSDynamics &dyn )
{
	
}
