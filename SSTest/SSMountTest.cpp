// SSMountTest.cpp
// SSCore
//
// Created by Tim DeBenedictis on 9/15/22.
// Copyright © 2022 Southern Stars. All rights reserved.
//
// Command-line test program for telescope mount communication classes.

#include <cstdio>
#include <iostream>
#include <unistd.h>

#include "SSMount.hpp"

int main ( int argc, const char * argv[] )
{
    vector<string> serialPortNames, serialPortPaths;
    
    int numPorts = SSSerial::listPorts ( serialPortNames, serialPortPaths );
    cout << "Found " << numPorts << " serial ports:" << endl;
    
    for ( int i = 0; i < numPorts; i++ )
        cout << "Mame: " << serialPortNames[i] << "  Path: " << serialPortPaths[i] << endl;

    SSTime now = SSTime::fromSystem();
    SSSpherical here = SSSpherical ( SSAngle::fromDegrees ( -122.0 ), SSAngle::fromDegrees ( 37.0 ) , 0.0 );
    SSCoordinates coords ( now, here );
    SSCelestronMount mount ( kAltAzimuthGotoMount, kSkyWatcherSynScan, coords );
    
    SSMount::Error err = mount.connect ( serialPortPaths[2], 0 );
    if ( err )
    {
        cout << "connect() returned error " << err << endl;
        return err;
    }

    cout << "Controller version: " << mount.getVersion() << endl;
    
    // Query whether mount is aligned or not
    
    bool status = false;
    err = mount.aligned ( status );
    if ( err )
    {
        cout << "aligned() returned error " << err << endl;
        return err;
    }
    
    cout << "Mount is " << ( status ? "aligned!" : "NOT aligned!" ) << endl;
    
    // Test setting date/time
    
    err = mount.setDateTime ( now );
    if ( err )
    {
        cout << "setDateTime() returned error " << err << endl;
        return err;
    }
    
    cout << "setDateTime() succeded!" << endl;
    
    // Test setting location
    
    err = mount.setLonLat ( here );
    if ( err )
    {
        cout << "setLonLat() returned error " << err << endl;
        return err;
    }
    
    cout << "setLonLat() succeded!" << endl;
    
    // Test reading RA/Dec
    
    SSAngle ra, dec;
    err = mount.read ( ra, dec );
    if ( err )
    {
        cout << "read() returned error " << err << endl;
        return err;
    }

    cout << "RA: " << SSHourMinSec ( ra ).toString() << "  Dec: " << SSDegMinSec ( dec ).toString() << endl;
    sleep ( 1 );
    
    // Test setting mount slew rate
    
    err = mount.rate ( mount.maxSlewRate() );
    if ( err )
    {
        cout << "rate() returned error " << err << endl;
        return err;
    }
    
    // Test slewing in Azimuth/RA
    
    err = mount.slew ( kAzmRAAxis, kSlewPositive );
    if ( err )
    {
        cout << "slew ( kAzmRAAxis, kSlewPositive ) returned error " << err << endl;
        return err;
    }

    cout << "slew ( kAzmRAAxis, kSlewPositive ) succeded!" << endl;
    sleep ( 3 );
    
    err = mount.slew ( kAzmRAAxis, kSlewOff );
    if ( err )
    {
        cout << "slew ( kAzmRAAxis, kSlewOff ) returned error " << err << endl;
        return err;
    }

    cout << "slew ( kAzmRAAxis, kSlewOff ) succeded!" << endl;
    sleep ( 1 );

    // Test slewing in Altitude/Dec
    
    err = mount.slew ( kAltDecAxis, kSlewNegative );
    if ( err )
    {
        cout << "slew ( kAltDecAxis, kSlewNegative ) returned error " << err << endl;
        return err;
    }

    cout << "slew ( kAltDecAxis, kSlewNegative ) succeded!" << endl;
    sleep ( 3 );

    err = mount.slew ( kAltDecAxis, kSlewOff );
    if ( err )
    {
        cout << "slew ( kAzmRAAxis, kSlewOff ) returned error " << err << endl;
        return err;
    }

    cout << "slew ( kAltDecAxis, kSlewOff ) succeded!" << endl;
    sleep ( 1 );
    
    // After slewing, test reading RA/Dec again
    
    SSAngle ra0 = ra, dec0 = dec;
    err = mount.read ( ra, dec );
    if ( err )
    {
        cout << "read() returned error " << err << endl;
        return err;
    }

    cout << "RA: " << SSHourMinSec ( ra ).toString() << "  Dec: " << SSDegMinSec ( dec ).toString() << endl;
    sleep ( 1 );

    // Test slewing by issuing a GoTo back to the original RA/Dec
    
    err = mount.slew ( ra0, dec0 );
    if ( err )
    {
        cout << "slew ( ra0, dec0 ) returned error " << err << endl;
        return err;
    }

    cout << "slew ( ra0, dec0 ) succeded!" << endl;
    sleep ( 1 );

    // ... but kill the GoTo after 1 second!
    
    err = mount.stop();
    if ( err )
    {
        cout << "stop() returned error " << err << endl;
        return err;
    }

    cout << "stop() succeded!" << endl;
    sleep ( 1 );
    
    // Finally resume GoTo to original RA/Dec
    
    err = mount.slew ( ra0, dec0 );
    if ( err )
    {
        cout << "slew ( ra0, dec0 ) returned error " << err << endl;
        return err;
    }

    cout << "slew ( ra0, dec0 ) succeded!" << endl;
    sleep ( 1 );

    // Test slewing() status query
    
    status = true;
    while ( status )
    {
        err = mount.slewing ( status );
        if ( err )
        {
            cout << "slewing() returned error " << err << endl;
            return err;
        }
        
        cout << "Still slewing..." << endl;
        sleep ( 1 );
    }
    
    // Read final RA/Dec
    
    err = mount.read ( ra, dec );
    if ( err )
    {
        cout << "read() returned error " << err << endl;
        return err;
    }

    cout << "RA: " << SSHourMinSec ( ra ).toString() << "  Dec: " << SSDegMinSec ( dec ).toString() << endl;

    // Sync on initial RA/Dec
    
    err = mount.sync ( ra0, dec0 );
    if ( err )
    {
        cout << "sync() returned error " << err << endl;
        return err;
    }

    cout << "sync() succeded!" << endl;
    sleep ( 1 );
    
    // Read RA/Dec after sync
    
    err = mount.read ( ra, dec );
    if ( err )
    {
        cout << "read() returned error " << err << endl;
        return err;
    }

    cout << "RA: " << SSHourMinSec ( ra ).toString() << "  Dec: " << SSDegMinSec ( dec ).toString() << endl;
    cout << "All tests succeeded!" << endl;
    
    mount.disconnect();
    return err;
}
