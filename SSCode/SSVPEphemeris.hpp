// SSVPEphemeris.hpp
// SSTest
//
// Created by Tim DeBenedictis on 4/16/20.
// Copyright © 2020 Southern Stars. All rights reserved.
//
// This is a C++ wrapper around Paul Shchlyter's C implementation of
// Van Flandern a& Pulkinnen's "Low-Precision Formulae for Planetary Positions" paper.
// These seem less accurate than Paul Schlyter's own newer formulae, especially for
// the Moon and Pluto, and Schlyter's formulae are much simpler and faster.
// So no real reason to use these, except for historical curiosity.

#ifndef SSVPEphemeris_hpp
#define SSVPEphemeris_hpp

#include "SSVector.hpp"

class SSVPEphemeris
{
public:
    static SSSpherical eclipticCoordinates ( int planet, double jed );
    static SSVector equatorialPosition ( int planet, double jed, double epoch );
    static void fundamentalPositionVelocity ( int planet, double jed, SSVector &pos, SSVector &vel );
};

#endif /* SSVPEphemeris_hpp */

