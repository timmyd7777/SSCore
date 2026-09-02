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
    char   source;        // 'v' = visual observation, 'd' = dimension-derived
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

// One record from Jonathan McDowell's GCAT satellite catalog

struct GCATData
{
    string jcat = "";             // GCAT/JCAT object identifier, e.g. "S00001"
    int    norad = 0;             // NORAD satellite catalog number (Satcat)
    
    string launchTag = "";        // GCAT launch identifier/tag
    string piece = "";            // International-designator/piece designation
    string type = "";             // GCAT object type/classification
    string name = "";             // Primary object name
    string plName = "";           // Payload/alternate program name
    
    string launchDate = "";       // Launch date (LDate), as supplied by GCAT
    string parent = "";           // Parent object JCAT identifier
    string separationDate = "";   // Separation/deployment date and time (SDate)
    string primary = "";          // Primary body orbited, e.g. Earth
    string decayDate = "";        // Decay/disposal date and time (DDate)
    
    string status = "";           // GCAT object status
    string destination = "";      // Destination field (Dest)
    
    string owner = "";            // Owning/operating organization
    string state = "";            // State/country code
    string manufacturer = "";     // Manufacturer
    string bus = "";              // Spacecraft bus/platform
    string motor = "";            // Propulsion/motor designation
    
    float  mass = 0.0;            // Object mass, zero if unknown (kg)
    string massFlag = "";         // Mass uncertainty/estimate flag
    float  dryMass = 0.0;         // Dry mass, zero if unknown (kg)
    string dryMassFlag = "";      // Dry mass uncertainty/estimate flag
    float  totalMass = 0.0;       // Total associated/launch mass, zero if unknown (kg)
    string totalMassFlag = "";    // Total mass uncertainty/estimate flag

    float  length = 0.0;          // Longest main-body dimension, zero if unknown (m)
    string lengthFlag = "";       // Length uncertainty/estimate flag
    float  diameter = 0.0;        // Second-longest/main-body diameter, zero if unknown (m)
    string diameterFlag = "";     // Diameter uncertainty/estimate flag
    float  span = 0.0;            // Maximum dimension including appendages, zero if unknown (m)
    string spanFlag = "";         // Span uncertainty/estimate flag
    string shape = "";            // Physical shape description
    
    string orbitDate = "";        // Epoch/date associated with orbit data (ODate)
    
    float  perigee = 0.0;         // Perigee altitude, zero if unknown (km)
    string perigeeFlag = "";      // Perigee uncertainty/estimate flag
    float  apogee = 0.0;          // Apogee altitude, zero if unknown (km)
    string apogeeFlag = "";       // Apogee uncertainty/estimate flag
    float  inclination = 0.0;     // Orbital inclination, zero if unknown (degrees)
    string inclinationFlag = "";  // Inclination uncertainty/estimate flag

    string orbit = "";            // GCAT orbital regime/class (OpOrbit)
    string orbitQuality = "";     // GCAT orbit quality (OQUAL)

    string altNames = "";         // Alternate names/identifiers (AltNames)
};

typedef vector<GCATData> GCATDataVec;

int SSImportN2Data ( const string &path, N2DataMap &n2data );
int SSImportN2Data ( const string &path, SSObjectVec &satellites );

int SSImportSCOREData ( const string &filename, SCOREDataVec &data );
int SCOREDataToStandardMagnitudes ( const SCOREDataVec &data, SCOREMagnitudeMap &magnitudes );
int SSImportSCOREData ( const string &filename, SSObjectVec &objects );

int SSImportGCATData ( const string &filename, GCATDataVec &data );
int SSImportGCATData ( const GCATDataVec &gcat, SSObjectVec &satellites );

float GCATEffectiveArea ( float length, float diameter, float span, const string &shape );
float GCATStandardMagnitude ( float area );

bool SSGetDefaultSatelliteData ( SSSatellitePtr pSat, float &stdmag, float &diameter );
int SSGetDefaultSatelliteData ( SSObjectVec &satellites );

#endif /* SSPlanet_hpp */
