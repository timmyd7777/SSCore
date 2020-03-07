//  SSAngle.cpp
//  SSCore
//
//  Created by Tim DeBenedictis on 2/24/20.
//  Copyright © 2020 Southern Stars. All rights reserved.

#include <math.h>
#include "SSAngle.hpp"

SSDegMinSec::SSDegMinSec ( char sign, short deg, short min, double sec )
{
	this->sign = sign;
	this->deg = deg;
	this->min = min;
	this->sec = sec;
}

SSDegMinSec::SSDegMinSec ( SSAngle ang )
{
    double degrees = fabs ( ang.toDegrees() );

	sign = ang.rad >= 0.0 ? '+' : '-';
	deg = degrees;
	min = 60.0 * ( degrees - deg );
	sec = 3600.0 * ( degrees - deg - min / 60.0 );
}

SSHourMinSec::SSHourMinSec ( char sign, short hour, short min, double sec )
{
	this->sign = sign;
	this->hour = hour;
	this->min = min;
	this->sec = sec;
}

SSHourMinSec::SSHourMinSec ( SSAngle ang )
{
	double hours = fabs ( ang.toHours() );

	sign = ang.rad >= 0.0 ? '+' : '-';
    hour = hours;
    min = 60.0 * ( hours - hour );
    sec = 3600.0 * ( hours - hour - min / 60.0 );
}

SSAngle::SSAngle ( void )
{
    rad = 0.0;
}

SSAngle::SSAngle ( double rad )
{
    this->rad = rad;
}

SSAngle::SSAngle ( SSDegMinSec dms )
{
	rad = kRadPerDeg * ( dms.deg + dms.min / 60.0 + dms.sec / 3600.0 ) * ( dms.sign == '+' ? 1 : -1 );
}

SSAngle::SSAngle ( SSHourMinSec hms )
{
	rad = kRadPerHour * ( hms.hour + hms.min / 60.0 + hms.sec / 3600.0 ) * ( hms.sign == '+' ? 1 : -1 );
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

SSAngle SSAngle::fromHours ( double hours )
{
    return SSAngle ( hours * kRadPerHour );
}

SSAngle SSAngle::mod2Pi ( void )
{
    return SSAngle ( rad - kTwoPi * floor ( rad / kTwoPi ) );
}

SSAngle SSAngle::modPi ( void )
{
    double x = mod2Pi().rad;
    
    if ( x > kPi )
        x -= kTwoPi;
    
    return ( SSAngle ( x ) );
}
