package com.southernstars.sscore;

public class JSSTime
{
    public double jd;
    public double zone;
    public int calendar;

    JSSTime()
    {
        jd = 0.0;
        zone = 0.0;
        calendar = 0;
    }

    JSSTime ( double j, double z, int c )
    {
        jd = j;
        zone = z;
        calendar = c;
    }

    public native static JSSTime fromSystem();
 /*
    {
        return new JSSTime ( 2451545.0, 0.0, 0 );
    }
*/
    public native double getJulianEphemerisDate();
/*    {
        return jd + 0.01;
    }
*/
}
