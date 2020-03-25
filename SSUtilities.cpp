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
