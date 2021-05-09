//  SSImportSKY2000.cpp
//  SSCore
//
//  Created by Tim DeBenedictis on 3/23/20.
//  Copyright © 2020 Southern Stars. All rights reserved.

#include "SSCoordinates.hpp"
#include "SSImportSKY2000.hpp"

#include <algorithm>
#include <iostream>
#include <fstream>

// Searches for double star in double star HTM (wdsHTM)
// within 1 arcminute of target coordinates (coords)
// matching a component character A, B, C, D, etc. (comp)
// with an angular separation in arcseconds (sep); will be ignored if zero.
// If found, returns primary component character (prim), and
// returns pointer to matching double star, or nullptr if none.

SSDoubleStarPtr getWDStar ( SSHTM &wdsHTM, SSSpherical coords, char comp, char &prim, float sep )
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

// Adds identifiers from other star catalog (stars) to a SKY2000 star (pStars).

void addSKY2000StarData ( SSObjectVec &stars, SSObjectMap &map, SSStarPtr pSkyStar )
{
    // Find pointer to corresponding star in other star vector,
    // using SKY2000 star's HD identifier.
    
    SSIdentifier ident = pSkyStar->getIdentifier ( kCatHD );
    SSStarPtr pStar = SSGetStarPtr ( SSIdentifierToObject ( ident, map, stars ) );
    
    // Continue if we don't find other corresponding star.
    
    if ( pStar == nullptr )
        return;
    
    // Get other star's HIP, Bayer, and GJ identifiers.
    
    SSIdentifier hipIdent = pStar->getIdentifier ( kCatHIP );
    SSIdentifier bayIdent = pStar->getIdentifier ( kCatBayer );
    SSIdentifier gjIdent = pStar->getIdentifier ( kCatGJ );
    
    // If the SKY2000 star does not already have identifiers in these catalogs, add them.
    
    if ( ! pSkyStar->getIdentifier ( kCatHIP ) )
        pSkyStar->addIdentifier ( hipIdent );
    
    if ( ! pSkyStar->getIdentifier ( kCatBayer ) )
        pSkyStar->addIdentifier ( bayIdent );

    if ( ! pSkyStar->getIdentifier ( kCatGJ ) )
        pSkyStar->addIdentifier ( gjIdent );

    // If the SKY2000 star has no proper names, but does have a GJ identifier, add the GJ star's proper names.
    
    if ( ! pSkyStar->getNames().size() && gjIdent && pStar->getNames().size() )
        pSkyStar->setNames ( pStar->getNames() );
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
// Adds additional HIP, Bayer, and GJ identifiers from vectors of
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

    // Make index of HD catalog numbers in the Hipparcos, GJ, GCVS star vectors.
    
    SSObjectMap hipMap = SSMakeObjectMap ( hipStars, kCatHD );
    SSObjectMap gjMap = SSMakeObjectMap ( gjStars, kCatHD );
    SSObjectMap gcvsMap = SSMakeObjectMap ( gcvsStars, kCatHD );
    
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
        
        // Get parallax in arcsec and if > 1 mas convert to distance in light years.
        
        double plx = strtofloat ( strPlx );
        if ( plx > 0.001 )
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

        // Look for a GCVS star with the same HD number as our SKY2000 star.
        // If we find one, add the GCVS star identifier to the SKY2000 star identifiers.
        // Otherwise, if we don't have any GCVS stars, use the GCVS identifier string from SKY2000.
        
        SSIdentifier hdIdent = SSIdentifier ( kCatHD, strtoint ( strHD ) ), gcvsIdent = 0;
        SSVariableStarPtr pGCVStar = SSGetVariableStarPtr ( SSIdentifierToObject ( hdIdent, gcvsMap, gcvsStars ) );
        if ( pGCVStar )
            gcvsIdent = pGCVStar->getIdentifier ( kCatGCVS );
        else if ( gcvsStars.size() == 0 && strVar.length() > 0 )
            gcvsIdent = SSIdentifier::fromString ( strVar );
        SSAddIdentifier ( gcvsIdent, idents );

        // If this star has a double star component string, look for a matching WDS star and get its primary component.
        
        char primComp = 0;
        SSDoubleStarPtr pWDStar = nullptr;
        if ( strDblComp.length() > 0 && wdsHTM.countRegions() > 0 )
            pWDStar = getWDStar ( wdsHTM, SSSpherical ( ra, dec, 1.0 ), strDblComp[0], primComp, strtofloat ( strDblSep ) );
        
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
        
        addSKY2000StarData ( hipStars, hipMap, pStar );
        addSKY2000StarData ( gjStars, gjMap, pStar );
        pStar->sortIdentifiers();
        
        SSVariableStarPtr pVar = SSGetVariableStarPtr ( pObj );
        if ( pVar != nullptr )
        {
            // If we have a matching star from the GCVS, copy its variability data;
            // otherwise use variability data already present in SKY2000.
            
            if ( pGCVStar )
            {
                pVar->setMinimumMagnitude ( pGCVStar->getMinimumMagnitude() );
                pVar->setMaximumMagnitude ( pGCVStar->getMaximumMagnitude() );
                pVar->setPeriod ( pGCVStar->getPeriod() );
                pVar->setEpoch ( pGCVStar->getEpoch() );
                pVar->setVariableType ( pGCVStar->getVariableType() );
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
                if ( pWDStar->hasOrbit() )
                {
                    char comp = strDblComp[0];
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
