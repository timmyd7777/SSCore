// SSImportTLE.hpp
// SSCore
//
// Created by Tim DeBenedictis on 1/18/23.
// Copyright © 2023 Southern Stars. All rights reserved.
//
// Routines for importing artificial satellite data in TLE format,
// and related data (radio frequencies, visual magnitudes, etc.)

#ifndef SSImportTLE_hpp
#define SSImportTLE_hpp

#include "SSPlanet.hpp"

// Holds satellite size and magnitude data from Mike McCants' satellite names file.

struct McName
{
    int    norad;         // NORAD number
    string name;          // Spacecraft name
    float  len, wid, dep; // Dimensions in meters
    float  mag;           // Magnitude at 1000 km range, 50% illumination.
};

typedef map<int,McName> McNameMap;

int SSImportSatellitesFromTLE ( const string &path, SSObjectVec &satellites );
int SSImportMcNames ( const string &path, McNameMap &mcnames );
int SSImportMcNames ( const string &filename, SSObjectVec &objects );

// Struct used to store CSV-parsed data from amateur satellite frequency table

typedef map<int,vector<SSSatellite::FreqData>> SatFreqMap;

int SSImportSatelliteFrequencyData ( const string &path, SatFreqMap &satfreqs );
int SSImportSatelliteFrequencyData ( const string &path, SSObjectVec &objects );

// Struct used to hold data from n2yo.com auxiliary data CSV file

struct N2Data
{
    int    norad;              // NORAD tracking number
    string name;               // object name
    string type;               // 'RB' = rocket body, 'DEB' = debris
    string source;             // source country abbreviation, e.g. "USA", "CIS", etc.
    string description;        // blob of descriptive text
    string launch_site;        // launch site abbreviation, e.g. "AFETR", etc.
    float  launch_date;        // Julian date of launch
    float  decay_date;         // Julian date of reentry
};

typedef map<int,N2Data> N2DataMap;

// One observation from the IAU SCORE (Satellite Constellation Observation Repository)
// CSV database: https://score.cps.iau.org/download

struct SCOREData
{
    string  name = "";            // Satellite name
    int     norad = 0;            // NORAD satellite catalog number
    double  time = 0.0;           // Julian date of observation in UTC
    double  time_err = 0.0;       // time uncertainty in days
    float   mag = INFINITY;       // observed satellite magnitude
    float   mag_err = 0.0;        // uncertainty in observed satellite magnitude
    double  lon = 0.0;            // observer longitude in radians, east positive
    double  lat = 0.0;            // observer latitude in radians, north positive
    double  alt = 0.0;            // observer altitude above WGS84 ellipsoid, kilometers
    string  filter = "";          // photometric filter ("V", "R", etc.)
    double  ra = 0.0;             // observed satellite J2000 right ascension. radians
    double  ra_err = 0.0;         // uncertainty in RA, radians
    double  dec = 0.0;            // observed satellite J2000 declination in radians
    double  dec_err = 0.0;        // uncertainty in Dec, radians
    double  range = 0.0;          // observed satellite distance in kilometers
    double  range_err = 0.0;      // uncertainty in observed satellite distance in kilometers
};

typedef vector<SCOREData> SCOREDataVec;

// Reduced McCants standard magnitude for one satellite.

struct SCOREMagnitude
{
    string name = "";           // Satellite name
    int    norad = 0;           // NORAD satellite catalog number
    float  mag = INFINITY;      // median McCants standard magnitude
    float  max = INFINITY;      // maximum 
    float  mad = 0.0;           // median absolute deviation (magnitudes)
    int    nobs = 0;            // number of observations
};

typedef map<int,SCOREMagnitude> SCOREMagnitudeMap;

int SSImportN2Data ( const string &path, N2DataMap &n2data );
int SSImportN2Data ( const string &path, SSObjectVec &satellites );

int SSImportSCOREData ( const string &filename, SCOREDataVec &data );
int SCOREDataToStandardMagnitudes ( const SCOREDataVec &data, SCOREMagnitudeMap &magnitudes );
int SSImportSCOREData ( const string &filename, SSObjectVec &objects );

#endif /* SSPlanet_hpp */
