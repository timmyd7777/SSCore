// VSOP2013.hpp
// SSTest
//
// Created by Tim DeBenedictis on 4/26/20.
// Copyright © 2020 Southern Stars. All rights reserved.
//
// Reads VSOP2013 solution files and computes orbital elements from them.

#ifndef VSOP2013_hpp
#define VSOP2013_hpp

#include <iostream>
#include <vector>

#include "SSOrbit.hpp"

struct VSOP2013Term
{
    short iphi[17];   // numerical coefficients of arguments a[i]
    double s, c;      // coefficients of sine (s) and cosine (c) of arguments
};

struct VSOP2013Series
{
    int ip;     // planet index, 1 = Mercury, 2 = Venus, .... 9 = Pluto
    int iv;     // variable index: 1 = a, 2 = l, 3 = k, 4 = h, 5 = q, 6 = p
    int it;     // time power (alpha)
    int nt;     // number of terms in series
    vector<VSOP2013Term> terms;
};

#define EMBED_SERIES 1   // 1 to include embedded series; 0 to use external data files only

class VSOP2013
{
public:
    double ll[17];
    vector<VSOP2013Series> planets[9];
    void evalLongitudes ( double t );
    double evalSeries ( double t, const VSOP2013Series &ser );
    void printSeries ( ostream &out, const vector<VSOP2013Series> &planet );
    int readFile ( const string &filename, int iplanet );
    SSOrbit getOrbit ( int iplanet, double jed );
#if EMBED_SERIES
    SSOrbit mercuryOrbit ( double jed );
    SSOrbit venusOrbit ( double jed );
    SSOrbit earthOrbit ( double jed );
    SSOrbit marsOrbit ( double jed );
    SSOrbit jupiterOrbit ( double jed );
    SSOrbit saturnOrbit ( double jed );
    SSOrbit uranusOrbit ( double jed );
    SSOrbit neptuneOrbit ( double jed );
    SSOrbit plutoOrbit ( double jed );
#else
    SSOrbit mercuryOrbit ( double jed ) { return getOrbit ( 1, jed ); }
    SSOrbit venusOrbit ( double jed ) { return getOrbit ( 2, jed ); }
    SSOrbit earthOrbit ( double jed ) { return getOrbit ( 3, jed ); }
    SSOrbit marsOrbit ( double jed ) { return getOrbit ( 4, jed ); }
    SSOrbit jupiterOrbit ( double jed ) { return getOrbit ( 5, jed ); }
    SSOrbit saturnOrbit ( double jed ) { return getOrbit ( 6, jed ); }
    SSOrbit uranusOrbit ( double jed ) { return getOrbit ( 7, jed ); }
    SSOrbit neptuneOrbit ( double jed ) { return getOrbit ( 8, jed ); }
    SSOrbit plutoOrbit ( double jed ) { return getOrbit ( 9, jed ); }
#endif
};

#endif /* VSOP2013_hpp */
