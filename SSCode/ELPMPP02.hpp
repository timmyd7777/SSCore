// ELPMPP02.hpp
//
// Created by Tim DeBenedictis on 4/27/20.
// Copyright © 2020 Southern Stars. All rights reserved.
//
// This is a C++ wrapper around a modified version of Yuk Tung Liu's
// implementation of the ELP/MPP02 lunar ephemeris found here:
// https://github.com/ytliu0/ElpMpp02
// The original ELP/MPP02 lunar ephemeris is dessribed in
// "The lunar theory ELP revisited. Introduction of new planetary perturbations",
// J. Chapront & G. Francou, Astronomy & Astrophysics 404, 735-742 (2003) with macnine-
// readable files here: ftp://cyrano-se.obspm.fr/pub/2_lunar_solutions/2_elpmpp02/
// This version uses the original Chapront ELPMPP02 data files instead of Liu's modified files,
// and optionally embeds the Chapront series data files embedded in source code to avoid reading
// them from the filesystem. It also computes the Moon's velocity, unlike Liu's version.

#ifndef ELPMPP02_hpp
#define ELPMPP02_hpp

#include "SSVector.hpp"

#define ELPMPP02_EMBED_SERIES 1     // 1 to include embedded series; 0 to use external data files only

using namespace std;

// Represents an individual term in an ELPMPP02 "main problem" series.

struct ELPMainTerm
{
    char  i[4];      // coefficients of Delaunay arguments
    double a;        // coefficient in arcseconds for longitude/latitude, km for distance
    double b[6];     // derivates of a with respect to six constants
};

// Represents a complete ELPMPP02 "main problem" series.

struct ELPMainSeries
{
    int iv;         // series variable: 1 = longitude, 2 = latitude, 3 = distance
    int nt;         // number of terms in series
    vector<ELPMainTerm> terms;
};

// Represents an individual term in an ELPMPP02 "perturbation" series.

struct ELPPertTerm
{
    double s, c;    // coefficients of sine and cosine
    char i[13];     // coefficients of fundamental arguments
};

// Represents a complete ELPMPP02 "perturbation" series.

struct ELPPertSeries
{
    int iv;         // series variable: 1 = longitude, 2 = latitude, 3 = distance
    int it;         // time power of series
    int nt;         // number of terms in series
    vector<ELPPertTerm> terms;
};

// This class stores ELPMPP02 lunar ephemeris series, reads them from data files,
// exports them to C++ source code, and computes lunar position/velocity from them.

class ELPMPP02
{
protected:
    ELPMainSeries mainLon;
    ELPMainSeries mainLat;
    ELPMainSeries mainDist;
    
    vector<ELPPertSeries> pertLon;
    vector<ELPPertSeries> pertLat;
    vector<ELPPertSeries> pertDist;
    
public:
    static constexpr double kMoonEarthMassRatio = 1.0 / 81.3005678;

    ELPMPP02 ( void );

    bool readSeries ( const string &datadir );

    int readMainSeries ( const string &filename, ELPMainSeries &main );
    int readPertSeries ( const string &filename, vector<ELPPertSeries> &pert );

    void printMainSeries ( ostream &out, const ELPMainSeries &main );
    void printPertSeries ( ostream &out, const vector<ELPPertSeries> &pert );
    
    static bool computePositionVelocity ( double jed, SSVector &pos, SSVector &vel );
};

#endif /* ELPMPP02_hpp */
