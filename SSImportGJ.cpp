// SSImportGJ.cpp
// SSCore
//
// Created by Tim DeBenedictis on 3/27/20.
// Copyright © 2020 Southern Stars. All rights reserved.

#include <iostream>
#include <fstream>

#include "SSDynamics.hpp"
#include "SSHipparcos.hpp"
#include "SSImportGJ.hpp"

// Comverts total proper motion (pm), position angle of motion (pa), and declination (dec)
// to proper motion in R.A. (pmra) and proper motion in Dec. (pmdec).  All angles in radians.

void pm_pa_to_pmra_pmdec ( double pm, double pa, double dec, double &pmra, double &pmdec )
{
    pmra = pm * sin ( pa ) / cos ( dec );
    pmdec = pm * cos ( pa );
}

int SSImportGJ ( const char *filename, SSIdentifierNameMap &nameMap, SSObjectVec &stars )
{
    // Open file; return on failure.

    ifstream file ( filename );
    if ( ! file )
        return ( 0 );

	// Set up matrix for precessing B1950 coordinates and proper motion to J2000
	
	SSMatrix precession = SSCoords::getPrecessionMatrix ( SSTime::kB1950 ).transpose();
	
    // Read file line-by-line until we reach end-of-file

    string line = "";
    int numStars = 0;

    while ( getline ( file, line ) )
    {
		if ( line.length() < 187 )
			continue;
		
        // Get Identifier, HD, DM catalog numbers.
        // Note we are ignoring the identifier prefix (GJ, Gl, NN, Wo)
        // and treating all identifiers as GJ numbers.
        
        string strGJ = trim ( line.substr ( 2, 6 ) );
        string strHD = trim ( line.substr ( 146, 6 ) );
        string strDM = trim ( line.substr ( 153, 12 ) );

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
        
//		SSUpdateStarCoordsMotion ( 1950.0, &precession, coords, motion );

        // Get parallax in milliarcsec and convert to distance if > 1 mas.
        
        float plx = strtofloat ( strPlx );
        if ( plx > 1.0 )
            coords.rad = 1000.0 * SSDynamics::kLYPerParsec / plx;
        
        // Get radial velocity in km/sec and convert to light speed.
        
        if ( ! strRV.empty() )
            motion.rad = strtofloat ( strRV ) / SSDynamics::kLightKmPerSec;
        
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

        if ( ! strGJ.empty() )
            addIdentifier ( idents, SSIdentifier::fromString ( "GJ " + strGJ ) );

        if ( ! strHD.empty() )
            addIdentifier ( idents, SSIdentifier ( kCatHD, strtoint ( strHD ) ) );
        
        if ( ! strDM.empty() )
            addIdentifier ( idents, SSIdentifier::fromString ( strDM ) );

        // Sert identifier vector.  Get name string(s) corresponding to identifier(s).
        // Construct star and insert into star vector.

        SSObjectType type = kTypeStar;
        
        SSObjectPtr pObj = SSNewObject ( type );
        SSStarPtr pStar = SSGetStarPtr ( pObj );
        
        if ( pStar != nullptr )
        {
            pStar->setNames ( names );
            pStar->setIdentifiers ( idents );
            pStar->setFundamentalMotion ( coords, motion );
            pStar->setVMagnitude ( vmag );
            pStar->setBMagnitude ( bmag );
            pStar->setSpectralType ( strSpec );

			cout << pStar->toCSV() << endl;
			stars.push_back ( pObj );
			numStars++;
		}
    }
    
    return numStars;
}
