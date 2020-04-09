package com.southernstars.sscore;

// Represents an angular value expressed in radians

public class JSSAngle
{
    public double rad;   // angular value in radians; infinite if unknown/undefined.

    public static double kPi  = Math.PI;                         // 3.141592653589793
    public static double kTwoPi = 2.0 * kPi;                     // 6.283185307179586
    public static double kHalfPi = kPi / 2.0;                    // 1.570796326794897

    public static double kDegPerRad = 180.0 / kPi;               // 57.295779513082321
    public static double kRadPerDeg = 1.0 / kDegPerRad;          //  0.017453292519943

    public static double kHourPerRad = kDegPerRad / 15.0;        // 3.819718634205488
    public static double kRadPerHour = 1.0 / kHourPerRad;        // 0.261799387799149

    public static double kArcminPerRad = 60.0 * kDegPerRad;      // 3437.746770784939253
    public static double kRadPerArcmin = 1.0 / kArcminPerRad;    //    0.000290888208666

    public static double kArcsecPerRad = 3600 * kDegPerRad;      // 206264.806247096355156
    public static double kRadPerArcsec = 1.0 / kArcsecPerRad;    //      0.000004848136811

    public JSSAngle()
    {
        rad = 0.0;
    }

    public JSSAngle ( double r )
    {
        rad = r;
    }

    public double toArcsec() { return rad * kArcsecPerRad; }
    public double toArcmin() { return rad * kArcminPerRad; }
    public double toDegrees() { return rad * kDegPerRad; }
    public double toHours() { return rad * kHourPerRad; }

    public static JSSAngle fromArcsec ( double arcsec ) { return new JSSAngle ( arcsec * kRadPerArcsec ); }
    public static JSSAngle fromArcmin ( double arcmin ) { return new JSSAngle ( arcmin * kRadPerArcmin ); }
    public static JSSAngle fromDegrees ( double degrees ) { return new JSSAngle ( degrees * kRadPerArcmin ); }
    public static JSSAngle fromHours ( double hours ) { return new JSSAngle ( hours * kRadPerHour ); }
}
