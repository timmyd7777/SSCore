//
//  SSNGCIC.cpp
//  SSCore
//
//  Created by Tim DeBenedictis on 3/23/20.
//  Copyright © 2020 Southern Stars. All rights reserved.
//

#include "SSDynamics.hpp"
#include "SSNGCIC.hpp"

#include <iostream>
#include <fstream>

void importNGCIC ( const char *filename, SSObjectVec &objects )
{
    // Open file; report error and return empty map on failure.

    ifstream file ( filename );
    if ( ! file )
    {
        cout << "Failure: can't open " << filename << endl;
        return;
    }

    // Read file line-by-line until we reach end-of-file

    string line = "";
    int linecount = 0;

    while ( getline ( file, line ) )
    {
		linecount++;
		
		// Split line into tokens separated by tabs.
		// Require at least 27 tokens.
		
		vector<string> tokens = split ( line, "\t" );
		if ( tokens.size() < 27 )
			continue;
		
		// Trim leading and trailing whitespace from tokens.
		
		for ( int i = 0; i < tokens.size(); i++ )
			tokens[i] = trim ( tokens[i] );
		
		// Skip entries that are duplicates of the same NGC or IC number.

		if ( ! tokens[3].empty() )
			if ( strtoint ( tokens[3] ) > 1 )
				continue;
		
		// Get object type. Type 7 and 8 are duplicates of other objects;
		// skip them.  Type 9 (stars) and 10 (not found) are unknown/nonexistent objects.
		
		int type = strtoint ( tokens[5] );
		SSObjectPtr pObject = nullptr;
		
		if ( type == 1 )
			pObject = SSNewObject ( kTypeGalaxy );
		else if ( type == 2 )
			pObject = SSNewObject ( kTypeBrightNebula );
		else if ( type == 3 || type == 6 )
			pObject = SSNewObject ( kTypePlanetaryNebula );
		else if ( type == 4 )
			pObject = SSNewObject ( kTypeOpenCluster );
		else if ( type == 5 )
			pObject = SSNewObject ( kTypeGlobularCluster );
		else if ( type == 9 )
			pObject = SSNewObject ( kTypeStar );
		else if ( type == 10 )
			pObject = SSNewObject ( kTypeUnknown );
		
		if ( pObject == nullptr )
			continue;
		
		// If RA hours or Dec degrees are missing, skip invalid record.
		
		if ( tokens[8].empty() || tokens[12].empty() )
			continue;
		
		// Get RA and Dec and convert to radians.
		
		string strRA = tokens[8] + " " + tokens[9] + " " + tokens[10];
		string strDec = tokens[11] + tokens[12] + " " + tokens[13] + " " + tokens[14];
		
		SSHourMinSec ra ( strRA );
		SSDegMinSec dec ( strDec );
		
		SSSpherical coords ( SSAngle ( ra ), SSAngle ( dec ), HUGE_VAL );
		SSSpherical motion ( HUGE_VAL, HUGE_VAL, HUGE_VAL );
		
		// Get Johnson V and B magnitudes, if present.
		
		float vmag = tokens[16].empty() ? HUGE_VAL : strtofloat ( tokens[16] );
		float bmag = tokens[15].empty() ? HUGE_VAL : strtofloat ( tokens[15] );

		// Get angular dimensions in arcmin and position angle in arcsec, and convert to radians
		
		float sizeX = tokens[19].empty() ? HUGE_VAL : strtofloat ( tokens[19] ) * SSAngle::kRadPerArcmin;
		float sizeY = tokens[20].empty() ? HUGE_VAL : strtofloat ( tokens[20] ) * SSAngle::kRadPerArcmin;
		float pa = tokens[21].empty() ? HUGE_VAL : strtofloat ( tokens[21] ) * SSAngle::kRadPerDeg;

		// Get redshift and convert to radial velocity
		
		motion.rad = tokens[23].empty() ? HUGE_VAL : SSDynamics::redshiftToRadVel ( strtofloat ( tokens[23] ) );
		
		// Get distance in parsecs.  Prefer metric over redshift-derived.
		
		if ( ! tokens[25].empty() )
			coords.rad = strtofloat ( tokens[25] ) * 1.0e6;
		else if ( ! tokens[24].empty() )
			coords.rad = strtofloat ( tokens[24] ) * 1.0e6;
		
		// Get Hubble morphological type
		
		string strType = tokens[22];
		
		// Generate NGC-IC identifier, if present.
		
		if ( tokens[1].empty() )
			continue;
		
		// Is this an IC object or an NGC object?

		vector<SSIdentifier> idents;
		
		if ( tokens[0][0] == 'N' )
			addIdentifier ( idents, SSIdentifier::fromString ( "NGC " + tokens[1] + tokens[2] ) );
		else if ( tokens[0][0] == 'I' )
			addIdentifier ( idents, SSIdentifier::fromString ( "IC " + tokens[1] + tokens[2] ) );

		for ( int k = 27; k < tokens.size(); k++ )
			if ( ! tokens[k].empty() )
				addIdentifier ( idents, SSIdentifier::fromString ( tokens[k] ) );
		
		SSDeepSkyPtr pDeepSkyObj = SSGetDeepSkyPtr ( pObject );
		if ( pDeepSkyObj != nullptr )
		{
			pDeepSkyObj->setIdentifiers ( idents );
			pDeepSkyObj->setFundamentalMotion ( coords, motion );
			pDeepSkyObj->setVMagnitude ( vmag );
			pDeepSkyObj->setBMagnitude ( bmag );
			pDeepSkyObj->setGalaxyType ( strType );
			pDeepSkyObj->setMajorAxis ( sizeX );
			pDeepSkyObj->setMinorAxis ( sizeY );
			pDeepSkyObj->setPositionAngle ( pa );

			cout << pDeepSkyObj->toCSV() << endl;
		}
	}
}
