// SSTLE.cpp
// SSCore
//
// Created by Tim DeBenedictis on 4/4/20.
// Copyright © 2020 Southern Stars. All rights reserved.

#include "SSUtilities.hpp"
#include "SSTime.hpp"
#include "SSTLE.hpp"

// Reads a TLE record from three lines of an input stream (file).
// Returns 0 if successful or a negative number of failure.

int SSTLE::read ( istream &file )
{
    string buf;
    int    year = 0, number = 0, iexp = 0, ibexp = 0;
    double xm0 = 0.0, xnode0 = 0.0, omega0 = 0.0;
    double e0 = 0.0, xn0 = 1.0, xndt20 = 0.0, xndd60 = 0.0;
    double day = 0.0, epoch = 0.0;
    double temp = M_2PI / 1440.0 / 1440.0;

    // Read first line; trim trailing whitespace; copy satellite name
    
    if ( ! getline ( file, buf ) )
        return ( -1 );

    name = trim ( buf );

    // Read second line; must start with a '1'
    
    if ( ! getline ( file, buf ) )
        return ( -2 );
    
    if ( buf[0] != '1' )
        return ( -2 );
    
    number = strtoint ( buf.substr ( 2, 5 ) );
    desig = trim ( buf.substr ( 9, 6 ) );
    epoch = strtofloat64 ( buf.substr ( 18, 14 ) );
    xndt20 = strtofloat64 ( buf.substr ( 33, 10 ) );
    xndd60 = strtofloat64 ( buf.substr ( 44, 6 ) );
    iexp = strtoint ( buf.substr ( 50, 2 ) );
    bstar = strtofloat64 ( buf.substr ( 53, 6 ) );
    ibexp = strtoint ( buf.substr ( 59, 2 ) );
    
    // Convert epoch to year and day of year

    year = epoch / 1000.0;
    day = epoch - year * 1000.0;
    if ( year > 56 )
        year += 1900;
    else
        year += 2000;

    // Convert other parameters

    norad = number;
    jdepoch = SSTime ( SSDate ( kGregorian, 0.0, year, 1, day, 0, 0, 0.0 ) );
    xndt2o = xndt20 * temp;
    xndd6o = xndd60 * 1.0e-5 * pow ( 10.0, iexp );
    bstar = bstar * 1.0e-5 * pow ( 10.0, ibexp );
             
    // Third line must start with a '2'
    
    if ( ! getline ( file, buf ) )
        return ( -3 );

    if ( buf[0] != '2' )
        return ( -3 );
    
    number = strtoint ( buf.substr ( 2, 5 ) );
    xincl = strtofloat64 ( buf.substr ( 8, 8 ) );
    xnode0 = strtofloat64 ( buf.substr ( 17, 8 ) );
    e0 = strtofloat64 ( buf.substr ( 26, 7 ) );
    omega0 = strtofloat64 ( buf.substr ( 34, 8 ) );
    xm0 = strtofloat64 ( buf.substr ( 43, 8 ) );
    xn0 = strtofloat64 ( buf.substr ( 52, 11 ) );
    
    // Convert other parameters
    
    xincl = degtorad ( xincl );
    xnodeo = degtorad ( xnode0 );
    eo = e0 * 1.0e-7;
    omegao = degtorad ( omega0 );
    xmo = degtorad ( xm0 );
    xno = xn0 * M_2PI / 1440.0;
             
    return ( 0 );
}

// Writes a TLE record to three lines of an output stream (file).
// Returns 0 if successful or a negative number of failure.

int SSTLE::write ( ostream &file )
{
    double temp = M_2PI / 1440.0 / 1440.0;
    double xincl0 = radtodeg ( xincl );  // inclination
    double xnode0 = radtodeg ( xnodeo ); // right ascension of ascending node
    double e0 = eo * 1.0e7;              // eccentricity
    double omega0 = radtodeg ( omegao ); // argument of perigee in degrees
    double xm0 = radtodeg ( xmo );       // mean anomaly in degrees
    double xn0 = 1440.0 * xno / M_2PI;   // mean motion in revolutions/day
    double xndt20 = xndt2o / temp;       // half of derivative of mean motion
    double xndd60 = fabs ( xndd6o );     // one sixth of second derivative of mean motion
    double bstar0 = fabs ( bstar );      // BSTAR drag coefficient
    int iexp = 0, ibexp = 0;             // exponents
    
    if ( xndd60 > 0.0 )
    {
        iexp = floor ( log10 ( xndd60 ) ) + 1;
        xndd60 = xndd60 / pow ( 10.0, iexp - 5 );
        xndd60 = xndd6o > 0.0 ? xndd60 : -xndd60;
    }
    
    if ( bstar0 > 0.0 )
    {
        ibexp = floor ( log10 ( bstar0 ) ) + 1;
        bstar0 = bstar0 / pow ( 10.0, ibexp - 5 );
        bstar0 = bstar > 0.0 ? bstar0 : -bstar0;
    }
    
    // epoch in TLE format
    
    SSDate date = SSDate ( SSTime ( jdepoch ) );
    double day = jdepoch - SSTime ( SSDate ( kGregorian, 0.0, date.year, 1, 0.0, 0, 0, 0.0 ) );
    double epoch = ( date.year % 100 ) * 1000.0 + day;
    
    // format first line of orbital elements
    
    string line1 = format ( "1 %05dU %-6s   %13.8f %c.%08.0f %+06.0f-%1d %+06.0f%+1d 0    00",
             norad, desig.c_str(), epoch,
             xndt20 > 0 ? '+' : '-', fabs ( xndt20 * 1.0e8 ),
             xndd60, -iexp, bstar0, ibexp );
    line1[69] = checksum ( line1 );
    
    // format second line of orbital element
    
    string line2 = format ( "2 %05d %08.4lf %08.4lf %07.0f %08.4lf %08.4lf %11.8lf    00",
            norad, xincl0, xnode0, e0, omega0, xm0, xn0 );
    line2[69] = checksum ( line2 );

    // write to output stream
    
    file << name << endl;
    file << line1 << endl;
    file << line2 << endl;

    return 0;
}

char SSTLE::checksum ( string &line )
{
    int i, sum = 0;
    
    for ( i = 0; i < 68; i++ )
    {
        char c = line[i];
        
        if ( c > '0' && c <= '9')
            sum += c - '0';
        
        if ( c == '-' )
            sum += 1;
    }
    
    return ( sum % 10 + '0' );
}
