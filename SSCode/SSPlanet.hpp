// SSPlanet.hpp
// SSCore
//
// Created by Tim DeBenedictis on 3/15/20.
// Copyright © 2020 Southern Stars. All rights reserved.
//
// This subclass of SSObject represents all solar system objects (not just planets,
// but also moons, asteroids, comets, satellites, etc.) Includes methods for computing
// solar system object positions, velocities, magnitudes, sizes, and rotational parameters.

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
    float       _mass;          // mass in Earth masses, 0 or infinite if unknown
    SSVector    _position;      // current heliocentric position in fundamental frame in AU
    SSVector    _velocity;      // current heliocentric velocity in fundamental frame in AU per day
    SSMatrix    _pmatrix;       // transforms from planetographic to fundamental J2000 mean equatorial frame.
    
    void computeMinorPlanetPositionVelocity ( double jed, double lt, SSVector &pos, SSVector &vel );
    void computeMoonPositionVelocity ( double jed, double lt, SSVector &pos, SSVector &vel );
    static void computePSPlanetMoonPositionVelocity ( int id, double jed, double lt, SSVector &pos, SSVector &vel );

    float computeAsteroidMagnitude ( double rad, double dist, double phase, double hmag, double gmag );
    float computeCometMagnitude ( double rad, double dist, double hmag, double kmag );

public:
    
    // IAU best estimates for planetary system masses from https://iau-a3.gitlab.io/NSFA/NSFA_cbe.html
    // JPL DE405 solar mass from https://ssd.jpl.nasa.gov/ftp/eph/planets/ioms/de405.iom.pdf
    // Also see https://en.wikipedia.org/wiki/Planetary_mass and https://ssd.jpl.nasa.gov/?constants
    
    static constexpr double kMassSun = 332946.050895;                         // JPL DE405
    static constexpr double kMassMercury = kMassSun / 6.023657330e6;          // IAU 2015
    static constexpr double kMassVenus = kMassSun / 4.08523719e5;             // IAU 2009
    static constexpr double kMassEarth = 1.0;
    static constexpr double kMassEarthSystem = kMassEarth + 1.23000371e-2;    // IAU 2009
    static constexpr double kMassMarsSystem = kMassSun / 3.09870359e6;        // IAU 2009
    static constexpr double kMassJupiterSystem = kMassSun / 1.047348644e3;    // IAU 2009
    static constexpr double kMassSaturnSystem = kMassSun / 3.4979018e3 ;      // IAU 2009
    static constexpr double kMassUranusSystem = kMassSun / 2.2902951e4;       // IAU 2015
    static constexpr double kMassNeptuneSystem = kMassSun / 1.941226e4;       // IAU 2009
    static constexpr double kMassPlutoSystem = kMassSun / 1.3605e8;           // IAU 2015. Value derived from SSMoonEphemeris::charonOrbit() is 0.002445344878
    
    SSPlanet ( SSObjectType type );
    SSPlanet ( SSObjectType type, SSPlanetID id );
    
    void setIdentifier ( SSIdentifier ident ) { _id = ident; }
    void setOrbit ( SSOrbit orbit ) { _orbit = orbit; }
    void setHMagnitude ( float hmag ) { _Hmag = hmag; }
    void setGMagnitude ( float gmag ) { _Gmag = gmag; }
    void setRadius ( float radius ) { _radius = radius; }
    void setMass ( float mass ) { _mass = mass; }

    SSIdentifier getIdentifier ( void ) { return _id; }
    SSIdentifier getIdentifier ( int i ) { return i == 0 ? _id : SSIdentifier(); }
    SSIdentifier getIdentifier ( SSCatalog cat );
    vector<SSIdentifier> getIdentifiers ( void ) { return vector<SSIdentifier> { _id }; }
    SSOrbit getOrbit ( void ) { return _orbit; }
    float getHMagnitude ( void ) { return _Hmag; }
    float getGMagnitude ( void ) { return _Gmag; }
    float getRadius ( void ) { return _radius; }
    float getMass ( void ) { return _mass; }
    double flattening ( void );

    SSVector getPosition ( void ) { return _position; }
    SSVector getVelocity ( void ) { return _velocity; }
    
    double distance ( SSPlanet &other ) { return _position.distance ( other._position ); }
    
    bool surfacePointDirection ( SSAngle lon, SSAngle lat, SSVector &dir, double &dist );

    static double horizonDistance ( double radius, double distance );
    double horizonDistance ( void );

    static double angularRadius ( double radius, double distance );
    double angularRadius ( float s = 1.0f );

    static double phaseAngle ( SSVector position, SSVector direction );
    double phaseAngle ( void );
    
    static double illumination ( double phase );
    double illumination ( void );
    double elongation ( SSPlanet *pSun );
    
    void rotationElements ( double jed, double &a0, double &d0, double &w0, double &wdot );
    SSMatrix getPlanetographicMatrix ( void ) { return _pmatrix; }
    SSMatrix setPlanetographicMatrix ( double jed );
    SSSpherical centralCoordinates ( void );
    SSSpherical subsolarCoordinates ( void );
    
    static double getGRSLongitude ( double jd );
    bool rayIntersect ( SSVector p, SSVector u, double &d, SSVector &q, float s = 1.0f );

    // Sets whether to use (accurate, but slow) VSOP/ELP planetary & lunar ephemeris when JPL DE438 is not available.
    // Also USE_VSOP_ELP must be #defined as 1 at the top of SSPlanet.cpp!
    
    static void useVSOPELP ( bool use );
    static bool useVSOPELP ( void );

    static void computeMajorPlanetPositionVelocity ( int id, double jed, double lt, SSVector &pos, SSVector &vel );
    virtual void computePositionVelocity ( double jed, double lt, SSVector &pos, SSVector &vel );
    virtual void computePositionVelocity  ( SSCoordinates &coords, SSVector &pos, SSVector &vel );
    virtual float computeMagnitude ( double rad, double dist, double phase );
    virtual void computeEphemeris ( SSCoordinates &coords );

    double umbraLength ( float s = 1.0f );
    double umbraRadius ( double d, float s = 1.0f );
    double penumbraRadius ( double d, float s = 1.0f );

    bool isSun ( void ) { return _type == kTypePlanet && _id.identifier() == kSun; }
    bool isLuna ( void ) { return _type == kTypeMoon && _id.identifier() == kLuna; }
    bool isEarth ( void ) { return _type == kTypePlanet && _id.identifier() == kEarth; }

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
