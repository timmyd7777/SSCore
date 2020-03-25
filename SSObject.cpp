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
	{ kTypePlanet, "PL" },
	{ kTypeMoon, "MN" },
	{ kTypeAsteroid, "AS" },
	{ kTypeComet, "CM" },
	{ kTypeSatellite, "ST" },
	{ kTypeSpacecraft, "SC" },
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
	{ kTypeConstellation, "CN" },
	{ kTypeAsterism, "AM" }
};

SSStringTypeMap _stringTypes =
{
	{ "PL", kTypePlanet },
	{ "MN", kTypeMoon },
	{ "AS", kTypeAsteroid },
	{ "CM", kTypeComet },
	{ "ST", kTypeSatellite },
	{ "SC", kTypeSpacecraft },
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
	{ "CN", kTypeConstellation },
	{ "AM", kTypeAsterism },
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
