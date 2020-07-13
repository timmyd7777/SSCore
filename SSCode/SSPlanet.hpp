// SSPlanet.hpp
// SSCore
//
// Created by Tim DeBenedictis on 3/15/20.
// Copyright © 2020 Southern Stars. All rights reserved.
//
// This subclass of SSObject represents all solar system objects (not just planets,
// but also moons, asteroids, comets, satellites, etc.)

#ifndef SSPlanet_hpp
#define SSPlanet_hpp

#include "SSObject.hpp"
#include "SSOrbit.hpp"
#include "SSCoordinates.hpp"
#include "SSTLE.hpp"

enum SSPlanetID
{
    kSun = 0,
    kMercury = 1,
    kVenus = 2,
    kEarth = 3,
    kMars = 4,
    kJupiter = 5,
    kSaturn = 6,
    kUranus = 7,
    kNeptune = 8,
    kPluto = 9,

    kLuna = 301,
    
    kPhobos = 401,
    kDeimos = 402,
    
    kIo = 501,
    kEuropa = 502,
    kGanymede = 503,
    kCallisto = 504,
    
    kMimas = 601,
    kEnceladus = 602,
    kTethys = 603,
    kDione = 604,
    kRhea = 605,
    kTitan = 606,
    kHyperion = 607,
    kIapetus = 608,
    kPhoebe = 609,
    
    kMiranda = 701,
    kAriel = 702,
    kUmbriel = 703,
    kTitania = 704,
    kOberon = 705,

    kTriton = 801,
    kNereid = 802,
    
    kCharon = 901
};

// This subclass of SSObject stores data for all solar system objects:
// major planets, moons, asteroids, comets, and artificial satellites,
// with a special subclass SSSatellite for the latter.

class SSPlanet : public SSObject
{
protected:

    SSIdentifier _id;           // planet/moon/asteroid/comet/satellite identifier
    SSOrbit     _orbit;         // current orbital elements
    float       _Hmag;          // absolute magnitude; infinite if unknown
    float       _Gmag;          // magnitude slope parameter; infinite if unknown
    float       _radius;        // equatorial radius in kilometers; 0 or infinite if unknown
    SSVector    _position;      // current heliocentric position in fundamental frame in AU
    SSVector    _velocity;      // current heliocentric velocity in fundamental frame in AU per day

    void computeMinorPlanetPositionVelocity ( double jed, double lt, SSVector &pos, SSVector &vel );
    void computeMoonPositionVelocity ( double jed, double lt, SSVector &pos, SSVector &vel );
    static void computePSPlanetMoonPositionVelocity ( int id, double jed, double lt, SSVector &pos, SSVector &vel );

    float computeAsteroidMagnitude ( double rad, double dist, double phase, double hmag, double gmag );
    float computeCometMagnitude ( double rad, double dist, double hmag, double kmag );

public:
    
    SSPlanet ( SSObjectType type );
    SSPlanet ( SSObjectType type, SSPlanetID id );
    
    void setIdentifier ( SSIdentifier ident ) { _id = ident; }
    void setOrbit ( SSOrbit orbit ) { _orbit = orbit; }
    void setHMagnitude ( float hmag ) { _Hmag = hmag; }
    void setGMagnitude ( float gmag ) { _Gmag = gmag; }
    void setRadius ( float radius ) { _radius = radius; }

    SSIdentifier getIdentifier ( void ) { return _id; }
    SSIdentifier getIdentifier ( SSCatalog cat );
    SSOrbit getOrbit ( void ) { return _orbit; }
    float getHMagnitude ( void ) { return _Hmag; }
    float getGMagnitude ( void ) { return _Gmag; }
    float getRadius ( void ) { return _radius; }
    
    SSVector getPosition ( void ) { return _position; }
    SSVector getVelocity ( void ) { return _velocity; }

    static double angularRadius ( double radius, double distance );
    double angularRadius ( void );

    static double phaseAngle ( SSVector position, SSVector direction );
    double phaseAngle ( void );
    
    static double illumination ( double phase );
    double illumination ( void );
    
    void rotationElements ( double jed, double &a0, double &d0, double &w0, double &wdot );
    SSMatrix planetographicMatrix ( double jed );

    
    // Sets whether to use (accurate, but slow) VSOP/ELP planetary & lunar ephemeris when JPL DE438 is not available.
    // Also USE_VSOP_ELP must be #defined as 1 at the top of SSPlanet.cpp!
    
    static void useVSOPELP ( bool use );
    static bool useVSOPELP ( void );
    double flattening ( void );

    static void computeMajorPlanetPositionVelocity ( int id, double jed, double lt, SSVector &pos, SSVector &vel );
    virtual void computePositionVelocity ( double jed, double lt, SSVector &pos, SSVector &vel );
    virtual float computeMagnitude ( double rad, double dist, double phase );
    virtual void computeEphemeris ( SSCoordinates &coords );
    
    // imports/exports from/to CSV-format text string
    
    static SSObjectPtr fromCSV ( string csv );
    string toCSV ( void );
};

// Subclass of solar system object for artificial Earth satellites.
// Includes methods for computing position and velocity from TLE elements via SGP4/SDP4 orbit model
// and computing visual magnitude with McCants formula.

class SSSatellite : public SSPlanet
{
protected:
    SSTLE _tle;
    
public:
    
    SSSatellite ( SSTLE &tle );
    
    SSTLE getTLE ( void ) { return _tle; }

    virtual void  computePositionVelocity ( double jed, double lt, SSVector &pos, SSVector &vel );
    virtual float computeMagnitude ( double rad, double dist, double phase );
    static  float computeSatelliteMagnitude ( double dist, double phase, double stdmag );
};

// convenient aliases for pointers to various subclasses of SSPlanet

typedef SSPlanet *SSPlanetPtr;
typedef SSSatellite *SSSatellitePtr;

// Downcasts a pointer from SSObject base class to SSPlanet or SSSatellite subclass.
// Returns nullptr if input object pointer is not actually an SSPlanet or SSSatellite.

SSPlanetPtr SSGetPlanetPtr ( SSObjectPtr ptr );
SSSatellitePtr SSGetSatellitePtr ( SSObjectPtr ptr );

// Holds satellite size and magnitude data from Mike McCants' satellite names file.

struct McName
{
    int    norad;         // NORAD number
    string name;          // Spacecraft name
    float  len, wid, dep; // Dimensions in meters
    float  mag;           // Magnitude at 1000 km range, 50% illumination.
};

typedef map<int,McName> McNameMap;

int SSImportSatellitesFromTLE ( const string &path, SSObjectVec &satellites );
int SSImportMcNames ( const string &path, McNameMap &mcnames );
int SSImportMcNames ( const string &filename, SSObjectVec &objects );

// Struct used to store CSV-parsed data from amateur satellite frequency table

struct SatFreqData
{
    int    norad;             // NORAD tracking number
    string name;              // satellite name
    string uplink;            // uplink frequency(ies), MHz
    string downlink;          // downlink frequency(ies), MHz
    string beacon;            // beacon frequency, MHz
    string mode;              // modulation scheme and rate, if known
    string callsign;          // callsign
    string status;            // current status
};

typedef map<int,vector<SatFreqData>> SatFreqMap;

int SSImportSatelliteFrequencyData ( const string &path, SatFreqMap &satfreqs );
int SSImportSatelliteFrequencyData ( const string &path, SSObjectVec &objects );

#endif /* SSPlanet_hpp */
