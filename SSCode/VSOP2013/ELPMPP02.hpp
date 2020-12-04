// ELPMPP02.hpp
//
// Created by Tim DeBenedictis on 4/27/20.
// Copyright © 2020 Southern Stars. All rights reserved.
//
// This is a C++ wrapper around a translation by Kam Seto from FORTRAN code
// supplied with the original ELP/MPP02 lunar ephemeris, described here:
// "The lunar theory ELP revisited. Introduction of new planetary perturbations",
// J. Chapront & G. Francou, Astronomy & Astrophysics 404, 735-742 (2003) with machine-
// readable files here: ftp://cyrano-se.obspm.fr/pub/2_lunar_solutions/2_elpmpp02/
// This version optionally embeds the Chapront series data files embedded in source code
// to avoid reading them from the filesystem. It also computes the Moon's velocity.
// The ELPMPP02 series embedded in this C++ code contains 1/5th of the "main problem" terms
// and 1/10th of the "perturbation" terms in the full series, resulting in a 5-10x performance
// increase and code-size decrease from the full series, yet maintains sub-arcsecond accuracy.

#ifndef ELPMPP02_hpp
#define ELPMPP02_hpp

#include <stdint.h>
#include <vector>

#include "SSVector.hpp"

#ifndef ELPMPP02_EMBED_SERIES
#define ELPMPP02_EMBED_SERIES 1     // 1 to include embedded series; 0 to use external data files only
#endif

using namespace std;

// Represents an individual term in an ELPMPP02 "main problem" series.

struct ELPMainTerm
{
    int8_t i[4];      // coefficients of Delaunay arguments
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
    int8_t i[13];     // coefficients of fundamental arguments
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

    // Reads ELPMPP02 series from external data files, or initializes from embedded C++ data.

    bool readSeries ( const string &datadir );
    static bool initSeries ( void );

    // Reads Chapront's original ELPMPP02 "main problem" and "perturbation" series data files

    int readMainSeries ( const string &filename, ELPMainSeries &main );
    int readPertSeries ( const string &filename, vector<ELPPertSeries> &pert );

    // Exports series to C++ code

    void printMainSeries ( ostream &out, const ELPMainSeries &main );
    void printPertSeries ( ostream &out, const vector<ELPPertSeries> &pert );

    // Computes Moon's geocentric position and velocity in AU and AU/day in J2000 equatorial frame (ICRS)

    bool computePositionVelocity ( double jed, SSVector &pos, SSVector &vel );
};

#endif /* ELPMPP02_hpp */
