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
    kAmalthea = 505,
    kHimalia = 506,
    kElara = 507,
    kPasiphae = 508,
    kSinope = 509,
    kLysithea = 510,
    kCarme = 511,
    kAnanke = 512,
    kLeda = 513,
    kThebe = 514,
    kAdrastea = 515,
    kMetis = 516,
    
    kMimas = 601,
    kEnceladus = 602,
    kTethys = 603,
    kDione = 604,
    kRhea = 605,
    kTitan = 606,
    kHyperion = 607,
    kIapetus = 608,
    kPhoebe = 609,
    kJanus = 610,
    kEpimetheus = 611,
    kHelene = 612,
    kTelesto = 613,
    kCalypso = 614,
    kAtlas = 615,
    kPrometheus = 616,
    kPandora = 617,
    kPan = 618,
    kMethone = 632,
    kPallene = 633,
    kPolydeuces = 634,
    kDaphnis = 635,
    kAnthe = 649,
    kAegaeon = 653,
    
    kAriel = 701,
    kUmbriel = 702,
    kTitania = 703,
    kOberon = 704,
    kMiranda = 705,
    kCordelia = 706,
    kOphelia = 707,
    kBianca = 708,
    kCressida = 709,
    kDesdemona = 710,
    kJuliet = 711,
    kPortia = 712,
    kRosalind = 713,
    kBelinda = 714,
    kPuck = 715,
    kPerdita = 725,
    kMab = 726,
    kCupid = 727,
    
    kTriton = 801,
    kNereid = 802,
    kNaiad = 803,
    kThalassa = 804,
    kDespina = 805,
    kGalatea = 806,
    kLarissa = 807,
    kProteus = 808,
    kHippocamp = 814,
    
    kCharon = 901,
    kNix = 902,
    kHydra = 903,
    kKerberos = 904,
    kStyx = 905
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
    float       _BminV;         // B-V color index; infinite if unknown
    float       _radius;        // equatorial radius in kilometers; zero if unknown
    float       _mass;          // mass in Earth masses, zero if unknown
    float       _rotper;        // sidereal rotation period in days, zero if unknown
    float       _albedo;        // geometric albedo; zero if unknown
    string      _taxonomy;      // spectral taxonomic type (Tholen) for asteroids; empty if unknown
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
    void setColorIndex ( float bmv ) { _BminV = bmv; }
    void setRadius ( float radius ) { _radius = radius; }
    void setMass ( float mass ) { _mass = mass; }
    void setRotationPeriod ( float period ) { _rotper = period; }
    void setAlbedo ( float a ) { _albedo = a; }
    void setTaxonomy ( string tax ) { _taxonomy = tax; }
    
    string getTypeName ( void );
    string getNumberName ( void );
    SSIdentifier getIdentifier ( void ) { return _id; }
    SSIdentifier getIdentifier ( int i ) { return i == 0 ? _id : SSIdentifier(); }
    SSIdentifier getIdentifier ( SSCatalog cat );
    vector<SSIdentifier> getIdentifiers ( void ) { return vector<SSIdentifier> { _id }; }
    SSOrbit getOrbit ( void ) { return _orbit; }
    float getHMagnitude ( void ) { return _Hmag; }
    float getGMagnitude ( void ) { return _Gmag; }
    float getColorIndex ( void ) { return _BminV; }
    float getRadius ( void ) { return _radius; }
    float getMass ( void ) { return _mass; }
    float getRotationPeriod ( void ) { return _rotper; }
    float getAlbedo ( void ) { return _albedo; }
    string getTaxonomy ( void ) { return _taxonomy; }
    double getFlattening ( void );

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

    // Sets whether to use (accurate, but slow) VSOP/ELP planetary & lunar ephemeris when JPL DE442 is not available.
    // Also USE_VSOP_ELP must be #defined as 1 at the top of SSPlanet.cpp!
    
    static void useVSOPELP ( bool use );
    static bool useVSOPELP ( void );

    static void computeMajorPlanetPositionVelocity ( int id, double jed, double lt, SSVector &pos, SSVector &vel );
    virtual void computePositionVelocity ( double jed, double lt, SSVector &pos, SSVector &vel );
    virtual void computePositionVelocity  ( SSCoordinates &coords, SSVector &pos, SSVector &vel );
    virtual float computeMagnitude ( double rad, double dist, double phase );
    virtual void computeEphemeris ( SSCoordinates &coords );
    SSSpherical computeApparentMotion ( SSCoordinates &coords, SSFrame frame = kFundamental );

    double umbraLength ( float s = 1.0f );
    double umbraRadius ( double d, float s = 1.0f );
    double penumbraRadius ( double d, float s = 1.0f );

    // Convenience methods for testing whether a planet is the Sun/Moon/Earth or a small moon.

    bool isSun ( void ) { return _type == kTypePlanet && _id.identifier() == kSun; }
    bool isLuna ( void ) { return _type == kTypeMoon && _id.identifier() == kLuna; }
    bool isEarth ( void ) { return _type == kTypePlanet && _id.identifier() == kEarth; }
    bool isSmallMoon ( void );
    
    // imports/exports from/to CSV-format text string
    
    static SSObjectPtr fromCSV ( string csv );
    string toCSV ( void );
    static string headerCSV ( void );
};

// Subclass of solar system object for artificial Earth satellites.
// Includes methods for computing position and velocity from TLE elements via SGP4/SDP4 orbit model
// and computing visual magnitude with McCants formula.

class SSSatellite : public SSPlanet
{
public:
    
    // N2YO satellite category identifiers
    // See https://www.n2yo.com/api/

    enum Category
    {
        kNone                  =  0,    // Missing/unknown category assignment
        kBrightest             =  1,    // Brightest
        kISS                   =  2,    // International Space Station
        kWeather               =  3,    // Weather
        kNOAA                  =  4,    // NOAA
        kGOES                  =  5,    // GOES
        kEarthResources        =  6,    // Earth Resources
        kSearchAndRescue       =  7,    // Search & Rescue
        kDisasterMonitoring    =  8,    // Disaster Monitoring
        kTDRS                  =  9,    // Tracking and Data Relay Satellite System (TDRSS)
        kGeostationary         = 10,    // Geostationary
        kIntelsat              = 11,    // Intelsat
        kGorizont              = 12,    // Gorizont
        kRaduga                = 13,    // Raduga
        kMolniya               = 14,    // Molniya
        kIridium               = 15,    // Iridium
        kOrbcomm               = 16,    // Orbcomm
        kGlobalstar            = 17,    // Globalstar
        kAmateur               = 18,    // Amateur Radio
        kExperimental          = 19,    // Experimental Communication
        kGPSOps                = 20,    // GPS Operational
        kGLONASSOps            = 21,    // GLONASS Operational
        kGalileo               = 22,    // Galileo
        kSBAS                  = 23,    // Satellite-Based Augmentation System (SBAS)
        kNNSS                  = 24,    // Navy Navigation Satellite System (NNSS)
        kMusson                = 25,    // Russian LEO Navigation
        kScience               = 26,    // Space & Earth Science
        kGeodetic              = 27,    // Geodetic
        kEngineering           = 28,    // Engineering
        kEducation             = 29,    // Education
        kMilitary              = 30,    // Military
        kRadar                 = 31,    // Radar Calibration
        kCubeSat               = 32,    // CubeSats
        kSiriusXM              = 33,    // XM & Sirius
        kTV                    = 34,    // Television
        kBeiDou                = 35,    // BeiDou Navigation
        kYaogan                = 36,    // Yaogan
        kWestford              = 37,    // Westford Needles
        kParus                 = 38,    // Parus
        kStrela                = 39,    // Strela
        kGonets                = 40,    // Gonets
        kTsiklon               = 41,    // Tsiklon
        kTsikada               = 42,    // Tsikada
        kO3b                   = 43,    // O3b
        kTselina               = 44,    // Tselina
        kCelestis              = 45,    // Celestis
        kIRNSS                 = 46,    // Indian Regional Navigation Satellite System (IRNSS / NavIC)
        kQZSS                  = 47,    // Quasi-Zenith Satellite System (QZSS)
        kFlock                 = 48,    // Planet Flock
        kLemur                 = 49,    // Spire Lemur
        kGPS                   = 50,    // GPS Constellation
        kGLONASS               = 51,    // GLONASS Constellation
        kStarlink              = 52,    // Starlink Constellation
        kOneWeb                = 53,    // OneWeb Constellation
        kChineseStation        = 54,    // Chinese Space Station
        kQianfan               = 55,    // Qianfan (Thousand Sails)
        kKuiper                = 56,    // Kuiper (AKA Leo) Constallation, not to be confused with LEO
        kGeeSAT                = 57,    // GeeSAT
        kJunk                  = 99,    // Space junk / debris - always last category!
        kSES                   = 100,   // SES
        kARGOS                 = 101,   // ARGOS Data Collection System
        kEutelsat              = 102,   // Eutelsat
        kHulianwang            = 103,   // Hulianwang (Guowang) National Internet Satellites
        kSatNOGS               = 104,   // SatNOGS
        kGEOProtectedZone      = 105,   // GEO Protected Zone
        kSAR                   = 106,   // Synthetic Aperture Radar (SAR)
        kTelesat               = 107,   // Telesat
        kAnalyst               = 108,   // Analyst (Unknown) Satellites
    };
    
    // Sources of standard magnitude and physical diameter data
    
    enum DataSrc
    {
        kMcCants = 'M',                 // Mike McCants "MacNames.txt" file, last updated 24 July 2019
        kConstellation = 'C',           // Default value for constellation
        kSCORE = 'S',                   // Derived from IAU SCORE observations
        kRCS = 'R'                      // Derived from radar cross section published by space-track.org
    };
        
    struct FreqData
    {
        int    norad = 0;             // NORAD tracking number
        string name;                  // satellite name
        string uplink;                // uplink frequency(ies), MHz
        string downlink;              // downlink frequency(ies), MHz
        string beacon;                // beacon frequency, MHz
        string mode;                  // modulation scheme and rate, if known
        string callsign;              // callsign
        string status;                // current status
    };

protected:
    
    SSTLE   _tle;                     // current orbit from Two-Line-Element data
    string  _sourceCountry;           // source country code
    string  _launchSite;              // launch side code
    string  _description;             // informative description
    float   _launchDate = INFINITY;   // launch date [JD]; infinity if unknown
    float   _decayDate = INFINITY;    // re-entry date (JD); infinity if unknonwn
    vector<Category> _categories;     // Categories to which this satellite belongs
    vector<FreqData> _frequencies;    // frequency data for radio transmitter(s)
    char   _magSrc = 0, _diamSrc = 0; // source of standard magnitude and diameter data, 0 = unknown
    
public:
    
    SSSatellite ( SSTLE &tle );
    
    void setTLE ( const SSTLE &tle );
    SSTLE getTLE ( void ) { return _tle; }
    
    virtual void  computePositionVelocity ( double jed, double lt, SSVector &pos, SSVector &vel );
    virtual float computeMagnitude ( double rad, double dist, double phase );
    static  float computeSatelliteMagnitude ( double dist, double phase, double stdmag );
    static  float computeStandardMagnitude ( double dist, double phase, double appmag );

    string getSourceCountry ( void ) { return _sourceCountry; }
    string getLaunchSite ( void ) { return _launchSite; }
    double getLaunchDate ( void ) { return _launchDate; }
    double getDecayDate ( void ) { return _decayDate; }
    string getDescription ( void ) { return _description; }
    vector<Category> getCategories ( void ) { return _categories; }
    vector<FreqData> getRadioFrequencies ( void ) { return _frequencies; }

    char getMagnitudeSource ( void ) { return _magSrc; }
    char getDiameterSource ( void ) { return _diamSrc; }
    void setMagnitudeSource ( char src ) { _magSrc = src; }
    void setDiameterSource ( char src ) { _diamSrc = src; }

    void addCategory ( Category c );
    bool hasCategory ( Category c );

    void setSourceCountry ( string source ) { _sourceCountry = source; }
    void setLaunchSite ( string site ) { _launchSite = site; }
    void setLaunchDate ( double jd ) { _launchDate = jd; }
    void setDecayDate ( double jd ) { _decayDate = jd; }
    void setDescription ( const string &description ) { _description = description; }
    void setCategories ( const vector<Category> &categories ) { _categories = categories; }
    void setRadioFrequencies ( const vector<FreqData> &freqs ) { _frequencies = freqs; }

    // imports/exports from/to CSV-format text string
    
    static SSObjectPtr fromCSV ( string csv );
    string toCSV ( void );
    static string headerCSV ( void );
};

// convenient aliases for pointers to various subclasses of SSPlanet

typedef SSPlanet *SSPlanetPtr;
typedef SSSatellite *SSSatellitePtr;

// Downcasts a pointer from SSObject base class to SSPlanet or SSSatellite subclass.
// Returns nullptr if input object pointer is not actually an SSPlanet or SSSatellite.

SSPlanetPtr SSGetPlanetPtr ( SSObjectPtr ptr );
SSSatellitePtr SSGetSatellitePtr ( SSObjectPtr ptr );

#endif /* SSPlanet_hpp */
