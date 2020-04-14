//  SSTest.cpp
//  SSCore
//
//  Created by Tim DeBenedictis on 2/24/20.
//  Copyright © 2020 Southern Stars. All rights reserved.

#include <cstdio>
#include <iostream>

#if defined __APPLE__
#include <TargetConditionals.h>
#endif

#include "SSCoords.hpp"
#include "SSOrbit.hpp"
#include "SSDynamics.hpp"
#include "SSPlanet.hpp"
#include "SSStar.hpp"
#include "SSConstellation.hpp"
#include "SSImportHIP.hpp"
#include "SSImportSKY2000.hpp"
#include "SSImportNGCIC.hpp"
#include "SSImportMPC.hpp"
#include "SSImportGJ.hpp"
#include "SSJPLDEphemeris.hpp"
#include "SSTLE.hpp"

void exportCatalog ( SSObjectVec &objects, SSCatalog cat, int first, int last )
{
    SSObjectMap map = SSMakeObjectMap ( objects, cat );

    for ( int n = first; n <= last; n++ )
    {
        int i = map[ SSIdentifier ( cat, n ) ];
        if ( i > 0 )
        {
            SSObjectPtr pObj = objects[ i - 1 ];
            cout << pObj.get()->toCSV() << endl;
        }
    }
}

void TestTime ( void )
{
    SSTime now = SSTime::fromSystem();
    SSDate date = now;

    cout << "Current local date is " << format ( "%04hd-%02hd-%02.0f", date.year, date.month, floor ( date.day ) ) << endl;
    cout << "Current local time is " << format ( "%02hd:%02hd:%04.1f", date.hour, date.min, date.sec ) << endl;
    cout << "Current local time is " << format ( "%+.2f", date.zone ) << " hours east of UTC" << endl;
    cout << "Current Julian Date is " << format ( "%.6f", now.jd ) << endl;

    cout << "Current working directory is " << getcwd() << endl << endl;
};

void TestSatellites ( string inputDir, string outputDir )
{
    string filename = inputDir + "/SolarSystem/Satellites/visual.txt";
    ifstream file ( filename );
    if ( ! file )
    {
        cout << "Failed to open " << filename << endl;
        return;
    }
    
    string outfilename = outputDir + "/ExportedSatellites.tle";
    ofstream outfile ( outfilename );
    if ( ! outfile )
    {
        cout << "Failed to open " << outfilename << endl;
        return;
    }
    
    SSTLE tle;
    
    while ( tle.read ( file ) == 0 )
    {
        tle.write ( cout );
 
        SSVector pos, vel;

        tle.toPositionVelocity ( tle.jdepoch + 10.0, pos, vel );

        cout << format ( "JD %.6f  ", tle.jdepoch + 10.0 );
        cout << format ( "pos %+10.3f %+10.3f %+10.3f  ", pos.x, pos.y, pos.z );
        cout << format ( "vel %+7.3f %+7.3f %+7.3f",   vel.x, vel.y, vel.z ) << endl;

        SSTLE tle1 = tle;
        tle1.fromPositionVelocity ( tle.jdepoch + 10.0, pos, vel );
        
        SSTLE tle2 = tle1;
        tle1.toPositionVelocity ( tle1.jdepoch - 10.0, pos, vel );
        tle2.fromPositionVelocity ( tle1.jdepoch - 10.0, pos, vel );
        tle2.write ( cout );
        tle2.write ( outfile );
     }
}

void TestSolarSystem ( string inputDir, string outputDir )
{
    SSObjectVec planets;
    int numPlanets = SSImportObjectsFromCSV ( inputDir + "/SolarSystem/Planets.csv", planets );
    cout << "Imported " << numPlanets << " major planets" << endl;
    numPlanets = SSExportObjectsToCSV ( "", planets );
    
    SSObjectVec moons;
    int numMoons = SSImportObjectsFromCSV ( inputDir + "/SolarSystem/Moons.csv", moons );
    cout << "Imported " << numMoons << " natural satellites" << endl;
    
    SSObjectVec comets;
    int numComets = SSImportMPCComets ( inputDir + "/SolarSystem/Comets.txt", comets );
    cout << "Imported " << numComets << " MPC comets" << endl;

    SSObjectVec asteroids;
    int numAsteroids = SSImportMPCAsteroids ( inputDir + "/SolarSystem/Asteroids.txt", asteroids );
    cout << "Imported " << numAsteroids << " MPC asteroids" << endl;

    if ( ! outputDir.empty() )
    {
        numMoons = SSExportObjectsToCSV ( outputDir + "/ExportedMoons.csv", moons );
        cout << "Exported " << numMoons << " natural satellites to " << outputDir + "/ExportedMoons.csv" << endl;

        numComets = SSExportObjectsToCSV ( outputDir + "/ExportedComets.csv", comets );
        cout << "Exported " << numComets << " MPC comets to " << outputDir + "/ExportedComets.csv" << endl;

        numAsteroids = SSExportObjectsToCSV ( outputDir + "/ExportedAsteroids.csv", asteroids );
        cout << "Exported " << numAsteroids << " MPC asteroids to " << outputDir + "/ExportedAsteroids.csv" << endl;
    }
}

void TestConstellations ( string inputDir, string outputDir )
{
    SSObjectVec constellations;
    
    int numCons = SSImportConstellations ( inputDir + "/Constellations/Constellations.csv", constellations );
    cout << "Imported " << numCons << " IAU constellations" << endl;
    
    int numVerts = SSImportConstellationBoundaries ( inputDir + "/Constellations/Boundaries.csv", constellations );
    cout << "Imported " << numVerts << " IAU constellation boundary vertices" << endl;

    int numLines = SSImportConstellationShapes ( inputDir + "/Constellations/Shapes.csv", constellations );
    cout << "Imported " << numLines << " IAU constellation shape lines" << endl;

    if ( ! outputDir.empty() )
    {
        numCons = SSExportObjectsToCSV ( outputDir + "/ExportedConstellations.csv", constellations );
        cout << "Exported " << numCons << " constellations to " << outputDir + "/ExportedConstellations.csv" << endl;
    }
}

void TestStars ( string inputDir, string outputDir )
{
    SSObjectVec nearest, brightest;
    
    int numStars = SSImportObjectsFromCSV ( inputDir + "/Stars/Nearest.csv", nearest );
    cout << "Imported " << numStars << " nearby stars" << endl;
    
    numStars = SSImportObjectsFromCSV ( inputDir + "/Stars/Brightest.csv", brightest );
    cout << "Imported " << numStars << " bright stars" << endl;
    
    if ( ! outputDir.empty() )
    {
        numStars = SSExportObjectsToCSV ( outputDir + "/ExportedNearbyStars.csv", nearest );
        cout << "Exported " << numStars << " nearby stars to " << outputDir + "/ExportedNearbyStars.csv" << endl;

        numStars = SSExportObjectsToCSV ( outputDir + "/ExportedBrightStars.csv", brightest );
        cout << "Exported " << numStars << " bright stars to " << outputDir + "/ExportedBrightStars.csv" << endl;
    }
}

void TestDeepSky ( string inputDir, string outputDir )
{
    SSObjectVec messier, caldwell;
    
    int numObjs = SSImportObjectsFromCSV ( inputDir + "/DeepSky/Messier.csv", messier );
    cout << "Imported " << numObjs << " Messier objects" << endl;
    
    numObjs = SSImportObjectsFromCSV ( inputDir + "/DeepSky/Caldwell.csv", caldwell );
    cout << "Imported " << numObjs << " Caldwell objects" << endl;
    
    if ( ! outputDir.empty() )
    {
        numObjs = SSExportObjectsToCSV ( outputDir + "/ExportedMessier.csv", messier );
        cout << "Exported " << numObjs << " Messier objects to " << outputDir + "/ExportedMessier.csv" << endl;

        numObjs = SSExportObjectsToCSV ( outputDir + "/ExportedCaldwell.csv", caldwell );
        cout << "Exported " << numObjs << " Caldwell objects to " << outputDir + "/ExportedCaldwell.csv" << endl;
    }
}

void TestJPLDEphemeris ( string inputDir )
{
    SSJPLDEphemeris jpldeph;
    
    string ephemFile = inputDir + "/SolarSystem/DE438/1950_2050.438";
    if ( ! jpldeph.open ( ephemFile ) )
    {
        cout << "Failed to open " << ephemFile << endl;
        return;
    }
    
    cout << "Successfully opened " << ephemFile << endl;
    cout << "JED " << jpldeph.getStartJED() << " to " << jpldeph.getStopJED() << endl;
    
    // for ( int i = 0; i < jpldeph.getConstantNumber(); i++ )
    //    cout << jpldeph.getConstantName ( i ) << " = " << jpldeph.getConstantValue ( i ) << endl;
        
    double jed = SSTime ( SSDate ( kGregorian, 0.0, 2020, 1, 1.0, 0, 0, 0.0 ) ).getJulianEphemerisDate();
    SSVector pos, vel;
    
    for ( int id = 0; id <= 10; id++ )
    {
        jpldeph.compute ( id, jed, true, pos, vel );
        cout << format ( "obj %2d  ", id );
        cout << format ( "pos %+12.8f %+12.8f %+12.8f  ", pos.x, pos.y, pos.z );
        cout << format ( "vel %+11.8f %+11.8f %+11.8f", vel.x, vel.y, vel.z ) << endl;
    }
    
    jpldeph.close();
}

void TestSolarEphemeris ( string inputDir, string outputDir )
{
    SSObjectVec solsys;
    
    string ephemFile = inputDir + "/SolarSystem/DE438/1950_2050.438";
    if ( SSJPLDEphemeris::open ( ephemFile ) )
        cout << "Successfully opened JPL DE438 ephemeris file." << endl;
    else
        cout << "Failed to open JPL DE438 ephemeris file " << ephemFile << endl;

    SSImportObjectsFromCSV ( inputDir + "/SolarSystem/Planets.csv", solsys );
    SSImportObjectsFromCSV ( inputDir + "/SolarSystem/Moons.csv", solsys );
    cout << "Imported " << solsys.size() << " solar system objects." << endl;
    
    SSDate date ( kGregorian, 0.0, 2020, 4, 15.0, 0, 0, 0.0 );
    SSTime time = SSTime ( date ); // SSTime::fromSystem();
    SSSpherical here = { SSAngle ( SSDegMinSec ( '-', 122, 25, 09.9 ) ), SSAngle ( SSDegMinSec ( '+', 37, 46, 29.7 ) ), 0.026 };
    SSDynamics dyn ( time, here.lon, here.lat, here.rad );
    
    cout << format ( "Test Date: %04d/%02hd/%02.0f", date.year, date.month, floor ( date.day ) ) << endl;
    cout << format ( "Test Time: %02hd:%02hd:%04.1f", date.hour, date.min, date.sec ) << endl;
    cout << format ( "Test Longitude: %s", SSDegMinSec ( here.lon ).toString().c_str() ) << endl;
    cout << format ( "Test Latitude:  %s", SSDegMinSec ( here.lat ).toString().c_str() ) << endl;
    cout << format ( "Test Altitude:  %.0f m", here.rad * 1000.0 ) << endl << endl;
    
    for ( int i = 0; i < 11 && i < solsys.size(); i++ )
    {
        SSPlanet *p = SSGetPlanetPtr ( solsys[i] );
        if ( p == nullptr )
            continue;
        
        p->computeEphemeris ( dyn );
        SSSpherical dir ( p->getDirection() );
        dir = dyn.coords.toEquatorial ( dir );
        SSHourMinSec ra ( dir.lon );
        SSDegMinSec dec ( dir.lat );
        double dist = p->getDistance();
        float mag = p->getMagnitude();

        cout << p->getName ( 0 ) << ":" << endl;
        cout << "RA:   " << ra.toString() << endl;
        cout << "Dec:  " << dec.toString() << endl;
        if ( dist > 0.1 )
            cout << "Dist: " << format ( "%.6f AU", dist ) << endl;
        else
            cout << "Dist: " << format ( "%.0f km", dist * SSDynamics::kKmPerAU ) << endl;
        cout << "Mag:  " << format ( "%+.2f", mag ) << endl << endl;
    }

    SSJPLDEphemeris::close();
}

// Android redirects stdout & stderr output to /dev/null. This uses Android logging functions to send
// output to logcat. From https://stackoverflow.com/questions/8870174/is-stdcout-usable-in-android-ndk

#ifdef ANDROID

#include <android/log.h>

#define LOG_TAG "sstest"

class androidbuf : public std::streambuf
{
public:
    enum { bufsize = 128 }; // ... or some other suitable buffer size
    androidbuf() { this->setp(buffer, buffer + bufsize - 1); }

private:
    int overflow(int c)
    {
        if (c == traits_type::eof()) {
            *this->pptr() = traits_type::to_char_type(c);
            this->sbumpc();
        }
        return this->sync()? traits_type::eof(): traits_type::not_eof(c);
    }

    int sync()
    {
        int rc = 0;
        if (this->pbase() != this->pptr()) {
            char writebuf[bufsize+1];
            memcpy(writebuf, this->pbase(), this->pptr() - this->pbase());
            writebuf[this->pptr() - this->pbase()] = '\0';

            rc = __android_log_write(ANDROID_LOG_INFO, LOG_TAG, writebuf) > 0;
            this->setp(buffer, buffer + bufsize - 1);
        }
        return rc;
    }

    char buffer[bufsize];
};

#endif

#if TARGET_OS_IOS || defined ( ANDROID )
#include "SSTest.h"

#define main(argc,argv) _main(argc,argv)

int _main ( int argc, const char *argv[] );

int SSTestMain ( const char *inputpath, const char *outputpath )
{
    const char *argv[] = { "SSTest", inputpath, outputpath };

#ifdef ANDROID
    std::cout.rdbuf ( new androidbuf );
#endif

    return _main ( 3, argv );
}

#endif

int main ( int argc, const char *argv[] )
{
    TestTime();

    if ( argc < 3 )
    {
        cout << "Usage: SSTest <inpath> <outpath>" << endl;
        cout << "inpath: path to SSData directory" << endl;
        cout << "outpath: path to output directory" << endl;
        exit ( -1 );
    }
    
    string inpath ( argv[1] );
    string outpath ( argv[2] );
    
    TestSolarEphemeris ( inpath, outpath );
/*
    TestSatellites ( inpath, outpath );
    TestJPLDEphemeris ( inpath );
    TestSolarSystem ( inpath, outpath );
    TestConstellations ( inpath, outpath );
    TestStars ( inpath, outpath );
    TestDeepSky ( inpath, outpath );
    
    SSObjectVec comets;
    int numcom = importMPCComets ( "/Users/timmyd/Projects/SouthernStars/Catalogs/Comets/MPC/CometEls.txt", comets );
    cout << "Imported " << numcom << " MPC comets" << endl;

    SSObjectVec asteroids;
    int numast = importMPCAsteroids ( "/Users/timmyd/Projects/SouthernStars/Catalogs/Asteroids/MPCORB/MPCORB.DAT", asteroids );
    cout << "Imported " << numast << " MPC asteroids" << endl;

    SSIdentifierNameMap ngcicNameMap;
    SSImportIdentifierNameMap ( "/Users/timmyd/Projects/SouthernStars/Projects/SSCore/CSVData/DeepSky/Names.csv", ngcicNameMap );

    SSObjectVec clusters;
    int numClus = SSImportDAML02 ( "/Users/timmyd/Projects/SouthernStars/Catalogs/Open Clusters/Dias 2016/clusters.txt", ngcicNameMap, clusters );
    cout << "Imported " << numClus << " open clusters" << endl;
    exportCatalog ( clusters );

    SSObjectVec globulars;
    int numGlobs = SSImportMWGC ( "/Users/timmyd/Projects/SouthernStars/Catalogs/Globular Clusters/Harris 2010/mwgc.original.txt", ngcicNameMap, globulars );
    cout << "Imported " << numGlobs << " globular clusters" << endl;
    exportCatalog ( globulars );

    SSObjectVec planNebs;
    int numPlanNebs = SSImportPNG ( "/Users/timmyd/Projects/SouthernStars/Catalogs/Planetary Nebulae/Strasbourg-ESO 1992/main.dat",
                                   "/Users/timmyd/Projects/SouthernStars/Catalogs/Planetary Nebulae/Strasbourg-ESO 1992/dist.dat",
                                   "/Users/timmyd/Projects/SouthernStars/Catalogs/Planetary Nebulae/Strasbourg-ESO 1992/diam.dat",
                                   "/Users/timmyd/Projects/SouthernStars/Catalogs/Planetary Nebulae/Strasbourg-ESO 1992/vel.dat", ngcicNameMap, planNebs );
    cout << "Imported " << numPlanNebs << " planetary nebulae" << endl;
    exportCatalog ( planNebs );

    int numobj = SSImportNGCIC ( "/Users/timmyd/Projects/SouthernStars/Catalogs/Revised NGC-IC 2019/NI2019.txt", ngcicNameMap, clusters, globulars, planNebs, objects );
    cout << "Imported " << numobj << " NGC-IC objects" << endl;
    exportCatalog ( objects );

    exportCatalog ( objects, kCatMessier, 1, 110 );
    exportCatalog ( objects, kCatCaldwell, 1, 110 );

    SSIdentifierMap hipHRMap;
    int n = SSImportHIPHRIdentifiers ( "/Users/timmyd/Projects/SouthernStars/Catalogs/Hipparcos/TABLES/IDENT3.DOC", hipHRMap );
    cout << "Imported " << n << " Hipparcos HR identifiers." << endl;

    SSIdentifierMap hipBayMap;
    n = SSImportHIPBayerIdentifiers ( "/Users/timmyd/Projects/SouthernStars/Catalogs/Hipparcos/TABLES/IDENT4.DOC", hipBayMap );
    cout << "Imported " << n << " Hipparcos Bayer/Flamsteed identifiers." << endl;

    SSIdentifierMap hipGCVSMap;
    n = SSImportHIPGCVSIdentifiers ( "/Users/timmyd/Projects/SouthernStars/Catalogs/Hipparcos/TABLES/IDENT5.DOC", hipGCVSMap );
    cout << "Imported " << n << " Hipparcos GCVS identifiers." << endl;

    SSIdentifierNameMap hipNames;
    n = SSImportHIPNames ( "/Users/timmyd/Projects/SouthernStars/Catalogs/Hipparcos/TABLES/IDENT6.DOC", hipNames );
    cout << "Imported " << n << " Hipparcos star names." << endl;
    
    SSObjectVec hicStars;
    n = SSImportHIC ( "/Users/timmyd/Projects/SouthernStars/Catalogs/Hipparcos Input Catalog/main.dat", hicStars );
    cout << "Imported " << n << " Hipparcos Input Catalog stars." << endl;

    SSObjectVec hip2Stars;
//  n = SSImportHIP2 ( "/Users/timmyd/Projects/SouthernStars/Catalogs/Hipparcos New Reduction 2007/hip2.dat", hip2Stars );
//  cout << "Imported " << n << " Hipparcos New Reduction stars." << endl;

    SSObjectVec hipStars;
    n = SSImportHIP ( "/Users/timmyd/Projects/SouthernStars/Catalogs/Hipparcos/CATS/HIP_MAIN.DAT", hipHRMap, hipBayMap, hipGCVSMap, hipNames, hicStars, hip2Stars, hipStars );
    cout << "Imported " << n << " Hipparcos stars." << endl;

    SSIdentifierNameMap iauNames;
    n = SSImportIAUStarNames ( "/Users/timmyd/Projects/SouthernStars/Projects/Star Names/IAU-CSN.txt", iauNames );
    cout << "Imported " << n << " IAU star names." << endl;

    SSIdentifierNameMap starNames;
    n = SSImportIdentifierNameMap ( "/Users/timmyd/Projects/SouthernStars/Projects/SSCore/CSVData/Stars/Names.csv", starNames );
    cout << "Imported " << n << " star names." << endl;

    SSObjectVec gjACStars;
    n = SSImportGJAC ( "/Users/timmyd/Projects/SouthernStars/Catalogs/Nearby Stars/Accurate Coordinates/table1.dat", hipStars, gjACStars );
    cout << "Imported " << n << " GJ accurate coordinate nearby stars." << endl;
//    exportCatalog ( gjACStars );

    SSObjectVec gjStars;
    n = SSImportGJCNS3 ( "/Users/timmyd/Projects/SouthernStars/Catalogs/Nearby Stars/CNS3/catalog.dat", starNames, gjACStars, gjStars );
    cout << "Imported " << n << " GJ nearby stars." << endl;
//    exportCatalog ( gjStars );

    SSObjectVec skyStars;
    n = SSImportSKY2000 ( "/Users/timmyd/Projects/SouthernStars/Catalogs/SKY2000 Master Star Catalog/ATT_sky2kv5.cat", iauNames, hipStars, gjStars, skyStars );
    cout << "Imported " << n << " SKY2000 stars." << endl;
    exportCatalog ( skyStars, kCatHR, 1, 9110 );

    SSAngle zero = 0.0;
    SSAngle one ( 1.0 );
    SSAngle two ( 2.0 );
    
    SSAngle sum = zero + one - 1.5;
    sum /= two;
    
    printf ( "%f\n", (double) sum );
            
    SSTime now = SSTime::fromSystem();
    SSSpherical here = { SSAngle ( SSDegMinSec ( '-', 122, 25, 55.3 ) ), SSAngle ( SSDegMinSec ( '+', 37, 46, 09.7 ) ) };
    SSDynamics dyn ( now.jd, here.lon, here.lat );
    
//    SSTime now ( SSDate ( kSSGregorian, -5.0, 1971, 12, 28, 11, 44, 0.0 ) );
    SSDate date = ( now );
    
    printf ( "Julian Date: %f\n", now.jd );
    printf ( "Time Zone: %.1f\n", now.zone );
    printf ( "Local Date: %04d-%02hd-%02.0f\n", date.year, date.month, floor ( date.day ) );
    printf ( "Local Time: %02d:%02d:%04.1f\n", date.hour, date.min, date.sec );

    SSSpherical siriusFun = { SSAngle ( SSHourMinSec ( '+', 06, 45, 08.92 ) ), SSAngle ( SSDegMinSec  ( '-', 16, 42, 58.0 ) ) };
    siriusFun = dyn.addAberration ( siriusFun );
    
    SSSpherical siriusEqu = dyn.coords.toEquatorial ( siriusFun );
    SSSpherical siriusEcl = dyn.coords.toEcliptic ( siriusFun );
    SSSpherical siriusGal = dyn.coords.toGalactic ( siriusFun );
    SSSpherical siriusHor = dyn.coords.toHorizon ( siriusFun );

    SSHourMinSec ra ( siriusFun.lon );
    SSDegMinSec dec ( siriusFun.lat );
    
    printf ( "Fundamental RA  = %02hd %02hd %05.2f\n", ra.hour, ra.min, ra.sec );
    printf ( "Fundamental Dec = %c%02hd %02hd %04.1f\n", dec.sign, dec.deg, dec.min, dec.sec );

    ra = SSHourMinSec ( siriusEqu.lon );
    dec = SSDegMinSec ( siriusEqu.lat );
    
    printf ( "Equatorial RA  = %02hd %02hd %05.2f\n", ra.hour, ra.min, ra.sec );
    printf ( "Equatorial Dec = %c%02hd %02hd %04.1f\n", dec.sign, dec.deg, dec.min, dec.sec );

    SSDegMinSec eclon ( siriusEcl.lon );
    SSDegMinSec eclat ( siriusEcl.lat );

    printf ( "Ecliptic Lon = %03hd %02hd %04.1f\n", eclon.deg, eclon.min, eclon.sec );
    printf ( "Ecliptic Lat = %c%02hd %02hd %04.1f\n", eclat.sign, eclat.deg, eclat.min, eclat.sec );

    SSDegMinSec galon ( siriusGal.lon );
    SSDegMinSec galat ( siriusGal.lat );

    printf ( "Galactic Lon = %03hd %02hd %04.1f\n", galon.deg, galon.min, galon.sec );
    printf ( "Galactic Lat = %c%02hd %02hd %04.1f\n", galat.sign, galat.deg, galat.min, galat.sec );

    SSDegMinSec azm ( siriusHor.lon );
    SSDegMinSec alt ( siriusHor.lat );

    printf ( "Azimuth  = %03hd %02hd %04.1f\n", azm.deg, azm.min,azm.sec );
    printf ( "Altitude = %c%02hd %02hd %04.1f\n", alt.sign, alt.deg, alt.min, alt.sec );

    // Print J2000 RA/Dec of north galactic pole
    
    SSSpherical galCen = dyn.coords.fromGalactic ( SSSpherical ( 0.0, 0.0 ) );
    ra = SSHourMinSec ( galCen.lon );
    dec = SSDegMinSec ( galCen.lat );
    
    printf ( "Gal Cen RA  = %02hd %02hd %05.2f\n", ra.hour, ra.min, ra.sec );
    printf ( "Gal Cen Dec = %c%02hd %02hd %04.1f\n", dec.sign, dec.deg, dec.min, dec.sec );

    // Print J2000 RA/Dec of galactic center
    
    SSSpherical ngp = dyn.coords.fromGalactic ( SSSpherical ( 0.0, SSAngle::fromDegrees ( 90.0 ) ) );
    ra = SSHourMinSec ( ngp.lon );
    dec = SSDegMinSec ( ngp.lat );
    printf ( "NGP RA  = %02hd %02hd %05.2f\n", ra.hour, ra.min, ra.sec );
    printf ( "NGP Dec = %c%02hd %02hd %04.1f\n", dec.sign, dec.deg, dec.min, dec.sec );

    double jde = now.getJulianEphemerisDate();
    SSOrbit orb = SSOrbit::getEarthOrbit ( jde );
    SSMatrix orbMat = SSCoords::getEclipticMatrix ( SSCoords::getObliquity ( SSTime::kJ2000 ) );
    
    SSVector pos, vel;
    orb.toPositionVelocity ( jde, pos, vel );
    
    pos = pos.multiplyBy ( -1.0 );
    pos = orbMat.multiply ( pos );
    vel = orbMat.multiply ( vel );
    
    pos = dyn.coords.toEquatorial ( pos );
    SSSpherical equ ( pos );
    ra = SSHourMinSec ( equ.lon );
    dec = SSDegMinSec ( equ.lat );

    printf ( "Sun RA   = %02hd %02hd %05.2f\n", ra.hour, ra.min, ra.sec );
    printf ( "Sun Dec  = %c%02hd %02hd %04.1f\n", dec.sign, dec.deg, dec.min, dec.sec );
    printf ( "Sun Dist = %f AU\n", equ.rad );

    SSPlanetID planetIDs[10] = { kSun, kMercury, kVenus, kEarth, kMars, kJupiter, kSaturn, kUranus, kNeptune, kPluto };
    
    for ( SSPlanetID id : planetIDs )
    {
        SSPlanet planet ( kTypePlanet, id );
        planet.computeEphemeris ( dyn );
        
        SSSpherical equ ( dyn.coords.toEquatorial ( planet.getDirection() ) );
        ra = SSHourMinSec ( equ.lon );
        dec = SSDegMinSec ( equ.lat );

        printf ( "%d RA   = %02hd %02hd %05.2f\n", id, ra.hour, ra.min, ra.sec );
        printf ( "%d Dec  = %c%02hd %02hd %04.1f\n", id, dec.sign, dec.deg, dec.min, dec.sec );
        printf ( "%d Dist = %f AU\n", id, planet.getDistance() );
    }
    
    dyn.getMoonPositionVelocity ( kLuna, dyn.jde, pos, vel );
    pos = pos.subtract ( dyn.obsPos );
    equ = SSSpherical ( dyn.coords.toEquatorial ( pos ) );
    ra = SSHourMinSec ( equ.lon );
    dec = SSDegMinSec ( equ.lat );
    
    printf ( "Moon RA   = %02hd %02hd %05.2f\n", ra.hour, ra.min, ra.sec );
    printf ( "Moon Dec  = %c%02hd %02hd %04.1f\n", dec.sign, dec.deg, dec.min, dec.sec );
    printf ( "Moon Dist = %f ER\n", equ.rad * SSDynamics::kKmPerAU );

    SSVector v1 ( 1.0, 2.0, 3.0 );
    SSVector v2 ( 4.0, 5.0, 6.0 );
    SSVector v3;
    
    v1.add ( v2 );
    
    
    v3 = v1.add ( v2 );
    
    printf ( "%lf %lf %lf\n", v1.x, v1.y, v1.z );
    printf ( "%lf %lf %lf\n", v2.x, v2.y, v2.z );
    printf ( "%lf %lf %lf\n", v3.x, v3.y, v3.z );
    
    v1 = v1.subtract ( v2 );
    printf ( "%lf %lf %lf\n", v1.x, v1.y, v1.z );
    
    SSMatrix i = SSMatrix::identity();
    
    printf ( "%lf %lf %lf\n", i.m00, i.m01, i.m02 );
    printf ( "%lf %lf %lf\n", i.m10, i.m11, i.m12 );
    printf ( "%lf %lf %lf\n", i.m20, i.m21, i.m22 );
*/
    return 0;
}

