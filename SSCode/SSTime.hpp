// SSTime.hpp
// SSCore
//
// Created by Tim DeBenedictis on 2/23/20.
// Copyright © 2020 Southern Stars. All rights reserved.
// 
// Classes for converting between Julian Day and calendar date/time; and between civil and dynamic time.

#ifndef SSTime_hpp
#define SSTime_hpp

#include <string>
#include <stdio.h>
#include <math.h>
#include <time.h>

#include "SSAngle.hpp"

using namespace std;

// Identifers for the calendar systems used in SSDate and SSTime.

enum SSCalendar
{
    kGregorianJulian = -1, // Gregorian after 4 October 1582; Julian before 15 October 1582.
    kGregorian = 0,        // Gregorian calendar, used after 15 October 1582 (JD 2299161).  Leap years every 4th year, but not every 100th, unless also 400th.
    kJulian = 1,           // Julian calendar, used before 4 October 1582 (JD 2299161).  Leap years every 4 years.
    kJewish = 2,           // modern Jewish calendar
    kIslamic = 3,          // Islamic civil calendar
    kIndian = 4            // Indian civil calendat
};

// Represents an instant in time as a calendar date in a local time zone.

struct SSTime;

struct SSDate
{
    SSCalendar calendar;   // calendar system: kGregorianJulian, kJewish, kIslamic, etc.
    double zone;           // local time zone offset from UTC in hours east of Greenwich; west is negative!
    int year;              // calendar year; note 0 = 1 BC, -1 = 2 BC, etc.
    short month;           // month; 1 = Jan, 2 = Feb, etc.
    short day;             // day of month, 1 to 31
    short hour;            // hour of day; 0 to 23
    short min;             // minute of hour; 0 to 59
    double sec;            // seconds of minute including fractional part; 0 to 59.999...
    
    SSDate ( SSCalendar calendar, double zone, int year, short month, short day, short hour, short min, double sec );
    SSDate ( SSCalendar calendar, double zone, int year, short month, double dayf );
    SSDate ( SSTime time, SSCalendar calendar = kGregorianJulian );
    SSDate ( const string &fmt, const string &str );
    SSDate ( void );
    
    static SSDate fromJulianDate ( SSTime time, SSCalendar cal );
    SSTime toJulianDate ( void );
    
    string format ( const string &fmt );
    bool parse ( const string &fmt, const string &str );
};

// Represents an instant in time as a Julian Date and a local time zone;
// zone is needed for date conversion, getting system time, and rise/set computation.

struct SSTime
{
    double        jd;              // Julian date in civil time (NOT epehemeris time!)
    double        zone;            // Local time zone, hours east of Greenwich

    static constexpr double kJ2000 = 2451545.0;      // JD of standard Julian epoch J2000
    static constexpr double kJ1970 = 2440587.5;      // JD of standard UNIX time base 1.0 January 1970 UTC
    static constexpr double kB1950 = 2433282.42346;  // JD of standard Besselian epoch B1950
    static constexpr double kB1900 = 2415020.31352;  // JD of standard Besselian epoch B1900

    static constexpr double kSecondsPerDay = 86400.0;   // Seconds per day
    static constexpr double kMinutesPerDay = 1440.0;    // Seconds per day
    static constexpr double kHoursPerDay = 24.0;        // Hours per day

    static constexpr double kDaysPerJulianYear = 365.25;               // Days per Julian year
    static constexpr double kDaysPerBesselianYear = 365.242198781;     // Days per Besselian year
    static constexpr double kSecondsPerJulianYear = 86400.0 * 365.25;  // Seconds per Julian year = 31557600.0

    static constexpr double kSiderealPerSolarDays = 1.00273790934;     // Sidereal days per Solar day
    static constexpr double kSolarPerSiderealDays = 0.99726957;        // Days per Sidereal Day

    // This operator allows using an SSTime as a double equal to its Juliam Date
    
    operator double () const { return jd; }
    
    // These operators preserve the time zone whem adding a time interval in days to an SSTime.
    
    SSTime operator + ( double k ) { return SSTime ( jd + k, zone ); }
    SSTime operator - ( double k ) { return SSTime ( jd - k, zone ); }
    SSTime operator * ( double k ) { return SSTime ( jd * k, zone ); }
    SSTime operator / ( double k ) { return SSTime ( jd / k, zone ); }

    void operator += ( double k ) { jd += k; }
    void operator -= ( double k ) { jd -= k; }
    void operator *= ( double k ) { jd *= k; }
    void operator /= ( double k ) { jd /= k; }

    SSTime ( void );
    SSTime ( double jd );
    SSTime ( double jd, double zone );
    SSTime ( SSDate date );
    
    static SSTime fromSystem ( void );
    static SSTime fromCalendarDate ( SSDate date ) { return SSTime ( date ); }
    static SSTime fromUnixTime ( time_t time );
    static SSTime fromJulianYear ( double year );
    static SSTime fromBesselianYear ( double year );
    
    SSDate  toCalendarDate ( SSCalendar cal ) { return SSDate ( *this, cal ); }
    time_t  toUnixTime ( void );
    double  toJulianYear ( void );
    double  toBesselianYear ( void );

    int     getWeekday ( void );
    double  getDeltaT ( void );
    double  getJulianEphemerisDate ( void );
    SSAngle getSiderealTime ( SSAngle lon );
    SSTime  getLocalMidnight ( void );
    
    static double CalendarToJD ( int y, short m, double d );
    static double GregorianToJD ( int y, short m, double d );
    static double JulianToJD ( int y, short m, double d );
    static double JewishToJD ( int y, short m, double d );
    static double IslamicToJD ( int y, short m, double d );
    static double IndianToJD ( int y, short m, double d );
    
    static void JDToCalendar ( double jd, int &y, short &m, double &d );
    static void JDToGregorian ( double jd, int &y, short &m, double &d );
    static void JDToJewish ( double jd, int &y, short &m, double &d );
    static void JDToJulian ( double jd, int &y, short &m, double &d );
    static void JDToIslamic ( double jd, int &y, short &m, double &d );
    static void JDToIndian ( double jd, int &y, short &m, double &d );
};

#endif /* SSTime_hpp */
