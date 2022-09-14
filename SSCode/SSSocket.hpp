// SSSocket.hpp
// SSCore
//
// Created by Tim DeBenedictis on 9/12/22.
// Copyright © 2022 Southern Stars Group, LLC. All rights reserved.
//
// This class implements low-level IPv4 network TCP and UDP socket communication.
// On Windows, make sure to link with WSOCK32.LIB!

#ifndef SSSocket_hpp
#define SSSocket_hpp

#include <string>
#include <vector>

#ifdef _WINDOWS
    #include <windows.h>
    #include <winsock.h>
#else
    #include <sys/socket.h>
    typedef int SOCKET;                 // Native socket data type
    constexpr int INVALID_SOCKET = -1;  // Native invalid socket value
#endif

using namespace std;

class SSSocket
{
protected:
    SOCKET _socket = INVALID_SOCKET;    // Native socket
    
public:
    typedef struct in_addr IPAddress;   // 32-bit numerical IPv4 address

    // Constructors and destructor
    
    SSSocket ( void ) { _socket = INVALID_SOCKET; }
    SSSocket ( SOCKET s ) { _socket = s; }
    ~SSSocket ( void ) { closeSocket(); }
    
    // Accessor returns native socket
    
    SOCKET getSocket ( void ) { return _socket; }
    
    // Socket library initialiation and cleanup - call once at program startup and exit
    
    static bool initialize ( void );
    static void finalize ( void );
    
    // Convert host names to IPv4 address and vice-versa; get IPv4 addresses of all local interfaces
    
    static vector<IPAddress> hostNameToIPs ( const string &hostname, bool useDNS );
    static string IPtoHostName ( IPAddress ip, bool useDNS );
    static vector<IPAddress> getLocalIPs ( void );
    
    // TCP (connection-oriented) sockets: open, read, write, close
    
    bool openSocket ( IPAddress serverIP, unsigned short port, int timeout_ms );
    bool socketOpen ( void );
    int writeSocket ( void *data, int size );
    int readSocket ( void *data, int size );
    void closeSocket ( void );
    bool getRemoteIP ( IPAddress &peerIP );

    // TCP server sockets: open, listen, accept incoming connections
    
    bool serverOpenSocket ( IPAddress serverIP, unsigned short port, int maxConnectioms );
    bool serverConnectionPending ( void );
    SSSocket serverAcceptConnection ( void );
    
    // UDP (connectionless) sockets: open, read, write
    
    bool openUDPSocket ( IPAddress localIP, unsigned short localPort );
    int writeUDPSocket ( void *data, int size, IPAddress destIP, unsigned short destPort );
    int readUDPSocket ( void *data, int size, IPAddress &senderIP, int timeout_ms );
    bool isUDPSocket ( void );
};

#endif /* SSSocket_hpp */
