//  test.cpp
//  SSCore
//
//  Created by Tim DeBenedictis on 2/24/20.
//  Copyright © 2020 Southern Stars. All rights reserved.

#include <stdio.h>
#include "SSTime.hpp"
#include "SSMatrix.hpp"

int main ( int argc, char *argv[] )
{
    SSTime now;
    
    int year = 0;
    short month = 0, hour = 0, min = 0;
    double jd = 0, day = 0, sec = 0, zone = 0;
    
    jd = now.getJulianDate();
    zone = now.getTimeZone();
    now.getCalendarDate ( year, month, day, hour, min, sec );
    
    printf ( "Julian Date: %f\n", jd );
    printf ( "Time Zone: %.1f\n", zone );
    printf ( "Calendar Date: %04d-%02hd-%02.0f %02d:%02d:%04.1f\n", year, month, floor ( day ), hour, min, sec );
    
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
