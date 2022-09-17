// SSMount.hpp
// SSCore
//
// Created by Tim DeBenedictis on 9/14/22.
// Copyright © 2022 Southern Stars. All rights reserved.
//

#ifndef SSMount_hpp
#define SSMount_hpp

#include <stdio.h>
#include <string>
#include <map>

#include "SSSerial.hpp"
#include "SSSocket.hpp"
#include "SSCoordinates.hpp"

enum SSMountType
{
    kAltAzimuthPushMount = 0,   // Dobsonians, manual alt-az forks
    kAltAzimuthGotoMount = 1,   // Computer-driven alt-az mounts
    kEquatorialPushMount = 2,   // Dobsonians on equatorial platforms
    kEquatorialGotoMount = 3    // Computer-driven equatorial mounts
};

enum SSMountProtocol
{
    kNoProtocol = 0,
    kMeadeLX200 = 1000,
    kMeadeETX = 1001,
    kCelestronNexStar = 2000,
    kSkyWatcherSynScan = 2001
};

// Directional slew axis identifiers

enum SSSlewAxis
{
    kAzmRAAxis = 0,           // azimuth or RA axis
    kAltDecAxis = 1,          // altitude or Dec axis
};

// Directional slew sign identifiers

enum SSSlewSign
{
    kSlewNegative = -1,         // slew in negative direction
    kSlewOff = 2,               // no directional slewing
    kSlewPositive = 1           // slew in positive direction
};

// Represents a telescope mount

class SSMount
{
public:

    // High-level mount communication error codes
    
    enum Error
    {
        kSuccess = 0,           // No error - all OK!
        kInvalidInput = 1,      // Input command parameter was not valid
        kInvalidOutput = 2,     // Output data from the mount was not valid (garbage, could not be parsed, etc.)
        kInvalidCoords = 3,     // Slew/sync coordinates sent to mount invalid, below horizon, out of reach of mount, etc.
        kOpenFail = 4,          // Can't open serial port or socket connection
        kCloseFail = 5,         // Can't close serial port or socket connection
        kReadFail = 6,          // Can't read from serial port or socket connection
        kWriteFail = 7,         // Can't write to serial port or socket connection
        kNotSupported = 8,      // Functionality is not implemented/not supported
        kTimedOut = 9,          // Read/write operation timed out before completion
    };

protected:
    SSMountType     _type;      // Physical mount type
    SSMountProtocol _protocol;  // Mount communication protocol identifier
    SSCoordinates &_coords;     // Reference to SSCoordinates object containing mount's geographic location and current date/time
    
    SSSerial _serial;           // Serial port used for communication; invalid if not connected
    SSSocket _socket;           // Socket used for communcation; invalid if not connected
    
    SSAngle _currRA, _currDec;  // Most recent coordinates reported from mount
    SSAngle _slewRA, _slewDec;  // Slew target coordinates

    SSSlewSign  _slewSign[2];   // Current slew sign on RA/Azm [0] and Alt/Dec [1] axes
    int         _slewRate;      // Current rate for directional slewing from 0 (off) to maxSlewRate() (fastest)
    bool        _slewing;       // true if a GoTo is currently in progress; false otherwise.
    bool        _connected;     // true if serial port or socket connection to mount is currently open.
    string      _version;       // mount controller firmware version string, read from mount during connect()

    virtual Error connect ( const string &path, unsigned short port, int baud, int party, int data, float stop );
    Error serialCommand ( const char *input, int inlen, char *output, int outlen, char term, int timeout_ms );
    Error socketCommand ( const char *input, int inlen, char *output, int outlen, char term, int timeout_ms );

public:
    // Constructor, destructor
    
    SSMount ( SSMountType type, SSCoordinates &coords );
    virtual ~SSMount ( void );
    
    // Accessors
    
    SSMountType getType ( void ) { return _type; }
    SSMountProtocol getProtocol ( void ) { return _protocol; }
    SSCoordinates &getCoordinates ( void ) { return _coords; }
    SSAngle getRA ( void ) { return _currRA; }
    SSAngle getDec ( void ) { return _currDec; }
    int getSlewRate ( void ) { return _slewRate; }
    SSSlewSign getSlewSign ( SSSlewAxis axis ) { return _slewSign[axis]; }
    string getVersion ( void ) { return _version; }
    bool slewing ( void ) { return _slewing; }
    bool connected ( void )  { return _connected; }

    // Open and close serial or socket connection to mount
    
    virtual Error connect ( const string &path, unsigned short port );
    virtual Error disconnect ( void );                                   // close connection
    
    // Send commands to mount and recieve replies
    
    Error command ( const char *input, int inlen, char *output, int outlen, char term, int timeout_ms = 2000 );
    Error command ( const string &input, string &output, int outlen, char term, int timeout_ms = 2000 );
    Error command ( const string &input );

    // High-level mount commands
    
    virtual Error read ( SSAngle &ra, SSAngle &dec );            // get current mount RA/Dec coordinates
    virtual Error slew ( SSAngle ra, SSAngle dec );              // start GoTo slewing to target RA/Dec coordinates at fastest possible rate
    virtual Error slew ( SSSlewAxis axis, SSSlewSign sign );     // start or stop slewing mount on an axis in positive or negative direction at current slew rate
    virtual Error rate ( int rate );                             // set rate for directional slewing
    virtual Error stop ( void );                                 // stop slewing, cancel any in-progress GoTo, and resume sidereal tracking
    virtual Error sync ( SSAngle ra, SSAngle dec );              // sync or align mount on the specified coordinates
    virtual Error slewing ( bool &status );                      // queries whether a GoTo slew is currently in progress
    virtual Error aligned ( bool &status );                      // queries whether mount is currently aligned or not
    virtual Error setDateTime ( SSTime time );                   // send local date and time to mount
    virtual Error setLonLat ( SSSpherical loc );                 // send geographic location to mount

    // Send date/time and lon/lat from mount's coordinates object reference
    
    Error setDateTime ( void ) { return setDateTime ( _coords.getTime() ); }
    Error setLonLat ( void ) { return setLonLat ( _coords.getLocation() ); }
    
    virtual int maxSlewRate ( void ) { return 4; }
};

// Overrides for Meade mounts

class SSMeadeMount : public SSMount
{
protected:
    
    virtual Error setTargetRADec ( SSAngle ra, SSAngle dec );
    
public:
    
    SSMeadeMount ( SSMountType type, SSMountProtocol variant, SSCoordinates &coords );

    virtual int maxSlewRate ( void ) { return _protocol == kMeadeETX ? 3 : 4; }
    virtual Error connect ( const string &path, unsigned short port );
    virtual Error read ( SSAngle &ra, SSAngle &dec );
    virtual Error slew ( SSAngle ra, SSAngle dec );
    virtual Error slew ( SSSlewAxis axis, SSSlewSign sign );
    virtual Error stop ( void );
    virtual Error rate ( int rate );
    virtual Error sync ( SSAngle ra, SSAngle dec );
    virtual Error slewing ( bool &status );
    virtual Error aligned ( bool &status );
    virtual Error setDateTime ( SSTime time );
    virtual Error setLonLat ( SSSpherical loc );
};

// Overrides for Celestron mounts

class SSCelestronMount : public SSMount
{
protected:
    
    // Celestron tracking modes

    enum TrackingMode
    {
        kUnknownTracking = -1,
        kNoTracking = 0,
        kAltAzTracking = 1,
        kEquNorthTracking = 2,
        kEquSouthTracking = 3
    }
    _trackMode;

    virtual Error getTrackingMode ( TrackingMode &mode );
    virtual Error setTrackingMode ( TrackingMode mode );
    
public:
    SSCelestronMount ( SSMountType type, SSMountProtocol variant, SSCoordinates &coords );

    virtual int maxSlewRate ( void ) { return 10; }
    virtual Error connect ( const string &path, unsigned short port );
    virtual Error read ( SSAngle &ra, SSAngle &dec );
    virtual Error slew ( SSAngle ra, SSAngle dec );
    virtual Error slew ( SSSlewAxis axis, SSSlewSign sign );
    virtual Error stop ( void );
    virtual Error sync ( SSAngle ra, SSAngle dec );
    virtual Error slewing ( bool &status );
    virtual Error aligned ( bool &status );
    virtual Error setDateTime ( SSTime time );
    virtual Error setLonLat ( SSSpherical loc );
};

// Obtains map of supported mount protocol names, indexed by protocol identifier

typedef map<SSMountProtocol,string> SSMountProtocolMap;
int SSGetMountProtocols ( SSMountProtocolMap &map );

typedef SSMount *SSMountPtr;
typedef SSMeadeMount *SSMeadeMountPtr;
typedef SSCelestronMount *SSCelestronMountPtr;

// Allocates new SSMount (or subclass of SSMount) depending on supplied protocol identifier.

SSMountPtr SSNewMount ( SSMountType type, SSMountProtocol protocol, SSCoordinates &coords );

// Downcasts a pointer from SSMount base class to SSMeadeMount or SSCelestronMount subclass.
// Returns nullptr if input mount pointer is not actually an SSMeadeMount or SSCelestronMount.

SSMeadeMountPtr SSGetMeadeMountPtr ( SSMountPtr ptr );
SSCelestronMountPtr SSGetCelestronMountPtr ( SSMountPtr ptr );

#endif /* SSMount_hpp */