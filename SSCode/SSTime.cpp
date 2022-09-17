//  SSTime.cpp
//  SSCore
//
//  Created by Tim DeBenedictis on 2/23/20.
//  Copyright © 2020 Southern Stars. All rights reserved.

#ifdef _MSC_VER
#include <windows.h>
#include <sstream>
#include <iomanip>
#else
#include <sys/time.h>
#endif

#include "SSAngle.hpp"
#include "SSTime.hpp"

// Converts a calendar date (year/month/day) in UTC to a Julian Date. Valid for all calendar dates.
// If the date is after 4 October 1582 it is assumed to be in the Gregorian calendar; otherwise, in the Julian calendar.
// From Jean Meeus, "Astronomical Algorithms", ch. 7, pp. 63-64.

double SSTime::CalendarToJD ( int y, short m, double d )
{
    if ( m < 3 )
    {
        y = y - 1;
        m = m + 12;
    }

    int b = 0;
    if ( ( y > 1582 ) || ( y == 1582 && m > 10 ) || ( y == 1582 && m == 10 && d >= 5.0 ) )
    {
        int a = floor ( y / 100.0 );
        b = 2 - a + floor ( a / 4.0 );
    }

    double jd = floor ( 365.25 * ( y + 4716 ) ) + floor ( 30.6001 * ( m + 1 ) ) + d + b - 1524.5;
    return jd;
}

static long tishri1 ( int y )
{
    long b = 31524 + 765433 * ( ( 235L * y - 234 ) / 19 );
    long d = b / 25920;
    long e = b % 25920;
    long f = 1 + d % 7;
    long g = ( 7L * y + 13 ) % 19 / 12;
    long h = ( 7L * y + 6 ) % 19 / 12;
    
    if ( e >= 19440 || ( e >= 9924 && f == 3 && g == 0 ) || ( e >= 16788 && f == 2 && g == 0 && h == 1 ) )
        d = d + 1;
    
    return d + ( ( d + 5 ) % 7 ) % 2 + 347997;
}

static long jewdays ( long k, long m )
{
    static int A[6][13] =
    {
        { 0, 30, 59, 88, 117, 147, 176, 206, 235, 265, 294, 324, 999 },
        { 0, 30, 59, 89, 118, 148, 177, 207, 236, 266, 295, 325, 999 },
        { 0, 30, 60, 90, 119, 149, 178, 208, 237, 267, 296, 326, 999 },
        { 0, 30, 59, 88, 117, 147, 177, 206, 236, 265, 295, 324, 354 },
        { 0, 30, 59, 89, 118, 148, 178, 207, 237, 266, 296, 325, 355 },
        { 0, 30, 60, 90, 119, 149, 179, 208, 238, 267, 297, 326, 356 },
    };

    return ( k > 0 && k < 7 && m > 0 && m < 14 ) ? A[k-1][m-1] : 0;
}

// Converts a Jewish calendar date to a Julian Date. Valid for Jewish years > 0.
// From "The Explanatory Supplement to the Astronomical Almanac", 3rd ed. (2012), pp. 619-621.
// and http://www.ricswal.plus.com/CALENDAR/ALGORITHMS.TXT

double SSTime::JewishToJD ( int y, short m, double d )
{
    long a = tishri1 ( y );
    long b = tishri1 ( y + 1 );
    long k = b - a - 352 - 27 * ( ( ( 7 * y + 13 ) % 19 ) / 12 );
    return a + jewdays ( k, m ) + d - 1.5;
}

// Converts a Gregorian calendar date to a Julian Date.
// Valid for all Gregorian calendar dates corresponding to JD >= 0, i.e. dates after -4713 November 23.
// From "The Explanatory Supplement to the Astronomical Almanac" (1992), pp. 603-606.

double SSTime::GregorianToJD ( int y, short m, double d )
{
    double    jd;
    
    jd = ( 1461 * ( y + 4800 + ( m - 14 ) / 12 ) ) / 4
       + ( 367 * ( m - 2 - 12 * ( ( m - 14 ) / 12 ) ) ) / 12
       - ( 3 * ( ( y + 4900 + ( m - 14 ) / 12 ) / 100 ) ) / 4
       + d - 32075.5;
       
    return ( jd );
}

// Converts a Julian calendar date to a Julian Date.
// Valid for all Julian calendar dates corresponding to JD >= 0, i.e. years after -4712.
// From "The Explanatory Supplement to the Astronomical Almanac" (1992), pp. 603-606.

double SSTime::JulianToJD ( int y, short m, double d )
{
    double jd;
    
    jd = 367 * y
       - ( 7 * ( y + 5001 + ( m - 9 ) / 7 ) ) / 4
       + ( 275 * m ) / 9 + d + 1729776.5;
       
    return ( jd );
}

// Converts an Islamic civil calendar date (y/m/d) to a Julian Date.
// Valid for all years >= 1, corresponding to JD >= 1948440. From:
// "The Explanatory Supplement to the Astronomical Almanac" (1992), pp. 603-606.

double SSTime::IslamicToJD ( int y, short m, double d )
{
    int      jd0 = 1948440;
    double   jd;
    
    jd = ( 11 * y + 3 ) / 30
       + 354 * y
       + 30 * m
       - ( m - 1 ) / 2
       + d + jd0 - 385.5;
       
    return jd;
}

// Converts an Indian civil calendar date (y/m/d) to a Julian Date.
// Valid for all years >= 1, corresponding to JD >= 17499995. From:
// "The Explanatory Supplement to the Astronomical Almanac" (1992), pp. 603-606.

double SSTime::IndianToJD ( int y, short m, double d )
{
    double jd;
    
    jd = 365 * y + ( y + 78 - 1 / m ) / 4 + 31 * m - ( m + 9 ) / 11
       - ( m / 7 ) * ( m - 7 )
       - ( 3 * ( ( y + 78 - 1 / m ) / 100 + 1 ) ) / 4
       + d + 1749578.5;
       
    return ( jd );
}

// Converts a Julian Date (jd) to a celendar date (year/month/day) in UTC. Valid for all Julian Dates.
// If the JD is after 2299161 it is assumed to be in the Gregorian calendar; otherwise, in the Julian calendar.
// From Jean Meeus, "Astronomical Algorithms", ch. 7, pp. 63-64.

void SSTime::JDToCalendar ( double jd, int &year, short &month, double &day )
{
    double    j = jd + 0.5;
    int       z = floor ( j ), a = 0;
    double    f = j - z;
    
    if ( j >= 2299161.0 )
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
}

// Converts a Julian Date (jd) to a date (y/m/d) in the Gregorian calendar.
// Valid for all Gregorian calendar dates corresponding to JD >= 0, i.e. dates after -4713 November 23.
// From "The Explanatory Supplement to the Astronomical Almanac" (1992), pp. 603-606.

void SSTime::JDToGregorian ( double jd, int &y, short &m, double &d )
{
    int    l, n, i, j;
    double f;
    
    jd = jd + 0.5;
    j = floor ( jd );
    f = jd - j;
    
    l = j + 68569;
    n = ( 4 * l ) / 146097;
    l = l - ( 146097 * n + 3 ) / 4;
    i = ( 4000 * ( l + 1 ) ) / 1461001;
    l = l - ( 1461 * i ) / 4 + 31;
    j = ( 80 * l ) / 2447;
    
    d = l - ( 2447 * j ) / 80 + f;
    l = j / 11;
    m = j + 2 - 12 * l;
    y = 100 * ( n - 49 ) + i + l;
}

// Converts a Julian Date (jd) to a date (y/m/d) in the Julian calendar.
// Valid for all Julian calendar dates corresponding to JD >= 0, i.e. years after -4712.
// From "The Explanatory Supplement to the Astronomical Almanac" (1992), pp. 603-606.

void SSTime::JDToJulian ( double jd, int &y, short &m, double &d )
{
    int    j, k, l, n, i;
    double f;
    
    jd = jd + 0.5;
    j = floor ( jd );
    f = jd - j;

    j = j + 1402;
    k = ( j - 1 ) / 1461;
    l = j - 1461 * k;
    n = ( l - 1 ) / 365 - l / 1461;
    i = l - 365 * n + 30;
    j = ( 80 * i ) / 2447;
    
    d = i - ( 2447 * j ) / 80 + f;
    i = j / 11;
    m = j + 2 - 12 * i;
    y = 4 * k + n + i - 4716;
}

// Converts a Julian Date to a Jewish calendar date. Valid for Jewish years > 0, i.e. JD >= 347997.5.
// From "The Explanatory Supplement to the Astronomical Almanac", 3rd ed. (2012), pp. 619-621.
// and http://www.ricswal.plus.com/CALENDAR/ALGORITHMS.TXT

void SSTime::JDToJewish ( double jd, int &y, short &m, double &d )
{
    jd = jd + 0.5;
    long j = floor ( jd );
    double f = jd - j;

    long M = ( 25920 * ( j - 347996 ) ) / 765433;
    
    y = 19 * ( M / 235 ) + ( 19 * ( M % 235 ) - 2 ) / 235 + 1;
    long j1 = tishri1 ( y );
    if ( j1 > j )
        y = y - 1;
    
    long a = tishri1 ( y );
    long b = tishri1 ( y + 1 );
    long k = b - a - 352 - 27 * ( ( ( 7 * y + 13 ) % 19 ) / 12 );
    long c = j - a + 1;
    
    for ( m = 1; m < 14; m++ )
        if ( jewdays ( k, m ) >= c )
            break;
    
    m = m - 1;
    d = c - jewdays ( k, m ) + f;
}

// Converts a Julian date (jd) to a date in the Islamic civil calendar (y/m/d).
// Valid for all years >= 1, corresponding to JD >= 1948440. From:
// "The Explanatory Supplement to the Astronomical Almanac" (1992), pp. 603-606.

void SSTime::JDToIslamic ( double jd, int &y, short &m, double &d )
{
    int     jd0 = 1948440, l, n, j;
    double  f;
    
    jd = jd + 0.5;
    j = floor ( jd );
    f = jd - j;

    l = j - jd0 + 10632;
    n = ( l - 1 ) / 10631;
    l = l - 10631 * n + 354;
    j = ( ( 10985 - l ) / 5316 ) * ( ( 50 * l ) / 17719 )
      + ( l / 5670 ) * ( ( 43 * l ) / 15238 );
    l = l - ( ( 30 - j ) / 15 ) * ( ( 17719 * j ) / 50 )
      - ( j / 16 ) * ( ( 15238 * j ) / 43 ) + 29;
      
    m = ( 24 * l ) / 709;
    d = l - ( m * 709 ) / 24 + f;
    y = 30 * n + j - 30;
}

// Converts a Julian date (jd) to a date in the Indian civil calendar (y/m/d).
// Valid for all years >= 1, corresponding to JD >= 1749995. From:
// "The Explanatory Supplement to the Astronomical Almanac" (1992), pp. 603-606.

void SSTime::JDToIndian ( double jd, int &y, short &m, double &d )
{
    int    l, n, i, j;
    double f;
    
    jd = jd + 0.5;
    j = floor ( jd );
    f = jd - j;

    l = j + 68518;
    n = ( 4 * l ) / 146097;
    l = l - ( 146097 * n + 3 ) / 4;
    i = ( 4000 * ( l + 1 ) ) / 1461001;
    l = l - ( 1461 * i ) / 4 + 1;
    j = ( ( l - 1 ) / 31 ) * ( 1 - l / 185 )
      + ( l / 185 ) * ( ( l - 156 ) / 30 + 5 ) - l / 366;
      
    d = l - 31 * j + ( ( j + 2 ) / 8 ) * ( j - 5 ) + f;
    l = j / 11;
    m = j + 2 - 12 * l;
    y = 100 * ( n - 49 ) + l + i - 78;
}

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

SSDate::SSDate ( SSTime time, SSCalendar cal )
{
    double  dayf = time.jd + time.zone / 24.0;
    
    if ( cal == kGregorian )
        SSTime::JDToGregorian ( dayf, year, month, dayf );
    else if ( cal == kJulian )
        SSTime::JDToJulian ( dayf, year, month, dayf );
    else if ( cal == kJewish )
        SSTime::JDToJewish ( dayf, year, month, dayf );
    else if ( cal == kIslamic )
        SSTime::JDToIslamic ( dayf, year, month, dayf );
    else if ( cal == kIndian )
        SSTime::JDToIndian ( dayf, year, month, dayf );
    else    // kGregorianJulian
        SSTime::JDToCalendar ( dayf, year, month, dayf );

    day = floor ( dayf );
    dayf = dayf - day;
    
    hour = dayf * 24.0;
    min = dayf * 1440.0 - hour * 60.0;
    sec = dayf * 86400.0 - hour * 3600.0 - min * 60.0;

    calendar = cal;
    zone = time.zone;
}

// Constructs date from string using the same format argument(s) as strptime().
// Date will be filled with zeros if string not parsed sucessfully.

SSDate::SSDate ( const string &fmt, const string &str )
{
    year = month = day = sec = hour = min = sec = zone = 0;
    calendar = kGregorian;
    parse ( fmt, str );
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
// Not thread-safe!

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
    
#ifndef _MSC_VER
    time.tm_gmtoff = zone * 3600.0;
#endif
    
    // time.tm_gmtoff is not available on Windows, and not used when formatting %z on Apple platforms (but is used for %z on Linux/Emscripten).
    // To make strftime() output time zone on Windows, Mac, and iOS, we must temporarily change the timezone global.
    // See https://www.gnu.org/software/libc/manual/html_node/Time-Zone-Functions.html
    
    double oldzone = get_timezone();
    set_timezone ( zone );
    strftime ( str, sizeof ( str ), fmt.c_str(), &time );
    set_timezone ( oldzone );
    
    return string ( str );
}

// Converts string to date using the same format argument(s) as strptime().
// Overwrites all internal fields except time zone and calendar system.
// Returns true if string parsed successfully or false otherwise.

bool SSDate::parse ( const string &fmt, const string &str )
{
    struct tm time = { 0 };
    int negyear = startsWith ( fmt, "%Y" ) && str[0] == '-';
        
#ifdef WIN32
    stringstream ss ( str );
    ss >> get_time ( &time, fmt.c_str() + negyear );
    if ( ss.fail() )
        return false;
#else
    if ( strptime ( str.c_str() + negyear, fmt.c_str(), &time ) == nullptr )
        return false;
#endif

    year = time.tm_year + 1900;
    month = time.tm_mon + 1;
    day = time.tm_mday;
    hour = time.tm_hour;
    min = time.tm_min;
    sec = time.tm_sec;
    
    if ( negyear )
        year = -year;
    
    return true;
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

SSTime::SSTime ( SSDate date )
{
    double dayf = date.day + date.hour / 24.0 + date.min / 1440.0 + date.sec / 86400.0 - date.zone / 24.0;
    
    if ( date.calendar == kGregorian )
        jd = SSTime::GregorianToJD ( date.year, date.month, dayf );
    else if ( date.calendar == kJulian )
        jd = SSTime::JulianToJD ( date.year, date.month, dayf );
    else if ( date.calendar == kJewish )
        jd = SSTime::JewishToJD ( date.year, date.month, dayf );
    else if ( date.calendar == kIslamic )
        jd = SSTime::IslamicToJD ( date.year, date.month, dayf );
    else if ( date.calendar == kIndian )
        jd = SSTime::IndianToJD ( date.year, date.month, dayf );
    else // kGregorianJulian
        jd = SSTime::CalendarToJD ( date.year, date.month, dayf );

    zone = date.zone;
}

// Constructs a time from the computer system time; the local time zone is also read from the system.
// The calendar system is set to use Gregorian.

SSTime SSTime::fromSystem ( void )
{
#ifdef _MSC_VER
    TIME_ZONE_INFORMATION tzinfo = { 0 };
    GetTimeZoneInformation ( &tzinfo );
    double zone = -tzinfo.Bias / 60.0;

    SYSTEMTIME    systime = { 0 };
    GetLocalTime ( &systime );
    SSDate date ( kGregorian, zone, systime.wYear, systime.wMonth, systime.wDay, systime.wHour, systime.wMinute, systime.wSecond + systime.wMilliseconds / 1000.0 );

    return SSTime ( date );
#else
    time_t t = time ( nullptr );
    struct tm lt = { 0 };
    struct timeval tv = { 0 };

    localtime_r ( &t, &lt );
    double zone = lt.tm_gmtoff / 3600.0;

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
