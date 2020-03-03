//  SSMatix.cpp
//  SSCore
//
//  Created by Tim DeBenedictis on 2/23/20.
//  Copyright © 2020 Southern Stars. All rights reserved.

#include <math.h>
#include <stdio.h>
#include <stdarg.h>
#include "SSMatrix.hpp"

SSMatrix::SSMatrix ( void )
{
    m00 = 0.0;  m01 = 0.0; m02 = 0.0;
    m10 = 0.0;  m11 = 0.0; m12 = 0.0;
    m20 = 0.0;  m21 = 0.0; m22 = 0.0;
}

SSMatrix::SSMatrix ( double m00, double m01, double m02, double m10, double m11, double m12, double m20, double m21, double m22 )
{
    this->m00 = m00; this->m01 = m01; this->m02 = m02;
    this->m10 = m10; this->m11 = m11; this->m12 = m12;
    this->m20 = m20; this->m21 = m21; this->m22 = m22;
}

SSMatrix SSMatrix::identity ( void )
{
    return SSMatrix ( 1.0, 0.0, 0.0,
                      0.0, 1.0, 0.0,
                      0.0, 0.0, 1.0 );
}

SSMatrix SSMatrix::transpose ( void )
{
    return SSMatrix ( m00, m10, m20,
                      m01, m11, m21,
                      m02, m12, m22 );
}

double SSMatrix::determinant ( void )
{
    double det = m00 * ( m11 * m22 - m12 * m21 )
               - m01 * ( m10 * m22 - m12 * m20 )
               + m02 * ( m10 * m21 - m11 * m20 );
    
    return ( det );
}

SSVector SSMatrix::multiply ( SSVector vec )
{
    double x = m00 * vec.x + m01 * vec.y + m02 * vec.z;
    double y = m10 * vec.x + m11 * vec.y + m12 * vec.z;
    double z = m20 * vec.x + m21 * vec.y + m22 * vec.z;
    
    return SSVector ( x, y, z );
}

SSMatrix SSMatrix::multiply ( SSMatrix mat )
{
    double x0 = m00 * mat.m00 + m01 * mat.m01 + m02 * mat.m02;
    double y0 = m10 * mat.m00 + m11 * mat.m01 + m12 * mat.m02;
    double z0 = m20 * mat.m00 + m21 * mat.m01 + m22 * mat.m02;

    double x1 = m00 * mat.m10 + m01 * mat.m11 + m02 * mat.m12;
    double y1 = m10 * mat.m10 + m11 * mat.m11 + m12 * mat.m12;
    double z1 = m20 * mat.m10 + m21 * mat.m11 + m22 * mat.m12;

    double x2 = m00 * mat.m20 + m01 * mat.m21 + m02 * mat.m22;
    double y2 = m10 * mat.m20 + m11 * mat.m21 + m12 * mat.m22;
    double z2 = m20 * mat.m20 + m21 * mat.m21 + m22 * mat.m22;
    
    return SSMatrix ( x0, y0, z0,
                      x1, y1, z1,
                      x2, y2, z2 );
}

#if 1

SSMatrix SSMatrix::rotate ( int axis, double angle )
{
    double cosa = cos ( angle );
    double sina = sin ( angle );

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
                   
#else

SSMatrix SSMatrix::rotate ( int axis, double angle )
{
    double cosa = cos ( angle );
    double sina = sin ( angle );

    if ( axis == 0 )
    {
        return multiply ( SSMatrix ( 1.0, 0.0,  0.0,
                        0.0, cosa, -sina,
                        0.0, sina, cosa ) );
    }
    else if ( axis == 1 )
    {
        return multiply ( SSMatrix ( cosa, 0.0, -sina,
                           0.0, 1.0, 0.0,
                          sina, 0.0, cosa ) );
    }
    else if ( axis == 2 )
    {
        return multiply ( SSMatrix ( cosa, -sina, 0.0,
                          sina,  cosa, 0.0,
                           0.0,   0.0, 1.0 ) );
    }
    else
    {
        return *this;
    }
}

#endif
                         
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
    
    return ( m );
}
