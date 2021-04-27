// SSImportWDS.cpp
// SSCore
//
// Created by Tim DeBenedictis on 4/26/21.
// Copyright © 2021 Southern Stars Group, LLC. All rights reserved.

#include <algorithm>
#include <iostream>
#include <fstream>
#include <map>

#include "SSImportWDS.hpp"

int SSImportORB6  ( const string &filename, SSObjectArray &stars )
{
    // Open file; return on failure.

    ifstream file ( filename );
    if ( ! file )
        return 0;

    // Read file line-by-line until we reach end-of-file

    string line = "";
    int numStars = 0;
    
    while ( getline ( file, line ) )
    {
        if ( line.length() < 214 )
            continue;

        string strRAh = trim ( line.substr ( 0, 2 ) );
        string strRAm = trim ( line.substr ( 2, 2 ) );
        string strRAs = trim ( line.substr ( 4, 5 ) );
        string strDecd = trim ( line.substr ( 9, 3 ) );
        string strDecm = trim ( line.substr ( 12, 2 ) );
        string strDecs = trim ( line.substr ( 14, 4 ) );
        string strMag1 = trim ( line.substr ( 66, 5 ) );
        string strMag2 = trim ( line.substr ( 72, 5 ) );
        string strPeriod = trim ( line.substr ( 80, 12 ) );
        string strSMAxis = trim ( line.substr ( 105, 9 ) );
        string strInclin = trim ( line.substr ( 125, 8 ) );
        string strNodePA = trim ( line.substr ( 143, 8 ) );
        string strEpoch = trim ( line.substr ( 162, 12 ) );
        string strEccen = trim ( line.substr ( 187, 8 ) );
        string strOmega = trim ( line.substr ( 205, 8 ) );
        string strHD = trim ( line.substr ( 51, 6 ) );
        string strHIP = trim ( line.substr ( 58, 6 ) );
        string strWDS = trim ( line.substr ( 19, 10 ) );
        string strADS = trim ( line.substr ( 45, 5 ) );
        string strRef = trim ( line.substr ( 30, 7 ) );
        string strComps = trim ( line.substr ( 37, 5 ) );
        
        if ( strRAh.empty() || strDecd.empty() )
            continue;
        
        // Get right ascension and declination and convert to radians.

        SSHourMinSec ra  ( strRAh + " " + strRAm + " " + strRAs );
        SSDegMinSec  dec ( strDecd + " " + strDecm + " " + strDecs );
        SSSpherical coords ( SSAngle ( ra ), SSAngle ( dec ), INFINITY );
        
        // Get Magnitudes
        
        float mag1 = strMag1[0] == '.' ? INFINITY : strtofloat ( strMag1 );
        float mag2 = strMag2[0] == '.' ? INFINITY : strtofloat ( strMag2 );

        // Get orbital period and convert to mean motion in radians per day

        SSOrbit orbit;
        char unit = line[92];
        orbit.mm = strPeriod[0] == '.' ? 0.0 : SSAngle::kTwoPi / strtofloat64 ( strPeriod );
        if ( unit == 'y' )
            orbit.mm /= SSTime::kDaysPerJulianYear;
        else if ( unit == 'c' )
            orbit.mm /= SSTime::kDaysPerJulianYear * 100.0;
        
        // Get eccentricity and periastron in arcseconds
        
        unit = line[114];
        orbit.e = strtofloat64 ( strEccen );
        orbit.q = strtofloat64 ( strSMAxis ) * ( 1.0 - orbit.e );
        if ( unit == 'm' )
            orbit.q /= 1.0e3;
        else if ( unit == 'u' )
            orbit.q /= 1.0e6;
        
        // Convert inclination, argument of periastron,
        // position angle of ascending node to radians;
        // mean anomaly is zero at periastron.
        
        orbit.i = degtorad ( strtofloat64 ( strInclin ) );
        orbit.w = degtorad ( strtofloat64 ( strOmega ) );
        orbit.n = degtorad ( strtofloat64 ( strNodePA ) );
        orbit.m = 0.0;
        
        // Convert epoch of periastron to Julian date
        
        unit = line[174];
        if ( unit == 'd' )
            orbit.t = strtofloat64 ( strEpoch ) + 2400000.0;
        else if ( unit == 'm' )
            orbit.t = strtofloat64 ( strEpoch ) + 2400000.5;
        else if ( unit == 'y' )
            orbit.t = SSTime::fromBesselianYear ( strtofloat64 ( strEpoch ) );

        // Get HD, Hipparcos, WDS catalog designations
        // TODO: Get ADS, discoverer reference
        
        vector<SSIdentifier> idents;
        
        if ( strHD[0] != '.' )
            idents.push_back ( SSIdentifier ( kCatHD, strtoint ( strHD ) ) );
        
        if ( strHIP[0] != '.' )
            idents.push_back ( SSIdentifier ( kCatHIP, strtoint ( strHD ) ) );

        if ( strWDS[0] != '.' )
            idents.push_back ( SSIdentifier::fromString ( "WDS " + strWDS ) );

        sort ( idents.begin(), idents.end(), compareSSIdentifiers );

        // Construct star and insert into star vector.

        SSObjectPtr pObj = SSNewObject ( kTypeDoubleStar );
        SSDoubleStarPtr pStar = SSGetDoubleStarPtr ( pObj );
        if ( pStar != nullptr )
        {
            pStar->setIdentifiers ( idents );
            pStar->setFundamentalCoords ( coords );
            pStar->setVMagnitude ( mag1 );
            pStar->setMagnitudeDelta ( ::isinf ( mag1 ) || ::isinf ( mag2 ) ? INFINITY : mag2 - mag1 );
            
            // Components field seems to often be overwritten with discoverer reference numbers,
            // so validate it by checking that the first character is a capital A thru D.

            if ( strComps[0] >= 'A' && strComps[0] <= 'D' )
                pStar->setComponents ( strComps );

            pStar->setOrbit ( orbit );

            cout << pStar->toCSV() << endl;
            stars.push_back ( pObj );
            numStars++;
        }
    }
    
    return numStars;
}
