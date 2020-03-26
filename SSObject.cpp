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
	{ kTypeNonexistent, "NO" },
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
	{ "NO", kTypeNonexistent },
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

SSObject::SSObject ( void ) : SSObject ( kTypeNonexistent )
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

// Default implementation of getIdentifer; overridden by subclasses.

SSIdentifier SSObject::getIdentifier ( SSCatalog cat )
{
	return SSIdentifier();
}

// Default implementation of toCSV; overridden by subclasses.

string SSObject::toCSV ( void )
{
	return "";
}

// Default implementation of compteEphemeris; overridden by subclasses.

void SSObject::computeEphemeris ( SSDynamics &dyn )
{
}

SSObjectMap SSMakeObjectMap ( SSObjectVec &objects, SSCatalog cat )
{
	SSObjectMap map;
	
	for ( int i = 0; i < objects.size(); i++ )
	{
		SSObject *ptr = objects[i].get();
		if ( ptr == nullptr )
			continue;
		
		SSIdentifier ident = ptr->getIdentifier ( cat );
		if ( ! ident )
			continue;
		
		map.insert ( { ident, i + 1 } );
	}
	
	return map;
}
