// SSJPLEphemeris.hpp
// SSCore
//
// Created by Tim DeBenedictis on 4/3/20.
// Copyright © 2020 Southern Stars. All rights reserved.
//
// This class reads JPL's binary DE43x series of ephemeris files
// and computes vdery fast, accurate lunar and planetary positions from them.

#ifndef SSJPLEphemeris_hpp
#define SSJPLEphemeris_hpp

#include <iostream>
#include <fstream>

#include "SSTime.hpp"
#include "SSAngle.hpp"
#include "SSVector.hpp"

// CAUTION: This class is a thin C++ wrapper around original C code from:
// https://apollo.astro.amu.edu.pl/PAD/index.php?n=Dybol.JPLEph
// This is a singleton class; you should only ever instantiate one of these!
// It is not thread safe, and is hard-coded to read only the DE43x series
// in little-endian (Intel) binary format.  It will not read the ASCII format
// of any ephemeris files, nor the DE43xt series which include time data.

class SSJPLDEphemeris
{
public:
    
    // Opens and closes ephemeris file
    
    bool open ( const string &filename );
    bool isOpen ( void );
    void close ( void );

    // Gets number of contants, name and value of i-th constant.
    
    int getConstantNumber ( void );
    string getConstantName ( int i );
    double getConstantValue ( int i );
    
    // Gets start and stop Julian Ephemeris Date, and time step in days
    
    double getStartJED ( void );
    double getStopJED ( void );
    double getStep ( void );

    // Computes object position and velocity at a given JED.
    
    bool compute ( int id, double jde, bool bary, SSVector &position, SSVector &velocity );
};

#endif /* SSJPLEphemeris_hpp */
