//  SSStar.hpp
//  SSCore
//
//  Created by Tim DeBenedictis on 3/15/20.
//  Copyright © 2020 Southern Stars. All rights reserved.
//  Represents a star or deep sky object

#ifndef SSStar_hpp
#define SSStar_hpp

#include "SSObject.hpp"

#pragma pack ( push, 1 )

// This subclass of SSObject stores basic data for stars.
// Its subclasses store double and variable star data,
// and deep sky object data.

class SSStar : public SSObject
{
protected:
    
    vector<SSIdentifier> _idents;
    
    SSVector _position;     // heliocentric position in fundamental frame at epoch J2000; in light years if _parallax > 0 or as unit vector if _parallax is zero
    SSVector _velocity;     // heliocentric space velocity in fundamental frame at epoch J2000, in light years per year (fraction of light speed) if _parallax > 0 or as unit vector if _parallax is zero
    
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

    void setIdentifiers ( vector<SSIdentifier> idents ) { _idents = idents; }
    void setFundamentalPosition ( SSVector pos ) { _position = pos; }
    void setFundamentalVelocity ( SSVector vel ) { _velocity = vel; }
    void setFundamentalCoords ( SSSpherical coords );
    void setFundamentalMotion ( SSSpherical coords, SSSpherical motion );
    void setVMagnitude ( float vmag ) { _Vmag = vmag; }
    void setBMagnitude ( float bmag ) { _Bmag = bmag; }
    void setSpectralType ( string spectrum ) { _spectrum = spectrum; }
    
    bool addIdentifier ( SSIdentifier ident );
    SSIdentifier getIdentifier ( SSCatalog cat );
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
    
    void computeEphemeris ( SSDynamics &dyn );
    
    // imports/exports from/to CSV-format text string
    
    static SSObjectPtr fromCSV ( string csv );
    virtual string toCSV ( void );
};

// This subclass of SSStar stores data for double stars
// in addition to the basic stellar data in the base class.

class SSDoubleStar : virtual public SSStar
{
protected:

    string _comps;              // component string, e.g. "A" for primary, "B" for secondary, "AB" for primary-secondary pair, etc.; empty if unknown
    float _magDelta;            // magnitude difference between components; infinite if unknown
    float _sep;                 // angular separation between components in radians; infinite if unknown
    float _PA;                  // position angle from brighter to fainter component in fundamental mean J2000 equatorial frame; infinite if unknown
    float _PAyr;                // Julian year of position angle measurement; infinite if unknown
    
    string toCSVD ( void );     // returns CSV string from double-star data (but not SStar base class).

public:
    
    SSDoubleStar ( void );
    
    void setComponents ( string comps ) { _comps = comps; }
    void setMagnitudeDelta ( float delta ) { _magDelta = delta; }
    void setSeparation ( float sep ) { _sep = sep; }
    void setPositionAngle ( float pa ) { _PA = pa; }
    void setPositionAngleYear ( float year ) { _PAyr = year; }
    
    string getComponents ( void ) { return _comps; }
    float getMagnitudeDelta ( void ) { return _magDelta; }
    float getSeparation ( void ) { return _sep; }
    float getPositionAngle ( void ) { return _PA; }
    float getPositionAngleYear ( void ) { return _PAyr; }

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
    float _PA;          // position angle of major axis from north in fundamental mean J2000 equatorial frame; infinite if unknown

    string toCSVDS ( void );   // returns CSV string from deep sky object data (but not SStar base class).

public:
    
    SSDeepSky ( SSObjectType type );
    
    void setMajorAxis ( float maj ) { _majAxis = maj; }
    void setMinorAxis ( float min ) { _minAxis = min; }
    void setPositionAngle ( float pa ) { _PA = pa; }
    void setGalaxyType ( string type ) { _spectrum = type; }
    
    float getMajorAxis ( void ) { return _majAxis; }
    float getMinorAxis ( void ) { return _minAxis; }
    float getPostionAngle ( void ) { return _PA; }
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
