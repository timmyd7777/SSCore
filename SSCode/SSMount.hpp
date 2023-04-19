// SSMount.hpp
// SSCore
//
// Created by Tim DeBenedictis on 9/14/22.
// Copyright © 2022 Southern Stars. All rights reserved.
//
// This class implements communication with common amateur telescope mount
// controllers over serial port and TCP/IP sockets. Supported protocols include
// Meade LX-200/Autostar, Celestron NexStar, and SkyWatcher/Orion SynScan.

#ifndef SSMount_hpp
#define SSMount_hpp

#include <stdio.h>
#include <string>
#include <map>
#include <mutex>

// Important for Windows - must #include "SSSocket.hpp" before "SSSerial.hpp"

#include "SSSocket.hpp"
#include "SSSerial.hpp"
#include "SSCoordinates.hpp"
#include "SSMountModel.hpp"

// Mechanical mount families

enum SSMountType
{
    kAltAzimuthPushMount = 0,   // Dobsonians, manual alt-az forks
    kAltAzimuthGotoMount = 1,   // Computer-driven alt-az mounts
    kEquatorialPushMount = 2,   // Dobsonians on equatorial platforms
    kEquatorialGotoMount = 3    // Computer-driven equatorial mounts
};

// Mount controller protocol identifiers

enum SSMountProtocol
{
    kNoProtocol = 0,            // No real mount communication - SSMount API works as a mount emulator with this "protocol"
    kMeadeLX200 = 1000,         // Meade LX-200 classic and LX-200GPS mounts
    kMeadeAutostar = 1001,      // Meade Autostar and Audiostar controllers
    kCelestronNexStar = 2000,   // Celestron NexStar and StarSense controllers
    kSkyWatcherSynScan = 2001,  // SkyWatcher and Orion SynScan controllers
    kSyntaDirect = 3000,        // Direct interface to Synta motor controllers (e.g. SynScan Wi-Fi, EQMOD, EQDIR)
    kCelestronAUX = 4000        // Celestron AUX port communication (e.g. Celestron Wi-Fi Link)
};

// Directional slew axis identifiers

enum SSSlewAxis
{
    kAzmRAAxis = 0,           // azimuth or RA axis
    kAltDecAxis = 1,          // altitude or Dec axis
};

// Represents a telescope mount and implements communication with the mount.
// The base SSMount class simulates an equatorial GoTo mount, but has no
// communication protocol and controls no real hardware.

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
        kBadAlignment = 10      // Alignment star could not be added to mount model
    };

    // Pointer to completion callback for asynchronouos command methods
    
    typedef void (* AsyncCmdCallback) ( SSMount *pMount, Error error, void *pUserData );

protected:
    
    SSMountType     _type;      // Physical mount type
    SSMountProtocol _protocol;  // Mount communication protocol identifier
    SSCoordinates &_coords;     // Reference to SSCoordinates object containing mount's geographic location and current date/time
    SSMountModel    _model;     // Celestial alignment model; used by Synta Direct and Celestron AUX mounts.
    
    SSSerial    _serial;        // Serial port used for communication; invalid if not connected
    SSSocket    _socket;        // Socket used for communcation; invalid if not connected
    SSIP        _addr;          // IP address of telescope mount, only valid for socket connections
    uint16_t    _port;          // TCP or UDP port for socket-based mount communication
    
    SSAngle _initLon, _initLat; // Mount coordinates where most recent slew began, in mount frame.
    SSAngle _currLon, _currLat; // Most recent coordinates, in mount frame (kEquatorial or kHorizon).
    SSAngle _slewLon, _slewLat; // GoTo target coordinates, in mount frame (kEquatorial or kHorizon).
    SSAngle _trackRA, _trackDec;// Tracking target coordinataes, in fundamental (J2000 mean equatorial) frame.
    
    double      _slewTime[2];   // slew start time (seconds since midnight) on RA/Azm [0] and Alt/Dec [1] axes
    int         _slewRate[2];   // Current slew rate on RA/Azm [0] and Alt/Dec [1] axes
    bool        _slewing;       // true if a GoTo is currently in progress; false otherwise.
    bool        _connected;     // true if serial port or socket connection to mount is currently open.
    bool        _aligned;       // true it mount has been star-aligned; false otherwise.
    bool        _tracking;      // true if mount is should track sidereal motion when not slewing.
    string      _version;       // mount controller firmware version string, read from mount during connect()

    mutex       _cmdMtx;        // for preventing resource contention with asynchronous command calls
    int         _retries;       // maximum number of command attempts before assuming failure
    int         _timeout;       // default command communication response timeout, milliseconds

    FILE        *_logFile;      // pointer to open log file; NULL if none
    double      _logStart;      // log file start time, seconds
    
    virtual Error connect ( const string &path, uint16_t port, int baud, int party, int data, float stop, bool udp = false );
    Error serialCommand ( const char *input, int inlen, char *output, int outlen, char term, int timeout_ms );
    Error socketCommand ( const char *input, int inlen, char *output, int outlen, char term, int timeout_ms );
    static SSAngle angularRate ( int rate );  // converts integer slew rate identifier to angular rate in radians/second.
    
public:
    // Constructor, destructor
    
    SSMount ( SSMountType type, SSCoordinates &coords );
    virtual ~SSMount ( void );
    
    // Accessors
    
    SSMountType getType ( void ) { return _type; }
    SSMountProtocol getProtocol ( void ) { return _protocol; }
    SSCoordinates &getCoordinates ( void ) { return _coords; }
    int getSlewRate ( SSSlewAxis axis ) { return _slewRate[axis]; }
    string getVersion ( void ) { return _version; }
    bool slewing ( void ) { return _slewing; }
    bool connected ( void )  { return _connected; }
    bool aligned ( void )  { return _aligned; }
    bool tracking ( void )  { return _tracking; }
    bool isEquatorial ( void ) { return _type == kEquatorialPushMount || _type == kEquatorialGotoMount; }
    bool isGoTo ( void ) { return _type == kAltAzimuthGotoMount || _type == kEquatorialGotoMount; }
    void fundamentalToMount ( SSAngle ra, SSAngle dec, SSAngle &lon, SSAngle &lat );
    void mountToFundamental ( SSAngle lon, SSAngle lat, SSAngle &ra, SSAngle &dec );

    // Open and close serial or socket connection to mount
    
    virtual Error connect ( const string &path, uint16_t port );
    virtual Error disconnect ( void );
    
    // Send commands to mount and recieve replies
    
    Error command ( const char *input, int inlen, char *output, int outlen, char term, int timeout_ms = 0 );
    Error command ( const string &input, string &output, int outlen, char term, int timeout_ms = 0 );
    Error command ( const string &input );

    void setRetries ( int attempts ) { _retries = attempts; }
    int  getRetries ( void ) { return _retries; }

    void setTimeout ( int millisecs ) { _timeout = millisecs; }
    int  getTimeout ( void ) { return _timeout; }

    // Communication logging to file (for debugging purposes)
    
    Error openLog ( const string &path );
    Error writeLog ( bool input, const char *data, int len, Error err );
    void closeLog ( void );

    // High-level mount commands, synchronous versions
    
    virtual Error read ( SSAngle &ra, SSAngle &dec );            // get current mount RA/Dec coordinates and slewing state
    virtual Error slew ( SSAngle ra, SSAngle dec );              // start GoTo slewing to target RA/Dec coordinates at fastest possible rate
    virtual Error slew ( SSSlewAxis axis, int rate );            // start or stop slewing mount on an axis at a positive or negative rate
    virtual Error stop ( void );                                 // stop slewing, cancel any in-progress GoTo, and resume sidereal tracking
    virtual Error sync ( SSAngle ra, SSAngle dec );              // sync or align mount on the specified coordinates
    virtual Error slewing ( bool &status );                      // queries whether a GoTo slew is currently in progress
    virtual Error aligned ( bool &status );                      // queries whether mount is currently aligned or not
    virtual Error tracking ( bool &state );                      // queries whether mount is currently tracking sidereal motion, or not.
    virtual Error tracking ( bool state );                       // starts or stops sidereal tracking.
    virtual Error setTime ( SSTime time );                       // send local date, time, and time zone to mount
    virtual Error setSite ( SSSpherical site );                  // send local site longitude and latitude to mount
    virtual Error getTime ( SSTime &time );                      // read local date, time, and time zone from mount
    virtual Error getSite ( SSSpherical &site );                 // read local site longitude and latitude from mount

    // High-level mount commands, asynchronous versions
    
    void readAsync ( AsyncCmdCallback callback = nullptr, void *userData = nullptr );
    void slewAsync ( SSAngle ra, SSAngle dec, AsyncCmdCallback callback = nullptr, void *userData = nullptr );
    void slewAsync ( SSSlewAxis axis, int rate, AsyncCmdCallback callback = nullptr, void *userData = nullptr );
    void stopAsync ( AsyncCmdCallback callback = nullptr, void *userData = nullptr );
    void syncAsync ( SSAngle ra, SSAngle dec, AsyncCmdCallback callback = nullptr, void *userData = nullptr );
    void lockMutex ( bool state ) { if ( state ) _cmdMtx.lock(); else _cmdMtx.unlock(); }
    
    // Send date/time and lon/lat from mount's coordinates object reference
    
    Error setTime ( void ) { return setTime ( _coords.getTime() ); }
    Error setSite ( void ) { return setSite ( _coords.getLocation() ); }

    virtual int maxSlewRate ( void ) { return 4; }
};

// Overrides for Meade mounts

class SSMeadeMount : public SSMount
{
protected:
    
    virtual Error setTargetRADec ( SSAngle ra, SSAngle dec );
    virtual Error setSlewRate ( int rate );
    
public:
    
    SSMeadeMount ( SSMountType type, SSMountProtocol variant, SSCoordinates &coords );

    virtual int maxSlewRate ( void ) { return _protocol == kMeadeAutostar ? 3 : 4; }
    virtual Error connect ( const string &path, uint16_t port );
    virtual Error read ( SSAngle &ra, SSAngle &dec );
    virtual Error slew ( SSAngle ra, SSAngle dec );
    virtual Error slew ( SSSlewAxis axis, int rate );
    virtual Error stop ( void );
    virtual Error sync ( SSAngle ra, SSAngle dec );
    virtual Error slewing ( bool &status );
    virtual Error aligned ( bool &status );
    virtual Error setTime ( SSTime time );
    virtual Error setSite ( SSSpherical site );
    virtual Error getTime ( SSTime &time );
    virtual Error getSite ( SSSpherical &site );
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

    virtual int maxSlewRate ( void ) { return 9; }
    virtual Error connect ( const string &path, uint16_t port );
    virtual Error read ( SSAngle &ra, SSAngle &dec );
    virtual Error slew ( SSAngle ra, SSAngle dec );
    virtual Error slew ( SSSlewAxis axis, int rate );
    virtual Error stop ( void );
    virtual Error sync ( SSAngle ra, SSAngle dec );
    virtual Error slewing ( bool &status );
    virtual Error aligned ( bool &status );
    virtual Error setTime ( SSTime time );
    virtual Error setSite ( SSSpherical site );
    virtual Error getTime ( SSTime &time );
    virtual Error getSite ( SSSpherical &site );
};

// Overrides for Synta Direct mounts.
// Implements direct communication with Synta (i.e. SkyWatcher/Orion)
// mount motors, bypassing the SynScan hand controller, via serial connection
// (like EQMOD or EQDIR) or SynScan Wi-Fi adapter.
// This class is still somewhat experimental; it should work if the mount is
// perfectly polar aligned (equatorial mounts) or perfectly level (alt-azimuth).
// Really we should use a mount pointing model and mult-star alignment. TBD!

class SSSyntaMount : public SSMount
{
protected:
    
    int _countsPerRev[2];       // counts per revolution on Azm/RA axis [0] and Alt/Dec axis [1]
    int _mcVersion[2];          // motor controller version (as integer) on both axes
    int _highSpeedRatio[2];     // high vs low motor speed motor ratio on both axes
    int _stepTimerFreq[2];      // frequency of stepping timer interrupt on both axes
    int _breakSteps[2];         // Break steps from slewing to stop on both axes
    
    bool _aligned;              // true if sync() has succeeded; false otherwise

    // axis status returned by mcGetAxisStatus()
    
    struct AxisStatus
    {
        bool fullStop;          // Axis is fully stopped
        bool slewing;           // Axis is running
        bool slewingTo;         // Axis in slewing (constant speed) mode
        bool slewingForward;    // Angle increases; otherwise angle decreases
        bool highSpeed;         // HighSpeed running mode
        bool notInitialized;    // MC is not initialized
    };
    
    Error motorCommand ( int axis, char cmd, string input, string &output );
    Error mcAxisStop ( int axis, bool instant );
    Error mcAxisSlew ( int axis, double speed );    // speed in radians/sec
    Error mcAxisSlewTo ( int axis, double radians );
    Error mcGetAxisStatus ( int axis, AxisStatus &status );
    Error mcGetAxisPosition ( int axis, SSAngle &radians );
    Error mcSetAxisPosition ( int axis, SSAngle radians );

    int angleToStep ( int axis, double rad ) { return _countsPerRev[axis] * rad / SSAngle::kTwoPi; }
    double stepToAngle ( int axis, int step ) { return SSAngle::kTwoPi * step / _countsPerRev[axis]; }
    int radSpeedToInt ( int axis, double rad ) { return _stepTimerFreq[axis] / angleToStep ( axis, rad ); }
    
public:
    
    SSSyntaMount ( SSMountType type, SSCoordinates &coords );

    virtual int maxSlewRate ( void ) { return 4; }
    virtual Error connect ( const string &path, uint16_t port );
    virtual Error read ( SSAngle &ra, SSAngle &dec );
    virtual Error slew ( SSAngle ra, SSAngle dec );
    virtual Error slew ( SSSlewAxis axis, int rate );
    virtual Error stop ( void );
    virtual Error sync ( SSAngle ra, SSAngle dec );
    virtual Error slewing ( bool &status );
    virtual Error aligned ( bool &status );
    virtual Error setTime ( SSTime time ) { return kNotSupported; }
    virtual Error setSite ( SSSpherical site ) { return kNotSupported; }
    virtual Error getTime ( SSTime &time ) { return kNotSupported; }
    virtual Error getSite ( SSSpherical &site ) { return kNotSupported; }
};

// Overrides for Celestron AUX port mount communication.
// Implements direct communication with Celestron mount motors via AUX port,
// bypassing the NexStar hand controller, via Celestron Wi-Fi adapter.
// This class is still somewhat experimental; it should work if the mount is
// perfectly polar aligned (equatorial mounts) or perfectly level (alt-azimuth).
// Really we should use a mount pointing model and mult-star alignment. TBD!

class SSCelestronAUXMount : public SSMount
{
protected:
    
    // Identifiers for known Celestron AUX bus devices.
    // See https://github.com/platini2/celestronauxbus/blob/main/celestron.py
    
    enum Device
    {
        kMainBoard = 0x01,          // Main Board
        kNexStarHC = 0x04,          // NexStar Hand Controller
        kNexStarPlusHC = 0x0d,      // NexStar+ Hand Controller
        kStarSenseHC = 0x0e,        // StarSense Hand Controller
        kAzimuthMC = 0x10,          // Azimuth Motor Controller
        kAltitudeMC = 0x11,         // Altitude Motor Controller
        kFocuser = 0x12,            // Focus motor
        kDewHeater = 0x17,          // Dew Heater
        kControlApp = 0x20,         // Control application (e.g. SkyPortal)
        kWiFiLink = 0xb3,           // WiFi Link acccessory
        kGPS = 0xb0                 // GPS Accessory
    };
    
    // Celestron AUX bus device command identifiers
    
    enum Command
    {
        kMCGetPosition = 0x01,      // Get position. Response is signed 24-bit fraction of a full rotation.
        kMCGotoFast = 0x02,         // Goto position at fastest rate. Position is signed 24-bit fraction of a full rotation.
        kMCSetPosition = 0x04,      // Set position. Position is signed 24-bit fraction of a full rotation.
        kMCGetModel = 0x05,         // Returns the model number of the telescope
        kMCGotoDone = 0x13,         // Checks if Goto is complete, where 0x00 = not done, 0xff = done.
        kMCGotoSlow = 0x17,         // Goto position with slow rate. Position is signed 24-bit fraction of a full rotation.
        kMCMovePositive = 0x24,     // Move positive (up/right) at rate 0 - 9, rate = 0 means stop.
        kMCMoveNegative = 0x25,     // Move negative (down/left) at rate 0 - 9, rate = 0 means stop.
        kGetVersion = 0xfe          // Get firmware version, where byte 0 = major, byte 1 = minor, byte 2-3 = build.
    };
    
    char _sendBuff[256];    // contains most-recently-sent AUX bus packet
    char _recvBuff[256];    // contains most-recently-read AUX bus packet
    
    Error sendAUXPacket ( uint8_t cmd, uint8_t len, uint8_t *data, uint8_t src, uint8_t dst );
    Error recvAUXPacket ( uint8_t &cmd, uint8_t &len, uint8_t *data, uint8_t &src, uint8_t &dst );
    Error commandAUXDevice ( uint8_t cmd, uint8_t dst, uint8_t sendLen, uint8_t *sendData, uint8_t &recvLen, uint8_t *recvData );
    
    static constexpr double kStepsPerRad = 0x01000000 / SSAngle::kTwoPi; // 2670176.85772
    static double stepsToRadians ( int32_t steps ) { return steps / kStepsPerRad; }
    static int32_t radiansToSteps ( double rad ) { return rad * kStepsPerRad; }
    
public:
    
    SSCelestronAUXMount ( SSMountType type, SSCoordinates &coords );
    virtual int maxSlewRate ( void ) { return 9; }

    virtual Error connect ( const string &path, uint16_t port );
    virtual Error read ( SSAngle &ra, SSAngle &dec );
    virtual Error slew ( SSAngle ra, SSAngle dec );
    virtual Error slew ( SSSlewAxis axis, int rate );
    virtual Error stop ( void );
    virtual Error sync ( SSAngle ra, SSAngle dec );
    virtual Error slewing ( bool &status );
};

// Obtains map of supported mount protocol names, indexed by protocol identifier

typedef map<SSMountProtocol,string> SSMountProtocolMap;
int SSGetMountProtocols ( SSMountProtocolMap &map );

typedef SSMount *SSMountPtr;
typedef SSMeadeMount *SSMeadeMountPtr;
typedef SSCelestronMount *SSCelestronMountPtr;
typedef SSSyntaMount *SSSyntaMountPtr;
typedef SSCelestronAUXMount *SSCelestronAUXMountPtr;

// Allocates new SSMount (or subclass of SSMount) depending on supplied protocol identifier.

SSMountPtr SSNewMount ( SSMountType type, SSMountProtocol protocol, SSCoordinates &coords );

// Downcasts a pointer from SSMount base class to SSMeadeMount or SSCelestronMount subclass.
// Returns nullptr if input mount pointer is not actually an SSMeadeMount or SSCelestronMount.

SSMeadeMountPtr SSGetMeadeMountPtr ( SSMountPtr ptr );
SSCelestronMountPtr SSGetCelestronMountPtr ( SSMountPtr ptr );
SSSyntaMountPtr SSGetSyntaMountPtr ( SSMountPtr ptr );
SSCelestronAUXMountPtr SSGetCelesronAUXMountPtr ( SSMountPtr ptr );

// Attempts to find IPv4 address of SkyFi with the given name.
// The function returns true if successful or false on failure.
// If successful, SkyFi's IPv4 address will be returned in addr.

bool SSFindSkyFi ( const string &name, SSIP &addr, int attempts = 3, int timeout = 1000 );

#endif /* SSMount_hpp */
