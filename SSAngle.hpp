//  SSAngle.hpp
//  SSCore
//
//  Created by Tim DeBenedictis on 2/24/20.
//  Copyright © 2020 Southern Stars. All rights reserved.

#ifndef SSAngle_hpp
#define SSAngle_hpp


class SSAngle
{
public:

    double a;       // always radians
    
    static constexpr double kPi  = 3.141592653589793;
    static constexpr double kTwoPi = 6.283185307179586;
    static constexpr double kHalfPi = 1.570796326794897;
    
    static constexpr double kDegPerRad = 57.295779513082321;
    static constexpr double kRadPerDeg = 0.017453292519943;
    
    static constexpr double kHourPerRad = 3.819718634205488;
    static constexpr double kRadPerHour = 0.261799387799149;

    struct DMS
    {
        char sign;
        short deg;
        short min;
        double sec;
    };

    struct HMS
    {
        char sign;
        short hour;
        short min;
        double sec;
    };

    SSAngle ( void );
    SSAngle ( double rad );
    
    double toDegrees ( void ) { return a * kDegPerRad; }
    void toDegMin ( char &sign, short &deg, double &min );
    void toDegMinSec ( char &sign, short &deg, short &min, double &sec );
    
    double toHours ( void ) { return a * kHourPerRad; }
    void toHourMin ( char &sign, short &deg, double &min );
    void toHourMinSec ( char &sign, short &deg, short &min, double &sec );

    HMS toHMS ( void );
    DMS toDMS ( void );
    
    static SSAngle fromDegrees ( double degrees );
    static SSAngle fromDegMin ( char sign, short deg, double min );
    static SSAngle fromDegMinSec ( char sign, short deg, short min, double sec );
    
    static SSAngle fromHours ( double hours );
    static SSAngle fromHourMin ( char sign, short hour, double min );
    static SSAngle fromHourMinSec ( char sign, short hour, short min, double sec );

    SSAngle modPi ( void );
    SSAngle mod2Pi ( void );
    
    SSAngle angularSeparation ( SSAngle lon0, SSAngle lat0, SSAngle lon1, SSAngle lat1 );
    SSAngle positionAngle ( SSAngle lon0, SSAngle lat0, SSAngle lon1, SSAngle lat1 );
};

#endif /* SSAngle_hpp */
