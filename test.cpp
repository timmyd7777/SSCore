#include <stdio.h>
#include "SSVector.hpp"

int main ( int argc, char *argv[] )
{
    SSVector v1 ( 1.0, 2.0, 3.0 );
    SSVector v2 ( 4.0, 5.0, 6.0 );
    SSVector v3;
    
    v1.add ( v2 );
    
    
    v3 = v1.add ( v2 );
    
    printf ( "%lf %lf %lf\n", v1.x, v1.y, v1.z );
    printf ( "%lf %lf %lf\n", v2.x, v2.y, v2.z );
    printf ( "%lf %lf %lf\n", v3.x, v3.y, v3.z );
    
    v1 = v1.subtract ( v2 );
    printf ( "%lf %lf %lf\n", v1.x, v1.y, v1.z );
    
    SSMatrix i = SSMatrix::identity();
    
    printf ( "%lf %lf %lf\n", i.m00, i.m01, i.m02 );
    printf ( "%lf %lf %lf\n", i.m10, i.m11, i.m12 );
    printf ( "%lf %lf %lf\n", i.m20, i.m21, i.m22 );
}