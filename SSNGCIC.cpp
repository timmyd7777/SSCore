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

// Imports Wolfgang Steinicke's Revised NGC-IC Catalogs, obtained from:
// http://www.klima-luft.de/steinicke/index_e.htm with data files here:
// http://www.klima-luft.de/steinicke/ngcic/rev2000/NI2019.zip
// This is an Excel file; convert it to tab-delimited text first.
// Inserts name strings from nameNap; if empty, no names will be added.
// Stores results in vector of SSObjects (objects).
// TODO: add Caldwell numbers; add Pleiades, Hyades, LMC, M 40, M 73.
// TODO: fix NGC-IC objects classified as stars!

void importNGCIC ( const char *filename, SSIdentifierNameMap &nameMap, SSObjectVec &objects )
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
		
		// get names from identifiers.  Sort identifier list.
		
		vector<string> names = getNamesFromIdentifiers( idents, nameMap );
        sort ( idents.begin(), idents.end(), compareSSIdentifiers );

		SSDeepSkyPtr pDeepSkyObj = SSGetDeepSkyPtr ( pObject );
		if ( pDeepSkyObj != nullptr )
		{
			pDeepSkyObj->setNames ( names );
			pDeepSkyObj->setIdentifiers ( idents );
			pDeepSkyObj->setFundamentalMotion ( coords, motion );
			pDeepSkyObj->setVMagnitude ( vmag );
			pDeepSkyObj->setBMagnitude ( bmag );
			pDeepSkyObj->setGalaxyType ( strType );
			pDeepSkyObj->setMajorAxis ( sizeX );
			pDeepSkyObj->setMinorAxis ( sizeY );
			pDeepSkyObj->setPositionAngle ( pa );

			if ( pDeepSkyObj->getIdentifier ( kCatMessier ) )
				cout << pDeepSkyObj->toCSV() << endl;
		}
	}
}

// Reads identifier-to-name map from filename and stores results in nameMap.
// If successful, nameMap will contain identifier-to-name pairs;
// on failure, nothing will be read into nameMap.

void importNGCICNameMap ( const char *filename, SSIdentifierNameMap &nameMap )
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
	int paircount = 0;
	
    while ( getline ( file, line ) )
    {
		linecount++;

		// Split line into tokens separated by commas.
		// Require at least 2 tokens.  First token is name.
		
		vector<string> tokens = split ( line, "," );
		if ( tokens.size() < 2 )
			continue;
		
		// For each token after the first, attempt to generate an identifier.
		// If successful, insert an identifier-name pair into the map.
		
		for ( int k = 1; k < tokens.size(); k++ )
		{
			SSIdentifier ident = SSIdentifier::fromString ( tokens[k] );
			if ( ident )
			{
				nameMap.insert ( { ident, tokens[0] } );
				paircount++;
			}
			else
			{
				cout << "Warning: can't convert " << tokens[k] << " for " << tokens[0] << endl;
			}
		}
	}
	
	cout << "Success: imported " << paircount << " identifier-name pairs." << endl;
}

// Given a vector of identifiers, returns vector of all corresponding name strings
// from the input identifier-to-name map.  If no names correspond to any identifier,
// returns a zero-length vector.

vector<string> getNamesFromIdentifiers ( vector<SSIdentifier> &idents, SSIdentifierNameMap &nameMap )
{
    vector<string> names;

    for ( SSIdentifier ident : idents )
    {
		auto nameRange = nameMap.equal_range ( ident );
		for ( auto i = nameRange.first; i != nameRange.second; i++ )
		{
			string name = i->second;
			
			if ( name.length() > 0 && find ( names.begin(), names.end(), name ) == names.end() )
				names.push_back ( name );
		}
    }

    return names;
}

static map<string,int> _messmap =
{
	{ "NGC 1951",	1 },	// Crab Nebula
	{ "NGC 7089",	2 },
	{ "NGC 5272",	3 },
	{ "NGC 6121",	4 },
	{ "NGC 5904", 	5 },
	{ "NGC 6405", 	6 },	// Butterfly Cluster
	{ "NGC 6475", 	7 },	// Ptolemy's Cluster
	{ "NGC 6523", 	8 },	// Lagoon Nebula
	{ "NGC 6333",   9 },
	{ "NGC 6254",  10 },
	{ "NGC 6705",  11 },	// Wild Duck Cluster
	{ "NGC 6218",  12 },
	{ "NGC 6205",  13 },	// Hercules Cluster
	{ "NGC 6402",  14 },
	{ "NGC 7078",  15 },
	{ "NGC 6611",  16 },	// Eagle Nebula
	{ "NGC 6618",  17 },	// Omega Nebula
	{ "NGC 6613",  18 },
	{ "NGC 6273",  19 },
	{ "NGC 6514",  20 },	// Trifid Nebula
	{ "NGC 6531",  21 },
	{ "NGC 6656",  22 },
	{ "NGC 6494",  23 },
	{ "IC 4715",   24 },	// Sagittarius Star Cloud
	{ "IC 4725",   25 },
	{ "NGC 6694",  26 },
	{ "NGC 6853",  27 },	// Dumbbell Nebula
	{ "NGC 6626",  28 },
	{ "NGC 6913",  29 },
	{ "NGC 7099",  30 },
};

static map<string,int> _caldmap =
{
	{ "NGC 188",	1 },
	{ "NGC 40",		2 },	// Bow-Tie Nebula
	{ "NGC 4236",	3 },
	{ "NGC 7023",	4 },	// Iris Nebula
	{ "IC 342", 	5 },
	{ "NGC 6543", 	6 },	// Cat Eye Nebula
	{ "NGC 2403", 	7 },
	{ "NGC 559", 	8 },
	{ "Sh2 155",    9 },	// Cave Nebula
	{ "NGC 663",   10 },
	{ "NGC 7635",  11 },	// Bubble Nebula
	{ "NGC 6946",  12 },	// Fireworks Galaxy
	{ "NGC 457",   13 },	// Owl Cluster
	{ "NGC 868",   14 },	// Double Cluster
	{ "NGC 884",   14 },	// Double Cluster
	{ "NGC 6826",  15 },	// Blinking Planetary
	{ "NGC 7243",  17 },
	{ "NGC 185",   18 },
	{ "IC 5146",   19 },
	{ "NGC 7000",  20 },	// North America Nebula
	{ "NGC 4449",  21 },
	{ "NGC 7662",  22 },	// Blue Snowball
	{ "NGC 891",   23 },
	{ "NGC 1275",  24 },	// Perseus A
	{ "NGC 2419",  25 },
	{ "NGC 4244",  26 },
	{ "NGC 6888",  27 },	// Crescent Nebula
	{ "NGC 752",   28 },
	{ "NGC 5005",  29 },
	{ "NGC 7331",  30 }
};

