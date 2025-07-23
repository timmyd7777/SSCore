// VSOP2013.cpp
// SSTest
//
// Created by Tim DeBenedictis on 4/26/20.
// Copyright © 2020 Southern Stars. All rights reserved.

#include "SSUtilities.hpp"
#include "SSMatrix.hpp"
#include "VSOP2013.hpp"

#include <iostream>
#include <fstream>

#define PRINT_SERIES    0       // 1 to comvert input series data files to output .cpp source code
#define TRUNC_FACTOR    100     // exported seriees truncation factor: 1 exports everything, 10 exports only first tenth; 100 exports only first hundredth, etc,

// Reads a VSOP2013 data file (filename) for the specified planet
// (iplanet) 1 = Mercury ... 9 = Pluto into this VSOP2013 object.
// Returns number of lines read from file.
// If PRINT_SERIES is #defined 1 above, exports the data file as
// C++ source code to a file with the same name as the input file
// plus ".cpp" appended.

int VSOP2013::readFile ( const string &filename, int iplanet )
{
    // Open file; return on failure.

    ifstream file ( filename );
    if ( ! file )
        return 0;

    // Read file line-by-line until we reach ene-of-file

    string line = "";
    int count = 0;

    while ( getline ( file, line ) )
    {
        count++;
        VSOP2013Series ser;
        
        ser.ip = strtoint ( line.substr ( 9, 3 ) );
        ser.iv = strtoint ( line.substr ( 12, 3 ) );
        ser.it = strtoint ( line.substr ( 15, 3 ) );
        ser.nt = strtoint ( line.substr ( 18, 7 ) );
        
        for ( int i = 0; i < ser.nt; i++ )
        {
            if ( ! getline ( file, line ) )
                break;
            
            count++;
            VSOP2013Term term;
            
            term.iphi[0]  = strtoint ( line.substr (  6, 3 ) );
            term.iphi[1]  = strtoint ( line.substr (  9, 3 ) );
            term.iphi[2]  = strtoint ( line.substr ( 12, 3 ) );
            term.iphi[3]  = strtoint ( line.substr ( 15, 3 ) );
            
            term.iphi[4]  = strtoint ( line.substr ( 19, 3 ) );
            term.iphi[5]  = strtoint ( line.substr ( 22, 3 ) );
            term.iphi[6]  = strtoint ( line.substr ( 25, 3 ) );
            term.iphi[7]  = strtoint ( line.substr ( 28, 3 ) );
            term.iphi[8]  = strtoint ( line.substr ( 31, 3 ) );
            
            term.iphi[9]  = strtoint ( line.substr ( 35, 4 ) );
            term.iphi[10] = strtoint ( line.substr ( 39, 4 ) );
            term.iphi[11] = strtoint ( line.substr ( 43, 4 ) );
            term.iphi[12] = strtoint ( line.substr ( 47, 4 ) );
            
            term.iphi[13] = strtoint ( line.substr ( 52, 6 ) );
            
            term.iphi[14] = strtoint ( line.substr ( 59, 3 ) );
            term.iphi[15] = strtoint ( line.substr ( 62, 3 ) );
            term.iphi[16] = strtoint ( line.substr ( 65, 3 ) );
            
            term.s = strtofloat64 ( line.substr ( 69, 20 ) );
            term.s *= pow ( 10.0, strtoint ( line.substr ( 89, 3 ) ) );
            
            term.c = strtofloat64 ( line.substr ( 93, 20 ) );
            term.c *= pow ( 10.0, strtoint ( line.substr ( 113, 3 ) ) );
            
            ser.terms.push_back ( term );
        }
        
        planets[iplanet-1].push_back ( ser );
    }

#if PRINT_SERIES
    ofstream outfile ( filename + ".cpp" );
    if ( outfile )
        printSeries ( outfile, planets[iplanet-1] );
#endif
    
    return count;
}

// Exports a planet's VSOP2013 series (planet) as C++ source code to an output stream (out).
// If TRUNC_FACTOR is #defined to be > 1, a trunctated subset of terms are exported.

void VSOP2013::printSeries ( ostream &out, const vector<VSOP2013Series> &planet )
{
    out << "#include \"VSOP2013.hpp\"\n" << endl;
    
    int iv = -1;
    int it = -1;
    
    for ( int s = 0; s < planet.size(); s++ )
    {
        const VSOP2013Series &ser = planet[s];
        
        int nt = (int) ser.terms.size() / TRUNC_FACTOR;
        if ( nt < 1 )
            continue;
        
        if ( ser.iv != iv )
        {
            // close preceding series, if there was one
            
            if ( iv > 0 )
                out << "} } };\n" << endl;
            
            if ( ser.iv == 1 )
                out << "static vector<VSOP2013Series> _a = {\n";
            if ( ser.iv == 2 )
                out << "static vector<VSOP2013Series> _l = {\n";
            if ( ser.iv == 3 )
                out << "static vector<VSOP2013Series> _k = {\n";
            if ( ser.iv == 4 )
                out << "static vector<VSOP2013Series> _h = {\n";
            if ( ser.iv == 5 )
                out << "static vector<VSOP2013Series> _q = {\n";
            if ( ser.iv == 6 )
                out << "static vector<VSOP2013Series> _p = {\n";
            
            iv = ser.iv;
            it = -1;
        }

        if ( ser.it != it )     // print terms header
        {
            if ( it >= 0 )
                out << "} }," << endl;   // close preceding terms

            out << formstr ( "{ %3d, %3d, %3d, %3d, {\n", ser.ip, ser.iv, ser.it, nt );
            it = ser.it;
        }
        
        for ( int k = 0; k < nt; k++ )  // print terms
        {
            VSOP2013Term term = ser.terms[k];
            
            out << "{ ";
            for ( int i = 0; i < 17; i++ )
            {
                out << formstr ( "%3hd, ", term.iphi[i] );
            }
            
            out << formstr ( "%+.15e, ", term.s );
            out << formstr ( "%+.15e }", term.c );
            
            if ( k == ser.terms.size() - 1 )
                out << "\n";
            else
                out << ",\n";
        }
    }

    out << "} } };\n" << endl;   // close final series
}

// Evaluates seventeen fundamental longitude arguments in radians (ll)
// at time (t) in Julian millenia of 365250 days from J2000 (JD 2451545.0)

void VSOP2013::evalLongitudes ( double t, double ll[17] )
{
    ll[0] =  4.402608631669 +  26087.90314068555 * t;     // Mercury
    ll[1] =  3.176134461576 +  10213.28554743445 * t;     // Venus
    ll[2] =  1.753470369433 +   6283.075850353215 * t;    // Earth-Moon Barycenter
    ll[3] =  6.203500014141 +   3340.612434145457 * t;    // Mars
    ll[4] =  4.091360003050 +   1731.170452721855 * t;    // Vesta
    ll[5] =  1.713740719173 +   1704.450855027201 * t;    // Iris
    ll[6] =  5.598641292287 +   1428.948917844273 * t;    // Bamberga
    ll[7] =  2.805136360408 +   1364.756513629990 * t;    // Ceres
    ll[8] =  2.326989734620 +   1361.923207632842 * t;    // Pallas
    ll[9]  = 0.599546107035 +    529.6909615623250 * t;   // Jupiter
    ll[10] = 0.874018510107 +    213.2990861084880 * t;   // Saturn
    ll[11] = 5.481225395663 +    74.78165903077800 * t;   // Uranus
    ll[12] = 5.311897933164 +    38.13297222612500 * t;   // Neptune
    ll[13] =                      0.3595362285049309 * t; // Pluto (mu)
    ll[14] = 5.198466400630 + 77713.7714481804 * t;       // Moon (D)
    ll[15] = 1.627905136020 + 84334.6615717837 * t;       // Moon (F)
    ll[16] = 2.355555638750 + 83286.9142477147 * t;       // Moon (l)
}

// Evaluates all terms in a particular VSOP2013 seris (ser) at time (t)
// in Julian millenia of 365250 days from J2000 (JD 2451545.0).
// Seventeen fundamental longitude arguments in radians (ll) must
// have been previously computed for time t using evalLongitudes().

double VSOP2013::evalSeries ( double t, const VSOP2013Series &ser, double ll[17] )
{
    double ta = pow ( t, ser.it );
    double sum = 0.0;
    
    for ( VSOP2013Term term : ser.terms )
    {
        double phi = 0.0;
        for ( int i = 0; i < 17; i++ )
            if ( term.iphi[i] )
                phi += term.iphi[i] * ll[i];
        sum += term.s * sin ( phi ) + term.c * cos ( phi );
    }
    
    return ta * sum;
}

// Returns J2000 ecliptic orbital elements for a planet (iplanet)
// 1 = Mercury .... 9 = Pluto at a specific Julian Ephemeris Date.
// This method only works if the planet's VSOP2013 series have been
// read from a VSOP2013 data file.

SSOrbit VSOP2013::getOrbit ( int iplanet, double jed )
{
    double a = 0.0, l = 0.0, k = 0.0, h = 0.0, q = 0.0, p = 0.0;
    double t = ( jed - 2451545.0 ) / 365250.0;
    double ll[17] = { 0 };
    
    evalLongitudes ( t, ll );
    vector<VSOP2013Series> &series = planets[iplanet - 1];
    for ( VSOP2013Series ser : series )
    {
        double sum = evalSeries ( t, ser, ll );
        if ( ser.iv == 1 )
            a += sum;
        else if ( ser.iv == 2 )
            l += sum;
        else if ( ser.iv == 3 )
            k += sum;
        else if ( ser.iv == 4 )
            h += sum;
        else if ( ser.iv == 5 )
            q += sum;
        else if ( ser.iv == 6 )
            p += sum;
    }
    
    double e = sqrt ( k * k + h * h );  // eccentricity
    double w = atan2 ( h, k );          // longitude of perihelion
    double n = atan2 ( p, q );          // longitude of ascending node
    double i = 2.0 * asin ( sqrt ( q * q + p * p ) ); // inclination
    double mm = getMeanMotion ( iplanet, a );
    return SSOrbit ( jed, a * ( 1.0 - e ), e, i, mod2pi ( w - n ), mod2pi ( n ), mod2pi ( l - w ), mm );
}

// Returns mean motion in radians per day for a planet (iplanet)
// 1 = Mercury .... 9 = Pluto with orbital semimajor axis (a) in AU.

double VSOP2013::getMeanMotion ( int iplanet, double a )
{
    double gmsol = 2.9591220836841438269e-04;
    double gmp[9] =
    {
        4.9125474514508118699e-11,
        7.2434524861627027000e-10,
        8.9970116036316091182e-10,
        9.5495351057792580598e-11,
        2.8253458420837780000e-07,
        8.4597151856806587398e-08,
        1.2920249167819693900e-08,
        1.5243589007842762800e-08,
        2.1886997654259696800e-12
    };
    double rgm = sqrt ( ( iplanet >= 1 && iplanet <= 9 ? gmp[iplanet-1] : 0.0 ) + gmsol );
    return rgm / pow ( a, 1.5 );
}

// Rotates a vector from the J2000 ecliptic frame to the ICRS (J2000 equatorial) frame.
// Returns the equatorial vector; the input ecliptic vector (ecl) is not modified.

SSVector VSOP2013::toEquatorial ( SSVector ecl )
{
    static double eps = degtorad ( 23.0 + 26.0 / 60.0 + 21.41136 / 3600.0 );
    static double phi = degtorad ( -0.05188 / 3600.0 );
    static double ceps = cos ( eps );
    static double seps = sin ( eps );
    static double cphi = cos ( phi );
    static double sphi = sin ( phi );
    static SSMatrix rot ( cphi, -sphi * ceps,  sphi * seps,
                          sphi,  cphi * ceps, -cphi * seps,
                           0.0,         seps,         ceps );
    return rot * ecl;
}

// Computes a planet's heliocentric position and velocity in the ICRS (J2000 equatorial) frame
// on a specific Julian Ephemeris Date (jed). The planet (iplanet) is 1 = Mercury .... 9 = Pluto.
// The position and velocity vectors (pos and vel) are returned in units of AU and AU/day.
// Computes position/velocity of Earth-Moon Barycenter (not Earth) for iplanet = 3.
// Returns true if successful or false if planet identifier not recognized.

bool VSOP2013::computePositionVelocity ( int iplanet, double jed, SSVector &pos, SSVector &vel )
{
    SSOrbit orbit;
    
    if ( iplanet == 0 )
    {
        pos = vel = SSVector ( 0.0, 0.0, 0.0 );
        return true;
    }
    
    if ( iplanet == 1 )
        orbit = mercuryOrbit ( jed );
    else if ( iplanet == 2 )
        orbit = venusOrbit ( jed );
    else if ( iplanet == 3 )
        orbit = earthOrbit ( jed );
    else if ( iplanet == 4 )
        orbit = marsOrbit ( jed );
    else if ( iplanet == 5 )
        orbit = jupiterOrbit ( jed );
    else if ( iplanet == 6 )
        orbit = saturnOrbit ( jed );
    else if ( iplanet == 7 )
        orbit = uranusOrbit ( jed );
    else if ( iplanet == 8 )
        orbit = neptuneOrbit ( jed );
    else if ( iplanet == 9 )
        orbit = plutoOrbit ( jed );
    else
        return false;
    
    orbit.toPositionVelocity ( jed, pos, vel );
    pos = toEquatorial ( pos );
    vel = toEquatorial ( vel );
    
    return true;
}
