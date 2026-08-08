//  SSTLETest.cpp
//
//  Created by Tim DeBenedictis on 3/9/24.
//  Copyright © 2024 Southern Stars. All rights reserved.

#include <iostream>
#include "SSCoordinates.hpp"
#include "SSTLE.hpp"
#include "SSImportTLE.hpp"

// Tests derivation of satellite standard magnitudes from SCORE data obtained
// in CSV format from https://score.cps.iau.org/download

void TestSCOREData ( const string &scorepath )
{
    SCOREDataVec observations;
    SCOREMagnitudeMap magnitudes;

    int nobs = SSImportSCOREData ( scorepath, observations );
    if ( nobs == 0 )
    {
        cerr << "Can't import SCORE data from " << scorepath << endl;
        return;
    }

    cerr << "Imported " << nobs << " SCORE observations." << endl;

    int nsat = SCOREDataToStandardMagnitudes ( observations, magnitudes );
    if ( nsat == 0 )
    {
        cerr << "Can't derive SCORE standard magnitudes." << endl;
        return;
    }

    cerr << "Derived standard magnitudes for " << nsat << " satellites." << endl << endl;
    fprintf ( stderr, "%6s  %-24s  %6s  %4s  %6s\n", "NORAD", "Name", "StdMag", "MAD", "NObs" );
    for ( auto &item : magnitudes )
    {
        int norad = item.first;
        SCOREMagnitude &mag = item.second;
        fprintf ( stderr, "%6d  %-24.24s  %+6.2f  %4.2f  %6d\n",
                 mag.norad,
                 mag.name.c_str(),
                 mag.mag,
                 mag.mad,
                 mag.nobs );
    }
}

// Imports one satellite category file in TLE or CelesTrak CSV format.
// Satellites already present in the input object vector simply receive the
// specified category. New satellites are created as needed.
// Returns the number of TLE records successfully processed.

int SSImportSatelliteCategoryFile ( const string &filename, SSSatellite::Category category, SSObjectVec &satellites )
{
    FILE *file = fopen ( filename.c_str(), "rb" );
    if ( file == nullptr )
        return 0;

    // build lookup table of satellites by NORAD ID
    
    map<int,SSSatellite *> satmap;
    for ( int i = 0; i < satellites.size(); i++ )
    {
        SSSatellite *pSat = SSGetSatellitePtr ( satellites[i] );
        if ( pSat )
            satmap[pSat->getTLE().norad] = pSat;
    }
    
    bool csv = toLower ( getFileExt ( filename ) ) == ".csv";

    SSTLE tle;
    int n = 0;

    while ( true )
    {
        int result = csv ? tle.read_csv ( file ) : tle.read ( file );

        if ( result == EOF )
            break;

        if ( result != 0 )
            continue;

        auto it = satmap.find ( tle.norad );
        if ( it != satmap.end() )
        {
            SSSatellite *pSat = it->second;
            pSat->addCategory ( category );
            if ( tle.jdepoch > pSat->getTLE().jdepoch )
                pSat->setTLE ( tle );
        }
        else
        {
            SSSatellite *pSat = new SSSatellite ( tle );
            if ( pSat )
            {
                pSat->addCategory ( category );
                satellites.append ( SSObjectPtr ( pSat ) );
                satmap.insert ( { tle.norad, pSat } );
            }
        }

        n++;
    }

    fclose ( file );
    return n;
}

// Imports all satellite files in a directory, either TLE or CSV format.
// Existing satellites receive additional categories; new satellites are created as needed.
// Returns the number of TLE records imported.

int SSImportTLEDirectory ( const string &dirname, const map<string,SSSatellite::Category> &categories, SSObjectVec &satellites )
{
    vector<string> files;
    int err = listDirectory ( dirname, files );
    if ( err != 0 )
        return 0;

    int n = 0;
    for ( string &path : files )
    {
		SSSatellite::Category category = SSSatellite::Category::kNone;
        string filename = getFileName ( path );
        auto it = categories.find ( filename );
        if ( it != categories.end() )
            category = it->second;
        
        path = dirname + "/" + filename;
        n += SSImportSatelliteCategoryFile ( path, category, satellites );
    }

    return n;
}

// Maps N2YO satellite category filenames to N2YO category identifiers.

static const map<string,SSSatellite::Category> N2YOSatelliteCategoryFiles =
{
    { "brightest.txt",         SSSatellite::kBrightest },
    { "stations.txt",          SSSatellite::kISS },
    { "weather.txt",           SSSatellite::kWeather },
    { "noaa.txt",              SSSatellite::kNOAA },
    { "goes.txt",              SSSatellite::kGOES },
    { "resource.txt",          SSSatellite::kEarthResources },
    { "sarsat.txt",            SSSatellite::kSearchAndRescue },
    { "dmc.txt",               SSSatellite::kDisasterMonitoring },
    { "tdrss.txt",             SSSatellite::kTDRS },
    { "geo.txt",               SSSatellite::kGeostationary },
    { "intelsat.txt",          SSSatellite::kIntelsat },
    { "gorizont.txt",          SSSatellite::kGorizont },
    { "raduga.txt",            SSSatellite::kRaduga },
    { "molniya.txt",           SSSatellite::kMolniya },
    { "iridium.txt",           SSSatellite::kIridium },
    { "orbcomm.txt",           SSSatellite::kOrbcomm },
    { "globalstar.txt",        SSSatellite::kGlobalstar },
    { "amateur.txt",           SSSatellite::kAmateur },
    { "x-comm.txt",            SSSatellite::kExperimental },
    { "gps-ops.txt",           SSSatellite::kGPSOps },
    { "glo-ops.txt",           SSSatellite::kGLONASSOps },
    { "galileo.txt",           SSSatellite::kGalileo },
    { "sbas.txt",              SSSatellite::kSBAS },
    { "nnss.txt",              SSSatellite::kNNSS },
    { "musson.txt",            SSSatellite::kMusson },
    { "science.txt",           SSSatellite::kScience },
    { "geodetic.txt",          SSSatellite::kGeodetic },
    { "engineering.txt",       SSSatellite::kEngineering },
    { "education.txt",         SSSatellite::kEducation },
    { "military.txt",          SSSatellite::kMilitary },
    { "radar.txt",             SSSatellite::kRadar },
    { "cubesat.txt",           SSSatellite::kCubeSat },
    { "sirius-xm.txt",         SSSatellite::kSiriusXM },
    { "tv.txt",                SSSatellite::kTV },
    { "beidou.txt",            SSSatellite::kBeiDou },
    { "yaogan.txt",            SSSatellite::kYaogan },
    { "westford.txt",          SSSatellite::kWestford },
    { "parus.txt",             SSSatellite::kParus },
    { "strela.txt",            SSSatellite::kStrela },
    { "gonets.txt",            SSSatellite::kGonets },
    { "tsiklon.txt",           SSSatellite::kTsiklon },
    { "tsikada.txt",           SSSatellite::kTsikada },
    { "o3b.txt",               SSSatellite::kO3b },
    { "tselina.txt",           SSSatellite::kTselina },
    { "celestis.txt",          SSSatellite::kCelestis },
    { "irnss.txt",             SSSatellite::kIRNSS },
    { "qzss.txt",              SSSatellite::kQZSS },
    { "flock.txt",             SSSatellite::kFlock },
    { "lemur.txt",             SSSatellite::kLemur },
    { "gps.txt",               SSSatellite::kGPS },
    { "glonass.txt",           SSSatellite::kGLONASS },
    { "starlink.txt",          SSSatellite::kStarlink },
    { "oneweb.txt",            SSSatellite::kOneWeb },
    { "chinese-stations.txt",  SSSatellite::kChineseStation },
    { "qianfan.txt",           SSSatellite::kQianfan },
    { "leo.txt",               SSSatellite::kKuiper },
    { "geesat.txt",            SSSatellite::kGeeSAT },
    { "junk.txt",              SSSatellite::kJunk }
};

// Maps CelesTrak satellite category filenames to category identifiers.

static const map<string,SSSatellite::Category> CelesTrakSatelliteCategoryFiles =
{
    { "visual.csv",        SSSatellite::kBrightest },
    { "stations.csv",      SSSatellite::kISS },
    { "weather.csv",       SSSatellite::kWeather },
    { "resource.csv",      SSSatellite::kEarthResources },
    { "sarsat.csv",        SSSatellite::kSearchAndRescue },
    { "tdrss.csv",         SSSatellite::kTDRS },
    { "geo.csv",           SSSatellite::kGeostationary },
    { "gpz.csv",           SSSatellite::kGEOProtectedZone },
    { "intelsat.csv",      SSSatellite::kIntelsat },
    { "eutelsat.csv",      SSSatellite::kEutelsat },
    { "iridium-next.csv",  SSSatellite::kIridium },
    { "orbcomm.csv",       SSSatellite::kOrbcomm },
    { "globalstar.csv",    SSSatellite::kGlobalstar },
    { "amateur.csv",       SSSatellite::kAmateur },
    { "x-comm.csv",        SSSatellite::kExperimental },
    { "gps-ops.csv",       SSSatellite::kGPSOps },
    { "glo-ops.csv",       SSSatellite::kGLONASSOps },
    { "galileo.csv",       SSSatellite::kGalileo },
    { "beidou.csv",        SSSatellite::kBeiDou },
    { "sbas.csv",          SSSatellite::kSBAS },
    { "gnss.csv",          SSSatellite::kGPS }, // suspect
    { "science.csv",       SSSatellite::kScience },
    { "geodetic.csv",      SSSatellite::kGeodetic },
    { "engineering.csv",   SSSatellite::kEngineering },
    { "education.csv",     SSSatellite::kEducation },
    { "military.csv",      SSSatellite::kMilitary },
    { "radar.csv",         SSSatellite::kRadar },
    { "cubesat.csv",       SSSatellite::kCubeSat },
    { "planet.csv",        SSSatellite::kFlock },
    { "sar.csv",           SSSatellite::kSAR },
    { "ses.csv",           SSSatellite::kSES },
    { "spire.csv",         SSSatellite::kLemur },
    { "satnogs.csv",       SSSatellite::kSatNOGS },
    { "starlink.csv",      SSSatellite::kStarlink },
    { "oneweb.csv",        SSSatellite::kOneWeb },
    { "qianfan.csv",       SSSatellite::kQianfan },
    { "kuiper.csv",        SSSatellite::kKuiper },
    { "hulianwang.csv",    SSSatellite::kHulianwang },
    { "ses.csv",           SSSatellite::kSES },
    { "argos.csv",         SSSatellite::kARGOS },
    { "eutelsat.csv",      SSSatellite::kEutelsat },
    { "gpz.csv",           SSSatellite::kGEOProtectedZone }, // DOUBLE CHECK
    { "sar.csv",           SSSatellite::kSAR },
    { "telesat.csv",       SSSatellite::kTelesat },
    { "analyst.csv",       SSSatellite::kAnalyst }
};

int main ( int argc, const char *argv[] )
{
    if ( argc < 7 )
    {
        cerr << "Usage: SSTLETest "
             << "celestrak_dir n2yo_dir n2yo_file mccants_file score_file je9pel_file"
             << endl;

        return -1;
    }

    string celestrakDir = argv[1];
    string n2yoDir      = argv[2];
    string n2yoFile     = argv[3];
    string mccantsFile  = argv[4];
    string scoreFile    = argv[5];
    string je9pelFile   = argv[6];

	TestSCOREData ( scoreFile );
	
    // Import base satellite records from CelesTrak CSV.

    SSObjectVec satellites;
    int nsat = SSImportTLEDirectory ( celestrakDir, CelesTrakSatelliteCategoryFiles, satellites );
    if ( nsat == 0 )
        cerr << "Failed to import satellites from " << celestrakDir << endl;
    else
        cerr << "Imported " << nsat << " satellites from " << celestrakDir << endl;

    // Import base satellite records from n2yo.com TLE.

    nsat = SSImportTLEDirectory ( n2yoDir, N2YOSatelliteCategoryFiles, satellites );
    if ( nsat == 0 )
        cerr << "Failed to import satellites from " << n2yoDir << endl;
    else
        cerr << "Imported " << nsat << " satellites from " << n2yoDir << endl;

    // Add N2YO country, launch, decay, taxonomy, and description data.

    int nn2yo = SSImportN2Data ( n2yoFile, satellites );
    cerr << "Added N2YO data to " << nn2yo << " satellites." << endl;

    // Get default standard magnitude and diameter for satellites in categorized constellations.
    
    int ndefaults = SSGetDefaultSatelliteData ( satellites );
    cerr << "Added constellation default standard magnitude and diameter to " << ndefaults << " satellites." << endl;

    // Add McCants standard magnitudes and physical sizes. This will override defaults.

    int nmccants = SSImportMcNames ( mccantsFile, satellites );
    cerr << "Added McCants data to " << nmccants << " satellites." << endl;

    // Add SCORE-derived standard magnitudes. Since this follows McCants and defaults,
    // SCORE magnitudes replace McCants and default magnitudes where both are available.

    int nscore = SSImportSCOREData ( scoreFile, satellites );
    cerr << "Added SCORE magnitudes to " << nscore << " satellites." << endl;

    // Add JE9PEL amateur-radio frequency data.

    int nfreq = SSImportSatelliteFrequencyData ( je9pelFile, satellites );
    cerr << "Added " << nfreq << " JE9PEL frequency records." << endl;

    // Test round-trip export-import-export of all satellite data
    
    for ( int i = 0; i < satellites.size(); i++ )
    {
        SSSatellite *pSat = SSGetSatellitePtr ( satellites[i] );
        if ( pSat )
        {
            int norad = pSat->getTLE().norad;
            string csv = pSat->toCSV();
            SSObjectPtr pObject = SSSatellite::fromCSV ( csv );
            SSSatellitePtr pSat = SSGetSatellitePtr ( pObject );
            if ( pSat == nullptr )
            {
                cerr << "CSV import failed for NORAD " << norad << endl;
                cerr << csv << endl;
                continue;
            }
            
            string csv2 = pSat->toCSV();
            if ( csv != csv2 )
            {
                cerr << "CSV round-trip mismatch for NORAD " << norad << endl;
                cerr << csv << endl;
                cerr << csv2 << endl;
                continue;
            }
        }
    }

    // Sort the resulting file by NORAD catalog number.
    // Display the assembled master satellite catalog.

    auto cmp = [] ( const SSObjectPtr &pObj1, const SSObjectPtr &pObj2 ) -> bool
    {
        SSSatellite *pSat1 = SSGetSatellitePtr( pObj1 );
        SSSatellite *pSat2 = SSGetSatellitePtr( pObj2 );
        return pSat1->getTLE().norad < pSat2->getTLE().norad;
    };
    
    satellites.sort ( cmp );

    cout << SSSatellite::headerCSV() << endl;
    for ( int i = 0; i < satellites.size(); i++ )
    {
        SSSatellite *pSat = SSGetSatellitePtr ( satellites[i] );
        if ( pSat )
            cout << pSat->toCSV() << endl;
        
    }

#if 0
    SSTLE tle;
    while ( true )
    {
        int result = csvformat ? tle.read_csv ( tlefile ) : tle.read ( tlefile );
        if ( result == EOF )
            break;
        else if ( result != 0 )
            continue;
        
        // Write TLE to standard output for verification
        if ( tle.write ( cout ) )
            cout << endl << endl;
        
        // Compute amd display satellite position and velocity
        // at 1-minute intervals for 1 day from element epoch.
        printf ( "%-6s %-9s %-9s %-9s %-9s %-9s %-9s %-9s %-9s\n", "t", "x", "y", "z", "vx", "vy", "vz", "r", "v" );
        for ( double tsince = 0.0; tsince <= 1440.0; tsince += 1.0 )
        {
            SSVector pos, vel;
            
            if ( tle.deep )
                tle.sdp4 ( tsince, pos, vel );
            else
                tle.sgp4 ( tsince, pos, vel );
            
            pos *= SSTLE::kmper();
            vel *= SSTLE::kmper() / 60.0;
            
            printf ( "%6.1f %+9.3f %+9.3f %+9.3f %+9.6f %+9.6f %+9.6f %9.3f %9.6f\n",
                    tsince, pos.x, pos.y, pos.z, vel.x, vel.y, vel.z, pos.magnitude(), vel.magnitude() );
        }
    }
    
    fclose ( tlefile );
#endif
    return 0;
}
