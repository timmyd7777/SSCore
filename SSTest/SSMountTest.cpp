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
#include <algorithm>

#include "SSMount.hpp"

int main ( int argc, const char * argv[] )
{
    // Display list of supported mount protocols, select one to use for testing
    
    SSMountProtocolMap protoMap;
    int numProtos = SSGetMountProtocols ( protoMap );
    auto iter = protoMap.begin();
    for ( int i = 0; i < numProtos; i++, iter++ )
        cout << "Mount Protocol " << i + 1 << ": " << iter->second << endl;

    int testProto = 0;
    while ( testProto < 1 || testProto > numProtos )
    {
        cout << "Test which mount protocol (1 thru " << numProtos << ")? " << endl;
        cin >> testProto;
    }
    
    iter = protoMap.begin();
    advance ( iter, testProto - 1 );

    // Obtain list of serial ports, select one to use for testing
    
    vector<string> serialPortNames, serialPortPaths;
    int numPorts = SSSerial::listPorts ( serialPortNames, serialPortPaths );
    cout << "Found " << numPorts << " serial ports:" << endl;
    if ( numPorts < 1 )
        return 0;
    
    for ( int i = 0; i < numPorts; i++ )
        cout << "Port " << i + 1 << ": " << serialPortNames[i] << " at " << serialPortPaths[i] << endl;
    
    int testPort = 0;
    while ( testPort < 1 || testPort > numPorts )
    {
        cout << "Use which port for testing (1 thru " << numPorts << ")? " << endl;
        cin >> testPort;
    }
    
    // Initialize telescope and create SSMount instance
    
    SSTime now = SSTime::fromSystem();
    SSSpherical here = SSSpherical ( SSAngle::fromDegrees ( -122.0 ), SSAngle::fromDegrees ( 37.0 ) , 0.0 );
    SSCoordinates coords ( now, here );
    SSMountPtr pMount = SSNewMount ( kAltAzimuthGotoMount, iter->first, coords );

    // Open serial or socket connection to mount
    
    SSMount::Error err = pMount->connect ( serialPortPaths[ testPort - 1 ], 0 );
    if ( err )
    {
        cout << "connect() returned error " << err << endl;
        return err;
    }

    // Display mount controller firmware version
    cout << "Mount controller version: " << pMount->getVersion() << endl;
    
    // Query whether mount is aligned or not
    
    bool status = false;
    err = pMount->aligned ( status );
    if ( err )
    {
        cout << "aligned() returned error " << err << endl;
        return err;
    }
    
    cout << "Mount is " << ( status ? "aligned!" : "NOT aligned!" ) << endl;
    
    // Test setting date/time
    
    err = pMount->setDateTime();
    if ( err )
    {
        cout << "setDateTime() returned error " << err << endl;
        return err;
    }
    
    cout << "setDateTime() succeded!" << endl;
    
    // Test setting location
    
    err = pMount->setLonLat();
    if ( err )
    {
        cout << "setLonLat() returned error " << err << endl;
        return err;
    }
    
    cout << "setLonLat() succeded!" << endl;
    
    // Test reading RA/Dec
    
    SSAngle ra, dec;
    err = pMount->read ( ra, dec );
    if ( err )
    {
        cout << "read() returned error " << err << endl;
        return err;
    }

    cout << "RA: " << SSHourMinSec ( ra ).toString() << "  Dec: " << SSDegMinSec ( dec ).toString() << endl;
    sleep ( 1 );
    
    // Test setting mount slew rate
    
    err = pMount->rate ( pMount->maxSlewRate() );
    if ( err )
    {
        cout << "rate() returned error " << err << endl;
        return err;
    }
    
    // Test slewing in Azimuth/RA
    
    err = pMount->slew ( kAzmRAAxis, kSlewPositive );
    if ( err )
    {
        cout << "slew ( kAzmRAAxis, kSlewPositive ) returned error " << err << endl;
        return err;
    }

    cout << "slew ( kAzmRAAxis, kSlewPositive ) succeded!" << endl;
    sleep ( 3 );
    
    err = pMount->slew ( kAzmRAAxis, kSlewOff );
    if ( err )
    {
        cout << "slew ( kAzmRAAxis, kSlewOff ) returned error " << err << endl;
        return err;
    }

    cout << "slew ( kAzmRAAxis, kSlewOff ) succeded!" << endl;
    sleep ( 1 );

    // Test slewing in Altitude/Dec
    
    err = pMount->slew ( kAltDecAxis, kSlewNegative );
    if ( err )
    {
        cout << "slew ( kAltDecAxis, kSlewNegative ) returned error " << err << endl;
        return err;
    }

    cout << "slew ( kAltDecAxis, kSlewNegative ) succeded!" << endl;
    sleep ( 3 );

    err = pMount->slew ( kAltDecAxis, kSlewOff );
    if ( err )
    {
        cout << "slew ( kAzmRAAxis, kSlewOff ) returned error " << err << endl;
        return err;
    }

    cout << "slew ( kAltDecAxis, kSlewOff ) succeded!" << endl;
    sleep ( 1 );
    
    // After slewing, test reading RA/Dec again
    
    SSAngle ra0 = ra, dec0 = dec;
    err = pMount->read ( ra, dec );
    if ( err )
    {
        cout << "read() returned error " << err << endl;
        return err;
    }

    cout << "RA: " << SSHourMinSec ( ra ).toString() << "  Dec: " << SSDegMinSec ( dec ).toString() << endl;
    sleep ( 1 );

    // Test slewing by issuing a GoTo back to the original RA/Dec
    
    err = pMount->slew ( ra0, dec0 );
    if ( err )
    {
        cout << "slew ( ra0, dec0 ) returned error " << err << endl;
        return err;
    }

    cout << "slew ( ra0, dec0 ) succeded!" << endl;
    sleep ( 1 );

    // ... but kill the GoTo after 1 second!
    
    err = pMount->stop();
    if ( err )
    {
        cout << "stop() returned error " << err << endl;
        return err;
    }

    cout << "stop() succeded!" << endl;
    sleep ( 1 );
    
    // Finally resume GoTo to original RA/Dec
    
    err = pMount->slew ( ra0, dec0 );
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
        err = pMount->slewing ( status );
        if ( err )
        {
            cout << "slewing() returned error " << err << endl;
            return err;
        }
        
        cout << "Still slewing..." << endl;
        sleep ( 1 );
    }
    
    // Read final RA/Dec
    
    err = pMount->read ( ra, dec );
    if ( err )
    {
        cout << "read() returned error " << err << endl;
        return err;
    }

    cout << "RA: " << SSHourMinSec ( ra ).toString() << "  Dec: " << SSDegMinSec ( dec ).toString() << endl;

    // Sync on initial RA/Dec
    
    err = pMount->sync ( ra0, dec0 );
    if ( err )
    {
        cout << "sync() returned error " << err << endl;
        return err;
    }

    cout << "sync() succeded!" << endl;
    sleep ( 1 );
    
    // Read RA/Dec after sync
    
    err = pMount->read ( ra, dec );
    if ( err )
    {
        cout << "read() returned error " << err << endl;
        return err;
    }

    cout << "RA: " << SSHourMinSec ( ra ).toString() << "  Dec: " << SSDegMinSec ( dec ).toString() << endl;
    cout << "All tests succeeded!" << endl;
    
    pMount->disconnect();
    return err;
}
