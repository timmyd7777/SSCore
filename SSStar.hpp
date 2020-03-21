//  SSStar.hpp
//  SSCore
//
//  Created by Tim DeBenedictis on 3/15/20.
//  Copyright © 2020 Southern Stars. All rights reserved.
//  Represents a star or deep sky object

#ifndef SSStar_hpp
#define SSStar_hpp

#include "SSObject.hpp"

class SSStar : public SSObject
{
protected:
    
	vector<SSIdentifier> _ids;
	
    SSVector _position;     // heliocentric position in fundamental frame at epoch J2000; parsecs if _parallax > 0 or unit vector if _parallax is zero
    SSVector _velocity;     // heliocentric space motion in fundamental frame at epoch J2000, parsecs per Julian year if _parallax > 0 or unit vector if _parallax is zero
    
    float   _parallax;      // heliocentric parallax in arcseconds (i.e. reciprocal of distance in parsecs), zero if unknown
    float   _radvel;        // radial velocity as fraction of light speed (i.e. light years per year), infinite if unknown
    
    float   _Vmag;          // visual magnitude at J2000
    float   _Bmag;          // blue magnitude at J2000

    string  _spectrum;      // Spectral type string
    
public:
    
	SSStar ( void );
    SSStar ( SSObjectType type );
    SSStar ( SSObjectType type, vector<string> names, vector<SSIdentifier> ids, SSSpherical position, SSSpherical motion, float vmag, float bmag, string spectrum );

	SSIdentifier getIdentifier ( SSCatalog cat );
	vector<SSIdentifier> getIdentifiers ( void );
	
	SSSpherical getFundamentalPosition ( void );
	SSSpherical getFundamentalProperMotion ( void );
	
	float getParallax ( void ) { return _parallax; }
	float getRadVel ( void ) { return _radvel; }
	
    void computeEphemeris ( SSDynamics dyn );
	
	string toCSV ( void );
};

#endif /* SSStar_hpp */
