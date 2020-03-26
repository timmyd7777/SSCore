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

// Mapping of NGC-IC numbers to Messier numbers.
// M 76 has two entries (NGC 650 and NGC 651).

static map<string,int> _messmap =
{
	{ "NGC 1952",	1 },	// Crab Nebula
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
	{ "NGC 224",   31 },	// Andromeda Galaxy
	{ "NGC 221",   32 },
	{ "NGC 598",   33 },	// Triangulum Galaxy
	{ "NGC 1039",  34 },
	{ "NGC 2168",  35 },
	{ "NGC 1960",  36 },
	{ "NGC 2099",  37 },
	{ "NGC 1912",  38 },
	{ "NGC 7092",  39 },
	{ "WNC 4",     40 },	// Winnecke 4
	{ "NGC 2287",  41 },
	{ "NGC 1976",  42 },	// Orion Nebula
	{ "NGC 1982",  43 },
	{ "NGC 2632",  44 },	// Beehive Cluster
	{ "Mel 22",    45 },	// Pleaiades
	{ "NGC 2437",  46 },
	{ "NGC 2422",  47 },
	{ "NGC 2548",  48 },
	{ "NGC 4472",  49 },
	{ "NGC 2323",  50 },
	{ "NGC 5194",  51 },	// Whirlpool Galaxy
	{ "NGC 7654",  52 },
	{ "NGC 5024",  53 },
	{ "NGC 6715",  54 },
	{ "NGC 6809",  55 },
	{ "NGC 6779",  56 },
	{ "NGC 6720",  57 },	// Ring Nebula
	{ "NGC 4579",  58 },
	{ "NGC 4621",  59 },
	{ "NGC 4649",  60 },
	{ "NGC 4303",  61 },
	{ "NGC 6266",  62 },
	{ "NGC 5055",  63 },	// Sunflower Galaxy
	{ "NGC 4826",  64 },	// Black Eye Galaxy
	{ "NGC 3623",  65 },
	{ "NGC 3627",  66 },
	{ "NGC 2682",  67 },
	{ "NGC 4590",  68 },
	{ "NGC 6637",  69 },
	{ "NGC 6681",  70 },
	{ "NGC 6838",  71 },
	{ "NGC 6981",  72 },
	{ "NGC 6994",  73 },
	{ "NGC 628",   74 },
	{ "NGC 6864",  75 },
	{ "NGC 650",   76 },	// Little Dumbbell Nebula
	{ "NGC 651",   76 },	// Little Dumbbell Nebula
	{ "NGC 1068",  77 },	// Cetus A
	{ "NGC 2068",  78 },
	{ "NGC 1904",  79 },
	{ "NGC 6093",  80 },
	{ "NGC 3031",  81 },	// Bode's Galaxy
	{ "NGC 3034",  82 },	// Cigar Galaxy
	{ "NGC 5236",  83 },
	{ "NGC 4374",  84 },	// Markarian's Chain
	{ "NGC 4382",  85 },
	{ "NGC 4406",  86 },	// Markarian's Chain
	{ "NGC 4486",  87 },	// Virgo A
	{ "NGC 4501",  88 },
	{ "NGC 4552",  89 },
	{ "NGC 4569",  90 },
	{ "NGC 4548",  91 },
	{ "NGC 6341",  92 },
	{ "NGC 2447",  93 },
	{ "NGC 4736",  94 },
	{ "NGC 3351",  95 },
	{ "NGC 3368",  96 },
	{ "NGC 3587",  97 },	// Owl Nebula
	{ "NGC 4192",  98 },
	{ "NGC 4254",  99 },
	{ "NGC 4321", 100 },
	{ "NGC 5457", 101 },	// Pinwheel Galaxy
	{ "NGC 5866", 102 },
	{ "NGC 581",  103 },
	{ "NGC 4594", 104 },	// Sombrero Galaxy
	{ "NGC 3379", 105 },
	{ "NGC 4258", 106 },
	{ "NGC 6171", 107 },
	{ "NGC 3556", 108 },
	{ "NGC 3992", 109 },
	{ "NGC 205",  110 }
};

// Mapping of NGC-IC numbers to Caldwell numbers.
// C 14 has two entries (NGC 869 and NGC 884).

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
	{ "NGC 7331",  30 },
	{ "IC 405",    31 },	// Flaming Star Nebula
	{ "NGC 4631",  32 },	// Whale Galaxy
	{ "NGC 6992",  33 },	// Eastern Veil Nebula
	{ "NGC 6960",  34 },	// Western Veil Nebula
	{ "NGC 4889",  35 },
	{ "NGC 4559",  36 },
	{ "NGC 6885",  37 },
	{ "NGC 4565",  38 },	// Needle Galaxy
	{ "NGC 2392",  39 },	// Eskimo Nebula
	{ "NGC 3626",  40 },
	{ "Mel 25",    41 },	// Hyades
	{ "NGC 7006",  42 },
	{ "NGC 7814",  43 },
	{ "NGC 7479",  44 },
	{ "NGC 5248",  45 },
	{ "NGC 2261",  46 },	// Hubble's Variable Nebula
	{ "NGC 6934",  47 },
	{ "NGC 2775",  48 },
	{ "NGC 2237",  49 },	// Rosette Nebula
	{ "NGC 2244",  50 },
	{ "IC 1613",   51 },
	{ "NGC 4697",  52 },
	{ "NGC 3115",  53 },	// Spindle Galaxy
	{ "NGC 2506",  54 },
	{ "NGC 7009",  55 },	// Saturn Nebula
	{ "NGC 246",   56 },
	{ "NGC 6822",  57 },	// Barnard's Galaxy
	{ "NGC 2360",  58 },
	{ "NGC 3242",  59 },	// Ghost of Jupiter
	{ "NGC 4038",  60 },	// Antennae Galaxies
	{ "NGC 4039",  61 },	// Antennae Galaxies
	{ "NGC 247",   62 },
	{ "NGC 7293",  63 },	// Helix Nebula
	{ "NGC 2362",  64 },
	{ "NGC 253",   65 },	// Sculptor Galaxy
	{ "NGC 5694",  66 },
	{ "NGC 1097",  67 },
	{ "NGC 6729",  68 },
	{ "NGC 6302",  69 },	// Bug Nebula
	{ "NGC 300",   70 },
	{ "NGC 2477",  71 },
	{ "NGC 55",    72 },
	{ "NGC 1851",  73 },
	{ "NGC 3132",  74 },	// Eight Burst Nebula
	{ "NGC 6124",  75 },
	{ "NGC 6231",  76 },
	{ "NGC 5128",  77 },	// Centaurus A
	{ "NGC 6541",  78 },
	{ "NGC 3201",  79 },
	{ "NGC 5139",  80 },	// Omega Centauri
	{ "NGC 6352",  81 },
	{ "NGC 6193",  82 },
	{ "NGC 4945",  83 },
	{ "NGC 5286",  84 },
	{ "IC 2391",   85 },	// Omicron Velorum Cluster
	{ "NGC 6397",  86 },
	{ "NGC 1261",  87 },
	{ "NGC 5823",  88 },
	{ "NGC 6087",  89 },
	{ "NGC 2867",  90 },
	{ "NGC 3532",  91 },	// Wishing Well Cluster
	{ "NGC 3372",  92 },	// Eta Carinae Nebula
	{ "NGC 6752",  93 },
	{ "NGC 4755",  94 },	// Jewel Box
	{ "NGC 6025",  95 },
	{ "NGC 2516",  96 },
	{ "NGC 3766",  97 },	// Pearl Cluster
	{ "NGC 4609",  98 },
	{ "Coalsack",  99 },	// Coalsack
	{ "IC 2944",  100 },	// Lambda Centauri Cluster
	{ "NGC 6744", 101 },
	{ "IC 2602",  102 },	// Southern Pleiades
	{ "NGC 2070", 103 },	// Tarantula Nebula
	{ "NGC 362",  104 },
	{ "NGC 4833", 105 },
	{ "NGC 104",  106 },	// 47 Tucanae
	{ "NGC 6101", 107 },
	{ "NGC 4372", 108 },
	{ "NGC 3195", 109 }
};


// Imports Wolfgang Steinicke's Revised NGC-IC Catalogs, obtained from:
// http://www.klima-luft.de/steinicke/index_e.htm with data files here:
// http://www.klima-luft.de/steinicke/ngcic/rev2000/NI2019.zip
// This is an Excel file; convert it to tab-delimited text first.
// Inserts name strings from nameNap; if empty, no names will be added.
// Stores results in vector of SSObjects (objects).
// Returns number of NGC-IC objects imported.
// TODO: add Caldwell numbers; add Pleiades, Hyades, LMC, M 40, M 73.
// TODO: fix NGC-IC objects classified as stars!

int SSImportNGCIC ( const char *filename, SSIdentifierNameMap &nameMap, SSObjectVec &objects )
{
    // Open file; return on failure.

    ifstream file ( filename );
    if ( ! file )
        return ( 0 );

    // Read file line-by-line until we reach end-of-file

    string line = "";
    int numObjects = 0;

    while ( getline ( file, line ) )
    {
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
		SSDeepSkyPtr pObject = nullptr;
		
		if ( type == 1 )
			pObject = new SSDeepSky ( kTypeGalaxy );
		else if ( type == 2 )
			pObject = new SSDeepSky ( kTypeBrightNebula );
		else if ( type == 3 || type == 6 )
			pObject = new SSDeepSky ( kTypePlanetaryNebula );
		else if ( type == 4 )
			pObject = new SSDeepSky ( kTypeOpenCluster );
		else if ( type == 5 )
			pObject = new SSDeepSky ( kTypeGlobularCluster );
		else if ( type == 9 )
			pObject = new SSDeepSky ( kTypeStar );
		else if ( type == 10 )
			pObject = new SSDeepSky ( kTypeNonexistent );
		
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
		
		// Generate main NGC or IC identifier

		vector<SSIdentifier> idents;
		
		string ngcicStr = "";
		
		if ( tokens[0][0] == 'N' )
			ngcicStr = "NGC " + tokens[1] + tokens[2];
		else if ( tokens[0][0] == 'I' )
			ngcicStr = "IC " + tokens[1] + tokens[2];

		addIdentifier ( idents, SSIdentifier::fromString ( ngcicStr ) );

		// Add Messier and Caldwell identifiers from NGC-IC string mappings
		
		int messNum = _messmap[ ngcicStr ];
		int caldNum = _caldmap[ ngcicStr ];
		
		if ( messNum > 0 )
			addIdentifier ( idents, SSIdentifier ( kCatMessier, messNum ) );

		if ( caldNum > 0 )
			addIdentifier ( idents, SSIdentifier ( kCatCaldwell, caldNum ) );

		// Get Principal Galaxy Catalog number, if any.
		
		if ( ! tokens[26].empty() )
			addIdentifier ( idents, SSIdentifier ( kCatPGC, strtoint ( tokens[26] ) ) );

		// Get additional identifiers from remaining tokens.
		
		for ( int k = 27; k < tokens.size(); k++ )
			if ( ! tokens[k].empty() )
				addIdentifier ( idents, SSIdentifier::fromString ( tokens[k] ) );
		
		// get names from identifiers.  Sort identifier list.
		
		vector<string> names = getNamesFromIdentifiers( idents, nameMap );
        sort ( idents.begin(), idents.end(), compareSSIdentifiers );

		pObject->setNames ( names );
		pObject->setIdentifiers ( idents );
		pObject->setFundamentalMotion ( coords, motion );
		pObject->setVMagnitude ( vmag );
		pObject->setBMagnitude ( bmag );
		pObject->setGalaxyType ( strType );
		pObject->setMajorAxis ( sizeX );
		pObject->setMinorAxis ( sizeY );
		pObject->setPositionAngle ( pa );
		
		// cout << pDeepSkyObj->toCSV() << endl;
		objects.push_back ( shared_ptr<SSObject> ( pObject ) );
		numObjects++;
	}

	return numObjects;
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
