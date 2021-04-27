// SSStar.hpp
// SSCore
//
// Created by Tim DeBenedictis on 3/15/20.
//
// Copyright © 2020 Southern Stars. All rights reserved.
//
// This subclass of SSObject represents all objects outside the solar system,
// including stars, star clusters, nebulae, and galaxies. SSStar has special
// subclasses for double and variable stars, and for deep sky objects.
// Includes utility methods for stellar magnitude computations
// (absolute <-> apparent magnitude, etc.) and Moffat-function stellar image profiles.

#ifndef SSStar_hpp
#define SSStar_hpp

#include "SSObject.hpp"
#include "SSOrbit.hpp"

#pragma pack ( push, 1 )

// This subclass of SSObject stores basic data for stars.
// Its subclasses store double and variable star data,
// and deep sky object data.

class SSStar : public SSObject
{
protected:
    
    vector<SSIdentifier> _idents;
    
    SSVector _position;     // heliocentric position unit vector in fundamental frame at epoch J2000
    SSVector _velocity;     // heliocentric space velocity vector in fundamental frame at epoch J2000 in distance units per Julian year; infinite if unknown
    
    float   _parallax;      // heliocentric parallax in arcseconds (i.e. reciprocal of distance in parsecs), zero if unknown
    float   _radvel;        // radial velocity as fraction of light speed (i.e. light years per year), infinite if unknown
    
    float   _Vmag;          // visual magnitude at J2000
    float   _Bmag;          // blue magnitude at J2000

    string  _spectrum;      // Spectral type string
    
    SSStar ( SSObjectType type ); // constructs a star with a specific type code
    string toCSV1 ( void );       // returns CSV string from base data (excluding names and identifiers).
    string toCSV2 ( void );       // returns CSV string from names and identifiers (excluding base data).

public:
    
    SSStar ( void );
    
    void setIdentifiers ( const vector<SSIdentifier> &idents ) { _idents = idents; }
    void setFundamentalPosition ( SSVector pos ) { _position = pos; }
    void setFundamentalVelocity ( SSVector vel ) { _velocity = vel; }
    void setFundamentalCoords ( SSSpherical coords );
    void setFundamentalMotion ( SSSpherical coords, SSSpherical motion );
    void setVMagnitude ( float vmag ) { _Vmag = vmag; }
    void setBMagnitude ( float bmag ) { _Bmag = bmag; }
    void setSpectralType ( const string &spectrum ) { _spectrum = spectrum; }
    
    bool addIdentifier ( SSIdentifier ident );
    SSIdentifier getIdentifier ( SSCatalog cat );
    SSIdentifier getIdentifier ( int i );
    vector<SSIdentifier> getIdentifiers ( void ) { return _idents; }
    void sortIdentifiers ( void );
    
    SSVector getFundamentalPosition ( void ) { return _position; }
    SSVector getFundamentalVelocity ( void ) { return _velocity; }
    SSSpherical getFundamentalCoords ( void );
    SSSpherical getFundamentalMotion ( void );
    float getVMagnitude ( void ) { return _Vmag; }
    float getBMagnitude ( void ) { return _Bmag; }
    string getSpectralType ( void ) { return _spectrum; }
    float getParallax ( void ) { return _parallax; }
    float getRadVel ( void ) { return _radvel; }
    
    void computeEphemeris ( SSCoordinates &coords );
    void computePositionVelocity ( SSCoordinates &coords, SSVector &pos, SSVector &vel );
    SSSpherical computeApparentMotion ( SSCoordinates &coords, SSFrame frame = kFundamental );

    // imports/exports from/to CSV-format text string
    
    static SSObjectPtr fromCSV ( string csv );
    virtual string toCSV ( void );
    
    // magnitude and color conversion utilities
    
    static void bmv2rgb ( float bmv, float &r, float &g, float &b );
    static float bmv2temp ( float bmv );
    static double absoluteMagnitude ( double appMag, double distPC );
    static double apparentMagnitude ( double absMag, double distPC );
    static double distanceFromMagnitude ( double appMag, double absMag );
    static double brightnessRatio ( double magDiff );
    static double magnitudeDifference ( double ratio );
    static double magnitudeSum ( double mag1, double mag2 );

    // Moffat functions
    
    static double moffatFunction ( double max, double r2, double beta );
    static double moffatRadius ( double z, double max, double beta );
    
    // Spectral types and luminosity classes
    
    enum SpecType
    {
        W0 = 0,     // Wolf-Rayet stars
        O0 = 10,
        B0 = 20,
        A0 = 30,
        F0 = 40,
        G0 = 50,
        K0 = 60,
        M0 = 70,
        L0 = 80,   // Brown dwarfs
        T0 = 90,   // Brown dwarfs
        R0 = 100,  // Carbon stars (obsolete)
        N0 = 110,  // Carbon stars (obsolete)
        S0 = 120,  // Carbon stars
        C0 = 130,  // Carbon stars
    };
    
    enum LumClass
    {
        Ia0 = 1,    // hypergiants
        Ia  = 2,    // luminous supergiants
        Iab = 3,    // intermediate supergiants, c
        Ib  = 4,    // less luminous supergiants
        II  = 5,    // bright giants
        III = 6,    // giants, g
        IV  = 7,    // subgiants, sg
        V   = 8,    // main-sequence, d
        VI  = 9,    // subdwarfs, sd
        VII = 10    // white dwarfs, D
    };
    
    // Information about a particular stellar spectral class. Values are INFINITY if unknown.
    
    struct SpecInfo
    {
        string spec;        // MK spectral type and luminosity class
        float Teff;         // Stellar surface effective temperature, Kelvins
        float BCv;          // Bolometric correction to visual (V) magnitude
        float logL;         // log10 of bolometric luminosity
        float Rsun;         // radius in solar radii
        float Mv;           // absolute visual (V) magnitude
        float BV;           // color index (B-V)
        float Msun;         // mass in solar masses
    };

    static int spectralType ( const string &spectrum );
    static int luminosityClass ( const string &spectrum );
    static bool parseSpectrum ( const string &spectrum, int &spectype, int &lumclass );
    static string formatSpectrum ( int spectype, int lumclass );
    static float bolometricCorrection ( float temp );
    static float colorTemperature ( float bmv, int lumclass );
    static SpecInfo spectralClassInfo ( int spectype, int lumclass );
    static float luminosity ( float mv, float bc );
    static float radius ( float lum, float temp );
};

// This subclass of SSStar stores data for double stars
// in addition to the basic stellar data in the base class.

class SSDoubleStar : virtual public SSStar
{
protected:

    string _comps;              // component string, e.g. "A" for primary, "B" for secondary, "AB" for primary-secondary pair, etc.; empty if unknown
    float _magDelta;            // magnitude difference between components; infinite if unknown
    float _sep;                 // angular separation between components in radians; infinite if unknown
    float _PA;                  // position angle from brighter to fainter component in radians in fundamental mean J2000 equatorial frame; infinite if unknown
    float _PAyr;                // Julian year of position angle measurement; infinite if unknown
    SSOrbit *_pOrbit;           // pointer to binary star orbit data, or nullptr if star has none

    string toCSVD ( void );     // returns CSV string from double-star data (but not SStar base class).

public:
    
    SSDoubleStar ( void );
    ~SSDoubleStar ( void );
    SSDoubleStar ( const SSDoubleStar &other );
    
    void setComponents ( string comps ) { _comps = comps; }
    void setMagnitudeDelta ( float delta ) { _magDelta = delta; }
    void setSeparation ( float sep ) { _sep = sep; }
    void setPositionAngle ( float pa ) { _PA = pa; }
    void setPositionAngleYear ( float year ) { _PAyr = year; }
    void setOrbit ( const SSOrbit &orb ) { delete _pOrbit; _pOrbit = new SSOrbit ( orb ); }
    
    string getComponents ( void ) { return _comps; }
    float getMagnitudeDelta ( void ) { return _magDelta; }
    float getSeparation ( void ) { return _sep; }
    float getPositionAngle ( void ) { return _PA; }
    float getPositionAngleYear ( void ) { return _PAyr; }
    bool hasOrbit ( void ) { return _pOrbit ? true : false; }
    SSOrbit getOrbit ( void ) { return _pOrbit ? *_pOrbit : SSOrbit(); }
    
    virtual string toCSV ( void );
};

// This subclass of SSStar stores data for variable stars
// in addition to the basic stellar data in the base class.

class SSVariableStar : virtual public SSStar
{
protected:
    
    string _varType;             // Variability type code string; empty if unknown
    float _varMaxMag;            // Maximum visual magnitude (i.e. when faintest); infinite if unknown
    float _varMinMag;            // Minimum visual magnitude (i.e. when brightest); infinity if unknown
    double _varPeriod;           // Variability period, in days; infinite if unknown
    double _varEpoch;            // Variability epoch, as Julian Date; infinite if unknown
    
    string toCSVV ( void );      // returns CSV string from variable-star data (but not SStar base class).

public:
    
    SSVariableStar ( void );

    void setVariableType ( string varType ) { _varType = varType; }
    void setMaximumMagnitude ( float maxMag ) { _varMaxMag = maxMag; }
    void setMinimumMagnitude ( float minMag ) { _varMinMag = minMag; }
    void setPeriod ( float period ) { _varPeriod = period; }
    void setEpoch ( double epoch ) { _varEpoch = epoch; }
    
    string getVariableType ( void ) { return _varType; }
    float getMaximumMagnitude ( void ) { return _varMaxMag; }
    float getMinimumMagnitude ( void ) { return _varMinMag; }
    double getPeriod ( void ) { return _varPeriod; }
    double getEpoch ( void ) { return _varEpoch; }
    
    virtual string toCSV ( void );
};

// This subclass of SSStar inherits from both SSDoubleStar and SSVariableStar,
// and stores data for stars which are both double and variable.

class SSDoubleVariableStar : public SSDoubleStar, public SSVariableStar
{
public:
    
    SSDoubleVariableStar ( void );

    virtual string toCSV ( void );
};

// This subclass of SSStar stores data for star clusters, nebulae, and galaxies.

class SSDeepSky : public SSStar
{
protected:
    
    float _majAxis;     // apparent size major axis, in radians; infinite if unknown
    float _minAxis;     // apparent size minor axis, in radians; infinite if unknown
    float _PA;          // position angle of major axis from north in fundamental mean J2000 equatorial frame, in radians; infinite if unknown

    string toCSVDS ( void );   // returns CSV string from deep sky object data (but not SStar base class).

public:
    
    SSDeepSky ( SSObjectType type );
    
    void setMajorAxis ( float maj ) { _majAxis = maj; }
    void setMinorAxis ( float min ) { _minAxis = min; }
    void setPositionAngle ( float pa ) { _PA = pa; }
    void setGalaxyType ( string type ) { _spectrum = type; }
    
    float getMajorAxis ( void ) { return _majAxis; }
    float getMinorAxis ( void ) { return _minAxis; }
    float getPositionAngle ( void ) { return _PA; }
    string getGalaxyType ( void ) { return _spectrum; }

    virtual string toCSV ( void );
};

#pragma pack ( pop )

// convenient aliases for pointers to various subclasses of SSStar

typedef SSStar *SSStarPtr;
typedef SSDoubleStar *SSDoubleStarPtr;
typedef SSVariableStar *SSVariableStarPtr;
typedef SSDeepSky *SSDeepSkyPtr;

// These functions downcast a pointer from the SSObject base class to its various SSStar subclasses.
// They all return null pointers if the input object pointer is not an instance of the expected derived class.

SSStarPtr SSGetStarPtr ( SSObjectPtr ptr );
SSDoubleStarPtr SSGetDoubleStarPtr ( SSObjectPtr ptr );
SSVariableStarPtr SSGetVariableStarPtr ( SSObjectPtr ptr );
SSDeepSkyPtr SSGetDeepSkyPtr ( SSObjectPtr ptr );

#endif /* SSStar_hpp */
