//  SSAngle.cpp
//  SSCore
//
//  Created by Tim DeBenedictis on 2/24/20.
//  Copyright © 2020 Southern Stars. All rights reserved.

#include <math.h>
#include "SSAngle.hpp"

// Constructs an angular value in degrees (d), minutes (m), seconds (s)
// with the + or - sign as a single character (c).

SSDegMinSec::SSDegMinSec ( char c, short d, short m, double s )
{
    sign = c;
    deg = d;
    min = m;
    sec = s;
}

// Constructs an angular value in degrees, minutes, seconds
// from a angle in decimal degrees (degrees).

SSDegMinSec::SSDegMinSec ( double degrees )
{
    sign = degrees >= 0.0 ? '+' : '-';
    degrees = fabs ( degrees );
    deg = (int) degrees;
    min = (int) ( 60.0 * ( degrees - deg ) );
    sec = 3600.0 * ( degrees - deg - min / 60.0 );
}

// Constructs an angular value in degrees, minutes, seconds
// from an angle in radians.

SSDegMinSec::SSDegMinSec ( SSAngle angle ) : SSDegMinSec ( angle.toDegrees() )
{
    // wow, wasn't that easy!
}

// Constructs an angular value from a sexagesimal string in any format:
// "DD MM SS.S", "DD MM.M", "DD.D". Assumes leading & trailing whitespace removed!

SSDegMinSec::SSDegMinSec ( string str ) : SSDegMinSec ( strtodeg ( str ) )
{
    // you're putting me out of work!
}

// Converts an angle in degrees, minutes, seconds to decimal degrees.

double SSDegMinSec::toDegrees ( void )
{
    double d = deg + min / 60.0 + sec / 3600.0;
    return sign == '-' ? -d : d;
}

// Converts an angle in degrees, minutes, seconds to a string.
// Prints seconds to 1 decimal place, and rounds up to avoid generating
// strings like "+89 59 60.0" (this would become "+90 00 00.0")

string SSDegMinSec::toString ( void )
{
    if ( sec >= 59.95 )
        return SSDegMinSec ( toDegrees() + ( sign == '-' ? -0.05 : 0.05 ) / 3600.0 ).toString();
    else
        return format ( "%c%02hd %02hd %04.1f", sign, deg, min, sec );
}

// Constructs an angular value in hours, minutes, seconds.

SSHourMinSec::SSHourMinSec ( short h, short m, double s )
{
    hour = h;
    min = m;
    sec = s;
}

// Constructs an angular value in hours, minutes, seconds
// from an angle in decimal hours.

SSHourMinSec::SSHourMinSec ( double hours )
{
    hours = mod24h ( hours );
    hour = (int) hours;
    min = (int) ( 60.0 * ( hours - hour ) );
    sec = 3600.0 * ( hours - hour - min / 60.0 );
}

// Constructs an angular value in hours, minutes, seconds from an angle in radians.

SSHourMinSec::SSHourMinSec ( SSAngle angle ) : SSHourMinSec ( angle.toHours() )
{
    // bam!
}

// Constructs an angular value from a sexagesimal string in any format:
// "HH MM SS.S", "HH MM.M", "HH.H". Assumes leading & trailing whitespace removed!

SSHourMinSec::SSHourMinSec ( string str ) : SSHourMinSec ( strtodeg ( str ) )
{
    // it's too much!
}

// Converts an angle in hours, minutes, seconds to decimal hours.

double SSHourMinSec::toHours ( void )
{
    return hour + min / 60.0 + sec / 3600.0;
}

// Converts an angle in hours, minutes, seconds to a string.
// Prints seconds to 2 decimal places, and rounds up to avoid generating
// strings like "23 59 60.00" (this would become "00 00 00.0")
// OMITS SIGN!

string SSHourMinSec::toString ( void )
{
    if ( sec >= 59.995 )
        return SSHourMinSec ( mod24h ( toHours() + 0.005 / 3600.0 ) ).toString();
    else
        return format ( "%02hd %02hd %05.2f", hour, min, sec );
}

// Constructs an angle in radians with the defautl value of zero.

SSAngle::SSAngle ( void )
{
    _rad = 0.0;
}

// Constructs an angle from a specfic value in radians.

SSAngle::SSAngle ( double rad )
{
    _rad = rad;
}

// Constructs an angle in radians from degrees, minutes, and seconds.

SSAngle::SSAngle ( SSDegMinSec dms )
{
    _rad = kRadPerDeg * ( dms.deg + dms.min / 60.0 + dms.sec / 3600.0 ) * ( dms.sign == '+' ? 1 : -1 );
}

// Constructs an angle in radians from hours, minutes, and seconds.

SSAngle::SSAngle ( SSHourMinSec hms )
{
    _rad = kRadPerHour * ( hms.hour + hms.min / 60.0 + hms.sec / 3600.0 );
}

// Constructs an angle in radians from an angle in arcseconds (360*60*60=1296000 arcseconds per circle)

SSAngle SSAngle::fromArcsec ( double arcsec )
{
    return SSAngle ( arcsec * kRadPerArcsec );
}

// Constructs an angle in radians from an angle in arcminutes (360*60=2100 arcminutes per circle)

SSAngle SSAngle::fromArcmin ( double arcmin )
{
    return SSAngle ( arcmin * kRadPerArcmin );
}

// Constructs an angle in radians from an angle in degrees (360 degrees per circle).

SSAngle SSAngle::fromDegrees ( double degrees )
{
    return SSAngle ( degrees * kRadPerDeg );
}

// Constructs an angle in radians from an angle in hours (24 hours per circle).

SSAngle SSAngle::fromHours ( double hours )
{
    return SSAngle ( hours * kRadPerHour );
}

// Reduces an angle in radians to the range 0 to kTwoPi.

SSAngle SSAngle::mod2Pi ( void )
{
    return SSAngle ( mod2pi ( _rad ) );
}

// Reduces an angle in radians to the range -kPi to +kPi.

SSAngle SSAngle::modPi ( void )
{
    return SSAngle ( modpi ( _rad ) );
}

// Returns arctangent of y / x in radians in the range 0 to kTwoPi.

SSAngle SSAngle::atan2Pi ( double y, double x )
{
    return SSAngle ( atan2pi ( y, x ) );
}
