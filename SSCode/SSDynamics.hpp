// SSDynamics.hpp
// SSCore
//
// Created by Tim DeBenedictis on 3/10/20.
// Copyright © 2020 Southern Stars. All rights reserved.
//
// Implements solar system and stellar dynamics. Computes position, motion, and visual magnitude of all
// types of celestial objects, as seen by an observer from a particular time and location, in a unified fashion.

#ifndef SSDynamics_hpp
#define SSDynamics_hpp

#include <stdio.h>

#include "SSCoords.hpp"
#include "SSPlanet.hpp"

class SSDynamics
{
    public:
    
    SSCoords    coords;
    SSVector    obsPos;
    SSVector    obsVel;
    
    SSMatrix    orbMat;
    
    double      jde;

    static constexpr double kKmPerAU = 149597870.7;
    static constexpr double kKmPerEarthRadii = 6378.137;
    static constexpr double kEarthFlattening = 1 / 298.257;
    static constexpr double kLightKmPerSec = 299792.458;                            // Speed of light in kilometers per second
    static constexpr double kLightAUPerDay = kLightKmPerSec * 86400.0 / kKmPerAU;   // Speed of lignt in astronomical units per day = 173.144
    static constexpr double kAUPerParsec = SSAngle::kArcsecPerRad;                  // Astronomical units per parsec = 206264.806247
    static constexpr double kParsecPerAU = 1.0 / kAUPerParsec;                      // Parsecs per astronomical unit
    static constexpr double kAUPerLY = kLightAUPerDay * 365.25;                     // Astronomical units per light year = 63241.077084 (assuming Julian year of exactly 365.25 days)
    static constexpr double kLYPerAU = 1.0 / kAUPerLY;                              // Light years per astronomical unit
    static constexpr double kLYPerParsec = kAUPerParsec / kAUPerLY;                 // Light years per parsec = 3.261563777179643
    static constexpr double kParsecPerLY = kAUPerLY / kAUPerParsec;                 // Parsecs per light year

    
    SSDynamics ( double jd, double lon, double lat );
    
    static SSVector toGeocentric ( SSSpherical geodetic, double re, double f );
    static SSSpherical toGeodetic ( SSVector geocentric, double re, double f );
    
    void getPlanetPositionVelocity ( SSPlanetID pid, double jde, SSVector &pos, SSVector &vel );
    void getMoonPositionVelocity ( SSPlanetID mid, double jde, SSVector &pos, SSVector &vel );
    
    SSVector addAberration ( SSVector funDir );
    SSVector subtractAberration ( SSVector aberrFunDir );
    
    static double redShiftToRadVel ( double );
    static double radVelToRedShift ( double );
};

#endif /* SSDynamics_hpp */
