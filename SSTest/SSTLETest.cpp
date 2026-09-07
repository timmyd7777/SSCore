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

#include <set>

void SCOREDataDiagnostics ( const SCOREDataVec &data )
{
    size_t nTotal = 0;
    size_t nMag = 0;
    size_t nObserver = 0;
    size_t nRADec = 0;
    size_t nRange = 0;
    size_t nRADecRange = 0;
    size_t nUsable = 0;

    set<int> satTotal;
    set<int> satMag;
    set<int> satObserver;
    set<int> satRADec;
    set<int> satRange;
    set<int> satRADecRange;
    set<int> satUsable;

    for ( const SCOREData &obs : data )
    {
        nTotal++;
        satTotal.insert ( obs.norad );

        // A missing magnitude is stored as INFINITY by SSImportSCOREData().
        if ( ! isfinite ( obs.mag ) )
            continue;

        nMag++;
        satMag.insert ( obs.norad );

        // Importer currently treats lon = lat = 0 as missing observer coordinates.
        if ( obs.lon == 0.0 && obs.lat == 0.0 )
            continue;

        nObserver++;
        satObserver.insert ( obs.norad );

        bool hasRADec = ! ( obs.ra == 0.0 && obs.dec == 0.0 );
        bool hasRange = isfinite ( obs.range ) && obs.range > 0.0;

        if ( hasRADec )
        {
            nRADec++;
            satRADec.insert ( obs.norad );
        }

        if ( hasRange )
        {
            nRange++;
            satRange.insert ( obs.norad );
        }

        if ( hasRADec && hasRange )
        {
            nRADecRange++;
            satRADecRange.insert ( obs.norad );
        }

        if ( ! hasRADec || ! hasRange )
            continue;

        nUsable++;
        satUsable.insert ( obs.norad );
    }

    printf ( "\nSCORE data diagnostics:\n" );
    printf ( "%-30s %10s %10s\n", "", "Obs", "Sats" );
    printf ( "%-30s %10zu %10zu\n", "Total:", nTotal, satTotal.size() );
    printf ( "%-30s %10zu %10zu\n", "Valid magnitude:", nMag, satMag.size() );
    printf ( "%-30s %10zu %10zu\n", "Valid observer position:", nObserver, satObserver.size() );
    printf ( "%-30s %10zu %10zu\n", "Has RA/Dec:", nRADec, satRADec.size() );
    printf ( "%-30s %10zu %10zu\n", "Has range:", nRange, satRange.size() );
    printf ( "%-30s %10zu %10zu\n", "Has RA/Dec + range:", nRADecRange, satRADecRange.size() );
    printf ( "%-30s %10zu %10zu\n", "Usable by current reduction:", nUsable, satUsable.size() );
}

void SCORECompareSatCheckerGeometry ( const SCOREDataVec &data )
{
    vector<double> sep;
    vector<double> rangeDiff;
    vector<double> rangeFrac;

    for ( const auto &obs : data )
    {
        if ( obs.ra == 0.0 && obs.dec == 0.0 )
            continue;

        if ( obs.range <= 0.0 )
            continue;

        if ( obs.ra_satchecker == 0.0 && obs.dec_satchecker == 0.0 )
            continue;

        if ( obs.range_satchecker <= 0.0 )
            continue;

        // Angular separation between observed and SatChecker directions.

        double cossep =
            sin ( obs.dec ) * sin ( obs.dec_satchecker ) +
            cos ( obs.dec ) * cos ( obs.dec_satchecker ) *
            cos ( obs.ra - obs.ra_satchecker );

        cossep = max ( -1.0, min ( 1.0, cossep ) );

        double angle = acos ( cossep );

        // Range residual: SatChecker minus observed.

        double dr = obs.range_satchecker - obs.range;

        sep.push_back ( radtodeg ( angle ) );
        rangeDiff.push_back ( dr );
        rangeFrac.push_back ( dr / obs.range );
    }

    if ( sep.empty() )
        return;

    double sepMed = median ( sep.data(), sep.size() );
    double sepMAD = mad ( sep.data(), sep.size(), sepMed );

    double rangeMed = median ( rangeDiff.data(), rangeDiff.size() );
    double rangeMAD = mad ( rangeDiff.data(), rangeDiff.size(), rangeMed );

    double fracMed = median ( rangeFrac.data(), rangeFrac.size() );
    double fracMAD = mad ( rangeFrac.data(), rangeFrac.size(), fracMed );

    printf ( "\nSCORE observed vs SatChecker geometry:\n\n" );
    printf ( "Observations compared: %zu\n", sep.size() );
    printf ( "Angular separation:   median %.6f deg, MAD %.6f deg\n",
             sepMed, sepMAD );
    printf ( "Range difference:     median %+.3f km, MAD %.3f km\n",
             rangeMed, rangeMAD );
    printf ( "Fractional range:     median %+.6f, MAD %.6f\n",
             fracMed, fracMAD );
}

void SCOREPrintInstrumentStatistics ( const SCOREDataVec &data )
{
    struct Stats
    {
        int nobs = 0;
        map<int,vector<float>> stdmags;
        vector<float> appmags;
        vector<double> ranges;
    };

    map<string,Stats> stats;

    for ( const SCOREData &obs : data )
    {
        if ( ! isfinite ( obs.mag ) )
            continue;

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

        if ( ( ra == 0.0 && dec == 0.0 ) || range <= 0.0 )
            continue;

        SSTime time ( obs.time );
        SSSpherical location ( obs.lon, obs.lat, obs.alt );
        SSCoordinates coords ( time, location );

        SSVector direction = SSSpherical ( ra, dec );
        SSVector position = coords.getObserverPosition()
                          + direction * ( range / SSCoordinates::kKmPerAU );

        double phase = SSPlanet::phaseAngle ( position, direction );

        float stdmag = SSSatellite::computeStandardMagnitude ( range, phase, obs.mag );
        if ( ! isfinite ( stdmag ) )
            continue;

        // Change this key if observer/mode/filter are added to SCOREData.
        string key = obs.instrument;

        Stats &s = stats[key];

        s.nobs++;
        s.stdmags[obs.norad].push_back ( stdmag );
        s.appmags.push_back ( obs.mag );
        s.ranges.push_back ( range );
    }

    printf ( "\n%-28s %7s %7s %10s %9s %9s %9s\n",
             "Instrument", "NObs", "NSat", "Range km",
             "AppMag", "StdMag", "MAD" );

    for ( auto &item : stats )
    {
        const string &instrument = item.first;
        Stats &s = item.second;

        vector<float> satmags;

        for ( auto &sat : s.stdmags )
        {
            vector<float> &mags = sat.second;

            if ( ! mags.empty() )
                satmags.push_back ( median ( mags.data(), mags.size() ) );
        }

        if ( satmags.empty() )
            continue;

        float appmed = median ( s.appmags.data(), s.appmags.size() );
        double rangemed = median ( s.ranges.data(), s.ranges.size() );

        float stdmed = median ( satmags.data(), satmags.size() );
        float stdmad = mad ( satmags.data(), satmags.size(), stdmed );

        printf ( "%-28s %7d %7zu %10.0f %9.2f %9.2f %9.2f\n",
                 instrument.c_str(),
                 s.nobs,
                 satmags.size(),
                 rangemed,
                 appmed,
                 stdmed,
                 stdmad );
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
    { "tle-new.csv",       SSSatellite::kNewest },
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
    { "gnss.csv",          SSSatellite::kGNSS },
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
    { "analyst.csv",       SSSatellite::kAnalyst },
    { "active.csv",        SSSatellite::kActive },
};

// Data used to calibrate and evaluate GCAT physical dimensions as a
// predictor of McCants-style visual standard magnitude.

struct GCATMagnitudeCalibration
{
    int    norad = 0;
    string name = "";
    float  mag = INFINITY;       // McCants standard magnitude
    float  length = 0.0;         // GCAT main-body length (m)
    float  diameter = 0.0;       // GCAT main-body diameter (m)
    float  area = 0.0;           // length * diameter (m^2)
    float  span = 0.0;
    float  constant = INFINITY;  // mag + 2.5 log10(area)
    string lengthFlag = "";
    string diameterFlag = "";
    string shape = "";
    string bus = "";
};

typedef vector<GCATMagnitudeCalibration> GCATMagnitudeCalibrationVec;

// Builds a calibration data set by matching McCants visual standard
// magnitudes with GCAT satellite dimensions by NORAD number.
// Only McCants magnitudes derived from visual observations are used.
// For each match, the GCAT main-body area is computed as length * diameter
// and the corresponding magnitude constant is:
//
// C = mag + 2.5 * log10 ( area )
//
// The median and median absolute deviation (MAD) of these constants are
// returned for use in calibrating GCAT-derived standard magnitudes.
// Parameters:
//
// mcnames         McCants satellite magnitude data.
// gcat            GCAT satellite physical data.
// calibration     Receives the matched calibration records.
// medianConstant  Receives the median magnitude constant.
// madConstant     Receives the MAD of the magnitude constants.
//
// Returns Number of matched calibration records.

int McNamesToGCATMagnitudeCalibration ( const McNameMap &mcnames,
                                       const GCATDataVec &gcat,
                                       GCATMagnitudeCalibrationVec &calibration,
                                       float &medianConstant,
                                       float &madConstant )
{
    map<int,const GCATData *> gcatmap;
    vector<float> constants;

    medianConstant = INFINITY;
    madConstant = INFINITY;

    // Build NORAD -> GCAT record map. Ignore GCAT records without
    // an assigned NORAD number.

    for ( const auto &data : gcat )
    {
        if ( data.norad > 0 )
            gcatmap[data.norad] = &data;
    }

    // Correlate visually-derived McCants magnitudes with GCAT dimensions.

    for ( const auto &item : mcnames )
    {
        const McName &mcname = item.second;

        // Only use McCants visually observed magnitudes. Dimension-derived
        // McCants magnitudes would make this calibration circular.

        if ( mcname.source != 'v' || ! isfinite ( mcname.mag ) )
            continue;

        auto it = gcatmap.find ( mcname.norad );
        if ( it == gcatmap.end() )
            continue;

        const GCATData &data = *it->second;

        if ( data.length <= 0.0 || data.diameter <= 0.0 )
            continue;

        float area = data.length * data.diameter;
        float constant = mcname.mag + 2.5 * log10 ( area );

        GCATMagnitudeCalibration result;

        result.norad = mcname.norad;
        result.name = mcname.name;
        result.mag = mcname.mag;
        result.length = data.length;
        result.diameter = data.diameter;
        result.area = area;
        result.span = data.span;
        result.constant = constant;
        result.lengthFlag = data.lengthFlag;
        result.diameterFlag = data.diameterFlag;
        result.shape = data.shape;
        result.bus = data.bus;

        calibration.push_back ( result );
        constants.push_back ( constant );
    }

    if ( constants.empty() )
        return 0;

    sort ( constants.begin(), constants.end() );
    medianConstant = median ( constants.data(), constants.size() );
    madConstant = mad ( constants.data(), constants.size(), medianConstant );

    return (int) constants.size();
}

// Analyzes the relationship between McCants visual standard magnitudes
// and GCAT satellite dimensions.
// Evaluates the GCAT area-based magnitude model, including fixed and
// free linear fits, residual statistics by satellite size and shape,
// and the largest individual residuals. GCATEffectiveArea() is used
// to account for broad appendages such as panels, sails, and plates.
// Paramter calibration is Matched McCants/GCAT calibration records.
// Results are written to standard output for diagnostic analysis.

void AnalyzeGCATMagnitudeCalibration ( const GCATMagnitudeCalibrationVec &calibration )
{
    vector<float> constants;
    vector<float> cleanConstants;
    vector<float> simpleConstants;
    vector<float> appendageConstants;

    double sx = 0.0, sy = 0.0;
    double sxx = 0.0, sxy = 0.0;
    int nfit = 0;

    double simpleSX = 0.0, simpleSY = 0.0;
    double simpleSXX = 0.0, simpleSXY = 0.0;
    int simpleN = 0;

    struct SizeBin
    {
        string name;
        vector<float> residuals;
    };

    SizeBin bins[] =
    {
        { "A < 0.1",      {} },
        { "0.1 <= A < 1", {} },
        { "1 <= A < 10",  {} },
        { "A >= 10",      {} }
    };

    struct Outlier
    {
        float absres;
        float residual;
        float predicted;
        float effectiveArea;
        const GCATMagnitudeCalibration *data;
    };

    vector<Outlier> outliers;
    const float c0 = 7.98689f;

    for ( const auto &data : calibration )
    {
        if ( data.area <= 0.0 || ! isfinite ( data.mag ) )
            continue;

        float area = GCATEffectiveArea( data.length, data.diameter, data.span, data.shape );
        float x = log10 ( area );
        float predicted = c0 - 2.5f * x;
        float residual = data.mag - predicted;

        constants.push_back ( data.constant );

        if ( data.lengthFlag != "?" && data.diameterFlag != "?" )
            cleanConstants.push_back ( data.constant );

        // Unconstrained linear fit:
        //
        // mag = a + b log10(area)

        sx += x;
        sy += data.mag;
        sxx += x * x;
        sxy += x * data.mag;
        nfit++;

        // Separate simple main-body shapes from objects whose
        // GCAT shape explicitly lists appendages.

        if ( data.shape.find ( '+' ) == string::npos )
        {
            simpleConstants.push_back ( data.constant );

            simpleSX += x;
            simpleSY += data.mag;
            simpleSXX += x * x;
            simpleSXY += x * data.mag;
            simpleN++;
        }
        else
        {
            appendageConstants.push_back ( data.constant );
        }

        if ( data.area < 0.1 )
            bins[0].residuals.push_back ( residual );
        else if ( data.area < 1.0 )
            bins[1].residuals.push_back ( residual );
        else if ( data.area < 10.0 )
            bins[2].residuals.push_back ( residual );
        else
            bins[3].residuals.push_back ( residual );

        outliers.push_back ( { fabs ( residual ), residual, predicted, area, &data } );
    }

    cout << endl;
    cout << "GCAT magnitude calibration analysis" << endl;
    cout << "-----------------------------------" << endl;

    if ( ! constants.empty() )
    {
        float med = median ( constants.data(), constants.size() );
        float md = mad ( constants.data(), constants.size(), med );

        cout << "All dimensions:" << endl;
        cout << "N = " << constants.size() << endl;
        cout << "Median C = " << med << endl;
        cout << "MAD C = " << md << endl;
        cout << endl;
    }

    if ( ! cleanConstants.empty() )
    {
        float med = median ( cleanConstants.data(), cleanConstants.size() );
        float md = mad ( cleanConstants.data(), cleanConstants.size(), med );

        cout << "Dimensions without ? flags:" << endl;
        cout << "N = " << cleanConstants.size() << endl;
        cout << "Median C = " << med << endl;
        cout << "MAD C = " << md << endl;
        cout << endl;
    }

    if ( nfit > 1 )
    {
        double denom = nfit * sxx - sx * sx;

        if ( denom != 0.0 )
        {
            double b = ( nfit * sxy - sx * sy ) / denom;
            double a = ( sy - b * sx ) / nfit;

            cout << "Free linear fit:" << endl;
            cout << "mag = " << a << " + " << b << " log10(area)" << endl;
            cout << "Expected slope = -2.5" << endl;
            cout << endl;
        }
    }

    if ( ! simpleConstants.empty() )
    {
        float med = median ( simpleConstants.data(), simpleConstants.size() );
        float md = mad ( simpleConstants.data(), simpleConstants.size(), med );

        cout << "Simple shapes:" << endl;
        cout << "N = " << simpleConstants.size() << endl;
        cout << "Median C = " << med << endl;
        cout << "MAD C = " << md << endl;
        cout << endl;
    }

    if ( ! appendageConstants.empty() )
    {
        float med = median ( appendageConstants.data(), appendageConstants.size() );
        float md = mad ( appendageConstants.data(), appendageConstants.size(), med );

        cout << "Shapes with appendages:" << endl;
        cout << "N = " << appendageConstants.size() << endl;
        cout << "Median C = " << med << endl;
        cout << "MAD C = " << md << endl;
    }

    if ( simpleN > 1 )
    {
        double denom = simpleN * simpleSXX - simpleSX * simpleSX;

        if ( denom != 0.0 )
        {
            double b = ( simpleN * simpleSXY - simpleSX * simpleSY ) / denom;
            double a = ( simpleSY - b * simpleSX ) / simpleN;

            cout << endl;
            cout << "Simple-shape free fit:" << endl;
            cout << "mag = " << a << " + " << b << " log10(area)" << endl;
        }
    }

    cout << endl;
    cout << "Residuals by area:" << endl;

    for ( auto &bin : bins )
    {
        if ( bin.residuals.empty() )
            continue;

        float med = median ( bin.residuals.data(), bin.residuals.size() );
        float md = mad ( bin.residuals.data(), bin.residuals.size(), med );

        cout << bin.name
             << ": N=" << bin.residuals.size()
             << " median=" << med
             << " MAD=" << md
             << endl;
    }

    cout << endl;

    sort ( outliers.begin(), outliers.end(),
        [] ( const Outlier &a, const Outlier &b )
        {
            return a.absres > b.absres;
        } );

    cout << "Largest residuals:" << endl;

    int nout = min ( 20, (int) outliers.size() );

    for ( int i = 0; i < nout; i++ )
    {
        const auto &o = outliers[i];
        const auto &d = *o.data;

        cout << d.norad << "\t"
             << d.name << "\t"
             << "obs=" << d.mag << "\t"
             << "pred=" << o.predicted << "\t"
             << "res=" << o.residual << "\t"
             << "L=" << d.length << d.lengthFlag << "\t"
             << "D=" << d.diameter << d.diameterFlag << "\t"
             << "A=" << d.area << "\t"
             << "span=" << d.span << "\t"
             << "Aeff=" << o.effectiveArea << "\t"
             << "shape=" << d.shape << "\t"
             << "bus=" << d.bus
             << endl;
    }
}

// Compares SCORE-derived standard magnitudes with magnitudes estimated
// from GCAT satellite dimensions.
// Matches SCORE and GCAT records by NORAD number, computes GCAT effective
// area and its corresponding McCants-style standard magnitude, then
// calculates residuals between the SCORE and GCAT estimates. Reports
// individual comparisons and robust median/MAD residual statistics.
// Parameters:
// score  SCORE-derived satellite standard magnitudes.
// gcat   GCAT satellite physical data.
// Results are written to standard output for diagnostic analysis.

void AnalyzeSCOREvsGCATMagnitudes ( const SCOREMagnitudeMap &score, const GCATDataVec &gcat )
{
    map<int,const GCATData *> gcatByNorad;

    for ( const auto &data : gcat )
    {
        if ( data.norad > 0 )
            gcatByNorad[data.norad] = &data;
    }

    vector<float> residuals;

    cout << endl;
    cout << "SCORE vs GCAT magnitude comparison" << endl;
    cout << "----------------------------------" << endl;
    cout << "NORAD\tName\tSCORE\tL\tD\tArea\tSpan\tAeff\tGCAT\tResidual\tShape\tBus" << endl;

    for ( const auto &entry : score )
    {
        const SCOREMagnitude &smag = entry.second;

        auto it = gcatByNorad.find ( smag.norad );
        if ( it == gcatByNorad.end() )
            continue;

        const GCATData &data = *it->second;

        if ( data.length <= 0.0f || data.diameter <= 0.0f )
            continue;

        float rawArea = data.length * data.diameter;
        float area = GCATEffectiveArea ( data.length, data.diameter,
                                         data.span, data.shape );

        if ( area <= 0.0f )
            continue;

        float gcatMag = 7.98689f - 2.5f * log10f ( area );
        float residual = smag.mag - gcatMag;

        residuals.push_back ( residual );

        cout << smag.norad << "\t"
             << smag.name << "\t"
             << smag.mag << "\t"
             << data.length
             << ( data.lengthFlag == "?" ? "?" : "" ) << "\t"
             << data.diameter
             << ( data.diameterFlag == "?" ? "?" : "" ) << "\t"
             << rawArea << "\t"
             << data.span << "\t"
             << area << "\t"
             << gcatMag << "\t"
             << residual << "\t"
             << data.shape << "\t"
             << data.bus << endl;
    }

    cout << endl;
    cout << "Matched satellites with GCAT dimensions: " << residuals.size() << endl;

    if ( residuals.empty() )
        return;

    float med = median ( residuals.data(), residuals.size() );
    float md = mad ( residuals.data(), residuals.size(), med );

    cout << "Median residual SCORE - GCAT: " << med << endl;
    cout << "MAD residual:                 " << md << endl;
}

int main ( int argc, const char *argv[] )
{
    if ( argc < 9 )
    {
        cerr << "Usage: SSTLETest "
             << "celestrak_dir n2yo_dir n2yo_file mccants_file score_file je9pel_file satcat_file satcat100k_file"
             << endl;

        return -1;
    }

    string celestrakDir   = argv[1];
    string n2yoDir        = argv[2];
    string n2yoFile       = argv[3];
    string mccantsFile    = argv[4];
    string scoreFile      = argv[5];
    string je9pelFile     = argv[6];
    string satcatFile     = argv[7];
    string satcat100kFile = argv[8];
    
    /* Test code for deriving relationship between Jonathan McDowell's GCAT spacecraft size
       versus McCants and SCORE satellite standard magnitude observations
     
    SCOREDataVec scoreobs;
    SCOREMagnitudeMap scoremags;

    int nobs = SSImportSCOREData ( scoreFile, scoreobs );
    cerr << "Imported " << nobs << " SCORE observations." << endl;
    
    int nscm = SCOREDataToStandardMagnitudes ( scoreobs, scoremags );
    cerr << "Imported and computed " << nscm << " SCORE satellite magnitudes." << endl;

    //SCOREDataDiagnostics ( scoreobs );
    //SCORECompareSatCheckerGeometry ( scoreobs );
    //SCOREPrintInstrumentStatistics ( scoreobs );

    McNameMap mcnames;
    int nmcn = SSImportMcNames ( mccantsFile, mcnames );
    cerr << "Imported " << nmcn << " McCants records." << endl;

    GCATDataVec gcat;
    int ngcat =  SSImportGCATData ( satcatFile, gcat );
    ngcat += SSImportGCATData ( satcat100kFile, gcat );
    cerr << "Imported " << ngcat << " GCAT satellite records." << endl;
    
    float cmed, cmad;
    GCATMagnitudeCalibrationVec calibration;
    int n = McNamesToGCATMagnitudeCalibration ( mcnames, gcat, calibration, cmed, cmad );

    cout << "GCAT/McCants visual matches: " << n << endl;
    cout << "Median constant: " << cmed << endl;
    cout << "MAD constant:    " << cmad << endl;
    
    AnalyzeGCATMagnitudeCalibration ( calibration );
    AnalyzeSCOREvsGCATMagnitudes ( scoremags, gcat );
    
    TestSCOREData ( scoreFile );  */
    
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

    // Read Jonathan McDowell's GCAT satellite catalog TSV files and get size, mass, and derived magnitudes.
    
    GCATDataVec gcat;
    int ngcat =  SSImportGCATData ( satcatFile, gcat );
    ngcat += SSImportGCATData ( satcat100kFile, gcat );
    cerr << "Imported " << ngcat << " GCAT satellite records." << endl;

    ngcat = SSImportGCATData( gcat, satellites );
    cerr << "Added GCAT size/mass/magnitude for " << ngcat << " satellites." << endl;

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
