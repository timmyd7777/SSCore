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

#ifdef _MSC_VER
#include <windows.h>
#endif

#include "SSCoordinates.hpp"
#include "SSOrbit.hpp"
#include "SSPlanet.hpp"
#include "SSFeature.hpp"
#include "SSStar.hpp"
#include "SSConstellation.hpp"
#include "SSImportGCVS.hpp"
#include "SSImportHIP.hpp"
#include "SSImportSKY2000.hpp"
#include "SSImportNGCIC.hpp"
#include "SSImportMPC.hpp"
#include "SSImportTLE.hpp"
#include "SSImportGJ.hpp"
#include "SSImportWDS.hpp"
#include "SSJPLDEphemeris.hpp"
#include "SSTLE.hpp"
#include "SSEvent.hpp"
#include "VSOP2013.hpp"
#include "ELPMPP02.hpp"

void exportCatalog ( SSObjectVec &objects )
{
    for ( int i = 0; i < objects.size(); i++ )
    {
        SSObjectPtr pObj = objects[i];
        if ( pObj )
            cout << pObj->toCSV() << endl;
    }
}

void exportCatalog ( SSObjectVec &objects, SSCatalog cat, int first, int last )
{
    SSObjectMap map = SSMakeObjectMap ( objects, cat );

    for ( int n = first; n <= last; n++ )
    {
        int i = map[ SSIdentifier ( cat, n ) ];
        if ( i > 0 )
        {
            SSObjectPtr pObj = objects[ i - 1 ];
            cout << pObj->toCSV() << endl;
        }
    }
}

void TestTime ( void )
{
    SSTime now = SSTime::fromSystem();
    SSDate date = now;

    cout << "Current local date is " << formstr ( "%04hd-%02hd-%0hd", date.year, date.month, date.day ) << endl;
    cout << "Current local time is " << formstr ( "%02hd:%02hd:%04.1f", date.hour, date.min, date.sec ) << endl;
    cout << "Current local time is " << formstr ( "%+.2f", date.zone ) << " hours east of UTC" << endl;
    cout << "Current Julian Date is " << formstr ( "%.6f", now.jd ) << endl;

    cout << "Current working directory is " << getcwd() << endl << endl;

    cout << "Before is "
         << formstr("%04hd-%02hd-%02.0f %02hd:%02hd:%04.1f",
              date.year,
              date.month,
              floor(date.day),
              date.hour,
              date.min,
              date.sec)
         << endl;
    SSTime time2 = date;
    SSDate date2 = time2;
    cout << "After is "
         << formstr("%04hd-%02hd-%02.0f %02hd:%02hd:%04.1f",
              date2.year,
              date2.month,
              floor(date2.day),
              date2.hour,
              date2.min,
              date2.sec)
         << endl;
};

void TestCalendars ( void )
{
    double step = 36522;
    double start = SSTime::kJ2000 + 0.5 - step * 10;
    double stop = SSTime::kJ2000 + 0.5 + step * 10;
    
    int year;
    short month;
    double day;
    
    for ( double jd0 = start; jd0 <= stop; jd0 += step )
    {
        SSTime::JDToCalendar ( jd0, year, month, day );
        printf ( "JD %10.1f = %5d/%02hd/%04.1f", jd0, year, month, day );
        SSTime::JDToGregorian ( jd0, year, month, day );
        printf ( " = Gregorian %5d/%02hd/%04.1f", year, month, day );
        double jd1 = SSTime::GregorianToJD ( year, month, day );
        printf ( " = JD %10.1f\n", jd1 );
    }

    for ( double jd0 = start; jd0 <= stop; jd0 += step )
    {
        SSTime::JDToCalendar ( jd0, year, month, day );
        printf ( "JD %10.1f = %5d/%02hd/%04.1f", jd0, year, month, day );
        SSTime::JDToJulian ( jd0, year, month, day );
        printf ( " = Julian %5d/%02hd/%04.1f", year, month, day );
        double jd1 = SSTime::JulianToJD ( year, month, day );
        printf ( " = JD %10.1f\n", jd1 );
    }

    for ( double jd0 = start; jd0 <= stop; jd0 += step )
    {
        SSTime::JDToCalendar ( jd0, year, month, day );
        printf ( "JD %10.1f = %5d/%02hd/%04.1f", jd0, year, month, day );
        SSTime::JDToJewish ( jd0, year, month, day );
        printf ( " = Jewish %5d/%02hd/%04.1f", year, month, day );
        double jd1 = SSTime::JewishToJD ( year, month, day );
        printf ( " = JD %10.1f\n", jd1 );
    }

    for ( double jd0 = start; jd0 <= stop; jd0 += step )
    {
        SSTime::JDToCalendar ( jd0, year, month, day );
        printf ( "JD %10.1f = %5d/%02hd/%04.1f", jd0, year, month, day );
        SSTime::JDToIslamic ( jd0, year, month, day );
        printf ( " = Moslem %5d/%02hd/%04.1f", year, month, day );
        double jd1 = SSTime::IslamicToJD ( year, month, day );
        printf ( " = JD %10.1f\n", jd1 );
    }

    for ( double jd0 = start; jd0 <= stop; jd0 += step )
    {
        SSTime::JDToCalendar ( jd0, year, month, day );
        printf ( "JD %10.1f = %5d/%02hd/%04.1f", jd0, year, month, day );
        SSTime::JDToIndian ( jd0, year, month, day );
        printf ( " = Indian %5d/%02hd/%04.1f", year, month, day );
        double jd1 = SSTime::IndianToJD ( year, month, day );
        printf ( " = JD %10.1f\n", jd1 );
    }
}

void TestSatellites ( string inputDir, string outputDir )
{
    string filename = inputDir + "/SolarSystem/Satellites/visual.txt";
    FILE *file = fopen ( filename.c_str(), "r" );
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

        cout << formstr ( "JD %.6f  ", tle.jdepoch + 10.0 );
        cout << formstr ( "pos %+10.3f %+10.3f %+10.3f  ", pos.x, pos.y, pos.z );
        cout << formstr ( "vel %+7.3f %+7.3f %+7.3f",   vel.x, vel.y, vel.z ) << endl;

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
    numPlanets = SSExportObjectsToCSV ( "", moons );

    SSObjectVec features;
    int numFeatures = SSImportObjectsFromCSV ( inputDir + "/SolarSystem/Features.csv", features );
    cout << "Imported " << numFeatures << " non-Earth planetary surface features" << endl;

    SSPlanetFeatureMap featureMap;
    int numFeaturePlanets = SSMakePlanetFeatureMap ( features, featureMap );
    cout << "Indexed " << numFeaturePlanets << " planets with surface features" << endl;

    SSObjectVec cities;
    int numCities = SSImportObjectsFromCSV ( inputDir + "/SolarSystem/Cities.csv", cities );
    cout << "Imported " << numCities << " cities" << endl;

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

        numFeatures = SSExportObjectsToCSV ( outputDir + "/ExportedFeatures.csv", features );
        cout << "Exported " << numFeatures << " non-Earth planetary surface features to " << outputDir + "/ExportedFeatures.csv" << endl;

        numCities = SSExportObjectsToCSV ( outputDir + "/ExportedCities.csv", cities );
        cout << "Exported " << numCities << " cities to " << outputDir + "/ExportedCities.csv" << endl;

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
        cout << formstr ( "obj %2d  ", id );
        cout << formstr ( "pos %+12.8f %+12.8f %+12.8f  ", pos.x, pos.y, pos.z );
        cout << formstr ( "vel %+11.8f %+11.8f %+11.8f", vel.x, vel.y, vel.z ) << endl;
    }
    
    jpldeph.close();
}

void TestEvents ( SSCoordinates coords, SSObjectVec &solsys )
{
    SSTime now = coords.getTime();
    
    // Compute Sun/Moon rise/transit/set circumstances

    if ( solsys.size() > 10 )
    {
        SSObjectPtr pSun = solsys[0];
        SSObjectPtr pMoon = solsys[10];

        SSPass sunpass = SSEvent::riseTransitSet ( now, coords, pSun, SSEvent::kSunMoonRiseSetAlt );
        SSPass moonpass = SSEvent::riseTransitSet ( now, coords, pMoon, SSEvent::kSunMoonRiseSetAlt );
        
        SSDate date = SSDate ( sunpass.rising.time );
        if ( isinf ( sunpass.rising.time ) )
            cout << "Sunrise:  none" << endl;
        else
            cout << "Sunrise:  " << date.format ( "%H:%M:%S" ) << formstr ( " @ %.1f°", sunpass.rising.azm * SSAngle::kDegPerRad ) << endl;

        date = SSDate ( sunpass.setting.time );
        if ( isinf ( sunpass.setting.time ) )
            cout << "Sunset:   none" << endl;
        else
            cout << "Sunset:   " << date.format ( "%H:%M:%S" ) << formstr ( " @ %.1f°", sunpass.setting.azm * SSAngle::kDegPerRad ) << endl;

        date = SSDate ( moonpass.rising.time );
        if ( isinf ( moonpass.rising.time ) )
            cout << "Moonrise: none" << endl;
        else
            cout << "Moonrise: " << date.format ( "%H:%M:%S" ) << formstr ( " @ %.1f°", moonpass.rising.azm * SSAngle::kDegPerRad ) << endl;

        date = SSDate ( moonpass.setting.time );
        if ( isinf ( moonpass.setting.time ) )
            cout << "Moonset:  none" << endl << endl;
        else
            cout << "Moonset:  " << date.format ( "%H:%M:%S" ) << formstr ( " @ %.1f°", moonpass.setting.azm * SSAngle::kDegPerRad ) << endl << endl;

        SSTime time = SSEvent::nextMoonPhase ( now, pSun, pMoon, SSEvent::kNewMoon );
        date = SSDate ( time );
        cout << "New Moon:       " << date.format ( "%Y/%m/%d %H:%M:%S" ) << endl;

        time = SSEvent::nextMoonPhase ( now, pSun, pMoon, SSEvent::kFirstQuarterMoon );
        date = SSDate ( time );
        cout << "First Quarter:  " << date.format ( "%Y/%m/%d %H:%M:%S" ) << endl;

        time = SSEvent::nextMoonPhase ( now, pSun, pMoon, SSEvent::kFullMoon );
        date = SSDate ( time );
        cout << "Full Moon:      " << date.format ( "%Y/%m/%d %H:%M:%S" ) << endl;

        time = SSEvent::nextMoonPhase ( now, pSun, pMoon, SSEvent::kLastQuarterMoon );
        date = SSDate ( time );
        cout << "Last Quarter:   " << date.format ( "%Y/%m/%d %H:%M:%S" ) << endl << endl;
        
        // Find Jupiter-Saturn conjunctions in the next year
        
        SSObjectPtr pJup = solsys[5];
        SSObjectPtr pSat = solsys[6];
        vector<SSEventTime> conjunctions;

        SSEvent::findConjunctions ( coords, pJup, pSat, now, now + 365.25, conjunctions, 10 );
        cout << conjunctions.size() << " Jupiter-Saturn Conjunctions in the next year:" << endl;
        for ( int i = 0; i < conjunctions.size(); i++ )
        {
            date = SSDate ( conjunctions[i].time );
            SSDegMinSec sep = SSAngle ( conjunctions[i].value );
            cout << sep.format ( "%2hd° %2hd' %4.1f\"" ) << " on " << date.format ( "%Y/%m/%d %H:%M:%S" ) << endl;
        }
        cout << endl;
    }

    // Find the ISS
    
    int i = 0;
    for ( i = 0; i < solsys.size(); i++ )
    {
        SSPlanet *p = SSGetPlanetPtr ( solsys[i] );
        if ( p->getType() == kTypeSatellite )
            if ( p->getIdentifier() == SSIdentifier ( kCatNORADSat, 25544 ) )
                break;
    }
    
    // If we found it, find and print all ISS passes in the next day.
    
    coords.setTime ( now );
    if ( i < solsys.size() )
    {
        vector<SSPass> passes;
        
        int numpasses = SSEvent::findSatellitePasses ( coords, solsys[i], now, now + 1.0, 0.0, passes, 10 );
        cout << numpasses << " ISS passes in the next day:" << endl;
        for ( i = 0; i < numpasses; i++ )
        {
            SSDate date = SSDate ( passes[i].rising.time );
            cout << formstr ( "Rise:  %02hd:%02hd:%02.0f @ %.1f°", date.hour, date.min, date.sec, passes[i].rising.azm * SSAngle::kDegPerRad ) << endl;

            date = SSDate ( passes[i].transit.time );
            cout << formstr ( "Peak:  %02hd:%02hd:%02.0f @ %.1f°", date.hour, date.min, date.sec, passes[i].transit.alt * SSAngle::kDegPerRad ) << endl;

            date = SSDate ( passes[i].setting.time );
            cout << formstr ( "Set:   %02hd:%02hd:%02.0f @ %.1f°", date.hour, date.min, date.sec, passes[i].setting.azm * SSAngle::kDegPerRad ) << endl << endl;
        }
    }
}

void TestEphemeris ( string inputDir, string outputDir )
{
    SSObjectVec solsys;
    
    SSPlanet::useVSOPELP ( false );
    string ephemFile = inputDir + "/SolarSystem/DE438/1950_2050.438";
    if ( SSJPLDEphemeris::open ( ephemFile ) )
        cout << "Successfully opened JPL DE438 ephemeris file." << endl;
    else
        cout << "Failed to open JPL DE438 ephemeris file " << ephemFile << endl;

    SSImportObjectsFromCSV ( inputDir + "/SolarSystem/Planets.csv", solsys );
    SSImportObjectsFromCSV ( inputDir + "/SolarSystem/Moons.csv", solsys );
    cout << "Imported " << solsys.size() << " solar system objects." << endl;
    
    int nsat = SSImportSatellitesFromTLE ( inputDir + "/SolarSystem/Satellites/visual.txt", solsys );
    cout << "Imported " << nsat << " artificial satellites." << endl;

    int nnames = SSImportMcNames ( inputDir + "/SolarSystem/Satellites/mcnames.txt", solsys );
    cout << "Imported " << nnames << " McCants satellite names." << endl;
    
    SatFreqMap satfreqs;
    int nfreqs = SSImportSatelliteFrequencyData ( inputDir + "/SolarSystem/Satellites/je9pel.csv", satfreqs );
    cout << "Imported " << nfreqs << " JE9PEL satellite radio frequencies." << endl << endl;

    SSDate date ( kGregorian, 0.0, 2020, 4, 15.0, 0, 0, 0.0 );
    SSTime now = SSTime::fromSystem(); // SSTime ( date );
    date = SSDate ( now );
    SSSpherical here = { SSAngle ( SSDegMinSec ( '-', 122, 25, 09.9 ) ), SSAngle ( SSDegMinSec ( '+', 37, 46, 29.7 ) ), 0.026 };
    SSCoordinates coords ( now, here );
    
    cout << formstr ( "Test Date: %04d/%02hd/%02hd", date.year, date.month, date.day ) << endl;
    cout << formstr ( "Test Time: %02hd:%02hd:%04.1f", date.hour, date.min, date.sec ) << endl;
    cout << formstr ( "Test Zone: %+.1f hours", date.zone ) << endl;
    cout << formstr ( "Test Longitude: %s", SSDegMinSec ( here.lon ).toString().c_str() ) << endl;
    cout << formstr ( "Test Latitude:  %s", SSDegMinSec ( here.lat ).toString().c_str() ) << endl;
    cout << formstr ( "Test Altitude:  %.0f m", here.rad * 1000.0 ) << endl << endl;

    coords.setAberration ( true );
    coords.setLightTime ( true );

    TestEvents ( coords, solsys );
    
    // Compute and print ephemeris of solar system objects.
    
    coords.setTime ( now );
    for ( int i = 0; i < solsys.size(); i++ )
    {
        SSPlanet *p = SSGetPlanetPtr ( solsys[i] );
        if ( p == nullptr )
            continue;
        
        // Skip everything except the first 34 solar system objects and the ISS.
        
        bool skip = i < 34 ? false : true;
        if ( p->getType() == kTypeSatellite )
            if ( p->getIdentifier() == SSIdentifier ( kCatNORADSat, 25544 ) )
                skip = false;

        if ( skip )
            continue;
        
        p->computeEphemeris ( coords );
        string con = SSConstellation::identify ( p->getDirection() );
        SSSpherical dir ( p->getDirection() );
        dir = coords.transform ( kFundamental, kEquatorial, dir );
        SSHourMinSec ra ( dir.lon );
        SSDegMinSec dec ( dir.lat );
        double dist = p->getDistance();
        float mag = p->getMagnitude();

        cout << p->getName ( 0 ) << ":" << endl;
        cout << "RA:   " << ra.format ( " %02hdh %02hdm %05.2fs" ) << endl;
        cout << "Dec:  " << dec.format ( "%c%02hd° %02hd' %04.1f\"" ) << endl;
        cout << "Con:  " << con << endl;
        if ( dist > 0.1 )
            cout << "Dist: " << formstr ( "%.6f AU", dist ) << endl;
        else
            cout << "Dist: " << formstr ( "%.0f km", dist * coords.kKmPerAU ) << endl;
        cout << "Mag:  " << formstr ( "%+.2f", mag ) << endl;
        cout << "Illum: " << formstr ( "%.1f%%", p->illumination() * 100.0 ) << endl;
        
        // For moons, print RA (X) and Dec (Y) offset in arcsec from primary planet.
        
        if ( p->getType() == kTypeMoon )
        {
            SSPlanet *primary = SSGetPlanetPtr ( solsys[ p->getIdentifier().identifier() / 100 ] );
            SSSpherical primaryDir ( primary->getDirection() );
            primaryDir = coords.transform ( kFundamental, kEquatorial, primaryDir );
            double x = modpi ( dir.lon - primaryDir.lon ) * SSAngle::kArcsecPerRad * cos ( primaryDir.lat );
            double y = modpi ( dir.lat - primaryDir.lat ) * SSAngle::kArcsecPerRad;
            cout << formstr ( "X: %+.1f\" Y: %+.1f\"", x, y ) << endl;
        }
        
        cout << endl;
    }

    SSJPLDEphemeris::close();

    // Compute and print ephemeris information for the 10 nearest stars
    
    SSObjectVec nearStars;
    int numStars = SSImportObjectsFromCSV ( inputDir + "/Stars/Nearest.csv", nearStars );
    cout << "Imported " << numStars << " nearby stars" << endl;

    coords.setStarParallax ( true );
    coords.setStarMotion ( true );
    coords.setAberration ( true );
    
    for ( int i = 0; i < nearStars.size(); i++ )
    {
        SSStar *pStar = SSGetStarPtr ( nearStars[i] );
        if ( pStar == nullptr )
            continue;
        
        // Stop after the first 11 stars.
        
         if ( i > 10 )
            break;
        
        pStar->computeEphemeris ( coords );
        string con = SSConstellation::identify ( pStar->getDirection() );
        SSSpherical dir ( pStar->getDirection() );
        dir = coords.transform ( kFundamental, kEquatorial, dir );
        SSHourMinSec ra ( dir.lon );
        SSDegMinSec dec ( dir.lat );
        double dist = pStar->getDistance() / coords.kAUPerParsec;
        float mag = pStar->getMagnitude();

        string name = pStar->getName ( 0 );
        if ( name.empty() )
            name = pStar->getIdentifiers()[0].toString();
            
        cout << name << ":" << endl;
        cout << "RA:   " << ra.toString() << endl;
        cout << "Dec:  " << dec.toString() << endl;
        cout << "Con:  " << con << endl;
        cout << "Dist: " << formstr ( "%.3f pc", dist ) << endl;
        cout << "Mag:  " << formstr ( "%+.2f", mag ) << endl << endl;
    }
}

void TestELPMPP02 ( const string &datadir )
{
    ELPMPP02 elp;

#if ! ELPMPP02_EMBED_SERIES
    elp.readSeries ( datadir );
    cout << "Testing ELPMPP02 using series read from " << datadir << endl;
#else
    cout << "Testing ELPMPP02 using embedded series." << endl;
#endif

    double testjd[5] = { 2444239.5, 2446239.5, 2448239.5, 2450239.5, 2452239.5 };
    
    for ( int i = 0; i < 5; i++ )
    {
        SSVector pos, vel;
        double jed = testjd[i];
        cout << formstr ( "JD %.1f", jed ) << endl;
        elp.computePositionVelocity ( jed, pos, vel );
        pos *= SSCoordinates::kKmPerAU;
        vel *= SSCoordinates::kKmPerAU;
        cout << formstr ( "moon pos: %+13.5f  %+13.5f  %+13.5f km  ", pos.x, pos.y, pos.z );
        cout << formstr ( "vel: %+13.5f  %+13.5f  %+13.5f km/day", vel.x, vel.y, vel.z ) << endl;
    }
    
    cout << endl;
}

void TestVSOP2013 ( const string &datadir )
{
    VSOP2013 vsop2013;

#if ! VSOP2013_EMBED_SERIES
    vsop2013.readFile ( datadir + "VSOP2013p1.dat", kMercury );
    vsop2013.readFile ( datadir + "VSOP2013p2.dat", kVenus );
    vsop2013.readFile ( datadir + "VSOP2013p3.dat", kEarth );
    vsop2013.readFile ( datadir + "VSOP2013p4.dat", kMars );
    vsop2013.readFile ( datadir + "VSOP2013p5.dat", kJupiter );
    vsop2013.readFile ( datadir + "VSOP2013p6.dat", kSaturn );
    vsop2013.readFile ( datadir + "VSOP2013p7.dat", kUranus );
    vsop2013.readFile ( datadir + "VSOP2013p8.dat", kNeptune );
    vsop2013.readFile ( datadir + "VSOP2013p9.dat", kPluto );
    cout << "Testing VSOP2013 using series read from " << datadir << endl;
#else
    cout << "Testing VSOP2013 using embedded series." << endl;
#endif
    
    // These Julian Ephemeris Dates match the test dates in the test cases provided with VSOP2013.
    
    for ( double jed = 2411545.0; jed <= 2491545.0; jed += 40000.0 )
    {
        SSVector pos, vel;
        
        cout << formstr ( "JD %.1f\n", jed );
        for ( int iplanet = 1; iplanet <= 9; iplanet++ )
        {
            vsop2013.computePositionVelocity ( iplanet, jed, pos, vel );
            cout << formstr ( "planet %d pos: %+14.10f  %+14.10f  %+14.10f AU  ", iplanet, pos.x, pos.y, pos.z );
            cout << formstr ( "vel: %+13.10f  %+13.10f  %+13.10f AU/day", vel.x, vel.y, vel.z ) << endl;
        }
    }
    
    cout << endl;
}

void TestPrecession ( void )
{
    SSMatrix p = SSCoordinates::getPrecessionMatrix ( 1219339.078000 );
    
    printf ( "%+.12f %+.12f %+.12f\n", p.m00, p.m01, p.m02 );
    printf ( "%+.12f %+.12f %+.12f\n", p.m10, p.m11, p.m12 );
    printf ( "%+.12f %+.12f %+.12f\n", p.m20, p.m21, p.m22 );
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

void ExportObjectsToHTM ( const string htmdir, SSObjectVec &objects, bool ngcic );

int main ( int argc, const char *argv[] )
{
// This bit of magic makes UTF-8 output with degree characters appear correctly on the Windows console;
// see https://www.codeproject.com/Articles/34068/Unicode-Output-to-the-Windows-Console

#ifdef _MSC_VER
    UINT oldcp = GetConsoleOutputCP();
    SetConsoleOutputCP ( CP_UTF8 );
#endif
    
    if ( argc < 3 )
    {
        cout << "Usage: SSTest <inpath> <outpath>" << endl;
        cout << "inpath: path to SSData directory" << endl;
        cout << "outpath: path to output directory" << endl;
        exit ( -1 );
    }
    
    string inpath ( argv[1] );
    string outpath ( argv[2] );
    
    TestTime();
    TestCalendars();
    TestELPMPP02 ( "/Users/timmyd/Projects/SouthernStars/Projects/Astro Code/ELPMPP02/Chapront/" );
    TestVSOP2013 ( "/Users/timmyd/Projects/SouthernStars/Projects/Astro Code/VSOP2013/solution/" );
    TestEphemeris ( inpath, outpath );
    TestPrecession();
    TestSatellites ( inpath, outpath );
    TestJPLDEphemeris ( inpath );
    TestSolarSystem ( inpath, outpath );
    TestConstellations ( inpath, outpath );
    TestStars ( inpath, outpath );
    TestDeepSky ( inpath, outpath );

#ifdef _MSC_VER
    SetConsoleOutputCP ( oldcp );
#endif

    return 0;
}
