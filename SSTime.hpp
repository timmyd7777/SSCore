//
//  SSTime.hpp
//  SSCore
//
//  Created by Tim DeBenedictis on 2/23/20.
//  Copyright © 2020 Southern Stars. All rights reserved.
//

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
	double		mJulianDate;		// Julian date in civil time (NOT epehmeris time!)
	double		mTimeZone;			// Local time offset from UTC in hours; east positive, west negative
	SSCalendar	mCalendar;			// Calendar system to use for date/time conversion
	string		mDateFormat;		// strftime()-style arguments for formatting date strings
	string		mTimeFormat;		// strftime()-style arguments for formatting time strings
	
public:
	
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

	SSTime ( void );
	SSTime ( double jd );
	SSTime ( int year, short month, double day, short hour, short min, double sec );
	SSTime ( time_t t );
	
	double		getJulianDate ( void ) { return mJulianDate; }
	void		setJulianDate ( double jd ) { mJulianDate = jd; }
	
	double		getJulianYear ( void ) { return ( mJulianDate - kJ2000 ) / kDaysPerJulianYear + 2000.0; }
	void		setJulianYear ( double year ) { mJulianDate = kJ2000 + kDaysPerJulianYear * ( year - 2000.0 ); }
	
	double		getBesselianYear ( void ) { return ( mJulianDate - kB1900 ) / kDaysPerJulianYear + 1900.0; }
	void		setBesselianYear ( double year ) { mJulianDate = kB1900 + kDaysPerBesselianYear * ( year - 1900.0 ); }

	SSCalendar	getCalendarSystem ( void ) { return mCalendar; }
	void		setCalendarSystem ( SSCalendar cal ) { mCalendar = cal; }
	
	double		getTimeZone ( void ) { return mTimeZone; }
	void		setTimeZome ( double tz ) { mTimeZone = tz; }
	
	int			getDayOfWeek ( void );
	
	void		setFromSystem ( void );
	
	void		getCalendarDate ( int &year, short &month, double &day, short &hour, short &min, double &dec );
	void		setCalendarDate ( int year, short month, double day, short hour, short min, double sec );
	
	time_t		getUnixTime ( void ) { return ( mJulianDate - kJ1970 ) * kSecPerDay; }
	void		setUnixTime ( time_t t ) { mJulianDate = t / kSecPerDay + kJ1970; }
	
	double		getDeltaT ( void );
	double		getJulianEphemerisDate ( void );
	
	double		getGreenwichMeanSiderealTime ( void );

	void		setDateFormat ( string fmt ) { mDateFormat = fmt; }
	string		getDateFormat ( void ) { return mDateFormat; }
	
	void		setTimeFormat ( string fmt ) { mTimeFormat = fmt; }
	string		getTimeFormat ( void ) { return mTimeFormat; }
	
	string		formatDate ( void );
	string		formatTime ( void );
};

#endif /* SSTime_hpp */
