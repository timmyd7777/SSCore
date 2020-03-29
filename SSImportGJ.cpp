// SSImportGJ.cpp
// SSCore
//
// Created by Tim DeBenedictis on 3/27/20.
// Copyright © 2020 Southern Stars. All rights reserved.

#include <iostream>
#include <fstream>

#include "SSDynamics.hpp"
#include "SSImportHIP.hpp"
#include "SSImportGJ.hpp"

// Comverts total proper motion (pm), position angle of motion (pa), and declination (dec)
// to proper motion in R.A. (pmra) and proper motion in Dec. (pmdec).  All angles in radians.

void pm_pa_to_pmra_pmdec ( double pm, double pa, double dec, double &pmra, double &pmdec )
{
    pmra = pm * sin ( pa ) / cos ( dec );
    pmdec = pm * cos ( pa );
}

// Comverts proper motion in R.A. (pmra) and proper motion in Dec. (pmdec), and declination (dec)
// to total proper motion (pm) and position angle of motion (pa). All angles in radians.
// TODO: NEEDS TESTING!

void pmra_pmdec_to_pm_pa ( double pmra, double pmdec, double dec, double &pm, double &pa )
{
	pmra *= cos ( dec );
	pm = sqrt ( pmra * pmra + pmdec * pmdec );
	pa = atan2pi ( pmra, pmdec );
}

// Imports Gliese-Jahreiss Catalog of Nearby Stars, 3rd (preliminary) Ed.:
// ftp://cdsarc.u-strasbg.fr/cats/V/70A/
// Imported stars are stored in the provided vector of SSObjects (stars).
// Names are added from nameMap, wherever possible.
// Accurate coordinates, proper motion, and HIP identifiers are added from hipStars.
// Returns the total number of stars imported (should be 3802; Sun is excluded).

int SSImportGJCNS3 ( const char *filename, SSIdentifierNameMap &nameMap, SSObjectVec &gjACStars, SSObjectVec &stars )
{
    // Open file; return on failure.

    ifstream file ( filename );
    if ( ! file )
        return ( 0 );

	// Set up GJ identifier mapping for retrieving accurate GJ coordinates and HIP identifiers.
	
	SSObjectMap map = SSMakeObjectMap ( gjACStars, kCatGJ );
	
	// Set up matrix for precessing B1950 coordinates and proper motion to J2000
	
	SSMatrix precession = SSCoords::getPrecessionMatrix ( SSTime::kB1950 ).transpose();
	
    // Read file line-by-line until we reach end-of-file

    string line = "";
    int numStars = 0;

    while ( getline ( file, line ) )
    {
		size_t len = line.length();
		if ( line.length() < 119 )
			continue;
		
        // Get GJ identifier and components (A, B, C, etc.)
        // Note we are ignoring the identifier prefix (GJ, Gl, NN, Wo)
        // and treating all identifiers as GJ numbers.
        
        string strGJ = trim ( line.substr ( 2, 10 ) );
 		string comps = trim ( line.substr ( 8, 2 ) );

        // Get Identifier, HD, DM catalog numbers.

		string strHD = len < 153 ? "" : trim ( line.substr ( 146, 6 ) );
		string strDM = len < 165 ? "" : trim ( line.substr ( 153, 12 ) );

		// Extract RA and Dec. If either are blank, skip this line.
        
        string strRA = trim ( line.substr ( 12,8 ) );
        string strDec = trim ( line.substr ( 21, 8 ) );
		if ( strRA.empty() || strDec.empty() )
			continue;
		
        // Extract proper motion and position angle of proper motion
        
        string strPM = trim ( line.substr ( 30, 6 ) );
        string strPA = trim ( line.substr ( 37, 5 ) );
        
        // Extract radial velocity and spectral type.
        
        string strRV = trim ( line.substr ( 43, 6 ) );
        string strSpec = trim ( line.substr ( 54, 12 ) );

        // Extract Johnson V magnitude and B-V color index.
        
        string strVmag = trim ( line.substr ( 67, 6 ) );
        string strBmV = trim ( line.substr ( 76, 5 ) );

        // Extract resulting parallax and standard error of parallax.
        
        string strPlx = trim ( line.substr ( 108, 6 ) );
        string strPlxErr = trim ( line.substr ( 114, 5 ) );
        
        // Get B1950 Right Ascension and Declination
        
        double ra = degtorad ( strtodeg ( strRA ) * 15.0 );
        double dec = degtorad ( strtodeg ( strDec ) );
        		
		// Get B1950 proper motion and position angle;
        // if both present convert to proper motion in R.A and Dec.

        double pmRA = HUGE_VAL, pmDec = HUGE_VAL;
        if ( ! strPM.empty() && ! strPA.empty() )
        {
            double pm = SSAngle::fromArcsec ( strtofloat64 ( strPM ) );
            double pa = SSAngle::fromDegrees ( strtofloat64 ( strPA ) );
            pm_pa_to_pmra_pmdec ( pm, pa, dec, pmRA, pmDec );
        }
        
		// Precess B1950 position and proper motion to J2000
		
        SSSpherical coords ( ra, dec, 1.0 );
        SSSpherical motion ( pmRA, pmDec, 0.0 );
        
		SSUpdateStarCoordsAndMotion ( 1950.0, &precession, coords, motion );

        // Get parallax in milliarcsec and convert to distance if > 1 mas.
        
        float plx = strtofloat ( strPlx );
        if ( plx > 1.0 )
            coords.rad = 1000.0 * SSDynamics::kLYPerParsec / plx;
        
        // Get radial velocity in km/sec and convert to light speed.
        
        motion.rad = strRV.empty() ? HUGE_VAL : strtofloat ( strRV ) / SSDynamics::kLightKmPerSec;
        
		// Get Johnson V magnitude
        
        float vmag = HUGE_VAL;
        if ( ! strVmag.empty() )
            vmag = strtofloat ( strVmag );
        
        // Get Johnson B magnitude from color index
        
        float bmag = HUGE_VAL;
        if ( ! strBmV.empty() )
            bmag = strtofloat ( strBmV ) + vmag;

        // Set up name and identifier vectors.

        vector<SSIdentifier> idents ( 0 );
        vector<string> names ( 0 );

		// Create GJ, HD, DM identifiers
		
		SSIdentifier identGJ, identHD, identDM;
		
        if ( ! strGJ.empty() )
			identGJ = SSIdentifier::fromString ( "GJ " + strGJ );
		
        if ( ! strHD.empty() )
            identHD = SSIdentifier ( kCatHD, strtoint ( strHD ) );
        
        if ( ! strDM.empty() )
            identDM = SSIdentifier::fromString ( strDM );

		SSAddIdentifier ( identGJ, idents );
		SSAddIdentifier ( identHD, idents );
		SSAddIdentifier ( identDM, idents );

		// Look up GJ star with accurate coordinates.  If we find one,
		// replace CNS3 coordinates and motion with accurate GJ coordinates, distance,
		// and proper motion (but not radial velocity!), and add HIP identifier.
		
		SSStarPtr pACStar = SSGetStarPtr ( SSIdentifierToObject ( identGJ, map, gjACStars ) );
		if ( pACStar != nullptr )
		{
			SSSpherical accCoords = pACStar->getFundamentalCoords();
			SSSpherical accMotion = pACStar->getFundamentalMotion();
			
			coords.lon = accCoords.lon;
			coords.lat = accCoords.lat;
			coords.rad = isinf ( accCoords.rad ) ? coords.rad : accCoords.rad;
			
			motion.lon = accMotion.lon;
			motion.lat = accMotion.lat;
			motion.rad = isinf ( accMotion.rad ) ? motion.rad : accMotion.rad;

			SSAddIdentifier ( pACStar->getIdentifier ( kCatHIP ), idents );
			SSAddIdentifier ( pACStar->getIdentifier ( kCatBayer ), idents );
			SSAddIdentifier ( pACStar->getIdentifier ( kCatFlamsteed ), idents );
			SSAddIdentifier ( pACStar->getIdentifier ( kCatGCVS ), idents );
		}

        // Sert identifier vector.  Get name string(s) corresponding to identifier(s).
        // Construct star and insert into star vector.

		sort ( idents.begin(), idents.end(), compareSSIdentifiers );
        SSObjectType type = comps.empty() ? kTypeStar : kTypeDoubleStar;
        SSObjectPtr pObj = SSNewObject ( type );
        SSStarPtr pStar = SSGetStarPtr ( pObj );
        SSDoubleStarPtr pDblStar = SSGetDoubleStarPtr ( pObj );
        
        if ( pStar != nullptr )
        {
            pStar->setNames ( names );
            pStar->setIdentifiers ( idents );
            pStar->setFundamentalMotion ( coords, motion );
            pStar->setVMagnitude ( vmag );
            pStar->setBMagnitude ( bmag );
            pStar->setSpectralType ( strSpec );

			if ( pDblStar != nullptr )
				pDblStar->setComponents ( comps );
			
			// cout << pStar->toCSV() << endl;
			stars.push_back ( pObj );
			numStars++;
		}
    }
    
	// Return imported star count; file is closed automatically.

	return numStars;
}

// Imports Accurate Coordinates for Gliese Catalog Stars:
// https://cdsarc.unistra.fr/ftp/J/PASP/122/885
// Imported stars are stored in the provided vector of SSObjects (stars).
// Parallaxes, magnitudes, and identifiers are taken from Hipparcos stars (hipStars).
// If no HIP stars provided, 2MASS J and H magnitudes are stored as V and B, respectively.
// Returns the total number of stars imported (should be 4106).

int SSImportGJAC ( const char *filename, SSObjectVec &hipStars, SSObjectVec &stars )
{
    // Open file; return on failure.

    ifstream file ( filename );
    if ( ! file )
        return ( 0 );

	// Set up HIP identifier mapping for retrieving Hipparcos stars.
	
	SSObjectMap map = SSMakeObjectMap ( hipStars, kCatHIP );
	
    // Read file line-by-line until we reach end-of-file

    string line = "";
    int numStars = 0;

    while ( getline ( file, line ) )
    {
		if ( line.length() < 124 )
			continue;
		
        // Get Gl/GJ/NN Identifier (including component A, B, C, etc.)
		// Get HIP or other identifier.
        
        string strGJ = trim ( line.substr ( 2, 20 ) );
        string strHIP = trim ( line.substr ( 22, 13 ) );
		
		// Extract component A, B, C from GJ identifier
		
		size_t pos = strGJ.find_first_of ( "ABC" );
 		string comps = pos == string::npos ? "" : trim ( line.substr ( pos, string::npos ) );

        // Extract RA and Dec. If either are blank, skip this line.
        
        string strRA = trim ( line.substr ( 36, 11 ) );
        string strDec = trim ( line.substr ( 48, 11 ) );
		if ( strRA.empty() || strDec.empty() )
			continue;
		
        // Extract proper motion in R.A. and Dec.
        
        string strPMRA = trim ( line.substr ( 61, 6 ) );
        string strPMDec = trim ( line.substr ( 69, 6 ) );
        
		// Extract 2MASS J and H magnitudes
		
		string strJmag = trim ( line.substr ( 94, 6 ) );
		string strHmag = trim ( line.substr ( 101, 6 ) );

		// Get J2000 Right Ascension and Declination
        
        double ra = degtorad ( strtodeg ( strRA ) * 15.0 );
        double dec = degtorad ( strtodeg ( strDec ) );
		
		// Convert J2000 proper motion from arcsec to radians
		
		float pmRA = HUGE_VAL;
		if ( ! strPMRA.empty() )
			pmRA = SSAngle::fromArcsec ( strtofloat ( strPMRA ) ) / cos ( dec );
		
		float pmDec = HUGE_VAL;
		if ( ! strPMDec.empty() )
			pmDec = SSAngle::fromArcsec ( strtofloat ( strPMDec ) );

		SSSpherical coords ( ra, dec, HUGE_VAL );
        SSSpherical motion ( pmRA, pmDec, HUGE_VAL );

        // Get 2MASS J and H magnitudes
        
        float jmag = strJmag.empty() ? HUGE_VAL : strtofloat ( strJmag );
        float hmag = strHmag.empty() ? HUGE_VAL : strtofloat ( strHmag );

		// Set up name and identifier vectors.

        vector<SSIdentifier> idents ( 0 );
        vector<string> names ( 0 );

        if ( ! strGJ.empty() )
            SSAddIdentifier ( SSIdentifier::fromString ( "GJ " + strGJ ), idents );

		SSIdentifier hipID = SSIdentifier::fromString ( strHIP );
		if ( hipID )
            SSAddIdentifier ( hipID, idents );

		// Look up Hipparcos star from HIP identifier.  If we find one,
		// add distance, magnitudes, and selected identifiers.
		
		SSStarPtr pHIPStar = SSGetStarPtr ( SSIdentifierToObject ( hipID, map, hipStars ) );
		if ( pHIPStar != nullptr )
		{
			coords.rad = SSDynamics::kLYPerParsec / pHIPStar->getParallax();
			motion.rad = pHIPStar->getRadVel();

			SSAddIdentifier ( pHIPStar->getIdentifier ( kCatBayer ), idents );
			SSAddIdentifier ( pHIPStar->getIdentifier ( kCatFlamsteed ), idents );
			SSAddIdentifier ( pHIPStar->getIdentifier ( kCatGCVS ), idents );
		}

		// Sert identifier vector.  Get name string(s) corresponding to identifier(s).
        // Construct star and insert into star vector.

		sort ( idents.begin(), idents.end(), compareSSIdentifiers );
		
        SSObjectType type = comps.empty() ? kTypeStar : kTypeDoubleStar;
        SSObjectPtr pObj = SSNewObject ( type );
        SSStarPtr pStar = SSGetStarPtr ( pObj );
        SSDoubleStarPtr pDblStar = SSGetDoubleStarPtr ( pObj );

        if ( pStar != nullptr )
        {
            pStar->setNames ( names );
            pStar->setIdentifiers ( idents );
            pStar->setFundamentalMotion ( coords, motion );
            pStar->setVMagnitude ( jmag );
            pStar->setBMagnitude ( hmag );

			if ( pDblStar != nullptr )
				pDblStar->setComponents ( comps );
			
			// cout << pStar->toCSV() << endl;
			stars.push_back ( pObj );
			numStars++;
		}
    }
    
	// Return imported star count; file is closed automatically.

	return numStars;
}
