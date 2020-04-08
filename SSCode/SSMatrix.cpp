//  SSMatix.cpp
//  SSCore
//
//  Created by Tim DeBenedictis on 2/23/20.
//  Copyright © 2020 Southern Stars. All rights reserved.

#include <math.h>
#include <stdio.h>
#include <stdarg.h>
#include "SSMatrix.hpp"

// Constructs a 3x3 matrix whose elements are all initialized to zero.

SSMatrix::SSMatrix ( void )
{
    m00 = 0.0;  m01 = 0.0; m02 = 0.0;
    m10 = 0.0;  m11 = 0.0; m12 = 0.0;
    m20 = 0.0;  m21 = 0.0; m22 = 0.0;
}

// Constructs a 3x3 matrix whose elements are specified individually by row.

SSMatrix::SSMatrix ( double m00, double m01, double m02, double m10, double m11, double m12, double m20, double m21, double m22 )
{
    this->m00 = m00; this->m01 = m01; this->m02 = m02;
    this->m10 = m10; this->m11 = m11; this->m12 = m12;
    this->m20 = m20; this->m21 = m21; this->m22 = m22;
}

// Returns a 3x3 identity matrix.

SSMatrix SSMatrix::identity ( void )
{
    return SSMatrix ( 1.0, 0.0, 0.0,
                      0.0, 1.0, 0.0,
                      0.0, 0.0, 1.0 );
}

// Returns a 3x3 matrix which is the transpose of this matrix.
// Does not transpose this matrix in place!
// For a rotation matrix, its transpose is also its inverse.

SSMatrix SSMatrix::transpose ( void )
{
    return SSMatrix ( m00, m10, m20,
                      m01, m11, m21,
                      m02, m12, m22 );
}

// Returns a 3x3 matrix which is the inverse of this matrix.
// Does not invert this matrix in place!
// For a rotation matrix, its transpose is also its inverse.
// If this matrix is singular, returns infinity matrix.
// CAUTION: for near-singular matrices, may not be accurate. From code here:
// https://stackoverflow.com/questions/983999/simple-3x3-matrix-inverse-code-c

SSMatrix SSMatrix::inverse ( void )
{
    double det = determinant();
    
    double w00 = ( m11 * m22 - m21 * m12 ) / det;
    double w01 = ( m02 * m21 - m01 * m22 ) / det;
    double w02 = ( m01 * m12 - m02 * m11 ) / det;
    double w10 = ( m12 * m20 - m10 * m22 ) / det;
    double w11 = ( m00 * m22 - m02 * m20 ) / det;
    double w12 = ( m10 * m02 - m00 * m12 ) / det;
    double w20 = ( m10 * m21 - m20 * m11 ) / det;
    double w21 = ( m20 * m01 - m00 * m21 ) / det;
    double w22 = ( m00 * m11 - m10 * m01 ) / det;
    
    return SSMatrix ( w00, w01, w02,
                      w10, w11, w12,
                      w20, w21, w22 );
}

// Returns the determinant of this 3x3 matrix.
// For a rotation matrix, the determinant is 1.0.

double SSMatrix::determinant ( void )
{
    double det = m00 * ( m11 * m22 - m12 * m21 )
               - m01 * ( m10 * m22 - m12 * m20 )
               + m02 * ( m10 * m21 - m11 * m20 );
    
    return ( det );
}

// Returns the product of this matrix and a 3-element vector
// as another vector.  Does not modify the input vector (vec)!

SSVector SSMatrix::multiply ( SSVector vec )
{
    double x = m00 * vec.x + m01 * vec.y + m02 * vec.z;
    double y = m10 * vec.x + m11 * vec.y + m12 * vec.z;
    double z = m20 * vec.x + m21 * vec.y + m22 * vec.z;
    
    return SSVector ( x, y, z );
}

// Returns the product of this matrix and another 3x3 matrix
// as another matrix.  Does not modify the input matrix (mat)!
// Note: matrix multiplication is NOT commutative; in other words,
// this.multiply(that) and that.multiply(this) return different matrices!

SSMatrix SSMatrix::multiply ( SSMatrix mat )
{
    double x0 = m00 * mat.m00 + m01 * mat.m10 + m02 * mat.m20;
    double x1 = m10 * mat.m00 + m11 * mat.m10 + m12 * mat.m20;
    double x2 = m20 * mat.m00 + m21 * mat.m10 + m22 * mat.m20;

    double y0 = m00 * mat.m01 + m01 * mat.m11 + m02 * mat.m21;
    double y1 = m10 * mat.m01 + m11 * mat.m11 + m12 * mat.m21;
    double y2 = m20 * mat.m01 + m21 * mat.m11 + m22 * mat.m21;

    double z0 = m00 * mat.m02 + m01 * mat.m12 + m02 * mat.m22;
    double z1 = m10 * mat.m02 + m11 * mat.m12 + m12 * mat.m22;
    double z2 = m20 * mat.m02 + m21 * mat.m12 + m22 * mat.m22;
    
    return SSMatrix ( x0, y0, z0,
                      x1, y1, z1,
                      x2, y2, z2 );
}

// Returns a matrix which represents this matrix rotated around
// a particular coordinate axis (0=X,1=Y,2=Z) by an angle in radians.
// Does not modify this matrix; returns a transformed copy!

SSMatrix SSMatrix::rotate ( int axis, double angle )
{
    double cosa = cos ( angle );
    double sina = sin ( angle );
    SSMatrix r;
    
    if ( axis == 0 )
    {
        return SSMatrix ( 1.0, 0.0,  0.0,
                          0.0, cosa, -sina,
                          0.0, sina, cosa ).multiply ( *this );
    }
    else if ( axis == 1 )
    {
        return SSMatrix ( cosa, 0.0, -sina,
                           0.0, 1.0, 0.0,
                          sina, 0.0, cosa ).multiply ( *this );
    }
    else if ( axis == 2 )
    {
        return SSMatrix ( cosa, -sina, 0.0,
                          sina,  cosa, 0.0,
                           0.0,   0.0, 1.0 ).multiply ( *this );
    }
    else
    {
        return *this;
    }
}

// Returns a matrix which represents an arbitrary set of rotations around
// the principal coordinate axes (X,Y,Z).  The number of rotations is n.
// For each rotation, a pair of arguments gives the axis and angle as per above,
// so the total numbe of arguments is 1 + 2 * n.  The order of rotations is important!
// For example, the matrix returned by rotation ( 3, 0, a, 1, b, 2, c ) is the inverse
// (i.e. transpose) of the matrix returned by rotation ( 3, 2, -c, 1, -b, 0, a )

SSMatrix SSMatrix::rotation ( int n, ... )
{
    SSMatrix m = SSMatrix::identity();
    va_list ap;

    va_start ( ap, n );

    for ( int k = 0; k < n; k++ )
    {
        int axis = va_arg ( ap, int );
        double angle = va_arg ( ap, double );
        m = m.rotate ( axis, angle );
    }
    
    va_end ( ap );
    return ( m );
}
