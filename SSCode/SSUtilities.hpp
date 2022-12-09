// SSUtilities.cpp
// SSCore
//
// Created by Tim DeBenedictis on 3/23/20.
// Copyright © 2020 Southern Stars. All rights reserved.
//
// A few useful string manipulation, angle conversion,
// and other utility functions that are not present in standard C++11.

#ifndef SSUtilities_hpp
#define SSUtilities_hpp

#include <algorithm>
#include <cmath>
#include <ctime>
#include <cstring>
#include <string>
#include <vector>

using namespace std;

// clamp(value,low,high) returns value clamped to limits (low, high).
// This is a C++11 replacement for std::clamp() declared in <algorithm> in C++17.

#if __cplusplus < 201703L
template <typename T>  const T &clamp ( const T &value, const T &low, const T &high ) { return value < low ? low : value > high ? high : value; }
#endif

// on Android, hijack fopen and route it through the android asset system
// so that we can pull things out of our package's APK. From:
// http://www.50ply.com/blog/2013/01/19/loading-compressed-android-assets-with-file-pointer/

#ifdef ANDROID
extern "C" FILE *android_fopen ( const char *fname, const char *mode );
#ifndef fopen
#define fopen(name,mode) android_fopen(name,mode)
#endif

// Android redirects stdout & stderr to /dev/null, so use Android logging to send output to logcat.

#ifndef printf
#include <android/log.h>
#define printf(...) __android_log_print(ANDROID_LOG_INFO, "SSCore", __VA_ARGS__)
#endif
#endif

#define M_2PI (2*M_PI)

string getcwd ( void );
bool fgetline ( FILE *infile, string &line );

string trim ( string str );
string format ( const char *fmt, ... );
vector<string> split ( string str, string delim );
vector<string> tokenize ( string str, string delim );
vector<string> split_csv ( const string &csv );

int compare ( const string &str1, const string &str2, size_t n, bool casesens = true );

void toLower ( string &str );
void toUpper ( string &str );

bool isNumeric ( const string &str );
bool endsWith ( const string &str, const string &suffix );
bool startsWith ( const string &str, const string &prefix );
void replaceAll ( string& str, const std::string& from, const std::string& to);

string getFileExt ( string path );
string setFileExt ( string path, string ext );
bool hasFileExt ( string path, vector<string> exts );

int strtoint ( string str );
int64_t strtoint64 ( string str );
float strtofloat ( string str );
double strtofloat64 ( string str );

double strtodeg ( string str );
double degtorad ( double deg );
double radtodeg ( double rad );
double sindeg ( double deg );
double cosdeg ( double deg );
double tandeg ( double deg );
double asindeg ( double y );
double acosdeg ( double x );
double atandeg ( double x );
double atan2pi ( double y, double x );
double atan2pideg ( double y, double x );
double modpi ( double rad );
double mod2pi ( double rad );
double mod180 ( double deg );
double mod360 ( double deg );
double mod24h ( double h );

// These POSIX sleep() functions are declared in <unistd.h>
// on MacOS/Linux; these are replacements for Windows.

#ifdef _MSC_VER
unsigned int sleep ( unsigned int secs );
int usleep ( uint32_t usecs );
#endif

void msleep ( uint32_t ms );

double unixtime ( void );
double unixtime_since ( double &start );

double clocksec ( void );
double clocksec_since ( double &start );

double get_timezone ( void );
void set_timezone ( double hours );

string get_timezonename ( void );
void set_timezonename ( string zonename );

size_t filesize ( const string &path );
time_t filetime ( const string &path );

string urlEncode ( const string &src );
string urlDecode ( const string &src );

#endif /* SSUtilities_hpp */
