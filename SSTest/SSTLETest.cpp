//  SSTLETest.cpp
//
//  Created by Tim DeBenedictis on 3/9/24.
//  Copyright © 2024 Southern Stars. All rights reserved.

#include <iostream>
#include "SSCoordinates.hpp"
#include "SSTLE.hpp"

int main ( int argc, const char *argv[] )
{
    // Get path to input TLE file from user, if not presetn in first command-line argument.
    char tlepath[256] = { 0 };
    if ( argc < 2 )
    {
        printf ( "Path to input TLE file: " );
        scanf ( "%s", tlepath );
    }
    else
    {
        strlcpy ( tlepath, argv[1], sizeof ( tlepath ) );
    }
    
    // Open TLE file; exit on failure.
    FILE *tlefile = fopen ( tlepath, "r" );
    if ( tlefile == NULL )
    {
        printf ( "Can't open TLE file %s!\n", tlepath );
        return -1;
    }
    
    // Read entries from TLE file; for each entry compute position and velocity for 1 day.
    SSTLE tle;
    while ( tle.read ( tlefile ) == 0 )
    {
        // Write TLE to standard output for verification
        if ( tle.write ( cout ) )
            cout << endl << endl;
        
        // Compute amd display satellite position and velocity
        // at 1-minute intervals for 1 day from element epoch.
        printf ( "%-6s %-9s %-9s %-9s %-9s %-9s %-9s %-9s %-9s\n", "t", "x", "y", "z", "vx", "vy", "vz", "r", "v" );
        for ( double tsince = 0.0; tsince <= 1440.0; tsince += 1.0 )
        {
            SSVector pos, vel;
            
            if ( tle.isdeep() )
                tle.sdp4 ( tsince, pos, vel );
            else
                tle.sgp4 ( tsince, pos, vel );
            
            pos *= SSCoordinates::kKmPerEarthRadii;
            vel *= SSCoordinates::kKmPerEarthRadii / 60.0;
            
            printf ( "%6.1f %+9.3f %+9.3f %+9.3f %+9.6f %+9.6f %+9.6f %9.3f %9.6f\n",
                    tsince, pos.x, pos.y, pos.z, vel.x, vel.y, vel.z, pos.magnitude(), vel.magnitude() );
        }
    }
    
    fclose ( tlefile );
    return 0;
}
