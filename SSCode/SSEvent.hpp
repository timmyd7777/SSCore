// SSEvent.hpp
// SSTest
//
// Created by Tim DeBenedictis on 4/18/20.
// Copyright © 2020 Southern Stars. All rights reserved.
//
// This class computes times and circumstances of astronomical events like
// object rising/transit/setting, satellite passes, moon phases, conjuctions,
// oppositions, etc.

#ifndef SSEvent_hpp
#define SSEvent_hpp

#include "SSCoordinates.hpp"
#include "SSObject.hpp"

// Describes the circumstances of an object rise/transit/set event

struct SSRTS
{
    SSTime  time;   // local time when the event takes place [Julian Date and time zone in hours]
    SSAngle azm;    // object's azimuth at the time of the event [radians]
    SSAngle alt;    // object's altitude at the time of the event [radians]
};

// Describes a complete overhead pass of an object across the sky; from rising, through transit, to setting.

struct SSPass
{
    SSRTS rising;       // circumstances of rising event
    SSRTS transit;      // circumstances of transit event
    SSRTS setting;      // circumstances of setting event
};

// Describes circumstances of a generic event: conjunction, opposition, etc.

struct SSEventTime
{
    SSTime time;        // time of event
    double value;       // value at time of event (angular distance in radiams, or physical distance in AU, etc.)
};

// Pointer to generic event-finding function

typedef double (*SSEventFunc) ( SSCoordinates &coords, SSObjectPtr pObj1, SSObjectPtr pObj2 );

class SSEvent
{
public:
    
    static const int kRise = -1;        // event sign for rising, to be used with riseTransSet, etc().
    static const int kTransit = 0;      // event sign for transit, to be used with riseTransSet, etc().
    static const int kSet = 1;          // event sign for setting, to be used with riseTransSet, etc().
    
    static constexpr double kDefaultRiseSetAlt = -30.0 / SSAngle::kArcminPerRad;        // geometric altitude of point object when rising/setting [radians]
    static constexpr double kSunMoonRiseSetAlt = -50.0 / SSAngle::kArcminPerRad;        // geometric altitude of Sun/Moon's apparent disk center when rising/setting [radians]
    static constexpr double kSunCivilDawnDuskAlt = -6.0 / SSAngle::kDegPerRad;          // geometric altitude of Sun's apparent disk center at civil dawn/dusk [radians]
    static constexpr double kSunNauticalDawnDuskAlt = -12.0 / SSAngle::kDegPerRad;      // geometric altitude of Sun's apparent disk center at nautical dawn/dusk [radians]
    static constexpr double kSunAstronomicalDawnDuskAlt = -18.0 / SSAngle::kDegPerRad;  // geometric altitude of Sun's apparent disk center at astronomical dawn/dusk [radians]

    static constexpr double kNewMoon = 0.0;                                             // Moon's ecliptic longitude offset from Sun when at new moon [radians]
    static constexpr double kFirstQuarterMoon = SSAngle::kHalfPi;                       // Moon's ecliptic longitude offset from Sun when at first quarter [radians]
    static constexpr double kFullMoon = SSAngle::kPi;                                   // Moon's ecliptic longitude offset from Sun when at full moon [radians]
    static constexpr double kLastQuarterMoon = 3.0 * SSAngle::kHalfPi;                  // Moon's ecliptic longitude offset from Sun when at last quarter [radians]
    
    static SSAngle semiDiurnalArc ( SSAngle lat, SSAngle dec, SSAngle alt );
    
    static SSTime riseTransitSet ( SSTime jd, SSAngle ra, SSAngle dec, int sign, SSAngle lon, SSAngle lat, SSAngle alt );
    static SSTime riseTransitSet ( SSTime time, SSCoordinates &coords, SSObjectPtr pObj, int sign, SSAngle alt );
    static SSTime riseTransitSetSearch ( SSTime time, SSCoordinates &coords, SSObjectPtr pObj, int sign, SSAngle alt );
    static SSTime riseTransitSetSearchDay ( SSTime today, SSCoordinates &coords, SSObjectPtr pObj, int sign, SSAngle alt );

    static SSPass riseTransitSet ( SSTime today, SSCoordinates &coords, SSObjectPtr pObj, SSAngle alt );
    static int findSatellitePasses ( SSCoordinates &coords, SSObjectPtr pSat, SSTime start, SSTime stop, double minAlt, vector<SSPass> &passes, int maxPasses );

    static SSTime nextMoonPhase ( SSTime time, SSObjectPtr pSun, SSObjectPtr pMoon, double phase );
    
    static void findEvents ( SSCoordinates &coords, SSObjectPtr pObj1, SSObjectPtr pObj2, SSTime start, SSTime stop, double step, bool max, double limit, SSEventFunc func, vector<SSEventTime> &events, int maxEvents );
    static void findEqualityEvents ( SSCoordinates &coords, SSObjectPtr pObj1, SSObjectPtr pObj2, SSTime start, SSTime stop, double step, bool max, double value, SSEventFunc func, vector<SSEventTime> &events, int maxEvents );
    static void findConjunctions ( SSCoordinates &coords, SSObjectPtr pObj1, SSObjectPtr pObj2, SSTime start, SSTime stop, vector<SSEventTime> &events, int maxEvents );
    static void findOppositions ( SSCoordinates &coords, SSObjectPtr pObj1, SSObjectPtr pObj2, SSTime start, SSTime stop, vector<SSEventTime> &events, int maxEvents );
    static void findNearestDistances ( SSCoordinates &coords, SSObjectPtr pObj1, SSObjectPtr pObj2, SSTime start, SSTime stop, vector<SSEventTime> &events, int maxEvents );
    static void findFarthestDistances ( SSCoordinates &coords, SSObjectPtr pObj1, SSObjectPtr pObj2, SSTime start, SSTime stop, vector<SSEventTime> &events, int maxEvents );
};

#endif /* SSEvent_hpp */
