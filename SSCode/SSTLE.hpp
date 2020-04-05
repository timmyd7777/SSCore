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

#include "SSVector.hpp"

using namespace std;

struct SSTLE
{
    string name;        // Satellite Name, e.g. "ISS"
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
    bool deep;          // If period > 225 minutes, should use deep space SDP4 ephemeris; see isdeep().
    
    union
    {
        struct sgp_args *sgp;
        struct sgp4_args *sgp4;
        struct sdp4_args *sdp4;
    }
    argp;

    SSTLE ( void );
    SSTLE ( const SSTLE &other );
    
    // Read from/write to input/output stream.
    
    int read ( istream &file );
    int write ( ostream &file );
    void delargs ( void );
    
    static char checksum ( string &line );
    
    // Orbit models
    
    void sgp ( double tsince, SSVector &pos, SSVector &vel );
    void sgp4 ( double tsince, SSVector &pos, SSVector &vel );
    void sdp4 ( double tsince, SSVector &pos, SSVector &vel );

    bool isdeep ( void );
    void dodeep ( int ientry, struct deep_args *args );
    
    void rvel ( SSVector &pos, SSVector &vel );
    void rv2el ( SSVector &pos, SSVector &vel );

    void toPositionVelocity ( double jd, SSVector &pos, SSVector &vel );
    void fromPositionVelocity ( double jd, SSVector &pos, SSVector &vel );
};

#endif /* SSTLE_hpp */
