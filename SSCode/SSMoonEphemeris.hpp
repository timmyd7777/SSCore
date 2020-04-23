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
    static void marsMoonPosition ( int id, double jed, SSVector &pos );
    static void jupiterMoonPosition ( int id, double jed, SSVector &pos );
    static void saturnMoonPosition ( int id, double jed, SSVector &pos );
    static void uranusMoonPosition ( int id, double jed, SSVector &pos );
    static void neptuneMoonPosition ( int id, double jed, SSVector &pos );
};

#endif /* SSMoonEphemeris_hpp */
