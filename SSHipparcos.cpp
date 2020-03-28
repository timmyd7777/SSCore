// SSHipparcos.cpp
// SSCore
//
// Created by Tim DeBenedictis on 3/23/20.
// Copyright © 2020 Southern Stars. All rights reserved.

#include <iostream>
#include <fstream>

#include "SSDynamics.hpp"
#include "SSHipparcos.hpp"

// Updates star coordinates and motion for space velocity from the input julian year (jyear) to 2000.0
// and for precession from an input equinox to J2000, using a rotation matrix (pMatrix) as returned by
// SSCoords::getPrecessionMatrxx() - but use transpose of matrix returned by that function!
// Pass null pointer for pMatrix if coords and motion alread refer to equinox J2000.
// This function uses a rigorous transformation which is accurate in all parts of the sky.

void SSUpdateStarCoordsMotion ( double jyear, SSMatrix *pMatrix, SSSpherical &coords, SSSpherical &motion )
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

// Imports Hipparcos star name identification table (IDENT6.DOC).
// Returns map of name strings identifiers indexed by HIP number,
// which should contain 96 entries if successful.

HIPNameMap importHIPNameMap ( const char *filename )
{
    HIPNameMap hipNameMap;
    
    // Open file; report error and return empty map on failure.

    ifstream file ( filename );
    if ( ! file )
    {
        cout << "Failure: can't open " << filename << endl;
        return hipNameMap;
    }

    // Read file line-by-line until we reach end-of-file

    string line = "";
    int linecount = 0;

    while ( getline ( file, line ) )
    {
        linecount++;
        string strHIP = trim ( line.substr ( 17, 6 ) );
        string strName = trim ( line.substr ( 0, 16 ) );
        
        int hip = strtoint ( strHIP );
        if ( ! hip )
            continue;
        
        // cout << hip << "," << strName << endl;
        hipNameMap.insert ( { hip, strName } );
    }

    return hipNameMap;
}

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

// Imports the Hipparcos Input Catalog, version 2.
// Still useful for SAO and variable star identifiers
// and radial velocities, all omitted from the final Hipparcos catalog.

SSStarMap importHIC ( const char *filename )
{
    SSStarMap starmap;
    ifstream hic_file ( filename );
    
    if ( ! hic_file )
        return starmap;
    
    string line;
    int linecount = 0;
    while ( getline ( hic_file, line ) )
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

        linecount++;
        
        SSSpherical position ( HUGE_VAL, HUGE_VAL, HUGE_VAL );
        SSSpherical velocity ( HUGE_VAL, HUGE_VAL, HUGE_VAL );
        
        position.lon = SSHourMinSec ( strRA );
        position.lat = SSDegMinSec ( strDec );
        
        float plx = strPlx.empty() ? 0.0 : strtofloat ( strPlx );
        if ( plx > 0.0 )
            position.rad = 1000.0 / plx;
        
        if ( ! strPMRA.empty() )
            velocity.lon = SSAngle::fromArcsec ( strtofloat ( strPMRA ) ) / cos ( position.lat );
        
        if ( ! strPMDec.empty() )
            velocity.lat = SSAngle::fromArcsec ( strtofloat ( strPMDec ) );
        
        if ( ! strRV.empty() )
            velocity.rad = strtofloat ( strRV ) / SSDynamics::kLightKmPerSec;
        
        float vmag = strMag.empty() ? HUGE_VAL : strtofloat ( strMag );
        float bmag = strBmV.empty() ? HUGE_VAL : vmag - strtofloat ( strBmV );
        
        vector<SSIdentifier> idents ( 0 );
        vector<string> names ( 0 );
        
        if ( ! strHD.empty() )
            addIdentifier ( idents, SSIdentifier ( kCatHD, strtoint ( strHD ) ) );

        if ( ! strSAO.empty() )
            addIdentifier ( idents, SSIdentifier ( kCatSAO, strtoint ( strSAO ) ) );

        if ( ! strHIP.empty() )
            addIdentifier ( idents, SSIdentifier ( kCatHIP, strtoint ( strHIP ) ) );
        
        int hip = strtoint ( strHIP );
        SSStar star;
        
        star.setNames ( names );
        star.setIdentifiers ( idents );
        star.setFundamentalMotion ( position, velocity );
        star.setVMagnitude ( vmag );
        star.setBMagnitude ( bmag );
        star.setSpectralType ( strSpec );
        
        // cout << star.toCSV() << endl;
        starmap.insert ( { hip, star } );
    }
    
    // Report success or failure.  Return star map object.

    if ( linecount == starmap.size() )
        cout << "Success: " << filename << " linecount " << linecount << " == starmap.size() " << starmap.size() << endl;
    else
        cout << "Failure: " << filename << " linecount " << linecount << " != starmap.size() " << starmap.size() << endl;

    return starmap;
}

// Imports the main Hipparcos star catalog.
// Adds HR, Bayer/Flamsteed, and GCVS identifiers from auxiliary identification tables (mapHIPtoHR, mapHIPtoBF, mapHIPtoVar).
// Adds SAO identifiers and radial velocity from Hipparcos Input Catalog (mapHIC).
// Uses position and proper motion with values from Hippacos New Reduction (mapHIP2) if possible.
// Adds star name strings from a mapping of identifiers to names (nameMap).
// Returns map of SSStar objects indexed by HIP number, which should contain 118218 entries if successful.

SSStarMap importHIP ( const char *filename, HIPMap mapHIPtoHR, HIPMap mapHIPtoBF, HIPMap mapHIPtoVar, SSStarMap mapHIC, SSStarMap mapHIP2, HIPNameMap nameMap )
{
    SSStarMap starmap;
    ifstream file ( filename );
    
    if ( ! file )
    {
        cout << "Failure: can't open " << filename << endl;
        return starmap;
    }

    // Read file line-by-line until we reach end-of-file

    string line = "";
    int linecount = 0;
    
    while ( getline ( file, line ) )
    {
        linecount++;
        
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

        SSSpherical position ( HUGE_VAL, HUGE_VAL, HUGE_VAL );
        SSSpherical velocity ( HUGE_VAL, HUGE_VAL, HUGE_VAL );
        
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
            SSUpdateStarCoordsMotion ( 1991.25, nullptr, position, velocity );
        
        // Get Johnson V magnitude, and (if present) get B-V color index then compute Johnson B magnitude.
        
        float vmag = strMag.empty() ? HUGE_VAL : strtofloat ( strMag );
        float bmag = strBmV.empty() ? HUGE_VAL : vmag - strtofloat ( strBmV );

        // If we have a parallax > 1 milliarcsec, use it to compute distance in parsecs.
        
        float plx = strPlx.empty() ? 0.0 : strtofloat ( strPlx );
        if ( plx > 0.0 )
            position.rad = 1000.0 / plx;
        
        // Set up name and identifier vectors.

        vector<SSIdentifier> idents ( 0 );
        vector<string> names ( 0 );

        // Parse HIP catalog number and add Hipparcos identifier.

        int hip = strtoint ( strHIP );
        SSIdentifier hipID = SSIdentifier ( kCatHIP, hip );
        addIdentifier ( idents, hipID );

        // Add Henry Draper and Durchmusterung identifiers.
        
        if ( ! strHD.empty() )
            addIdentifier ( idents, SSIdentifier ( kCatHD, strtoint ( strHD ) ) );

        if ( ! strBD.empty() )
            addIdentifier ( idents, SSIdentifier::fromString ( "BD " + strBD ) );
        
        if ( ! strCD.empty() )
            addIdentifier ( idents, SSIdentifier::fromString ( "CD " + strCD ) );

        if ( ! strCP.empty() )
            addIdentifier ( idents, SSIdentifier::fromString ( "CP " + strCP ) );

        // Add HR identification (if present) from Bright Star identification table.
        
        auto rangeHR = mapHIPtoHR.equal_range ( hip );
        for ( auto i = rangeHR.first; i != rangeHR.second; i++ )
            addIdentifier ( idents, i->second );

        // Add Bayer and Flamsteed identifier(s) (if present) from Bayer identification table.

        auto rangeBF = mapHIPtoBF.equal_range ( hip );
        for ( auto i = rangeBF.first; i != rangeBF.second; i++ )
            addIdentifier ( idents, i->second );
        
        // Add GCVS identifier(s) from the variable star ident table.
        // Don't add GCVS identifiers which are Bayer/Flamsteed letters!

        auto rangeVar = mapHIPtoVar.equal_range ( hip );
        for ( auto i = rangeVar.first; i != rangeVar.second; i++ )
            if ( i->second.catalog() == kCatGCVS )
                addIdentifier ( idents, i->second );

        // Add names(s) from identifier-to-name map.

        auto nameRange = nameMap.equal_range ( hip );
        for ( auto i = nameRange.first; i != nameRange.second; i++ )
            names.push_back ( i->second );

        // If we found a matching Hipparcos New Reduction star,
        // replace position and velocity with newer values.
        
        SSStar hip2Star = mapHIP2[ hip ];
        if ( hip2Star.getIdentifier ( kCatHIP ) == hipID )
        {
            position = hip2Star.getFundamentalCoords();
            velocity = hip2Star.getFundamentalMotion();
        }

        // If we found a matching Hipparcos Input Catalog star,
        // splice in SAO identifier and radial velocity.
        
        SSStar hicStar = mapHIC[ hip ];
        if ( hicStar.getIdentifier ( kCatHIP ) == hipID )
        {
            SSIdentifier saoID = hicStar.getIdentifier ( kCatSAO );
            if ( saoID )
                addIdentifier ( idents, saoID );
            
            velocity.rad = hicStar.getRadVel();
        }

        // Sert identifier vector. Construct star and insert into star map object.
        
        sort ( idents.begin(), idents.end() ); // , compareSSIdentifiers );
        
        SSStar star;
        
        star.setNames ( names );
        star.setIdentifiers ( idents );
        star.setFundamentalMotion ( position, velocity );
        star.setVMagnitude ( vmag );
        star.setBMagnitude ( bmag );
        star.setSpectralType ( strSpec );
        
        cout << star.toCSV() << endl;
        starmap.insert ( { hip, star } );
    }

    // Report success or failure.  Return star map object.

    if ( linecount == starmap.size() )
        cout << "Success: " << filename << " linecount " << linecount << " == starmap.size() " << starmap.size() << endl;
    else
        cout << "Failure: " << filename << " linecount " << linecount << " != starmap.size() " << starmap.size() << endl;

    return starmap;
}

// Imports Hipparcos HR (Bright Star) identifier table (IDENT3.DOC).
// Returns map of HR identifiers indexed by HIP number,
// which should contain 9077 entries if successful.

HIPMap importHIPtoHRMap ( const char *filename )
{
    HIPMap mapHIPtoHR;
    
    // Open file; report error and return empty map on failure.
    
    ifstream file ( filename );
    if ( ! file )
    {
        cout << "Failure: can't open " << filename << endl;
        return mapHIPtoHR;
    }
    
    // Read file line-by-line until we reach end-of-file

    string line ( "" );
    int linecount = 0;
    
    while ( getline ( file, line ) )
    {
        linecount++;
        string strHR = trim ( line.substr ( 0, 6 ) );
        string strHIP = trim ( line.substr ( 7, 6 ) );
        int hip = strtoint ( strHIP );

        SSIdentifier id = SSIdentifier ( kCatHR, strtoint ( strHR ) );
        // cout << hip << "," << id.toString() << "," << endl;
        mapHIPtoHR.insert ( { hip, id } );
    }
    
    // Report success or failure.  Return identifier map object.

    if ( linecount == mapHIPtoHR.size() )
        cout << "Success: " << filename << " linecount " << linecount << " == mapHIPtoHR.size() " << mapHIPtoHR.size() << endl;
    else
        cout << "Failure: " << filename << " linecount " << linecount << " != mapHIPtoHR.size() " << mapHIPtoHR.size() << endl;

    return mapHIPtoHR;
}

// Imports Hipparcos Bayer/Flamsteed identifier table (IDENT4.DOC).
// Returns map of Bayer/Flamsteed identifiers indexed by HIP number,
// which should contain 4440 entries if successful.

HIPMap importHIPtoBayerFlamsteedMap ( const char *filename )
{
    HIPMap mapHIPtoBF;
    
    // Open file; report error and return empty map on failure.

    ifstream file ( filename );
    if ( ! file )
    {
        cout << "Failure: can't open " << filename << endl;
        return mapHIPtoBF;
    }
    
    // Read file line-by-line until we reach end-of-file

    string line = "";
    int linecount = 0;

    while ( getline ( file, line ) )
    {
        linecount++;
        string strBF = trim ( line.substr ( 0, 11 ) );
        string strHIP = trim ( line.substr ( 12, 6 ) );
        int hip = strtoint ( strHIP );

        strBF = cleanHIPNameString ( strBF );
        SSIdentifier id = SSIdentifier::fromString ( strBF );
        
        // cout << hip << "," << id.toString() << endl;
        if ( id )
            mapHIPtoBF.insert ( { hip, id } );
        else
            cout << "Warning: con't convert " << strBF << " for HIP " << hip << endl;
    }

    // Report success or failure.  Return identifier map object.

    if ( linecount == mapHIPtoBF.size() )
        cout << "Success: " << filename << " linecount " << linecount << " == mapHIPtoBF.size() " << mapHIPtoBF.size() << endl;
    else
        cout << "Failure: " << filename << " linecount " << linecount << " != mapHIPtoBF.size() " << mapHIPtoBF.size() << endl;

    return mapHIPtoBF;
}

// Imports Hipparcos variable star identifier table (IDENT5.DOC).
// Returns map of GCVS identifiers indexed by HIP number,
// which should contain 6390 entries if successful.

HIPMap importHIPtoVarMap ( const char *filename )
{
    HIPMap mapHIPtoVar;

    // Open file; report error and return empty map on failure.

    ifstream file ( filename );
    if ( ! file )
    {
        cout << "Failure: can't open " << filename << endl;
        return mapHIPtoVar;
    }
    
    // Read file line-by-line until we reach end-of-file

    string line = "";
    int linecount = 0;
    
    while ( getline ( file, line ) )
    {
        linecount++;
        string strVar = trim ( line.substr ( 0, 11 ) );
        string strHIP = trim ( line.substr ( 12, 6 ) );
        int hip = strtoint ( strHIP );

        strVar = cleanHIPNameString ( strVar );
        SSIdentifier id = SSIdentifier::fromString ( strVar );
        // cout << hip << "," << id.toString() << endl;
        
        if ( id )
            mapHIPtoVar.insert ( { hip, id } );
        else
            cout << "Warning: con't convert " << strVar << " for HIP " << hip << endl;
    }

    // Report success or failure.  Return identifier map object.

    if ( linecount == mapHIPtoVar.size() )
        cout << "Success: " << filename << " linecount " << linecount << " == mapHIPtoVar.size() " << mapHIPtoVar.size() << endl;
    else
        cout << "Failure: " << filename << " linecount " << linecount << " != mapHIPtoVar.size() " << mapHIPtoVar.size() << endl;
    
    return mapHIPtoVar;
}

// Imports Hipparcos New Reduction 2007 star catalog.
// Returns map of SSStar objects indexed by Hipparcos number.
// If successful, map should contain 117955 entries.

SSStarMap importHIP2 ( const char *filename )
{
    SSStarMap mapHIP2;
    
    // Open file; report error and return empty map on failure.

    ifstream file ( filename );
    if ( ! file )
    {
        cout << "Failure: can't open " << filename << endl;
        return mapHIP2;
    }

    // Read file line-by-line until we reach end-of-file

    string line = "";
    int linecount = 0;
    
    while ( getline ( file, line ) )
    {
        linecount++;
        
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
        
        SSSpherical position ( HUGE_VAL, HUGE_VAL, HUGE_VAL );
        SSSpherical velocity ( HUGE_VAL, HUGE_VAL, HUGE_VAL );

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
            SSUpdateStarCoordsMotion ( 1991.25, nullptr, position, velocity );
        
        // Get Hipparcos magnitude
        
        float vmag = HUGE_VAL;
        if ( ! strMag.empty() )
            vmag = strtofloat ( strMag );
        
        // Get B-V color index and use it to convert Hipparcos magnitude to Johnson B and V
        
        float bmv = HUGE_VAL, bmag = HUGE_VAL;
        if ( ! strBmV.empty() )
        {
            bmv = strtofloat ( strBmV );
            vmag += -0.2964 * bmv + 0.1110 * bmv * bmv;
            bmag = vmag + bmv;
        }
        
        // If we have a parallax greater than 1 milliarcec, use it to compute distance in parsecs
        
        if ( ! strPlx.empty() )
        {
            float plx = strtofloat ( strPlx );
            if ( plx > 1.0 )
                position.rad = 1000.0 / plx;
        }
        
        // Add single Hipparcos identifier and empty name string.
        
        vector<SSIdentifier> idents ( 0 );
        vector<string> names ( 0 );

        int hip = strtoint ( strHIP );
        addIdentifier ( idents, SSIdentifier ( kCatHIP, hip ) );
        
        // Construct star and insert into map.
        
        SSStar star;
        
        star.setNames ( names );
        star.setIdentifiers ( idents );
        star.setFundamentalMotion ( position, velocity );
        star.setVMagnitude ( vmag );
        star.setBMagnitude ( bmag );

        // cout << star.toCSV() << endl;
        mapHIP2.insert ( { hip, star } );
    }
    
    // Report success or failure.  Return star map object.
    
    if ( linecount == mapHIP2.size() )
        cout << "Success: " << filename << " linecount " << linecount << " == mapHIP2.size() " << mapHIP2.size() << endl;
    else
        cout << "Failure: " << filename << " linecount " << linecount << " != mapHIP2.size() " << mapHIP2.size() << endl;

    return mapHIP2;
}
