// SSMountTest.cpp
// SSCore
//
// Created by Tim DeBenedictis on 9/15/22.
// Copyright © 2022 Southern Stars. All rights reserved.
//
// Command-line test program for telescope mount communication classes.

#include <cstdio>
#include <iostream>
#include <algorithm>
#include <map>
#include <thread>

#include "SSMount.hpp"
#include "SSUtilities.hpp"

map<SSMount::Error,string> SSMountErrors =
{
    { SSMount::kSuccess, "kSuccess" },
    { SSMount::kInvalidInput, "kInvalidInput" },
    { SSMount::kInvalidOutput, "kInvalidOutput" },
    { SSMount::kInvalidCoords, "kInvalidCoords" },
    { SSMount::kOpenFail, "kOpenFail" },
    { SSMount::kCloseFail, "kCloseFail" },
    { SSMount::kReadFail, "kReadFail" },
    { SSMount::kWriteFail, "kWriteFail" },
    { SSMount::kNotSupported, "kNotSupported" },
    { SSMount::kTimedOut, "kTimedOut" }
};

int main ( int argc, const char * argv[] )
{
    SSIP skyfiIP;
    bool success = SSFindSkyFi ( "SkyFi", skyfiIP );
    if ( success )
        printf ( "SkyFi found at %s\n", skyfiIP.toString().c_str() );
    else
        printf ( "Failed to find SkyFi IP!\n" );
    
    exit ( 0 );
    
//    SSSocket::initialize();

    SSIP v4 ( "192.168.0.2" );
    SSIP v6 ( "2345:0425:2CA1:0000:0000:0567:5673:23b5" );
    SSIP v8 ( "This is a dummy string" );
    
    cout << v4.toString() << endl;
    cout << v6.toString() << endl;
    cout << v8.toString() << endl;
    cout << SSSocket::IPtoHostName ( v6 ) << endl;
    
    cout << "Testing SSSocket::hostNameToIPs( \"ip-api.com\", false )" << endl;
    vector<SSIP> ips = SSSocket::hostNameToIPs( "ip-api.com", false );
    for ( SSIP &ip : ips )
        cout << ip.toString() <<  ", " << SSSocket::IPtoHostName ( ip ) << endl;
    
    cout << "Testing SSSocket::hostNameToIPs( \"www.southernstars.com\", true )" << endl;
    ips = SSSocket::hostNameToIPs( "www.southernstars.com", true );
    for ( SSIP &ip : ips )
        cout << ip.toString() << ", " << SSSocket::IPtoHostName ( ip ) << endl;

    cout << "Testing SSSocket::getLocalIPs ( false )" << endl;
    ips = SSSocket::getLocalIPs ( false );
    for ( SSIP &ip : ips )
        cout << ip.toString() << endl;

    cout << "Testing SSSocket::getLocalIPs ( true )" << endl;
    ips = SSSocket::getLocalIPs ( true );
    for ( SSIP &ip : ips )
        cout << ip.toString() << endl;

    SSSocket sock;
    vector<SSIP> server = SSSocket::hostNameToIPs ( "www.southernstars.com", false );
    if ( server.size() > 0 )
	{
		cout << "www.southernstars.com is ";
		v6 = server[0].toIPv6();
		cout << v6.toString() << endl;
	
		if ( sock.openSocket ( v6, 80, 1000 ) )
			cout << "sock.openSocket ( server, 80, 1000 ) succeeded!" << endl;
		else
			cout << "sock.openSocket ( server, 80, 1000 ) failed!" << endl;
    
		SSIP peer;
		peer.ipv6 = true;
		if ( sock.getRemoteIP ( peer ) )
			cout << "sock.getPeerIP ( peer ) succeeded!" << endl;
		else
			cout << "sock.getPeerIP ( peer ) failed!" << endl;

		cout << peer.toString() << endl;
		sock.closeSocket();
	}
	else
	{
		cout << "SSSocket::hostNameToIPs ( \"www.southernstars.com\", false ) failed!" << endl;
	}
	
    // Get current location from IP address - this tests SSHTTP API!
    
    SSSpherical here;
    if ( SSLocationFromIP ( here ) )
        cout << "SSLocationFromIP() returned"
             << " lon " << SSDegMinSec ( here.lon ).toString()
             << " lat " << SSDegMinSec ( here.lat ).toString() << endl;
    else
        cout << "SSLocationFromIP() failed!\n" << endl;

    // Display list of supported mount protocols, select one to use for testing
    
    SSMountProtocolMap protoMap;
    int numProtos = SSGetMountProtocols ( protoMap );
    auto iter = protoMap.begin();
    for ( int i = 0; i < numProtos; i++, iter++ )
        cout << "Mount Protocol " << i + 1 << ": " << iter->second << endl;

    int testProto = 0;
    while ( testProto < 1 || testProto > numProtos )
    {
        cout << "Test which mount protocol (1 thru " << numProtos << ")? ";
        cin >> testProto;
    }
    
    iter = protoMap.begin();
    advance ( iter, testProto - 1 );

    // Obtain list of serial ports, select one to use for testing
    
    vector<string> serialPortNames, serialPortPaths;
    int numPorts = SSSerial::listPorts ( serialPortNames, serialPortPaths );
    cout << "Found " << numPorts << " serial ports." << endl;
    for ( int i = 0; i < numPorts; i++ )
        cout << "Port " << i + 1 << ": " << serialPortNames[i] << " at " << serialPortPaths[i] << endl;
    
    int testPort = 0;
    if ( numPorts > 0 )
    {
        do
        {
            cout << "Use which port for testing (1 thru " << numPorts << " or 0 for network)? ";
            cin >> testPort;
        }
        while ( testPort < 0 || testPort > numPorts );
    }

    // If no serial port was selected, get network address and TCP port from user
    
    string netAddress = "10.0.0.1";
    unsigned short tcpPort = 4030;
    
    if ( testPort == 0 )
    {
        cout << "Mount network or IP address: ";
        cin >> netAddress;
        
        cout << "Mount TCP port: ";
        cin >> tcpPort;
    }
    
    // Get log file path
    
    string logPath;
    cout << "Log file path (0 = no logging): ";
    cin >> logPath;
    if ( logPath.back() == '\n' )
        logPath.pop_back();
    
    // Initialize telescope and create SSMount instance
    
    SSTime now = SSTime::fromSystem();
    SSCoordinates coords ( now, here );
    SSMountPtr pMount = SSNewMount ( kAltAzimuthGotoMount, iter->first, coords );
    if ( pMount == nullptr )
    {
        cout << "SSNewMount() failed to create mount; exiting!" << endl;
        return -1;
    }

    // If we have a valid log file path, open log first.

    SSMount::Error err = SSMount::kSuccess;
    if ( logPath.length() > 1 )
    {
        err = pMount->openLog ( logPath );
        if ( err )
            cout << "openLog() returned error "  << SSMountErrors[err] << endl;
        else
            cout << "openLog() succeeded!" << endl;
    }
    
    // Open serial or socket connection to mount
    
    if ( testPort > 0 )
        err = pMount->connect ( serialPortPaths[ testPort - 1 ], 0 );
    else
        err = pMount->connect ( netAddress, tcpPort );

    if ( err )
    {
        cout << "connect() returned error " << SSMountErrors[err] << endl;
        return err;
    }

    // Display mount controller firmware version
    cout << "Mount controller version: " << pMount->getVersion() << endl;
    
    // Query whether mount is aligned or not
    
    bool status = false;
    err = pMount->aligned ( status );
    if ( err )
        cout << "aligned() returned error " << SSMountErrors[err] << endl;
    else
        cout << "Mount is " << ( status ? "aligned!" : "NOT aligned!" ) << endl;
    
    // Test getting date/time
    
    SSTime mountTime;
    err = pMount->getTime ( mountTime );
    if ( err )
        cout << "getTime() returned error " << SSMountErrors[err] << endl;
    else
        cout << "getTime() returned " << SSDate ( mountTime ).format ( "%Y/%m/%d %H:%M:%S UTC%z" ) << endl;
    
    // Test setting date/time
    
    err = pMount->setTime();
    if ( err )
        cout << "setTime() returned error " << SSMountErrors[err] << endl;
    else
        cout << "setTime() succeded!" << endl;
    
    // Test getting date/time again, after setting
    
    err = pMount->getTime ( mountTime );
    if ( err )
        cout << "getTime() returned error " << SSMountErrors[err] << endl;
    else
        cout << "getTime() returned " << SSDate ( mountTime ).format ( "%Y/%m/%d %H:%M:%S UTC%z" ) << endl;
    
    // Test getting longitude/latitude
    
    SSSpherical mountSite;
    err = pMount->getSite ( mountSite );
    if ( err )
        cout << "getSite() returned error " << SSMountErrors[err] << endl;
    else
        cout << "getSite() returned longitude " << SSDegMinSec ( mountSite.lon ).toString()
                                << " latitude " << SSDegMinSec ( mountSite.lat ).toString() << endl;

    // Test setting longitude/latitude
    
    err = pMount->setSite();
    if ( err )
        cout << "setSite() returned error " << SSMountErrors[err] << endl;
    else
        cout << "setSite() succeded!" << endl;
    
    // Test getting longitude/latitude after changing site
    
    err = pMount->getSite ( mountSite );
    if ( err )
        cout << "getSite() returned error " << SSMountErrors[err] << endl;
    else
        cout << "getSite() returned longitude " << SSDegMinSec ( mountSite.lon ).toString()
                                << " latitude " << SSDegMinSec ( mountSite.lat ).toString() << endl;
    
    SSAngle ra, dec;
    //while ( true )
    {
        err = pMount->read ( ra, dec );
        if ( err )
            cout << "read() returned error " << SSMountErrors[err] << endl;
        else
            cout << "read() returned RA: " << SSHourMinSec ( ra ).toString() << "  Dec: " << SSDegMinSec ( dec ).toString() << endl;
        sleep ( 1 );
    }
    
    // Test slewing in Azimuth/RA
    
    int rate = pMount->maxSlewRate();
    err = pMount->slew ( kAzmRAAxis, rate );
    if ( err )
        cout << "slew ( kAzmRAAxis, rate ) returned error " << SSMountErrors[err] << endl;
    else
        cout << "slew ( kAzmRAAxis, rate ) succeded!" << endl;
    sleep ( 3 );
    
    // Stop slewing in azimuth/RA
    
    err = pMount->slew ( kAzmRAAxis, 0 );
    if ( err )
        cout << "slew ( kAzmRAAxis, 0 ) returned error " << SSMountErrors[err] << endl;
    else
        cout << "slew ( kAzmRAAxis, 0 ) succeded!" << endl;
    sleep ( 1 );

    // Test slewing in Altitude/Dec
    
    err = pMount->slew ( kAltDecAxis, -rate );
    if ( err )
        cout << "slew ( kAltDecAxis, -rate ) returned error " << SSMountErrors[err] << endl;
    else
        cout << "slew ( kAltDecAxis, -rate ) succeded!" << endl;
    sleep ( 3 );

    // Stop slewing in Altitude/Dec
    
    err = pMount->slew ( kAltDecAxis, 0 );
    if ( err )
        cout << "slew ( kAzmRAAxis, 0 ) returned error " << SSMountErrors[err] << endl;
    else
        cout << "slew ( kAltDecAxis, 0 ) succeded!" << endl;
    sleep ( 1 );
    
    // After slewing, test reading RA/Dec again
    
    SSAngle ra0 = ra, dec0 = dec;
    err = pMount->read ( ra, dec );
    if ( err )
        cout << "read() returned error " << SSMountErrors[err] << endl;
    else
        cout << "read() returned RA: " << SSHourMinSec ( ra ).toString() << "  Dec: " << SSDegMinSec ( dec ).toString() << endl;
    sleep ( 1 );

    // Test slewing by issuing a GoTo back to the original RA/Dec
    
    err = pMount->slew ( ra0, dec0 );
    if ( err )
        cout << "slew ( ra0, dec0 ) returned error " << SSMountErrors[err] << endl;
    else
        cout << "slew ( ra0, dec0 ) succeded!" << endl;
    sleep ( 1 );

    // ... but stop the GoTo after 1 second!
    
    err = pMount->stop();
    if ( err )
        cout << "stop() returned error " << SSMountErrors[err] << endl;
    else
        cout << "stop() succeded!" << endl;
    sleep ( 1 );
    
    // Finally resume GoTo to original RA/Dec
    
    err = pMount->slew ( ra0, dec0 );
    if ( err )
        cout << "slew ( ra0, dec0 ) returned error " << SSMountErrors[err] << endl;
    else
        cout << "slew ( ra0, dec0 ) succeded!" << endl;
    sleep ( 1 );

    // Test slewing() status query
    
    status = true;
    while ( status )
    {
        err = pMount->slewing ( status );
        if ( err )
            cout << "slewing() returned error " << SSMountErrors[err] << endl;
        else
            cout << "Still slewing..." << endl;
        sleep ( 1 );
    }
    
    // Read final RA/Dec
    
    err = pMount->read ( ra, dec );
    if ( err )
        cout << "read() returned error " << SSMountErrors[err] << endl;
    else
        cout << "read() returned RA: " << SSHourMinSec ( ra ).toString() << "  Dec: " << SSDegMinSec ( dec ).toString() << endl;

    // Sync on initial RA/Dec
    
    err = pMount->sync ( ra0, dec0 );
    if ( err )
        cout << "sync() returned error " << SSMountErrors[err] << endl;
    else
        cout << "sync() succeded!" << endl;
    sleep ( 1 );
    
    // Read RA/Dec after sync
    
    err = pMount->read ( ra, dec );
    if ( err )
        cout << "read() returned error " << SSMountErrors[err] << endl;
    else
        cout << "read() returned RA: " << SSHourMinSec ( ra ).toString() << "  Dec: " << SSDegMinSec ( dec ).toString() << endl;
    
    // SSMount destructor closes log and disconnects

    delete pMount;
    SSSocket::finalize();
    return err;
}
