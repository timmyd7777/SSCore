// SSImportGCVS.cpp
// SSCore
//
// Created by Tim DeBenedictis on 4/26/21.
// Copyright © 2021 Southern Stars. All rights reserved.

#include <algorithm>
#include <iostream>
#include <fstream>
#include <map>

#include "SSImportGCVS.hpp"
#include "SSStar.hpp"

// Converts GCVS star catalog identifier strings to numerical
// GCVS or Bayer identifiers. Returns zero if conversion fails.

static SSIdentifier GCVSIdentifier ( const string &strGCVS )
{
    static map<string,string> letters =
    {
        { "alf", "alpha" },
        { "bet", "beta" },
        { "gam", "gamma" },
        { "del", "delta" },
        { "eps", "epsilon" },
        { "zet", "zeta" },
        { "eta", "eta" },
        { "tet", "theta" },
        { "iot", "iota" },
        { "kap", "kappa" },
        { "lam", "lambda" },
        { "mu.", "mu" },
        { "nu.", "nu" },
        { "ksi", "xi" },
        { "omi", "omicron" },
        { "pi.", "pi" },
        { "rho", "rho" },
        { "sig", "sigma" },
        { "tau", "tau" },
        { "ups", "upsilon" },
        { "khi", "chi" },
        { "psi", "psi" },
        { "ome", "omega" }
    };

    SSIdentifier gcvs;

    // Convert GCVS Bayer abbreviations to full Bayer letters.
    // Convert GCVS strings to GCVS and/or Bayer identifiers.
    
    string bayer = letters[ strGCVS.substr ( 0, 3 ) ];
    if ( bayer.empty() )
        gcvs = SSIdentifier::fromString ( strGCVS, kTypeVariableStar, true );
    else    // for Bayer abbreviations, eliminate whitespace after abbreviation to allow parsing superscript.
        gcvs = SSIdentifier::fromString ( bayer + strGCVS.substr ( 4, 7 ), kTypeStar, true );

    return gcvs;
}

// Imports GCVS cross-identifier table (crossid.txt).
// Inserts results into map of catalog identifiers, indexed by GCVS identifier (identmap),
// and returns number of identifiers inserted into map.

int SSImportGCVSCrossIdentifiers ( const string &filename, SSIdentifierMap &identmap )
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
        if ( line.length() < 47 )
            continue;
        
        string strGCVS = line.substr ( 36, 11 );
        string strIdent = line.substr ( 5, 9 );

        SSIdentifier ident;
        if ( line.compare ( 0, 2, "BS" ) == 0 )
            ident = SSIdentifier::fromString ( "HR " + strIdent, kTypeStar, true );

        if ( line.compare ( 0, 2, "BD" ) == 0 )
            ident = SSIdentifier::fromString ( "BD " + strIdent, kTypeStar, true );
        
        if ( line.compare ( 0, 3, "CPD" ) == 0 )
            ident = SSIdentifier::fromString ( "CP " + strIdent, kTypeStar, true );

        if ( line.compare ( 0, 3, "CoD" ) == 0 )
            ident = SSIdentifier::fromString ( "CD " + strIdent, kTypeStar, true );

        if ( line.compare ( 0, 3, "FLM" ) == 0 )
            ident = SSIdentifier::fromString ( strIdent.substr ( 4, 3 ) + " " + strIdent.substr ( 0, 3 ), kTypeStar, true );
        
        if ( line.compare ( 0, 2, "Gl" ) == 0 )
            ident = SSIdentifier::fromString ( "Gl " + strIdent + line[31], kTypeStar, true );

        if ( line.compare ( 0, 2, "HD" ) == 0 )
            ident = SSIdentifier::fromString ( "HD " + strIdent, kTypeStar, true );

        if ( line.compare ( 0, 3, "Hip" ) == 0 )
            ident = SSIdentifier::fromString ( "HIP " + strIdent, kTypeStar, true );

        if ( line.compare ( 0, 3, "SAO" ) == 0 )
            ident = SSIdentifier::fromString ( "SAO " + strIdent, kTypeStar, true );

        SSIdentifier gcvs = GCVSIdentifier ( strGCVS );
        if ( gcvs == 0 || ident == 0 )
            continue;
        
        // cout << gcvs.toString() << "," << ident.toString() << "," << endl;
        identmap.insert ( { gcvs, ident } );
        count++;
    }
    
    // Return count of identifiers added.  File is closed automatically.

    return count;
}

// Imports the main General Catalog of Variable Stars, version 5 (gcvs5.txt) from http://www.sai.msu.su/gcvs/gcvs/
// Adds HR, GJ, Flamsteed, HD, DM, SAO identifiers from cross identification table (identmap).
// Stores results in vector of SSObjects (stars).
// Returns number of objects imported (57241 if successful).

int SSImportGCVS ( const string &filename, SSIdentifierMap &identmap, SSObjectArray &stars )
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
        if ( line.length() < 236 )
            continue;
        
        string strGCVS = trim ( line.substr ( 8, 10 ) );
        string strRAh = trim ( line.substr ( 20, 2 ) );
        string strRAm = trim ( line.substr ( 22, 2 ) );
        string strRAs = trim ( line.substr ( 24, 5 ) );
        string strDecd = trim ( line.substr ( 30, 3 ) );
        string strDecm = trim ( line.substr ( 33, 2 ) );
        string strDecs = trim ( line.substr ( 35, 4 ) );
        string strPMRA = trim ( line.substr ( 179, 6 ) );
        string strPMDec = trim ( line.substr ( 186, 6 ) );
        string strMagMax = trim ( line.substr ( 53, 6 ) );  // here maximum light = smallest magnitude = brightest
        string strMagMin = trim ( line.substr ( 64, 6 ) );  // here minimum light = largest magnitude = faintest
        string strEpoch = trim ( line.substr ( 91, 11 ) );
        string strPeriod = trim ( line.substr ( 111, 16 ) );
        string strSpec = trim ( line.substr ( 137, 17 ) );
        string strVarType = trim ( line.substr ( 41, 10 ) );

        if ( strRAh.empty() || strDecd.empty() )
            continue;
        
        // Get right ascension and declination and convert to radians.

        SSHourMinSec ra  ( strRAh + " " + strRAm + " " + strRAs );
        SSDegMinSec  dec ( strDecd + " " + strDecm + " " + strDecs );
        SSSpherical coords ( SSAngle ( ra ), SSAngle ( dec ), INFINITY );

        // Get proper motion in right ascension and declination and convert to radians/year.

        double pmRA = strPMRA.empty() ? INFINITY : strtofloat64 ( strPMRA ) / cos ( coords.lat );
        double pmDec = strPMDec.empty() ? INFINITY : strtofloat64 ( strPMDec );
        SSSpherical motion ( SSAngle::fromArcsec ( pmRA ), SSAngle::fromArcsec ( pmDec ), INFINITY );
        
        // Get magnitude at maximum light (numerically smaller)
        // Get magnitude at minimum light (numerically larger). If an amplitude, convert to magnitude.

        float magMax = strMagMax.empty() ? INFINITY : strtofloat ( strMagMax );
        float magMin = strMagMin.empty() ? INFINITY : strtofloat ( strMagMin );
        if ( line[62] == '(' )
            magMin = magMax + magMin;
        
        // Get photometric band for magnitude measurements.
        // Use magnitude at minimum light (faintest, numerically larger) for visual magnitude if available, otherwise use magnitude at maximum.
        // Get period and epoch.

        char band = line[88];
        float mag = isinf ( magMin ) ? magMax : magMin;
        double epoch = strEpoch.empty() ? INFINITY : strtofloat64 ( strEpoch ) + 2400000.0;
        double period = strPeriod.empty() ? INFINITY : strtofloat64 ( strPeriod );
        
        // Get GCVS identifier. If we fail, assume the record is invalid.
        // Add alternate catalog idents from the cross-identification table.
        // Sert identifier vector.

        vector <SSIdentifier> idents;
        SSIdentifier gcvs = GCVSIdentifier ( strGCVS );
        if ( gcvs == 0 )
            continue;
        
        idents.push_back ( gcvs );
        SSAddIdentifiers ( gcvs, identmap, idents );
        sort ( idents.begin(), idents.end(), compareSSIdentifiers );

        // Construct star and insert into star vector.

        SSObjectPtr pObj = SSNewObject ( kTypeVariableStar );
        SSVariableStarPtr pStar = SSGetVariableStarPtr ( pObj );
        if ( pStar != nullptr )
        {
            pStar->setIdentifiers ( idents );
            pStar->setFundamentalMotion ( coords, motion );
            
            // If photometric band is visual, Hipparcos, or Tycho, assign minimum magnitude to visual.
            // If photometric band is blue, photographic, or ultraviolet, assign minimum magnitude to blue.
            // Don't assign any H,J,K,L,M band magnitudes - these are far-IR and come out erroneously bright.
             
            if ( band == 'V' || band == 'v' || ( band == 'H' && line[89] == 'p' ) || band == 'T' )
                pStar->setVMagnitude ( mag );
            else if ( band == 'B' || band == 'b' || band == 'p' || band == 'U' || band == 'u' )
                pStar->setBMagnitude ( mag );
            else if ( band != 'H' && band != 'J' && band != 'K' && band != 'L' && band != 'M' )
                pStar->setVMagnitude ( mag );

            pStar->setEpoch ( epoch );
            pStar->setPeriod ( period );
            pStar->setSpectralType ( strSpec );
            pStar->setVariableType ( strVarType );
            pStar->setMaximumMagnitude ( magMin );  // here maximum magnitude = minimum light = faintest
            pStar->setMinimumMagnitude ( magMax );  // here minimum magnitude = maximum light = brightest

            // cout << pStar->toCSV() << endl;
            stars.append ( pObj );
            numStars++;
        }
    }

    // Return imported star count; file is closed automatically.
    
    return numStars;
}

// Copies variable star data from GCVS star (pWDStar) into target star (pStar).
// Returns true if successful or false on failure.

bool SSCopyVariableStarData ( SSVariableStarPtr pGCVStar, SSStarPtr pStar )
{
    SSVariableStarPtr pVar = SSGetVariableStarPtr ( pStar );
    if ( pGCVStar && pVar )
    {
        pVar->setMinimumMagnitude ( pGCVStar->getMinimumMagnitude() );
        pVar->setMaximumMagnitude ( pGCVStar->getMaximumMagnitude() );
        pVar->setPeriod ( pGCVStar->getPeriod() );
        pVar->setEpoch ( pGCVStar->getEpoch() );
        pVar->setVariableType ( pGCVStar->getVariableType() );
        return true;
    }
    return false;
}

