//  SSTime.cpp
//  SSCore
//
//  Created by Tim DeBenedictis on 2/23/20.
//  Copyright © 2020 Southern Stars. All rights reserved.

#ifndef ANDROID
#define _GNU_SOURCE /* for tm_gmtoff and tm_zone */
#endif

#ifdef _WIN32
#include <windows.h>
#else
#include <sys/time.h>
#endif

#include "SSAngle.hpp"
#include "SSTime.hpp"

// Constructs a calendar date/time from the specified calendar system, local time zone in hours east of UTC,
// and year/month/day including fractional part of day.

SSDate::SSDate ( SSCalendar calendar, double zone, int year, short month, double dayf )
{
    this->calendar = calendar;
    this->zone = zone;
    this->year = year;
    this->month = month;
    this->day = floor ( dayf );
    this->hour = 24.0 * ( dayf - day );
    this->min = 1440.0 * ( dayf - day - hour / 24.0 );
    this->sec = 86400.0 * ( dayf - day - hour / 24.0 - min / 1440.0 );
}

// Constructs a calendar date/time from the specified calendar system, local time zone in hours east of UTC,
// and year/month/day hour:min:sec.

SSDate::SSDate ( SSCalendar calendar, double zone, int year, short month, short day, short hour, short min, double sec )
{
    this->calendar = calendar;
    this->zone = zone;
    this->year = year;
    this->month = month;
    this->day = day;
    this->hour = hour;
    this->min = min;
    this->sec = sec;
}

// Constructs a local calendar date/time from an SSTime object representing a moment in time as a Julian Date.
// The SSTime object specifies the local time zone used for converting to calendar date/time.
// From Jean Meeus, "Astronomical Algorithms", ch. 7, pp. 63-64.

SSDate::SSDate ( SSTime time, SSCalendar cal )
{
    double    j = time.jd + 0.5 + time.zone / 24.0;
    int       z = floor ( j ), a = 0;
    double    f = j - z;
    
    if ( cal == kGregorian )
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

    day = b - d - floor ( 30.6001 * e );
    
    if ( e < 14 )
        month = e - 1;
    else
        month = e - 13;

    if ( month > 2 )
        year = c - 4716;
    else
        year = c - 4715;

    hour = f * 24.0;
    min = f * 1440.0 - hour * 60.0;
    sec = f * 86400.0 - hour * 3600.0 - min * 60.0;

    calendar = cal;
    zone = time.zone;
}

// Returns an SSDate object representing the calendar date that corresponds
// to the Julian date in the provided SSTime object, using the time zone
// stored in the SSTime object, and the specified calendar system (calendar).

SSDate SSDate::fromJulianDate ( SSTime time, SSCalendar cal )
{
    return SSDate ( time, cal );
}

// Returns an SSTime object representing the Julian Date that corresponds to this calendar date.

SSTime SSDate::toJulianDate ( void )
{
    return SSTime ( *this );
}

// Converts date to a string using the same format argument(s) as strftime().

string SSDate::format ( const string &fmt )
{
    char str[1024] = { 0 };
    struct tm time = { 0 };
    
    time.tm_year = year - 1900;
    time.tm_mon = month - 1;
    time.tm_mday = day;
    time.tm_hour = hour;
    time.tm_min = min;
    time.tm_sec = sec;
    time.tm_wday = SSTime ( *this ).getWeekday();
#ifndef _WINDOWS
    time.tm_gmtoff = zone * 3600.0;
#endif

    strftime ( str, sizeof ( str ), fmt.c_str(), &time );
    return string ( str );
}

// Constructs a time with default values of 1.5 Jan 2000 UTC.

SSTime::SSTime ( void )
{
    jd = kJ2000;
    zone = 0.0;
}

// Constructs a time from a specific Julian date in UTC.

SSTime::SSTime ( double jd )
{
    this->jd = jd;
    this->zone = 0.0;
}

// Constructs a time from a specific Julian date and time zone in hours east of Greenwich.

SSTime::SSTime ( double jd, double zone )
{
    this->jd = jd;
    this->zone = zone;
}

// Constructs a time from an SSDate object represeting a local calendar date/time.
// The calendar system and local time zone are specified in the SSDate object.
// From Jean Meeus, "Astronomical Algorithms", ch. 7, pp. 60-61.

SSTime::SSTime ( SSDate date )
{
    double day = date.day + date.hour / 24.0 + date.min / 1440.0 + date.sec / 86400.0 - date.zone / 24.0;
    
    if ( date.month < 3 )
    {
        date.year = date.year - 1;
        date.month = date.month + 12;
    }

    int b = 0;
    if ( date.calendar == kGregorian )
    {
        int a = floor ( date.year / 100.0 );
        b = 2 - a + floor ( a / 4.0 );
    }

    jd = floor ( 365.25 * ( date.year + 4716 ) ) + floor ( 30.6001 * ( date.month + 1 ) ) + day + b - 1524.5;
    zone = date.zone;
}

// Constructs a time from the computer system time; the local time zone is also read from the system.
// The calendar system is set to use Gregorian.

SSTime SSTime::fromSystem ( void )
{
#ifdef _WIN32
    TIME_ZONE_INFORMATION tzinfo = { 0 };
    GetTimeZoneInformation ( &tzinfo );
    double zone = -tzinfo.Bias / 60.0;

    SYSTEMTIME    systime = { 0 };
    GetLocalTime ( &systime );
    SSDate date ( kGregorian, zone, systime.wYear, systime.wMonth, systime.wDay, systime.wHour, systime.wMinute, systime.wSecond + systime.wMilliseconds / 1000.0 );

    return SSTime ( date );
#else
    time_t t = time ( nullptr );
    double zone = localtime ( &t )->tm_gmtoff / 3600.0;
    
    struct timeval tv = { 0 };
    gettimeofday ( &tv, nullptr );
    double jd = kJ1970 + ( tv.tv_sec + tv.tv_usec / 1000000.0 ) / 86400.0;

    return SSTime ( jd, zone );
#endif
}

// Constructs a time from an arbitrary unix time (i.e. seconds since 1.0 Jan 1970 UTC).
// The time zone is UTC.

SSTime SSTime::fromUnixTime ( time_t time )
{
    return SSTime ( time / kSecondsPerDay + kJ1970 );
}

// Constructs a time from a Julian year number (i.e. Julian years since 1.5 Jan 2000 UTC).
// The time zone is UTC.

SSTime SSTime::fromJulianYear ( double year )
{
    return SSTime ( kJ2000 + kDaysPerJulianYear * ( year - 2000.0 ) );
}

// Constructs a time from a Besselian year number (i.e. Besselian years since 1.0 Jan 1900).
// The time zone is UTC.

SSTime SSTime::fromBesselianYear ( double year )
{
    return SSTime ( kB1900 + kDaysPerBesselianYear * ( year - 1900.0 ), 0.0 );
}

// Returns the unix time (i.e. seconds since 1.0 Jan 1970 UTC)
// corresponding to this time object's Julian Date.

time_t SSTime::toUnixTime ( void )
{
    return kSecondsPerDay * ( jd - kJ1970 );
}

// Returns the Julian year corresponding to this time object's Julian Date
// (i.e. Julian years since 1.5 Jan 2000 UTC).

double SSTime::toJulianYear ( void )
{
    return ( jd - kJ2000 ) / kDaysPerJulianYear + 2000.0;
}

// Returns the Besselian year corresponding to this time object's Julian Date
// (i.e. Besselian years since 1.0 Jan 1900).

double SSTime::toBesselianYear ( void )
{
    return ( jd - kB1900 ) / kDaysPerJulianYear + 1900.0;
}

// Returns the local week day; 0 = Sun, 1 = Mon, etc.

int SSTime::getWeekday ( void )
{
    int    d = floor ( jd + zone / 24.0 + 0.5 );
    
    d = ( d + 1 ) % 7;
    if ( d < 0 )
        d += 7;
        
    return ( d );
}

// Returns the time offset in seconds from civil time (UT) to dynamic time (DT)
// at this time object's current Julian Date, i.e. DT = UT + DeltaT.
// From an algorithm by F. Espenak and J. Meeus, described here:
// https://eclipse.gsfc.nasa.gov/SEhelp/deltatpoly2004.html

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
        //         The new formula below the commented-out original better fits actual
        //        published Delta T data from 2000 to 2015, and still maintains the
        //        projected value of 93 seconds at year 2050.
        //        dt = 62.92 + 0.32217 * t + 0.005589 * t2;
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

// Returns the Julian Ephemeris Date (i.e., TDT) corresponding
// to this time object's current Julian Date, which is in UTC.

double SSTime::getJulianEphemerisDate ( void )
{
    return jd + getDeltaT() / kSecondsPerDay;
}

// Returns local mean sidereal time as an angle in radians
// at this time object's current Julian Date and a particular longitude
// in radians, where east is positive, wast is negative.
// For Greenwich Mean Sidereal Time, pass lon = 0.0.
// Note: add nutation in longitude to obtain local apparent (true) sidereal time.
// From Jean Meeus, "Astronomical Algorithms", ch. 12, p. 88

SSAngle SSTime::getSiderealTime ( SSAngle lon )
{
    double jd0 = floor ( jd - 0.5 ) + 0.5;
    double t = ( jd0 - 2451545.0 ) / 36525.0;
    double t2 = t * t;
    double t3 = t2 * t;
    double gmst = 280.46061837 + 360.98564736629 * ( jd - kJ2000 ) + 0.000387933 * t2 - t3 / 38710000.0;
    
    return ( SSAngle::fromDegrees ( gmst ) + lon ).mod2Pi();
}

// Returns the Julian Dates that corresponds to the start of the local day.
     
SSTime SSTime::getLocalMidnight ( void )
{
    double jd0 = floor ( jd - 0.5 + zone / 24.0 ) + 0.5 - zone / 24.0;
    return SSTime ( jd0, zone );
}
