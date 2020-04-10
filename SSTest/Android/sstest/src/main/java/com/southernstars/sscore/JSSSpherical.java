package com.southernstars.sscore;

import com.southernstars.sscore.JSSAngle;
import com.southernstars.sscore.JSSVector;

// Represents a point in a spherical coordinate system (lon/lat, RA/Dec, Az/Alt)

public class JSSSpherical
{
    public double lon;    // Longitude coordinate, in radians from 0 to kTwoPi.
    public double lat;    // Latitude coordinate, in radians from -kHalfPi to +kHalfPi.
    public double rad;    // Radial distance from origin of coordinate system, in arbitrary units; 1.0 is a unit vector.

    public JSSSpherical()
    {
        lon = lat = rad = 0.0;
    }

    public JSSSpherical ( double l, double b )
    {
        lon = l;
        lat = b;
        rad = 1.0;
    }

    public JSSSpherical ( double l, double b, double r )
    {
        lon = l;
        lat = b;
        rad = r;
    }

    public static native JSSSpherical fromVector ( JSSVector vec );
    public native JSSVector toVector();
    public native JSSVector toVectorVelocity ( JSSSpherical vsph );

    public native double angularSeparation ( JSSSpherical other );
    public native double positionAngle ( JSSSpherical other );
}
