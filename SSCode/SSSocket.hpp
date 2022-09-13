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
    typedef int SOCKET;
    constexpr int INVALID_SOCKET = -1;
#endif

using namespace std;

class SSSocket
{
protected:
    SOCKET _socket = INVALID_SOCKET;
    
public:
    typedef in_addr IPAddress;

    // Constructor and destructor
    
    SSSocket ( void ) { _socket = INVALID_SOCKET; }
    SSSocket ( SOCKET s ) { _socket = s; }
    ~SSSocket ( void ) { closeSocket(); }
    
    static bool initialize ( void );
    static void finalize ( void );
    
    static vector<IPAddress> hostNameToIPs ( const string &hostname, bool useDNS );
    static string IPtoHostName ( IPAddress ip, bool useDNS );
    static IPAddress getLocalIP ( void );
    static vector<IPAddress> getLocalIPs ( void );
    
    bool getRemoteIP ( IPAddress &peerIP );
    bool openSocket ( IPAddress serverIP, unsigned short port, int timeout );
    bool socketOpen ( void );
    int writeSocket ( void *data, int size );
    int readSocket ( void *data, int size );
    void closeSocket ( void );
    
    bool serverOpenSocket ( IPAddress serverIP, unsigned short port, int maxConnectioms );
    bool serverConnectionPending ( void );
    SSSocket serverAcceptConnection ( void );
    
    bool openUDPSocket ( IPAddress localIP, unsigned short localPort );
    int writeUDPSocket ( void *data, int size, IPAddress destIP, unsigned short destPort );
    int readUDPSocket ( void *data, int size, IPAddress &senderIP, long timeout );
};

#endif /* SSSocket_hpp */
