// SSJPLEphemeris.hpp
// SSCore
//
// Created by Tim DeBenedictis on 4/3/20.
// Copyright © 2020 Southern Stars. All rights reserved.
// Routines for reading JPL's binary DE ephemeris files
// and computing lunar and planetary positions from them.
// Based on "C version software for the JPL planetary ephemerides"
// by Piotr A. Dybczynski (dybol@amu.edu.pl),
// Astronomical Observatory of the A. Mickiewicz Universty,
// Sloneczna 36, 60-286 Poznan, POLAND:
// https://apollo.astro.amu.edu.pl/PAD/index.php?n=Dybol.JPLEph

#ifndef SSJPLEphemeris_hpp
#define SSJPLEphemeris_hpp

#include <iostream>
#include <fstream>

#include "SSTime.hpp"
#include "SSAngle.hpp"
#include "SSVector.hpp"

enum SSJPLDESeries
{
    kJPLDE200 = 200,
    kJPLDE403 = 403,
    kJPLDE404 = 404,
    kJPLDE405 = 405,
    kJPLDE406 = 406,
    kJPLDE421 = 421,
    kJPLDE422 = 422,
    kJPLDE430 = 430,
    kJPLDE431 = 431,
    kJPLDE435 = 435,
    kJPLDE438 = 438
};

/* the followin two definitions adapt the software for larger number of constants
(eg. for DE430 and DE431) */

#define NMAX 1000
#define OLDMAX 400

class SSJPLDEphemeris
{
protected:
    int _ksize;
    ifstream _file;
    
    int nvs;                // number of constants
    char nams[NMAX][6];     // names of constants
    double vals[NMAX];      // values of constants
    double ss[3];           // Start JED, Stop JED, Step
    
    void constan ( char nam[][6], double val[], double sss[], int *n );
    void pleph ( double et, int ntarg, int ncent, double rrd[] );
    void state ( double et2[2], int list[12], double pv[][6], double nut[4] );

public:
    
    SSJPLDEphemeris ( void );

    int getConstantNumber ( void ) { return nvs; }
    string getConstantName ( int i ) { return i >= 0 && i < nvs ? nams[i] : ""; }
    double getConstantValue ( int i ) { return i >= 0 && i < nvs ? vals[i] : 0.0; }
    
    double getStart ( void ) { return ss[0]; }
    double getStop ( void ) { return ss[1]; }
    double getStep ( void ) { return ss[2]; }

    bool open ( const string &filename, SSJPLDESeries series );
    void close ( void );
    bool compute ( int id, double jde, SSVector &position, SSVector &velocity );
};

#endif /* SSJPLEphemeris_hpp */
