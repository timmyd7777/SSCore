// SSPSEphemeris.hpp
// SSTest
//
// Created by Tim DeBenedictis on 4/14/20.
// Copyright © 2020 Southern Stars. All rights reserved.
//
// Implements Paul Schlyter's planetary and lunar position computation methods, described here:
// http://stjarnhimlen.se/comp/ppcomp.html
// This is the simplest way to compute planetary/lunar positions with an accuracy of 1-2 arc minutes
// over a thousand-year timespan from the present (accuracy degrades outside that interval).
// Includes a "one-hour tweak" which dramatically improves accuracy (to 10-20 arcsec)
// for the Sun and inner planets; see comments in earthOrbit().

#ifndef SSPSEphemeris_hpp
#define SSPSEphemeris_hpp

#include "SSVector.hpp"
#include "SSUtilities.hpp"
#include "SSOrbit.hpp"

class SSPSEphemeris
{
public:
    
    // compute heliocentric position and velocity at Julian Ephemeris Date
    // in AU and AU/day referred to ecliptic of date; return ecliptic lon, lat, dist.
    
    static SSSpherical sun ( double jed, SSVector &pos, SSVector &vel );
    static SSSpherical mercury ( double jed, SSVector &pos, SSVector &vel );
    static SSSpherical venus ( double jed, SSVector &pos, SSVector &vel );
    static SSSpherical earth ( double jed, SSVector &pos, SSVector &vel );
    static SSSpherical mars ( double jed, SSVector &pos, SSVector &vel );
    static SSSpherical jupiter ( double jed, SSVector &pos, SSVector &vel );
    static SSSpherical saturn ( double jed, SSVector &pos, SSVector &vel );
    static SSSpherical uranus ( double jed, SSVector &pos, SSVector &vel );
    static SSSpherical neptune ( double jed, SSVector &pos, SSVector &vel );
    static SSSpherical pluto ( double jed, SSVector &pos, SSVector &vel );
    static SSSpherical moon ( double jed, SSVector &pos, SSVector &vel );   // geocentric, in Earth-radii and Earth-radii per day!

    // return mean helioncentric orbit elements at Julian Ephemeris Date
    // referred to ecliptic of date; q in AU except for Moon.
    
    static SSOrbit mercuryOrbit ( double jed );
    static SSOrbit venusOrbit ( double jed );
    static SSOrbit earthOrbit ( double jed );
    static SSOrbit marsOrbit ( double jed );
    static SSOrbit jupiterOrbit ( double jed );
    static SSOrbit saturnOrbit ( double jed );
    static SSOrbit uranusOrbit ( double jed );
    static SSOrbit neptuneOrbit ( double jed );
    static SSOrbit moonOrbit ( double jed );    // geocentric; q in Earth-radii

    // internal conversion routines
    
    static SSSpherical toEcliptic ( SSOrbit orbit, SSVector &pos, SSVector &vel );
    static SSSpherical toEcliptic ( SSOrbit orbit );
    static SSVector toEquatorial ( SSSpherical coords, double jed, double epoch );
};

#endif /* SSPSEphemeris_hpp */
