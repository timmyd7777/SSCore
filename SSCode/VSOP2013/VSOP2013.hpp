// VSOP2013.hpp
// SSTest
//
// Created by Tim DeBenedictis on 4/26/20.
// Copyright © 2020 Southern Stars. All rights reserved.
//
// Reads VSOP2013 solution files and computes planetary orbital elements,
// positions, and velocities from them. Can export C++ source code containing
// all or a subset of the original solution files, and/or use embedded C++ series
// for a 10x - 100x computation performance gain.
// The embedded C++ VSOP2013 series included here contains 1/100th of the
// original terms (1/10th for Pluto) but maintains sub-arcsecond agreement
// with the original series for all planets over the -4000 to +8000 year
// original VSOP2013 timespan.
// Based on "New analytical planetary theories VSOP2013 and TOP2013", by
// J.-L. Simon1, G. Francou, A. Fienga, and H. Manche, Astronomy & Astrophysics 557,
// A49 (May 2013). Machine-readable version at ftp.imcce.fr/pub/ephem/planets/vsop2013

#ifndef VSOP2013_hpp
#define VSOP2013_hpp

#include <iostream>
#include <vector>

#include "SSOrbit.hpp"

// Stores data for an individual term in a VSOP2013 series

struct VSOP2013Term
{
    short iphi[17];   // numerical coefficients of arguments a[i]
    double s, c;      // coefficients of sine (s) and cosine (c) of arguments
};

// Stores data for an entire series of VSOP2013 terms.

struct VSOP2013Series
{
    int ip;     // planet index, 1 = Mercury, 2 = Venus, .... 9 = Pluto
    int iv;     // variable index: 1 = a, 2 = l, 3 = k, 4 = h, 5 = q, 6 = p
    int it;     // time power (alpha)
    int nt;     // number of terms in series
    vector<VSOP2013Term> terms;
};

#ifndef VSOP2013_EMBED_SERIES
#define VSOP2013_EMBED_SERIES 1   // 1 to include embedded series; 0 to use external data files only
#endif

// This class stores VSOP2013 planetary ephemeris series, reads them from data files,
// exports them to C++ source code, and computes planetary position/velocity from them.

class VSOP2013
{
protected:
    vector<VSOP2013Series> planets[9];      // series for each planet 0 = Mercury ... 8 = Pluto
    
public:
    void evalLongitudes ( double t, double ll[17] );
    double evalSeries ( double t, const VSOP2013Series &ser, double ll[17] );
    void printSeries ( ostream &out, const vector<VSOP2013Series> &planet );
    int readFile ( const string &filename, int iplanet );
    SSOrbit getOrbit ( int iplanet, double jed );
    double getMeanMotion ( int iplanet, double a );
    SSVector toEquatorial ( SSVector ecl );
    bool computePositionVelocity ( int iplanet, double jed, SSVector &pos, SSVector &vel );
    
#if VSOP2013_EMBED_SERIES
    SSOrbit mercuryOrbit ( double jed );
    SSOrbit venusOrbit ( double jed );
    SSOrbit earthOrbit ( double jed );      // Earth-Moon barycenter
    SSOrbit marsOrbit ( double jed );
    SSOrbit jupiterOrbit ( double jed );
    SSOrbit saturnOrbit ( double jed );
    SSOrbit uranusOrbit ( double jed );
    SSOrbit neptuneOrbit ( double jed );
    SSOrbit plutoOrbit ( double jed );
#else
    SSOrbit mercuryOrbit ( double jed ) { return getOrbit ( 1, jed ); }
    SSOrbit venusOrbit ( double jed ) { return getOrbit ( 2, jed ); }
    SSOrbit earthOrbit ( double jed ) { return getOrbit ( 3, jed ); }      // Earth-Moon barycenter
    SSOrbit marsOrbit ( double jed ) { return getOrbit ( 4, jed ); }
    SSOrbit jupiterOrbit ( double jed ) { return getOrbit ( 5, jed ); }
    SSOrbit saturnOrbit ( double jed ) { return getOrbit ( 6, jed ); }
    SSOrbit uranusOrbit ( double jed ) { return getOrbit ( 7, jed ); }
    SSOrbit neptuneOrbit ( double jed ) { return getOrbit ( 8, jed ); }
    SSOrbit plutoOrbit ( double jed ) { return getOrbit ( 9, jed ); }
#endif
};

#endif /* VSOP2013_hpp */
