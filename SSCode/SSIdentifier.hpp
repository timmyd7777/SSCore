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
    kCatLBN = 35,           // Lynds Bright Nebula catalog
    kCatPNG = 36,           // Galactic Planetary Nebulae (Strasbourg-ESO)
    kCatPK = 37,            // Perek-Kohoutek Catalog (planetary nebulae)
    kCatPGC = 38,           // Principal Galaxy Catalog
    kCatUGC = 39,           // Uppsala Galaxy Catalog
    kCatUGCA = 40           // Uppsala Galaxy Catalog Appendix
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
    static SSIdentifier fromString ( const string &s );
    
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
};

typedef vector<SSIdentifier> SSIdentifierVec;
typedef multimap<SSIdentifier,SSIdentifier> SSIdentifierMap;
typedef multimap<SSIdentifier,string> SSIdentifierNameMap;

int SSImportIdentifierNameMap ( const char *filename, SSIdentifierNameMap &nameMap );
vector<string> SSIdentifiersToNames ( SSIdentifierVec &idents, SSIdentifierNameMap &nameMap );

bool compareSSIdentifiers ( const SSIdentifier &id1, const SSIdentifier &id2 );

bool SSAddIdentifier ( SSIdentifier ident, vector<SSIdentifier> &identVec );
int SSAddIdentifiers ( SSIdentifier ident, SSIdentifierMap &map, SSIdentifierVec &idents );

#endif /* SSIdentifier_hpp */
