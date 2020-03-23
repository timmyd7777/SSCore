//  SSPlanet.hpp
//  SSCore
//
//  Created by Tim DeBenedictis on 3/15/20.
//  Copyright © 2020 Southern Stars. All rights reserved.
//  Represents solar system objects (planets, moons, asteroids, etc.)

#ifndef SSPlanet_hpp
#define SSPlanet_hpp

#include "SSObject.hpp"
#include "SSOrbit.hpp"

enum SSPlanetID
{
    kSun = 0,
    kMercury = 1,
    kVenus = 2,
    kEarth = 3,
    kMars = 4,
    kJupiter = 5,
    kSaturn = 6,
    kUranus = 7,
    kNeptune = 8,
    kPluto = 9,

    kLuna = 301,
    
    kPhobos = 401,
    kDeimos = 402,
    
    kIo = 501,
    kEuropa = 502,
    kGanymede = 503,
    kCallisto = 504,
    
    kMimas = 601,
    kEnceladus = 602,
    kTethys = 603,
    kDione = 604,
    kRhea = 605,
    kTitan = 606,
    kHyperion = 607,
    kIapetus = 608,
    kPhoebe = 609,
    
    kMiranda = 701,
    kAriel = 702,
    kUmbriel = 703,
    kTitania = 704,
    kOberon = 705,

    kTriton = 801,
    kNereid = 802,
    
    kCharon = 901
};

class SSPlanet : public SSObject
{
protected:

    SSPlanetID      _id;    // planet/moon identifier

    SSOrbit     _orbit;     // current orbital elements
    
    SSVector    _position;  // current heliocentric position in fundamental frame in AU
    SSVector    _velocity;  // current heliocentric velocity in fundamental frame in AU per day

public:
    
	SSPlanet ( void );
    SSPlanet ( SSObjectType type, SSPlanetID id );
    void computeEphemeris ( SSDynamics &dyn );
};

#endif /* SSPlanet_hpp */
