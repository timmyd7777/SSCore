// SSOrbit.hpp
// SSCore
//
// Created by Tim DeBenedictis on 3/7/20.
// Copyright © 2020 Southern Stars. All rights reserved.
//
// This class stores Keplerian orbital elements, computes position/velocity at a given time from them, and vice-versa.

#ifndef SSOrbit_hpp
#define SSOrbit_hpp

#include <math.h>
#include "SSMatrix.hpp"

// Stores Keplerian orbital elements, solves Kepler's equation, and computes position/velocity
// at a given time; also computes orbit from position & velocity.
// For heliocentric orbits, the reference plane is usually the J2000 ecliptic, and periapse distance is measured in AU.

struct SSOrbit
{
    double t;        // epoch of orbital elements as Julian Ephemeris Date
    double q;        // periapse distance in astronomical units for solar system objects, or in arcseconds for binary stars
    double e;        // eccentricity: 0.0 = circular, 1.0 = parabolic, > 1.0 = hyperbolic
    double i;        // inclination to reference plance in radians
    double w;        // argument of periapse in radians
    double n;        // longitude of ascending node in radians
    double m;        // mean anomaly at epoch in radians
    double mm;       // mean motion, radians per day

    static constexpr double kGravity = 6.67259e-20;                 // Newtonian gravitational constant for mass in kilograms, time in seconds, distance in kilometers [km^3 / kg / sec^2] (JPL)
    static constexpr double kGaussGravHelio = 0.01720209895;        // Gaussian gravitational constant for heliocentric orbits with time in days and distance in AU
    static constexpr double kGaussGravGeo = 0.0743669161;           // Gaussian gravitational constant for geocentric orbits with time in minutes and distance in Earth-radii
    
    SSOrbit ( void );
    SSOrbit ( double t, double q, double e, double i, double w, double n, double m, double mm );

    static double meanMotion ( double e, double q, double g = kGaussGravHelio );
    static double periapseDistance ( double e, double mm, double g = kGaussGravHelio );
    static double gravityConstant ( double e, double q, double mm );
    
    void solveKeplerEquation ( double jde, double &nu, double &r );
    static SSOrbit fromPositionVelocity ( double jde, SSVector pos, SSVector vel, double g = kGaussGravHelio );
    void toPositionVelocity ( double jde, SSVector &pos, SSVector &vel );
    void toPositionSeparation ( double jde, SSAngle &pa, double &r, double &sep );
    SSOrbit transform ( SSMatrix &m );
    void computePoints ( double nu0, int npoints, vector<SSVector> &points );

    double semiMajorAxis ( void ) { return e == 1.0 ? INFINITY : q / ( 1.0 - e ); }
    double apoapse ( void ) { return e >= 1.0 ? INFINITY : semiMajorAxis() * ( 1.0 + e ); }
    double period ( void ) { return e < 1.0 ? M_2PI / mm : INFINITY; }
    
    static SSOrbit getMercuryOrbit ( double jde );
    static SSOrbit getVenusOrbit ( double jde );
    static SSOrbit getEarthOrbit ( double jde );
    static SSOrbit getMarsOrbit ( double jde );
    static SSOrbit getJupiterOrbit ( double jde );
    static SSOrbit getSaturnOrbit ( double jde );
    static SSOrbit getUranusOrbit ( double jde );
    static SSOrbit getNeptuneOrbit ( double jde );
    static SSOrbit getPlutoOrbit ( double jde );
};

#endif /* SSOrbit_hpp */
