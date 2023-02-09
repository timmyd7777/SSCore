// SSHipparcos.cpp
// SSCore
//
// Created by Tim DeBenedictis on 3/23/20.
// Copyright © 2020 Southern Stars. All rights reserved.

#include <algorithm>
#include <iostream>
#include <fstream>

#include "SSCoordinates.hpp"
#include "SSImportGCVS.hpp"
#include "SSImportHIP.hpp"
#include "SSImportSKY2000.hpp"

// Cleans up some oddball conventions in the Hipparcos star name identification tables
// for Bayer, Flamsteed, and variable star names so SSIdentifier understands them.
// Returns cleaned-up name string, does not modify input string.

string cleanHIPNameString ( string str )
{
    // Change abbreviation for "alpha" from "alf" to "alp"
    
    if ( str.find ( "alf" ) == 0 )
        str.replace ( 0, 3, "alp" );
    
    // Change abbreviation for "xi" from "ksi"
    
    if ( str.find ( "ksi" ) == 0 )
        str.replace ( 0, 3, "xi." );
    
    // Remove "." after "mu", "nu", "xi"
    
    size_t idx = str.find ( "." );
    if ( idx != string::npos )
        str.erase ( idx, 1 );
    
    // Remove multiple star designations "_A", "_B", "_C" etc. after constellation
    
    size_t len = str.length();
    if ( str[ len - 2 ] == '_' )
        str.erase ( len - 2, 2 );
    
    // Convert remaining underscores to whitespace.
    
    idx = str.find ( "_" );
    if ( idx != string::npos )
        str.replace ( idx, 1, " " );
    
    return str;
}

// Imports Hipparcos star name identification table (IDENT6.DOC)
// into map of HIP identifiers to name strings (nameMap).
// Returns number of names imported (should be 96 if successful).

int SSImportHIPNames ( const string &filename, SSIdentifierNameMap &nameMap )
{
    // Open file; return on failure.

    ifstream file ( filename );
    if ( ! file )
        return 0;

    // Read file line-by-line until we reach end-of-file

    string line = "";
    int nameCount = 0;

    while ( getline ( file, line ) )
    {
        string strHIP = trim ( line.substr ( 17, 6 ) );
        string strName = trim ( line.substr ( 0, 16 ) );
        
        int hip = strtoint ( strHIP );
        if ( ! hip )
            continue;
        
        // cout << hip << "," << strName << endl;
        nameMap.insert ( { SSIdentifier ( kCatHIP, hip ), strName } );
        nameCount++;
    }

    // Return imported name count; file is closed automatically.

    return nameCount;
}

// Updates star coordinates and motion for space velocity from the input julian year (jyear) to 2000.0
// and for precession from an input equinox to J2000, using a rotation matrix (pMatrix) as returned by
// SSCoordinates::getPrecessionMatrxx() - but use transpose of matrix returned by that function!
// Pass null pointer for pMatrix if coords and motion alread refer to equinox J2000.
// This function uses a rigorous transformation which is accurate in all parts of the sky.

void SSUpdateStarCoordsAndMotion ( double jyear, SSMatrix *pMatrix, SSSpherical &coords, SSSpherical &motion )
{
    double rad = coords.rad;
    double radvel = motion.rad;
    
    coords.rad = 1.0;
    motion.rad = 0.0;
    
    SSVector position = coords.toVectorPosition();
    SSVector velocity = coords.toVectorVelocity ( motion );
    
    if ( jyear != 2000.0 )
    {
        position += velocity * ( 2000.0 - jyear );
        position = position.normalize();
    }
    
    if ( pMatrix != nullptr )
    {
        position = *pMatrix * position;
        velocity = *pMatrix * velocity;
    }
    
    coords = position.toSpherical();
    motion = position.toSphericalVelocity ( velocity );
    
    coords.rad = rad;
    motion.rad = radvel;
}

// Imports the Hipparcos Input Catalog, version 2.
// Still useful for SAO and variable star identifiers
// and radial velocities, all omitted from the final Hipparcos catalog.
// Stores results in vector of SSObjects (stars).
// Returns number of objects imported (118209 if successful).

int SSImportHIC ( const string &filename, SSObjectVec &stars )
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
        string strHIP = trim ( line.substr ( 0, 6 ) );
        string strRA = trim ( line.substr ( 13, 12 ) );
        string strDec = trim ( line.substr ( 26, 12 ) );
        string strPMRA = trim ( line.substr ( 155, 6 ) );
        string strPMDec = trim ( line.substr ( 162, 6 ) );
        string strMag = trim ( line.substr ( 190, 6 ) );
        string strBmV = trim ( line.substr ( 202, 6 ) );
        string strSpec = trim ( line.substr ( 216, 11 ) );
        string strPlx = trim ( line.substr ( 230, 6 ) );
        string strRV = trim ( line.substr ( 241, 6 ) );
        string strVar = trim ( line.substr ( 251, 9 ) );
        string strVarType = trim ( line.substr ( 261, 3 ) );
        string strVarPer = trim ( line.substr ( 265, 6 ) );
        string strVarMax = trim ( line.substr ( 272, 4 ) );
        string strVarMin = trim ( line.substr ( 277, 4 ) );
        string strHD = trim ( line.substr ( 359, 6 ) );
        string strBD = trim ( line.substr ( 320, 10 ) );
        string strCD = trim ( line.substr ( 334, 10 ) );
        string strCP = trim ( line.substr ( 348, 10 ) );
        string strSAO = trim ( line.substr ( 385, 6 ) );

        // Get J2000 Right Ascension and Declination
        
        SSSpherical position ( INFINITY, INFINITY, INFINITY );
        SSSpherical velocity ( INFINITY, INFINITY, INFINITY );
        
        position.lon = SSHourMinSec ( strRA );
        position.lat = SSDegMinSec ( strDec );
        
        // If we have a parallax, use it to compute distance in light years
        
        float plx = strPlx.empty() ? 0.0 : strtofloat ( strPlx );
        if ( plx > 0.0 )
            position.rad = 1000.0 * SSCoordinates::kLYPerParsec / plx;
        
        // Convert proper motion to radians per year
        
        if ( ! strPMRA.empty() )
            velocity.lon = SSAngle::fromArcsec ( strtofloat ( strPMRA ) ) / cos ( position.lat );
        
        if ( ! strPMDec.empty() )
            velocity.lat = SSAngle::fromArcsec ( strtofloat ( strPMDec ) );
        
        // Convert radial velocity from km/sec to fraction of light speed
        
        if ( ! strRV.empty() )
            velocity.rad = strtofloat ( strRV ) / SSCoordinates::kLightKmPerSec;
        
        // Get Johnson V magnitude; get B magnitude from B-V color index.
        
        float vmag = strMag.empty() ? INFINITY : strtofloat ( strMag );
        float bmag = strBmV.empty() ? INFINITY : strtofloat ( strBmV ) + vmag;
        
        vector<SSIdentifier> idents ( 0 );
        vector<string> names ( 0 );
        
        if ( ! strHD.empty() )
            SSAddIdentifier ( SSIdentifier ( kCatHD, strtoint ( strHD ) ), idents );

        if ( ! strSAO.empty() )
            SSAddIdentifier ( SSIdentifier ( kCatSAO, strtoint ( strSAO ) ), idents );

        if ( ! strHIP.empty() )
            SSAddIdentifier ( SSIdentifier ( kCatHIP, strtoint ( strHIP ) ), idents );
        
        // Sert identifier vector.  Get name string(s) corresponding to identifier(s).
        // Construct star and insert into star vector.

        sort ( idents.begin(), idents.end(), compareSSIdentifiers );
        SSObjectType type = kTypeStar;

        SSObjectPtr pObj = SSNewObject ( type );
        SSStarPtr pStar = SSGetStarPtr ( pObj );
        
        if ( pStar != nullptr )
        {
            pStar->setNames ( names );
            pStar->setIdentifiers ( idents );
            pStar->setFundamentalMotion ( position, velocity );
            pStar->setVMagnitude ( vmag );
            pStar->setBMagnitude ( bmag );
            pStar->setSpectralType ( strSpec );

            // cout << pStar->toCSV() << endl;
            stars.append ( pObj );
            numStars++;
        }
    }

    // Return imported star count; file is closed automatically.
    
    return numStars;
}

// Imports Hipparcos New Reduction 2007 star catalog (HIP2).
// Stores results in vector of SSObjects (stars).
// Returns number of objects imported (117955 if successful).

int SSImportHIP2 ( const string &filename, SSObjectVec &stars )
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
        string strHIP = trim ( line.substr ( 0, 6 ) );
        string strRA = trim ( line.substr ( 15, 13 ) );
        string strDec = trim ( line.substr ( 29, 13 ) );
        string strPMRA = trim ( line.substr ( 51, 8 ) );
        string strPMDec = trim ( line.substr ( 60, 8 ) );
        string strMag = trim ( line.substr ( 129, 7 ) );
        string strBmV = trim ( line.substr ( 152, 6 ) );
        string strPlx = trim ( line.substr ( 43, 7 ) );

        if ( strRA.empty() || strDec.empty() )
            continue;
        
        SSSpherical position ( INFINITY, INFINITY, INFINITY );
        SSSpherical velocity ( INFINITY, INFINITY, INFINITY );

        // Get right ascension and declination in radians
        
        position.lon = strtofloat ( strRA );
        position.lat = strtofloat ( strDec );
        
        // Get proper motion in RA and Dec and convert to radians per year
        
        if ( ! strPMRA.empty() )
            velocity.lon = SSAngle::fromArcsec ( strtofloat ( strPMRA ) / 1000.0 ) / cos ( position.lat );
        
        if ( ! strPMDec.empty() )
            velocity.lat = SSAngle::fromArcsec ( strtofloat ( strPMDec ) / 1000.0 );
        
        // If proper motion is valid, use it to bring position from J1991.25 to J2000
        
        if ( ! isinf ( velocity.lon ) && ! isinf ( velocity.lat ) )
            SSUpdateStarCoordsAndMotion ( 1991.25, nullptr, position, velocity );
        
        // Get Hipparcos magnitude
        
        float vmag = INFINITY;
        if ( ! strMag.empty() )
            vmag = strtofloat ( strMag );
        
        // Get B-V color index and use it to convert Hipparcos magnitude to Johnson B and V
        
        float bmv = INFINITY, bmag = INFINITY;
        if ( ! strBmV.empty() )
        {
            bmv = strtofloat ( strBmV );
            vmag += -0.2964 * bmv + 0.1110 * bmv * bmv;
            bmag = vmag + bmv;
        }
        
        // If we have a positive parallax, use it to compute distance in light years
        
        if ( ! strPlx.empty() )
        {
            float plx = strtofloat ( strPlx );
            if ( plx > 0.0 )
                position.rad = 1000.0 * SSCoordinates::kLYPerParsec / plx;
        }
        
        // Add single Hipparcos identifier and empty name string.
        
        vector<SSIdentifier> idents ( 0 );
        vector<string> names ( 0 );

        int hip = strtoint ( strHIP );
        SSAddIdentifier ( SSIdentifier ( kCatHIP, hip ), idents );
        
        // Sert identifier vector.  Get name string(s) corresponding to identifier(s).
        // Construct star and insert into star vector.

        sort ( idents.begin(), idents.end(), compareSSIdentifiers );
        SSObjectType type = kTypeStar;

        SSObjectPtr pObj = SSNewObject ( type );
        SSStarPtr pStar = SSGetStarPtr ( pObj );
        
        if ( pStar != nullptr )
        {
            pStar->setNames ( names );
            pStar->setIdentifiers ( idents );
            pStar->setFundamentalMotion ( position, velocity );
            pStar->setVMagnitude ( vmag );
            pStar->setBMagnitude ( bmag );

            // cout << pStar->toCSV() << endl;
            stars.append ( pObj );
            numStars++;
        }
    }

    // Return imported star count; file is closed automatically.
    
    return numStars;
}

// Imports the main Hipparcos star catalog.
// Adds HR, Bayer/Flamsteed, and GCVS identifiers from auxiliary identification tables (mapHIPtoHR, mapHIPtoBF, mapHIPtoVar).
// Adds SAO identifiers and radial velocity from Hipparcos Input Catalog (hicStars).
// Uses position and proper motion with values from Hippacos New Reduction (hip2Stars) if possible.
// Adds star name strings from a mapping of identifiers to names (nameMap).
// Adds variability data from variable star vector (varStars), if not empty.
// Stores results in vector of SSObjects (stars).
// Returns number of objects imported (118218 if successful).

int SSImportHIP ( const string &filename, SSIdentifierMap &hrMap, SSIdentifierMap &bayMap, SSIdentifierMap &gcvsMap, SSIdentifierNameMap &nameMap, SSObjectVec &hicStars, SSObjectVec &hip2Stars, SSObjectVec &gcvsStars, SSObjectVec &stars )
{
    // Open file; return on failure.

    ifstream file ( filename );
    if ( ! file )
        return 0;

    // Make mappings of HIP identifiers to object indices
    // in HIC and HIP2 star vectors.
    
    SSObjectMap hicMap = SSMakeObjectMap ( hicStars, kCatHIP );
    SSObjectMap hip2Map = SSMakeObjectMap ( hip2Stars, kCatHIP );

    // Make mapping of GCVS and HIP identifiers to GCVS stars
    
    SSObjectMaps gcvsMaps;
    SSMakeObjectMaps ( gcvsStars, { kCatGCVS, kCatHIP }, gcvsMaps );

    // Read file line-by-line until we reach end-of-file

    string line = "";
    int numStars = 0;
    
    while ( getline ( file, line ) )
    {
        string strHIP = trim ( line.substr ( 8, 6 ) );
        string strRA = trim ( line.substr ( 51, 12 ) );
        string strDec = trim ( line.substr ( 64, 12 ) );
        string strPMRA = trim ( line.substr ( 87, 8 ) );
        string strPMDec = trim ( line.substr ( 96, 8 ) );
        string strMag = trim ( line.substr ( 41, 5 ) );
        string strBmV = trim ( line.substr ( 245, 6 ) );
        string strPlx = trim ( line.substr ( 79, 7 ) );
        string strSpec = trim ( line.substr ( 435, 12 ) );
        string strHD = trim ( line.substr ( 390, 6 ) );
        string strBD = trim ( line.substr ( 398, 9 ) );
        string strCD = trim ( line.substr ( 409, 9 ) );
        string strCP = trim ( line.substr ( 420, 9 ) );

        SSSpherical position ( INFINITY, INFINITY, INFINITY );
        SSSpherical velocity ( INFINITY, INFINITY, INFINITY );
        
        // Get right ascension and convert to radians
        
        if ( ! strRA.empty() )
            position.lon = SSAngle::fromDegrees ( strtofloat ( strRA ) );
        else
            position.lon = SSHourMinSec ( trim ( line.substr ( 17, 11 ) ) );
        
        // Get declination and convert to radians
        
        if ( ! strDec.empty() )
            position.lat = SSAngle::fromDegrees ( strtofloat ( strDec ) );
        else
            position.lat = SSDegMinSec ( trim ( line.substr ( 29, 11 ) ) );
        
        // Get proper motion in RA and convert to radians per year
        
        if ( ! strPMRA.empty() )
            velocity.lon = SSAngle::fromArcsec ( strtofloat ( strPMRA ) / 1000.0 ) / cos ( position.lat );
        
        // Get proper motion in Dec and convert to radians per year
        
        if ( ! strPMDec.empty() )
            velocity.lat = SSAngle::fromArcsec ( strtofloat ( strPMDec ) / 1000.0 );
        
        // If proper motion is valid, use it to update position and proper motion from J1991.25 to J2000.
        
        if ( ! isinf ( velocity.lon ) && ! isinf ( velocity.lat ) )
            SSUpdateStarCoordsAndMotion ( 1991.25, nullptr, position, velocity );
        
        // Get Johnson V magnitude, and (if present) get B-V color index then compute Johnson B magnitude.
        
        float vmag = strMag.empty() ? INFINITY : strtofloat ( strMag );
        float bmag = strBmV.empty() ? INFINITY : strtofloat ( strBmV ) + vmag;

        // If we have a positive parallax, use it to compute distance in light years.
        
        float plx = strPlx.empty() ? 0.0 : strtofloat ( strPlx );
        if ( plx > 0.0 )
            position.rad = 1000.0 * SSCoordinates::kLYPerParsec / plx;
        
        // Set up name and identifier vectors.

        vector<SSIdentifier> idents ( 0 );
        vector<string> names ( 0 );

        // Parse HIP catalog number and add Hipparcos identifier.

        int hip = strtoint ( strHIP );
        SSIdentifier hipID = SSIdentifier ( kCatHIP, hip );
        SSAddIdentifier ( hipID, idents );

        // Add Henry Draper and Durchmusterung identifiers.
        
        if ( ! strHD.empty() )
            SSAddIdentifier ( SSIdentifier ( kCatHD, strtoint ( strHD ) ), idents );

        if ( ! strBD.empty() )
            SSAddIdentifier ( SSIdentifier::fromString ( "BD " + strBD ), idents );
        
        if ( ! strCD.empty() )
            SSAddIdentifier ( SSIdentifier::fromString ( "CD " + strCD ), idents );

        if ( ! strCP.empty() )
            SSAddIdentifier ( SSIdentifier::fromString ( "CP " + strCP ), idents );

        // Add HR identification (if present) from Bright Star identification table.
        // Add Bayer and Flamsteed identifier(s) (if present) from Bayer identification table.
        // Add GCVS identifier(s) from the variable star ident table.

        SSAddIdentifiers ( hipID, hrMap, idents );
        SSAddIdentifiers ( hipID, bayMap, idents );
        SSAddIdentifiers ( hipID, gcvsMap, idents );

        // Add names(s) from identifier-to-name map.

        names = SSIdentifiersToNames ( idents, nameMap );

        // If we found a matching Hipparcos New Reduction star,
        // replace position and velocity with newer values.
        
        SSStarPtr pStar = SSGetStarPtr ( SSIdentifierToObject ( hipID, hip2Map, hip2Stars ) );
        if ( pStar != nullptr )
        {
            position = pStar->getFundamentalCoords();
            velocity = pStar->getFundamentalMotion();
        }

        // If parallax is unknown, compute distance in light years from spectral class and magnitudes
        
        if ( isinf ( position.rad ) )
            position.rad = SSCoordinates::kLYPerParsec * SSStar::spectralDistance ( strSpec, vmag, bmag );

        // If we found a matching Hipparcos Input Catalog star,
        // splice in SAO identifier and radial velocity.
        
        pStar = SSGetStarPtr ( SSIdentifierToObject ( hipID, hicMap, hicStars ) );
        if ( pStar != nullptr )
        {
            SSIdentifier saoID = pStar->getIdentifier ( kCatSAO );
            if ( saoID )
                SSAddIdentifier ( saoID, idents );
            
            velocity.rad = pStar->getRadVel();
        }

        // Can we find a corresponding variable star in the GCVS star vector?
        // If yes, add it GCVS identifier; we'll add other variability data below.
        
        SSVariableStarPtr pGCVStar = SSGetVariableStarPtr ( SSGetMatchingStar ( idents, gcvsMaps, gcvsStars ) );
        if ( pGCVStar )
            SSAddIdentifier ( pGCVStar->getIdentifier ( kCatGCVS ), idents );
        
        // Sert identifier vector.  Get name string(s) corresponding to identifier(s).
        // Construct star and insert into star vector.

        sort ( idents.begin(), idents.end(), compareSSIdentifiers );
        SSObjectType type = pGCVStar ? kTypeVariableStar : kTypeStar;

        SSObjectPtr pObj = SSNewObject ( type );
        pStar = SSGetStarPtr ( pObj );
        
        if ( pStar != nullptr )
        {
            pStar->setNames ( names );
            pStar->setIdentifiers ( idents );
            pStar->setFundamentalMotion ( position, velocity );
            pStar->setVMagnitude ( vmag );
            pStar->setBMagnitude ( bmag );
            pStar->setSpectralType ( strSpec );

            if ( pGCVStar )
                SSCopyVariableStarData ( pGCVStar, pStar );

            // cout << pStar->toCSV() << endl;
            stars.append ( pObj );
            numStars++;
        }
    }

    // Return imported star count; file is closed automatically.
    
    return numStars;
}

// Imports Hipparcos HR (Bright Star) identifier table (IDENT3.DOC).
// Returns map of HR identifiers indexed by HIP number,
// which should contain 9077 entries if successful.

int SSImportHIPHRIdentifiers ( const string &filename, SSIdentifierMap &map )
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
        string strHR = trim ( line.substr ( 0, 6 ) );
        string strHIP = trim ( line.substr ( 7, 6 ) );
        int hip = strtoint ( strHIP );
        int hr = strtoint ( strHR );
        
        if ( hip == 0 || hr == 0 )
            continue;
        
        // cout << hip << "," << hr << "," << endl;
        map.insert ( { SSIdentifier ( kCatHIP, hip ), SSIdentifier ( kCatHR, hr ) } );
        count++;
    }
    
    // Return count of identifiers added.  File closed automatically.

    return count;
}

// Imports Hipparcos Bayer/Flamsteed identifier table (IDENT4.DOC).
// Returns map of Bayer/Flamsteed identifiers indexed by HIP number,
// which should contain 4440 entries if successful.

int SSImportHIPBayerIdentifiers ( const string &filename, SSIdentifierMap &map )
{
    // Open file; return on failure.

    ifstream file ( filename );
    if ( ! file )
        return 0;
    
    // Read file line-by-line until we reach end-of-file

    string line = "";
    int count = 0;

    while ( getline ( file, line ) )
    {
        string strBF = trim ( line.substr ( 0, 11 ) );
        string strHIP = trim ( line.substr ( 12, 6 ) );

        SSIdentifier id = SSIdentifier::fromString ( cleanHIPNameString ( strBF ) );
        int hip = strtoint ( strHIP );

        if ( hip == 0 || id == 0 )
            continue;
        
        // cout << hip << "," << id.toString() << endl;
        map.insert ( { SSIdentifier ( kCatHIP, hip ), id } );
        count++;
    }
    
    // Return count of identifiers added.  File closed automatically.

    return count;
}

// Imports Hipparcos variable star identifier table (IDENT5.DOC).
// Returns map of GCVS identifiers indexed by HIP number,
// which should contain 6390 entries if successful.

int SSImportHIPGCVSIdentifiers ( const string &filename, SSIdentifierMap &map )
{
    // Open file; return on failure.

    ifstream file ( filename );
    if ( ! file )
        return 0;
    
    // Read file line-by-line until we reach end-of-file

    string line = "";
    int count = 0;
    
    while ( getline ( file, line ) )
    {
        string strVar = trim ( line.substr ( 0, 11 ) );
        string strHIP = trim ( line.substr ( 12, 6 ) );

        int hip = strtoint ( strHIP );
        SSIdentifier id = SSIdentifier::fromString ( cleanHIPNameString ( strVar ) );
        
        // cout << hip << "," << id.toString() << endl;
        
        if ( id == 0 || hip == 0 )
        {
            cout << "Warning: con't convert " << strVar << " for HIP " << hip << endl;
            continue;
        }
        
        // cout << hip << "," << id.toString() << endl;
        map.insert ( { SSIdentifier ( kCatHIP, hip ), id } );
        count++;
    }
    
    // Return count of identifiers added.  File closed automatically.

    return count;
}
