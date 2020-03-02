//  SSAngle.cpp
//  SSCore
//
//  Created by Tim DeBenedictis on 2/24/20.
//  Copyright © 2020 Southern Stars. All rights reserved.

#include <math.h>
#include "SSAngle.hpp"

SSAngle::SSAngle ( void )
{
    a = 0.0;
}

SSAngle::SSAngle ( double a )
{
    this->a = a;
}

void SSAngle::toDegMin ( char &sign, short &deg, double &min )
{
    double degrees = fabs ( a ) * kDegPerRad;
    
    sign = a >= 0.0 ? '+' : '-';
    deg = floor ( degrees );
    min = 60.0 * ( degrees - deg );
}

void SSAngle::toHourMin ( char &sign, short &hour, double &min )
{
    double hours = fabs ( a ) * kHourPerRad;
    
    sign = a >= 0.0 ? '+' : '-';
    hour = floor ( hours );
    min = 60.0 * ( hours - hour );
}

void SSAngle::toHourMinSec ( char &sign, short &hour, short &min, double &sec )
{
    double hours = fabs ( a ) * kHourPerRad;
    
    sign = a >= 0.0 ? '+' : '-';
    hour = floor ( hours );
    min = 60.0 * ( hours - hour );
    sec = 3600.0 * ( hours - hour - min / 60.0 );
}

SSAngle::HMS SSAngle::toHMS ( void )
{
    SSAngle::HMS hms = { 0 };
    
    toHourMinSec ( hms.sign, hms.hour, hms.min, hms.sec );
    
    return ( hms );
}

void SSAngle::toDegMinSec ( char &sign, short &deg, short &min, double &sec )
{
    double degrees = fabs ( a ) * kDegPerRad;
    
    sign = a >= 0.0 ? '+' : '-';
    deg = floor ( degrees );
    min = 60.0 * ( degrees - deg );
    sec = 3600.0 * ( degrees - deg - min / 60.0 );
}

SSAngle::DMS SSAngle::toDMS ( void )
{
    SSAngle::DMS dms = { 0 };
    
    toDegMinSec ( dms.sign, dms.deg, dms.min, dms.sec );
    
    return ( dms );
}

SSAngle SSAngle::fromArcsec ( double arcsec )
{
    return SSAngle ( arcsec * kRadPerArcsec );
}

SSAngle SSAngle::fromArcmin ( double arcmin )
{
    return SSAngle ( arcmin * kRadPerArcmin );
}

SSAngle SSAngle::fromDegrees ( double degrees )
{
    return SSAngle ( degrees * kRadPerDeg );
}

SSAngle SSAngle::fromDegMin ( char sign, short deg, double min )
{
    double rad = ( deg + min / 60.0 ) * kRadPerDeg;
    return SSAngle ( sign == '+' ? rad : -rad );
}

SSAngle SSAngle::fromDegMinSec ( char sign, short deg, short min, double sec )
{
    double rad = ( deg + min / 60.0 + sec / 3600.0 ) * kRadPerDeg;
    return SSAngle ( sign == '+' ? rad : -rad );
}

SSAngle SSAngle::fromHours ( double hours )
{
    return SSAngle ( hours * kRadPerHour );
}

SSAngle SSAngle::fromHourMin ( char sign, short hour, double min )
{
    double rad = ( hour + min / 60.0 ) * kRadPerHour;
    return SSAngle ( sign == '+' ? rad : -rad );
}

SSAngle SSAngle::fromHourMinSec ( char sign, short hour, short min, double sec )
{
    double rad = ( hour + min / 60.0 + sec / 3600.0 ) * kRadPerHour;
    return SSAngle ( sign == '+' ? rad : -rad );
}

SSAngle SSAngle::mod2Pi ( void )
{
    return SSAngle ( a - kTwoPi * floor ( a / kTwoPi ) );
}

SSAngle SSAngle::modPi ( void )
{
    double x = mod2Pi().a;
    
    if ( x > kPi )
        x -= kTwoPi;
    
    return ( SSAngle ( x ) );
}
