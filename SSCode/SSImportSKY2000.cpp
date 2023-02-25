//  SSImportSKY2000.cpp
//  SSCore
//
//  Created by Tim DeBenedictis on 3/23/20.
//  Copyright © 2020 Southern Stars. All rights reserved.

#include "SSCoordinates.hpp"
#include "SSImportGCVS.hpp"
#include "SSImportSKY2000.hpp"
#include "SSImportWDS.hpp"

#include <algorithm>
#include <iostream>
#include <fstream>

// Populates identifier maps from a vector of stars.
// Call this before cross indexing.

void SSMakeObjectMaps ( SSObjectVec &stars, const vector<SSCatalog> &catalogs, SSObjectMaps &maps )
{
    for ( const SSCatalog &cat : catalogs )
        maps[cat] = SSMakeObjectMap ( stars, cat );
}

// Given a vector of identifiers (idents), returns pointer to the first star contining any of those
// identifiers from a vector of stars (stars), using the initialized identifier maps (maps) for fast
// lookups. If no star in (stars) contains any of the required identifiers (idents), returns nullptr.

SSStarPtr SSGetMatchingStar ( vector<SSIdentifier> &idents, SSObjectMaps &maps, SSObjectVec &stars )
{
    for ( auto it = maps.begin(); it != maps.end(); it++ )
    {
        SSCatalog cat = it->first;
        SSIdentifier id = SSGetIdentifier ( cat, idents );
        if ( ! id )
            continue;
        SSStarPtr pStar1 = SSGetStarPtr ( SSIdentifierToObject ( id, it->second, stars ) );
        if ( pStar1 )
            return pStar1;
    }

    return nullptr;
}

SSStarPtr SSGetMatchingStar ( SSStarPtr pStar, SSObjectMaps &maps, SSObjectVec &stars )
{
    SSIdentifierVec idents = pStar->getIdentifiers();
    return SSGetMatchingStar ( idents, maps, stars );
}

// Adds identifiers from other star catalog (stars) to a SKY2000 star (pStars).

void addSKY2000StarData ( SSObjectVec &stars, SSObjectMaps &maps, SSStarPtr pSkyStar )
{
    // Find pointer to corresponding star in other star vector,
    // using SKY2000 star's HD, BD, CD, CP identifiers.
    // Return if we don't find other corresponding star.
    
    SSStarPtr pStar = SSGetMatchingStar ( pSkyStar, maps, stars );
    if ( pStar == nullptr )
        return;
    
    // Get other star's HIP, TYC, GAIA, Bayer, and GJ identifiers.
    
    SSIdentifier hipIdent = pStar->getIdentifier ( kCatHIP );
    SSIdentifier tycIdent = pStar->getIdentifier ( kCatTYC );
    SSIdentifier gaiIdent = pStar->getIdentifier ( kCatGAIA );
    SSIdentifier bayIdent = pStar->getIdentifier ( kCatBayer );
    SSIdentifier gjIdent = pStar->getIdentifier ( kCatGJ );
    
    // If the SKY2000 star does not already have identifiers in these catalogs, add them.
    
    if ( ! pSkyStar->getIdentifier ( kCatHIP ) )
        pSkyStar->addIdentifier ( hipIdent );
    
    if ( ! pSkyStar->getIdentifier ( kCatTYC ) )
        pSkyStar->addIdentifier ( tycIdent );

    if ( ! pSkyStar->getIdentifier ( kCatGAIA ) )
        pSkyStar->addIdentifier ( gaiIdent );

    if ( ! pSkyStar->getIdentifier ( kCatBayer ) )
        pSkyStar->addIdentifier ( bayIdent );

    if ( ! pSkyStar->getIdentifier ( kCatGJ ) )
        pSkyStar->addIdentifier ( gjIdent );

    // If the SKY2000 star has no proper names, but does have a GJ identifier, add the GJ star's proper names.
    
    if ( ! pSkyStar->getNames().size() && gjIdent && pStar->getNames().size() )
        pSkyStar->setNames ( pStar->getNames() );
    
    // Copy parallax, proper motion, and radial velocity from GAIA to SKY2000 star, if present in GAIA.
    // But keep SKY2000 positions.
    
    if ( gaiIdent )
    {
        float plx = pStar->getParallax();
        if ( plx > 0.0 )
            pSkyStar->setParallax ( plx );

        SSVector vel = pStar->getFundamentalVelocity();
        if ( ! vel.isinf() )
            pSkyStar->setFundamentalVelocity ( vel );
        
        float rv = pStar->getRadVel();
        if ( ! ::isinf ( rv ) )
            pSkyStar->setRadVel ( rv );
    }
}

// Imports IAU official star name table from Working Group on Star Names
// from http://www.pas.rochester.edu/~emamajek/WGSN/IAU-CSN.txt
// Stores imported pairs of names and identifiers in nameMap.
// Returns total number of paris added to map.

int SSImportIAUStarNames ( const string &filename, SSIdentifierNameMap &nameMap )
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
        if ( line.length() < 96 )
            continue;
        
        // Extract main identifier, Hipparcos number, and name
        
        string strIdent = trim ( line.substr ( 36, 13 ) );
        string strHIP = trim ( line.substr ( 91, 6 ) );
        string strName = trim ( line.substr ( 0, 18 ) );
        
        // Construct identifier from main ident string, or HIP number if that fails.
        
        SSIdentifier ident = SSIdentifier::fromString ( strIdent );
        if ( ! ident )
        {
            int hip = strtoint ( strHIP );
            if ( hip )
                ident = SSIdentifier ( kCatHIP, hip );
        }
        
        // If successful, insert identifier and name into map; display warning on failure.
        
        if ( ! ident )
        {
            cout << "Warning: can't convert " << strIdent << " for " << strName << endl;
            continue;
        }
        
        nameMap.insert ( { ident, strName } );
        count++;
    }

    // Return fully-imported name map.  File closed automatically.
    
    return count;
}

// Converts SKY2000 integer variable star type codes to GCVS variable star type strings.
// Returns empty string if input type code is not recognized.

string SKY2000VariableTypeString ( int type )
{
    static map<int,string> vartypes =
    {
        { 111, "DCEP" },
        { 112, "CW" },
        { 113, "CEP" },
        { 114, "CEP(B)" },
        { 115, "DCEPS" },
        { 120, "RR" },
        { 121, "RRAB" },
        { 122, "RRC" },
        { 123, "RR(B)" },
        { 130, "RV" },
        { 131, "RVA" },
        { 132, "RVB" },
        { 133, "ACYG" },
        { 134, "SXPHE" },
        { 140, "BCEP" },
        { 141, "BCEPS" },
        { 150, "DSCT" },
        { 160, "ACV" },
        { 161, "ACVO" },
        { 170, "L" },
        { 171, "LB" },
        { 172, "LC" },
        { 180, "M" },
        { 181, "PVTEL" },
        { 190, "SR" },
        { 191, "SRA" },
        { 192, "SRC" },
        { 193, "SRD" },
        { 194, "SRB" },
        { 200, "IA" },
        { 201, "WR" },
        { 210, "INT" },
        { 220, "UV" },
        { 221, "UVN" },
        { 230, "RCB" },
        { 240, "UG" },
        { 241, "UGSS" },
        { 242, "UGSU" },
        { 243, "UGZ" },
        { 244, "ZAND" },
        { 245, "ZZC" },
        { 260, "N" },
        { 261, "NA" },
        { 262, "NB" },
        { 263, "NR" },
        { 264, "GCAS" },
        { 265, "SN" },
        { 266, "NC" },
        { 267, "NI" },
        { 270, "I" },
        { 271, "IN" },
        { 272, "XI" },
        { 273, "RS" },
        { 274, "IS" },
        { 275, "S" },
        { 300, "E" },
        { 310, "EA" },
        { 320, "EB" },
        { 330, "EW" },
        { 340, "ELL" },
        { 350, "SDOR" },
        { 410, "FKCOM" },
        { 420, "SXARI" },
        { 430, "ELL" },
        { 440, "BY" }
    };
    
    return vartypes[ type ];
};

// Imports SKY2000 Master Star Catalog v5:
// https://cdsarc.unistra.fr/ftp/V/145
// Adds name strings from nameNap.
// Adds additional HIP, TYC, Bayer, and GJ identifiers from vectors of
// Hipparcos stars (hipStars) and Gliese-Jahreiss nearby stars (gjStars).
// Nothing will be added if these star vectors are empty.
// Adds GCVS identifiers and variability information from vector of GCVS stars (gcvsStars);
// if this is empty, uses GCVS identifiers and variability info already present in SKY2000.
// Returns number of SKY2000 stars imported (299460 if successful).
// If a non-null filter function (filter) is provided, objects are exported
// only if they pass the filter; optional data pointer (userData) is passed
// to the filter but not used otherwise.

int SSImportSKY2000 ( const string &filename, SSIdentifierNameMap &nameMap, SSObjectVec &hipStars, SSObjectVec &gjStars, SSObjectVec &gcvsStars, SSHTM &wdsHTM, SSObjectVec &stars, SSObjectFilter filter, void *userData )
{
    // Open file; return on failure.

    ifstream file ( filename );
    if ( ! file )
        return 0;

    // Make cross-indexes of identifiers in other star vectors.
    
    SSObjectMaps hipMaps, gjMaps, gcvsMaps;
    SSMakeObjectMaps ( hipStars, { kCatHD, kCatBD, kCatCD, kCatCP }, hipMaps );
    SSMakeObjectMaps ( gjStars, { kCatHD, kCatBD, kCatCD, kCatCP }, gjMaps );
    SSMakeObjectMaps ( gcvsStars, { kCatGCVS, kCatHD, kCatBD, kCatCD, kCatCP }, gcvsMaps );
    
    // Read file line-by-line until we reach end-of-file

    string line = "";
    int numStars = 0;

    while ( getline ( file, line ) )
    {
        if ( line.length() < 521 )
            continue;
        
        string strHD = trim ( line.substr ( 35, 6 ) );
        string strSAO = trim ( line.substr ( 43, 6 ) );
        string strDM = trim ( line.substr ( 50, 5 ) ) + " " + trim ( line.substr ( 55, 5 ) );
        string strHR = trim ( line.substr ( 63, 4 ) );
        string strWDS = trim ( line.substr ( 67, 12 ) );
        
        // Extract Bayer/Flamsteed names like "21alp And", "1pi 3Ori", and "kap1Scl"; ignore AG catalog numbers.
        
        string strBay = "";
        string strNum = "";
        string strFlm = "";
        string strName = line.substr ( 98, 10 );
 
        if ( strName.find ( "AG" ) != 0 )
        {
            string strCon = trim ( line.substr ( 105, 3 ) );
            if ( ! strCon.empty() )
            {
                strFlm = trim ( line.substr ( 98, 3 ) );
                strBay = trim ( line.substr ( 101, 3 ) );
                strNum = trim ( line.substr ( 104, 1 ) );
                
                if ( ! strFlm.empty() )
                    strFlm = strFlm + " " + strCon;
                
                if ( ! strBay.empty() )
                    strBay = strBay + strNum + " " + strCon;
            }
        }
        
        // Get GCVS identifier, but reject all-numeric NSV variable star identifiers.
        
        string strVar = trim ( line.substr ( 108, 10 ) );
        if ( isNumeric ( strVar ) )
            strVar = "";
        
        // Extract RA and Dec, adding whitespace to separate hour/deg, min, sec
        
        string strRA = trim ( line.substr ( 118, 2 ) ) + " "
                     + trim ( line.substr ( 120, 2 ) ) + " "
                     + trim ( line.substr ( 122, 7 ) );
        
        string strDec = trim ( line.substr ( 129, 1 ) )
                      + trim ( line.substr ( 130, 2 ) ) + " "
                      + trim ( line.substr ( 132, 2 ) ) + " "
                      + trim ( line.substr ( 134, 6 ) );

        // Extract proper motion, removing whitepace after sign of PM in Dec.
        
        string strPMRA = trim ( line.substr ( 149, 8 ) );
        string strPMDec = trim ( line.substr ( 157, 1 ) )
                        + trim ( line.substr ( 158, 7 ) );
        
        // Extract radial velocity, removing whitespace after sign
        
        string strRV = trim ( line.substr ( 167, 1 ) )
                     + trim ( line.substr ( 168, 5 ) );
        
        // Extract parallax and standard error of parallax.
        
        string strPlx = trim ( line.substr ( 175, 8 ) );
        string strPlxErr = trim ( line.substr ( 183, 8 ) );
        
        // Extract Johnson V magnitude and B-V color index.
        // Get observed V if present; otherwise get derived V.
        
        string strMag = trim ( line.substr ( 232, 6 ) );
        if ( strMag.empty() )
            strMag = trim ( line.substr ( 238, 5 ) );
        
        string strBmV = trim ( line.substr ( 258, 6 ) );

        // Extract spectral type. Prefer full two-dimensional MK spectral type;
        // use one-dimensional HD spectral type if MK is missing. ***/
        
        string strSpec = trim ( line.substr ( 304, 30 ) );
        if ( strSpec.empty() )
            strSpec = trim ( line.substr ( 336, 3 ) );
        
        // Extract separation and magnitude difference between components,
        // position angle, year of measurement, and component identifiers.

        string strDblSep = trim ( line.substr ( 341, 7 ) );
        string strDblMag = trim ( line.substr ( 348, 5 ) );
        string strDblPA = trim ( line.substr ( 360, 3 ) );
        string strDblPAyr = trim ( line.substr ( 363, 7 ) );
        string strDblComp = trim ( line.substr ( 77, 5 ) );

        // Extract variability data: magnitude at maximum and minimum light,
        // period, epoch; convert numeric variability type code to GCVS type string.
        
        string strVarMax = trim ( line.substr ( 411, 5 ) );
        string strVarMin = trim ( line.substr ( 416, 5 ) );
        string strVarPer = trim ( line.substr ( 427, 8 ) );
        string strVarEpoch = trim ( line.substr ( 435, 8 ) );
        string strVarType = trim ( line.substr ( 443, 3 ) );
        strVarType = SKY2000VariableTypeString ( strtoint ( strVarType ) );
        
        // Get J2000 Right Ascension and Declination
        
        SSHourMinSec ra ( strRA );
        SSDegMinSec dec ( strDec );
        
        // Get J2000 proper in seconds of time per year (RA) and arcsec per year (Dec).
        
        double pmRA = INFINITY;
        if ( ! strPMRA.empty() )
            pmRA = SSAngle::fromArcsec ( strtofloat ( strPMRA ) * 15.0 );
        
        double pmDec = INFINITY;
        if ( ! strPMDec.empty() )
            pmDec = SSAngle::fromArcsec ( strtofloat ( strPMDec ) );
        
        SSSpherical position ( ra, dec, INFINITY );
        SSSpherical velocity ( pmRA, pmDec, INFINITY );
        
        // Get parallax in arcsec and convert to distance in light years.
        
        double plx = strtofloat ( strPlx );
        if ( plx > 0.0 )
            position.rad = SSCoordinates::kLYPerParsec / plx;

        // Get radial velocity in km/sec and convert to fraction of light speed (light years per year).
        
        if ( ! strRV.empty() )
            velocity.rad = strtofloat ( strRV ) / SSCoordinates::kLightKmPerSec;
        
        // Get Johnson V magnitude
        
        float vmag = INFINITY;
        if ( ! strMag.empty() )
            vmag = strtofloat ( strMag );
        
        // Get Johnson B magnitude from B-V color index
        
        float bmag = INFINITY;
        if ( ! strBmV.empty() )
            bmag = strtofloat ( strBmV ) + vmag;
        
        // If parallax is unknown, compute distance in light years from spectral class and magnitudes
        
        if ( isinf ( position.rad ) )
            position.rad = SSCoordinates::kLYPerParsec * SSStar::spectralDistance ( strSpec, vmag, bmag );
        
        // Set up name and identifier vectors.

        vector<SSIdentifier> idents ( 0 );
        vector<string> names ( 0 );
        
        if ( ! strBay.empty() )
            SSAddIdentifier ( SSIdentifier::fromString ( strBay ), idents );
        
        if ( ! strFlm.empty() )
            SSAddIdentifier ( SSIdentifier::fromString ( strFlm ), idents );
        
        if ( ! strHR.empty() )
            SSAddIdentifier ( SSIdentifier ( kCatHR, strtoint ( strHR ) ), idents );

        if ( ! strHD.empty() )
            SSAddIdentifier ( SSIdentifier ( kCatHD, strtoint ( strHD ) ), idents );
        
        if ( ! strSAO.empty() )
            SSAddIdentifier ( SSIdentifier ( kCatSAO, strtoint ( strSAO ) ), idents );

        if ( ! strDM.empty() )
            SSAddIdentifier ( SSIdentifier::fromString ( strDM ), idents );
        
        if ( ! strWDS.empty() )
            SSAddIdentifier ( SSIdentifier::fromString ( "WDS " + strWDS ), idents );
        
        // SKY2000 is missing four HR stars; all of them are double star secondary components.
        // This adds those missing HR identifiers to their four SKY2000 primary stars.

        if ( ! strHR.empty() )
        {
            static map<int,int> missingHR = // map from SKY2000 HR to missing HR
            { { 927, 928 }, { 4375, 4374 }, { 4968, 4969 }, { 5978, 5977 } };
            int hr = missingHR[ strtoint ( strHR ) ];
            if ( hr )
                SSAddIdentifier ( SSIdentifier ( kCatHR, hr ), idents );
        }

        // Get GCVS identifier string from SKY2000.
        
        SSIdentifier gcvsIdent = SSIdentifier::fromString ( strVar );
        if ( gcvsIdent.catalog() == kCatGCVS )
            SSAddIdentifier ( gcvsIdent, idents );

        // Look for a GCVS star with the same HD/BD/CD/CP/GCVS identifier as our SKY2000 star.
        // If we find one, add the GCVS star identifier to the SKY2000 star identifiers.

        SSVariableStarPtr pGCVStar = SSGetVariableStarPtr ( SSGetMatchingStar ( idents, gcvsMaps, gcvsStars ) );
        if ( pGCVStar )
            SSAddIdentifier ( pGCVStar->getIdentifier ( kCatGCVS ), idents );

        // If this star has a double star component string, look for a matching WDS star and get its primary component.
        
        char primComp = 0;
        SSDoubleStarPtr pWDStar = nullptr;
        if ( strDblComp.length() > 0 && wdsHTM.countRegions() > 0 )
            pWDStar = SSFindWDSStar ( wdsHTM, SSSpherical ( ra, dec, 1.0 ), strDblComp[0], primComp, strtofloat ( strDblSep ) );
        
        // Get name string(s) corresponding to identifier(s).
        // Construct star and insert into star vector.
        
        names = SSIdentifiersToNames ( idents, nameMap );

        bool isVar = pGCVStar != nullptr || (int64_t) gcvsIdent > 0;
        bool isDbl = ! strWDS.empty();

        SSObjectType type = kTypeNonexistent;

        if ( isDbl && isVar )
            type = kTypeDoubleVariableStar;
        else if ( isDbl )
            type = kTypeDoubleStar;
        else if ( isVar )
            type = kTypeVariableStar;
        else
            type = kTypeStar;

        SSObjectPtr pObj = SSNewObject ( type );
        SSStarPtr pStar = SSGetStarPtr ( pObj );
        if ( pStar == nullptr )
            continue;
        
        pStar->setNames ( names );
        pStar->setIdentifiers ( idents );
        pStar->setFundamentalMotion ( position, velocity );
        pStar->setVMagnitude ( vmag );
        pStar->setBMagnitude ( bmag );
        pStar->setSpectralType ( strSpec );

        // Add additional HIP, Bayer, and GJ identifiers from other catalogs.
        // Sert star's identifier vector.
        
        addSKY2000StarData ( hipStars, hipMaps, pStar );
        addSKY2000StarData ( gjStars, gjMaps, pStar );
        pStar->sortIdentifiers();
        
        SSVariableStarPtr pVar = SSGetVariableStarPtr ( pObj );
        if ( pVar != nullptr )
        {
            // If we have a matching star from the GCVS, copy its variability data;
            // otherwise use variability data already present in SKY2000.
            
            if ( pGCVStar )
            {
                SSCopyVariableStarData ( pGCVStar, pVar );
            }
            else
            {
                // Minimum magnitude is magnitude at maximum light, and vice-vera!
                
                if ( ! strVarMin.empty() )
                    pVar->setMinimumMagnitude ( strtofloat ( strVarMax ) );

                if ( ! strVarMax.empty() )
                    pVar->setMaximumMagnitude ( strtofloat ( strVarMin ) );

                // Get variability period in days and convert epoch to Julian Date.
                
                if ( ! strVarPer.empty() )
                    pVar->setPeriod ( strtofloat ( strVarPer ) );

                if ( ! strVarEpoch.empty() )
                    pVar->setEpoch ( strtofloat ( strVarEpoch ) + 2400000.0 );

                // Store variability type
                
                if ( ! strVarType.empty() )
                    pVar->setVariableType ( strVarType );
            }
        }
        
        SSDoubleStarPtr pDbl = SSGetDoubleStarPtr ( pObj );
        if ( pDbl != nullptr )
        {
            // If we have a matching WDS star, copy orbit and double star info from WDS star;
            // otherwise use double star info from SKY2000.
            
            if ( pWDStar )
            {
                SSCopyDoubleStarData ( pWDStar, strDblComp[0], primComp, pDbl );
            }
            else
            {
                if ( ! strDblComp.empty() )
                    pDbl->setComponents ( strDblComp );
                
                if ( ! strDblMag.empty() )
                    pDbl->setMagnitudeDelta ( strtofloat ( strDblMag ) );
                
                if ( ! strDblSep.empty() )
                    pDbl->setSeparation ( SSAngle::fromArcsec ( strtofloat ( strDblSep ) ) );
                
                if ( ! strDblPA.empty() )
                    pDbl->setPositionAngle( SSAngle::fromDegrees ( strtofloat ( strDblPA ) ) );
                
                if ( ! strDblPAyr.empty() )
                    pDbl->setPositionAngleYear ( strtofloat ( strDblPAyr ) );
            }
        }
        
        // cout << pStar->toCSV() << endl;
        if ( filter == nullptr || filter ( pObj, userData ) )
        {
            stars.append ( pObj );
            numStars++;
        }
    }
    
    // Return imported star count; file is closed automatically.
    
    return numStars;
}

// Merges Hipparcos-Tycho catalog (hipStars) into SKY2000 catalog (skyStars) using HD identifiers as cross-match.
// On input, hipStars should contain Hipparcos and Tycho stars; and skyStars should contain SKY2000 stars.
// On return, hipStars will be empty, and skyStars will contain merged catalog with Hipparcos/Tycho stars appended.
// Returns total number of stars in merged catalog.

int SSMergeHIPTYCtoSKY2000 ( SSObjectVec &hipStars, SSObjectVec &skyStars )
{
    SSObjectMaps skyMaps;
    SSMakeObjectMaps ( skyStars, { kCatHR, kCatHD, kCatSAO, kCatBD, kCatCD, kCatCP, kCatHIP, kCatTYC }, skyMaps );
    
    // For each Hipparcos/Tycho star, search for a SKY2000 star with the same HR/HD/SAO/BD/CD/CP/HIP/TYC identifiers.
    // If we don't find one, append the Hipparcos/Tycho star to the SKY2000 star vector and remove it
    // from the HIP/TYC star vector (but don't delete it!)
    
    for ( int i = 0; i < hipStars.size(); i++ )
    {
        SSStarPtr pHipStar = SSGetStarPtr ( hipStars.get ( i ) );
        SSStarPtr pSkyStar = SSGetMatchingStar ( pHipStar, skyMaps, skyStars);
        if ( pSkyStar == nullptr )
        {
            if ( pHipStar->getVMagnitude() < 8.0 )
                cout << pHipStar->toCSV() << endl;
            
            skyStars.append ( pHipStar );
            hipStars.set ( i, nullptr );
        }
    }
    
    // Delete remaining Hip/Tyc star array to prevent double-deletes.
    
    hipStars.erase();
    return (int) skyStars.size();
}

// Merges SKY2000/HIP/TYC stars (skyStars) and nearby stars vector (nearStars), avoiding duplicates.
// On input, nearStars contains all stars closer than 10 parsecs; skyStars contains all stars.
// On return, skyStars will be empty, and nearStars will contain merged catalog with other stars appended.
// Returns total number of stars in merged catalog.

int SSMergeNearbyStars ( SSObjectVec &skyStars, SSObjectVec &nearStars )
{
    SSObjectMaps nearMaps;
    SSMakeObjectMaps ( nearStars, { kCatGJ, kCatHIP, kCatHD, kCatTYC, kCatGAIA }, nearMaps );
    
    for ( int i = 0; i < skyStars.size(); i++ )
    {
        SSStarPtr pSkyStar = SSGetStarPtr ( skyStars.get ( i ) );
        SSStarPtr pNearStar = SSGetMatchingStar ( pSkyStar, nearMaps, nearStars );
        if ( pNearStar == nullptr )
        {
            nearStars.append ( pSkyStar );
            skyStars.set ( i, nullptr );
        }
    }
    
    skyStars.erase();
    return (int) nearStars.size();
}
