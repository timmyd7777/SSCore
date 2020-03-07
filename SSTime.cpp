//  SSTime.cpp
//  SSCore
//
//  Created by Tim DeBenedictis on 2/23/20.
//  Copyright © 2020 Southern Stars. All rights reserved.

#include <sys/time.h>

#include "SSAngle.hpp"
#include "SSTime.hpp"

SSTime::SSTime ( void )
{
	jd = kJ2000;
	zone = 0.0;
}

SSTime::SSTime ( double jd, double zone )
{
    this->jd = jd;
	this->zone = zone;
}

SSTime SSTime::fromSystem ( void )
{
    struct timeval    tv = { 0 };
    struct timezone    tz = { 0 };
    
    gettimeofday ( &tv, &tz );
    double jd = kJ1970 + ( tv.tv_sec + tv.tv_usec / 1000000.0 ) / 86400.0;
    double zone = -tz.tz_minuteswest / 60.0;
    
    return SSTime ( jd, zone );
}

SSTime SSTime::fromUnixTime ( time_t time )
{
    return SSTime ( time / kSecondsPerDay + kJ1970, 0.0 );
}

SSTime SSTime::fromJulianYear ( double year )
{
    return SSTime ( kJ2000 + kDaysPerJulianYear * ( year - 2000.0 ), 0.0 );
}

SSTime SSTime::fromBesselianYear ( double year )
{
    return SSTime ( kB1900 + kDaysPerBesselianYear * ( year - 1900.0 ), 0.0 );
}

SSTime SSTime::fromCalendarDate ( SSCalendar calendar, double zone, int year, short month, double day, short hour, short minute, double second )
{
    day += hour / 24.0 + minute / 1440.0 + second / 86400.0 - zone / 24.0;

    if ( month < 3 )
    {
        year = year - 1;
        month = month + 12;
    }

    int b = 0;
    if ( calendar == kSSCalendarGregorian )
    {
        int a = floor ( year / 100.0 );
        b = 2 - a + floor ( a / 4.0 );
    }

    double jd = floor ( 365.25 * ( year + 4716 ) ) + floor ( 30.6001 * ( month + 1 ) ) + day + b - 1524.5;
    return SSTime ( jd, zone );
}

void SSTime::toCalendarDate ( SSCalendar calendar, double zone, int &year, short &month, double &day, short &hour, short &minute, double &second )
{
	double	j = jd + 0.5 + zone / 24.0;
	int		z = floor ( j ), a = 0;
	double	f = j - z;
	
	if ( calendar == kSSCalendarGregorian )
	{
		a = ( z - 1867216.25 ) / 36524.25;
		a = z + 1 + a - a / 4;
	}
	else
	{
		a = z;
	}
	
	int b = a + 1524;
	int c = floor ( ( b - 122.1 ) / 365.25 );
	int d = floor ( 365.25 * c );
	int e = ( b - d ) / 30.6001;

	day = b - d - floor ( 30.6001 * e ) + f;

	if ( e < 14 )
		month = e - 1;
	else
		month = e - 13;

	if ( month > 2 )
		year = c - 4716;
	else
		year = c - 4715;

	hour = f * 24.0;
	minute = f * 1440.0 - hour * 60.0;
	second = f * 86400.0 - hour * 3600.0 - minute * 60.0;
}

SSTime::CalendarDate SSTime::toCalendarDate ( SSCalendar calendar, double zone )
{
    SSTime::CalendarDate    date = { calendar, zone, 0, 0, 0, 0, 0, 0 };
    
    toCalendarDate ( calendar, zone, date.year, date.month, date.day, date.hour, date.min, date.sec );
    
    return ( date );
}

SSTime::CalendarDate SSTime::toCalendarDate ( SSCalendar calendar )
{
    return toCalendarDate ( calendar, zone );
}

double SSTime::toJulianYear ( void )
{
    return ( jd - kJ2000 ) / kDaysPerJulianYear + 2000.0;
}

double SSTime::toBesselianYear ( void )
{
    return ( jd - kB1900 ) / kDaysPerJulianYear + 1900.0;
}

int SSTime::getWeekday ( void )
{
	int	d = floor ( jd + zone / 24.0 + 0.5 );
	
	d = d % 7;
	if ( d < 0 )
		d += 7;
		
	return ( d );
}

double SSTime::getDeltaT ( void )
{
	double y = toJulianYear() - 0.5 / 12.0;
	double u, u2, u3, u4, u5, u6;
	double t, t2, t3, t4, t5, t6, t7;
	double dt = 0;
	
	if ( y < -500.0 )
	{
		u = ( y - 1820.0 ) / 100.0;
		dt = -20.0 + 32.0 * u * u;
	}
	else if ( y < 500.0 )
	{
		u = y / 100.0;
		u2 = u * u;
		u3 = u2 * u;
		u4 = u3 * u;
		u5 = u4 * u;
		u6 = u5 * u;
		dt = 10538.6 - 1014.41 * u + 33.78311 * u2 - 5.952053 * u3
		- 0.1798452 * u4 + 0.022174192 * u5 + 0.0090316521 * u6;
	}
	else if ( y < 1600.0 )
	{
		u = ( y - 1000.0 ) / 100.0;
		u2 = u * u;
		u3 = u2 * u;
		u4 = u3 * u;
		u5 = u4 * u;
		u6 = u5 * u;
		dt = 1574.2 - 556.01 * u + 71.23472 * u2 + 0.319781 * u3
		- 0.8503463 * u4 - 0.005050998 * u5 + 0.0083572073 * u6;
	}
	else if ( y < 1700.0 )
	{
		t = y - 1600.0;
		t2 = t * t;
		t3 = t2 * t;
		dt = 120.0 - 0.9808 * t - 0.01532 * t2 + t3 / 7129.0;
	}
	else if ( y < 1800.0 )
	{
		t = y - 1700.0;
		t2 = t * t;
		t3 = t2 * t;
		t4 = t3 * t;
		dt = 8.83 + 0.1603 * t - 0.0059285 * t2 + 0.00013336 * t3 - t4 / 1174000.0;
	}
	else if ( y < 1860.0 )
	{
		t = y - 1800.0;
		t2 = t * t;
		t3 = t2 * t;
		t4 = t3 * t;
		t5 = t4 * t;
		t6 = t5 * t;
		t7 = t6 * t;
		dt = 13.72 - 0.332447 * t + 0.0068612 * t2 + 0.0041116 * t3 - 0.00037436 * t4
		+ 0.0000121272 * t5 - 0.0000001699 * t6 + 0.000000000875 * t7;
	}
	else if ( y < 1900.0 )
	{
		t = y - 1860.0;
		t2 = t * t;
		t3 = t2 * t;
		t4 = t3 * t;
		t5 = t4 * t;
		dt = 7.62 + 0.5737 * t - 0.251754 * t2 + 0.01680668 * t3
		- 0.0004473624 * t4 + t5 / 233174.0;
	}
	else if ( y < 1920.0 )
	{
		t = y - 1900.0;
		t2 = t * t;
		t3 = t2 * t;
		t4 = t3 * t;
		dt = -2.79 + 1.494119 * t - 0.0598939 * t2 + 0.0061966 * t3 - 0.000197 * t4;
	}
	else if ( y < 1940.0 )
	{
		t = y - 1920.0;
		t2 = t * t;
		t3 = t2 * t;
		dt = 21.20 + 0.84493 * t - 0.076100 * t2 + 0.0020936 * t3;
	}
	else if ( y < 1960.0 )
	{
		t = y - 1950.0;
		t2 = t * t;
		t3 = t2 * t;
		dt = 29.07 + 0.407 * t - t2 / 233.0 + t3 / 2547.0;
	}
	else if ( y < 1985.0 )
	{
		t = y - 1975.0;
		t2 = t * t;
		t3 = t2 * t;
		dt = 45.45 + 1.067 * t - t2 / 260.0 - t3 / 718.0;
	}
	else if ( y < 2005.0 )
	{
		t = y - 2000.0;
		t2 = t * t;
		t3 = t2 * t;
		t4 = t3 * t;
		t5 = t4 * t;
		dt = 63.86 + 0.3345 * t - 0.060374 * t2 + 0.0017275 * t3 + 0.000651814 * t4
		+ 0.00002373599 * t5;
	}
	else if ( y < 2050.0 )
	{
		t = y - 2000.0;
		t2 = t * t;
		// 		The new formula below the commented-out original better fits actual
		//		published Delta T data from 2000 to 2015, and still maintains the
		//		projected value of 93 seconds at year 2050.
		//		dt = 62.92 + 0.32217 * t + 0.005589 * t2;
		dt = 63.83 + 0.1102 * t + 0.009464 * t2;
	}
	else if ( y < 2150.0 )
	{
		u = ( y - 1820.0 ) / 100.0;
		dt = -20 + 32 * u * u - 0.5628 * ( 2150.0 - y );
	}
	else
	{
		u = ( y - 1820.0 ) / 100.0;
		dt = -20.0 + 32.0 * u * u;
	}
	
	return ( dt );
}

double SSTime::getGreenwichMeanSiderealTime ( void )
{
	double jd0 = floor ( jd - 0.5 ) + 0.5;
	double t = ( jd0 - 2451545.0 ) / 36525.0;
	double t2 = t * t;
	double t3 = t2 * t;
	double gmst = 280.46061837 + 360.98564736629 * ( jd - kJ2000 ) + 0.000387933 * t2 - t3 / 38710000.0;

	return ( SSAngle::fromDegrees ( gmst ).mod2Pi().a );
}
