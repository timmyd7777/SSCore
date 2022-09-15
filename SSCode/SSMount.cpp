// SSMount.cpp
// SSCore
//
// Created by Tim DeBenedictis on 9/14/22.
// Copyright © 2022 Southern Stars. All rights reserved.
//

#include "SSMount.hpp"

#include <unistd.h>

// Constructor

SSMount::SSMount ( SSMountType type, SSCoordinates &coords ) : _coords ( coords )
{
    _type = type;
    _protocol = kNoProtocol;
    
    _serial = SSSerial();
    _socket = SSSocket();
    
    _currRA = _currDec = INFINITY;
    _slewRA = _slewDec = INFINITY;
    
    _slewRate = maxSlewRate();
    _slewSign[ kAzmRAAxis ] = _slewSign[ kAltDecAxis ] = kSlewOff;
}

// Destructor

SSMount::~SSMount ( void )
{
    disconnect();
}

// open serial or socket connection, and configure serial port settings for communication

SSMount::Error SSMount::connect ( const string &path, unsigned short port, int baud, int parity, int data, float stop )
{
    if ( connected() )
        disconnect();
    
    if ( port )
    {
        SSIP addr ( path );
        if ( addr )
            _socket.openSocket ( addr, port, 2000 );
        
        if ( ! _socket.socketOpen() )
        {
            vector<SSIP> addrs = SSSocket::hostNameToIPs ( path );
            for ( SSIP &addr : addrs )
                if ( _socket.openSocket ( addr, port, 2000 ) )
                    break;
        }
        
        if ( ! _socket.socketOpen() )
            return kOpenFail;
    }
    else
    {
        _serial.openPort ( path );
        if ( ! _serial.portOpen() )
            return kOpenFail;
        
        if ( ! _serial.setPortConfig ( baud, parity, data, stop ) )
        {
            _serial.closePort();
            return kOpenFail;
        }
    }
    
    return kSuccess;
}

// close serial or socket connection to mount

SSMount::Error SSMount::disconnect ( void )
{
    if ( _socket.socketOpen() )
        _socket.closeSocket();
    
    if ( _serial.portOpen() )
        _serial.closePort();
    
    return kSuccess;
}

// returns true if serial port or socket connection to mount is currently open

bool SSMount::connected ( void )
{
    return _socket.socketOpen() || _serial.portOpen();
}

SSMount::Error SSMount::serialCommand ( const char *input, int inlen, char *output, int outlen, char term, int timeout_ms )
{
    // If any bytes are available in the serial input buffer, read and discard them.
    
    int bytes = _serial.inputBytes();
    if ( bytes > 0 )
    {
        vector<char> junk ( bytes );
        if ( _serial.readPort ( &junk[0], bytes ) )
            return kReadFail;
    }
    
    // If input command length not specified, use input string length.
    
    if ( inlen == 0 && input != nullptr )
        inlen = (int) strlen ( input );
    
    // If we have valid command input, write it to the serial port
    
    if ( inlen > 0 && input != nullptr )
        if ( _serial.writePort ( input, inlen ) != inlen )
            return kWriteFail;
    
    // If we don't want a reply, we are done!
    
    if ( outlen < 1 )
        return kSuccess;

    // Otherwise read a reply from the serial port, one byte at a time,
    // until we receieve a terminator character, fill the output buffer, or time out.
    
    int bytesRead = 0;
    double start = clocksec();
    while ( clocksec() - start < timeout_ms / 1000.0 )
    {
        if ( _serial.inputBytes() < 1 )
        {
            usleep ( 1000 );
        }
        else
        {
            bytes = _serial.readPort ( output + bytesRead, 1 );
            if ( bytes < 1 )
                return kReadFail;
            
            bytesRead++;
            if ( ( term && output[ bytesRead - 1 ] == term ) || bytesRead == outlen )
                return kSuccess;
        }
    }
    
    return kTimedOut;   // we timed out
}

SSMount::Error SSMount::socketCommand ( const char *input, int inlen, char *output, int outlen, char term, int timeout_ms )
{
    int bytes = _socket.readSocket ( nullptr, 0 );
    if ( bytes )
    {
        vector<char> junk ( bytes );
        if ( _socket.readSocket ( &junk, bytes ) != bytes )
            return kReadFail;
    }
    
    // If input command length not specified, use input string length.
    
    if ( inlen == 0 && input != nullptr )
        inlen = (int) strlen ( input );
    
    // If we have valid command input, write it to the serial port
    
    if ( inlen > 0 && input != nullptr )
        if ( _socket.writeSocket ( input, inlen ) != inlen )
            return kWriteFail;
    
    // If we don't want a reply, we are done!
    
    if ( outlen < 1 )
        return kSuccess;

    // Otherwise read a reply from the socket, one byte at a time,
    // until we receieve a terminator character, fill the output buffer, or time out.
    
    int bytesRead = 0;
    double start = clocksec();
    while ( clocksec() - start < timeout_ms / 1000.0 )
    {
        if ( _socket.readSocket ( nullptr, 0 ) < 1 )
        {
            usleep ( 1000 );
        }
        else
        {
            bytes = _socket.readSocket ( output + bytesRead, 1 );
            if ( bytes < 1 )
                return kReadFail;
            
            bytesRead++;
            if ( ( term && output[ bytesRead - 1 ] == term ) || bytesRead == outlen )
                return kSuccess;
        }
    }

    return kTimedOut;   // we timed out
}

SSMount::Error SSMount::command ( const char *input, int inlen, char *output, int outlen, char term, int timeout_ms )
{
    if ( _serial.portOpen() )
        return serialCommand ( input, inlen, output, outlen, term, timeout_ms );
    else if ( _socket.socketOpen() )
        return socketCommand ( input, inlen, output, outlen, term, timeout_ms );
    else
        return kInvalidInput;
}

SSMount::Error SSMount::command ( const string &instr, string &outstr, char term, int timeout_ms )
{
    char output[1024] = { 0 };
    Error result = command ( instr.c_str(), (int) instr.length(), output, sizeof ( output ), term, timeout_ms );
    outstr = string ( output );
    return result;
}

// get current RA/Dec coordinates from mount

SSMount::Error SSMount::read ( SSAngle &ra, SSAngle &dec )
{
    ra = _currRA;
    dec = _currDec;
    return kSuccess;
};

// start slewing to target RA/Dec coordinates (GoTo) at fastest possible rate

SSMount::Error SSMount::slew ( SSAngle ra, SSAngle dec )
{
    _slewRA = ra;
    _slewDec = dec;
    return kSuccess;
}

// start or stop slewing mount on am axis in positive or negative direction at current slew rate

SSMount::Error SSMount::slew ( SSSlewAxis axis, SSSlewSign sign )
{
    _slewSign[ axis ] = sign;
    return kSuccess;
}

// set motion rate for directional slewing

SSMount::Error SSMount::rate ( int rate )
{
    if ( rate < 0 || rate > maxSlewRate() )
        return kInvalidInput;
    
    _slewRate = rate;
    return kSuccess;
}

// stop slewing, cancel any in-progress GoTo, and resume sidereal tracking

SSMount::Error SSMount::stop ( void )
{
    _slewSign[ kAzmRAAxis ] = _slewSign[ kAltDecAxis ] = kSlewOff;
    return kSuccess;
}

// sync or align mount on the specified coordinates

SSMount::Error SSMount::sync ( SSAngle ra, SSAngle dec )
{
    _currRA = ra;
    _currDec = dec;
    return kSuccess;
}

// Queries mount to determine whether a GoTo slew is currently in progress

SSMount::Error SSMount::slewing ( bool &status )
{
    return kSuccess;
}

// Queries mount to determine whether initial star alignment is complete

SSMount::Error SSMount::aligned ( bool &status )
{
    return kSuccess;
}

// returns true if mount is currently slewing to target RA/Dec (i.e. doing a GoTo)

bool SSMount::slewing ( void )
{
    return true;
}

// returns true if mount is currently tracking sidereal motion, but not slewing

bool SSMount::tracking ( void )
{
    return true;
}

// send local date and time to mount

SSMount::Error SSMount::setDateTime ( SSTime time )
{
    return kSuccess;
}

// send geographic location to mount

SSMount::Error SSMount::setLonLat ( SSSpherical loc )
{
    return kSuccess;
}

// Overrides and mount-specific methods for Celestron NexStar and SkyWatcher/Orion SynScan controllers

SSCelestronMount::SSCelestronMount ( SSMountType type, SSMountProtocol protocol, SSCoordinates &coords ) : SSMount ( type, coords )
{
    _protocol = protocol;
    _trackMode = kUnknownTracking;
}

SSMount::Error SSCelestronMount::getTrackingMode ( TrackingMode &mode )
{
    char input[2] = { 0 }, output[4] = { 0 };

    input[0] = 't';
    Error err = command ( input, 1, output, 2, '#' );
    if ( err )
        return err;
    
    if ( output[0] == 0 )
        mode = kNoTracking;
    else if ( output[0] == 1 )
        mode = kAltAzTracking;
    else if ( output[0] == 2 )
        mode = kEquNorthTracking;
    else if ( output[0] == 2 )
        mode = kEquSouthTracking;
    else
        mode = kUnknownTracking;
    
    return kSuccess;
}

SSMount::Error SSCelestronMount::setTrackingMode ( TrackingMode mode )
{
    char input[4] = { 0 }, output[2] = { 0 };

    input[0] = 'T';
    input[1] = mode;

    Error err = command ( input, 2, output, 1, '#' );
    return err;
}

SSMount::Error SSCelestronMount::connect ( const string &path, unsigned short port )
{
    Error err = kSuccess;
    
    // Open serial communication at 9600 baud, no parity, 8 data bits, 1 stop bit
    
    err = SSMount::connect ( path, port, 9600, SSSerial::kNoParity, SSSerial::k8DataBits, SSSerial::k1StopBits );
    if ( err != kSuccess )
        return err;

    // Get hand controller firmware version.
    // SynScan controllers return a 6-character version string terminated by a '#'.
    // NexStar hand controllers return a 3-character version string with no terminator.

    char reply[8] = { 0 };
    if ( _protocol == kSkyWatcherSynScan )
        err = command ( "V#", 0, reply, 7, '#' );
    else
        err = command ( "V#", 0, reply, 3, 0 );
    if ( err != kSuccess )
        return err;
    
    // SynScan firmware version strings are three hex digits corresponding to decimals.
    // For example hex 042507 corresponds to 04.37.07.

    if ( _protocol == kSkyWatcherSynScan )
    {
        int ver[3] = { 0 };
        if ( sscanf ( reply, "%2x%2x%2x", &ver[0], &ver[1], &ver[2] ) < 3 )
        {
            disconnect();
            return kInvalidOutput;
        }
        _version = format ( "%d.%d.%d", ver[0], ver[1], ver[2] );
    }
    else
    {
        // NexStar hand controller firmware version < 4.0 MAY be a StarSense hand controller,
        // or it may not - Celestron started renumbering StarSense HC firmware versions at 1.0!
        
        _version = format ( "%d.%d", reply[0], reply[1] );
        if ( _version.compare ( "4.0" ) < 0 )
        {
            // If controller responds to this platform query, it's a StarSense.  We'll
            // distinguish internally by prefixing with a '1': i.e. StarSense firmware 1.1
            // becomes our 10.1, StarSense 2.4 becomes our 12.4 etc.  That works for us
            // until StarSense firmware versions reach 10.0 :-)
            
            err = command ( "v#", 0, reply, 3, '#' );
            if ( err == kSuccess )
                _version = format ( "1%d.%d", reply[0], reply[1] );
        }
    }
    
    return kSuccess;
}

SSMount::Error SSCelestronMount::read ( SSAngle &ra, SSAngle &dec )
{
    Error err = kSuccess;
    char  reply[256] = { 0 };
 
    // Send the 32-bit "Get R.A./Dec." command.

    err = command ( "e", 0, reply, 18, '#' );
    if ( err != kSuccess )
        return err;
    
    // If we got a valid response, read the telescope's right
    // ascension and declination from the response string.

    if ( strlen ( reply ) < 18 || reply[8] != ',' || reply[17] != '#' )
        return kInvalidOutput;

    unsigned int hexRA = 0, hexDec = 0, sign = 0;

    // Convert hex values to integers

    sscanf ( &reply[0], "%x", &hexRA );
    sscanf ( &reply[9], "%x", &hexDec );
    
    // Convert 32-bit values to degrees
    
    ra   = SSAngle::fromDegrees ( hexRA  / ( 4294967296.0 / 360.0 ) );
    dec  = SSAngle::fromDegrees ( hexDec / ( 4294967296.0 / 360.0 ) );
    sign = ( 0xC0000000 & hexDec ) >> 16;

    // Now adjust the declination if it's greater than 90 degrees or less than -90 degrees.
    
    if ( sign == 0x4000 )             // Cross the north pole.
        dec = dec - SSAngle::kHalfPi;
    else if ( sign == 0x8000 )        // Cross the equator and set the sign for the southern hemisphere.
        dec = - ( dec - SSAngle::kPi );
    else if ( sign == 0xC000 )        // Cross the south pole and set the sign for the southern hemisphere.
        dec = dec - SSAngle::kTwoPi;

    // NexStar HC firmware versions >= 4.18 and StarSense HC (version > 10) report RA/Dec
    // in current precession epoch, not J2000; so convert from current to J2000.
    // SynScan controllers always use J2000.

    if ( _protocol == kCelestronNexStar && strtofloat ( _version ) > 4.175 )
        _coords.transform ( kEquatorial, kFundamental, ra, dec );
    
    return kSuccess;
}

SSMount::Error SSCelestronMount::slew ( SSAngle ra, SSAngle dec )
{
    // NexStar HC firmware versions >= 4.18 (and StarSense HC versions > 10) accept RA/Dec
    // in current precession epoch, not J2000; so convert from J2000 to current.
    // SynScan controllers always use J2000.
    
    if ( _protocol == kCelestronNexStar && strtofloat ( _version ) > 4.175 )
        _coords.transform ( kFundamental, kEquatorial, ra, dec );

    // Use the 32-bit version of the goto-RA/Dec command.
    // Allow an unusually-long (5-sec) timeout for this command, but no retries.
    // Some old Celestrons (and SynScan clones) are particularly slow with this.

    unsigned int hexRA  = ra  * 4294967296.0 / SSAngle::kTwoPi;
    int          hexDec = dec * 4294967296.0 / SSAngle::kTwoPi;
    
    string input = format ( "r%08X,%08X", hexRA, hexDec ), output;
    Error err = command ( input, output, '#', 5000 );
    
    return err;
}

SSMount::Error SSCelestronMount::slew ( SSSlewAxis axis, SSSlewSign sign )
{
    Error err = kSuccess;
    unsigned char input[8] = { 0 } , output[2] = { 0 };
    
    // If we have stopped slewing on both axes, get the current sidereal tracking mode
    
    if ( _slewSign[kAzmRAAxis] == kSlewOff && _slewSign[kAltDecAxis] == kSlewOff )
    {
        err = getTrackingMode ( _trackMode );
        if ( err )
            return err;
    }
            
    // Set up the input command
    
    input[0] = 'P';
    input[1] = 2;
    input[2] = axis == kAltDecAxis ? 17 : 16;
    input[3] = sign == kSlewPositive ? 36 : 37;
    input[4] = sign == kSlewOff ? 0 : _slewRate;
    input[5] = 0;
    input[6] = 0;
    input[7] = 0;

    // Send the fixed-track-rate command.
    
    err = command ( (char *) input, 8, (char *) output, 1, 0 );
    if ( err )
        return err;

    // If mount did not return 35 or 80, return error code.
    
    if ( output[0] != 'P' && output[0] != '#' )
        return kInvalidOutput;
    
    // If we have stopped slewing on both axes, restore the original tracking mode
    
    _slewSign[ axis ] = sign;
    if ( _slewSign[kAzmRAAxis] == kSlewOff && _slewSign[kAltDecAxis] == kSlewOff )
        setTrackingMode ( _trackMode );
    
    return kSuccess;
}

SSMount::Error SSCelestronMount::stop ( void )
{
    char output[2] = { 0 };
    Error err = command ( "M", 1, output, 1, '#' );
    if ( err )
        return err;
    
    // This fixes a bug in the SynScan firmware <= v3.36: the scope stops
    // moving after the above "Stop" command - but then starts moving again!
    // Here we explicitly kill the axis motion.
    
    if ( _protocol == kSkyWatcherSynScan && strtofloat ( _version ) < 3.355 )
    {
        slew ( kAzmRAAxis, kSlewOff );
        slew ( kAltDecAxis, kSlewOff );
    }
    
    return kSuccess;
}

SSMount::Error SSCelestronMount::slewing ( bool &status )
{
    char output[2] = { 0 };
    Error err = command ( "L", 1, output, 2, '#' );
    if ( err )
        return err;
    
    status = output[0] == '1';  // response is ASCII '1' or '0'
    return kSuccess;
}

SSMount::Error SSCelestronMount::aligned ( bool &status )
{
    char output[2] = { 0 };
    Error err = command ( "J", 1, output, 2, '#' );
    if ( err )
        return err;
    
    status = output[0] == 1;    // response is binary 1 or 0
    return kSuccess;
}

SSMount::Error SSCelestronMount::sync ( SSAngle ra, SSAngle dec )
{
    // SynScan controllers running firmware versions > 3.37 or 4.37
    // support a native Sync command on the hand controller.
    
    if ( _protocol == kSkyWatcherSynScan )
    {
        if ( _version[0] == '3' && strtofloat ( _version ) < 3.365 )
            return kNotSupported;
        if ( _version[0] == '4' && strtofloat ( _version ) < 3.365 )
            return kNotSupported;
    }

    // Celestron hand controllers running firmware >= version 4.10 support native sync.
    
    if ( _protocol == kCelestronNexStar )
    {
        if ( strtofloat ( _version ) < 4.095 )
            return kNotSupported;
    }

    // NexStar HC firmware versions >= 4.18 (and StarSense HC versions > 10) accept RA/Dec
    // in current precession epoch, not J2000; so convert from J2000 to current.
    // SynScan controllers always use J2000.
    
    if ( _protocol == kCelestronNexStar && strtofloat ( _version ) > 4.175 )
        _coords.transform ( kFundamental, kEquatorial, ra, dec );

    unsigned int hexRA  = ra  * 4294967296.0 / SSAngle::kTwoPi;
    int          hexDec = dec * 4294967296.0 / SSAngle::kTwoPi;
    
    string input = format ( "s%08X,%08X", hexRA, hexDec ), output;
    Error err = command ( input, output, '#' );
    
    return err;
}

SSMount::Error SSCelestronMount::setDateTime ( SSTime time )
{
    SSDate date ( time );
    
    // if daylight savings time, adjust time zone to standard
    
    bool dst = _coords.isDST();
    if ( dst )
        time.zone -= 1.0;
    
    // format input command string
    
    char input[10] = { 0 }, output[2] = { 0 };

    input[0] = 'H';
    input[1] = date.hour;
    input[2] = date.min;
    input[3] = min ( date.sec, 59.0 );
    input[4] = date.month;
    input[5] = date.day;
    input[6] = date.year % 100;
    input[7] = time.zone; //  >= 0.0 ? time.zone : 256 - time.zone;
    input[8] = dst;

    // Send date/time/zone to mount; return error code
    
    Error err = command ( input, 9, output, 1, '#' );
    return err;
}

SSMount::Error SSCelestronMount::setLonLat ( SSSpherical loc )
{
    // Compute latitude, longitude degrees/minutes/seconds

    SSDegMinSec lat ( loc.lat );
    SSDegMinSec lon ( loc.lon );
    
    // format command input string
    
    char input[10] = { 0 }, output[2] = { 0 };

    input[0] = 'W';
    input[1] = lat.deg;
    input[2] = lat.min;
    input[3] = lat.sec;
    input[4] = lat.sign == '+' ? 0 : 1;
    input[5] = lon.deg;
    input[6] = lon.min;
    input[7] = lon.sec;
    input[8] = lon.sign == '+' ? 0 : 1;
    
    // Send location to the mount; return error code.

    Error err = command ( input, 9, output, 1, '#' );
    return err;
}
