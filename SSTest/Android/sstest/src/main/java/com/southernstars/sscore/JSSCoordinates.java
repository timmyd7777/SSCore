package com.southernstars.sscore;

import com.southernstars.sscore.JSSAngle;
import com.southernstars.sscore.JSSDouble;
import com.southernstars.sscore.JSSMatrix;
import com.southernstars.sscore.JSSVector;
import com.southernstars.sscore.JSSSpherical;

// This class converts rectangular and spherical coordinates between different astronomical reference frames
// at a particular time and geographic location. It also handles precession, nutation, aberration, refraction,
// and other coordinate-related issues.

public class JSSCoordinates
{
    private long pCoords;   // pointer to native C++ SSObject

    public static final int kFundamental = 0;   // ICRS: the mean equatorial reference frame at epoch J2000 (excludes nutation); X/Y plane is Earth's equatorial plane, +X toward vernal equinox, +Z toward north pole; spherical coords are RA/Dec
    public static final int kEquatorial = 1;    // equatorial frame at a specific epoch (including nutation); X/Y plane is Earth's equatorial plane, +X toward vernal equinox, +Z toward north pole; spherical coords are RA/Dec
    public static final int kEcliptic = 2;      // ecliptic frame at a specific epoch (includes nutation); X/Y plane is Earth's orbital plane; +X toward vernal equinox, +Z toward north ecliptic pole; spherical coords are ccliptic lon/lat
    public static final int kGalactic = 3;      // galactic frame; fixed relative to ICRS; X/Y plane is galactic equator; +X toward galactic center, +Z toward north galactic pole; spherical coords are galactic lon/lat
    public static final int kHorizon = 4;       // local horizon frame; X/Y plane is local horizon, +X is north, +Z is zenith; ; spherical coords are azimuth/altitude

    public static final double kKmPerAU = 149597870.700;                               // kilometers per Astronomical Unit (IAU 2012)
    public static final double kKmPerEarthRadii = 6378.137;                            // kilometers per equatorial Earth radius (WGS84)
    public static final double kEarthFlattening = 1 / 298.257;                         // Earth ellipsoid flattening factor (WGS84)
    public static final double kLightKmPerSec = 299792.458;                            // Speed of light in kilometers per second
    public static final double kLightAUPerDay = kLightKmPerSec * 86400.0 / kKmPerAU;   // Speed of lignt in astronomical units per day = 173.144
    public static final double kAUPerParsec = JSSAngle.kArcsecPerRad;                  // Astronomical units per parsec = 206264.806247
    public static final double kParsecPerAU = 1.0 / kAUPerParsec;                      // Parsecs per astronomical unit
    public static final double kAUPerLY = kLightAUPerDay * 365.25;                     // Astronomical units per light year = 63241.077084 (assuming Julian year of exactly 365.25 days)
    public static final double kLYPerAU = 1.0 / kAUPerLY;                              // Light years per astronomical unit
    public static final double kLYPerParsec = kAUPerParsec / kAUPerLY;                 // Light years per parsec = 3.261563777179643
    public static final double kParsecPerLY = kAUPerLY / kAUPerParsec;                 // Parsecs per light year

    private JSSCoordinates()
    {
        pCoords = 0;
    }

    public void finalize()
    {
        destroy();
    }

    public JSSCoordinates ( JSSTime time, JSSSpherical loc )
    {
        create ( time, loc );
    }

    public native void create ( JSSTime time, JSSSpherical loc );
    public native void destroy();

    public native void setTime ( JSSTime time );
    public native void setLocation ( JSSSpherical location );

    public native JSSVector getObserverPosition();
    public native JSSVector getObserverVelocity();

    public native boolean getStarParallax();
    public native boolean getStarMotion();
    public native boolean getAberration();
    public native boolean getLightTime();

    public native void setStarParallax ( boolean parallax );
    public native void setStarMotion ( boolean motion );
    public native void setAberration ( boolean aberration );
    public native void setLightTime ( boolean lighttime );

    public native JSSTime getTime();
    public native JSSSpherical getLocation();
    public native double getJED();
    public native double getLST();

    public static native double getObliquity ( double epoch );
    public static native void getNutationConstants ( double jd, JSSDouble de, JSSDouble dl );
    public static native void getPrecessionConstants ( double jd, JSSDouble zeta, JSSDouble z, JSSDouble theta );

    public static native JSSMatrix getPrecessionMatrix ( double epoch );
    public static native JSSMatrix getNutationMatrix ( double onliquity, double nutLon, double nutObq );
    public static native JSSMatrix getEclipticMatrix ( double obliquity );
    public static native JSSMatrix getHorizonMatrix ( double lst, double lat );
    public static native JSSMatrix getGalacticMatrix();

    public native JSSVector    transform ( int from, int to, JSSVector vec );
    public native JSSSpherical transform ( int from, int to, JSSSpherical sph );

    public native JSSVector applyAberration ( JSSVector direction );
    public native JSSVector removeAberration ( JSSVector direction );

    public static native double redShiftToRadVel ( double z );
    public static native double radVelToRedShift ( double rv );

    public static native JSSVector toGeocentric ( JSSSpherical geodetic, double re, double f );
    public static native JSSSpherical toGeodetic ( JSSVector geocentric, double re, double f );

    public static native double refractionAngle ( double alt, boolean a );
    public static native double applyRefraction ( double alt );
    public static native double removeRefraction ( double alt );
}
