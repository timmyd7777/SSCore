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
#include "SSMountModel.hpp"
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

// Tests SSMountModel class with sample data provided by Project Pluto.
// Expected output should look like this:
// Aligned model parameters:
// -0.0007669904
// -2.1942641713
//  0.0007669904
// -0.1358079290
// Xencoder   Yencoder     Azimuth   Altitude      AzmPred    AltPred     AzmResid   AltResid
//    17.00    3866.00      -4.135     63.928       -7.034     64.385       -1.253      0.457
//  2097.00    3987.00      83.201     58.157       84.372     59.068        0.602      0.911
// ...
// RMS alignment error: 1.3236 degrees

void TestMountModel ( void )
{
    // alignment stars format is { x, y, alt, azm },
    // (x,y) are encoder counts; (alt,azm) are in degrees
    
    double stars[20][4] =
    {
        { 17, 3866, 63.927860, -4.134850 },
        { 2097, 3987, 58.157393, 83.200673 },
        { 1349, 4072, 54.451366, 54.451366 },
        { 1378, -3978, 48.301383, 52.845623 },
        { 2297, -3709, 36.089183, 92.311798 },
        { 1066, -3626, 32.031212, 39.272662 },
        { 1251, 3683, 71.564240, 48.027036 },
        { 289, -3854, 43.183365, 5.714938 },
        { -734, -4035, 52.109516, -38.657697 },
        { -863, -3695, 36.817657, -44.943200 },
        { -1114, 3673, 73.631468, -51.839778 },
        { -1451, -3813, 42.814768, -70.685456 },
        { -2219, -3702, 38.135595, -105.351328 },
        { -2459, 4072, 57.049984, -115.766982 },
        { -3297, -3997, 51.568455, -154.108377 },
        { -3850, -3652, 35.250054, -178.349256 },
        { -4035, 3784, 68.866570, 171.432717 },
        { 3004, 3796, 67.167076, 120.747715 },
        { 3518, -3685, 36.055417, 145.187806 },
        { 354, 3871, 63.454505, 9.651296 }
    };
    
    // Create mount model with 8192-step encoders on both axes.
    // Then add alignment stars.
    
    SSMountModel model ( 8192, 8192 );
    for ( int i = 0; i < 20; i++ )
        model.addStar ( stars[i][0], stars[i][1], SSAngle::fromDegrees ( stars[i][3] ), SSAngle::fromDegrees ( stars[i][2] ) );
    
    // Align the model, then print best-fit model parameters
    
    double rms_err = model.align();
    printf ( "Aligned model parameters:\n" );
    for ( int i = 0; i < 4; i++ )
        printf ( "%13.10f\n", model.getParameter ( i ) );
    
    // Print encoder positions, celestial coordinates, and residuals for alignment stars
    
    printf( "\n%10s %10s  %10s %10s   %10s %10s   %10s %10s\n",
           "Xencoder", "Yencoder", "Azimuth", "Altitude",
           "AzmPred", "AltPred", "AzmResid", "AltResid" );

    for ( int i = 0; i < 20; i++ )
    {
        SSAngle azm, alt;
        model.encodersToCelestial ( stars[i][0], stars[i][1], azm, alt );
        double azm_resid, alt_resid;
        model.getResiduals ( i, azm_resid, alt_resid );
        printf( "%10.2lf %10.2lf  %10.3lf %10.3lf   %10.3lf %10.3lf   %10.3lf %10.3lf\n",
               stars[i][0], stars[i][1], stars[i][3], stars[i][2], azm.toDegrees(), alt.toDegrees(),
               radtodeg ( azm_resid ), radtodeg ( alt_resid ) );
    }
    
    // Print total RMS alignment error
    
    cout << "RMS alignment error: " << radtodeg ( rms_err ) << " degrees\n\n";
}

int main ( int argc, const char * argv[] )
{
    // TestMountModel();
    
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
