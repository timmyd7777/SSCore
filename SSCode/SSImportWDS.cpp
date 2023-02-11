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

// Imports the Sixth Catalog of Orbits of Visual Binary Stars
// (orb6orbits.txt) from http://www.astro.gsu.edu/wds/orb6/
// Adds Bayer, Flamsteed, identifiers from cross index (identmap).
// Stores results in vector of SSObjects (stars).
// Returns number of objects imported.

int SSImportORB6 ( const string &filename, const SSIdentifierMap &identMap, SSObjectArray &stars )
{
    // Open file; return on failure.

    ifstream file ( filename );
    if ( ! file )
        return 0;

    // Read file line-by-line until we reach end-of-file

    string line = "";
    int numStars = 0, numLines = 0;
    
    while ( getline ( file, line ) )
    {
        // Skip first 8 lines of header information
        
        numLines++;
        if ( numLines < 8 )
            continue;
        
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
        
        // Components field usually begins at char 38, but sometimes char 37, and is overwritten with
        // discoverer designation, so erase any leading characters before the first component designator.
        
        string strComps = trim ( line.substr ( 36, 6 ) );
        strComps.erase ( 0, strComps.find_first_of ( "ABCDEF" ) );

        if ( strRAh.empty() || strDecd.empty() )
            continue;
        
        // Get right ascension and declination and convert to radians.

        SSHourMinSec ra  ( strRAh + " " + strRAm + " " + strRAs );
        SSDegMinSec  dec ( strDecd + " " + strDecm + " " + strDecs );
        SSSpherical coords ( SSAngle ( ra ), SSAngle ( dec ), INFINITY );
        
        // Get Magnitudes. Swap if needed so mag2 is always fainter than mag1.
        
        float mag1 = strMag1[0] == '.' ? INFINITY : strtofloat ( strMag1 );
        float mag2 = strMag2[0] == '.' ? INFINITY : strtofloat ( strMag2 );
        if ( mag2 < mag1 )
            swap<float> ( mag1, mag2 );

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
        else if ( unit == 'M' )
            orbit.q *= 60.0;

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
            idents.push_back ( SSIdentifier ( kCatHIP, strtoint ( strHIP ) ) );

        if ( strWDS[0] != '.' )
            idents.push_back ( SSIdentifier::fromString ( "WDS " + strWDS ) );

        SSAddIdentifiers ( SSGetIdentifier ( kCatWDS, idents ), identMap, idents );
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
            pStar->setComponents ( strComps.empty() ? string ( "AB" ) : strComps );
            pStar->setOrbit ( orbit, coords.lon, coords.lat );

            // Compute separation and position angle for the year 2020
            
            SSAngle pa;
            double r, sep;
            orbit.toPositionSeparation ( SSTime::fromJulianYear ( 2020.0 ), pa, r, sep );
            pStar->setSeparation ( sep / SSAngle::kArcsecPerRad );
            pStar->setPositionAngle ( pa );
            pStar->setPositionAngleYear ( 2020.0 );

            // cout << pStar->toCSV() << endl;
            stars.append ( pObj );
            numStars++;
        }
    }
    
    return numStars;
}

// Imports WDS-Bayer-Flamsteed cross-index (wdsbf.txt).
// Inserts results into map of catalog identifiers, indexed by WDS identifier (identmap),
// and returns number of identifiers inserted into map.

int SSImportWDSBFCrossIndex ( const string &filename, SSIdentifierMap &identmap )
{
    // Open file; return on failure.
    
    ifstream file ( filename );
    if ( ! file )
        return 0;
    
    // Read file line-by-line until we reach end-of-file

    string line ( "" );
    int count = 0;
    
    while ( getline ( file, line ) )
    {
        if ( line.length() < 55 )
            continue;
        
        string strWDS = line.substr ( 0, 10 );
        string strID1 = line.substr ( 22, 9 );
        string strID2 = line.substr ( 44, 9 );

        SSIdentifier wds = SSIdentifier::fromString ( "WDS " + strWDS );
        SSIdentifier id1 = strID1[0] == '.' ? SSIdentifier() : SSIdentifier::fromString ( strID1 );
        SSIdentifier id2 = strID2[0] == '.' ? SSIdentifier() : SSIdentifier::fromString ( strID2 );

        if ( wds == 0 )
            continue;
        
        // cout << wds.toString() << "," << id1.toString() << "," << id2.toString() << endl;
        
        if ( id1 )
        {
            identmap.insert ( { wds, id1 } );
            count++;
        }
        
        if ( id2 && id2 != id1.strip() )
        {
            identmap.insert ( { wds, id2 } );
            count++;
        }
    }
    
    // Return count of identifiers added.  File is closed automatically.

    return count;
}

// Imports WDS-Hipparcos cross-index (wds2hds2hip.txt).
// Inserts results into map of catalog identifiers, indexed by WDS identifier (identmap),
// and returns number of identifiers inserted into map.

int SSImportWDSHIPCrossIndex ( const string &filename, SSIdentifierMap &identmap )
{
    // Open file; return on failure.
    
    ifstream file ( filename );
    if ( ! file )
        return 0;
    
    // Read file line-by-line until we reach end-of-file

    string line ( "" );
    int count = 0;
    
    while ( getline ( file, line ) )
    {
        if ( line.length() < 29 )
            continue;
        
        string strWDS = line.substr ( 0, 10 );
        string strHIP = line.substr ( 23, 6 );

        SSIdentifier wds = SSIdentifier::fromString ( "WDS " + strWDS );
        SSIdentifier hip = SSIdentifier::fromString ( "HIP " + strHIP );

        if ( wds == 0 || hip == 0 )
            continue;
        
        // cout << wds.toString() << "," << hip.toString() << "," << endl;
        identmap.insert ( { wds, hip } );
        count++;
    }
    
    // Return count of identifiers added.  File is closed automatically.

    return count;
}

// Imports the Washington Double Star Catalog (wdsweb_summ.txt) from http://www.astro.gsu.edu/wds/
// Adds Bayer, Flamsteed, HIP, TYC identifiers from cross index (identmap).
// Stores results in vector of SSObjects (stars).
// Returns number of objects imported.

int SSImportWDS ( const string &filename, const SSIdentifierMap &identmap, SSObjectArray &stars )
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
        if ( line.length() < 130 )
            continue;

        // Get notes.  An X indicates a "dubious double" or "bogus binary" - skip it.
        
        string notes = trim ( line.substr ( 107, 4 ) );
        if ( notes.find ( "X" ) != string::npos )
            continue;

        string strRAh = trim ( line.substr ( 112, 2 ) );
        string strRAm = trim ( line.substr ( 114, 2 ) );
        string strRAs = trim ( line.substr ( 116, 5 ) );
        string strDecd = trim ( line.substr ( 121, 3 ) );
        string strDecm = trim ( line.substr ( 124, 2 ) );
        string strDecs = trim ( line.substr ( 126, 4 ) );
        string strPMRA = trim ( line.substr ( 80, 4 ) );
        string strPMDec = trim ( line.substr ( 84, 4 ) );
        string strMag1 = trim ( line.substr ( 58, 5 ) );
        string strMag2 = trim ( line.substr ( 64, 5 ) );
        string strSep = trim ( line.substr ( 52, 5 ) );
        string strPA = trim ( line.substr ( 42, 3 ) );
        string strPAyr = trim ( line.substr ( 28, 4 ) );
        string strComps = trim ( line.substr ( 17, 5 ) );
        string strSpec = trim ( line.substr ( 70, 9 ) );
        string strWDS = trim ( line.substr ( 0, 10 ) );
        string strRef = trim ( line.substr ( 10, 7 ) );
        string strDM = trim ( line.substr ( 98, 8 ) );
        
        // Ignore entries without precise coordinates.
        
        if ( strRAh.empty() || strDecd.empty() )
            continue;
        
        // Get right ascension and declination and convert to radians.

        SSHourMinSec ra  ( strRAh + " " + strRAm + " " + strRAs );
        SSDegMinSec  dec ( strDecd + " " + strDecm + " " + strDecs );
        SSSpherical coords ( SSAngle ( ra ), SSAngle ( dec ), INFINITY );

        // Get proper motion in right ascension and declination and convert to radians/year.

        double pmRA = strPMRA.empty() ? INFINITY : strtofloat64 ( strPMRA ) * 1.0e-3 / cos ( coords.lat );
        double pmDec = strPMDec.empty() ? INFINITY : strtofloat64 ( strPMDec ) * 1.0e-3;
        SSSpherical motion ( SSAngle::fromArcsec ( pmRA ), SSAngle::fromArcsec ( pmDec ), INFINITY );

        // Get Magnitudes. Swap if needed so mag2 is always fainter than mag1.
        
        float mag1 = strMag1.empty() ? INFINITY : strtofloat ( strMag1 );
        float mag2 = strMag2.empty() ? INFINITY : strtofloat ( strMag2 );
        if ( mag2 < mag1 )
            swap<float> ( mag1, mag2 );
        
        // K in the notes indicates a infrared magnitudes that we can't use.

        if ( notes.find ( "K" ) != string::npos )
            mag1 = mag2 = INFINITY;

        // Get most recent component separation, position angle, and year of measurement.
        // A number of WDS entries have separations of 999.9 arcsec - this is obviously
        // some kind of error, so we eliminate those entries entirely.

        float sep = strSep.empty() ? INFINITY : strtofloat ( strSep );
        if ( sep * 10 == 9999 )
            continue;

        float pa = strPA.empty() ? INFINITY : strtofloat ( strPA );
        float payr = strPAyr.empty() ? INFINITY : strtofloat ( strPAyr );
        
        // Get WDS and DM catalog designations
        // TODO: Get ADS, discoverer reference
        
        vector<SSIdentifier> idents;
        SSIdentifier wds = SSIdentifier::fromString ( "WDS " + strWDS );
        if ( wds == 0 )
            continue;
        
        if ( strDM.length() )
        {
            int dec = strtoint ( strDM );
            if ( dec > -23 && dec < 90 )
                idents.push_back ( SSIdentifier::fromString ( "BD " + strDM ) );
            else if ( dec > -52 )
                idents.push_back ( SSIdentifier::fromString ( "CD " + strDM ) );
            else if ( dec > -90 )
                idents.push_back ( SSIdentifier::fromString ( "CP " + strDM ) );
        }
        
        // Add WDS and alternate catalog idents from the cross-identification table.
        // Sert identifier vector.

        idents.push_back ( wds );
        SSAddIdentifiers ( wds, identmap, idents );
        sort ( idents.begin(), idents.end(), compareSSIdentifiers );

        // Construct star and insert into star vector.

        SSObjectPtr pObj = SSNewObject ( kTypeDoubleStar );
        SSDoubleStarPtr pStar = SSGetDoubleStarPtr ( pObj );
        if ( pStar != nullptr )
        {
            pStar->setIdentifiers ( idents );
            pStar->setFundamentalCoords ( coords );
            pStar->setSpectralType ( strSpec );
            pStar->setMagnitudeDelta ( ::isinf ( mag1 ) || ::isinf ( mag2 ) ? INFINITY : mag2 - mag1 );
            pStar->setSeparation ( degtorad ( sep / 3600 ) );
            pStar->setPositionAngle ( degtorad ( pa ) );
            pStar->setPositionAngleYear ( payr );
            pStar->setComponents ( strComps.empty() ? string ( "AB" ) : strComps );

            // B in the notes indicates a blue photographic magnitude.
            
            if ( notes.find ( "B" ) != string::npos )
                pStar->setBMagnitude ( mag1 );
            else
                pStar->setVMagnitude ( mag1 );

            // cout << pStar->toCSV() << endl;
            stars.append ( pObj );
            numStars++;
        }
    }
    
    return numStars;
}

// Imports ORB6 catalog to Heirarchal Triangular Mesh (htm) instead of SSObjectArray.
// Returns number of ORB6 stars stored in HTM.

int SSImportORB6toHTM ( const string &filename, const SSIdentifierMap &identMap, SSHTM &htm )
{
    SSObjectArray stars;
    int n = SSImportORB6 ( filename, identMap, stars );
    if ( n > 0 )
    {
        vector<float> magLevels = { INFINITY };
        htm = SSHTM ( magLevels, "" );
        n = htm.store ( stars );
        if ( n > 0 )
        {
            stars.clear();
            return n;
        }
    }

    return 0;
}

// Imports WDS catalog to Heirarchal Triangular Mesh (htm) instead of SSObjectArray.
// Returns number of WDS stars stored in HTM.

int SSImportWDStoHTM ( const string &filename, const SSIdentifierMap &identmap, SSHTM &htm )
{
    SSObjectArray stars;
    int n = SSImportWDS ( filename, identmap, stars );
    if ( n > 0 )
    {
        vector<float> magLevels = { 6.0, 7.2, 8.4, INFINITY };
        htm = SSHTM ( magLevels, "" );
        n = htm.store ( stars );
        if ( n > 0 )
        {
            stars.clear();
            return n;
        }
    }

    return 0;
}

// Searches for double star in double star HTM (wdsHTM)
// within 1 arcminute of target coordinates (coords)
// matching a component character A, B, C, D, etc. (comp)
// with an angular separation in arcseconds (sep); will be ignored if zero.
// If found, returns primary component character (prim), and
// returns pointer to matching double star, or nullptr if none.

SSDoubleStarPtr SSFindWDSStar ( SSHTM &wdsHTM, SSSpherical coords, char comp, char &prim, float sep )
{
    if ( comp < 'A' || comp > 'F' )
       return nullptr;
        
    // Find all WDS stars whose primaries are within 1 arcminute of the target coordinates.
    
    vector<SSObjectPtr> results;
    wdsHTM.search ( 0, coords, SSAngle::fromArcmin ( 1.0 ), results );
    if ( results.size() < 1 )
        return nullptr;
    
    for ( SSObjectPtr &pObj : results )
    {
        SSDoubleStarPtr pWDStar = SSGetDoubleStarPtr ( pObj );
        if ( pWDStar == nullptr )
            continue;
        
        // Reject binary orbits for unseen components like Aa, Bb, etc.
        
        string compsWD = pWDStar->getComponents();
        if ( compsWD.length() < 2 || ( compsWD[1] >= 'a' && compsWD[1] <= 'z' ) )
            continue;

        // Reject orbit if separation is greater than twice the apastron.
        
        if ( pWDStar->hasOrbit() )
        {
            SSOrbit orbit = pWDStar->getOrbit();
            if ( sep > 0.0 && sep > orbit.apoapse() * 2 )
                continue;
        }
        else
        {
            if ( sep > 0.0 && SSAngle::fromArcsec ( sep ) > pWDStar->getSeparation() * 2 )
                continue;
        }
        
        // cout << pWDStar->toCSV() << endl;

        // If component matches first char of WDS component string like AB, BC, CD, component is primary.
        
        if ( compsWD.length() == 2 && compsWD[0] == comp && compsWD[1] == compsWD[0] + 1 )
        {
            prim = comp;
            return pWDStar;
        }
        
        // If component matches last char of WDS component string, component is secondary.
        
        if ( compsWD.back() == comp )
        {
            prim = compsWD[0];
            return pWDStar;
        }
    }
    
    return nullptr;
}

// Copies double star data (including binary star orbit if present)
// from WDS star (pWDStar) into target star (pStar).
// The target star's component designation A, B, C, D is (comp).
// The WDS star's primary component designation A, B, C, D is (primComp).

bool SSCopyDoubleStarData ( SSDoubleStarPtr pWDStar, char comp, char primComp, SSStarPtr pStar )
{
    SSDoubleStarPtr pDbl = SSGetDoubleStarPtr ( pStar );
    if ( pDbl == nullptr )
        return false;
    
    if ( pWDStar->hasOrbit() )
    {
        pDbl->setOrbit ( pWDStar->getOrbit() );
        if ( comp == primComp )
            pDbl->setComponents ( pWDStar->getComponents() );
        else
            pDbl->setComponents ( string ( 1, comp ) + string ( 1, primComp ) );
    }
    
    pDbl->setMagnitudeDelta ( pWDStar->getMagnitudeDelta() );
    pDbl->setSeparation ( pWDStar->getSeparation() );
    pDbl->setPositionAngle ( pWDStar->getPositionAngle() );
    pDbl->setPositionAngleYear ( pWDStar->getPositionAngleYear() );
    pDbl->addIdentifier ( pWDStar->getIdentifier ( kCatWDS ) );
    pDbl->sortIdentifiers();
    return true;
}

