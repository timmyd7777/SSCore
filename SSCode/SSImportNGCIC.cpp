//
//  SSImportNGCIC.cpp
//  SSCore
//
//  Created by Tim DeBenedictis on 3/23/20.
//  Copyright © 2020 Southern Stars. All rights reserved.
//

#include "SSCoordinates.hpp"
#include "SSImportHIP.hpp"
#include "SSImportNGCIC.hpp"

#include <algorithm>
#include <iostream>
#include <fstream>

// Mapping of NGC-IC numbers to Messier numbers.
// M 76 has two entries (NGC 650 and NGC 651).

static map<string,int> _messmap =
{
    { "NGC 1952",   1 },    // Crab Nebula
    { "NGC 7089",   2 },
    { "NGC 5272",   3 },
    { "NGC 6121",   4 },
    { "NGC 5904",   5 },
    { "NGC 6405",   6 },    // Butterfly Cluster
    { "NGC 6475",   7 },    // Ptolemy's Cluster
    { "NGC 6523",   8 },    // Lagoon Nebula
    { "NGC 6333",   9 },
    { "NGC 6254",  10 },
    { "NGC 6705",  11 },    // Wild Duck Cluster
    { "NGC 6218",  12 },
    { "NGC 6205",  13 },    // Hercules Cluster
    { "NGC 6402",  14 },
    { "NGC 7078",  15 },
    { "NGC 6611",  16 },    // Eagle Nebula
    { "NGC 6618",  17 },    // Omega Nebula
    { "NGC 6613",  18 },
    { "NGC 6273",  19 },
    { "NGC 6514",  20 },    // Trifid Nebula
    { "NGC 6531",  21 },
    { "NGC 6656",  22 },
    { "NGC 6494",  23 },
    { "IC 4715",   24 },    // Sagittarius Star Cloud
    { "IC 4725",   25 },
    { "NGC 6694",  26 },
    { "NGC 6853",  27 },    // Dumbbell Nebula
    { "NGC 6626",  28 },
    { "NGC 6913",  29 },
    { "NGC 7099",  30 },
    { "NGC 224",   31 },    // Andromeda Galaxy
    { "NGC 221",   32 },
    { "NGC 598",   33 },    // Triangulum Galaxy
    { "NGC 1039",  34 },
    { "NGC 2168",  35 },
    { "NGC 1960",  36 },
    { "NGC 2099",  37 },
    { "NGC 1912",  38 },
    { "NGC 7092",  39 },
    { "WNC 4",     40 },    // Winnecke 4
    { "NGC 2287",  41 },
    { "NGC 1976",  42 },    // Orion Nebula
    { "NGC 1982",  43 },
    { "NGC 2632",  44 },    // Beehive Cluster
    { "Melotte 22",45 },    // Pleaiades
    { "NGC 2437",  46 },
    { "NGC 2422",  47 },
    { "NGC 2548",  48 },
    { "NGC 4472",  49 },
    { "NGC 2323",  50 },
    { "NGC 5194",  51 },    // Whirlpool Galaxy
    { "NGC 7654",  52 },
    { "NGC 5024",  53 },
    { "NGC 6715",  54 },
    { "NGC 6809",  55 },
    { "NGC 6779",  56 },
    { "NGC 6720",  57 },    // Ring Nebula
    { "NGC 4579",  58 },
    { "NGC 4621",  59 },
    { "NGC 4649",  60 },
    { "NGC 4303",  61 },
    { "NGC 6266",  62 },
    { "NGC 5055",  63 },    // Sunflower Galaxy
    { "NGC 4826",  64 },    // Black Eye Galaxy
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
    { "NGC 650",   76 },    // Little Dumbbell Nebula
    { "NGC 651",   76 },    // Little Dumbbell Nebula
    { "NGC 1068",  77 },    // Cetus A
    { "NGC 2068",  78 },
    { "NGC 1904",  79 },
    { "NGC 6093",  80 },
    { "NGC 3031",  81 },    // Bode's Galaxy
    { "NGC 3034",  82 },    // Cigar Galaxy
    { "NGC 5236",  83 },
    { "NGC 4374",  84 },    // Markarian's Chain
    { "NGC 4382",  85 },
    { "NGC 4406",  86 },    // Markarian's Chain
    { "NGC 4486",  87 },    // Virgo A
    { "NGC 4501",  88 },
    { "NGC 4552",  89 },
    { "NGC 4569",  90 },
    { "NGC 4548",  91 },
    { "NGC 6341",  92 },
    { "NGC 2447",  93 },
    { "NGC 4736",  94 },
    { "NGC 3351",  95 },
    { "NGC 3368",  96 },
    { "NGC 3587",  97 },    // Owl Nebula
    { "NGC 4192",  98 },
    { "NGC 4254",  99 },
    { "NGC 4321", 100 },
    { "NGC 5457", 101 },    // Pinwheel Galaxy
    { "NGC 5866", 102 },
    { "NGC 581",  103 },
    { "NGC 4594", 104 },    // Sombrero Galaxy
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
    { "NGC 188",    1 },
    { "NGC 40",     2 },    // Bow-Tie Nebula
    { "NGC 4236",   3 },
    { "NGC 7023",   4 },    // Iris Nebula
    { "IC 342",     5 },
    { "NGC 6543",   6 },    // Cat Eye Nebula
    { "NGC 2403",   7 },
    { "NGC 559",    8 },
    { "Sh2 155",    9 },    // Cave Nebula
    { "NGC 663",   10 },
    { "NGC 7635",  11 },    // Bubble Nebula
    { "NGC 6946",  12 },    // Fireworks Galaxy
    { "NGC 457",   13 },    // Owl Cluster
    { "NGC 869",   14 },    // Double Cluster
    { "NGC 884",   14 },    // Double Cluster
    { "NGC 6826",  15 },    // Blinking Planetary
    { "NGC 7243",  16 },
    { "NGC 147",   17 },
    { "NGC 185",   18 },
    { "IC 5146",   19 },
    { "NGC 7000",  20 },    // North America Nebula
    { "NGC 4449",  21 },
    { "NGC 7662",  22 },    // Blue Snowball
    { "NGC 891",   23 },
    { "NGC 1275",  24 },    // Perseus A
    { "NGC 2419",  25 },
    { "NGC 4244",  26 },
    { "NGC 6888",  27 },    // Crescent Nebula
    { "NGC 752",   28 },
    { "NGC 5005",  29 },
    { "NGC 7331",  30 },
    { "IC 405",    31 },    // Flaming Star Nebula
    { "NGC 4631",  32 },    // Whale Galaxy
    { "NGC 6992",  33 },    // Eastern Veil Nebula
    { "NGC 6960",  34 },    // Western Veil Nebula
    { "NGC 4889",  35 },
    { "NGC 4559",  36 },
    { "NGC 6885",  37 },
    { "NGC 4565",  38 },    // Needle Galaxy
    { "NGC 2392",  39 },    // Eskimo Nebula
    { "NGC 3626",  40 },
    { "NGC 3632",  40 },    // duplicate of NGC 3626
    { "Melotte 25",41 },    // Hyades
    { "NGC 7006",  42 },
    { "NGC 7814",  43 },
    { "NGC 7479",  44 },
    { "NGC 5248",  45 },
    { "NGC 2261",  46 },    // Hubble's Variable Nebula
    { "NGC 6934",  47 },
    { "NGC 2775",  48 },
    { "NGC 2237",  49 },    // Rosette Nebula
    { "NGC 2244",  50 },
    { "IC 1613",   51 },
    { "NGC 4697",  52 },
    { "NGC 3115",  53 },    // Spindle Galaxy
    { "NGC 2506",  54 },
    { "NGC 7009",  55 },    // Saturn Nebula
    { "NGC 246",   56 },
    { "NGC 6822",  57 },    // Barnard's Galaxy
    { "NGC 2360",  58 },
    { "NGC 3242",  59 },    // Ghost of Jupiter
    { "NGC 4038",  60 },    // Antennae Galaxies
    { "NGC 4039",  61 },    // Antennae Galaxies
    { "NGC 247",   62 },
    { "NGC 7293",  63 },    // Helix Nebula
    { "NGC 2362",  64 },
    { "NGC 253",   65 },    // Sculptor Galaxy
    { "NGC 5694",  66 },
    { "NGC 1097",  67 },
    { "NGC 6729",  68 },
    { "NGC 6302",  69 },    // Bug Nebula
    { "NGC 300",   70 },
    { "NGC 2477",  71 },
    { "NGC 55",    72 },
    { "NGC 1851",  73 },
    { "NGC 3132",  74 },    // Eight Burst Nebula
    { "NGC 6124",  75 },
    { "NGC 6231",  76 },
    { "NGC 5128",  77 },    // Centaurus A
    { "NGC 6541",  78 },
    { "NGC 3201",  79 },
    { "NGC 5139",  80 },    // Omega Centauri
    { "NGC 6352",  81 },
    { "NGC 6193",  82 },
    { "NGC 4945",  83 },
    { "NGC 5286",  84 },
    { "IC 2391",   85 },    // Omicron Velorum Cluster
    { "NGC 6397",  86 },
    { "NGC 1261",  87 },
    { "NGC 5823",  88 },
    { "NGC 6087",  89 },
    { "NGC 2867",  90 },
    { "NGC 3532",  91 },    // Wishing Well Cluster
    { "NGC 3372",  92 },    // Eta Carinae Nebula
    { "NGC 6752",  93 },
    { "NGC 4755",  94 },    // Jewel Box
    { "NGC 6025",  95 },
    { "NGC 2516",  96 },
    { "NGC 3766",  97 },    // Pearl Cluster
    { "NGC 4609",  98 },
    { "Coalsack",  99 },    // Coalsack
    { "IC 2944",  100 },    // Lambda Centauri Cluster
    { "NGC 6744", 101 },
    { "IC 2602",  102 },    // Southern Pleiades
    { "NGC 2070", 103 },    // Tarantula Nebula
    { "NGC 362",  104 },
    { "NGC 4833", 105 },
    { "NGC 104",  106 },    // 47 Tucanae
    { "NGC 6101", 107 },
    { "NGC 4372", 108 },
    { "NGC 3195", 109 }
};

// Add Messier and Caldwell numbers to a vector of identifiers
// from NGC-IC string mappings.

void addMCIdentifiers ( vector<SSIdentifier> &idents, string ngcicStr )
{
    int messNum = _messmap[ ngcicStr ];
    int caldNum = _caldmap[ ngcicStr ];

    if ( messNum > 0 )
        SSAddIdentifier ( SSIdentifier ( kCatMessier, messNum ), idents );

    if ( caldNum > 0 )
        SSAddIdentifier ( SSIdentifier ( kCatCaldwell, caldNum ), idents );
}

// Adds data from other deep sky catalogs to NGC-IC object data.
// Open clusters: adds proper motion, radial velocities, distances.
// Globular clusters: adds all of the above and spectral types.
// Planetary nebulae: adds all of the above, and PNG identifiers.

void addNGCICObjectData ( SSObjectVec &clusters, SSObjectVec &objects )
{
    SSObjectMap ngcMap = SSMakeObjectMap ( clusters, kCatNGC );
    SSObjectMap icMap = SSMakeObjectMap ( clusters, kCatIC );

    // For each NGC-IC object...
    
    for ( int i = 0; i < objects.size(); i++ )
    {
        // Get pointer to NGC-IC deep sky object.
        
        SSDeepSkyPtr pObject = SSGetDeepSkyPtr ( objects[i] );
        if ( pObject == nullptr )
            continue;

        // Find pointer to corresponding object in other object vector,
        // first using object's NGC identifier, then IC identifier.
        
        SSIdentifier ident = objects[i]->getIdentifier ( kCatNGC );
        SSDeepSkyPtr pCluster = SSGetDeepSkyPtr ( SSIdentifierToObject ( ident, ngcMap, clusters ) );
        
        if ( pCluster == nullptr )
        {
            SSIdentifier ident = objects[i]->getIdentifier ( kCatIC );
            pCluster = SSGetDeepSkyPtr ( SSIdentifierToObject ( ident, icMap, clusters ) );
        }
        
        // Continue if we don't find other corresponding object.
        
        if ( pCluster == nullptr )
            continue;
        
        // Get NGC-IC object coordinates, and cluster coordinates and motion.
        
        SSSpherical objCoords = pObject->getFundamentalCoords();
        SSSpherical clusCoords = pCluster->getFundamentalCoords();
        SSSpherical clusMotion = pCluster->getFundamentalMotion();

        // Copy cluster distance, proper motion, and radial velocity into NGC-IC object.
        
        objCoords.rad = clusCoords.rad;
        pObject->setFundamentalMotion ( objCoords, clusMotion );
        
        // For globular clusters also copy spectral type.
        
        if ( pCluster->getType() == kTypeGlobularCluster )
            pObject->setSpectralType ( pCluster->getSpectralType() );

        // For planetary nebulae, add PNG identifiers.
        
        if ( pCluster->getType() == kTypePlanetaryNebula )
            pObject->addIdentifier ( pCluster->getIdentifier ( kCatPNG ) );
    }
}

// Imports Wolfgang Steinicke's Revised NGC-IC Catalogs, obtained from:
// http://www.klima-luft.de/steinicke/index_e.htm with data files here:
// http://www.klima-luft.de/steinicke/ngcic/rev2000/NI2019.zip
// This is an Excel file; convert it to tab-delimited text first.
// Inserts name strings from nameNap; if empty, no names will be added.
// NGC-IC objects which are duplicates of other objects are discarded;
// instead, the other object receives an additional NGC or IC identifier
// of the duplicate. Nonexistent NGC-IC objects are imported as objects
// of kTypeNonexistent. The goal is to have every NGC-IC entry represented
// once, but not more than once.  This function also adds distances,
// proper motions, radial velocities from other catalogs of open and
// globulars clusters (clusters and globulars), planetary nebulae (planNebs).
// If these other catalogs are empty, no data will be added.
// The function stores imported objects in vector of SSObjects (objects).
// It returns the number of NGC-IC objects imported (13027 if successful).

int SSImportNGCIC ( const char *filename, SSIdentifierNameMap &nameMap, SSObjectVec &clusters, SSObjectVec &globulars, SSObjectVec &planNebs, SSObjectVec &objects )
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
        
        // Get object type from status. Status 7 and 8 are duplicates of other objects; skip them.
        // Status 9 indicates stars and status 10 (not found) are unknown/nonexistent objects.
        
        int status = strtoint ( tokens[5] );
        SSObjectType type = kTypeNonexistent;
        
        if ( status == 1 )
            type = kTypeGalaxy;
        else if ( status == 2 || status == 6 )
            type = kTypeBrightNebula;
        else if ( status == 3 )
            type = kTypePlanetaryNebula;
        else if ( status == 4 )
            type = kTypeOpenCluster;
        else if ( status == 5 )
            type = kTypeGlobularCluster;
        else if ( status == 9 )
            type = kTypeStar;
        else if ( status == 10 )
            type = kTypeNonexistent;
        else if ( status == 7 || status == 8 )
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

        // Get redshift and convert to radial velocity as fraction of light speed.
        
        motion.rad = tokens[23].empty() ? HUGE_VAL : SSCoordinates::redShiftToRadVel ( strtofloat ( tokens[23] ) );
        
        // Get distance in megaparsecs and convert to light years.  Prefer metric distance over redshift-derived.
        
        if ( ! tokens[25].empty() )
            coords.rad = strtofloat ( tokens[25] ) * 1.0e6 * SSCoordinates::kLYPerParsec;
        else if ( ! tokens[24].empty() )
            coords.rad = strtofloat ( tokens[24] ) * 1.0e6 * SSCoordinates::kLYPerParsec;
        
        // Get Hubble morphological type
        
        string strType = tokens[22];
        
        // Generate NGC-IC identifier, if present.
        
        if ( tokens[1].empty() )
            continue;
        
        // Generate main NGC or IC identifier
        // Add Messier and Caldwell identifiers from NGC-IC string mappings

        vector<SSIdentifier> idents;
        string ngcicStr = "";
        
        if ( tokens[0][0] == 'N' )
            ngcicStr = "NGC " + tokens[1] + tokens[2];
        else if ( tokens[0][0] == 'I' )
            ngcicStr = "IC " + tokens[1] + tokens[2];

        SSAddIdentifier ( SSIdentifier::fromString ( ngcicStr ), idents );
        addMCIdentifiers ( idents, ngcicStr );

        // Get Principal Galaxy Catalog number, if any.
        
        if ( ! tokens[26].empty() )
            SSAddIdentifier ( SSIdentifier ( kCatPGC, strtoint ( tokens[26] ) ), idents );

        // Get additional identifiers from remaining tokens.
        
        for ( int k = 27; k < tokens.size(); k++ )
            if ( ! tokens[k].empty() )
                SSAddIdentifier ( SSIdentifier::fromString ( tokens[k] ), idents );
        
        // get names from identifiers.  Sort identifier list.
        
        vector<string> names = SSIdentifiersToNames ( idents, nameMap );
        sort ( idents.begin(), idents.end(), compareSSIdentifiers );

        SSDeepSky *pObject = new SSDeepSky ( type );
        if ( pObject == nullptr )
            continue;
        
        // Allocate new deep sky object and store values if successful

        pObject->setNames ( names );
        pObject->setIdentifiers ( idents );
        pObject->setFundamentalMotion ( coords, motion );
        pObject->setVMagnitude ( vmag );
        pObject->setBMagnitude ( bmag );
        pObject->setGalaxyType ( strType );
        pObject->setMajorAxis ( sizeX );
        pObject->setMinorAxis ( sizeY );
        pObject->setPositionAngle ( pa );
        
        // cout << pObject->toCSV() << endl;
        objects.push_back ( pObject );
        numObjects++;
    }

    // Now add propper motions, distances, radial velocities, etc.
    // from other deep sky object catalogs, if we have them.
    
    addNGCICObjectData ( clusters, objects );
    addNGCICObjectData ( globulars, objects );
    addNGCICObjectData ( planNebs, objects );

    // Return number of objects imported.  File closed automatically.
    
    return numObjects;
}

// Imports Wilton Dias "Open Cluster and Galactic Structure" catalog:
// https://wilton.unifei.edu.br/ocdb/clusters.txt
// Adds names from input deep sky object name table.
// Adds Messier and Caldwell numbers when possible.
// Returns total number of clusters imported (should be 2167).

int SSImportDAML02 ( const char *filename, SSIdentifierNameMap &nameMap, SSObjectVec &clusters )
{
    // Open file; return on failure.

    ifstream file ( filename );
    if ( ! file )
        return ( 0 );

    // Read file line-by-line until we reach end-of-file

    string line = "";
    int numClusters = 0;

    while ( getline ( file, line ) )
    {
        // Get R.A. and Dec; convert to radians
        
        string strRA = line.substr ( 18, 8 );
        string strDec = line.substr ( 28, 9 );
        
        SSHourMinSec ra ( strRA );
        SSDegMinSec dec ( strDec );
        
        SSSpherical coords ( SSAngle ( ra ), SSAngle ( dec ), HUGE_VAL );
        SSSpherical motion ( HUGE_VAL, HUGE_VAL, HUGE_VAL );

        // Get proper motion in R.A. and convert to radians/year

        string strPMRA = trim ( line.substr ( 84, 6 ) );
        if ( ! strPMRA.empty() )
            motion.lon = SSAngle::fromArcsec ( strtofloat ( strPMRA ) / 1000.0 ) / cos ( coords.lat );

        // Get proper motion in Dec. and convert to radians/year

        string strPMDec = trim ( line.substr ( 100, 6 ) );
        if ( ! strPMDec.empty() )
            motion.lat = SSAngle::fromArcsec ( strtofloat ( strPMDec ) / 1000.0 );

        // Get radial velocity in km/sec and convert to fraction of light speed
        
        string strRV = trim ( line.substr ( 127, 6 ) );
        if ( ! strRV.empty() )
            motion.rad = strtofloat ( strRV ) / SSCoordinates::kLightKmPerSec;

        // Get distance in parsecs and convert to light years

        string strDist = trim ( line.substr ( 55, 5 ) );
        if ( ! strDist.empty() )
            coords.rad = strtofloat ( strDist ) * SSCoordinates::kLYPerParsec;
        
        // Get angular diameter in arcmin and convert to radians
        
        string strDiam = trim ( line.substr ( 46, 5 ) );
        float diam = strDiam.empty() ? HUGE_VAL : degtorad ( strtofloat ( strDiam ) / 60.0 );

        // Get name. Attempt to parse identifier from it.  If we recognize the name
        // as an identifier, add Messier and Caldwell numbers; get names from identifiers,
        // sort identifier list. If we can't parse name as an identifier, use it verbatim as name.

        vector<string> names;
        vector<SSIdentifier> idents;
        
        string name = trim ( line.substr ( 0, 18 ) );
        SSIdentifier ident = SSIdentifier::fromString ( name );
        
        if ( ident )
        {
            idents.push_back ( ident );
            addMCIdentifiers ( idents, name );
            names = SSIdentifiersToNames ( idents, nameMap );
            sort ( idents.begin(), idents.end(), compareSSIdentifiers );
        }
        else
        {
            names.push_back ( name );
        }

        // Allocate new deep sky object and store values if successful
        
        SSDeepSky *pObject = new SSDeepSky ( kTypeOpenCluster );
        if ( pObject == nullptr )
            continue;
        
        pObject->setNames ( names );
        pObject->setIdentifiers ( idents );
        pObject->setFundamentalMotion ( coords, motion );
        pObject->setMajorAxis ( diam );

        // cout << pObject->toCSV() << endl;
        clusters.push_back ( pObject );
        numClusters++;
    }
    
    return numClusters;
}

// Imports William Harris "Globular Clusters in the Milky Way" catalog:
// http://physwww.mcmaster.ca/~harris/mwgc.dat
// Note this file is in three parts; this function assumes that
// consolidated information for each object on a single line,
// so you'll need to manually rearrange the original Harris file!
// This function adds names from input deep sky object name table,
// and adds Messier and Caldwell numbers when possible.
// Returns total number of clusters imported (should be 157).

int SSImportMWGC ( const char *filename, SSIdentifierNameMap &nameMap, SSObjectVec &clusters )
{
    // Open file; return on failure.

    ifstream file ( filename );
    if ( ! file )
        return ( 0 );

    // Read file line-by-line until we reach end-of-file

    string line = "";
    int numClusters = 0;

    while ( getline ( file, line ) )
    {
        // Get R.A. and Dec; convert to radians
        
        string strRA = line.substr ( 24, 11 );
        string strDec = line.substr ( 37, 11 );
        
        SSHourMinSec ra ( strRA );
        SSDegMinSec dec ( strDec );
        
        SSSpherical coords ( SSAngle ( ra ), SSAngle ( dec ), HUGE_VAL );
        SSSpherical motion ( HUGE_VAL, HUGE_VAL, HUGE_VAL );

        // Get V magnitude
        
        string strVmag = trim ( line.substr ( 126, 5 ) );
        float vmag = strVmag.empty() ? HUGE_VAL : strtofloat ( strVmag );
        
        // Get B magnitude from color index
        
        string strBmV = trim ( line.substr ( 147, 4 ) );
        float bmag = strBmV.empty() ? HUGE_VAL : strtofloat ( strBmV ) + vmag;
            
        // Get radial velocity in km/sec and convert to fraction of light speed
        
        string strRV = trim ( line.substr ( 177, 6 ) );
        if ( ! strRV.empty() )
            motion.rad = strtofloat ( strRV ) / SSCoordinates::kLightKmPerSec;

        // Get distance in kiloparsecs and convert to light years

        string strDist = trim ( line.substr ( 67, 5 ) );
        if ( ! strDist.empty() )
            coords.rad = strtofloat ( strDist ) * 1000.0 * SSCoordinates::kLYPerParsec;
        
        // Get half-light radius in arcmin and convert to diameter in radians
        
        string strRad = trim ( line.substr ( 230, 4 ) );
        float diam = strRad.empty() ? HUGE_VAL : 2.0 * degtorad ( strtofloat ( strRad ) / 60.0 );

        // Get spectral type
        
        string specStr = trim ( line.substr ( 165, 4 ) );

        // Get name. Attempt to parse identifier from it.  If we recognize the name
        // as an identifier, add Messier and Caldwell numbers; get names from identifiers,
        // sort identifier list. If we can't parse name as an identifier, use it verbatim as name.

        vector<string> names;
        vector<SSIdentifier> idents;

        string name = trim ( line.substr ( 0, 9 ) );
        SSIdentifier ident = SSIdentifier::fromString ( name );

        if ( ident )
        {
            idents.push_back ( ident );
            addMCIdentifiers ( idents, name );
            names = SSIdentifiersToNames ( idents, nameMap );
            sort ( idents.begin(), idents.end(), compareSSIdentifiers );
        }
        else
        {
            names.push_back ( name );
        }

        // Allocate new deep sky object and store values if successful
        
        SSDeepSky *pObject = new SSDeepSky ( kTypeGlobularCluster );
        if ( pObject == nullptr )
            continue;
        
        pObject->setNames ( names );
        pObject->setIdentifiers ( idents );
        pObject->setFundamentalMotion ( coords, motion );
        pObject->setVMagnitude ( vmag );
        pObject->setBMagnitude ( bmag );
        pObject->setMajorAxis ( diam );
        pObject->setSpectralType ( specStr );
        
        // cout << pObject->toCSV() << endl;
        clusters.push_back ( pObject );
        numClusters++;
    }

    return numClusters;
}

// Imports Strasbourg-ESO catalog of Galactic Planetary Nebulae:
// https://cdsarc.unistra.fr/ftp/V/84
// https://cdsarc.unistra.fr/ftp/V/84/main.dat.gz
// This catalog contains a main file with identifiers and B1950 coordinates;
// distances, diameters, and velocities are in auxiliary files.
// This function blends them all (but only main file is required);
// it adds names from input deep sky object name table,
// and adds Messier and Caldwell numbers when possible.
// Returns total number of planetary nebule imported (should be 1143).

int SSImportPNG ( const char *main_filename, const char *dist_filename, const char *diam_filename, const char *vel_filename, SSIdentifierNameMap &nameMap, SSObjectVec &nebulae )
{
    string line = "";
    map<SSIdentifier,float> distMap;
    map<SSIdentifier,float> diamMap;
    map<SSIdentifier,float> velMap;

    // First open distance distance file.
    // If successful create mapping of PNG identifiers to distances.
    
    ifstream file ( dist_filename );
    if ( file )
    {
        int n = 0;
        SSIdentifier lastIdent ( kCatUnknown, 0 );
        
        // Read file line-by-line until we reach end-of-file
        
        while ( getline ( file, line ) )
        {
            if ( line.length() < 28 )
                continue;

            // Get PNG identifier
            
            string strPNG = trim ( line.substr ( 0, 10 ) );
            SSIdentifier ident = SSIdentifier::fromString ( "PNG " + strPNG );
            if ( ! ident )
                continue;
            
            // Get distance in kiloparsecs and convert to light years
            
            string distStr = trim ( line.substr ( 22, 6 ) );
            float dist = distStr.empty() ? HUGE_VAL : strtofloat ( distStr ) * 1000.0 * SSCoordinates::kLYPerParsec;
            if ( isinf ( dist ) )
                continue;
            
            // Distance file contains multiple distance estimates for each object on consecutive lines;
            // we average them into a single estimate for each individual PNG object.

            if ( ident == lastIdent )
            {
                distMap[ident] = ( distMap[ident] * n + dist ) / ( n + 1 );
                n++;
            }
            else
            {
                distMap[ident] = dist;
                n = 1;
            }
            
            lastIdent = ident;
        }
    }

    // Close previous file; open diameter file.
    // If successful create mapping of PNG identifiers to angular diameters.

    file.close();
    file.open ( diam_filename );
    if ( file )
    {
        // Read file line-by-line until we reach end-of-file
        
        while ( getline ( file, line ) )
        {
            if ( line.length() < 18 )
                continue;
            
            // Get PNG identifier
            
            string strPNG = trim ( line.substr ( 0, 10 ) );
            SSIdentifier ident = SSIdentifier::fromString ( "PNG " + strPNG );
            if ( ! ident )
                continue;
            
            // Get angular diameter in arcsec and convert to radians
            // If nonzero, store diameter in mapping of PNG identifiers

            string diamStr = trim ( line.substr ( 12, 6 ) );
            float diam = strtofloat ( diamStr ) * SSAngle::kRadPerArcsec;
            if ( diam != 0.0 )
                diamMap[ident] = diam;
        }
    }

    // Close previous file; open velocity file.
    // If successful create mapping of PNG identifiers to radial velocities.

    file.close();
    file.open ( vel_filename );
    if ( file )
    {
        // Read file line-by-line until we reach end-of-file
        
        while ( getline ( file, line ) )
        {
            if ( line.length() < 18 )
                continue;

            // Get PNG identifier
            
            string strPNG = trim ( line.substr ( 1, 10 ) );
            SSIdentifier ident = SSIdentifier::fromString ( "PNG " + strPNG );
            if ( ! ident )
                continue;
            
            // Get radial velocity in km/sec and convert to fraction of light speed
            // If valid, store radial velocity in mapping of PNG identifiers

            string velStr = trim ( line.substr ( 12, 6 ) );
            float radVel = strtofloat ( velStr ) / SSCoordinates::kLightKmPerSec;
            if ( radVel != 0.0 )
                velMap[ident] = radVel;
        }
    }

    // Close previous file; open main file and return on failure.

    file.close();
    file.open ( main_filename );
    if ( ! file )
        return 0;
    
    // Set up matrix for precessing B1950 coordinates and proper motion to J2000.
    // Read file line-by-line until we reach end-of-file.

    SSMatrix precession = SSCoordinates::getPrecessionMatrix ( SSTime::kB1950 ).transpose();
    int numNebulae = 0;

    while ( getline ( file, line ) )
    {
        if ( line.length() < 58 )
            continue;

        // Get B1950 R.A. and Dec; convert to radians.
        
        string strRA = line.substr ( 12, 11 );
        string strDec = line.substr ( 23, 11 );
        
        SSHourMinSec ra ( strRA );
        SSDegMinSec dec ( strDec );
        
        // Precess B1950 coords and motion to J2000!

        SSSpherical coords ( SSAngle ( ra ), SSAngle ( dec ), HUGE_VAL );
        SSSpherical motion ( HUGE_VAL, HUGE_VAL, HUGE_VAL );

        SSUpdateStarCoordsAndMotion ( 2000.0, &precession, coords, motion );

        // Get PNG and PK identifiers
        
        vector<string> names;
        vector<SSIdentifier> idents;
        
        string strPNG = trim ( line.substr ( 0, 10 ) );
        if ( ! strPNG.empty() )
            idents.push_back ( SSIdentifier::fromString ( "PNG " + strPNG ) );
    
        string strPK = trim ( line.substr ( 59, 9 ) );
        if ( ! strPK.empty() )
            idents.push_back ( SSIdentifier::fromString ( "PK " + strPK ) );

        // Use distance from PNG-identifier-to-distance mapping created above, if nonzero.
        
        float dist = distMap[ idents[0] ];
        if ( dist != 0.0 )
            coords.rad = dist;
        
        // Use angular diameter from PNG-identifier-to-distance mapping created above, if nonzero.
        
        float diam = diamMap[ idents[0] ];
        if ( diam == 0.0 )
            diam = HUGE_VAL;

        // Use radial velocity from PNG-identifier-to-velocity mapping created above, if nonzero.
        
        float radVel = velMap[ idents[0] ];
        if ( radVel != 0.0 )
            motion.rad = radVel;

        // Get name. If it's an NGC-IC, parse identifier from it, add Messier and Caldwell numbers;
        // get names from identifiers, sort identifier list. Otherwise, use name verbatim.

        string name = trim ( line.substr ( 45, 13 ) );
        
        if ( name.find ( "NGC" ) == 0 || name.find ( "IC" ) == 0 )
        {
            SSIdentifier ident = SSIdentifier::fromString ( name );
            idents.push_back ( ident );
            addMCIdentifiers ( idents, name );
            names = SSIdentifiersToNames ( idents, nameMap );
            sort ( idents.begin(), idents.end(), compareSSIdentifiers );
        }
        else
        {
            names.push_back ( name );
        }

        // Allocate new deep sky object and store values if successful
        
        SSDeepSky *pObject = new SSDeepSky ( kTypePlanetaryNebula );
        if ( pObject == nullptr )
            continue;
        
        pObject->setNames ( names );
        pObject->setIdentifiers ( idents );
        pObject->setFundamentalMotion ( coords, motion );
        pObject->setMajorAxis ( diam );
        
        // cout << pObject->toCSV() << endl;
        nebulae.push_back ( pObject );
        numNebulae++;
    }
    
    return numNebulae;
}
