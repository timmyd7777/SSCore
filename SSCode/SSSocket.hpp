// SSSocket.hpp
// SSCore
//
// Created by Tim DeBenedictis on 9/12/22.
// Copyright © 2022 Southern Stars Group, LLC. All rights reserved.
//
// This class implements basic IPv4 and IPv6 network TCP and UDP socket communication.
// TCP server sockets are supported. SSL and not supported. Implements basic HTTP 1.1 requests.
// On Windows, make sure to link with WS2_32.LIB and IPHLPAPI.LIB!
// On MacOS and iOS, make sure your Entitlements file supports both
// incoming and outgoing network connections; see SSSocket.cpp for details

#ifndef SSSocket_hpp
#define SSSocket_hpp

// Important - must #include <winsock2.h> before <windows.h> !

#ifdef _MSC_VER
    #include <winsock2.h>
    #include <WS2tcpip.h>
    #include <windows.h>
#else
    #include <unistd.h>
    #include <sys/socket.h>
    #include <netinet/in.h>
    typedef int SOCKET;                 // Native socket data type
    constexpr int INVALID_SOCKET = -1;  // Native invalid socket value
#endif

#include <string>
#include <vector>

using namespace std;

#include "SSTime.hpp"
#include "SSVector.hpp"

// Represents an IPv4 or IPv6 address.
// Implemented as wrapper around platform-native IPv4/v6 address structs.

struct SSIP
{
    union
    {
        in_addr addr;    // Native IPv4 address; internals are platform-dependent!
        in6_addr add6;   // Native IPv6 address; internals are platform-dependent!
    };
    bool ipv6;           // if true, this is an IPv6 address; if false, an IPv4.
    
    // Constructors
    
    SSIP ( void );
    SSIP ( const string &str );
    SSIP ( const struct in_addr &add );
    SSIP ( const struct in6_addr &add );
    SSIP ( const uint32_t val );
    
    operator uint32_t() const;  // returns IPv4 address as 32-bit unsigned integer on all platforms; returns zero if IPv6.
    bool specified ( void );    // return true if address is nonzero; works for both IPv4 amd v6
    
    // Converts IP address to/from dotted notation (like "192.168.0.1" or "2345:425:2ca1::567:5673:23b5")
    
    string toString ( void );
    static SSIP fromString ( const string &str );
    
    SSIP toIPv6 ( void );   // Converts IPv4 address to IPv4-mapped IPv6 address. Returns IPv6 addresses unchanged.
    SSIP toIPv4 ( void );   // Converts IPv4-mapped IPv6 addresses to IPv4. Returns IPv4 addresses unchanged.
};

class SSSocket
{
protected:
    SOCKET _socket = INVALID_SOCKET;    // Native socket

public:
    // Constructors and destructor

    SSSocket ( void ) { initialize(); _socket = INVALID_SOCKET; }
    SSSocket ( SOCKET s ) { initialize(); _socket = s; }
    ~SSSocket ( void ) { closeSocket(); }
    
    // Accessor returns native socket
    
    SOCKET getSocket ( void ) { return _socket; }

    // Socket library initialiation and cleanup - call once at program startup and exit
    
    static bool initialize ( void );
    static void finalize ( void );
    
    // Converts host names to IPv4 or IPv6 addresses and vice-versa;
    // Get IPv4 or v6 addresses of all local interfaces
    
    static vector<SSIP> hostNameToIPs ( const string &hostname, bool ipv6 = false );
    static string IPtoHostName ( const SSIP &ip );
    static vector<SSIP> getLocalIPs ( bool ipv6 = false );
    
    // TCP (connection-oriented) sockets: open, read, write, close
    
    bool openSocket ( SSIP serverIP, unsigned short port, int timeout_ms );
    bool socketOpen ( void );
    int writeSocket ( const void *data, int size );
    int readSocket ( void *data, int size );
    void closeSocket ( void );
    bool getRemoteIP ( SSIP &peerIP );

    // TCP server sockets: open, listen, accept incoming connections
    
    bool serverOpenSocket ( SSIP serverIP, unsigned short port, int maxConnectioms );
    bool serverConnectionPending ( void );
    SSSocket serverAcceptConnection ( void );
    
    // UDP (connectionless) sockets: open, read, write
    
    bool openUDPSocket ( SSIP localIP, unsigned short localPort );
    int writeUDPSocket ( const void *data, int size, SSIP destIP, unsigned short destPort );
    int readUDPSocket ( void *data, int size, SSIP &senderIP, int timeout_ms );
    bool isUDPSocket ( void );
};

// Implements basic HTTP 1.1 requests and responses using SSSocket class.
// All communication is synchronous on the current thread.

class SSHTTP
{
protected:
    string      _url;           // Uniform Resource Locator for this request
    string      _type;          // resource type ("http", "https", "file", etc.)
    string      _host;          // hostname of server
    uint16_t    _port;          // TCP port for server
    string      _path;          // path of desired resource on server
    uint32_t    _timeout;       // communication timeout [milliseconds]
    
    SSSocket    _socket;        // TCP socket connection to server
    
    string      _respHead;      // response header string; empty until request is sent
    int         _respCode;      // response code: 200 = OK, 404 = Not Found, etc.
    SSDate      _date;          // response date
    string      _location;      // response location (for redirects)
    size_t      _contLen;       // request or response content length, bytes
    string      _contType;      // reqeust or response content MIME type string
    vector<char> _content;      // buffer containing sent or received content

    int sendRequestHeader ( size_t postSize = 0 );
    int readResponseHeader ( void );
    int readContent ( void );
    int sendContent ( const void *content, size_t len );
    
public:
    
    // common HTTP response codes; see https://en.wikipedia.org/wiki/List_of_HTTP_status_codes
    
    static constexpr int kOK = 200;
    static constexpr int kCreated = 201;
    static constexpr int kAccepted = 202;
    static constexpr int kNonAuthoritative = 203;
    static constexpr int kNoContent = 204;
    static constexpr int kResetContent = 205;
    static constexpr int kMultipleChoices = 300;
    static constexpr int kMovedPermanently = 301;
    static constexpr int kFound = 302;
    static constexpr int kSeeOther = 303;
    static constexpr int kNotModified = 304;
    static constexpr int kBadRequest = 400;
    static constexpr int kUnauthorized = 401;
    static constexpr int kPaymentRequired = 402;
    static constexpr int kForbidden = 403;
    static constexpr int kNotFound = 404;
    static constexpr int kMethodNotAllowed = 405;
    static constexpr int kRequestTimeout = 408;
    static constexpr int kServerError = 500;
    static constexpr int kNotImplemented = 501;
    static constexpr int kBadGateway = 502;
    static constexpr int kServiceUnavailable = 503;
    static constexpr int kGatewayTimeout = 504;
    static constexpr int kNotSupported = 505;
    
    // constructor, destructor
    
    SSHTTP ( const string &url, uint32_t timeout = 60000 );
    virtual ~SSHTTP ( void );
    
    // accessors and modifiers

    string getURL ( void ) { return _url; }
    uint32_t getTimeout ( void ) { return _timeout; }
    string getHost ( void ) { return _host; }
    uint16_t getPort ( void ) { return _port; }
    string getPath ( void ) { return _path; }
    string getResponseHeaders ( void ) { return _respHead; }
    string getHeaderValue ( const string &key );
    int getResponseCode ( void ) { return _respCode; }
    SSDate getDate ( void ) { return _date; }
    string getLocation ( void ) { return _location; }
    size_t getContentLength ( void ) { return _contLen; }
    string getContentType ( void ) { return _contType; }
    const void *getContent ( void ) { return &_content[0]; }
    size_t getContentSize ( void ) { return _content.size(); }
    string getContentString ( void );

    void setURL ( const string &url );
    void setTimeout ( uint32_t timeout ) { _timeout = timeout; }
    void setContentLength ( size_t len ) { _contLen = len; }
    void setContentType ( string type ) { _contType = type; }
    void setContent ( const void *content, size_t size );
    void setContentString ( const string &str );

    // high-level requests
    
    int get ( void );
    int post ( void );
    int post ( const void *postData, size_t postSize );
};

// Obtains geographic location from local IP address.
// Returns true if successful or false on failure.

bool SSLocationFromIP ( SSSpherical &loc );

#endif /* SSSocket_hpp */
