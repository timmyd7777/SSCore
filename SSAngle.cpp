//  SSAngle.cpp
//  SSCore
//
//  Created by Tim DeBenedictis on 2/24/20.
//  Copyright © 2020 Southern Stars. All rights reserved.

#include <math.h>
#include "SSAngle.hpp"

// Returns C++ string constructed from printf()-style input arguments.

string format ( const char *fmt, ... )
{
	char buf[1024] = { 0 };

    va_list args;
    va_start ( args, fmt );
    vsnprintf ( buf, sizeof buf, fmt, args );
    va_end ( args );

	return string ( buf );
}

// Returns a C++ string which has leading and trailing whitespace
// trimmed from the input string (does not modify input string).

string trim ( string str )
{
    auto start = str.find_first_not_of ( " \t\r\n" );
    auto end = str.find_last_not_of ( " \t\r\n" );

    if ( start == string::npos )
        return string ( "" );
    else
        return str.substr ( start, ( end - start ) + 1 );
}

// Converts string to 32-bit signed integer.
// Returns zero if string cannot be converted.

int strtoint ( string str )
{
	int i;
	
	try
	{
		i = stoi ( str );
	}
	catch ( ... )
	{
		i = 0;
	}
	
	return i;
}

// Converts string to 64-bit signed integer.
// Returns zero if string cannot be converted.

int64_t strtoint64 ( string str )
{
	int64_t i;
	
	try
	{
		i = stoll ( str );
	}
	catch ( ... )
	{
		i = 0;
	}
	
	return i;
}

// Converts string to 32-bit single precision floating point value.
// Returns zero if string cannot be converted.

float strtofloat ( string str )
{
	float f;
	
	try
	{
		f = stof ( str );
	}
	catch ( ... )
	{
		f = 0.0;
	}
	
	return f;
}

// Converts string to 64-bit double precision floating point value.
// Returns zero if string cannot be converted.

double strtofloat64 ( string str )
{
	double d;
	
	try
	{
		d = stof ( str );
	}
	catch ( ... )
	{
		d = 0.0;
	}
	
	return d;
}

// Constructs an angular value in degrees, minutes, seconds with the given sign.

SSDegMinSec::SSDegMinSec ( char sign, short deg, short min, double sec )
{
	this->sign = sign;
	this->deg = deg;
	this->min = min;
	this->sec = sec;
}

// Constructs an angular value in degrees, minutes, seconds from an angle in radians.

SSDegMinSec::SSDegMinSec ( SSAngle ang )
{
    double degrees = fabs ( ang.toDegrees() );

	sign = ang >= 0.0 ? '+' : '-';
	deg = degrees;
	min = 60.0 * ( degrees - deg );
	sec = 3600.0 * ( degrees - deg - min / 60.0 );
}

// Constructs an angular value from a string in degrees, minutes, and seconds

SSDegMinSec::SSDegMinSec ( string str )
{
    const char *cstr = str.c_str();
    
    deg = min = sec = 0;
    sscanf ( cstr, "%hd %hd %lf", &deg, &min, &sec );
	deg = abs ( deg );
    sign = cstr[0] == '-' ? '-' : '+';
}

// Converts an angle in degrees, minutes, seconds to a string

string SSDegMinSec::toString ( void )
{
	return format ( "%c%02hd %02hd %04.1f", sign, deg, min, sec );
}

// Constructs an angular value in hours, minutes, seconds with the given sign.

SSHourMinSec::SSHourMinSec ( char sign, short hour, short min, double sec )
{
	this->sign = sign;
	this->hour = hour;
	this->min = min;
	this->sec = sec;
}

// Constructs an angular value in hours, minutes, seconds from an angle in radians.

SSHourMinSec::SSHourMinSec ( SSAngle ang )
{
	double hours = fabs ( ang.toHours() );

	sign = ang >= 0.0 ? '+' : '-';
    hour = hours;
    min = 60.0 * ( hours - hour );
    sec = 3600.0 * ( hours - hour - min / 60.0 );
}

// Constructs an angular value from a string in degrees, minutes, and seconds

SSHourMinSec::SSHourMinSec ( string str )
{
    const char *cstr = str.c_str();
    
    hour = min = sec = 0;
    sscanf ( cstr, "%hd %hd %lf", &hour, &min, &sec );
	hour = abs ( hour );
    sign = cstr[0] == '-' ? '-' : '+';
}

// Converts an angle in hours, minutes, seconds to a string.  OMITS SIGN!

string SSHourMinSec::toString ( void )
{
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
	_rad = kRadPerHour * ( hms.hour + hms.min / 60.0 + hms.sec / 3600.0 ) * ( hms.sign == '+' ? 1 : -1 );
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
    return SSAngle ( _rad - kTwoPi * floor ( _rad / kTwoPi ) );
}

// Reduces an angle in radians to the range -kPi to +kPi.

SSAngle SSAngle::modPi ( void )
{
    SSAngle x = mod2Pi();
    
    if ( x > kPi )
        x -= kTwoPi;

    return x;
}

// Returns arctangent of y / x in radians in the range 0 to kTwoPi.

SSAngle SSAngle::atan2Pi ( double y, double x )
{
	if ( y < 0.0 )
		return SSAngle ( atan2 ( y, x ) + kTwoPi );
	else
		return SSAngle ( atan2 ( y, x ) );
}

// Converts an angle in radians to degrees

double toDegrees ( double rad )
{
	return rad * SSAngle::kDegPerRad;
}

// Converts an angle in degrees to radians

double toRadians ( double deg )
{
	return deg * SSAngle::kRadPerDeg;
}
