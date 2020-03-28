// SSUtilities.cpp
// SSCore
//
// Created by Tim DeBenedictis on 3/23/20.
// Copyright © 2020 Southern Stars. All rights reserved.

#include "SSUtilities.hpp"

// Returns a C++ string which has leading and trailing whitespace
// trimmed from the input string (does not modify input string).

string trim ( string str )
{
    auto start = str.find_first_not_of ( " \t\r\n" );
    auto end = str.find_last_not_of ( " \t\r\n" );

    if ( start == string::npos )
        return string ( "" );
    else
        return str.substr ( start, ( end - start ) + 1 );
}

// Returns C++ string constructed from printf()-style input arguments.

string format ( const char *fmt, ... )
{
    char buf[1024] = { 0 };

    va_list args;
    va_start ( args, fmt );
    vsnprintf ( buf, sizeof buf, fmt, args );
    va_end ( args );

    return string ( buf );
}

// Splits a string into a vector of token strings separated by the specified delimiter.
// Two adjacent delimiters generate an empty token string.
// The original string is not modified.

vector<string> split ( string str, string delim )
{
    vector<string> tokens;
	
    size_t start = 0;
    size_t end = str.find ( delim );
    while ( end != std::string::npos )
    {
        tokens.push_back ( str.substr ( start, end - start ) );
        start = end + delim.length();
        end = str.find ( delim, start );
    }

    tokens.push_back ( str.substr ( start, end ) );
	return tokens;
}

// Converts string to 32-bit signed integer.
// Returns zero if string cannot be converted.

int strtoint ( string str )
{
    int i;
    
    try
    {
        i = stoi ( str );
    }
    catch ( ... )
    {
        i = 0;
    }
    
    return i;
}

// Converts string to 64-bit signed integer.
// Returns zero if string cannot be converted.

int64_t strtoint64 ( string str )
{
    int64_t i;
    
    try
    {
        i = stoll ( str );
    }
    catch ( ... )
    {
        i = 0;
    }
    
    return i;
}

// Converts string to 32-bit single precision floating point value.
// Returns zero if string cannot be converted.

float strtofloat ( string str )
{
    float f;
    
    try
    {
        f = stof ( str );
    }
    catch ( ... )
    {
        f = 0.0;
    }
    
    return f;
}

// Converts string to 64-bit double precision floating point value.
// Returns zero if string cannot be converted.

double strtofloat64 ( string str )
{
    double d;
    
    try
    {
        d = stod ( str );
    }
    catch ( ... )
    {
        d = 0.0;
    }
    
    return d;
}

// Converts a string representing an angle in deg min sec to decimal degrees.
// Works with angle strings in any format (DD MM SS.S, DD MM.M, DD.D, etc.)
// Assumes leading whitespace has been removed from string!

double strtodeg ( string str )
{
	const char *cstr = str.c_str();

	double deg = 0.0, min = 0.0, sec = 0.0;
	sscanf ( cstr, "%lf %lf %lf", &deg, &min, &sec );
	deg = fabs ( deg ) + min / 60.0 + sec / 3600.0;
	
	return cstr[0] == '-' ? -deg : deg;
}

// Converts angle in degrees to radians.

double degtorad ( double deg )
{
	return deg * M_PI / 180.0;
}

// Converts angle in radians to degrees.

double radtodeg ( double rad )
{
	return rad * 180.0 / M_PI;
}

// Returns sine of angle in degrees.

double sindeg ( double deg )
{
	return sin ( degtorad ( deg ) );
}

// Returns cosine of angle in degrees.

double cosdeg ( double deg )
{
	return cos ( degtorad ( deg ) );
}

// Returns tangent of angle in degrees.

double tandeg ( double deg )
{
	return tan ( degtorad ( deg ) );
}

// Returns arcsine in degrees.

double asindeg ( double y )
{
	return radtodeg ( asin ( y ) );
}

// Returns arccosine in degrees.

double acosdeg ( double x )
{
	return radtodeg ( acos ( x ) );
}

// Returns arctangent in degrees.

double atandeg ( double x )
{
	return radtodeg ( atan ( x ) );
}

// Returns arctangent of y / x in radians in the range 0 to 2.0 * M_PI.

double atan2pi ( double y, double x )
{
	if ( y < 0.0 )
		return atan2 ( y, x ) + 2.0 * M_PI;
	else
		return atan2 ( y, x );
}

// Returns arctangent of y / x in degress in the range 0 to 360.

double atan2pideg ( double y, double x )
{
	return radtodeg ( atan2pi ( y, x ) );
}

// Reduces an angle in degrees to the range 0 to 2.0 * M_PI.

double mod2pi ( double rad )
{
    return rad - M_2PI * floor ( rad / M_2PI );
}

// Reduces an angle in degrees to the range -M_PI to +M_PI.

double modpi ( double rad )
{
    rad = mod2pi ( rad );
    
    if ( rad > M_PI )
        rad -= M_2PI;

    return rad;
}

// Reduces an angle in degrees to the range 0 to 360.

double mod360 ( double deg )
{
    return deg - 360 * floor ( deg / 360 );
}

// Reduces an angle in degrees to the range -180 to +180.

double mod180 ( double deg )
{
    deg = mod360 ( deg );
    
    if ( deg > 180 )
        deg -= 360;

    return deg;
}
