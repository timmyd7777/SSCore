// SSIdentifier.hpp
// SSCore
//
// Created by Tim DeBenedictis on 3/20/20.
// Copyright © 2020 Southern Stars. All rights reserved.
//
// This class represents object identifiers used in a wide variety of astronomical catalogs
// with a unified system of 64-bit integers, and contains methods for converting identifiers
// from string representations ("HR 7001", "NGC 1976", etc.) to 64-bit integers and vice-versa.

#ifndef SSIdentifier_hpp
#define SSIdentifier_hpp

#include <string>
#include <map>

using namespace std;

// Recognized astronomical object types

enum SSObjectType
{
    kTypeNonexistent = 0,            // Nonexistent/unknown object or erroneous catalog entry
    kTypePlanet = 1,                 // Major planet (Mercury, Venus, etc.)
    kTypeMoon = 2,                   // Natural satellite (Moon, Io, Europa, etc.)
    kTypeAsteroid = 3,               // Minor planet (Ceres, Pallas, etc.)
    kTypeComet = 4,                  // Comet (Halley, Encke, etc.)
    kTypeSatellite = 5,              // Artificial satellite (ISS, HST, etc.)
    kTypeSpacecraft = 6,             // Interplanetary spacecraft (Voyager, Cassini, etc.)
    kTypeFeature = 7,                // Planetary surface features (Tycho, Jezero, etc.)
    kTypeCity = 8,                   // City (San Francisco, New York City, etc.)
    kTypeStar = 10,                  // Single star (Canopus, Vega, etc.)
    kTypeDoubleStar = 12,            // Double star (Alpha Cen, Sirius, etc.)
    kTypeVariableStar = 13,          // Variable single star (Mira, etc.)
    kTypeDoubleVariableStar = 14,    // Double star with variable component (Betelgeuse, Algol, etc.)
    kTypeOpenCluster = 20,           // Open star cluster (M45, Hyades, etc.)
    kTypeGlobularCluster = 21,       // Globular star cluster (M13, etc.)
    kTypeBrightNebula = 22,          // Emission, reflection nebula or supernova remnant (M42, M78, M1, etc.)
    kTypeDarkNebula = 23,            // Dark nebula (Coalsack, Horsehead, etc.)
    kTypePlanetaryNebula = 24,       // Planetary nebula (M57, M27, etc.)
    kTypeGalaxy = 25,                // Galaxy (M31, LMC, SMC, etc.)
    kTypeConstellation = 30,         // Constellation officially recognized by IAU (Andromeda, Antlia, etc.)
    kTypeAsterism = 31,              // Common but informally recognized star pattern (Big Dipper, Summer Triangle, etc.)
};

// Recognized astronomical object catalogs

enum SSCatalog
{
    kCatUnknown = 0,        // Unknown catalog
    
    kCatJPLanet = 1,        // JPL NAIF planet/moon identifiers (Mercury = 1, Venus = 2, etc.)
    kCatAstNum = 2,         // Numbered asteroids (Ceres = 1, Pallas = 2, etc.)
    kCatComNum = 3,         // Numbered periodic comets (Halley = 1, Encke = 2, etc.)
    kCatNORADSat = 4,       // NORAD satellite catalog (ISS = 25544, HST = 20580, etc.)
    
    kCatBayer = 10,         // Bayer star letters (Alpha CMa, etc.)
    kCatFlamsteed = 11,     // Flamsteed star numbers (9 CMa, etc.)
    kCatGCVS = 12,          // General Catalog of Variable Stars (R And, etc.)
    kCatHR = 13,            // Harvard Revised (Yale Bright Star) catalog
    kCatGJ = 14,            // Gliese-Jahreiss Catalog of Nearby Stars
    kCatHD = 15,            // Henry Draper star catalog
    kCatSAO = 16,           // Sminthsonian Astrophysical Observatory star catalog
    kCatBD = 17,            // Bonner Durchmusterung star catalog
    kCatCD = 18,            // Cordoba Durchmusterung star catalog
    kCatCP = 19,            // Cape Photographic Durchmusterung star catalog
    kCatHIP = 20,           // Hipparcos star catalog
    kCatWDS = 21,           // Washington Double Star catalog
    
    kCatMessier = 30,       // Messier deep sky objects
    kCatCaldwell = 31,      // Caldwell deep sky objects
    kCatNGC = 32,           // New General Catalog of deep sky objects
    kCatIC = 33,            // Index Catalog of deep sky objects
    kCatMel = 34,           // Melotte Catalog of open clusters
    kCatSh2 = 35,           // Sharpless Bright Nebula catalog
    kCatLBN = 36,           // Lynds Bright Nebula catalog
    kCatLDN = 37,           // Lynds Dark Nebula catalog
    kCatPNG = 38,           // Galactic Planetary Nebulae (Strasbourg-ESO)
    kCatPK = 39,            // Perek-Kohoutek Catalog (planetary nebulae)
    kCatPGC = 40,           // Principal Galaxy Catalog
    kCatUGC = 41,           // Uppsala Galaxy Catalog
    kCatUGCA = 42           // Uppsala Galaxy Catalog Appendix
};

class SSIdentifier
{
protected:
    
    int64_t        _id;     // catalog identifier encoded as a 64-bit integer
    
public:
    
    SSIdentifier ( void );
    SSIdentifier ( int64_t ident );
    SSIdentifier ( SSCatalog catalog, int64_t id );
    
    SSCatalog catalog ( void );
    int64_t identifier ( void );
    
    string toString ( void );
    static SSIdentifier fromString ( const string &s, SSObjectType type = kTypeNonexistent, bool casesens = true );
    
    bool operator > ( SSIdentifier other ) { return _id > other._id; }
    bool operator < ( SSIdentifier &other ) const { return _id < other._id; }
    operator int64_t() const { return _id; }
    
    static SSIdentifier fromBayer ( string s );
    static SSIdentifier fromFlamsteed ( string s );
    static SSIdentifier fromGCVS ( string s );
    static SSIdentifier fromHR ( string s );
    static SSIdentifier fromHD ( string s );
    static SSIdentifier fromSAO ( string s );
    static SSIdentifier fromBD ( string s );
    static SSIdentifier fromCD ( string s );
    static SSIdentifier fromCP ( string s );
    static SSIdentifier fromHIP ( string s );
    
    static SSIdentifier fromMessier ( string s );
    static SSIdentifier fromCaldwell ( string s );
    static SSIdentifier fromNGC ( string s );
    static SSIdentifier fromIC ( string s );
    
    SSIdentifier strip ( void );
};

typedef vector<SSIdentifier> SSIdentifierVec;
typedef multimap<SSIdentifier,SSIdentifier> SSIdentifierMap;
typedef multimap<SSIdentifier,string> SSIdentifierNameMap;

int SSImportIdentifierNameMap ( const string &filename, SSIdentifierNameMap &nameMap );
vector<string> SSIdentifiersToNames ( SSIdentifierVec &idents, SSIdentifierNameMap &nameMap );

bool compareSSIdentifiers ( const SSIdentifier &id1, const SSIdentifier &id2 );

bool SSAddIdentifier ( SSIdentifier ident, vector<SSIdentifier> &identVec );
int SSAddIdentifiers ( SSIdentifier ident, SSIdentifierMap &map, SSIdentifierVec &idents );

string catalog_to_string ( SSCatalog cat );
SSCatalog string_to_catalog ( string str );

string con_to_string ( int con );
string bayer_to_string ( int64_t bay );

#endif /* SSIdentifier_hpp */
