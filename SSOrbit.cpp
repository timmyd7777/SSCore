//
//  SSOrbit.cpp
//  SSCore
//
//  Created by Tim DeBenedictis on 3/7/20.
//  Copyright © 2020 Southern Stars. All rights reserved.
//

#include <math.h>
#include "SSOrbit.hpp"
#include "SSTime.hpp"

double sindeg ( double deg )
{
	return sin ( deg * SSAngle::kRadPerDeg );
}

double cosdeg ( double deg )
{
	return cos ( deg * SSAngle::kRadPerDeg );
}

double ksinkdeg ( double k, double deg )
{
	return SSAngle::kRadPerDeg * k * sin ( k * deg * SSAngle::kRadPerDeg );
}

double kcoskdeg ( double k, double deg )
{
	return SSAngle::kRadPerDeg * k * cos ( k * deg * SSAngle::kRadPerDeg );
}

// Constructs an orbit with default values of zero.

SSOrbit::SSOrbit ( void )
{
    t = q = e = i = w = n = m = mm = 0.0;
}

// Constructs an orbit from the specified set of Keplerian elements.

SSOrbit::SSOrbit ( double t, double q, double e, double i, double w, double n, double m, double mm )
{
	this->t = t;
	this->q = q;
	this->e = e;
	this->i = i;
	this->w = w;
	this->n = n;
	this->m = m;
	this->mm = mm;
}

void SSOrbit::solveKeplerEquation ( double jde, double &nu, double &r )
{
	int		i = 0;
	double	ma = m + mm * ( jde - t );
	double	ea = 0.0, ha = 0.0, delta = 0.0, delta_ea = 0.0;

	if ( q == 0.0 )
		nu = r = 0.0;

	// handle negative eccentricities

	if ( e < 0.0 )
		e = -e;

	// Elliptical orbits: use modified Newton's method per Astronomical Algorithms
	
	if ( e < 1.0 )
	{
		ma = fmod ( ma, 2.0 * M_PI );
		if ( ma < 0.0 )
			ma += 2.0 * M_PI;
		
		ea = ma;

		do
		{
			delta = ea - e * sin ( ea ) - ma;
			delta_ea = delta / ( 1.0 - e * cos ( ea ) );
			
			if ( delta_ea > 0.5 )
				delta_ea = 0.5;
			else if ( delta_ea < -0.5 )
				delta_ea = -0.5;
				
			ea -= delta_ea;
			i++;
		}
		while ( fabs ( ea ) < 1.0e8 && fabs ( delta ) > kTolerance && i < kMaxIterations );

		nu = 2.0 * atan ( sqrt ( ( 1.0 + e ) / ( 1.0 - e ) ) * tan ( ea / 2.0 ) );
		r = q * ( 1.0 + e ) / ( 1.0 + e * cos ( nu ) );
	}
	
	// Parabolic orbits
	
	if ( e == 1.0 )
	{
		double s = ma, s2 = 0.0, s3 = 0.0;
		
		do
		{
			s2 = s * s;
			s3 = s2 * s;
			s = ( 2.0 * s3 + m ) / ( 3.0 * ( s2 + 1.0 ) );
			delta = s3 + 3.0 * s - m;
			i++;
		}
		while ( fabs ( delta ) > kTolerance && i < kMaxIterations );
		
		nu = 2.0 * atan ( s );
		r = q * ( 1.0 + s2 );
	}
	
	// Hyperbolic orbits
	
	if ( e > 1.0 )
	{
		ha = asinh ( ma / e );
		do
		{
			delta = ha - e * sinh ( ha ) + m;
			ha -= delta / ( 1.0 - e * cosh ( ha ) );
			i++;
		}
		while ( fabs ( delta ) > kTolerance && i < kMaxIterations );
		
		nu = 2.0 * atan ( sqrt ( ( e + 1.0 ) / ( e - 1.0 ) ) * tanh ( ha / 2.0 ) );
		r = q * ( 1.0 + e ) / ( 1.0 + e * cos ( nu ) );
	}
}

void SSOrbit::inverseKeplerEquation ( double nu, double r )
{
	double ea, s, ha;
	
	// Elliptical Orbits
	
	if ( e < 1.0 )
	{
		ea = 2.0 * atan ( sqrt ( ( 1.0 - e ) / ( 1.0 + e ) ) * tan ( nu / 2.0 ) );
		m = ea - e * sin ( ea );
		q = r * ( 1.0 + e * cos ( nu ) ) / ( 1.0 + e );
	}

	// Parabolic orbits
	
	if ( e == 1.0 )
	{
		s = tan ( nu / 2.0 );
		m = s * s * s + 3.0 * s;
		q = r / ( 1.0 + s * s );
	}

	// Hyperbolic orbits
	
	if ( e > 1.0 )
	{
		ha = 2.0 * atanh ( sqrt ( ( e - 1.0 ) / ( e + 1.0 ) ) * tan ( nu / 2.0 ) );
		m = e * sinh ( ha ) - ha;
		q = r * ( 1.0 + e * cos ( nu ) ) / ( 1.0 + e );
	}
}

void SSOrbit::toPositionVelocity ( double jde, SSVector &pos, SSVector &vel )
{
	double a, nu, mu, r, p, h, dnu, dr;
	double u, cu, su, ci, si, cn, sn;

	solveKeplerEquation ( jde, nu, r );

	if ( e < 1.0 )	// Elliptical orbits
	{
    	a = q / ( 1.0 - e );
    	mu = a * a * a * mm * mm;
  	}
	else if ( e == 1.0 )	// Parabolic orbits
	{
		mu = 2.0 * q * q * q * mm * mm / 9.0;
	}
	else 	// e > 1.0, Hyperbolic orbits
	{
		a = q / ( e - 1.0 );
		mu = a * a * a * mm * mm;
	}

	p = q * ( 1.0 + e );
	h = sqrt ( mu * p );
	dnu = h / ( r * r );
	dr = h * e * sin ( nu ) / p;
    
	u = w + nu;
	cu = cos ( u );
	su = sin ( u );
	ci = cos ( i );
	si = sin ( i );
	cn = cos ( n );
	sn = sin ( n );

	pos.x = r * ( cu * cn - su * ci * sn );
	pos.y = r * ( cu * sn + su * ci * cn );
	pos.z = r * ( su * si );
  
	vel.x = pos.x * dr / r + r * dnu * ( - su * cn - cu * ci * sn );
	vel.y = pos.y * dr / r + r * dnu * ( - su * sn + cu * ci * cn );
	vel.z = pos.z * dr / r + r * dnu * ( cu * si );
}

SSOrbit SSOrbit::fromPositionVelocity ( double jde, double mu, SSVector pos, SSVector vel )
{
	double hx = pos.y * vel.z - pos.z * vel.y;
	double hy = pos.z * vel.x - pos.x * vel.z;
	double hz = pos.x * vel.y - pos.y * vel.x;
	
	double r2 = pos.x * pos.x + pos.y * pos.y + pos.z * pos.z;
	double v2 = vel.x * vel.x + vel.y * vel.y + vel.z * vel.z;
	double rv = pos.x * vel.x + pos.y * vel.y + pos.z * vel.z;
	
	double h2 = hx * hx + hy * hy + hz * hz;
	double h = sqrt ( h2 );
	double r = sqrt ( r2 );

	double p = h2 / mu;
	double a = 1.0 / ( 2.0 / r - v2 / mu );
	double e = sqrt ( 1.0 - p / a );
	if ( fabs ( e - 1.0 ) < 1.0e-6 )
		e = 1.0;
	
	double q = p / ( 1.0 + e );
	double nu = acos ( ( p / r - 1.0 ) / e );
	if ( rv < 0.0 )
		nu = - nu;
	
	double m = 0, mm = 0;
	
	if ( e < 1.0 )	// elliptical orbit
	{
		double ea = 2.0 * atan ( sqrt ( ( 1.0 - e ) / ( 1.0 + e ) ) * tan ( nu / 2.0 ) );
		m = ea - e * sin ( ea );
		mm = sqrt ( mu / ( a * a * a ) );
	}
	else if ( e == 1.0 )	// parabolic orbit
	{
		double s = tan ( nu / 2.0 );
		m = s * s * s + 3.0 * s;
		mm = 3.0 * sqrt ( mu / ( 2.0 * q * q * q ) );
	}
	else // e > 1.0, hyperbolic orbit
	{
		double ha = 2.0 * atanh ( sqrt ( ( e - 1.0 ) / ( e + 1.0 ) ) * tan ( nu / 2.0 ) );
		m = e * sinh ( ha ) - ha;
		mm = sqrt ( - mu / ( a * a * a ) );
	}

	double i = acos ( hz / h );
	double n = SSAngle::atan2Pi ( hx, - hy );
	double u = SSAngle::atan2Pi ( ( - pos.x * hx - pos.y * hy ) / hz, ( - pos.x * hy + pos.y * hx ) / h );
	double w = SSAngle ( u - nu ).mod2Pi();
	
	return SSOrbit ( jde, q, e, i, w, n, m, mm );
}

// Constructs Mercury's heliocentric orbital elements at a specific Julian Ephemeris Date (jde)
// referred to the J2000 ecliptic.  Only valid for years from -3000 to +3000.
// For 1800 - 2100, positions predicted using this orbit are accurate to about 1 arcminute;
// outside that interval, accuracy is about 10 arcminutes.  Based on formulae from
// E. M. Standish, "Keplerian Elements for Approximate Positions of the Major Planets",
// Solar System Dynamics Group, JPL/Caltech, https://ssd.jpl.nasa.gov/?planet_pos

SSOrbit SSOrbit::getMercuryOrbit ( double jde )
{
	double t = ( jde - SSTime::kJ2000 ) / 36525.0;
	double l, p, a, e, i, n, mm;
	
	if ( t >= -2.0 && t < 1.0 )
	{
		a  =      0.38709927 +      0.00000037 * t;
		e  =      0.20563593 +      0.00001906 * t;
		i  =      7.00497902 -      0.00594749 * t;
		l  =    252.25032350 + 149472.67411175 * t;
		p  =     77.45779628 +      0.16047689 * t;
		n  =     48.33076593 -      0.12534081 * t;
		mm = 149472.67411175 -      0.16047689;
	}
	else
	{
		a  =      0.3870943;
		e  =      0.20563651 +      0.00002123 * t;
		i  =      7.00559432 -      0.00590158 * t;
		l  =    252.25166724 + 149472.67486623 * t;
		p  =     77.45771895 +      0.15940013 * t;
		n  =     48.33961819 -      0.12214182 * t;
		mm = 149472.67486623 -      0.15940013;
	}
	
	return SSOrbit ( jde,
				 	 a * ( 1.0 - e ),
					 e,
					 SSAngle::fromDegrees ( i ),
					 SSAngle::fromDegrees ( p - n ).mod2Pi(),
					 SSAngle::fromDegrees ( n ),
					 SSAngle::fromDegrees ( l - p ).mod2Pi(),
					 SSAngle::fromDegrees ( mm / 36525.0 ) );
}

// Constructs Venus's orbital elements at a specific Julian Ephemeris Date (jde)
// referred to the J2000 ecliptic.  See comments for getMercuryOrbit regarding
// validity range, accuracy, and source.

SSOrbit SSOrbit::getVenusOrbit ( double jde )
{
	double t = ( jde - SSTime::kJ2000 ) / 36525.0;
	double l, p, a, e, i, n, mm;

	if ( t >= -2.0 && t < 1.0 )
	{
		a  =     0.72333566 +     0.00000390 * t;
		e  =     0.00677672 +     0.00004107 * t;
		i  =     3.39467605 -     0.00078890 * t;
		l  =   181.97909950 + 58517.81538729 * t;
		p  =   131.60246718 +     0.00268329 * t;
		n  =    76.67984255 -     0.27769418 * t;
		mm = 58517.81538729 -     0.00268329;
	}
	else
	{
		a  =     0.72332102 -     0.00000026 * t;
		e  =     0.00676399 -     0.00005107 * t;
		i  =     3.39777545 +     0.00043494 * t;
		l  =   181.97970850 + 58517.81560260 * t;
		p  =   131.76755713 +     0.05679648 * t;
		n  =    76.67261496 -     0.27274174 * t;
		mm = 58517.81560260 -     0.05679648;
	}
	
	return SSOrbit ( jde,
					 a * ( 1.0 - e ),
					 e,
					 SSAngle::fromDegrees ( i ),
					 SSAngle::fromDegrees ( p - n ).mod2Pi(),
					 SSAngle::fromDegrees ( n ),
					 SSAngle::fromDegrees ( l - p ).mod2Pi(),
					 SSAngle::fromDegrees ( mm / 36525.0 ) );
}

// Constructs Earth's orbital elements at a specific Julian Ephemeris Date (jde)
// referred to the J2000 ecliptic.  See comments for getMercuryOrbit regarding
// validity range, accuracy, and source.

SSOrbit SSOrbit::getEarthOrbit ( double jde )
{
	double t = ( jde - SSTime::kJ2000 ) / 36525.0;
	double l, p, a, e, i, n, mm;

	if ( t >= -2.0 && t < 1.0 )
	{
		a  =     1.00000261 +     0.00000562 * t;
		e  =     0.01671123 -     0.00004392 * t;
		i  =    -0.00001531 -     0.01294668 * t;
		l  =   100.46457166 + 35999.37244981 * t;
		p  =   102.93768193 +     0.32327364 * t;
		n  =     0.0;
		mm = 35999.37244981 -     0.32327364;
	}
	else
	{
		a  =     1.00000018 -     0.00000003 * t;
		e  =     0.01673163 -     0.00003661 * t;
		i  =    -0.00054346 -     0.01337178 * t;
		l  =   100.46691572 + 35999.37306329 * t;
		p  =   102.93005885 +     0.31795260 * t;
		n  =    -5.11260389 -     0.24123856 * t;
		mm = 35999.37306329 -     0.31795260;
	}

	return SSOrbit ( jde,
					 a * ( 1.0 - e ),
					 e,
					 SSAngle::fromDegrees ( i ),
					 SSAngle::fromDegrees ( p - n ).mod2Pi(),
					 SSAngle::fromDegrees ( n ),
					 SSAngle::fromDegrees ( l - p ).mod2Pi(),
					 SSAngle::fromDegrees ( mm / 36525.0 ) );
}

// Constructs Mars's orbital elements at a specific Julian Ephemeris Date (jde)
// referred to the J2000 ecliptic.  See comments for getMercuryOrbit regarding
// validity range, accuracy, and source.

SSOrbit SSOrbit::getMarsOrbit ( double jde )
{
	double t = ( jde - SSTime::kJ2000 ) / 36525.0;
	double l, p, a, e, i, n, mm;

	if ( t >= -2.0 && t < 1.0 )
	{
		a  =     1.52371034 -     0.00001847 * t;
		e  =     0.09339410 +     0.00007882 * t;
		i  =     1.84969142 -     0.00813131 * t;
		l  =    -4.55343205 + 19140.30268499 * t;
		p  =   -23.94362959 +     0.44441088 * t;
		n  =    49.55953891 -     0.29257343 * t;
		mm = 19140.30268499 -     0.44441088;
	}
	else
	{
		a  =     1.52371243 +     0.00000097 * t;
		e  =     0.09336511 +     0.00009149 * t;
		i  =     1.85181869 -     0.00724757 * t;
		l  =    -4.56813164 + 19140.29934243 * t;
		p  =   -23.91744784 +     0.45223625 * t;
		n  =    49.71320984 -     0.26852431 * t;
		mm = 19140.29934243 -     0.45223625;
	}
	
	return SSOrbit ( jde,
					 a * ( 1.0 - e ),
					 e,
					 SSAngle::fromDegrees ( i ),
					 SSAngle::fromDegrees ( p - n ).mod2Pi(),
					 SSAngle::fromDegrees ( n ),
					 SSAngle::fromDegrees ( l - p ).mod2Pi(),
					 SSAngle::fromDegrees ( mm / 36525.0 ) );
}

// Constructs Jupiter's orbital elements at a specific Julian Ephemeris Date (jde)
// referred to the J2000 ecliptic.  See comments for getMercuryOrbit regarding
// validity range, accuracy, and source.

SSOrbit SSOrbit::getJupiterOrbit ( double jde )
{
	double t = ( jde - SSTime::kJ2000 ) / 36525.0;
	double l, p, a, e, i, n, mm;

	if ( t >= -2.0 && t < 1.0 )
	{
		a  =    5.20288700 -    0.00011607 * t;
		e  =    0.04838624 -    0.00013253 * t;
		i  =    1.30439695 -    0.00183714 * t;
		l  =   34.39644051 + 3034.74612775 * t;
		p  =   14.72847983 +    0.21252668 * t;
		n  =  100.47390909 +    0.20469106 * t;
		mm = 3034.74612775 -    0.21252668;
	}
	else
	{
		a  =     5.20248019 - 0.00002864 * t;
		e  =     0.04853590 + 0.00018026 * t;
		i  =     1.29861416 - 0.00322699 * t;
		l  =    34.33479152 + 3034.90371757 * t
						    - 0.00012452 * t * t
		                    + 0.06064060 * cosdeg ( 38.35125 * t )
						    - 0.35635438 * sindeg ( 38.35125 * t );
		p  =    14.27495244 + 0.18199196 * t;
		n  =   100.29282564 + 0.13024619 * t;
		mm =  3034.90371757 - 0.18199196
						    - 0.00012452 * 2 * t
						    - 0.06064060 * ksinkdeg ( 38.35125, t )
						    - 0.35635438 * kcoskdeg ( 38.35125, t );
	}
	
	return SSOrbit ( jde,
					 a * ( 1.0 - e ),
					 e,
					 SSAngle::fromDegrees ( i ),
					 SSAngle::fromDegrees ( p - n ).mod2Pi(),
					 SSAngle::fromDegrees ( n ),
					 SSAngle::fromDegrees ( l - p ).mod2Pi(),
					 SSAngle::fromDegrees ( mm / 36525.0 ) );
}

// Constructs Saturn's orbital elements at a specific Julian Ephemeris Date (jde)
// referred to the J2000 ecliptic.  See comments for getMercuryOrbit regarding
// validity range, accuracy, and source.

SSOrbit SSOrbit::getSaturnOrbit ( double jde )
{
	double t = ( jde - SSTime::kJ2000 ) / 36525.0;
	double l, p, a, e, i, n, mm;

	if ( t >= -2.0 && t < 1.0 )
	{
		a  =    9.53667594 -    0.00125060 * t;
		e  =    0.05386179 -    0.00050991 * t;
		i  =    2.48599187 +    0.00193609 * t;
		l  =   49.95424423 + 1222.49362201 * t;
		p  =   92.59887831 -    0.41897216 * t;
		n  =  113.66242448 -    0.28867794 * t;
		mm = 1222.49362201 +    0.41897216;
	}
	else
	{
		a  =     9.54149883 -    0.00003065 * t;
		e  =     0.05550825 -    0.00032044 * t;
		i  =     2.49424192 +    0.00451969 * t;
		l  =    50.07571329 + 1222.11494724 * t
						    +    0.00025899 * t * t
						    -    0.13434469 * cosdeg ( 38.35125 * t )
						    +    0.87320147 * sindeg ( 38.35125 * t );
		p  =    92.86136063 +    0.54179478 * t;
		n  =   113.63998702 -    0.25015002 * t;
		mm =  1222.11494724 -    0.54179478
						    +    0.00025899 * 2 * t
						    +    0.13434469 * ksinkdeg ( 38.35125, t )
						    +    0.87320147 * kcoskdeg ( 38.35125, t );
	}

	return SSOrbit ( jde,
					 a * ( 1.0 - e ),
					 e,
					 SSAngle::fromDegrees ( i ),
					 SSAngle::fromDegrees ( p - n ).mod2Pi(),
					 SSAngle::fromDegrees ( n ),
					 SSAngle::fromDegrees ( l - p ).mod2Pi(),
					 SSAngle::fromDegrees ( mm / 36525.0 ) );
}

// Constructs Uranus's orbital elements at a specific Julian Ephemeris Date (jde)
// referred to the J2000 ecliptic.  See comments for getMercuryOrbit regarding
// validity range, accuracy, and source.

SSOrbit SSOrbit::getUranusOrbit ( double jde )
{
	double t = ( jde - SSTime::kJ2000 ) / 36525.0;
	double l, p, a, e, i, n, mm;

	if ( t >= -2.0 && t < 1.0 )
	{
		a  =   19.18916464 -   0.00196176 * t;
		e  =    0.04725744 -   0.00004397 * t;
		i  =    0.77263783 -   0.00242939 * t;
		l  =  313.23810451 + 428.48202785 * t;
		p  =  170.95427630 +   0.40805281 * t;
		n  =   74.01692503 +   0.04240589 * t;
		mm =  428.48202785 -   0.40805281;
	}
	else
	{
		a  =   19.18797948 -   0.00020455 * t;
		e  =    0.04685740 -   0.00001550 * t;
		i  =    0.77298127 -   0.00180155 * t;
		l  =  314.20276625 + 428.49512595 * t
						   +   0.00058331 * t * t
						   -   0.97731848 * cosdeg ( 7.67025 * t )
						   +   0.17689245 * sindeg ( 7.67025 * t );
		p  =  172.43404441 +   0.09266985 * t;
		n  =   73.96250215 +   0.05739699 * t;
		mm =  428.49512595 -   0.09266985
						   +   0.00058331 * 2 * t
						   +   0.97731848 * ksinkdeg ( 7.67025, t )
						   +   0.17689245 * kcoskdeg ( 7.67025, t );
	}

	return SSOrbit ( jde,
					 a * ( 1.0 - e ),
					 e,
					 SSAngle::fromDegrees ( i ),
					 SSAngle::fromDegrees ( p - n ).mod2Pi(),
					 SSAngle::fromDegrees ( n ),
					 SSAngle::fromDegrees ( l - p ).mod2Pi(),
					 SSAngle::fromDegrees ( mm / 36525.0 ) );
}

// Constructs Neptune's orbital elements at a specific Julian Ephemeris Date (jde)
// referred to the J2000 ecliptic.  See comments for getMercuryOrbit regarding
// validity range, accuracy, and source.

SSOrbit SSOrbit::getNeptuneOrbit ( double jde )
{
	double t = ( jde - SSTime::kJ2000 ) / 36525.0;
	double l, p, a, e, i, n, mm;

	if ( t >= -2.0 && t < 1.0 )
	{
		a  =  30.06992276 +   0.00026291 * t;
		e  =   0.00859048 +   0.00005105 * t;
		i  =   1.77004347 +   0.00035372 * t;
		l  = -55.12002969 + 218.45945325 * t;
		p  =  44.96476227 -   0.32241464 * t;
		n  = 131.78422574 -   0.00508664 * t;
		mm = 218.45945325 +   0.32241464;
	}
	else
	{
		a  =  30.06952752 +   0.00006447 * t;
		e  =   0.00895439 +   0.00000818 * t;
		i  =   1.77005520 +   0.00022400 * t;
		l  = 304.22289287 + 218.46515314 * t
						  -   0.00041348 * t * t
						  +   0.68346318 * cosdeg ( 7.67025 * t )
						  -   0.10162547 * sindeg ( 7.67025 * t );
		p  =  46.68158724 +   0.01009938 * t;
		n  = 131.78635853 -   0.00606302 * t;
		mm = 218.46515314 -   0.01009938
						  -   0.00041348 * 2 * t
						  -   0.68346318 * ksinkdeg ( 7.67025, t )
						  -   0.10162547 * kcoskdeg ( 7.67025, t );
	}

	return SSOrbit ( jde,
					 a * ( 1.0 - e ),
					 e,
					 SSAngle::fromDegrees ( i ),
					 SSAngle::fromDegrees ( p - n ).mod2Pi(),
					 SSAngle::fromDegrees ( n ),
					 SSAngle::fromDegrees ( l - p ).mod2Pi(),
					 SSAngle::fromDegrees ( mm / 36525.0 ) );
}

// Constructs Pluto's orbital elements at a specific Julian Ephemeris Date (jde)
// referred to the J2000 ecliptic.  See comments for getMercuryOrbit regarding
// validity range, accuracy, and source.

SSOrbit SSOrbit::getPlutoOrbit ( double jde )
{
	double t = ( jde - SSTime::kJ2000 ) / 36525.0;
	double l, p, a, e, i, n, mm;

	if ( t >= -2.0 && t <= 1.0 )
	{
		a  =  39.48211675 -    0.00031596 * t;
		e  =   0.24882730 +    0.00005170 * t;
		i  =  17.14001206 +    0.00004818 * t;
		l  = 238.92903833 + 145.20780515 * t;
		p  = 224.06891629 -   0.04062942 * t;
		n  = 110.47390909 -   0.20469106 * t;
		mm = 145.20780515 +   0.04062942 * t;
	}
	else
	{
		a  =  39.48686035 +   0.00449751 * t;
		e  =   0.24885238 +   0.00006016 * t;
		i  =  17.14104260 +   0.00000501 * t;
		l  = 238.96535011 + 145.18042903 * t - 0.01262724 * t * t;
		p  = 224.09702598 -   0.00968827 * t;
		n  = 110.30167986 -   0.00809981 * t;
		mm = 145.18042903 +   0.00968827 - 0.01262724 * 2 * t;
	}
	
	return SSOrbit ( jde,
					 a * ( 1.0 - e ),
					 e,
					 SSAngle::fromDegrees ( i ),
					 SSAngle::fromDegrees ( p - n ).mod2Pi(),
					 SSAngle::fromDegrees ( n ),
					 SSAngle::fromDegrees ( l - p ).mod2Pi(),
					 SSAngle::fromDegrees ( mm / 36525.0 ) );
}
