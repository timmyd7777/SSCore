//  SSSKY2000.cpp
//  SSCore
//
//  Created by Tim DeBenedictis on 3/23/20.
//  Copyright © 2020 Southern Stars. All rights reserved.

#include "SSDynamics.hpp"
#include "SSSKY2000.hpp"

#include <iostream>
#include <fstream>

// Imports IAU official star name table from Working Group on Star Names
// from http://www.pas.rochester.edu/~emamajek/WGSN/IAU-CSN.txt
// Assumes names are unique (i.e. only one name per identifier);
// discards additional names beyond the first for any given identifier.
// Returns map of name strings indexed by identifier.

SSStarNameMap importIAUStarNames ( const char *filename )
{
    SSStarNameMap nameMap;
    
    // Open file; report error and return empty map on failure.

    ifstream file ( filename );
    if ( ! file )
    {
        cout << "Failure: can't open " << filename << endl;
        return nameMap;
    }

    // Read file line-by-line until we reach end-of-file

    string line = "";
    int linecount = 0;

    while ( getline ( file, line ) )
    {
        linecount++;
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
        
        // If successful, iInsert identifier and name into map; display warning on failure.
        
        if ( ident )
            nameMap.insert ( { ident, strName } );
        else
            cout << "Warning: can't convert " << strIdent << " for " << strName << endl;
    }

    // Return fully-imported name map.
    
    return nameMap;
}

// Given a vector of identifiers, returns vector of all corresponding name strings
// from the input star name map.  If no names correspond to any identifiers,
// returns a zero-length vector.

vector<string> getStarNames ( vector<SSIdentifier> &idents, SSStarNameMap nameMap )
{
    vector<string> names ( 0 );

    for ( SSIdentifier ident : idents )
    {
        string name = nameMap[ ident ];
        if ( name.length() > 0 )
            names.push_back ( name );
    }

    return names;
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

// Imports SKY2000 Master Star Catalog v5.
// Inserts name strings from nameNap;
// Returns vector of SSStar objects which should contain 299460 entries if successful.
// TODO: add HIP numbers and add'l Bayer letters from Hipparcos. Add nearby stars from RECONS.

vector<SSStar> importSKY2000 ( const char *filename, SSStarNameMap &nameMap )
{
    vector<SSStar> starVec;
    
    // Open file; report error and return empty map on failure.

    ifstream file ( filename );
    if ( ! file )
    {
        cout << "Failure: can't open " << filename << endl;
        return starVec;
    }

    // Read file line-by-line until we reach end-of-file

    string line = "";
    int linecount = 0;

    while ( getline ( file, line ) )
    {
        linecount++;
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
        
        string strVar = trim ( line.substr ( 108, 10 ) );

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
        // and J2000 proper in R.A and Dec.
        
        SSHourMinSec ra ( strRA );
        SSDegMinSec dec ( strDec );
        
        double pmRA = HUGE_VAL;
        if ( ! strPMRA.empty() )
            pmRA = SSAngle::fromArcsec ( strtofloat ( strPMRA ) );
        
        double pmDec = HUGE_VAL;
        if ( ! strPMDec.empty() )
            pmDec = SSAngle::fromArcsec ( strtofloat ( strPMDec ) );
        
        SSSpherical position ( ra, dec, HUGE_VAL );
        SSSpherical velocity ( pmRA, pmDec, HUGE_VAL );
        
        // Get parallax in arcsec and convert to distance if > 1 mas.
        
        double plx = strtofloat ( strPlx );
        if ( plx > 0.001 )
            position.rad = 1.0 / plx;
        
        // Get radial velocity in km/sec and convert to light speed.
        
        if ( ! strRV.empty() )
            velocity.rad = strtofloat ( strRV ) / SSDynamics::kLightKmPerSec;
        
        // Get Johnson V magnitude
        
        float vmag = HUGE_VAL;
        if ( ! strMag.empty() )
            vmag = strtofloat ( strMag );
        
        // Get Johnson B magnitude from color index
        
        float bmag = HUGE_VAL;
        if ( ! strBmV.empty() )
            bmag = strtofloat ( strBmV ) + vmag;
        
        // Set up name and identifier vectors.

        vector<SSIdentifier> idents ( 0 );
        vector<string> names ( 0 );
        
        if ( ! strBay.empty() )
            addIdentifier ( idents, SSIdentifier::fromString ( strBay ) );
        
        if ( ! strFlm.empty() )
            addIdentifier ( idents, SSIdentifier::fromString ( strFlm ) );
        
        if ( ! strVar.empty() )
            addIdentifier ( idents, SSIdentifier::fromString ( strVar ) );

        if ( ! strHR.empty() )
            addIdentifier ( idents, SSIdentifier ( kCatHR, strtoint ( strHR ) ) );

        if ( ! strHD.empty() )
            addIdentifier ( idents, SSIdentifier ( kCatHD, strtoint ( strHD ) ) );
        
        if ( ! strSAO.empty() )
            addIdentifier ( idents, SSIdentifier ( kCatSAO, strtoint ( strSAO ) ) );

        if ( ! strDM.empty() )
            addIdentifier ( idents, SSIdentifier::fromString ( strDM ) );
        
        if ( ! strWDS.empty() )
            addIdentifier ( idents, SSIdentifier::fromString ( "WDS " + strWDS ) );
        
        // Sert identifier vector.  Get name string(s) corresponding to identifier(s).
        // Construct star and insert into star vector.
        
        sort ( idents.begin(), idents.end(), compareSSIdentifiers );
        names = getStarNames ( idents, nameMap );
        
        bool isVar = ! ( strVarType.empty() && strVarMax.empty() && strVarMax.empty() && strVarPer.empty() );
        bool isDbl = ! ( strWDS.empty() && strDblMag.empty() && strDblSep.empty() );

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
        
        if ( pStar != nullptr )
        {
            pStar->setNames ( names );
            pStar->setIdentifiers ( idents );
            pStar->setFundamentalMotion ( position, velocity );
            pStar->setVMagnitude ( vmag );
            pStar->setBMagnitude ( bmag );
            pStar->setSpectralType ( strSpec );
        }
        
        SSVariableStarPtr pVar = SSGetVariableStarPtr ( pObj );
        if ( pVar != nullptr )
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
        
        SSDoubleStarPtr pDbl = SSGetDoubleStarPtr ( pObj );
        if ( pDbl != nullptr )
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
        
        cout << pStar->toCSV() << endl;
//        starVec.push_back ( star );
    }

    // Report success or failure.  Return star map object.

    if ( linecount == starVec.size() )
        cout << "Success: " << filename << " linecount " << linecount << " == starVec.size() " << starVec.size() << endl;
    else
        cout << "Failure: " << filename << " linecount " << linecount << " != starVec.size() " << starVec.size() << endl;

    return starVec;
}
