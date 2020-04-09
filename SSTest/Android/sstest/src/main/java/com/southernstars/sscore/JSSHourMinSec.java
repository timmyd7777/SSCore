package com.southernstars.sscore;

import com.southernstars.sscore.JSSAngle;

// Represents an angular value expressed in hours, minutes, seconds

public class JSSHourMinSec
{
    char sign;        // sign of angle, either '+' or '-'
    short hour;       // hours portion of angle, 0 - 23, always positive
    short min;        // minutes portion of angle, 0 - 59, always positive
    double sec;       // seconds portion of angle, 0 - 59.999..., always positive

    JSSHourMinSec()
    {
        sign = '+';
        hour = 0;
        min = 0;
        sec = 0.0;
    }

    JSSHourMinSec ( char p, short h, short m, double s )
    {
        sign = p;
        hour = h;
        min = m;
        sec = s;
    }

    public static JSSHourMinSec fromHours ( double hours )
    {
        char sign = hours >= 0.0 ? '+' : '-';
        hours = Math.abs ( hours );
        short hour = (short) hours;
        short min = (short) ( 60.0 * ( hours - hour ) );
        double sec = 3600.0 * ( hours - hour - min / 60.0 );

        return new JSSHourMinSec ( sign, hour, min, sec );
    }

    public static JSSHourMinSec fromRadians ( double radians )
    {
        return fromHours ( radians * JSSAngle.kHourPerRad );
    }

    public double toHours()
    {
        double h = hour + min / 60.0 + sec / 3600.0;
        return sign == '-' ? -h : h;
    }

    public double toRadians()
    {
        return toHours() * JSSAngle.kRadPerHour;
    }

    public static native JSSHourMinSec fromString ( String string );
    public native String toString();
}
