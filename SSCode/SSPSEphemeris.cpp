// SSPSEphemeris.cpp
// SSCore
//
// Created by Tim DeBenedictis on 4/14/20.
// Copyright © 2020 Southern Stars. All rights reserved.

#include "SSPSEphemeris.hpp"
#include "SSTime.hpp"

// Computes Sun's heliocentric position (pos) and velocity (vel) at a specific
// Julian Ephemersi Date (jed) referred to the ecliptic of that date, in units
// of AU and AU per day. Returns equivalent heliocentric spherical coordinates:
// ecliptic longitude and latitude in radians, and heliocentic distance in AU.

SSSpherical SSPSEphemeris::sun ( double jed, SSVector &pos, SSVector &vel )
{
    pos = vel = SSVector ( 0.0, 0.0, 0.0 );
    return SSSpherical ( 0.0, 0.0, 0.0 );
}

// Computes Mercury's heliocentric position (pos) and velocity (vel) at a specific
// Julian Ephemersi Date (jed) referred to the ecliptic of that date, in units
// of AU and AU per day. Returns equivalent heliocentric spherical coordinates:
// ecliptic longitude and latitude in radians, and heliocentic distance in AU.

// For Mercury, Venus, Earth, and Mars we can ignore all perturbations.

SSSpherical SSPSEphemeris::mercury ( double jed, SSVector &pos, SSVector &vel )
{
    return toEcliptic ( mercuryOrbit ( jed ), pos, vel );
}

// Computes Venus's heliocentric position (pos) and velocity (vel) at a specific
// Julian Ephemersi Date (jed) referred to the ecliptic of that date, in units
// of AU and AU per day. Returns equivalent heliocentric spherical coordinates:
// ecliptic longitude and latitude in radians, and heliocentic distance in AU.

SSSpherical SSPSEphemeris::venus ( double jed, SSVector &pos, SSVector &vel )
{
    return toEcliptic ( venusOrbit ( jed ), pos, vel );
}

// Computes Earth's heliocentric position (pos) and velocity (vel) at a specific
// Julian Ephemersi Date (jed) referred to the ecliptic of that date, in units
// of AU and AU per day. Returns equivalent heliocentric spherical coordinates:
// ecliptic longitude and latitude in radians, and heliocentic distance in AU.

SSSpherical SSPSEphemeris::earth ( double jed, SSVector &pos, SSVector &vel )
{
    return toEcliptic ( earthOrbit ( jed ), pos, vel );
}

// Computes Mars's heliocentric position (pos) and velocity (vel) at a specific
// Julian Ephemersi Date (jed) referred to the ecliptic of that date, in units
// of AU and AU per day. Returns equivalent heliocentric spherical coordinates:
// ecliptic longitude and latitude in radians, and heliocentic distance in AU.

SSSpherical SSPSEphemeris::mars ( double jed, SSVector &pos, SSVector &vel )
{
    return toEcliptic ( marsOrbit ( jed ), pos, vel );
}

// Computes Jupiter's heliocentric position (pos) and velocity (vel) at a specific
// Julian Ephemersi Date (jed) referred to the ecliptic of that date, in units
// of AU and AU per day. Returns equivalent heliocentric spherical coordinates:
// ecliptic longitude and latitude in radians, and heliocentic distance in AU.

SSSpherical SSPSEphemeris::jupiter ( double jed, SSVector &pos, SSVector &vel )
{
    double d = jed - SSTime::kJ2000 + 1.5;
    double Mj = mod360 (  19.8950 + 0.0830853001 * d );   // Mean anomaly of Jupiter, degrees
    double Ms = mod360 ( 316.9670 + 0.0334442282 * d );   // Mean anomaly of Saturn, degrees
    
    SSSpherical ecl = toEcliptic ( jupiterOrbit ( jed ), pos, vel );
    
    // perturbations
    
    ecl.lon += degtorad (
            -0.332 * sindeg ( 2 * Mj - 5 * Ms - 67.6 )  // great Jupiter-Saturn term
            -0.056 * sindeg ( 2 * Mj - 2 * Ms + 21 )
            +0.042 * sindeg ( 3 * Mj - 5 * Ms + 21 )
            -0.036 * sindeg ( Mj - 2 * Ms )
            +0.022 * cosdeg ( Mj - Ms )
            +0.023 * sindeg ( 2 * Mj - 3 * Ms + 52 )
            -0.016 * sindeg ( Mj - 5 * Ms - 69 ) );
    
    pos = SSVector ( ecl );
    return ecl;
}

// Computes Saturn's heliocentric position (pos) and velocity (vel) at a specific
// Julian Ephemersi Date (jed) referred to the ecliptic of that date, in units
// of AU and AU per day. Returns equivalent heliocentric spherical coordinates:
// ecliptic longitude and latitude in radians, and heliocentic distance in AU.

SSSpherical SSPSEphemeris::saturn ( double jed, SSVector &pos, SSVector &vel )
{
    double d = jed - SSTime::kJ2000 + 1.5;
    double Mj = mod360 (  19.8950 + 0.0830853001 * d );   // Mean anomaly of Jupiter, degrees
    double Ms = mod360 ( 316.9670 + 0.0334442282 * d );   // Mean anomaly of Saturn, degrees
    
    SSSpherical ecl = toEcliptic ( saturnOrbit ( jed ), pos, vel );
    
    // perturbations
    
    ecl.lon += degtorad (
            +0.812 * sindeg ( 2 * Mj - 5 * Ms - 67.6 )  // great Jupiter-Saturn term
            -0.229 * cosdeg ( 2 * Mj - 4 * Ms - 2 )
            +0.119 * sindeg ( Mj - 2 * Ms - 3 )
            +0.046 * sindeg ( 2 * Mj - 6 * Ms - 69 )
            +0.014 * sindeg ( Mj - 3 * Ms + 32 ) );

    ecl.lat += degtorad (
            -0.020 * cosdeg ( 2 * Mj - 4 * Ms - 2 )
            +0.018 * sindeg ( 2 * Mj - 6 * Ms - 49 ) );

    pos = SSVector ( ecl );
    return ecl;
}

// Computes Uranus's heliocentric position (pos) and velocity (vel) at a specific
// Julian Ephemersi Date (jed) referred to the ecliptic of that date, in units
// of AU and AU per day. Returns equivalent heliocentric spherical coordinates:
// ecliptic longitude and latitude in radians, and heliocentic distance in AU.

SSSpherical SSPSEphemeris::uranus ( double jed, SSVector &pos, SSVector &vel )
{
    double d = jed - SSTime::kJ2000 + 1.5;
    double Mj = mod360 (  19.8950 + 0.0830853001 * d );   // Mean anomaly of Jupiter, degrees
    double Ms = mod360 ( 316.9670 + 0.0334442282 * d );   // Mean anomaly of Saturn, degrees
    double Mu = mod360 ( 142.5905 + 0.011725806 * d );    // Mean anomaly of Uranus, degrees

    SSSpherical ecl = toEcliptic ( uranusOrbit ( jed ), pos, vel );
    
    // perturbations
    
    ecl.lon += degtorad (
            +0.040 * sindeg ( Ms - 2 * Mu + 6 )
            +0.035 * sindeg ( Ms - 3 * Mu + 33 ) // great Saturn-Uranus term
            -0.015 * sindeg ( Mj - Mu + 20 ) );

    pos = SSVector ( ecl );
    return ecl;
}

// Computes Neptune's heliocentric position (pos) and velocity (vel) at a specific
// Julian Ephemersi Date (jed) referred to the ecliptic of that date, in units
// of AU and AU per day. Returns equivalent heliocentric spherical coordinates:
// ecliptic longitude and latitude in radians, and heliocentic distance in AU.

SSSpherical SSPSEphemeris::neptune ( double jed, SSVector &pos, SSVector &vel )
{
    // the "great Uranus-Neptune term", which has a period of 4220 years and an amplitude of about one degree
    // is not included here, instead it is included in the orbital elements of Uranus and Neptune.
    
    return toEcliptic ( neptuneOrbit ( jed ), pos, vel );
}

// Computes Pluto's heliocentric position (pos) and velocity (vel) at a specific
// Julian Ephemersi Date (jed) referred to the ecliptic of that date, in units
// of AU and AU per day. Returns equivalent heliocentric spherical coordinates:
// ecliptic longitude and latitude in radians, and heliocentic distance in AU.
// The formulae below are a curve-fit to numerical integration, and are valid
// from about 1800 to 2100. Pluto's velocity is not computed.

SSSpherical SSPSEphemeris::pluto ( double jed, SSVector &pos, SSVector &vel )
{
    double d = jed - SSTime::kJ2000 + 1.5;
    double S = degtorad ( mod360 (  50.03  +  0.033459652 * d ) );
    double P = degtorad ( mod360 ( 238.95  +  0.003968789 * d ) );

    double lonecl = degtorad ( mod360 ( 238.9508 + 0.00400703 * d
                  - 19.799 * sin ( P )     + 19.848 * cos ( P )
                  +  0.897 * sin ( 2 * P ) -  4.956 * cos ( 2 * P )
                  +  0.610 * sin ( 3 * P ) +  1.211 * cos ( 3 * P )
                  -  0.341 * sin ( 4 * P ) -  0.190 * cos ( 4 * P )
                  +  0.128 * sin ( 5 * P ) -  0.034 * cos ( 5 * P )
                  -  0.038 * sin ( 6 * P ) +  0.031 * cos ( 6 * P )
                  +  0.020 * sin ( S - P ) -  0.010 * cos ( S - P ) ) );

     double latecl = degtorad ( -3.9082
                   - 5.453 * sin ( P )     - 14.975 * cos ( P )
                   + 3.527 * sin ( 2 * P ) +  1.673 * cos ( 2 * P )
                   - 1.051 * sin ( 3 * P ) +  0.328 * cos ( 3 * P )
                   + 0.179 * sin ( 4 * P ) -  0.292 * cos ( 4 * P )
                   + 0.019 * sin ( 5 * P ) +  0.100 * cos ( 5 * P )
                   - 0.031 * sin ( 6 * P ) -  0.026 * cos ( 6 * P )
                                           +  0.011 * cos ( S - P ) );

    double r = 40.72
             +  6.68 * sin ( P )     + 6.90 * cos ( P )
             -  1.18 * sin ( 2 * P ) - 0.03 * cos ( 2 * P )
             +  0.15 * sin ( 3 * P ) - 0.14 * cos ( 3 * P );

    SSSpherical ecl ( lonecl, latecl, r );
    pos = SSVector ( ecl );
    return ecl;
}

// Computes Moon's geocentric position (pos) and velocity (vel) at a specific
// Julian Ephemersi Date (jed) referred to the ecliptic of that date, in units
// of Earth-radii and Earth-radii per day. Returns equivalent geocentric spherical
// coordinates: ecliptic longitude and latitude in radians, and geocentric distance
// in Earth-radii.

SSSpherical SSPSEphemeris::moon ( double jed, SSVector &pos, SSVector &vel )
{
    double d = jed - SSTime::kJ2000 + 1.5;
    double Ms = mod2pi ( degtorad ( 356.0470 +  0.9856002585 * d ) );
    double Mm = mod2pi ( degtorad ( 115.3654 + 13.0649929509 * d ) );
    double Nm = mod2pi ( degtorad ( 125.1228 -  0.0529538083 * d ) );
    double ws = mod2pi ( degtorad ( 282.9404 +  4.70935e-5 * d ) );
    double wm = mod2pi ( degtorad ( 318.0634 +  0.1643573223 * d ) );
    double Ls = mod2pi ( Ms + ws );
    double Lm = mod2pi ( Mm + wm + Nm );
    double D = mod2pi ( Lm - Ls );
    double F = mod2pi ( Lm - Nm );
    
    SSSpherical ecl = toEcliptic ( moonOrbit ( jed ), pos, vel );
    
    // perturations
    
    ecl.lon += degtorad (
            -1.274 * sin ( Mm - 2 * D )          // (the Evection)
            +0.658 * sin ( 2 * D )               // (the Variation)
            -0.186 * sin ( Ms )                  // (the Yearly Equation)
            -0.059 * sin ( 2 * Mm - 2 * D )
            -0.057 * sin ( Mm - 2 * D + Ms )
            +0.053 * sin ( Mm + 2 * D )
            +0.046 * sin ( 2 * D - Ms )
            +0.041 * sin ( Mm - Ms)
            -0.035 * sin ( D )                   // (the Parallactic Equation)
            -0.031 * sin ( Mm + Ms )
            -0.015 * sin ( 2 * F - 2 * D )
            +0.011 * sin ( Mm - 4 * D ) );

    ecl.lat += degtorad (
            -0.173 * sin ( F - 2 * D )
            -0.055 * sin ( Mm - F - 2 * D )
            -0.046 * sin ( Mm + F - 2 * D )
            +0.033 * sin ( F + 2 * D )
            +0.017 * sin ( 2 * Mm + F ) );

    ecl.rad += -0.58 * cos ( Mm - 2 * D )
            -0.46 * cos ( 2 * D );

    pos = SSVector ( ecl );
    return ecl;
}

// Returns Mercury's mean heliocentric orbital elements referred to the ecliptic
// of a specific Julian Ephemeris Date (jed).  Periapse distance (orbit.q) is in AU.

SSOrbit SSPSEphemeris::mercuryOrbit ( double jed )
{
    double d = jed - SSTime::kJ2000 + 1.5;
    double a = 0.387098; // AU
    double e = 0.205635 + 5.59e-10 * d;
    double i = degtorad ( 7.0047 + 5.00e-8 * d );
    double M = mod2pi ( degtorad ( 168.6562 + 4.0923344368 * d ) );
    double N = mod2pi ( degtorad (  48.3313 + 3.24587e-5 * d ) );
    double w = mod2pi ( degtorad (  29.1241 + 1.01444e-5 * d ) );

    return SSOrbit ( jed, a * ( 1.0 - e ), e, i, w, N, M, degtorad ( 4.0923344368 ) );
}

// Returns Venus's mean heliocentric orbital elements referred to the ecliptic
// of a specific Julian Ephemeris Date (jed).  Periapse distance (orbit.q) is in AU.

SSOrbit SSPSEphemeris::venusOrbit ( double jed )
{
    double d = jed - SSTime::kJ2000 + 1.5;
    double a = 0.723330; // AU
    double e = 0.006773 - 1.302e-9 * d;
    double i = degtorad ( 3.3946 + 2.75E-8 * d );
    double M = mod2pi ( degtorad ( 48.0052 + 1.6021302244 * d ) );
    double N = mod2pi ( degtorad ( 76.6799 + 2.46590E-5 * d ) );
    double w = mod2pi ( degtorad ( 54.8910 + 1.38374E-5 * d ) );

    return SSOrbit ( jed, a * ( 1.0 - e ), e, i, w, N, M, degtorad ( 1.6021302244 ) );
}

// Returns Earth's mean heliocentric orbital elements referred to the ecliptic
// of a specific Julian Ephemeris Date (jed).  Periapse distance (orbit.q) is in AU.

SSOrbit SSPSEphemeris::earthOrbit ( double jed )
{
    double d = jed - SSTime::kJ2000 + 1.5;
    double a = 1.000000; // AU
    double e = 0.016709 - 1.151E-9 * d;
    double i = 0.0;
    double M = mod2pi ( degtorad ( 356.0470 + 0.9856002585 * d ) );
    double N = 0.0;
    double w = mod2pi ( degtorad ( 102.9404 + 4.70935e-5 * d ) );

    return SSOrbit ( jed, a * ( 1.0 - e ), e, i, w, N, M, degtorad ( 0.9856002585 ) );
}

// Returns Mars's mean heliocentric orbital elements referred to the ecliptic
// of a specific Julian Ephemeris Date (jed).  Periapse distance (orbit.q) is in AU.

SSOrbit SSPSEphemeris::marsOrbit ( double jed )
{
    double d = jed - SSTime::kJ2000 + 1.5;
    double a = 1.523688; // AU
    double e = 0.093405 + 2.516E-9 * d;
    double i = degtorad ( 1.8497 - 1.78E-8 * d );
    double M = mod2pi ( degtorad (  18.6021 + 0.5240207766 * d ) );
    double N = mod2pi ( degtorad (  49.5574 + 2.11081e-5 * d ) );
    double w = mod2pi ( degtorad ( 286.5016 + 2.92961E-5 * d ) );

    return SSOrbit ( jed, a * ( 1.0 - e ), e, i, w, N, M, degtorad ( 0.5240207766 ) );
}

// Returns Jupiter's mean heliocentric orbital elements referred to the ecliptic
// of a specific Julian Ephemeris Date (jed).  Periapse distance (orbit.q) is in AU.

SSOrbit SSPSEphemeris::jupiterOrbit ( double jed )
{
    double d = jed - SSTime::kJ2000 + 1.5;
    double a = 5.20256; // AU
    double e = 0.048498 + 4.469E-9 * d;
    double i = degtorad ( 1.3030 - 1.557E-7 * d );
    double M = mod2pi ( degtorad (  19.8950 + 0.0830853001 * d ) );
    double N = mod2pi ( degtorad ( 100.4542 + 2.76854E-5 * d ) );
    double w = mod2pi ( degtorad ( 273.8777 + 1.64505E-5 * d ) );

    return SSOrbit ( jed, a * ( 1.0 - e ), e, i, w, N, M, degtorad ( 0.0830853001 ) );
}

// Returns Saturn's mean heliocentric orbital elements referred to the ecliptic
// of a specific Julian Ephemeris Date (jed).  Periapse distance (orbit.q) is in AU.

SSOrbit SSPSEphemeris::saturnOrbit ( double jed )
{
    double d = jed - SSTime::kJ2000 + 1.5;
    double a = 9.55475; // AU
    double e = 0.055546 - 9.499E-9 * d;
    double i = degtorad ( 2.4886 - 1.081E-7 * d );
    double M = mod2pi ( degtorad ( 316.9670 + 0.0334442282 * d ) );
    double N = mod2pi ( degtorad ( 113.6634 + 2.38980E-5 * d ) );
    double w = mod2pi ( degtorad ( 339.3939 + 2.97661E-5 * d ) );

    return SSOrbit ( jed, a * ( 1.0 - e ), e, i, w, N, M, degtorad ( 0.0334442282 ) );
}

// Returns Uranus's mean heliocentric orbital elements referred to the ecliptic
// of a specific Julian Ephemeris Date (jed).  Periapse distance (orbit.q) is in AU.

SSOrbit SSPSEphemeris::uranusOrbit ( double jed )
{
    double d = jed - SSTime::kJ2000 + 1.5;
    double a = 19.18171 - 1.55E-8 * d; // AU
    double e = 0.047318 + 7.45E-9 * d;
    double i = degtorad ( 0.7733 + 1.9E-8 * d );
    double M = mod2pi ( degtorad ( 142.5905 + 0.011725806 * d ) );
    double N = mod2pi ( degtorad (  74.0005 + 1.3978E-5 * d ) );
    double w = mod2pi ( degtorad (  96.6612 + 3.0565E-5 * d ) );

    return SSOrbit ( jed, a * ( 1.0 - e ), e, i, w, N, M, degtorad ( 0.011725806 ) );
}

// Returns Neptune's mean heliocentric orbital elements referred to the ecliptic
// of a specific Julian Ephemeris Date (jed).  Periapse distance (orbit.q) is in AU.

SSOrbit SSPSEphemeris::neptuneOrbit ( double jed )
{
    double d = jed - SSTime::kJ2000 + 1.5;
    double a = 30.05826 + 3.313E-8 * d; // AU
    double e = 0.008606 + 2.15E-9 * d;
    double i = degtorad ( 1.7700 - 2.55E-7 * d );
    double M = mod2pi ( degtorad ( 260.2471 + 0.005995147 * d ) );
    double N = mod2pi ( degtorad ( 131.7806 + 3.0173E-5 * d ) );
    double w = mod2pi ( degtorad ( 272.8461 - 6.027E-6 * d ) );

    return SSOrbit ( jed, a * ( 1.0 - e ), e, i, w, N, M, degtorad ( 0.005995147 ) );
}

// Returns Moon's mean geocentric orbital elements referred to the ecliptic
// of Julian Ephemeris Date (jed).  Periapse distance (orbit.q) is in Earth-radii.

SSOrbit SSPSEphemeris::moonOrbit ( double jed )
{
    double d = jed - SSTime::kJ2000 + 1.5;
    double a = 60.2666; // Earth radii
    double e = 0.054900;
    double i = degtorad ( 5.1454 );
    double M = mod2pi ( degtorad ( 115.3654 + 13.0649929509 * d ) );
    double N = mod2pi ( degtorad ( 125.1228 -  0.0529538083 * d ) );
    double w = mod2pi ( degtorad ( 318.0634 +  0.1643573223 * d ) );

    return SSOrbit ( jed, a * ( 1.0 - e ), e, i, w, N, M, degtorad ( 13.0649929509 ) );
}

// Converts ecliptic spherical coordinates (ecliptic) at a particular Julian Ephemeris Date (jed)
// to equatorial rectangular coordinates at a particular precession epoch (epoch), expressed as a
// year, like 2000.0. For current equatorial coordinates (i.e. epoch equals jed), pass 0 for epoch.

SSVector SSPSEphemeris::toEquatorial ( SSSpherical ecliptic, double jed, double epoch )
{
    SSVector equatorial;

    double d = jed - SSTime::kJ2000 + 1.5;
    double lon_corr = epoch ? 3.82394E-5 * ( ( epoch - 2000.0 ) - d ) : 0.0;    // precession correction, degrees
    double lonecl = ecliptic.lon + degtorad ( lon_corr );
    double latecl = ecliptic.lat;
    double x = ecliptic.rad * cos ( lonecl ) * cos ( latecl );
    double y = ecliptic.rad * sin ( lonecl ) * cos ( latecl );
    double z = ecliptic.rad * sin ( latecl );
    double ecl = degtorad ( 23.4393 - 3.563e-7 * d );   // obliquity of the ecliptic
    
    equatorial.x = x;
    equatorial.y = y * cos ( ecl ) - z * sin ( ecl );
    equatorial.z = y * sin ( ecl ) + z * cos ( ecl );

    return equatorial;
}

// Given an object's orbital elements (orbit), computes its ecliptic position (pos)
// and velocity (vel) vectors; and returns its ecliptic longitude, latitude, and
// radial distance from its primary at the epoch of the orbit (orbit.t).
// Returned ecliptic lon and lat are in radians, referred to the same equinox
// as orbit.t. Returned distance, position, and velocity are in same units
// as periapse (orbit.q).

SSSpherical SSPSEphemeris::toEcliptic ( SSOrbit orbit, SSVector &pos, SSVector &vel )
{
    orbit.toPositionVelocity ( orbit.t, pos, vel );

    double lonecl = atan2 ( pos.y, pos.x );
    double latecl = atan2 ( pos.z, sqrt ( pos.x * pos.x + pos.y * pos.y ) );
    double r = pos.magnitude();

    return SSSpherical ( lonecl, latecl, r );
}

// Given an object's orbital elements (orbit), computes its ecliptic longitude, latitude,
// and radial distance from its primary at the epoch of the orbit (orbit.t).
// Returned ecliptic lon and lat are in radians, referred to the same equinox
// as orbit.t. Returned distance is in same units as periapse (orbit.q).

SSSpherical SSPSEphemeris::toEcliptic ( SSOrbit orbit )
{
    double e = orbit.e;
    double a = orbit.q / ( 1.0 - e );
    double i = orbit.i;
    double w = orbit.w;
    double N = orbit.n;
    double M = orbit.m;
    double E = M + e * sin ( M ) * ( 1.0 + e * cos ( M ) ), E1 = E, E0 = E + 1.0;

    // solve Kepler's equation
    
    while ( fabs ( E1 - E0 ) > degtorad ( 0.0001 ) )
    {
        E0 = E1;
        E1 = E0 - ( E0 - e * sin ( E0 ) - M ) / ( 1 - e * cos ( E0 ) );
    }
    
    E = E1;

    // compute distance from primary and true anomaly
    
    double xv = a * ( cos( E ) - e );
    double yv = a * ( sqrt ( 1.0 - e * e ) * sin ( E ) );

    double v = atan2 ( yv, xv );
    double r = sqrt ( xv * xv + yv * yv );

    // compute position in 3-dimensional space
    
    double xh = r * ( cos ( N ) * cos ( v + w ) - sin ( N ) * sin ( v + w ) * cos ( i ) );
    double yh = r * ( sin ( N ) * cos ( v + w ) + cos ( N ) * sin ( v + w ) * cos ( i ) );
    double zh = r * ( sin ( v + w ) * sin ( i ) );

    // ecliptic longitude and latitude
    
    double lonecl = atan2 ( yh, xh );
    double latecl = atan2 ( zh, sqrt ( xh * xh + yh * yh ) );
    
    return SSSpherical ( lonecl, latecl, r );
}
