//  SSTime.hpp
//  SSCore
//
//  Created by Tim DeBenedictis on 2/23/20.
//  Copyright © 2020 Southern Stars. All rights reserved.

#ifndef SSTime_hpp
#define SSTime_hpp

#include <string>
#include <stdio.h>
#include <math.h>
#include <time.h>

using namespace std;

enum SSCalendar
{
	kSSCalendarGregorian = 0,
	kSSCalendarJulian = 1
};

class SSTime
{
    public:
    
	double		jd;		        // Julian date in civil time (NOT epehemeris time!)
	double		zone;			// Local time zone offset from UTC [Hours east of Greenwich]
	
    static constexpr double		kJ2000 = 2451545.0;		// JD of standard Julian epoch J2000
	static constexpr double		kJ1970 = 2440587.5;		// JD of standard UNIX time base 1.0 January 1970 UTC
	static constexpr double		kB1950 = 2433282.423;	// JD of standard Besselian epoch B1950
	static constexpr double		kB1900 = 2433282.423;	// JD of standard Besselian epoch B1900

	static constexpr double		kSecondsPerDay = 86400.0;	// Seconds per day
	static constexpr double		kMinutesPerDay = 1440.0;	// Seconds per day
	static constexpr double		kHoursPerDay = 24.0;		// Hours per day

	static constexpr double		kDaysPerJulianYear = 365.25;			// Days per Julian year
	static constexpr double		kDaysPerBesselianYear = 365.242198781;	// Days per Besselian year

	static constexpr double		kSiderealPerSolarDays = 1.00273790934;	// Sidereal days per Solar day
	static constexpr double		kSolarPerSiderealDays = 0.99726957;		// Days per Sidereal Day

    struct CalendarDate
    {
        SSCalendar calendar;
        double zone;
        int year;
        short month;
        double day;
        short hour;
        short min;
        double sec;
    };
    
	SSTime ( void );
	SSTime ( double jd, double zone );
	
    static SSTime   fromSystem ( void );
    static SSTime   fromUnixTime ( time_t time );
    static SSTime   fromJulianYear ( double year );
    static SSTime   fromBesselianYear ( double year );
    static SSTime   fromCalendarDate ( SSCalendar cal, double zone, int year, short month, double day, short hour, short min, double sec );

    void            toCalendarDate ( SSCalendar cal, double zone, int &year, short &month, double &day, short &hour, short &min, double &sec );
    CalendarDate    toCalendarDate ( SSCalendar cal, double zone );
    CalendarDate    toCalendarDate ( SSCalendar cal );

    time_t          toUnixTime ( void );
    double          toJulianYear ( void );
    double          toBesselianYear ( void );

    int             getWeekday ( void );
	double		    getDeltaT ( void );		// in seconds
	double		    getJulianEphemerisDate ( void );
	double		    getGreenwichMeanSiderealTime ( void );		// in radians
};

#endif /* SSTime_hpp */
