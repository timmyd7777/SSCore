package com.southernstars.sscore;

import com.southernstars.sscore.JSSAngle;
import com.southernstars.sscore.JSSSpherical;

// Represents a point in a rectangular (x,y,z) coordinate system.

public class JSSVector
{
    public double x, y, z;    // Point's distance from origin along X, Y, Z axes, in arbitrary units.

    public JSSVector()
    {
        x = y = z = 0.0;
    }

    public JSSVector ( double x, double y, double z )
    {
        this.x = x;
        this.y = y;
        this.z = z;
    }

    public double magnitude()
    {
        return Math.sqrt ( x * x + y * y + z * z );
    }

    public JSSVector normalize()
    {
        double s = magnitude();
        if ( s > 0.0 )
            return new JSSVector ( x / s, y / s, z / s );
        else
            return new JSSVector();
    }

    public JSSVector add ( JSSVector other )
    {
        return new JSSVector ( x + other.x, y + other.y, z + other. z );
    }

    public JSSVector subtract ( JSSVector other )
    {
        return new JSSVector ( x - other.z, y - other.y, z - other.z );
    }

    public JSSVector multiplyBy ( double s )
    {
        return new JSSVector ( x * s, y * s, z * s );
    }

    public JSSVector divideBy ( double s )
    {
        return new JSSVector ( x / s, y / s, z / s );
    }

    public double dotProduct ( JSSVector other )
    {
        return x * other.x + y * other.y + z * other.z;
    }

    public JSSVector crossProduct ( JSSVector other )
    {
        double u = y * other.z - z * other.y;
        double v = z * other.x - x * other.z;
        double w = x * other.y - y * other.x;

        return new JSSVector ( u, v, w );
    }

    public double distance ( JSSVector other )
    {
        return subtract ( other ).magnitude();
    }

    public static native JSSVector fromSpherical ( JSSSpherical lbr );
    public native JSSSpherical toSpherical();
    public native JSSSpherical toSphericalVelocity ( JSSVector vvec );

    public native double angularSeparation ( JSSVector other );
    public native double positionAngle ( JSSVector other );
}
