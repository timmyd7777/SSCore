package com.southernstars.sscore;

import com.southernstars.sscore.JSSAngle;

// Represents an angular value expressed in hours, minutes, seconds
// Always positive and in the range 0 to 24h

public class JSSHourMinSec
{
    public short hour;       // hours portion of angle, 0 - 23, always positive
    public short min;        // minutes portion of angle, 0 - 59, always positive
    public double sec;       // seconds portion of angle, 0 - 59.999..., always positive

    public JSSHourMinSec()
    {
        hour = 0;
        min = 0;
        sec = 0.0;
    }

    public JSSHourMinSec ( short h, short m, double s )
    {
        hour = h;
        min = m;
        sec = s;
    }

    public static JSSHourMinSec fromHours ( double hours )
    {
        hours = hours - 24.0 * Math.floor ( hours / 24.0 );
        short hour = (short) hours;
        short min = (short) ( 60.0 * ( hours - hour ) );
        double sec = 3600.0 * ( hours - hour - min / 60.0 );

        return new JSSHourMinSec ( hour, min, sec );
    }

    public static JSSHourMinSec fromRadians ( double radians )
    {
        return fromHours ( radians * JSSAngle.kHourPerRad );
    }

    public double toHours()
    {
        return hour + min / 60.0 + sec / 3600.0;
    }

    public double toRadians()
    {
        return toHours() * JSSAngle.kRadPerHour;
    }

    public static native JSSHourMinSec fromString ( String string );
    public native String toString();
}
