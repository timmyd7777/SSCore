// SSImportTYC.cpp
// SSCore
//
// Created by Tim DeBenedictis on 6/1/21.
// Copyright ©2021 Southern Stars Group, LLC. All rights reserved.

#include <algorithm>
#include <iostream>
#include <fstream>

#include "SSImportGCVS.hpp"
#include "SSImportHIP.hpp"
#include "SSImportTYC.hpp"
#include "SSImportSKY2000.hpp"

// Converts Tycho B (bt) and V (vt) magnitudes to Johnson B (bj) and V (vj) magnitudes

void TychoToJohnsonMagnitude ( float bt, float vt, float &bj, float &vj )
{
    float ct = bt - vt;
    vj = vt - ct * 0.090;
    bj = vj + 0.85 * ct;
}

// Imports the main Tycho-1 catalog from a local file (filename) into the SSObject array (stars).
// Henry Draper catalog identifiers and spectral types are inserted from the Tycho-2 HD Identifications (tyc2hdmap).
// Returns the total number of stars imported (1058332 if successful).
// If stars vector alreadt cotains Hipparcos star catalog on input, Tycho-1 stars
// not already in Hipparcos will be appended, and existing Hipparcos stars will get
// TYC identifiers from Tycho-1.

int SSImportTYC ( const string &filename, TYC2HDMap &tyc2hdmap, SSObjectVec &gcvsStars, SSObjectVec &stars )
{
    // Open file; return on failure.

    ifstream file ( filename );
    if ( ! file )
        return 0;
    
    SSObjectMaps gcvsMaps;
    SSMakeObjectMaps ( gcvsStars, { kCatHD, kCatBD, kCatCD, kCatCP, kCatGCVS }, gcvsMaps );
    SSObjectMap hipMap = SSMakeObjectMap ( stars, kCatHIP );
    
    // Read file line-by-line until we reach end-of-file

    string line = "";
    int numStars = 0;

    while ( getline ( file, line ) )
    {
        string strTYC = trim ( line.substr ( 2, 12 ) );
        string strRA = trim ( line.substr ( 51, 12 ) );
        string strDec = trim ( line.substr ( 64, 12 ) );
        string strPMRA = trim ( line.substr ( 87, 8 ) );
        string strPMDec = trim ( line.substr ( 96, 8 ) );
        string strMag = trim ( line.substr ( 41, 5 ) );
        string strBmV = trim ( line.substr ( 245, 6 ) );
        string strMax = trim ( line.substr ( 279, 1 ) );
        string strMin = trim ( line.substr ( 285, 5 ) );
        string strPlx = trim ( line.substr ( 79, 7 ) );
        string strHIP = trim ( line.substr ( 210, 6 ) );
        string strHD = trim ( line.substr ( 309, 6 ) );
        string strBD = trim ( line.substr ( 317, 9 ) );
        string strCD = trim ( line.substr ( 328, 9 ) );
        string strCP = trim ( line.substr ( 339, 9 ) );

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

        // Get Tycho V magnitude; get Tycho B magnitude from B-V color index.
        // If both are presemt, convert Tycho magnitudes to Johnson scale.

        float vmag = strMag.empty() ? INFINITY : strtofloat ( strMag );
        float bmag = strBmV.empty() ? INFINITY : strtofloat ( strBmV ) + vmag;
        
        if ( ! isinf ( vmag ) && ! isinf ( bmag ) )
            TychoToJohnsonMagnitude ( bmag, vmag, bmag, vmag );
        
        // Get the magnitude range.

        float maxMag = strMax.empty() ? INFINITY : strtofloat ( strMax );
        float minMag = strMin.empty() ? INFINITY : strtofloat ( strMin );

        // If we have a positive parallax, use it to compute distance in light years.
        // Don't believe any parallaxes larger than 100 milliarcsec.
        
        float plx = strPlx.empty() ? 0.0 : strtofloat ( strPlx );
        if ( plx > 0.0 && plx < 100.0 )
            position.rad = 1000.0 * SSCoordinates::kLYPerParsec / plx;

        vector<SSIdentifier> idents ( 0 );
        vector<string> names ( 0 );
        
        if ( ! strHIP.empty() )
            SSAddIdentifier ( SSIdentifier ( kCatHIP, strtoint ( strHIP ) ), idents );

        if ( ! strHD.empty() )
            SSAddIdentifier ( SSIdentifier ( kCatHD, strtoint ( strHD ) ), idents );

        if ( ! strBD.empty() )
            SSAddIdentifier ( SSIdentifier::fromString ( "BD " + strBD ), idents );
        
        if ( ! strCD.empty() )
            SSAddIdentifier ( SSIdentifier::fromString ( "CD " + strCD ), idents );

        if ( ! strCP.empty() )
            SSAddIdentifier ( SSIdentifier::fromString ( "CP " + strCP ), idents );

        SSIdentifier tyc = SSIdentifier::fromString ( "TYC " + strTYC );
        if ( tyc )
            SSAddIdentifier ( tyc, idents );

        // Look for a GCVS star with the same HD/BD/CD/CP identifier as our Tycho star.
        // If we find one, add the GCVS star identifier to the Tycho star identifiers.

        SSIdentifier gcvs = 0;
        SSVariableStarPtr pGCVStar = SSGetVariableStarPtr ( SSGetMatchingStar ( idents, gcvsMaps, gcvsStars ) );
        if ( pGCVStar )
        {
            gcvs = pGCVStar->getIdentifier ( kCatGCVS );
            SSAddIdentifier ( gcvs, idents );
        }
        
        // If this is a Hipparcos star, find corresponding Hipparcos star.
        // Copy TYC indetifier from Tycho-1 star if valid into HIP star
        // but do not append a new star to the star vector.
        
        if ( ! strHIP.empty() )
        {
            SSIdentifier hip = SSIdentifier ( kCatHIP, strtoint ( strHIP ) );
            SSStarPtr pStar1 = SSGetStarPtr ( SSIdentifierToObject ( hip, hipMap, stars ) );
            if ( pStar1 )
            {
                pStar1->addIdentifier ( tyc );
                numStars++;
                continue;
            }
        }
        
        // Otherwise, add a new star to the Tycho star vector
        // Sert identifier vector.  Get name string(s) corresponding to identifier(s).
        // Construct star and insert into star vector.

        sort ( idents.begin(), idents.end(), compareSSIdentifiers );
        SSObjectType type = pGCVStar ? kTypeVariableStar : kTypeStar;

        SSObjectPtr pObj = SSNewObject ( type );
        SSStarPtr pStar = SSGetStarPtr ( pObj );

        if ( pStar != nullptr )
        {
            pStar->setNames ( names );
            pStar->setIdentifiers ( idents );
            pStar->setFundamentalMotion ( position, velocity );
            pStar->setVMagnitude ( vmag );
            pStar->setBMagnitude ( bmag );

            TYC2HDMap::iterator it = tyc2hdmap.find ( tyc );
            if ( it != tyc2hdmap.end() )
                pStar->setSpectralType( it->second.spectrum );
            
            // cout << pStar->toCSV() << endl;
            stars.append ( pObj );
            numStars++;
        }
        
        // If we have a matching star from the GCVS, copy its variability data.
        
        if ( pGCVStar )
            SSCopyVariableStarData( pGCVStar, pStar );
    }

    // Return imported star count; file is closed automatically.
    
    return numStars;
}

// Imports the Tycho-2 catalog: https://cdsarc.unistra.fr/ftp/I/259/
// into a vector of SSObjects (stars). Henry Draper identifiers and spectral
// types are inserted from the Tycho-2 HD Identifications (tyc2hdmap).
// If stars vector already cotains Tycho-1 star catalog on input, Tycho-2 stars
// not already in Tycho-1 will be appended, and existing Tycho-1 stars will get
// updated position, motion, and magnitude data from Tycho-2.
// Returns the total number of stars imported (2539913 if successful).

int SSImportTYC2 ( const string &filename, TYC2HDMap &tyc2hdmap, SSObjectVec &stars )
{
    // Open file; return on failure.

    ifstream file ( filename );
    if ( ! file )
        return 0;
    
    SSObjectMap hipMap = SSMakeObjectMap ( stars, kCatHIP );
    SSObjectMap tycMap = SSMakeObjectMap ( stars, kCatTYC );
    
    // Read file line-by-line until we reach end-of-file

    string line = "";
    int numStars = 0;

    while ( getline ( file, line ) )
    {
        string strTYC = trim ( line.substr ( 0, 12 ) );
        string strRA = trim ( line.substr ( 15, 12 ) );
        string strDec = trim ( line.substr ( 28, 12 ) );
        string strPMRA = trim ( line.substr ( 41, 7 ) );
        string strPMDec = trim ( line.substr ( 49, 7 ) );
        string strVMag = trim ( line.substr ( 123, 6 ) );
        string strBMag = trim ( line.substr ( 110, 6 ) );
        string strTYC1 = trim ( line.substr ( 140, 1 ) );
        string strHIP = trim ( line.substr ( 142, 6 ) );
        string strCCDM = trim ( line.substr ( 148, 3 ) );
        
        // Use the Epoch 2000.0 R.A. and Dec. where possible; if not, use the observed epoch R.A. and Dec.
        // The observed epoch is always within 2.4 years of 1990; this is close enough to 2000 that we won't quibble,
        // and Tycho-2 stars without J2000 RA/Dec have no proper motion, so we can't update their RA/Dec to 2000, anyhow.
        
        string strRAEp;
        if ( strRA.empty() )
            strRA = trim ( line.substr ( 152, 12 ) );
        
        string strMEpDec;
        if ( strDec.empty() )
            strDec = trim ( line.substr ( 165, 12 ) );
        
        SSSpherical position ( INFINITY, INFINITY, INFINITY );
        SSSpherical velocity ( INFINITY, INFINITY, INFINITY );
        
        // Get right ascension and convert to radians
        
        position.lon = SSAngle::fromDegrees ( strtofloat ( strRA ) );
        position.lat = SSAngle::fromDegrees ( strtofloat ( strDec ) );
        
        // Get proper motion in RA and convert to radians per year
        
        if ( ! strPMRA.empty() )
            velocity.lon = SSAngle::fromArcsec ( strtofloat ( strPMRA ) / 1000.0 ) / cos ( position.lat );
        
        // Get proper motion in Dec and convert to radians per year
        
        if ( ! strPMDec.empty() )
            velocity.lat = SSAngle::fromArcsec ( strtofloat ( strPMDec ) / 1000.0 );
        
        // Get Tycho V magnitude; get Tycho B magnitude from B-V color index.
        // If both are presemt, convert Tycho magnitudes to Johnson scale.

        float vmag = strVMag.empty() ? INFINITY : strtofloat ( strVMag );
        float bmag = strBMag.empty() ? INFINITY : strtofloat ( strBMag );
        
        if ( ! isinf ( vmag ) && ! isinf ( bmag ) )
            TychoToJohnsonMagnitude ( bmag, vmag, bmag, vmag );
        
        vector<SSIdentifier> idents ( 0 );
        vector<string> names ( 0 );
        
        // Get hipparcos and tycho identifiers
        
        SSIdentifier hip;
        if ( ! strHIP.empty() )
            hip = SSIdentifier ( kCatHIP, strtoint ( strHIP ) );
        if ( hip )
            SSAddIdentifier ( hip, idents );

        SSIdentifier tyc = SSIdentifier::fromString ( "TYC " + strTYC );
        if ( tyc )
            SSAddIdentifier ( tyc, idents );

        // Get HD identifier from TYC2-HD cross index
        
        TYC2HDMap::iterator it = tyc2hdmap.find ( tyc );
        if ( it != tyc2hdmap.end() )
            idents.push_back ( it->second.hd );
                
        // Is this a Tycho-1 star?
        
        bool tyc1 = false;
        if ( strTYC1.length() > 0 && strTYC1[0] == 'T' )
            tyc1 = true;
        
        // If this is a Tycho-1 star, find corresponding Tycho-1 star.
        // Copy position, motion, magnitudes from Tycho-2.
        // If this is a Hipparcos star, don't overwite Hipparcos star data.

        if ( tyc && tyc1 )
        {
            SSStarPtr pStar1 = SSGetStarPtr ( SSIdentifierToObject ( tyc, tycMap, stars ) );
            if ( pStar1 )
            {
                if ( ! hip )
                {
                    float plx = pStar1->getParallax();
                    if ( plx > 0.0 )
                        position.rad = SSCoordinates::kLYPerParsec / plx;
                    
                    pStar1->setFundamentalMotion ( position, velocity );
                    pStar1->setVMagnitude ( vmag );
                    pStar1->setBMagnitude ( bmag );
                }
                numStars++;
                continue;
            }
        }
        
        // Otherwise, add a new Tycho-2 star to the Tycho star vector
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

            if ( it != tyc2hdmap.end() )
                pStar->setSpectralType ( it->second.spectrum );

            stars.append ( pObj );
            numStars++;
        }
    }

    // Return imported star count; file is closed automatically.
    
    return numStars;
}

// Imports Henry Draper Catalogue identifications for Tycho-2 stars: https://cdsarc.unistra.fr/ftp/IV/25/
// into map indexed by Tycho catalog number. The local path (filename) is the tyc2_hd.dat file.
// Returns number of identifications imported (353527 if successful).

int SSImportTYC2HD ( const string &filename, TYC2HDMap &tyc2hdmap )
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
        string strTYC = trim ( line.substr ( 0, 12 ) );
        string strHD = trim ( line.substr ( 14, 6 ) );
        string strSpec = trim ( line.substr ( 21, 3 ) );

        if ( strTYC.empty() || strHD.empty() )
            continue;
        
        SSIdentifier tyc = SSIdentifier::fromString ( "TYC " + strTYC );
        if ( tyc == 0 )
            continue;
        
        SSIdentifier hd = SSIdentifier ( kCatHD, strtoint ( strHD ) );
        if ( hd == 0 )
            continue;
        
        TYC2HD tyc2hd = { tyc, hd, strSpec };
        tyc2hdmap.insert ( { tyc, tyc2hd } );
        numStars++;
    }

    // Return imported star count; file is closed automatically.
    
    return numStars;
}
