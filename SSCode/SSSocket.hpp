// SSSocket.hpp
// SSCore
//
// Created by Tim DeBenedictis on 9/12/22.
// Copyright © 2022 Southern Stars Group, LLC. All rights reserved.
//
// This class implements basic IPv4 network TCP and UDP socket communication.
// TCP server sockets are supported. IPv6 and SSL and not supported.

#ifndef SSSocket_hpp
#define SSSocket_hpp

#include <string>
#include <vector>

#ifdef _WIN32
    #include <windows.h>
    #include <winsock.h>
#else
    #include <sys/socket.h>
    #include <netinet/in.h>
    typedef int SOCKET;                 // Native socket data type
    constexpr int INVALID_SOCKET = -1;  // Native invalid socket value
#endif

using namespace std;

// Represents an IPv4 address.
// Implemented as wrapper around platform-native IPv4 address struct.

struct SSIP
{
    struct in_addr addr;   // Native IPv4 address; internals are platform-dependent!
    
    // Constructors
    
    SSIP ( void );
    SSIP ( const string &str );
    SSIP ( const struct in_addr &add );
    SSIP ( const uint32_t val );
    
    // Returns IP address as 32-bit unsigned integer on all platforms
    
    operator uint32_t() const;

    // Converts IP address to/from dotted notation (like "192.168.0.1")
    
    string toString ( void );
    static SSIP fromString ( const string &str );
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
    
    // Convert host names to IPv4 address and vice-versa; get IPv4 addresses of all local interfaces
    
    static vector<SSIP> hostNameToIPs ( const string &hostname );
    static string IPtoHostName ( const SSIP &ip );
    static vector<SSIP> getLocalIPs ( void );
    
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
    int writeUDPSocket ( void *data, int size, SSIP destIP, unsigned short destPort );
    int readUDPSocket ( void *data, int size, SSIP &senderIP, int timeout_ms );
    bool isUDPSocket ( void );
};

#endif /* SSSocket_hpp */
