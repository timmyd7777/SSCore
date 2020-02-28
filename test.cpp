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
    double zone = 0;
    SSTime now = SSTime::fromSystem ( zone );
    SSTime::CalendarDate date = now.toCalendarDate ( kSSCalendarGregorian, zone );
    
    printf ( "Julian Date: %f\n", now.jd );
    printf ( "Time Zone: %.1f\n", zone );
    printf ( "Calendar Date: %04d-%02hd-%02.0f %02d:%02d:%04.1f\n", date.year, date.month, floor ( date.day ), date.hour, date.min, date.sec );
    
    SSAngle ra ( 1 );
    SSAngle dec = SSAngle::fromDegMinSec ( '+', 12, 34, 56.0 );
    
    SSAngle::HMS hms = ra.toHMS();
    SSAngle::DMS dms = dec.toDMS();
    
    printf ( "RA = %c%02hd %02hd %02.0f\n", hms.sign, hms.hour, hms.min, hms.sec );
    printf ( "Dec = %c%02hd %02hd %02.0f\n", dms.sign, dms.deg, dms.min, dms.sec );

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
