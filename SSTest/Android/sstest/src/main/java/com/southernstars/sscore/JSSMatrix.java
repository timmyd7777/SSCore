package com.southernstars.sscore;

import com.southernstars.sscore.JSSVector;

// Represents a 3x3 matrix, with routines for performing simple matrix and vector-matrix arithmetic.

public class JSSMatrix
{
    public double m00, m01, m02;
    public double m10, m11, m12;
    public double m20, m21, m22;

    public JSSMatrix()
    {
        m00 = m01 = m02 = 0.0;
        m10 = m11 = m12 = 0.0;
        m20 = m21 = m22 = 0.0;
    }

    public JSSMatrix ( double m00, double m01, double m02, double m10, double m11, double m12, double m20, double m21, double m22 )
    {
        this.m00 = m00; this.m01 = m01; this.m02 = m02;
        this.m10 = m10; this.m11 = m11; this.m12 = m12;
        this.m20 = m20; this.m21 = m21; this.m22 = m22;
    }

    public JSSMatrix transpose()
    {
        return new JSSMatrix ( m00, m10, m20,
                               m01, m11, m21,
                               m02, m21, m22 );
    }

    public native JSSMatrix inverse();
    public native double determinant();

    public static JSSMatrix identity()
    {
        return new JSSMatrix ( 1.0, 0.0, 0.0,
                               0.0, 1.0, 0.0,
                               0.0, 0.0, 1.0 );
    }

    public native JSSVector multiply ( JSSVector vec );
    public native JSSMatrix multiply ( JSSMatrix mat );
    public native JSSMatrix rotate ( int axis, double angle );
}
