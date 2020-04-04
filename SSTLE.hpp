// SSTLE.hpp
// SSCore
//
// Created by Tim DeBenedictis on 4/4/20.
// Copyright © 2020 Southern Stars. All rights reserved.
// Routines for reading satellite orbital elements from TLE (Two/Three-Line Element) files,
// and computing satellite position/velocity from them using the SGP, SGP4, and SDP4 orbit
// models, and vice-versa.

#ifndef SSTLE_hpp
#define SSTLE_hpp

#include <string>
#include <iostream>
#include <fstream>

using namespace std;

struct SSTLE
{
    string name;        // Object Name, e.g. "ISS"
    string desig;       // International designation, e.g. "98067A"
    int    norad;       // NORAD tracking number, e.g. 25544
    double jdepoch;     // Epoch (as Julian Date) in Civil (UTC) time scale, not Dynamic time (TDT)
    double xndt2o;      // One half of first derivative of mean motion (radians/minute^2)
    double xndd6o;      // One sixth of second derivatibe of mean motion (radians/minute^3)
    double bstar;       // BSTAR drag coefficient
    double xincl;       // Inclination (radians)
    double xnodeo;      // Right Ascension of Ascending Node (radians)
    double eo;          // Eccentricity
    double omegao;      // Argument of Perigee (radians)
    double xmo;         // Mean anomaly (radians)
    double xno;         // Mean motion (radians/minute)
    
    // Read from/write to input/output stream.
    
    int read ( istream &file );
    int write ( ostream &file );
    
    static char checksum ( string &line );
};

#endif /* SSTLE_hpp */
