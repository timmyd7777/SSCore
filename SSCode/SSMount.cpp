// SSMount.cpp
// SSCore
//
// Created by Tim DeBenedictis on 9/14/22.
// Copyright © 2022 Southern Stars Group, LLC. All rights reserved.
//
// This class implements communication with common amateur telescope mount
// controllers over serial port and TCP/IP sockets. Supported protocols include
// Meade LX-200/Autostar, Celestron NexStar, and SkyWatcher/Orion SynScan.

#include <thread>

#include "SSMount.hpp"
#include "SSUtilities.hpp"

// Map of supported telescope mount protocols, indexed by protocol identifier.

static SSMountProtocolMap _protocols =
{
    { kNoProtocol,        "Mount Simulator" },
    { kMeadeLX200,        "Meade LX200" },
    { kMeadeAutostar,     "Meade Autostar" },
    { kCelestronNexStar,  "Celestron NexStar" },
    { kSkyWatcherSynScan, "Skywatcher SynScan" },
    { kSyntaDirect,       "Synta Direct" },
    { kCelestronAUX,      "Celestron AUX" }
};

// Obtains map of supported mount protocol names, indexed by protocol identifier.
// Returns total number of supported protocols

int SSGetMountProtocols ( SSMountProtocolMap &map )
{
    map = _protocols;
    return (int) map.size();
}

// Allocates a new SSMount (or subclass of SSMount) instance, depending on protocol
// Returns null pointer on failure.

SSMountPtr SSNewMount ( SSMountType type, SSMountProtocol protocol, SSCoordinates &coords )
{
    if ( protocol == kMeadeLX200 || protocol == kMeadeAutostar )
        return new SSMeadeMount ( type, protocol, coords );
    
    if ( protocol == kCelestronNexStar || protocol == kSkyWatcherSynScan )
        return new SSCelestronMount ( type, protocol, coords );

    if ( protocol == kSyntaDirect )
        return new SSSyntaMount ( type, coords );

    if ( protocol == kCelestronAUX )
        return new SSCelestronAUXMount ( type, coords );

    return new SSMount ( type, coords );
};

// Downcasts generic SSMount pointer to SSMeadeMount pointer.
// Returns nullptr if input pointer is not an instance of SSMeadeMount!

SSMeadeMountPtr SSGetMeadeMountPtr ( SSMountPtr ptr )
{
    return dynamic_cast<SSMeadeMount *> ( ptr );
}

// Downcasts generic SSMount pointer to SSCelestronMount pointer.
// Returns nullptr if input pointer is not an instance of SSCelestronMount!

SSCelestronMountPtr SSGetCelestronMountPtr ( SSMountPtr ptr )
{
    return dynamic_cast<SSCelestronMount *> ( ptr );
}

// Downcasts generic SSMount pointer to SSSyntaMount pointer.
// Returns nullptr if input pointer is not an instance of SSSyntaMount!

SSSyntaMountPtr SSGetSyntaMountPtr ( SSMountPtr ptr )
{
    return dynamic_cast<SSSyntaMount *> ( ptr );
}

// Downcasts generic SSMount pointer to SSCelestronAUXMount pointer.
// Returns nullptr if input pointer is not an instance of SSCelestronAUXMount!

SSCelestronAUXMountPtr SSGetCelestronAUXMountPtr ( SSMountPtr ptr )
{
    return dynamic_cast<SSCelestronAUXMount *> ( ptr );
}

// Attempts to find SkyFi IPv4 address on the local network via UDP broadcast.
// Name of the specific SkyFi to find should be passed in (name); pass an empty
// string to find any SkyFi on the net. Pass the number of attempts (at least 1)
// and timeout for each attempt in milliseconds.
// If found, SkyFi's IPv4 address will be returned in addr.
// The function returns true if successful or false on failure.
// Default SkyFi IP address is 10.0.0.1, TCP port 4030.

bool SSFindSkyFi ( const string &name, SSIP &addr, int attempts, int timeout )
{
    vector<SSIP> localIPs = SSSocket::getLocalIPs();
    for ( SSIP ip : localIPs )
    {
        // format query to broadcast
        
        string out = "skyfi?";
        if ( ! name.empty() )
            out = "skyfi:" + name + "?";
        
        string str = ip.toString();
        printf ( "%s\n", str.c_str() );

        for ( int i = 0; i < attempts; i++ )
        {
            SSSocket sock;
            if ( sock.openUDPSocket ( ip, 0 ) )
            {
                // broadcast UDP query, then parse response if we recieved one

                if ( sock.writeUDPSocket ( out.c_str(), (int) out.length(), SSIP ( INADDR_BROADCAST ), 4031 ) == out.length() )
                {
                    char data[256] = { 0 };
                    if ( sock.readUDPSocket ( data, sizeof ( data ), addr, timeout ) > out.length() )
                    {
                        if ( strncmp ( data, out.c_str(), out.length() - 1 ) == 0 )
                        {
                            addr = SSIP ( data + out.length() );
                            sock.closeSocket();
                            return true;
                        }
                    }
                }
                sock.closeSocket();
            }
        }
    }

    return false;
}

// SSMount base class constructor. Mount has no protocol;
// all member variables are initialized to invalid values.
// The base SSMount class simulates an equatorial GoTo mount
// but has no communication protocol and controls no hardware.

SSMount::SSMount ( SSMountType type, SSCoordinates &coords ) : _coords ( coords ), _model ( SSMountModel ( 0, 0 ) )
{
    _type = type;
    _protocol = kNoProtocol;
    
    _serial = SSSerial();
    _socket = SSSocket();
    
    _addr = SSIP();
    _port = 0;
    
    _retries = 1;
    _timeout = 3000;

    _initLon = _initLat = 0;
    _currLon = _currLat = 0;
    _slewLon = _slewLat = 0;
    
    _slewRate[ kAzmRAAxis ] = _slewRate[ kAltDecAxis ] = 0;
    _slewTime[ kAzmRAAxis ] = _slewTime[ kAltDecAxis ] = 0;
    
    _connected = _slewing = _aligned = false;
    
    _logFile = NULL;
    _logStart = 0;
}

// Destructor disconnects any open serial or socket connection.

SSMount::~SSMount ( void )
{
    closeLog();
    disconnect();
}

// Default connect() method for mounts with no protocol does nothing
// other than set a dummy protocol version string.

SSMount::Error SSMount::connect ( const string &path, uint16_t port )
{
    _connected = true;
    _version = "0.0";
    return kSuccess;
}

// Opens serial or socket connection, and configure serial port settings for communication.
// Only available to subclasses of SSMount, not a public SSMount method!

SSMount::Error SSMount::connect ( const string &path, uint16_t port, int baud, int parity, int data, float stop, bool udp )
{
    // On Apple and Linux platforms, writes to closed sockets can cause a crash due to "signal 13".
    // This line converts those crashes to write failures.

#ifndef _MSC_VER_
    signal ( SIGPIPE, SIG_IGN );
#endif
    
    if ( connected() )
        disconnect();
    
    // Parse mount IP address from path string, first as dotted form,
    // then as fully-qualified domain name string using DNS.
    
    SSIP addr ( path );
    vector<SSIP> addrs;
    if ( addr )
        addrs.push_back ( addr );
    else
        addrs = SSSocket::hostNameToIPs ( path );

    // If we have a valid TCP or UDP port, opem socket connection to mount
    
    if ( port )
    {
        if ( udp )
        {
            // vector<SSIP> localIPs = SSSocket::getLocalIPs();
            // _socket.openUDPSocket ( localIPs[0], port );
            _socket.openUDPSocket ( SSIP(), port );
            if ( ! _socket.socketOpen() )
                return kOpenFail;
        }
        else
        {
            // For TCP, try connecting to all IP addresses
            // and save the first that succeeds.
            
            for ( int i = 0; i < addrs.size(); i++ )
                if ( _socket.openSocket ( addrs[i], port, _timeout ) )
                {
                    addr = addrs[i];
                    break;
                }
            
            if ( ! _socket.socketOpen() )
                return kOpenFail;
        }
        
        // Save mount IP address and TCP/UDP port
        
        _addr = addr;
        _port = port;
    }
    else    // open local serial port connection to mount
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
    
    _connected = true;
    return kSuccess;
}

// Closes serial or socket connection to mount

SSMount::Error SSMount::disconnect ( void )
{
    if ( _socket.socketOpen() )
        _socket.closeSocket();
    
    if ( _serial.portOpen() )
        _serial.closePort();
    
    _addr = SSIP();
    _port = 0;
    _connected = false;
    return kSuccess;
}

// Sends data (input) to telescope mount via serial port, and optionally waits for output.
// If input length (inlen) is zero, will use input string length unless input is null.
// If input length is zero and input is null, nothing will be sent to mount.
// Returns when output of (outlen) bytes is received, when a term character is receieved,
// or until a timeout (in milliseconds) occurrs. Returns an error code or zero if successful.

SSMount::Error SSMount::serialCommand ( const char *input, int inlen, char *output, int outlen, char term, int timeout_ms )
{
    Error err = kSuccess;
    
    // If input command length not specified, use input string length.
    
    if ( inlen == 0 && input != nullptr )
        inlen = (int) strlen ( input );
    
    // If we are going to send a command.
    
    if ( inlen > 0 && input != nullptr )
    {
        // First clear out any bytes remaining in the serial input buffer
        
        int bytes = _serial.inputBytes();
        if ( bytes > 0 )
        {
            vector<char> junk ( bytes );
            if ( _serial.readPort ( &junk[0], bytes ) < bytes )
                return kReadFail;
        }

        // Then send command input to the serial port.
        
        if ( _serial.writePort ( input, inlen ) != inlen )
            err = kWriteFail;
        
        // Log what we sent, return if we failed.
        
        writeLog ( true, input, inlen, err );
        if ( err )
            return err;
    }
    
    // If we don't want output, we are done!
    
    if ( outlen < 1 || output == nullptr )
        return kSuccess;

    // Otherwise read output data from the serial port, one byte at a time,
    // until we receieve a terminator character, fill the output buffer, or time out.
    
    int bytesRead = 0;
    double start = clocksec();
    while ( err == kSuccess )
    {
        int bytes = _serial.inputBytes();
        if ( bytes < 0 )
        {
            err = kReadFail;
            break;
        }
        else if ( bytes < 1 )
        {
            msleep ( 1 );
            if ( clocksec() - start > timeout_ms / 1000.0 )
                err = kTimedOut;
            continue;
        }

        bytes = _serial.readPort ( output + bytesRead, 1 );
        if ( bytes < 1 )
            err = kReadFail;
        else
            bytesRead += bytes;
        if ( ( term && output[ bytesRead - 1 ] == term ) || bytesRead == outlen )
            break;
    }
    
    // Log what we received, return any error code
    
    writeLog ( false, output, bytesRead, err );
    return err;
}

// Sends data (input) to telescope mount via TCP socket and optionally waits for output.
// All paramters are as described in serialCommand().
// Returns zero if successful or nonzero error code on failure.

SSMount::Error SSMount::socketCommand ( const char *input, int inlen, char *output, int outlen, char term, int timeout_ms )
{
    Error err = kSuccess;
    
    // If this a TCP connection, reopen if it's been closed; return error on failure.
    
    bool udp = _socket.isUDPSocket();
    if ( ! udp && ! _socket.socketOpen() )
        if ( ! _socket.openSocket ( _addr, _port, _timeout ) )
            return kOpenFail;
    
    // If input command length not specified, use input string length.
    
    if ( inlen == 0 && input != nullptr )
        inlen = (int) strlen ( input );
    
    // If we are going to send a command:
    
    if ( inlen > 0 && input != nullptr )
    {
        // First clear out any bytes currently remaining to be received
        
        int bytes = 0;
        if ( udp )
        {
            SSIP sender;
            char junk = 0;
            do
            {
                bytes = _socket.readUDPSocket ( &junk, 1, sender, 1 );
#ifndef _MSC_VER    // This is an expected failure on Windows; calling recvfrom() before sendto() will not implicitly bind the socket.
                if ( bytes < 0 )
                    return kReadFail;
#endif
            }
            while ( bytes > 0 );
        }
        else
        {
            int bytes = _socket.readSocket ( nullptr, 0 );
            if ( bytes > 0 )
            {
                vector<char> junk ( bytes );
                if ( _socket.readSocket ( &junk[0], bytes ) != bytes )
                    return kReadFail;
            }
        }
        
        // Now send command input to the TCP or UDP socket
        
        if ( udp )
            bytes = _socket.writeUDPSocket ( input, inlen, _addr, _port );
        else
            bytes = _socket.writeSocket ( input, inlen );
        
        if ( bytes != inlen )
            err = kWriteFail;

        // Log what we sent, return if we failed.
        
        writeLog ( true, input, inlen, err );
        if ( err )
            return err;
    }
    
    // If we don't want output, we are done!
    
    if ( outlen < 1 || output == nullptr )
        return kSuccess;

    // Otherwise read output data from the socket, one byte at a time,
    // until we receieve a terminator character, fill the output buffer, or time out.
    
    int bytesRead = 0;
    if ( udp )
    {
        SSIP sender;
        bytesRead = _socket.readUDPSocket ( output, outlen, sender, timeout_ms );
        if ( bytesRead < 0 )
            err = kReadFail;
        else if ( bytesRead < 1 )
            err = kTimedOut;
    }
    else
    {
        double start = clocksec();
        while ( err == kSuccess )
        {
            int bytes = _socket.readSocket ( nullptr, 0 );
            if ( bytes < 0 )
            {
                err = kReadFail;
                break;
            }
            else if ( bytes < 1 )
            {
                msleep ( 1 );
                if ( clocksec() - start > timeout_ms / 1000.0 )
                    err = kTimedOut;
                continue;
            }
            
            bytes = _socket.readSocket ( output + bytesRead, 1 );
            if ( bytes < 1 )
                err = kReadFail;
            else
                bytesRead += bytes;
            if ( ( term && output[ bytesRead - 1 ] == term ) || bytesRead == outlen )
                break;
        }
    }
        
    // Log what we received, return any error code
    
    writeLog ( false, output, bytesRead, err );
    return err;
}

// Sends data (input) to telescope mount via serial port or TCP socket, and optionally waits for output.
// High-level wrapper for serialCommand() and socketCommand(); all parameters are as described for those methods.
// Returns zero if successful or nonzero error code on failure.

SSMount::Error SSMount::command ( const char *input, int inlen, char *output, int outlen, char term, int timeout_ms )
{
    Error err = kInvalidInput;
    
    if ( timeout_ms == 0 )
        timeout_ms = _timeout;
    
    for ( int i = 0; i < _retries && err != kSuccess; i++ )
    {
        if ( _serial.portOpen() )
            err = serialCommand ( input, inlen, output, outlen, term, timeout_ms );
        else if ( _socket.socketOpen() )
            err = socketCommand ( input, inlen, output, outlen, term, timeout_ms );
        else
            err = kInvalidInput;
    }
    
    return err;
}

// Sends string (instr) to telescope mount via serial port or TCP socket, and optionally waits for output string.
// High-level wrapper for serialCommand() and socketCommand(); all parameters are as described for those methods.
// Returns zero if successful or nonzero error code on failure.

SSMount::Error SSMount::command ( const string &instr, string &outstr, int outlen, char term, int timeout_ms )
{
    vector<char> output ( outlen + 1 );
    memset ( &output[0], 0, output.size() );
    Error result = command ( instr.c_str(), (int) instr.length(), &output[0], outlen, term, timeout_ms );
    outstr = string ( &output[0] );
    return result;
}

// Sends string (instr) to telescope mount and does not waits for any output.
// Returns zero if successful or nonzero error code on failure.

SSMount::Error SSMount::command ( const string &instr )
{
    return command ( instr.c_str(), (int) instr.length(), nullptr, 0, 0 );
}

// Opens communication log file at the specified path; overwrites any existing log there.
// Closes any current log before openeing new one. Returns zero if successful or error code.

SSMount::Error SSMount::openLog ( const string &path )
{
    closeLog();
    
    _logFile = fopen ( path.c_str(), "w" );
    if ( _logFile == NULL )
        return kOpenFail;

    _logStart = clocksec();
    return kSuccess;
}

// Writes data to command log file; len is number of bytes of data to write.
// If input is true, the data is command input; if false, it's output.
// The error code associated with the communication is err.
// Returns an error code or zero if successfully able to write to log file.

SSMount::Error SSMount::writeLog ( bool input, const char *data, int len, Error err )
{
    if ( _logFile == NULL )
        return kWriteFail;
    
    // write timestamp, input/output flag
    
    double timestamp = clocksec() - _logStart;
    fprintf ( _logFile, "%10.6fs: %s ", timestamp, input ? "send" : "recv" );

    if ( len > 0 )
    {
        // If we have data, write in decimal format
        
        int i;
        for ( i = 0; i < len; i++ )
            fprintf ( _logFile, "%03d ", (unsigned char) data[i] );

        // Convert data buffer to ASCII zero-terminated string

        vector<char> ascii ( len + 1 );
        for ( i = 0; i < len; i++ )
            if ( data[i] >= 32 && data[i] <= 127 )
                ascii[i] = data[i];
            else
                ascii[i] = ' '; // replace non-ASCII characters with whitespace
        ascii[i] = 0;
        
        // Write ASCII string in quotes..

        fprintf ( _logFile, "\"%s\" ", &ascii[0] );
     }
    
    // Write error code
    
    fprintf ( _logFile, err ? "error %d\n" : "success\n", err );
    fflush ( _logFile );
    return kSuccess;
}

// Closes communication log file.

void SSMount::closeLog ( void )
{
    if ( _logFile != NULL )
    {
        fclose ( _logFile );
        _logFile = NULL;
        _logStart = 0.0;
    }
}

// converts integer slew rate identifier to angular rate in radians/second.
// For SSMount base class simulated mount, uses Meade LX-200 angular rates.

SSAngle SSMount::angularRate ( int rate )
{
    SSAngle angRate;
    int absRate = abs ( rate );
    
    if ( absRate == 4 )
        angRate = SSAngle::fromDegrees ( 8.0 );
    else if ( absRate == 3 )
        angRate = SSAngle::fromDegrees ( 2.0 );
    else if ( absRate == 2 )
        angRate = SSAngle::fromArcsec ( 960.0 );    // 32x sidereal
    else if ( absRate == 1 )
        angRate = SSAngle::fromArcsec ( 30.0 );     // 2x sidereal

    if ( rate < 0 )
        angRate = -angRate;
    
    return angRate;
}

// Obtains current RA/Dec coordinates from mount in J2000 equatorial (fundamental) frame.
// Returns zero if successful or nonzero error code on failure. Also checks slew state.

SSMount::Error SSMount::read ( SSAngle &ra, SSAngle &dec )
{
    // If slewing on RA axis, compute angular rate and elapsed seconds since slew started.
    // Compute current RA, and stop slewing if we've reached the target RA.
    
    if ( _slewRate[ kAzmRAAxis ] )
    {
        double angRate = angularRate ( _slewRate[kAzmRAAxis] );
        double elapSec = clocksec() - _slewTime[ kAzmRAAxis ];
        _currLon = mod2pi ( _initLon + elapSec * angRate );
        if ( _slewing && ( elapSec > modpi ( _slewLon - _initLon ) / angRate ) )
        {
            _currLon = _slewLon;
            _slewRate[ kAzmRAAxis ] = _slewTime[ kAzmRAAxis ] = 0;
        }
    }
    
    // If slewing on Dec axis, compute angular rate and elapsed seconds since slew started.
    // Compute current Dec, and stop slewing if we've reached the target Dec.

    if ( _slewRate[ kAltDecAxis ] )
    {
        double angRate = angularRate ( _slewRate[kAltDecAxis] );
        double elapSec = clocksec() - _slewTime[ kAltDecAxis ];
        _currLat = clamp ( (double) _initLat + elapSec * angRate, -SSAngle::kHalfPi, SSAngle::kHalfPi );
        if ( _slewing && ( elapSec > ( _slewLat - _initLat ) / angRate ) )
        {
            _currLat = _slewLat;
            _slewRate[ kAltDecAxis ] = _slewTime[ kAltDecAxis ] = 0;
        }
    }

    // If slewing a GoTo, and motion on both axes has stopped, the GoTo has finished.
    
    if ( _slewing )
        if ( _slewRate[ kAzmRAAxis ] == 0 && _slewRate[ kAltDecAxis ] == 0 )
            _slewing = false;
    
    mountToFundamental ( _currLon, _currLat, ra, dec );
    return kSuccess;
};

// Start slewing to target RA/Dec coordinates (GoTo) at fastest possible rate.
// Input (ra,dec) are assumed to be in J2000 equatorial (fundamental) frame.
// Returns zero if successful or nonzero error code on failure.

SSMount::Error SSMount::slew ( SSAngle ra, SSAngle dec )
{
    _initLon = _currLon;
    _initLat = _currLat;
    fundamentalToMount ( ra, dec, _slewLon, _slewLat );

    _slewing = true;
    _slewTime[kAzmRAAxis] = _slewTime[kAltDecAxis] = clocksec();
    _slewRate[kAzmRAAxis] = modpi ( _slewLon - _initLon ) >= 0.0 ? 4 : -4;
    _slewRate[kAltDecAxis] = _slewLat >= _initLat ? 4 : -4;
    return kSuccess;
}

// Starts or stops slewing mount on an axis at a positive or negative rate,
// where rate is 1 for slowest ... maxSlewRate() for fastest slewing;
// and rate = 0 stops slewing on the specified axis.
// Returns zero if successful or nonzero error code on failure.

SSMount::Error SSMount::slew ( SSSlewAxis axis, int rate )
{
    if ( abs ( rate ) <= maxSlewRate() )
    {
        if ( rate == 0 )    // If stopping, update mount position and reset time.
        {
            SSAngle ra, dec;
            read ( ra, dec );
            fundamentalToMount ( ra, dec, _currLon, _currLat );
            _slewTime[ axis ] = 0.0;
        }
        else    // If starting, save start position and time.
        {
            _initLon = _currLon;
            _initLat = _currLat;
            _slewTime[ axis ] = clocksec();
        }

        _slewRate[ axis ] = rate;
        return kSuccess;
    }
    
    return kInvalidInput;
}

// Stops slewing, cancels any in-progress GoTo, and resumes sidereal tracking
// Returns zero if successful or nonzero error code on failure.

SSMount::Error SSMount::stop ( void )
{
    SSAngle ra, dec;
    if ( _slewing )
        read ( ra, dec );
    
    _slewRate[ kAzmRAAxis ] = _slewRate[ kAltDecAxis ] = 0;
    _slewTime[ kAzmRAAxis ] = _slewTime[ kAltDecAxis ] = 0;
    _slewing = false;
    return kSuccess;
}

// Syncs or align mount on the specified coordinates.
// Input (ra,dec) are assumed to be in J2000 equatorial (fundamental) frame.
// Returns zero if successful or nonzero error code on failure.

SSMount::Error SSMount::sync ( SSAngle ra, SSAngle dec )
{
    fundamentalToMount ( ra, dec, _currLon, _currLat );
    _aligned = true;
    return kSuccess;
}

// Queries mount to determine whether a GoTo slew is currently in progress
// Returns zero if successful or nonzero error code on failure.

SSMount::Error SSMount::slewing ( bool &status )
{
    status = _slewing;
    return kSuccess;
}

// Queries mount to determine whether initial star alignment is complete.
// Returns zero if successful or nonzero error code on failure.

SSMount::Error SSMount::aligned ( bool &status )
{
    status = _aligned;
    return kSuccess;
}

// Send local date, time, and time zone to mount. Implemented by SSMount subclasses.
// Returns zero if successful or nonzero error code on failure.

SSMount::Error SSMount::setTime ( SSTime time )
{
    return kSuccess;
}

// Send site longitude and latitude to mount. Implemented by SSMount subclasses.
// Returns zero if successful or nonzero error code on failure.

SSMount::Error SSMount::setSite ( SSSpherical site )
{
    return kSuccess;
}

// Reads local date, time, and time zone from mount. Implemented by SSMount subclasses.
// For simulated mount base class, mount time is system time!
// Returns zero if successful or nonzero error code on failure.

SSMount::Error SSMount::getTime ( SSTime &time )
{
    time = SSTime::fromSystem();
    return kSuccess;
}

// Reads local site longitude and latitude from mount. Implemented by SSMount subclasses.
// For simulated mount base class, mount site is determined from local system IP address.
// Returns zero if successful or nonzero error code on failure.

SSMount::Error SSMount::getSite ( SSSpherical &site )
{
    return SSLocationFromIP ( site ) ? kSuccess : kTimedOut;
}

// Converts spherical coordinates (ra,dec) in the fundamental (J2000 mean equatorial) frame
// to spherical coordinates (lon,lat) in the mount's reference frame, where (lon,lat) are:
// (hour angle,declination) for equatorial mounts
// (azimuth,altitude) for alt-azimuth mounts

void SSMount::fundamentalToMount ( SSAngle ra, SSAngle dec, SSAngle &lon, SSAngle &lat )
{
    lon = ra; lat = dec;
    _coords.transform ( kFundamental, isEquatorial() ? kEquatorial : kHorizon, lon, lat );
    if ( isEquatorial() )   // HA = LST - RA
        lon = mod2pi ( _coords.getLST() - lon );
}

// Converts spherical coordinates (lon,lat) in the mount's reference frame, where (lon,lat) are:
// (hour angle,declination) for equatorial mounts
// (azimuth,altitude) for alt-azimuth mounts
// to spherical coordinates (ra,dec) in the fundamental (J2000 mean equatorial) frame.

void SSMount::mountToFundamental ( SSAngle lon, SSAngle lat, SSAngle &ra, SSAngle &dec )
{
    if ( isEquatorial() )   // RA = LST - HA
        lon = mod2pi ( _coords.getLST() - lon );
    _coords.transform ( isEquatorial() ? kEquatorial : kHorizon, kFundamental, lon, lat );
    ra = lon; dec = lat;
}

// Aynchronous read command. Launches read() in a background thread,
// calls callback with error code and userData when command returns.
// Obtain updated mount RA/Dec with getRA and getDec() accessors.

void SSMount::readAsync ( AsyncCmdCallback callback, void *userData )
{
    auto func = [=] ( SSMountPtr pMount )
    {
        SSAngle ra, dec;
        pMount->lockMutex ( true );
        Error err = pMount->read ( ra, dec );
        pMount->lockMutex ( false );
        if ( callback )
            callback ( pMount, err, userData );
    };
    
    thread t = thread ( func, this );
    t.detach();
}

// Aynchronous GoTo command. Launches slew() in a background thread,
// calls callback with error code and userData when command returns.
// GoTo target ra and dec are identical to synchronous slew() inputs.

void SSMount::slewAsync ( SSAngle ra, SSAngle dec, AsyncCmdCallback callback, void *userData )
{
    auto func = [=] ( SSMountPtr pMount )
    {
        pMount->lockMutex ( true );
        Error err = pMount->slew ( ra, dec );
        pMount->lockMutex ( false );
        if ( callback )
            callback ( pMount, err, userData );
    };

    thread t = thread ( func, this );
    t.detach();
}

// Aynchronous slew command. Launches slew() in a background thread,
// calls callback with error code and userData when command returns.
// Slew axis and rate are identical to synchronous slew() inputs.

void SSMount::slewAsync ( SSSlewAxis axis, int rate, AsyncCmdCallback callback, void *userData )
{
    auto func = [=] ( SSMountPtr pMount )
    {
        pMount->lockMutex ( true );
        Error err = pMount->slew ( axis, rate );
        pMount->lockMutex ( false );
        if ( callback )
            callback ( pMount, err, userData );
    };

    thread t = thread ( func, this );
    t.detach();
}

// Aynchronous stop command. Launches stop() in a background thread,
// calls callback with error code and userData when command returns.

void SSMount::stopAsync ( AsyncCmdCallback callback, void *userData )
{
    auto func = [=] ( SSMountPtr pMount )
    {
        pMount->lockMutex ( true );
        Error err = pMount->stop();
        pMount->lockMutex ( false );
        if ( callback )
            callback ( pMount, err, userData );
    };

    thread t = thread ( func, this );
    t.detach();
}

// Aynchronous sync command. Launches sync() in a background thread,
// calls callback with error code and userData when command returns.
// Sync target ra and dec are identical to synchronous sync() inputs.

void SSMount::syncAsync ( SSAngle ra, SSAngle dec, AsyncCmdCallback callback, void *userData )
{
    auto func = [=] ( SSMountPtr pMount )
    {
        pMount->lockMutex ( true );
        Error err = pMount->sync ( ra, dec );
        pMount->lockMutex ( false );
        if ( callback )
            callback ( pMount, err, userData );
    };

    thread t = thread ( func, this );
    t.detach();
}

// Overrides and mount-specific methods for Celestron NexStar and SkyWatcher/Orion SynScan controllers.
// Based on documentation provided here: https://www.nexstarsite.com/PCControl/ProgrammingNexStar.htm
// SSCelestronMount constructor:

SSCelestronMount::SSCelestronMount ( SSMountType type, SSMountProtocol protocol, SSCoordinates &coords ) : SSMount ( type, coords )
{
    _protocol = protocol;
    _trackMode = kUnknownTracking;
}

// Queries current tracking mode from NexStar/SynScan controller.
// Returns zero if successful or nonzero error code on failure.

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

// Changes current tracking mode for NexStar/SynScan controller.
// Returns zero if successful or nonzero error code on failure.

SSMount::Error SSCelestronMount::setTrackingMode ( TrackingMode mode )
{
    char input[4] = { 0 }, output[2] = { 0 };

    input[0] = 'T';
    input[1] = mode;

    Error err = command ( input, 2, output, 1, '#' );
    return err;
}

// Opens serial or socket connection to NexStar/SynScan controller and reads controller firmware version string.
// If port is zero, path is a serial device file (like "/dev/ttyUSBserial0" on Linux or "\\.\COM3" on Windows)
// If port is nonzero, path is mount IP address or fully-qualified domain name (like "10.0.0.1" or "mount.meade.com")
// Returns zero if successful or nonzero error code on failure.

SSMount::Error SSCelestronMount::connect ( const string &path, uint16_t port )
{
    Error err = kSuccess;
    
    // Open serial communication at 9600 baud, no parity, 8 data bits, 1 stop bit
    
    err = SSMount::connect ( path, port, 9600, SSSerial::kNoParity, SSSerial::k8DataBits, SSSerial::k1StopBits );
    if ( err != kSuccess )
        return err;

    // Get hand controller firmware version.
    // SynScan controllers return a 6-character version string terminated by a '#'.
    // NexStar hand controllers return a 3-character version string with no terminator.

    char output[10] = { 0 };
    if ( _protocol == kCelestronNexStar )
        err = command ( "V#", 0, output, 3, 0 );
    else
        err = command ( "V#", 0, output, 7, '#' );

    if ( err != kSuccess )
        return err;
    
    // SynScan firmware version strings are three hex digits corresponding to decimals.
    // For example hex 042507 corresponds to 04.37.07.

    if ( _protocol == kSkyWatcherSynScan )
    {
        int ver[3] = { 0 };
        if ( sscanf ( output, "%2x%2x%2x", &ver[0], &ver[1], &ver[2] ) == 3 )
            _version = format ( "%d.%d.%d", ver[0], ver[1], ver[2] );
    }
    else
    {
        // NexStar hand controller firmware version < 4.0 MAY be a StarSense hand controller,
        // or it may not - Celestron started renumbering StarSense HC firmware versions at 1.0!
        
        _version = format ( "%d.%d", output[0], output[1] );
        if ( _version.compare ( "4.0" ) < 0 )
        {
            // If controller responds to this platform query, it's a StarSense.  We'll
            // distinguish internally by prefixing with a '1': i.e. StarSense firmware 1.1
            // becomes our 10.1, StarSense 2.4 becomes our 12.4 etc.  That works for us
            // until StarSense firmware versions reach 10.0 :-)
            
            err = command ( "v#", 0, output, 3, '#' );
            if ( err == kSuccess )
                _version = format ( "1%d.%d", output[0], output[1] );
        }
    }
    
    return kSuccess;
}

// Gets mount RA/Dec in J2000 mean equatorial (fundamental) frame from NexStar/SynScan controller.
// Returns zero if successful or nonzero error code on failure. Also checks slew state.

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
    
    // If the mount is currently slewing, check to see if the slew has stopped.
    
    if ( _slewing )
        slewing ( _slewing );
    
    _currLon = ra;
    _currLat = dec;
    return kSuccess;
}

// Starts GoTo target RA/Dec coordinates in J2000 mean equatorial (fundamental) frame.
// Returns zero if successful or nonzero error code on failure.

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
    Error err = command ( input, output, 1, '#', 5000 );
    if ( err )
        return err;
    
    // If successful, save slew target RA/Dec and set slewing state flag
    
    _slewLon = ra;
    _slewLat = dec;
    _slewing = true;
    return kSuccess;
}

// Starts or stops slewing mount on an axis at a positive or negative rate
// from zero ... maxSlewRate(). If rate is zero, stops slewing on the specified axis.
// Returns zero if successful or nonzero error code on failure.

SSMount::Error SSCelestronMount::slew ( SSSlewAxis axis, int rate )
{
    Error err = kSuccess;
    unsigned char input[8] = { 0 } , output[2] = { 0 };
    
    if ( abs ( rate ) > maxSlewRate() )
        return kInvalidInput;
    
    // If we have stopped slewing on both axes, get the current sidereal tracking mode
    
    if ( _slewRate[kAzmRAAxis] == 0 && _slewRate[kAltDecAxis] == 0 )
    {
        err = getTrackingMode ( _trackMode );
        if ( err )
            return err;
    }
            
    // Set up the input fixed-track-rate command
    
    input[0] = 'P';
    input[1] = 2;
    input[2] = axis == kAltDecAxis ? 17 : 16;
    input[3] = rate > 0 ? 36 : 37;
    input[4] = abs ( rate );
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
    
    _slewRate[ axis ] = rate;
    if ( _slewRate[kAzmRAAxis] == 0 && _slewRate[kAltDecAxis] == 0 )
        setTrackingMode ( _trackMode );
    
    return kSuccess;
}

// Stops any in-progress GoTo or slew, and resumes sidereal tracking.
// Returns zero if successful or nonzero error code on failure.

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
        slew ( kAzmRAAxis, 0 );
        slew ( kAltDecAxis, 0 );
    }
    
    _slewRate[ kAzmRAAxis ] = _slewRate[ kAltDecAxis ] = 0;
    return kSuccess;
}

// Queries status of a GoTo: true = in progress, false = not slewing.
// Returns error code or zero if successful.

SSMount::Error SSCelestronMount::slewing ( bool &status )
{
    char output[2] = { 0 };
    Error err = command ( "L", 1, output, 2, '#' );
    if ( err )
        return err;
    
    _slewing = status = output[0] == '1';  // response is ASCII '1' or '0'
    return kSuccess;
}

// Queries mount alignment status: true = aligned, false = not aligned.
// Returns error code or zero if successful.

SSMount::Error SSCelestronMount::aligned ( bool &status )
{
    char output[2] = { 0 };
    Error err = command ( "J", 1, output, 2, '#' );
    if ( err )
        return err;
    
    status = output[0] == 1;    // response is binary 1 or 0
    return kSuccess;
}

// Syncs mount on the specified RA/Dec in J2000 mean equatorial (fundamental) coordinates.
// Returns zero if successful or nonzero error code on failure.

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
    Error err = command ( input, output, 1, '#' );
    
    return err;
}

// Sends date, time, and time zone from SSTime object to NexStar/SynScan controller.
// Returns zero if successful or error code on failure.

SSMount::Error SSCelestronMount::setTime ( SSTime time )
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

// Send site longitude and latitude from SSSpherical object to .
// Returns zero if successful or error code on failure.

SSMount::Error SSCelestronMount::setSite ( SSSpherical site )
{
    // Compute latitude, longitude degrees/minutes/seconds

    SSDegMinSec lat ( site.lat );
    SSDegMinSec lon ( site.lon );
    
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

// Reads local date, time, and time zone from NexStar/SynScan controller to SSTime object.
// Returns zero if successful or nonzero error code on failure.

SSMount::Error SSCelestronMount::getTime ( SSTime &time )
{
    char output[10] = { 0 };
    Error err = command ( "h", 0, output, 10, '#' );
    if ( err )
        return err;

    SSDate date ( kGregorian, output[6] + output[7], output[5] + 2000, output[3], output[4], output[0], output[1], output[2] );
    time = SSTime ( date );
    return kSuccess;
}

// Reads local site longitude and latitude from NexStar/SynScan controller to SSpherical object.
// Returns zero if successful or nonzero error code on failure.

SSMount::Error SSCelestronMount::getSite ( SSSpherical &site )
{
    char output[10] = { 0 };
    Error err = command ( "w", 0, output, 10, '#' );
    if ( err )
        return err;

    SSDegMinSec lon ( output[7] ? '-' : '+', output[4], output[5], output[6] );
    SSDegMinSec lat ( output[3] ? '-' : '+', output[0], output[1], output[2] );
    
    site = SSSpherical ( SSAngle ( lon ), SSAngle ( lat ) );
    return kSuccess;
}

// Overrides and mount-specific methods for Meade LX-200 and Autostar/ETX controllers
// Based on documentation provided here: http://www.weasner.com/etx/autostar/2010/AutostarSerialProtocol2007oct.pdf
// and here: https://www.astro.louisville.edu/software/xmtel/archive/xmtel-indi-6.0/xmtel-6.0l/support/lx200/CommandSet.html
// and here: https://astro-physics.info/tech_support/mounts/protocol-cp3-cp4.pdf
// and here: https://www.ioptron.com/v/ASCOM/RS-232_Command_Language2014V310.pdf

SSMeadeMount::SSMeadeMount ( SSMountType type, SSMountProtocol protocol, SSCoordinates &coords ) : SSMount ( type, coords )
{
    _protocol = protocol;
}

// Opens serial or socket connection to Meade mount and reads mount controller firmware version string.
// If port is zero, path is a serial device file (like "/dev/ttyUSBserial0" on Linux or "\\.\COM3" on Windows)
// If port is nonzero, path is mount IP address or fully-qualified domain name (like "10.0.0.1" or "mount.meade.com")
// Returns zero if successful or nonzero error code on failure.

SSMount::Error SSMeadeMount::connect ( const string &path, uint16_t port )
{
    // Open serial communication at 9600 baud, no parity, 8 data bits, 1 stop bit
    
    Error err = SSMount::connect ( path, port, 9600, SSSerial::kNoParity, SSSerial::k8DataBits, SSSerial::k1StopBits );
    if ( err != kSuccess )
        return err;

    // Get firmware version number and erase any terminating '#' character
    
    err = command ( ":GVN#", _version, 32, '#' );
    if ( err )
        return err;
    
    if ( _version.back() == '#' )
        _version.pop_back();
    
    // Get declination.

    string output;
    err = command ( ":GD#", output, 11, '#' );
    if ( err )
        return err;
    
    // If output string length indicates low-precision format, toggle to high precision.
    
    if ( output.length() < 10 )
    {
        err = command ( ":U#", 0, nullptr, 0, 0 );
        if ( err )
            return err;
    }
    
    return kSuccess;
}

// Gets mount RA/Dec in J2000 mean equatorial (fundamental) frame from NexStar/SynScan controller.
// Returns zero if successful or nonzero error code on failure.

SSMount::Error SSMeadeMount::read ( SSAngle &ra, SSAngle &dec )
{
    string output;
    
    // Get right ascension
    
    Error err = command ( ":GR#", output, 10, '#' );
    if ( err )
        return err;
    
    if ( output.length() < 9 )
        return kInvalidOutput;
    
    output[2] = output[5] = ' ';
    SSHourMinSec hms = SSHourMinSec ( output );
    
    // Get declination
    
    err = command ( ":GD#", output, 11, '#' );
    if ( err )
        return err;
    
    if ( output.length() < 10 )
        return kInvalidOutput;
    
    output[3] = output[6] = ' ';
    SSDegMinSec dms = SSDegMinSec ( output );
    
    // Convert results to radians, precess to J2000
    
    ra = SSAngle ( hms );
    dec = SSAngle ( dms );
    _coords.transform ( kEquatorial, kFundamental, ra, dec );
    _currLon = ra;
    _currLat = dec;
    
    // If slewing in progress, check to see if the slew has stopped
    
    if ( _slewing )
        slewing ( _slewing );
    
    return kSuccess;
}

SSMount::Error SSMeadeMount::setTargetRADec ( SSAngle ra, SSAngle dec )
{
    _coords.transform ( kFundamental, kEquatorial, ra, dec );
    
    SSHourMinSec hms ( ra );
    SSDegMinSec dms ( dec );
    
    // Set slew target right ascension in high-precision format
    
    string output, input = format ( ":Sr%02hd:%02hd:%02.0f#", hms.hour, hms.min, min ( hms.sec, 59.0 ) );
    Error err = command ( input, output, 1, '#' );
    if ( err )
        return err;
    
    if ( output[0] != '1' )
        return kInvalidCoords;
    
    // Set slew target declination in high-precision format

    input = format ( ":Sd%c%02hd:%02hd:%02.0f#", dms.sign, dms.deg, dms.min, min ( dms.sec, 59.0 ) );
    err = command ( input, output, 1, '#' );
    if ( err )
        return err;
    
    if ( output[0] != '1' )
        return kInvalidCoords;

    return kSuccess;
}

// Starts GoTo target RA/Dec coordinates in J2000 mean equatorial (fundamental) frame.
// Returns zero if successful or nonzero error code on failure.

SSMount::Error SSMeadeMount::slew ( SSAngle ra, SSAngle dec )
{
    // Send slew target coordinates to mount
    
    Error err = setTargetRADec ( ra, dec );
    if ( err )
        return err;
    
    // Slew to target coordindates
    
    string output;
    err = command ( ":MS#", output, 1, '#' );
    if ( err )
        return err;
    
    if ( output[0] != '0' )
        return kInvalidCoords;
    
    // If successful, save goto target RA/Dec and set slewing state flag.
    
    _slewLon = ra;
    _slewLat = dec;
    _slewing = true;
    return kSuccess;
}

// Stops any in-progress GoTo or slew, and resumes sidereal tracking.
// Returns zero if successful or nonzero error code on failure.

SSMount::Error SSMeadeMount::stop ( void )
{
    return command ( ":Q#", 0, nullptr, 0, 0 );
}

// Syncs mount on the specified RA/Dec in J2000 mean equatorial (fundamental) coordinates.
// Returns zero if successful or nonzero error code on failure.

SSMount::Error SSMeadeMount::sync ( SSAngle ra, SSAngle dec )
{
    // Send slew target coordinates to mount
    
    Error err = setTargetRADec ( ra, dec );
    if ( err )
        return err;

    // Align on target coordindates
    
    string output;
    err = command ( ":CM#", output, 255, '#' );
    return err;
}

// Starts or stops slewing mount on an axis at a positive or negative rate
// from zero ... maxSlewRate(). If rate is zero, stops slewing on the specified axis.
// Returns zero if successful or nonzero error code on failure.

SSMount::Error SSMeadeMount::slew ( SSSlewAxis axis, int rate )
{
    Error err = kSuccess;
    bool swapEW = false;
    
    // Set motion rate for subsequent move commands
    
    if ( rate != 0 )
    {
        err = setSlewRate ( abs ( rate ) );
        if ( err != kSuccess )
            return err;
    }
    
    // LX-200 GPS and LX-600 scopes have a firmware bug which causes their east-west motion
    // to be reversed when they are on Alt-Azimuth mounts

    if ( axis == kAzmRAAxis )
    {
        if ( rate > 0 )
            err = command ( swapEW ? ":Mw#" : ":Me#" );
        else if ( rate < 0 )
            err = command ( swapEW ? ":Me#" : ":Mw#" );
        else if ( rate == 0 )
            err = command ( _slewRate[axis] > 0 ? ":Qe#" : ":Qw#" );
    }
    else if ( axis == kAltDecAxis )
    {
        if ( rate > 0 )
            err = command ( ":Mn#" );
        else if ( rate < 0 )
            err = command ( ":Ms#" );
        else if ( rate == 0 )
            err = command ( _slewRate[axis] > 0 ? ":Qn#" : ":Qs#" );
    }
    
    if ( err )
        return err;
    
    _slewRate[ axis ] = rate;
    return kSuccess;
}

// This Meade-specific command sets rate for directional slewing.
// Note setting rate to zero does nothing!

SSMount::Error SSMeadeMount::setSlewRate ( int rate )
{
    Error err = kSuccess;
    
    if ( rate > maxSlewRate() )
        return kInvalidInput;
    
    if ( _protocol == kMeadeAutostar )
    {
        string output;
        
        if ( rate == 1 )
            err = command ( ":Sw2#", output, 1, 0 );
        else if ( rate == 2 )
            err = command ( ":Sw3#", output, 1, 0 );
        else if ( rate == 3 )
            err = command ( ":Sw4#", output, 1, 0 );
        
        if ( err == kSuccess && ( output.length() < 1 || output[0] != '1' ) )
            err = kInvalidOutput;
    }
    else
    {
        if ( rate == 1 )
            err = command ( ":RG#" );
        else if ( rate == 2 )
            err = command ( ":RC#" );
        else if ( rate == 3 )
            err = command ( ":RM#" );
        else if ( rate == 4 )
            err = command ( ":RS#" );
    }
    
    return err;
}

// Send site longitude and latitude from SSSpherical object to Meade mount.
// Returns zero if successful or error code on failure.

SSMount::Error SSMeadeMount::setSite ( SSSpherical site )
{
    // Meade considers east longitude to be negative, and does not accept a sign.
    // So, send longitude from 0 - 360 degrees and force positive.

    SSDegMinSec lon ( SSAngle ( mod2pi ( -site.lon ) ) );
    string input = format ( ":Sg%03hd*%02hd#", lon.deg, lon.min ), output;
    Error err = command ( input, output, 1, 0 );
    if ( err )
        return err;
    
    if ( output.length() < 1 || output[0] != '1' )
        return kInvalidOutput;

    // Send latitude.

    SSDegMinSec lat ( site.lat );
    input = format ( ":St%c%02hd*%02hd#", lat.sign, lat.deg, lat.min );
    err = command ( input, output, 1, 0 );
    if ( err )
        return err;
    
    // Autostar #497 controllers always return '0', so ignore output.
    // Otherwise '0' indicates failure and '1' indicates success.
    
    if ( _protocol != kMeadeAutostar )
        if ( output.length() < 1 || output[0] != '1' )
            return kInvalidOutput;

    return kSuccess;
}

// Sends date, time, and time zone from SSTime object to Meade mount.
// Returns zero if successful or error code on failure.

SSMount::Error SSMeadeMount::setTime ( SSTime time )
{
    // Send time zone in hours west of UTC
    
    string input = format ( ":SG%+03.0f#", -time.zone ), output;
    Error err = command ( input, output, 1, 0 );
    if ( err )
        return err;
    
    if ( output.length() < 1 || output[0] != '1' )
        return kInvalidOutput;
    
    // Send local time
    
    SSDate date ( time );
    input = format ( ":SL%02hd:%02hd:%02.0f#", date.hour, date.min, min ( date.sec, 59.0 ) );
    err = command ( input, output, 1, 0 );
    if ( err )
        return err;
    
    if ( output.length() < 1 || output[0] != '1' )
        return kInvalidOutput;
    
    // Send local date. If the date is valid two strings are returned, each 31 bytes long.
    // The first is: "Updating planetary data#" followed by a second string of 30 spaces terminated by ’#’
    // The string outputs follow almost immediately from the ETX, but after a long pause on the LX-200.
    
    input = format ( ":SC%02hd/%02.0f/%02d#", date.month, floor ( date.day ), date.year % 100 );
    err = command ( input, output, 33, '#', 10000 );
    if ( err )
        return err;
    
    char junk[33] = { 0 };
    command ( nullptr, 0, junk, 33, '#', 10000 );

    if ( output.length() < 1 || output[0] != '1' )
        return kInvalidOutput;

    return kSuccess;
}

// Reads local date, time, and time zone from LX-200/Autostar controller to SSpherical object.
// Returns zero if successful or nonzero error code on failure.

SSMount::Error SSMeadeMount::getTime ( SSTime &time )
{
    SSDate date;

    // Read time zone.
    
    string output;
    Error err = command ( ":GG#", output, 7, '#' );
    if ( err )
        return err;

    date.zone = -strtofloat64 ( output );
    
    // Read local time
    
    err = command ( ":GL#", output, 10, '#' );
    if ( err )
        return err;
    
    if ( sscanf ( output.c_str(), "%hd:%hd:%lf", &date.hour, &date.min, &date.sec ) < 3 )
        return kInvalidOutput;
    
    // Read current local calendar date
    
    err = command ( ":GC#", output, 10, '#' );
    if ( err )
        return err;
    
    if ( sscanf ( output.c_str(), "%hd/%hd/%d", &date.month, &date.day, &date.year ) < 3 )
        return kInvalidOutput;

    // Convert to SSTime and return successful result code
    
    date.year += 2000;
    time = SSTime ( date );
    return kSuccess;
}

// Reads local site longitude and latitude from LX-200/Autostar controller to SSpherical object.
// Returns zero if successful or nonzero error code on failure.

SSMount::Error SSMeadeMount::getSite ( SSSpherical &site )
{
    // Read latitude
    
    string output;
    Error err = command ( ":Gt#", output, 9, '#' );
    if ( err )
        return err;
    
    output[3] = ' ';
    SSDegMinSec lat ( output );
    
    // Read longitude. Note Meade considers east positive!
    
    err = command ( ":Gg#", output, 10, '#' );
    if ( err )
        return err;
    
    output[4] = ' ';
    SSDegMinSec lon ( output );
    
    // Convert to SSSpherical and return successful result code

    site = SSSpherical ( -SSAngle ( lon ), SSAngle ( lat ) );
    return kSuccess;
}

// Queries status of a GoTo: true = in progress, false = not slewing.
// For LX-200 mounts, assumes read() has been called previously; see below.
// Returns error code or zero if successful.

SSMount::Error SSMeadeMount::slewing ( bool &status )
{
    Error err = kSuccess;

    if ( _protocol == kMeadeAutostar )
    {
        // Request a string of "bar" chars (0x7f) indicating the distance to the current target object.

        string output;
        err = command ( ":D#", output, 255, '#' );
        if ( err )
            return err;
        
        // Autostar returns a single 0x7f if continuing to slew and an empty string otherwise.
        
        _slewing = status = output.length() > 1 && output[0] == 0x7f;
    }
    else
    {
        // The ":D#" commmand is not supported on many LX-200 clones (Losmany Gemini, AstroPhysics GTO)
        // so test whether a GoTo is in progress by comparing mount's current RA/Dec to target RA/Dec.
        // This assumes current RA/Dec has previously been read with a read() command!
        
        if ( _slewing )
        {
            SSAngle sep = SSSpherical ( _currLon, _currLat ).angularSeparation ( SSSpherical ( _slewLon, _slewLat ) );
            if ( sep < SSAngle::fromDegrees ( 1.0 ) )
                _slewing = false;
        }
    }
    
    status = _slewing;
    return err;
}

// Queries mount alignment status: true = aligned, false = not aligned.
// Returns error code or zero if successful. This probably does not work as intended!

SSMount::Error SSMeadeMount::aligned ( bool &status )
{
    // ACK <0x06> is query of alignment mounting mode
    
    char input = 0x06, output;
    Error err = command ( &input, 1, &output, 1, 0 );
    if ( err )
        return err;
    
    status = output == 'A' || output == 'G' || output == 'L' || output == 'P';
    return kSuccess;
}

// Overrides and mount-specific methods for Synta (SkyWatcher/Orion) direct motor controllers.
// See https://inter-static.skywatcher.com/downloads/skywatcher_motor_controller_command_set.pdf
// and sample code here: https://github.com/skywatcher-pacific/skywatcher_open
// and here: https://github.com/liwn/Skywatcher
// SSSyntaMount constructor:

SSSyntaMount::SSSyntaMount ( SSMountType type, SSCoordinates &coords ) : SSMount ( type, coords )
{
    _protocol = kSyntaDirect;
    _countsPerRev[kAzmRAAxis] = _countsPerRev[kAltDecAxis] = 0;
    _breakSteps[kAzmRAAxis] = _breakSteps[kAltDecAxis] = 3500;
    _aligned = false;
}

// Sends 1-byte Synta motor command and listens for response.
// For RA/Azm motor, axis = 0. For Dec/Alt motor, axis = 1.
// Payload data associated with command may be zero-length.
// Returns error code or zero if successful.

SSMount::Error SSSyntaMount::motorCommand ( int axis, char cmd, string indata, string &outdata )
{
    // If we have input data, swap byte order
    
    if ( indata.length() == 6 )
    {
        swap ( indata[0], indata[4] );
        swap ( indata[1], indata[5] );
    }
    else if ( indata.length() == 4 )
    {
        swap ( indata[0], indata[2] );
        swap ( indata[1], indata[3] );
    }

    // format and send command, listen for response, return if error.
    // Note mount expects RA/Azm axis = 1 and Alt/Dec axis = 2!
    // On most Alt/Az mounts, serial TX and RX lines are connected together
    // so the command will be echoed before the response.
    // If we see this, keep reading to get the real response.
    
    string output, input = format ( ":%c%d%s\r", cmd, axis + 1, indata.c_str() );
    Error err = command ( input, output, 10, '\r' );
    if ( err == kSuccess && output.compare ( input ) == 0 )
        err = command ( "", output, 10, '\r' );
    if ( err != kSuccess )
        return err;

    // Ensure response indicates success
    
    if ( output.length() < 2 || output[0] != '=' || output.back() != '\r' )
        return kInvalidOutput;
    
    // Extact paylod data from output string and byte-swap.
    
    outdata = output.substr ( 1, output.length() - 2 );
    if ( outdata.length() == 6 )
    {
        swap ( outdata[0], outdata[4] );
        swap ( outdata[1], outdata[5] );
    }
    else if ( outdata.length() == 4 )
    {
        swap ( outdata[0], outdata[2] );
        swap ( outdata[1], outdata[3] );
    }
    
    return kSuccess;
}

// Opens serial or socket connection to Synta direct motor controller and reads controller firmware version string.
// If port is zero, path is a serial device file (like "/dev/ttyUSBserial0" on Linux or "\\.\COM3" on Windows)
// If port is nonzero, path is mount IP address or fully-qualified domain name (like "192.168.4.1")
// For SynScan Wi-Fi Adapter in Access Point mode, default IP address is 192.168.4.1, UDP port 11880.
// Returns zero if successful or nonzero error code on failure.

SSMount::Error SSSyntaMount::connect ( const string &path, uint16_t port )
{
    Error err = kSuccess;
    
    // Open serial communication at 9600 baud, no parity, 8 data bits, 1 stop bit
    // Socket communication on port 11880 is assumed to be UDP (SynScan Wi-Fi)
    
    err = SSMount::connect ( path, port, 9600, SSSerial::kNoParity, SSSerial::k8DataBits, SSSerial::k1StopBits, port == 11880 );
    if ( err != kSuccess )
        return err;

    // Get motor board version on both axes
    
    string data;
    for ( int axis = kAzmRAAxis; axis <= kAltDecAxis; axis++ )
    {
        err = motorCommand ( axis, 'e', "", data );
        if ( err != kSuccess )
            return err;
        int tmpMCVersion = 0;
        if ( sscanf ( data.c_str(), "%x", &tmpMCVersion ) < 1 )
            return kInvalidOutput;
        _mcVersion[axis] = ( ( tmpMCVersion & 0xFF ) << 16 ) | ( ( tmpMCVersion & 0xFF00 ) ) | ( ( tmpMCVersion & 0xFF0000) >> 16 );
        _version += data;
    }

    // Get counts per revolution on both axes.

    for ( int axis = kAzmRAAxis; axis <= kAltDecAxis; axis++ )
    {
        err = motorCommand ( axis, 'a', "", data );
        if ( err != kSuccess )
            return err;
        if ( sscanf ( data.c_str(), "%x", &_countsPerRev[axis] ) < 1 )
            return kInvalidOutput;
    }

    // Inquire high speed ratio on both axes.

    for ( int axis = kAzmRAAxis; axis <= kAltDecAxis; axis++ )
    {
        err = motorCommand ( axis, 'g', "", data );
        if ( err != kSuccess )
            return err;
        if ( sscanf ( data.c_str(), "%x", &_highSpeedRatio[axis] ) < 1 )
            return kInvalidOutput;
    }

    // Inquire stepper timer interrupt frequency on both axes.

    for ( int axis = kAzmRAAxis; axis <= kAltDecAxis; axis++ )
    {
        err = motorCommand ( axis, 'b', "", data );
        if ( err != kSuccess )
            return err;
        if ( sscanf ( data.c_str(), "%x", &_stepTimerFreq[axis] ) < 1 )
            return kInvalidOutput;
    }
    
    // Finish initialization.

    for ( int axis = kAzmRAAxis; axis <= kAltDecAxis; axis++ )
    {
        err = motorCommand ( axis, 'F', "", data );
        if ( err != kSuccess )
            return err;
    }

    return kSuccess;
}

SSMount::Error SSSyntaMount::mcAxisStop ( int axis, bool instant )
{
    string data;
    Error err = motorCommand ( axis, instant ? 'L' : 'K', "", data );
    return err;
}

static double MAX_SPEED = degtorad ( 3.4 );
static double SIDEREALRATE = SSAngle::kTwoPi / SSTime::kSecondsPerDay / SSTime::kSiderealPerSolarDays;
static double LOW_SPEED_MARGIN = 128.0 * SIDEREALRATE;

SSMount::Error SSSyntaMount::mcAxisSlew ( int axis, double speed )
{
    // 3.4 degrees/sec, 800X sidereal rate, is the highest speed.

    speed = clamp ( speed, -MAX_SPEED, MAX_SPEED );
    double internalSpeed = speed;

    // InternalSpeed lower than 1/1000 of sidereal rate?
    
    if ( fabs ( internalSpeed ) <= SIDEREALRATE / 1000.0 )
        return mcAxisStop ( axis, false );

    // Stop motor and set motion mode if necessary.
    
    bool forward = internalSpeed > 0.0;
    internalSpeed = fabs ( internalSpeed );

    // High speed adjustment
    
    bool highspeed = internalSpeed > LOW_SPEED_MARGIN;
    if ( highspeed )
        internalSpeed = internalSpeed / (double) _highSpeedRatio[axis];

    // prepare for slewing
    
    AxisStatus status;
    Error err = mcGetAxisStatus ( axis, status );
    if ( err )
        return err;
    
    if ( ! status.fullStop )
    {
        // We need to stop the motor first if...
        
        if ( status.slewingTo || status.highSpeed || highspeed  // GOTO in action, currently high speed slewing, or will be high speed slewing
        || ( status.slewingForward != forward ) )               // Different direction
            mcAxisStop ( axis, false );

        while ( true )  // Wait until the axis stop
        {
            err = mcGetAxisStatus ( axis, status );
            if ( err != kSuccess )
                return err;

            if ( status.fullStop )
                break;

            msleep ( 100 );
        }
    }
    
    // Set motion mode
    
    string outdata;
    err = motorCommand ( axis, 'G', format ( "%d%d", highspeed ? 3 : 1, forward ? 0 : 1 ), outdata );
    if ( err )
        return err;
    
    // Calculate and set step period.
    
    int speedInt = radSpeedToInt ( axis, internalSpeed );
    int version = _mcVersion[axis];
    if ( version == 0x010600 || version == 0x010601 ) // For special MC version.
        speedInt -= 3;
    
    if ( speedInt < 6 )
        speedInt = 6;

    string data = format ( "%06X", speedInt );
    err = motorCommand ( axis, 'I', data, outdata );
    if ( err )
        return err;
    
    // Start motion
    
    err = motorCommand ( axis, 'J', "", data );
    return err;
}

SSMount::Error SSSyntaMount::mcAxisSlewTo ( int axis, double targetPosition )
{
    // Get current position of the axis.
    
    SSAngle curPosition = 0.0;
    Error err = mcGetAxisPosition ( axis, curPosition );
    if ( err )
        return err;
    
    // Calculate slewing distance. Reduce to range [-pi ... +pi].
    // TODO: For EQ mount, Positions[AXIS1] is offset( -PI/2 ) adjusted in UpdateAxisPosition().
    
    double angle = modpi ( targetPosition - curPosition );
    
    // Convert distance in radian into steps.
    // if there is no increment, return directly.
    
    int steps = angleToStep ( axis, angle );
    if ( steps == 0 )
        return kSuccess;

    // Set moving direction
    
    bool forward = steps > 0;
    steps = abs ( steps );
    
    // TODO: Might need to check whether motor has stopped.

    // Check if the distance is long enough to trigger a high speed GOTO.
    // LowSpeedGotoMargin is calculated from slewing for 5 seconds at 128x sidereal rate
    
    int lowSpeedGotoMargin = angleToStep ( axis, 640 * SIDEREALRATE );
    bool highspeed = steps > lowSpeedGotoMargin;
    
    // Set motion mode
    
    string outdata;
    err = motorCommand ( axis, 'G', format ( "%d%d", highspeed ? 0 : 2, forward ? 0 : 1 ), outdata );
    if ( err )
        return err;

    // Set GoTo target increment
    
    err = motorCommand ( axis, 'H', format ( "%06X", steps ), outdata );
    if ( err )
        return err;
    
    // Set break point increment
    
    err = motorCommand ( axis, 'M', format ( "%06X", _breakSteps[axis] ), outdata );
    if ( err )
        return err;
    
    // Start motion
    
    err = motorCommand ( axis, 'J', "", outdata );
    return err;
}

SSMount::Error SSSyntaMount::mcGetAxisStatus ( int axis, AxisStatus &status )
{
    string response;
    Error err = motorCommand ( axis, 'f', "", response );
    if ( err != kSuccess )
        return err;
    
    if ( response.length() < 3 )
        return kInvalidOutput;
    
    status.fullStop = response[1] & 0x01 ? false : true;
    status.slewing = ! status.fullStop && ( response[0] & 0x01 ? true : false );
    status.slewingTo = ! status.fullStop && ! status.slewing;
    status.slewingForward = response[0] & 0x02 ? false : true;
    status.highSpeed = response[0] & 0x04 ? true : false;
    status.notInitialized = response[2] & 1 ? false : true;
    
    return kSuccess;
}

SSMount::Error SSSyntaMount::mcGetAxisPosition ( int axis, SSAngle &rad )
{
    string response;
    Error err = motorCommand ( axis, 'j', "", response ) ;
    if ( err != kSuccess)
        return err;
    
    int iPosition = 0;
    if ( sscanf ( response.c_str(), "%x", &iPosition ) < 1 )
        return kInvalidOutput;
    
    iPosition -= 0x00800000;
    rad = stepToAngle ( axis, iPosition ); // SSAngle::kTwoPi * iPosition / _countsPerRev[axis];
    return kSuccess;
}

SSMount::Error SSSyntaMount::mcSetAxisPosition ( int axis, SSAngle rad )
{
    string response;
    int iPosition = angleToStep ( axis, rad ) + 0x00800000;
    Error err = motorCommand ( axis, 'E', format ( "%06X", iPosition ), response );
    return err;
}

// Gets mount RA/Dec in J2000 mean equatorial (fundamental) frame from Synta motor controller.
// Assumes mount is perfectly polar-aligned if equatorial, or perfectly level if altazimuth!
// Returns zero if successful or nonzero error code on failure. Also checks slew state.

SSMount::Error SSSyntaMount::read ( SSAngle &ra, SSAngle &dec )
{
    Error err = mcGetAxisPosition ( kAzmRAAxis, _currLon );
    if ( err == kSuccess )
        err = mcGetAxisPosition ( kAltDecAxis, _currLat );
    if ( err )
        return err;
    
    // Convert from mount frame of reference to fundamental RA/Dec.
    // Really we should use a mount model. TBD.
    
    mountToFundamental ( _currLon, _currLat, ra, dec );
    
    // If slewing in progress, check to see if the slew has stopped
    
    if ( _slewing )
        slewing ( _slewing );
    
    return kSuccess;
}

// Starts or stops slewing mount on an axis at a positive or negative rate
// from zero ... maxSlewRate(). If rate is zero, stops slewing on the specified axis.
// Returns zero if successful or nonzero error code on failure.

SSMount::Error SSSyntaMount::slew ( SSSlewAxis axis, int rate )
{
    double speed = 0.0;
    
    // We support four rates, emulating Meade LX-200 protocol
    
    int absrate = abs ( rate );
    if ( absrate > maxSlewRate() )
        return kInvalidInput;
    
    // Convert rate to speed in radians per second
    
    if ( absrate == 4 )
        speed = MAX_SPEED;
    else if ( absrate == 3 )
        speed = MAX_SPEED / 3;
    else if ( absrate == 2 )
        speed = 32 * SIDEREALRATE;
    else if ( absrate == 1 )
        speed = 2 * SIDEREALRATE;
    
    if ( rate < 0 )
        speed = -speed;
    
    return mcAxisSlew ( axis, speed );
}

// Stops any in-progress GoTo or slew, and resumes sidereal tracking.
// Returns zero if successful or nonzero error code on failure.

SSMount::Error SSSyntaMount::stop ( void )
{
    Error err = mcAxisStop ( kAzmRAAxis, true );
    if ( err == kSuccess )
        err = mcAxisStop ( kAltDecAxis, true );
    if ( err == kSuccess )
        _slewing = false;
    return err;
}

// Starts SlewTo target RA/Dec coordinates in J2000 mean equatorial (fundamental) frame.
// Assumes mount is perfectly polar-aligned if equatorial, or perfectly level if altazimuth!
// Returns zero if successful or nonzero error code on failure.

SSMount::Error SSSyntaMount::slew ( SSAngle ra, SSAngle dec )
{
    // Convert from fundamental RA/Dec to mount frame of reference.
    // Really we should use a mount model. TBD.

    SSAngle lon, lat;
    fundamentalToMount ( ra, dec, lon, lat );
    
    Error err = mcAxisSlewTo ( kAzmRAAxis, lon );
    if ( err == kSuccess )
        err = mcAxisSlewTo ( kAltDecAxis, lat );

    if ( err == kSuccess )
    {
        _slewing = true;
        _slewLon = lon;
        _slewLat = lat;
    }
    
    return err;
}

// Syncs mount on the specified RA/Dec in J2000 mean equatorial (fundamental) coordinates.
// Assumes mount is perfectly polar-aligned if equatorial, or perfectly level if altazimuth!
// Returns zero if successful or nonzero error code on failure.

SSMount::Error SSSyntaMount::sync ( SSAngle ra, SSAngle dec )
{
    // Convert from fundamental RA/Dec to mount frame of reference.
    // Really we should use a mount model. TBD.

    fundamentalToMount ( ra, dec, _currLon, _currLat );

    Error err = mcSetAxisPosition ( kAzmRAAxis, _currLon );
    if ( err == kSuccess )
        err = mcSetAxisPosition ( kAltDecAxis, _currLat );

    if ( err == kSuccess )
        _aligned = true;

    return err;
}

// Queries status of a GoTo: true = in progress, false = not slewing.
// Returns error code or zero if successful.

SSMount::Error SSSyntaMount::slewing ( bool &status )
{
    AxisStatus axstat1, axstat2;
    
    Error err = mcGetAxisStatus ( kAzmRAAxis, axstat1 );
    if ( err == kSuccess )
        err = mcGetAxisStatus ( kAltDecAxis, axstat2 );
    if ( err )
        return err;
    
    status = axstat1.slewingTo || axstat2.slewingTo;
    return err;
}

// Queries mount alignment status: true = aligned, false = not aligned.
// Returns error code or zero if successful.

SSMount::Error SSSyntaMount::aligned ( bool &status )
{
    status = _aligned;
    return kSuccess;
}

// Overrides and mount-specific methods for Celestron AUX mount motor controllers.
// SSCelestronAUXMount constructor:

SSCelestronAUXMount::SSCelestronAUXMount ( SSMountType type, SSCoordinates &coords ) : SSMount ( type, coords )
{
    _protocol = kCelestronAUX;
    _model = SSMountModel ( M_2PI, M_2PI );
    _aligned = false;
    
    memset ( _sendBuff, 0, sizeof ( _sendBuff ) );
    memset ( _recvBuff, 0, sizeof ( _recvBuff ) );
}

// Sends a Celestron AUX bus packet containing a command to a specific device.
// See http://www.paquettefamily.ca/nexstar/NexStar_AUX_Commands_10.pdf
// The source device (src) identifies which AUX bus device is sending the packet.
// The destination device (dst) identifies which AUX bus device should recieve the packet.
// The command identifier (cmd) is specific to the destination device.
// Any command-specific parameters (data) should be provided in the supplied buffer.
// The data buffer length (len) is the number of bytes of command-specific data.
// This method returns kSuccess or a non-zero error code on failure.

SSMount::Error SSCelestronAUXMount::sendAUXPacket ( uint8_t cmd, uint8_t len, uint8_t *data, uint8_t src, uint8_t dst )
{
    memset ( _sendBuff, 0, sizeof ( _sendBuff ) );
    
    _sendBuff[0] = 0x3b;
    _sendBuff[1] = len + 3;
    _sendBuff[2] = src;
    _sendBuff[3] = dst;
    _sendBuff[4] = cmd;

    if ( data != nullptr && len > 0 )
        memcpy ( &_sendBuff[5], data, len );
    
    char checksum = 0;
    for ( int i = 1; i < len + 5; i++ )
        checksum += _sendBuff[i];
    
    _sendBuff[len + 5] = -checksum;
    return command ( _sendBuff, len + 6, NULL, 0, 0 );
}

// Receives a Celestron AUX bus packet in reply to a previously-send command packet.
// See http://www.paquettefamily.ca/nexstar/NexStar_AUX_Commands_10.pdf
// The source device (src) identifies the AUX bus sending the response packet.
// The destination device (dst) identifies the AUX bus that originally sent the packet.
// The command identifier (cmd) echoes the command originally sent to the device.
// Any reply-specific parameters (data) will be copied to this buffer, which must be able to receive up to 255 bytes.
// The number of bytes actually copied to the data buffer (len) may be from 0 to 255.
// This method returns kSuccess or a non-zero error code on failure.

SSMount::Error SSCelestronAUXMount::recvAUXPacket ( uint8_t &cmd, uint8_t &len, uint8_t *data, uint8_t &src, uint8_t &dst )
{
    memset ( _recvBuff, 0, sizeof ( _sendBuff ) );
    Error err = command ( NULL, 0, _recvBuff, 5, 0 );
    if ( err != kSuccess )
        return err;
    
    if ( _recvBuff[0] != 0x3b )
        return kInvalidOutput;
    
    len = _recvBuff[1] - 3;
    src = _recvBuff[2];
    dst = _recvBuff[3];
    cmd = _recvBuff[4];
    
    err = command ( NULL, 0, &_recvBuff[5], len + 1, 0 );
    if ( err != kSuccess )
        return err;
    
    if ( data != nullptr && len > 0 )
        memcpy ( data, &_recvBuff[5], len );
    
    char checksum = 0;
    for ( int i = 1; i < len + 5; i++ )
        checksum += _recvBuff[i];

    // negate checkum first, so comparison works correctly if char type is unsigned!
    
    checksum = -checksum;
    if ( _recvBuff[len + 5] != checksum )
        return kInvalidOutput;
    
    return kSuccess;
}

// Wrapper for the above methods which combine them into a single step.
// All parameters are as described for the above methods.

SSMount::Error SSCelestronAUXMount::commandAUXDevice ( uint8_t cmd, uint8_t dst,
uint8_t sendLen, uint8_t *sendData, uint8_t &recvLen, uint8_t *recvData )
{
    // Send command packet; return error code on failure
    
    SSMount::Error err = sendAUXPacket ( cmd, sendLen, sendData, kControlApp, dst );
    if ( err != kSuccess )
        return err;
    
    // Receive response packet; return error code on failure
    
    uint8_t recvCmd = 0, recvSrc = 0, recvDst = 0;
    err = recvAUXPacket ( recvCmd, recvLen, recvData, recvSrc, recvDst );
    if ( err != kSuccess )
        return err;
    
    // Some AUX bus devices echo the command we sent, before the real response.
    // If we recieved an echoed copy of the packet we just sent, get another packet.
    
    if ( sendLen == recvLen && memcmp ( _sendBuff, _recvBuff, sendLen + 6 ) == 0 )
    {
        err = recvAUXPacket ( recvCmd, recvLen, recvData, recvSrc, recvDst );
        if ( err != kSuccess )
            return err;
    }
    
    // Make sure we received a response that corresponds to the command we sent!
    
    if ( recvCmd != cmd || recvSrc != dst || recvDst != kControlApp )
        return kInvalidOutput;
    
    return kSuccess;
}

// Opens serial or socket connection to Celestron AUX mount and reads controller firmware version strings.
// If port is zero, path is a serial device file (like "/dev/ttyUSBserial0" on Linux or "\\.\COM3" on Windows)
// If port is nonzero, path is mount IP address or fully-qualified domain name (like "1.2.3.4")
// For Celestron Wi-Fi/SkyQ Link in Access Point mode, default IP address is 1.2.3.4, TCP port 2000.
// Returns zero if successful or nonzero error code on failure.

SSMount::Error SSCelestronAUXMount::connect ( const string &path, uint16_t port )
{
    Error err = kSuccess;
    
    // Open serial communication at 9600 baud, no parity, 8 data bits, 1 stop bit.
    
    err = SSMount::connect ( path, port, 9600, SSSerial::kNoParity, SSSerial::k8DataBits, SSSerial::k1StopBits );
    if ( err != kSuccess )
        return err;

    // This pause is for the original SkyQ Link, which sends a *HELLO* string after opening.
    // We send the first AUX commands after *HELLO*, so it's not mistaken for the AUX response.
    
    msleep ( 500 );
    
    // Get motor board version on both axes
    
    for ( int axis = kAzmRAAxis; axis <= kAltDecAxis; axis++ )
    {
        uint8_t len = 0, data[256] = { 0 };
        err = commandAUXDevice ( kGetVersion, kAzimuthMC + axis, 0, nullptr, len, data );
        if ( err != kSuccess )
            return err;

        // Parse both 2- and 4-byte respobses to the version command.
        
        if ( len == 2 )
            _version += format ( "%d.%d", data[0], data[1] );
        else if ( len == 4 )
            _version += format ( "%d.%d.%d", data[0], data[1], 256 * (int) data[2] + data[3] );
        else
            return kInvalidOutput;
        
        if ( axis == kAzmRAAxis )
            _version += ",";
    }

    return kSuccess;
}

// Gets mount RA/Dec in J2000 mean equatorial (fundamental) frame from Celestron AUX motor controller.
// Assumes mount is perfectly polar-aligned if equatorial, or perfectly level if altazimuth!
// Returns zero if successful or nonzero error code on failure. Also checks slew state.

SSMount::Error SSCelestronAUXMount::read ( SSAngle &ra, SSAngle &dec )
{
    Error err = kSuccess;
    uint32_t pos[2] = { 0 };
    
    for ( int axis = kAzmRAAxis; axis <= kAltDecAxis; axis++ )
    {
        uint8_t len = 0, data[256] = { 0 };
        err = commandAUXDevice ( kMCGetPosition, kAzimuthMC + axis, 0, nullptr, len, data );
        if ( err != kSuccess )
            return err;
        
        if ( len == 3 )
            pos[axis] = (uint32_t) data[0] << 16 | (uint32_t) data[1] << 8 | data[2];
        else
            return kInvalidOutput;
    }
    
    _currLon = stepsToRadians ( pos[kAzmRAAxis] );
    _currLat = stepsToRadians ( pos[kAltDecAxis] );

    // Convert from mount frame of reference to fundamental RA/Dec.
    // Really we should use a mount model. TBD.

    SSAngle lon = _currLon, lat = _currLat;
    if ( _aligned )
        _model.encodersToCelestial ( _currLon, _currLat, lon, lat );
    mountToFundamental ( lon, lat, ra, dec );

    // If slewing in progress, check to see if the slew has stopped
    
    if ( _slewing )
        slewing ( _slewing );
    
    return kSuccess;
}

// Starts GoTo target RA/Dec coordinates in J2000 mean equatorial (fundamental) frame.
// Assumes mount is perfectly polar-aligned if equatorial, or perfectly level if altazimuth!
// Returns zero if successful or nonzero error code on failure.

SSMount::Error SSCelestronAUXMount::slew ( SSAngle ra, SSAngle dec )
{
    // Convert from fundamental RA/Dec to mount frame of reference.
    // For equatorial mounts, convert RA to hour angle.
    // Really we should use a mount model. TBD.

    double lon, lat;
    SSAngle mlon, mlat;
    fundamentalToMount ( ra, dec, mlon, mlat );
    if ( _aligned )
        _model.celestialToEncoders ( mlon, mlat, lon, lat );
    
    int32_t steps[2] = { 0 };
    steps[0] = radiansToSteps ( lon );
    steps[1] = radiansToSteps ( lat );
    
    Error err = kSuccess;
    for ( int axis = kAzmRAAxis; axis <= kAltDecAxis; axis++ )
    {
        uint8_t len = 0, data[256] = { 0 };
        
        data[0] = steps[axis] >> 16;
        data[1] = steps[axis] >> 8;
        data[2] = steps[axis];
        
        // TODO: use GotoSlow if the target coordinates are very close to current position?
        
        err = commandAUXDevice ( kMCGotoFast, kAzimuthMC + axis, 3, data, len, nullptr );
        if ( err != kSuccess )
            break;
    }

    if ( err == kSuccess )
    {
        _slewing = true;
        _slewLon = lon;
        _slewLat = lat;
    }
    
    return err;
}

// Starts or stops slewing mount on an axis at a positive or negative rate
// from zero ... maxSlewRate(). If rate is zero, stops slewing on the specified axis.
// Returns zero if successful or nonzero error code on failure.

SSMount::Error SSCelestronAUXMount::slew ( SSSlewAxis axis, int rate )
{
    if ( abs ( rate ) > maxSlewRate() )
        return kInvalidInput;
    
    // TODO: If we have stopped slewing on both axes, get the current sidereal tracking mode
    
    if ( _slewRate[kAzmRAAxis] == 0 && _slewRate[kAltDecAxis] == 0 )
        ;

    uint8_t data = abs ( rate );
    uint8_t cmd = rate > 0 ? kMCMovePositive : kMCMoveNegative;
    uint8_t dst = axis == kAzmRAAxis ? kAzimuthMC : kAltitudeMC;
    uint8_t len = 0;
    
    Error err = commandAUXDevice ( cmd, dst, 1, &data, len, nullptr );
    if ( err )
        return err;
    
    // If we have stopped slewing on both axes, restore the original tracking mode
    
    _slewRate[ axis ] = rate;
    if ( _slewRate[kAzmRAAxis] == 0 && _slewRate[kAltDecAxis] == 0 )
        ; // TODO: setTrackingMode ( _trackMode );
    
    return kSuccess;
}

// Stops any in-progress GoTo or slew, and resumes sidereal tracking.
// Returns zero if successful or nonzero error code on failure.

SSMount::Error SSCelestronAUXMount::stop ( void )
{
    Error err0 = slew ( kAzmRAAxis, 0 );
    Error err1 = slew ( kAltDecAxis, 0 );
    if ( err0 == kSuccess && err1 == kSuccess )
        _slewing = false;
    return err0 == kSuccess ? err1 : err0;
}

// Syncs mount on the specified RA/Dec in J2000 mean equatorial (fundamental) coordinates.
// Assumes mount is perfectly polar-aligned if equatorial, or perfectly level if altazimuth!
// Returns zero if successful or nonzero error code on failure.

SSMount::Error SSCelestronAUXMount::sync ( SSAngle ra, SSAngle dec )
{
    SSAngle lon, lat;
    fundamentalToMount ( ra, dec, lon, lat );
    
    // check if star coordinates are within 1 degree of predicted
    // TODO: test whether new alignment star is within 1 degree of existing alignment star(s) - prevent duplicates!
    
    if ( _aligned )
    {
        SSAngle plon, plat;
        _model.encodersToCelestial ( _currLon, _currLat, plon, plat );
        if ( SSSpherical ( plon, plat ).angularSeparation ( SSSpherical ( lon, lat ) ) > SSAngle::kRadPerDeg )
            return kBadAlignment;
    }
    
    // Add alignment star, but only keep the three most recent.
    
    _model.addStar ( _currLon, _currLat, lon, lat );
    if ( _model.numStars() > 3 )
        _model.delStar ( 0 );
    
    // Enable higher-order model parameters as we add stars.
    
    if ( _model.numStars() >= 2 )
    {
        _model.adjustable ( MODEL_TILT1, true );
        _model.adjustable ( MODEL_TILT2, true );
    }
    
    if ( _model.numStars() >= 3 )
    {
        _model.adjustable ( MODEL_MISALIGN, true );
        _model.adjustable ( MODEL_FLEXURE, true );
    }
    
    // Now recompute the mount model alignment.
    
    _model.align();
    _aligned = true;
    return kSuccess;
    
    // Convert from fundamental RA/Dec to mount frame of reference.
    // Really we should use a mount model. TBD.

    fundamentalToMount ( ra, dec, _currLon, _currLat );
    
    int32_t steps[2] = { 0 };
    steps[0] = radiansToSteps ( _currLon );
    steps[1] = radiansToSteps ( _currLat );
    
    Error err = kSuccess;
    for ( int axis = kAzmRAAxis; axis <= kAltDecAxis; axis++ )
    {
        uint8_t len = 0, data[256] = { 0 };
        
        data[0] = steps[axis] >> 16;
        data[1] = steps[axis] >> 8;
        data[2] = steps[axis];
        
        err = commandAUXDevice ( kMCSetPosition, kAzimuthMC + axis, 3, data, len, nullptr );
        if ( err != kSuccess )
            break;
    }

    if ( err == kSuccess )
        _aligned = true;
    
    return err;
}

// Queries status of a GoTo: true = in progress, false = not slewing.
// Returns error code or zero if successful.

SSMount::Error SSCelestronAUXMount::slewing ( bool &status )
{
    uint8_t done[2] = { 0 };
    
    for ( int axis = kAzmRAAxis; axis <= kAltDecAxis; axis++ )
    {
        uint8_t len = 0, data[256] = { 0 };
        Error err = commandAUXDevice ( kMCGotoDone, kAzimuthMC, 0, nullptr, len, data );
        if ( err != kSuccess )
            return err;
        
        if ( len == 1 )
            done[axis] = data[0];
        else
            return kInvalidOutput;
    }
    
    status = done[0] && done[1] ? false : true;
    return kSuccess;
}
