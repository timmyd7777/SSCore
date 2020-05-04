package com.southernstars.sscore;

import com.southernstars.sscore.JSSDate;

public class JSSTime
{
    public double  jd;              // Julian date in civil time (NOT epehemeris time!)
    public double  zone;            // Local time zone to use for converting to local calendar date/time, hours east of Greenwich

    public static double kJ2000 = 2451545.0;      // JD of standard Julian epoch J2000
    public static double kJ1970 = 2440587.5;      // JD of standard UNIX time base 1.0 January 1970 UTC
    public static double kB1950 = 2433282.42346;  // JD of standard Besselian epoch B1950
    public static double kB1900 = 2415020.31352;  // JD of standard Besselian epoch B1900

    public static double kSecondsPerDay = 86400.0;          // Seconds per day
    public static double kMinutesPerDay = 1440.0;           // Seconds per day
    public static double kHoursPerDay = 24.0;               // Hours per day

    public static double kDaysPerJulianYear = 365.25;               // Days per Julian year
    public static double kDaysPerBesselianYear = 365.242198781;     // Days per Besselian year

    public static double kSiderealPerSolarDays = 1.00273790934;     // Sidereal days per Solar day
    public static double kSolarPerSiderealDays = 0.99726957;        // Days per Sidereal Day

    public JSSTime()
    {
        jd = 0.0;
        zone = 0.0;
    }

    public JSSTime ( double j )
    {
        jd = jd;
        zone = 0.0;
    }

    public JSSTime ( double j, double z )
    {
        jd = j;
        zone = z;
    }

    public native static JSSTime fromCalendarDate ( JSSDate date );
    public native static JSSTime fromSystem();

    public static JSSTime fromUnixTime ( long time )
    {
        return new JSSTime ( time / kSecondsPerDay + kJ1970 );
    }

    public static JSSTime fromJulianYear ( double year )
    {
        return new JSSTime ( kJ2000 + kDaysPerJulianYear * ( year - 2000.0 ) );
    }

    public static JSSTime fromBesselianYear ( double year )
    {
        return new JSSTime ( kB1900 + kDaysPerBesselianYear * ( year - 1900.0 ) );
    }

    public native JSSDate toCalendarDate ( int calendar );

    public long toUnixTime()
    {
        return (long) ( kSecondsPerDay * ( jd - kJ1970 ) );
    }

    public double toJulianYear()
    {
        return ( jd - kJ2000 ) / kDaysPerJulianYear + 2000.0;
    }

    public double toBesselianYear()
    {
        return ( jd - kB1900 ) / kDaysPerJulianYear + 1900.0;
    }

    public native int getWeekday();
    public native double getDeltaT();
    public native double getJulianEphemerisDate();
    public native double getSiderealTime ( double lon );
}
