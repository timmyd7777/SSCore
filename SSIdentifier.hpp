//  SSIdentifier.hpp
//  SSCore
//
//  Created by Tim DeBenedictis on 3/20/20.
//  Copyright © 2020 Southern Stars. All rights reserved.
//  This class represents object identifiers in a wide variety of astronomical catalogs.

#ifndef SSIdentifier_hpp
#define SSIdentifier_hpp

#include <string>

using namespace std;

enum SSCatalog
{
	kCatUnknown = 0,		// Unknown catalog
	
	kCatJPLanet = 1,		// JPL planet/moon identifiers (Mercury = 1, Venus = 2, etc.)
	kCatAstNum = 2,			// Numbered asteroids (Ceres = 1, Pallas = 2, etc.)
	kCatComNum = 3,			// Numbered periodic comets (Halley = 1, Encke = 2, etc.)
	kCatNORADSat = 4,		// NORAD satellite catalog (ISS = 25544, HST = 20580, etc.)
	
	kCatBayer = 10,			// Bayer star letters (Alpha CMa, etc.)
	kCatFlamsteed = 11,		// Flamsteed star numbers (9 CMa, etc.)
	kCatGCVS = 12,			// General Catalog of Variable Stars (R And, etc.)
	kCatHR = 12,			// Harvard Revised (Yale Bright Star) catalog
	kCatHD = 13,			// Henry Draper star catalog
	kCatSAO = 14,			// Sminthsonian Astrophysical Observatory star catalog
	kCatBD = 15,			// Bonner Durchmusterung star catalog
	kCatCD = 16,			// Cordoba Durchmusterung star catalog
	kCatCP = 17,			// Cape Photographic Durchmusterung star catalog
	kCatHIP = 18,			// Hipparcos star catalog
	
	kCatMessier = 20,		// Messier deep sky objects
	kCatCaldwell = 21,		// Caldwell deep sky objects
	kCatNGC = 22,			// New General Catalog of deep sky objects
	kCatIC = 23				// Index Catalog of deep sky objects
};

class SSIdentifier
{
protected:
	
	int64_t		_id;		// catalog identifier encoded as a 64-bit integer
	
public:
	
	SSIdentifier ( void );
	SSIdentifier ( SSCatalog catalog, int64_t id );
	
	SSCatalog catalog ( void );
	int64_t identifier ( void );
	
	string toString ( void );
	static SSIdentifier fromString ( string s );
	
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

#endif /* SSIdentifier_hpp */
