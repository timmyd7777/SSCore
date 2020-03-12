//
//  SSDynamics.hpp
//  SSCore
//
//  Created by Tim DeBenedictis on 3/10/20.
//  Copyright © 2020 Southern Stars. All rights reserved.
//

#ifndef SSDynamics_hpp
#define SSDynamics_hpp

#include <stdio.h>

#include "SSCoords.hpp"
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
};

enum SSMoonID
{
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

class SSPlanet
{
public:
    
    string      name;
    SSPlanetID  id;
    SSOrbit     orb;
    
    SSVector    pos;
    SSVector    vel;

    SSVector    dir;
    double      dist;
    float       mag;

    SSPlanet ( SSPlanetID id );
    void computeEphemeris ( class SSDynamics );
};

class SSDynamics
{
    public:
    
    SSCoords    coords;
    SSVector    obsPos;
    SSVector    obsVel;
    
    SSMatrix    orbMat;
    
    double      jde;

	static constexpr double	kKmPerAU = 149597870.7;
	static constexpr double	kKmPerEarthRadii = 6378.137;
	static constexpr double kEarthFlattening = 1 / 198.257;
    static constexpr double kLightKmPerSec = 299792.458;                            // Speed of Light in Kilometers per Second
    static constexpr double kLightAUPerDay = kLightKmPerSec * 86400.0 / kKmPerAU;   // Speed of lignt in days per AU
    
    SSDynamics ( double jd, double lon, double lat );
    
    static SSVector toGeocentric ( SSSpherical geodetic, double re, double f );
    static SSSpherical toGeodetic ( SSVector geocentric, double re, double f );
    
    void getPlanetPositionVelocity ( SSPlanetID, double jde, SSVector &pos, SSVector &vel );
    void getMoonPositionVelocity ( SSMoonID, double jde, SSVector &pos, SSVector &vel );
    
    SSVector addAberration ( SSVector funDir );
    SSVector subtractAberration ( SSVector aberrFunDir );
};

#endif /* SSDynamics_hpp */
