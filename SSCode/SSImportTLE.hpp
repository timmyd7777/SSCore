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

int SSImportN2Data ( const string &path, N2DataMap &n2data );
int SSImportN2Data ( const string &path, SSObjectVec &satellites );

#endif /* SSPlanet_hpp */
