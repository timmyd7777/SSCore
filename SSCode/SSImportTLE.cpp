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
        McName mcname = { 0, "", 0.0, 0.0, 0.0, 0.0, 0 };
        
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
        mcname.source = len > 42 ? line[42] : 0;
        
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
            pSat->setMagnitudeSource ( SSSatellite::kMcCants );
            pSat->setDiameterSource ( SSSatellite::kMcCants );
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
    
    if ( ! freqvec.empty() )
        freqmap.insert ( { freqvec[0].norad, freqvec } );
    
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
        
        // Get satellite's NORAD number. Look for N2Data record with same number.
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
            pSat->setDecayDate ( data.decay_date );
            n++;
        }
    }
    
    // Return total number of McName records matched.
    
    return n;
}

// Import all SCORE observations from CSV file downloaded
// from https://score.cps.iau.org/download
// The enum below defines the columns in the SCORE CSV file.

enum
{
    kSatelliteName                  = 0,
    kNORADCatID                     = 1,
    kObservationTimeUTC             = 2,
    kObservationTimeUncertaintySec  = 3,
    kApparentMagnitude              = 4,
    kMagnitudeUncertainty           = 5,
    kObserverLatitudeDeg            = 6,
    kObserverLongitudeDeg           = 7,
    kObserverAltitudeM              = 8,
    kLimitingMagnitude              = 9,
    kInstrument                     = 10,
    kObservingMode                  = 11,
    kObservingFilter                = 12,
    kObserverORCID                  = 13,
    kSatelliteRADeg                 = 14,
    kSatelliteDecDeg                = 15,
    kSigma2RA                       = 16,
    kSigmaRASigmaDec                = 17,
    kSigma2Dec                      = 18,
    kRangeKm                        = 19,
    kRangeUncertaintyKm             = 20,
    kRangeRateKmPerSec              = 21,
    kRangeRateUncertaintyKmPerSec   = 22,
    kComments                       = 23,
    kDataArchiveLink                = 24,
    kMPCCode                        = 25,
    kSatCheckerRADeg                = 26,
    kSatCheckerDecDeg               = 27,
    kSatCheckerRangeKm              = 28,
    kSatCheckerRangeRateKmPerSec    = 29,
    kSatCheckerDDecDegPerSec        = 30,
    kSatCheckerDRACosDecDegPerSec   = 31,
    kSatCheckerPhaseAngleDeg        = 32,
    kSatCheckerAltitudeDeg          = 33,
    kSatCheckerAzimuthDeg           = 34,
    kSatCheckerSatelliteAltitudeKm  = 35,
    kSatCheckerSolarElevationDeg    = 36,
    kSatCheckerSolarAzimuthDeg      = 37,
    kIlluminated                    = 38,
    kInternationalDesignator        = 39,
    kPotentiallyDiscrepant          = 40
};

int SSImportSCOREData ( const string &filename, SCOREDataVec &data )
{
    ifstream file ( filename );
    if ( ! file )
        return 0;

    string line;

    // Skip CSV header.
    if ( ! getline ( file, line ) )
        return 0;

    while ( getline ( file, line ) )
    {
        vector<string> fields = split_csv ( line );

        if ( fields.size() < 41 ) {
            cerr << line << endl;
            continue;
        }
        
        SCOREData score;

        score.name             = fields[kSatelliteName];
        score.norad            = strtoint ( fields[kNORADCatID] );
        score.time             = SSTime ( SSDate ( "%Y-%m-%dT%H:%M:%S", fields[kObservationTimeUTC] ) );
        score.time_err         = strtofloat64 ( fields[kObservationTimeUncertaintySec] ) / SSTime::kSecondsPerDay;
        score.mag              = fields[kApparentMagnitude].empty() ? INFINITY : strtofloat ( fields[kApparentMagnitude] );
        score.mag_err          = fields[kMagnitudeUncertainty].empty() ? 0.0 : strtofloat ( fields[kMagnitudeUncertainty] );
        score.lon              = degtorad ( strtofloat64 ( fields[kObserverLongitudeDeg] ) );
        score.lat              = degtorad ( strtofloat64 ( fields[kObserverLatitudeDeg] ) );
        score.alt              = strtofloat ( fields[kObserverAltitudeM] ) / 1000.0;
        score.instrument       = fields[kInstrument];
        score.filter           = fields[kObservingFilter];
        score.ra               = degtorad ( strtofloat64 ( fields[kSatelliteRADeg] ) );
        score.ra_err           = degtorad ( sqrt ( strtofloat64 ( fields[kSigma2RA] ) ) );
        score.dec              = degtorad ( strtofloat ( fields[kSatelliteDecDeg] ) );
        score.dec_err          = degtorad ( sqrt ( strtofloat ( fields[kSigma2Dec] ) ) );
        score.range            = strtofloat ( fields[kRangeKm] );
        score.range_err        = strtofloat ( fields[kRangeUncertaintyKm] );
        score.ra_satchecker    = degtorad ( strtofloat64 ( fields[kSatCheckerRADeg] ) );
        score.dec_satchecker   = degtorad ( strtofloat64 ( fields[kSatCheckerDecDeg] ) );
        score.range_satchecker = strtofloat64 ( fields[kSatCheckerRangeKm] );
        
        // Reject observations with missing NORAD ID, observer location, or satellite astrometry or range.
        // A coordinate pair of exactly (0,0) is extraordinarily unlikely for observer location or satellite position.

        if ( score.norad > 0 && score.time > SSTime::kJ2000 ) // && ! ( score.lon == 0.0 && score.lat == 0.0 ) && ! ( score.ra == 0.0 && score.dec == 0.0 ) && score.range > 0.0 )
            data.push_back ( score );
        else
            cerr << line << endl;
    }

    return (int) data.size();
}

// Reduce SCORE observations to McCants standard magnitudes.

int SCOREDataToStandardMagnitudes ( const SCOREDataVec &data, SCOREMagnitudeMap &magnitudes )
{
    map<int,vector<float>> mags;
    map<int,string> names;
    
    for ( auto &obs : data )
    {
        // Smartphone photometry shows large systematic scatter and is not
        // sufficiently reliable for deriving satellite standard magnitudes.

        if ( obs.instrument == "iPhone 12 Mini" )
            continue;

        if ( ! isfinite ( obs.mag ) )
            continue;

        if ( obs.lon == 0.0 && obs.lat == 0.0 )
            continue;

        // Prefer observed geometry when available; otherwise use geometry
        // computed by SatChecker from historical orbital elements.

        double ra = obs.ra;
        double dec = obs.dec;
        double range = obs.range;

        if ( ra == 0.0 && dec == 0.0 )
        {
            ra = obs.ra_satchecker;
            dec = obs.dec_satchecker;
        }

        if ( range <= 0.0 )
            range = obs.range_satchecker;

        if ( ra == 0.0 && dec == 0.0 )
            continue;

        if ( range <= 0.0 )
            continue;
        
        // Computes the Sun-satellite-observer phase angle for one SCORE observation.
        // SCORE right ascension and declination are in the fundamental J2000 frame.
        // Observation range is in kilometers. Returned phase angle is in radians.
        
        SSTime time ( obs.time );
        SSSpherical location ( obs.lon, obs.lat, obs.alt );
        SSCoordinates coords ( time, location );
        SSVector direction = SSSpherical ( ra, dec );
        SSVector position = coords.getObserverPosition() + direction * ( range / SSCoordinates::kKmPerAU );
        double phase = SSPlanet::phaseAngle ( position, direction );
        float stdmag = SSSatellite::computeStandardMagnitude ( range, phase, obs.mag );
        if ( ! isfinite ( stdmag ) )
            continue;
        
        mags[obs.norad].push_back ( stdmag );
        if ( names[obs.norad].empty() && ! obs.name.empty() )
            names[obs.norad] = obs.name;
    }

    for ( auto &it : mags )
    {
        vector<float> &v = it.second;
        sort ( v.begin(), v.end() );
        
        SCOREMagnitude m;
        m.name = names[it.first];
        m.norad = it.first;
        m.nobs = (int) v.size();
        m.mag = median ( v.data(), v.size() );
        m.mad = mad ( v.data(), v.size(), m.mag );
        magnitudes[it.first] = m;
    }

    return (int) magnitudes.size();
}

// Import SCORE standard magnitudes into satellite objects.

int SSImportSCOREData ( const string &filename, SSObjectVec &objects )
{
    SCOREDataVec observations;
    SCOREMagnitudeMap magnitudes;

    if ( SSImportSCOREData ( filename, observations ) == 0 )
        return 0;

    if ( SCOREDataToStandardMagnitudes ( observations, magnitudes ) == 0 )
        return 0;

    int n = 0;

    for ( int i = 0; i < objects.size(); i++ )
    {
        SSSatellite *pSat = SSGetSatellitePtr ( objects[i] );
        if ( pSat == nullptr )
            continue;

        int norad = pSat->getTLE().norad;
        auto it = magnitudes.find ( norad );
        if ( it == magnitudes.end() )
            continue;

        pSat->setHMagnitude ( it->second.mag );
        pSat->setMagnitudeSource ( SSSatellite::kSCORE );
        n++;
    }

    return n;
}

// Imports Jonathan McDowell's GCAT satellite catalog file(s) in TSV format, from:
// https://www.planet4589.org/space/gcat/tsv/cat/satcat.tsv
// https://www.planet4589.org/space/gcat/tsv/cat/satcat100k.tsv
// GCATData are appended to the input vector (data).
// Returns the number of GCATData structs appended.

int SSImportGCATData ( const string &filename, vector<GCATData> &data )
{
    ifstream file ( filename );

    if ( ! file )
        return 0;

    string line;
    int count = 0;

    while ( getline ( file, line ) )
    {
        // Skip header, comments, and blank lines.

        if ( line.empty() || line[0] == '#' )
            continue;

        vector<string> fields = split ( line, "\t" );
        if ( fields.size() < 42 )
            continue;

        GCATData gcat;

        gcat.jcat             = trim ( fields[0] );
        gcat.norad            = strtoint ( trim ( fields[1] ) );
        gcat.launchTag        = trim ( fields[2] );
        gcat.piece            = trim ( fields[3] );
        gcat.type             = trim ( fields[4] );
        gcat.name             = trim ( fields[5] );
        gcat.plName           = trim ( fields[6] );

        gcat.launchDate       = trim ( fields[7] );
        gcat.parent           = trim ( fields[8] );
        gcat.separationDate   = trim ( fields[9] );
        gcat.primary          = trim ( fields[10] );
        gcat.decayDate        = trim ( fields[11] );

        gcat.status           = trim ( fields[12] );
        gcat.destination      = trim ( fields[13] );
        gcat.owner            = trim ( fields[14] );
        gcat.state            = trim ( fields[15] );
        gcat.manufacturer     = trim ( fields[16] );
        gcat.bus              = trim ( fields[17] );
        gcat.motor            = trim ( fields[18] );

        gcat.mass             = strtofloat ( fields[19] );
        gcat.massFlag         = trim ( fields[20] );
        gcat.dryMass          = strtofloat ( fields[21] );
        gcat.dryMassFlag      = trim ( fields[22] );
        gcat.totalMass        = strtofloat ( fields[23] );
        gcat.totalMassFlag    = trim ( fields[24] );

        gcat.length           = strtofloat ( fields[25] );
        gcat.lengthFlag       = trim ( fields[26] );
        gcat.diameter         = strtofloat ( fields[27] );
        gcat.diameterFlag     = trim ( fields[28] );
        gcat.span             = strtofloat ( fields[29] );
        gcat.spanFlag         = trim ( fields[30] );

        gcat.shape            = trim ( fields[31] );
        gcat.orbitDate        = trim ( fields[32] );

        gcat.perigee          = strtofloat ( fields[33] );
        gcat.perigeeFlag      = trim ( fields[34] );
        gcat.apogee           = strtofloat ( fields[35] );
        gcat.apogeeFlag       = trim ( fields[36] );
        gcat.inclination      = strtofloat ( fields[37] );
        gcat.inclinationFlag  = trim ( fields[38] );

        gcat.orbit            = trim ( fields[39] );
        gcat.orbitQuality     = trim ( fields[40] );
        gcat.altNames         = trim ( fields[41] );

        data.push_back ( gcat );
        count++;
    }

    return count;
}

// Estimates effective optical cross-sectional area from GCAT dimensions.
// GCAT length and diameter describe the main spacecraft body, while span
// includes appendages. For spacecraft with broad appendages such as solar
// panels, sails, or plates, diameter * span is used as an alternate area
// estimate when larger than the main-body length * diameter.
//
// Antennas and tethers are excluded because their span can greatly exceed
// their reflective area. This is an empirical fallback estimator calibrated
// against McCants visual standard magnitudes and independently checked against
// SCORE photometry; it is intended for satellites lacking measured magnitudes.

float GCATEffectiveArea ( float length, float diameter, float span, const string &shape )
{
    float area = length * diameter;
    if ( span > 0.0f )
    {
        string shapel = toLower ( shape );

        bool broad =
            shapel.find ( "pan" ) != string::npos ||
            shapel.find ( "sail" ) != string::npos ||
            shapel.find ( "plate" ) != string::npos;

        bool skinny =
            shapel.find ( "ant" ) != string::npos ||
            shapel.find ( "tether" ) != string::npos;

        if ( broad && ! skinny )
            area = max ( area, diameter * span );
    }

    return area;
}

// Estimate McCants-style standard magnitude from GCAT effective area in square meters.
// The constant was calibrated against McCants visual magnitudes using GCAT dimensions.

float GCATStandardMagnitude ( float area )
{
    if ( area <= 0.0f )
        return INFINITY;

    return 7.98689f - 2.5f * log10f ( area );
}

// Applies GCAT physical data to existing satellites matched by NORAD number.
// GCAT data is used only to fill information not already available from
// higher-priority sources, including mass, physical size, and McCants-style
// standard magnitude estimated from GCAT effective area.
// The caller must first combine all desired GCAT catalogs into the input vector.
// Returns the number of satellites updated with GCAT data.

int SSImportGCATData ( const GCATDataVec &gcat, SSObjectVec &satellites )
{
    // Build NORAD -> GCAT record map.

    map<int,const GCATData *> gcatmap;
    for ( const auto &data : gcat )
        if ( data.norad > 0 )
            gcatmap[data.norad] = &data;

    // Apply GCAT data to each matching satellite.

    int n = 0;
    for ( int i = 0; i < satellites.size(); i++ )
    {
        SSSatellite *pSat = SSGetSatellitePtr ( satellites[i] );
        if ( pSat == nullptr )
            continue;

        int norad = pSat->getTLE().norad;
        auto it = gcatmap.find ( norad );
        if ( it == gcatmap.end() )
            continue;

        const GCATData &data = *it->second;
        bool updated = false;

        // Fill missing physical mass. GCAT mass is in kilograms;
        // SSPlanet stores mass in Earth masses.

        if ( pSat->getMass() <= 0.0 && data.mass > 0.0f )
        {
            pSat->setMass ( data.mass / SSCoordinates::kKgPerEarthMass );
            updated = true;
        }

        // Fill missing physical size. GCAT length is the longest dimension
        // of the main satellite body and is stored here as representative
        // physical diameter, in the same manner as McCants length.

        if ( pSat->getRadius() <= 0.0 && data.length > 0.0f )
        {
            pSat->setRadius ( data.length / 2000.0 );
            pSat->setDiameterSource ( SSSatellite::kGCAT );
            updated = true;
        }

        // Fill missing standard magnitude from GCAT effective optical area.

        if ( ! isfinite ( pSat->getHMagnitude() ) )
        {
            float area = GCATEffectiveArea ( data.length, data.diameter, data.span, data.shape );
            float stdmag = GCATStandardMagnitude ( area );
            if ( isfinite ( stdmag ) )
            {
                pSat->setHMagnitude ( stdmag );
                pSat->setMagnitudeSource ( SSSatellite::kGCAT );
                updated = true;
            }
        }

        if ( updated )
            n++;
    }

    return n;
}

// Returns reasonable fallback McCants standard magnitude and physical
// diameter in meters for a well-known satellite constellation.
// Returns false if no useful default is available.
//
// These values should only fill missing data; they should never replace
// McCants, SCORE-derived, or otherwise measured values.
//
// Starlink visual magnitude estimates are based in part on published photometry:
// Mallama & Respler (2022), "Visual Brightness Characteristics of Starlink
// Generation 1 Satellites", https://arxiv.org/abs/2210.17268
// Mallama et al. (2023), "Starlink Generation 2 Mini Satellites:
// Photometric Characterization", https://arxiv.org/abs/2306.06657
//
// SCORE-derived values are based on modern measured photometry reduced
// to the McCants standard condition (1000 km, 50% illumination).
//
// Other legacy-constellation magnitudes are approximate engineering
// estimates intended only as fallbacks where no measured McCants or SCORE
// magnitude is available.

bool SSGetDefaultSatelliteData ( SSSatellitePtr pSat, float &stdmag, float &diameter )
{
    stdmag = INFINITY;
    diameter = INFINITY;
    auto categories = pSat->getCategories();
    
    for ( auto category : categories )
    {
        switch ( category )
        {
            case SSSatellite::kStarlink:
            {
                SSTLE tle = pSat->getTLE();
                int norad = tle.norad;
                string name = tle.name;

                // Extract spacecraft number from names like STARLINK-30042, STARLINK-11107 [DTC]
                
                int number = 0;
                if ( name.compare ( 0, 9, "STARLINK-" ) == 0 )
                    number = strtoint ( name.substr ( 9 ) );

                if ( norad == 44932 )
                {
                    // DarkSat.
                    stdmag = +6.0;
                    diameter = 4.0;
                }
                else if ( number >= 11000 && number < 12000 )
                {
                    // V2 Mini Direct-to-Cell: SCORE median standard magnitude is about +6.3.
                    
                    stdmag = +6.3;
                    diameter = 5.5;
                }
                else if ( number >= 30000 && number < 40000 )
                {
                    // V2 Mini: SCORE median standard magnitude is about +7.6.
                    // Gen2 Mini launches began in February 2023:  physically larger
                    // but incorporate improved brightness-mitigation measures.
                    
                    stdmag = +7.6;
                    diameter = 5.5;
                }
                else
                {
                    // Gen 1 / V1.5: retain the existing launch-date distinction.
                    // Later v1.0 and v1.5 spacecraft used VisorSat or related
                    // brightness mitigation. Use a compromise value because
                    // launch batches overlap the exact design transitions.
                    
                    double launchDate = pSat->getLaunchDate();
                    if ( launchDate >= SSTime ( SSDate ( kGregorian, 0.0, 2020, 6, 1.0 ) ) )
                    {
                        stdmag = +5.8;
                        diameter = 4.0;
                    }
                    else
                    {
                        stdmag = +5.5;
                        diameter = 4.0;
                    }
                }

                return true;
            }
                
            // Approximation. Published OneWeb photometry gives a mean
            // magnitude of +7.18 when normalized to 1000 km, but this
            // is not directly a McCants 90-degree-phase magnitude.

            case SSSatellite::kOneWeb:
                stdmag = +6.0;
                diameter = 6.0;
                return true;

            // Approximate historical visual-brightness from McCants.
            // Iridium brightness is strongly attitude-dependent and the
            // original spacecraft were capable of large specular flares.

            case SSSatellite::kIridium:
                stdmag = +3.5;
                diameter = 4.0;
                return true;

            // Approximation for the small ORBCOMM LEO spacecraft.
                
            case SSSatellite::kOrbcomm:
                stdmag = +9.2;
                diameter = 1.0;
                return true;

            // Approximation for first-generation Globalstar communications satellites.
            
            case SSSatellite::kGlobalstar:
                stdmag = +4.2;
                diameter = 5.0;
                return true;

            // Approximation for O3b MEO communications spacecraft.
                
            case SSSatellite::kO3b:
                stdmag = +6.8;
                diameter = 4.5;
                return true;

            // Approximation for Planet Dove/Flock spacecraft as 3U CubeSats.
                
            case SSSatellite::kFlock:
                stdmag = +9.5;
                diameter = 0.30;
                return true;

            // Approximation for Spire LEMUR spacecraft, based on 3U CubeSat form factor.
                
            case SSSatellite::kLemur:
                stdmag = +9.0;
                diameter = 0.35;
                return true;

            // Based on the SCORE-derived magnitudes for Kuiper satellites.
                
            case SSSatellite::kKuiper:
                stdmag = +6.8;
                diameter = 5.0;
                return true;

            // Based on SCORE: median +5.28 from 73 satellites.
                
            case SSSatellite::kQianfan:
                stdmag = +5.3;
                diameter = 5.0;
                return true;

            // Based on SCORE: median +4.05 from 19 satellites.
                
            case SSSatellite::kHulianwang:
                stdmag = +4.1;
                diameter = 5.0;
                return true;

            // Approximate; no reliable constellation-wide  visual photometry.
                
            case SSSatellite::kGeeSAT:
                stdmag = +7.5;
                diameter = 1.5;
                return true;

            // Approximation based on McCants values. GPS spans several spacecraft
            // blocks with different buses and solar-array configurations.
            
            case SSSatellite::kGPS:
            case SSSatellite::kGPSOps:
                stdmag = +4.5;
                diameter = 5.0;
                return true;

            // Approximation from McCants values across multiple GLONASS generations.
            
            case SSSatellite::kGLONASS:
            case SSSatellite::kGLONASSOps:
                stdmag = +4.8;
                diameter = 4.0;
                return true;

            // Approximation for Galileo navigation spacecraft based on McCants values.
                
            case SSSatellite::kGalileo:
                stdmag = +5.0;
                diameter = 4.0;
                return true;

            // Approximation for BeiDou includes multiple spacecraft designs operating in MEO, IGSO, and GEO.
            
            case SSSatellite::kBeiDou:
                stdmag = +5.0;
                diameter = 4.0;
                return true;

            // Approximation for IRNSS/NavIC GEO and IGSO navigation spacecraft;
            // no reliable constellation-wide visual photometry.
                
            case SSSatellite::kIRNSS:
                stdmag = +5.5;
                diameter = 4.0;
                return true;

            // Approximation for QZSS spacecraft designs in IGSO and GEO orbits.
                
            case SSSatellite::kQZSS:
                stdmag = +4.5;
                diameter = 5.0;
                return true;

            default:
                break;
        }
    }

    return false;
}

// Sets constellation-default standard magnitude and diameter for all satellites in a veector of SSObjectPtr.
// Returns the total number of satellites to which magnitude and diameter have been assigned.

int SSGetDefaultSatelliteData ( SSObjectVec &satellites )
{
    int n = 0;
    for ( size_t i = 0; i < satellites.size(); i++ )
    {
        SSSatellitePtr pSat = SSGetSatellitePtr ( satellites[i] );
        if ( pSat == nullptr )
            continue;
        
        float stdmag = INFINITY, diam = 0.0;
        if ( SSGetDefaultSatelliteData ( pSat, stdmag, diam ) )
        {
            pSat->setHMagnitude ( stdmag );
            pSat->setRadius ( diam / 2000.0 );
            pSat->setMagnitudeSource ( SSSatellite::kConstellation );
            pSat->setDiameterSource ( SSSatellite::kConstellation );
            n++;
        }
    }
    return n;
}
