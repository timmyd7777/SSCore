// SSPlanet.cpp
// SSCore
//
// Created by Tim DeBenedictis on 3/15/20.
// Copyright © 2020 Southern Stars. All rights reserved.

#include "SSImportTLE.hpp"

// Imports satellites from TLE-formatted text file (filename).
// Imported satellites are appended to the input vector of SSObjects (satellites).
// Returns number of satellites successfully imported.

int SSImportSatellitesFromTLE ( const string &filename, SSObjectVec &satellites )
{
    // Open file; return on failure.

    FILE *file = fopen ( filename.c_str(), "rb" );
    if ( ! file )
        return 0;

    // Read file line-by-line until we reach end-of-file

    string line = "";
    int numSats = 0;
    SSTLE tle;
    
    while ( tle.read ( file ) == 0 )
    {
        // Attempt to create solar system object from TLE; if successful add to object vector.
        
        SSSatellite *pSat = new SSSatellite ( tle );
        if ( pSat )
        {
            satellites.append ( SSObjectPtr ( pSat ) );
            numSats++;
        }
    }
    
    // Close file. Return number of objects added to object vector.

    fclose ( file );
    return numSats;
}


// Imports a Mike McCants satellite names file, here:
// https://www.prismnet.com/~mmccants/tles/mcnames.zip
// into a map of McName structs indexed by NORAD number.
// Returns number of McNames imported from file.

int SSImportMcNames ( const string &filename, McNameMap &mcnames )
{
    // Open file; return on failure.

    FILE *file = fopen ( filename.c_str(), "rb" );
    if ( ! file )
        return 0;

    // Read file line-by-line until we reach end-of-file

    string line = "";
    int nMcNames = 0;
    
    while ( fgetline ( file, line ) )
    {
        McName mcname = { 0, "", 0.0, 0.0, 0.0, 0.0 };
        
        // Attempt to read McName from line.
        
        size_t len = line.length();
        if ( len < 5 )
            continue;
        
        mcname.norad = strtoint ( line.substr ( 0, 5 ) );
        if ( mcname.norad == 0 )
            continue;

        mcname.name = len > 22 ? trim ( line.substr ( 6, 17 ) ) : "";
        mcname.len = len > 26 ? strtofloat ( line.substr ( 22, 4 ) ) : 0.0;
        mcname.wid = len > 31 ? strtofloat ( line.substr ( 27, 4 ) ) : 0.0;
        mcname.dep = len > 36 ? strtofloat ( line.substr ( 32, 4 ) ) : 0.0;
        mcname.mag = len > 41 ? strtofloat ( line.substr ( 37, 4 ) ) : 0.0;
        
        if ( mcname.mag == 0.0 )
            mcname.mag = INFINITY;
        
        mcnames.insert ( { mcname.norad, mcname } );
        nMcNames++;
    }
    
    // Close file. Return number of objects added to object vector.

    fclose ( file );
    return nMcNames;
}

// Imports satellite standard magnitudes and sizes from Mike McCants satellite
// names file, found here: https://www.prismnet.com/~mmccants/tles/mcnames.zip
// Magnitudes and sizes are inserted into the vector of SSObjects (objects),
// which may contain any solar system objects in addition to satellites.
// Returns number of McCants magnitudes & sizes successfully imported.

int SSImportMcNames ( const string &filename, SSObjectVec &objects )
{
    McNameMap mcnamemap;
    
    // First read the McNames file; return 0 if we fail.
    
    int n = SSImportMcNames ( filename, mcnamemap );
    if ( n == 0 || mcnamemap.size() == 0 )
        return 0;
    
    // For each object in the solar system object vector...
    
    n = 0;
    for ( int i = 0; i < objects.size(); i++ )
    {
        SSObjectPtr pObj = objects[i];
        
        // If the object is not a satellite, continue
        
        SSSatellite *pSat = SSGetSatellitePtr ( pObj );
        if ( pSat == nullptr )
            continue;
        
        // Get satellite's NORAD number. Look for McName record with same number.
        // If we find one, copy McName magnitude and size into satellite.
        
        int norad = pSat->getTLE().norad;
        McName mcname = mcnamemap[norad];
        if ( mcname.norad == norad )
        {
            pSat->setHMagnitude ( mcname.mag );
            pSat->setRadius ( mcname.len / 2000.0 );
            n++;
        }
    }
    
    // Return total number of McName records matched.
    
    return n;
}

// Imports a Mineo Wakita amateur radio satellite frequency data file:
// http://www.ne.jp/asahi/hamradio/je9pel/satslist.csv
// into a map of vectors of SatFreqData structs indexed by NORAD number.
// Returns number of SatFreqData records imported from file.

int SSImportSatelliteFrequencyData ( const string &filename, SatFreqMap &freqmap )
{
    // Open file; return on failure.

    ifstream file ( filename );
    if ( ! file )
        return 0;

    // Read file line-by-line until we reach end-of-file

    string line = "";
    int nFreqs = 0;
    vector<SSSatellite::FreqData> freqvec;

    while ( getline ( file, line ) )
    {
        vector<string> fields = split ( line, ";" );
        if ( fields.size() < 8 )
            continue;
        
        SSSatellite::FreqData freq = { 0, "", "", "", "", "", "", "" };
        
        freq.name = trim ( fields[0] );
        freq.norad = strtoint ( fields[1] );
        freq.uplink = trim ( fields[2] );
        freq.downlink = trim ( fields[3] );
        freq.beacon = trim ( fields[4] );
        freq.mode = trim ( fields[5] );
        freq.callsign = trim ( fields[6] );
        freq.status = trim ( fields[7] );
        
        if ( freq.norad < 1 )
            continue;
        
        nFreqs++;

        if ( freqvec.size() > 0 )
        {
            if ( freq.norad != freqvec[0].norad )
            {
                freqmap.insert ( { freqvec[0].norad, freqvec } );
                freqvec.clear();
            }
        }

        freqvec.push_back ( freq );
    }
    
    // Return number of frequencies read from file.  File will close automatically.

    return nFreqs;
}

// Imports satellite amateur radio frequency data into a vector of SSObjects (objects),
// which may contain any solar system objects in addition to satellites.
// Returns number of satellite radio frequencies successfully imported.

int SSImportSatelliteFrequencyData ( const string &filename, SSObjectVec &objects )
{
    SatFreqMap freqmap;
    
    // First read the satellite amateur radio frequency file; return 0 if we fail.
    
    int n = SSImportSatelliteFrequencyData ( filename, freqmap );
    if ( n == 0 || freqmap.size() == 0 )
        return 0;
    
    // For each object in the solar system object vector...
    
    n = 0;
    for ( int i = 0; i < objects.size(); i++ )
    {
        SSObjectPtr pObj = objects[i];

        // If the object is not a satellite, continue
        
        SSSatellite *pSat = SSGetSatellitePtr ( pObj );
        if ( pSat == nullptr )
            continue;
        
        // Get satellite's NORAD number. Look for satellite frequency vector with same number.
        // If we find one, copy satellite frequency vector into satellite.
        
        int norad = pSat->getTLE().norad;
        vector<SSSatellite::FreqData> freqvec = freqmap[norad];
        if ( freqvec.size() > 0 && freqvec[0].norad == norad )
        {
            pSat->setRadioFrequencies ( freqvec );
            n += freqvec.size();
        }
    }
    
    // Return total number of satellites matched to amateur radio frequencies.
    
    return n;
}

// Imports N2YO satellite auxiliar data file:
// http://www.ne.jp/asahi/hamradio/je9pel/satslist.csv
// into a map of vectors of N2Data structs indexed by NORAD number.
// Returns number of N2Data records imported from file.

int SSImportN2Data ( const string &filename, N2DataMap &datamap )
{
    // Open file; return on failure.

    ifstream file ( filename );
    if ( ! file )
        return 0;

    // Read file line-by-line until we reach end-of-file

    string line = "";
    int n = 0;

    while ( getline ( file, line ) )
    {
        vector<string> fields = split_csv ( line );
        if ( fields.size() < 8 )
            continue;
        
        N2Data data;
        
        data.norad = strtoint ( fields[0] );
        data.name = fields[1];
        data.type = fields[2];
        data.source = fields[3];
        data.description = fields[4];
        data.launch_site = fields[5];
        data.launch_date = SSTime ( SSDate ( "%Y-%m-%d", fields[6] ) );
        data.decay_date = SSTime ( SSDate ( "%Y-%m-%d", fields[7] ) );
        if ( data.decay_date < SSTime::kB1950 )
            data.decay_date = INFINITY;
        
        if ( data.norad < 1 )
            continue;
        
        datamap.insert ( { data.norad, data } );
        n++;
    }
    
    // Return number of N2Data structs read from file.  File will close automatically.

    return n;
}

// Imports N2YO satellite auxiliar data file into the vector of SSObjects (satellites),
// which may contain any solar system objects in addition to satellites.
// Returns number of N2Data records successfully imported.

int SSImportN2Data ( const string &filename, SSObjectVec &satellites )
{
    N2DataMap datamap;
    
    // First read the McNames file; return 0 if we fail.
    
    int n = SSImportN2Data ( filename, datamap );
    if ( n == 0 || datamap.size() == 0 )
        return 0;
    
    // For each object in the solar system object vector...
    
    n = 0;
    for ( int i = 0; i < satellites.size(); i++ )
    {
        // If the object is not a satellite, continue
        
        SSSatellite *pSat = SSGetSatellitePtr ( satellites[i] );
        if ( pSat == nullptr )
            continue;
        
        // Get satellite's NORAD number. Look for McName record with same number.
        // If we find one, copy McName magnitude and size into satellite.
        
        int norad = pSat->getTLE().norad;
        N2Data data = datamap[norad];
        if ( data.norad == norad )
        {
            pSat->setTaxonomy ( data.type );
            pSat->setDescription ( data.description );
            pSat->setSourceCountry ( data.source );
            pSat->setLaunchSite ( data.launch_site );
            pSat->setLaunchDate ( data.launch_date );
            n++;
        }
    }
    
    // Return total number of McName records matched.
    
    return n;
}
