//
//  SSOrbit.hpp
//  SSCore
//
//  Created by Tim DeBenedictis on 3/7/20.
//  Copyright © 2020 Southern Stars. All rights reserved.
//  Class for storing Keplerian orbital elements, computing position/velocity at a given time from them, and vice-versa.

#ifndef SSOrbit_hpp
#define SSOrbit_hpp

#include <stdio.h>
#include "SSVector.hpp"

// Stores Keplerian orbital elements.  For heliocentric orbits, the reference plane
// is usually the J2000 ecliptic, and the periapse distance is measured in AU.

class SSOrbit
{
	double		t;		// epoch of orbital elements as Julian Ephemeris Date
	double		q;		// periapse distance in astronomical units
	double		e;		// eccentricity: 0.0 = circular, 1.0 = parabolic, > 1.0 = hyperbolic
	double		i;		// inclination to reference plance in radians
	double		w;		// argument of peripse in radians
	double		n;		// longitude of ascending node in radians
	double		m;		// mean anomaly at epoch in radians
	double		mm;		// mean motion, radians per day

public:
	
	static const int	kMaxIterations = 1000;		// Maximum number of iterations for solving Kepler's equation
	static constexpr double	kTolerance = 1.0e-9;	// Tolerance for solving Kepler's eqn is about 0.0002 arcsec
	static constexpr double	kGaussianGravityConstant = 1.0;
//	static double	mu = kGaussianGravityConstant;

	SSOrbit ( double t, double q, double e, double i, double w, double n, double m, double mm );
	
	void solveKeplerEquation ( double jde, double &nu, double &r );
	void inverseKeplerEquation ( double nu, double r );
	
	static SSOrbit fromPositionVelocity ( double jde, double mu, SSVector pos, SSVector vel );
	void toPositionVelocity ( double jde, SSVector &pos, SSVector &vel );

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
