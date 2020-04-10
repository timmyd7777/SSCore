package com.southernstars.sscore;

public class JSSDate
{
    public int calendar;          // calendar system: kGregorian or kJulian.
    public double zone;           // local time zone offset from UTC in hours east of Greenwich; wwst is negative!
    public int year;              // calendar year; note 0 = 1 BC, -1 = 2 BC, etc.
    public short month;           // month; 1 = Jan, 2 = Feb, etc.
    public double day;            // day including fractional part; 1.0 to 31.999...
    public short hour;            // hour of day; 0 to 23
    public short min;             // minute of hour; 0 to 59
    public double sec;            // seconds of minute including fractional part; 0 to 59.999...

    public static int kGregorian = 0;        // Gregorian calendar, used after 15 October 1582 (JD 2299161).  Leap years every 4th year, but not every 100th, unless also 400th.
    public static int kJulian = 1;           // Julian calendar, used before 4 October 1582 (JD 2299161).  Leap years every 4 years.

    public JSSDate()
    {
        calendar = kGregorian;
        zone = 0.0;
        year = 0;
        month = 0;
        day = 0;
        hour = 0;
        min = 0;
        sec = 0.0;
    }

    public JSSDate ( int c, double z, int y, short m, double d, short h, short n, double s )
    {
        calendar = c;
        zone = z;
        year = y;
        month = m;
        day = d;
        hour = h;
        min = n;
        sec = s;
    }

    public static native JSSDate fromJulianDate ( JSSTime time );
    public native JSSTime toJulianDate();
}
