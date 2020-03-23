// SSUtilities.cpp
// SSCore
//
// Created by Tim DeBenedictis on 3/23/20.
// Copyright © 2020 Southern Stars. All rights reserved.
//
// A few useful string manipulation and other utility functions
// that are not present in standard C++11.

#ifndef SSUtilities_hpp
#define SSUtilities_hpp

#include <string>
#include <vector>

using namespace std;

string trim ( string str );
string format ( const char *fmt, ... );
vector<string> split ( string str, string delim );

int strtoint ( string str );
int64_t strtoint64 ( string str );
float strtofloat ( string str );
double strtofloat64 ( string str );

#endif /* SSUtilities_hpp */
