// SSMoonEphemeris.hpp
// SSTest
//
// Created by Tim DeBenedictis on 4/22/20.
// Copyright © 2020 Southern Stars. All rights reserved.
//
// Ephemeris routines for moons of Mars, Jupiter, Saturn, Uranus, Neptune, Pluto.
// For Earth's Moon, use SSDPLDEpehemris or SSPSEphemeris.

#ifndef SSMoonEphemeris_hpp
#define SSMoonEphemeris_hpp

#include "SSVector.hpp"

class SSMoonEphemeris
{
public:
    static bool marsMoonPositionVelocity ( int id, double jed, SSVector &pos, SSVector &vel );
    static bool jupiterMoonPositionVelocity ( int id, double jed, SSVector &pos, SSVector &vel );
    static bool saturnMoonPositionVelocity ( int id, double jed, SSVector &pos, SSVector &vel );
    static bool uranusMoonPositionVelocity ( int id, double jed, SSVector &pos, SSVector &vel );
    static bool neptuneMoonPositionVelocity ( int id, double jed, SSVector &pos, SSVector &vel );
    static bool plutoMoonPositionVelocity ( int id, double jed, SSVector &pos, SSVector &vel );
};

#endif /* SSMoonEphemeris_hpp */
