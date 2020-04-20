// SSAngle.hpp
// SSCore
//
// Created by Tim DeBenedictis on 2/24/20.
// Copyright © 2020 Southern Stars. All rights reserved.
//
// Classes for converting angular values from radians
// to degrees/hours, minutes, seconds; and vice-versa.

#ifndef SSAngle_hpp
#define SSAngle_hpp

#include "SSUtilities.hpp"

// Represents an angular value expressed in positive or negative degrees, minutes, seconds

struct SSDegMinSec
{
    char sign;        // sign of angle, either '+' or '-'
    short deg;        // degrees portion of angle, 0 - 360, always positive
    short min;        // minutes portion of angle, 0 - 59, always positive
    double sec;       // seconds portion of angle, 0 - 59.999..., always positive
    
    SSDegMinSec ( string s );
    SSDegMinSec ( double degrees );
    SSDegMinSec ( char sign, short deg, short min, double sec );
    SSDegMinSec ( class SSAngle );
    
    double toDegrees ( void );
    string toString ( void );
    string format ( const string &fmt );
};

// Represents an angular value expressed in hours, minutes, seconds
// Always positive, in range 0 to 24 hours.

struct SSHourMinSec
{
    short hour;       // hours portion of angle, 0 - 23, always positive
    short min;        // minutes portion of angle, 0 - 59, always positive
    double sec;       // seconds portion of angle, 0 - 59.999..., always positive

    SSHourMinSec ( string s );
    SSHourMinSec ( double hours );
    SSHourMinSec ( short hour, short min, double sec );
    SSHourMinSec ( class SSAngle );

    double toHours ( void );
    string toString ( void );
    string format ( const string &fmt );
};

// Represents an angular value expressed in radians

class SSAngle
{
protected:
    
    double _rad;      // angular value in radians; infinite if unknown/undefined.
    
public:

    static constexpr double kPi  = M_PI;                            // 3.141592653589793
    static constexpr double kTwoPi = M_2PI;                         // 6.283185307179586
    static constexpr double kHalfPi = M_PI_2;                       // 1.570796326794897
    
    static constexpr double kDegPerRad = 180.0 / kPi;               // 57.295779513082321
    static constexpr double kRadPerDeg = 1.0 / kDegPerRad;          //  0.017453292519943
    
    static constexpr double kHourPerRad = kDegPerRad / 15.0;        // 3.819718634205488
    static constexpr double kRadPerHour = 1.0 / kHourPerRad;        // 0.261799387799149

    static constexpr double kArcminPerRad = 60.0 * kDegPerRad;      // 3437.746770784939253
    static constexpr double kRadPerArcmin = 1.0 / kArcminPerRad;    //    0.000290888208666

    static constexpr double kArcsecPerRad = 3600 * kDegPerRad;      // 206264.806247096355156
    static constexpr double kRadPerArcsec = 1.0 / kArcsecPerRad;    //      0.000004848136811
    
    SSAngle ( void );
    SSAngle ( double rad );
    SSAngle ( SSDegMinSec dms );
    SSAngle ( SSHourMinSec hms );
    
    double toArcsec ( void ) { return _rad * kArcsecPerRad; }
    double toArcmin ( void ) { return _rad * kArcminPerRad; }
    double toDegrees ( void ) { return _rad * kDegPerRad; }
    double toHours ( void ) { return _rad * kHourPerRad; }

    static SSAngle fromArcsec ( double arcsec );
    static SSAngle fromArcmin ( double arcmin );
    static SSAngle fromDegrees ( double degrees );
    static SSAngle fromHours ( double hours );
 
    operator double() const { return _rad; }

    SSAngle operator + ( double k ) { return SSAngle ( _rad + k ); }
    SSAngle operator - ( double k ) { return SSAngle ( _rad - k ); }
    SSAngle operator * ( double k ) { return SSAngle ( _rad * k ); }
    SSAngle operator / ( double k ) { return SSAngle ( _rad / k ); }

    void operator += ( double k ) { _rad += k; }
    void operator -= ( double k ) { _rad -= k; }
    void operator *= ( double k ) { _rad *= k; }
    void operator /= ( double k ) { _rad /= k; }
    
    SSAngle modPi ( void );
    SSAngle mod2Pi ( void );
    
    static SSAngle atan2Pi ( double y, double x );
};

#endif /* SSAngle_hpp */
