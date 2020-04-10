package com.southernstars.sscore;

import com.southernstars.sscore.JSSAngle;

// Represents an angular value expressed in degrees, minutes, seconds

public class JSSDegMinSec
{
    public char sign;        // sign of angle, either '+' or '-'
    public short deg;        // degrees portion of angle, 0 - 360, always positive
    public short min;        // minutes portion of angle, 0 - 59, always positive
    public double sec;       // seconds portion of angle, 0 - 59.999..., always positive

    public JSSDegMinSec()
    {
        sign = '+';
        deg = 0;
        min = 0;
        sec = 0.0;
    }

    public JSSDegMinSec ( char p, short d, short m, double s )
    {
        sign = p;
        deg = d;
        min = m;
        sec = s;
    }

    public static JSSDegMinSec fromDegrees ( double degrees )
    {
        char sign = degrees >= 0.0 ? '+' : '-';
        degrees = Math.abs ( degrees );
        short deg = (short) degrees;
        short min = (short) ( 60.0 * ( degrees - deg ) );
        double sec = 3600.0 * ( degrees - deg - min / 60.0 );

        return new JSSDegMinSec ( sign, deg, min, sec );
    }

    public static JSSDegMinSec fromRadians ( double radians )
    {
        return fromDegrees ( radians * JSSAngle.kDegPerRad );
    }

    public double toDegrees()
    {
        double d = deg + min / 60.0 + sec / 3600.0;
        return sign == '-' ? -d : d;
    }

    public double toRadians()
    {
        return toDegrees() * JSSAngle.kRadPerDeg;
    }

    public static native JSSDegMinSec fromString ( String string );
    public native String toString();
}
