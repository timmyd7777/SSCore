#include "SSAngle.hpp"

class SSVector
{
public:
  
    double x, y, z;

    SSVector ( void );
    SSVector ( double x, double y, double z );
    
    double magnitude ( void );
    double normalize ( void );
    
    SSVector add ( SSVector other );
    SSVector subtract ( SSVector other );
    SSVector multiplyBy ( double s );
    SSVector divideBy ( double s );
    
    double dotProduct ( SSVector other );
    SSVector crossProduct ( SSVector other );
       
    void toSpherical ( SSAngle &lon, SSAngle &lat, double &rad );
    void fromSpherical ( SSAngle lon, SSAngle lat, double rad );

    SSAngle angularSeparation ( SSVector other );
    SSAngle positionAngle ( SSVector other );
    
    double distance ( SSVector other );
};
