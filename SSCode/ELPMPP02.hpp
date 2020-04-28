// ELPMPP02.hpp
// SSTest
//
// Created by Tim DeBenedictis on 4/27/20.
// Copyright © 2020 Southern Stars. All rights reserved.

#ifndef ELPMPP02_hpp
#define ELPMPP02_hpp

#include "SSVector.hpp"

using namespace std;

struct ELPMainTerm
{
    char  i[4];     // coefficients of Delaunay arguments
    float a;        // coefficient in arcseconds for longitude/latitude, km for distance
    float b[6];     // derivates of a with respect to six constants
};

struct ELPMainSeries
{
    int iv;         // series variable: 1 = longitude, 2 = latitude, 3 = distance
    int nt;         // number of terms in series
    vector<ELPMainTerm> terms;
};

struct ELPPertTerm
{
    double s, c;    // coefficients of sine and cosine
    char i[13];     // coefficients of fundamental arguments
};

struct ELPPertSeries
{
    int iv;         // series variable: 1 = longitude, 2 = latitude, 3 = distance
    int it;         // time power of series
    int nt;         // number of terms in series
    vector<ELPPertTerm> terms;
};

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
    
    int readMainSeries ( const string &filename );
    int readPertSeries ( const string &filename );

    void printMainSeries ( ostream &out, const ELPMainSeries &main );
    void printPertSeries ( ostream &out, const vector<ELPPertSeries> &pert );
    
    static bool open ( const string &datadir );
    static bool computePositionVelocity ( double jed, SSVector &pos, SSVector &vel );
};

#endif /* ELPMPP02_hpp */
