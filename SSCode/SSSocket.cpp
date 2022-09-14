// SSSocket.cpp
// SSCore
//
// Created by Tim DeBenedictis on 9/12/22.
// Copyright © 2022 Southern Stars Group, LLC. All rights reserved.
//
// This class implements basic IPv4 network TCP and UDP socket communication.
// TCP server sockets are supported. IPv6 and SSL and not supported.
// On Windows, make sure to link with WSOCK32.LIB!
// Based primarily on example code from Beej's Guide to Network Programming:
// https://beej.us/guide/bgnet/html/

#include "SSSocket.hpp"
#include <string.h>

#ifdef _WINDOWS

typedef int socklen_t;
typedef ULONG in_addr_t;

// Initializes socket communcation module.
// Returns true if successful or false on failure.
// Call once at application startup.

bool SSSocket::initialize ( void )
{
    WORD    wVersionRequested;
    WSADATA wsaData;

    // Load and initialize the WinSock DLL. Make sure we have WinSock version 2
    
    wVersionRequested = MAKEWORD ( 2, 0 );

    if ( WSAStartup ( wVersionRequested, &wsaData ) )
        return false;

    if ( LOBYTE ( wsaData.wVersion ) != 2 || HIBYTE ( wsaData.wVersion ) != 0 )
    {
        WSACleanup();
        return false;
    }

    return ( true );
}

// Finalizes socket communcation module.
// Call once before application shutdown.

void SSSocket::finalize ( void )
{
    WSACleanup();
}

#else   // MacOS/Linux implementation

#include <arpa/inet.h>
#include <ifaddrs.h>
#include <netdb.h>
#include <sys/ioctl.h>
#include <unistd.h>

#define ioctlsocket(s,p,o) ioctl(s,p,o)
#define closesocket(s) close(s)
#define SOCKET_ERROR -1

bool SSSocket::initialize ( void )
{
    return true;    // nothing to do on MacOS/Linux!
}

void SSSocket::finalize ( void )
{
    // Nothing to do on MacOS/Linux
}

#endif

// Returns vector of IPv4 addresses corresponding to a network host name string.
// If (useDNS) is false, the function will try to parse the host name string
// as an IPv4 address in dotted form, like "192.168.1.1".
// If (useDNS) is true, the function will attempt to resolve the host name as a fully-
// qualified domain name (like "www.southernstars.com") using DNS, if it cannot be
// parsed as an IP address in dotted form.

vector<SSSocket::IPAddress> SSSocket::hostNameToIPs ( const string &hostName, bool useDNS )
{
    int               i, nIPs = 0;
    in_addr_t         lIP;
    struct hostent    *pHostEnt;
    vector<IPAddress> vIPs;
    
    // First, try to parse the host name string as an IPv4 dotted address
    // (e.g. "192.168.1.1").  If we fail, then try to resolve the host name
    // string using DNS, if so requested by the caller. ***/

    lIP = inet_addr ( hostName.c_str() );
    if ( lIP != INADDR_NONE )
    {
        IPAddress addr;
        addr.s_addr = lIP;
        vIPs.push_back ( addr );
        return vIPs;
    }

    // If requested, use DNS to resolve all IP addresses,
    // up to the maximum number passed in the input array,
    // from the host name.
    
    if ( useDNS )
    {
        pHostEnt = gethostbyname ( hostName.c_str() );
        if ( pHostEnt == NULL )
            return vIPs;

        nIPs = pHostEnt->h_length / sizeof ( struct in_addr );
        for ( i = 0; i < nIPs; i++ )
            vIPs.push_back ( *( (IPAddress *) pHostEnt->h_addr_list[i] ) );
    }

    return vIPs;
}

// Determines the host name corresponding to an IPv4 address.
// If (useDNS) is false, this method will return the numerical value
// of the IP address specified in dotted form, like "192.168.0.1".
// If (useDNS) is true, this method will attempt to determine the fully-
// qualified domain name (like "cnn.com") corresponding to the IP address.
// If the method fails to find the host's fully-qualified domain name,
// it will return the dotted form of the IP address (e.g. "192.168.1.1").

string SSSocket::IPtoHostName ( IPAddress address, bool useDNS )
{
    if ( useDNS )
    {
        struct hostent *pHostEnt = gethostbyaddr ( (char *) &address, sizeof ( address ), AF_INET );
        if ( pHostEnt != nullptr )
            return string ( pHostEnt->h_name );
    }
    else
    {
        char *pszIPAddress = inet_ntoa ( address );
        if ( pszIPAddress != nullptr )
            return string ( pszIPAddress );
    }
    
    return string ( "" );
}

#ifdef _WINDOWS

// Returns the IPv4 address corresponding to the local machine's host name.

vector<SSSocket::IPAddress> SSSocket::getLocalIPs ( void )
{
    char szHost[256] = { 0 };
    
    if ( gethostname ( szHost, sizeof ( szHost ) ) == 0 )
        return hostNameToIPs ( string ( szHost ), true );
    else
        return vector<IPAddress> ( 0 );
}

#else

// Returns vector of IPv4 addresses of all network interfaces on the local system.

vector<SSSocket::IPAddress> SSSocket::getLocalIPs ( void )
{
    vector<IPAddress> vIPs;
    struct ifaddrs *ifa = NULL, *pifa = ifa;
    int nIPs = 0;
    
    // Enumerate interfaces

    if ( getifaddrs ( &ifa ) != 0 )
        return vIPs;

    // Walk the linked list of interfaces

    for ( pifa = ifa; pifa != NULL; pifa = pifa->ifa_next )
    {
        if ( pifa->ifa_addr->sa_family != AF_INET )
            continue;
        
        vIPs.push_back ( ( (sockaddr_in *) pifa->ifa_addr )->sin_addr );
        nIPs++;
    }
    
    return vIPs;
}

#endif

// Obtains the IP address of the remote (i.e. peer) machine to which
// this TCP socket is connected. Will not work for UDP sockets.
// Returns true if successful, or false on failure.

bool SSSocket::getRemoteIP ( IPAddress &peerIP )
{
    struct sockaddr_in address;
    socklen_t nLength = sizeof ( address );

    if ( getpeername ( _socket, (struct sockaddr *) &address, &nLength ) == 0 )
    {
        peerIP = address.sin_addr;
        return true;
    }
    
    return false;
}

// Opens a TCP socket connection with a remote server with IPv4 address (serverIP)
// at TCP port number (wPort). Maximum number of milliseconds to wait for server
// to accept connection (nTimeout).
// If the remote server accepts the connection, the function returns true.
// On failure, or if the server refuses to accept the connection within timeout,
// the function returns false.
// To wait an indefinite amount of time for the connection to be accepted,
// pass a value <= 0 for the timeout parameter.

bool SSSocket::openSocket ( IPAddress serverIP, unsigned short wPort, int nTimeout )
{
    int                   nResult;
    SOCKET                nSocket;
    long                  dwValue;
    int                   nError;
    socklen_t             nSize;
    struct sockaddr_in    address;
    fd_set                writefds;
    struct timeval        timeout;
    
    address.sin_family = AF_INET;
    address.sin_port = htons ( wPort );
    address.sin_addr = serverIP;
    memset(&(address.sin_zero), 0, sizeof ( address.sin_zero ) );

    nSocket = socket ( AF_INET, SOCK_STREAM, 0 );
    if ( nSocket == SOCKET_ERROR )
        return false;

    if ( nTimeout > 0 )
    {
        // Put socket into non-blocking I/O mode
        
        dwValue = 1;
        nResult = ioctlsocket( nSocket, FIONBIO, &dwValue );
        if ( nResult == SOCKET_ERROR )
        {
            closesocket ( nSocket );
            return false;
        }
        
        // Now connect - should return SOCKET_ERROR and WSAGetLastError() should return WSAEWOULDBLOCK
        
        nResult = connect ( nSocket, (struct sockaddr *) &address, sizeof ( struct sockaddr ) );

        // Wait for timeout for socket to become writable, or for an error
        
        FD_ZERO ( &writefds );
        FD_SET ( nSocket, &writefds );
        
        timeout.tv_sec = nTimeout / 1000;
        timeout.tv_usec = 1000 * ( nTimeout % 1000 );
        
        nResult = select ( nSocket + 1, NULL, &writefds, NULL, &timeout );
        if ( nResult < 1 )
        {
            closesocket ( nSocket );
            return false;
        }
        
        // Check for socket error conditions
        
        nSize = sizeof ( nError );
        nResult = getsockopt ( nSocket, SOL_SOCKET, SO_ERROR, &nError, &nSize );
        if ( nResult == SOCKET_ERROR || nError != 0 )
        {
            closesocket ( nSocket );
            return false;
        }
        
        // Put the socket back into blocking I/O mode
        
        dwValue = 0;
        nResult = ioctlsocket ( nSocket, FIONBIO, &dwValue );
        if ( nResult < 0 )
        {
            closesocket ( nSocket );
            return false;
        }
    }
    else
    {
        // Connect to socket - will block until remote server accepts
        
        nResult = connect ( nSocket, (struct sockaddr *) &address, sizeof ( struct sockaddr ) );
        if ( nResult < 0 )
        {
            closesocket ( nSocket );
            return false;
        }
    }
    
    _socket = nSocket;
    return true;
}

// Determines whether the peer at the remote end of a TCP socket connection
// has closed the connection.
// Returns true if the remote peer has not yet closed the socket connection,
// or if there is any data on the connection which has not yet been received.
// Returns false if the remote peer has closed the connection and all data
// on the connection has been received.

bool SSSocket::socketOpen ( void )
{
    char   cData = 0;
    int    nResult = 0;
    unsigned long   lFlag = 0;

    if ( _socket == INVALID_SOCKET )
        return false;
    
    // The "right" way to do this is rather obscure.  First put the socket into
    // non-blocking mode and call recv() to see if there's any data waiting to be
    // read, without actually reading it.

    lFlag = true;
    if ( ioctlsocket ( _socket, FIONBIO, &lFlag ) == 0 )
    {
        nResult = (int) recv ( _socket, &cData, 1, MSG_PEEK );

        // Finally put the socket back into blocking mode. We have to do it this way
        // because select() and ioctlsocket() only tell us if there's unread data on the
        // connection, not whether the connection is still closed.

        lFlag = false;
        ioctlsocket ( _socket, FIONBIO, &lFlag );
    }

    // If the connection has been closed and all data received, recv() returns zero.
    // Otherwise, recv() returns the number of bytes it could have read (if there's
    // any data remaining in the buffer), or an error code indicating that the recv()
    // call would block (because there's nothing in the buffer but the remote peer
    // hasn't yet closed the connection).

    return nResult == 0 ? false : true;
}

// Writes data to this TCP socket connection.
// Pointer to buffer containing data to write is (data).
// Number of bytes of data to write is (size).
// Returns the number of bytes actually written to the socket,
// if successful, or SOCKET_ERROR (-1) on failure.
// For UDP sockets, see writeUDPSocket().

int SSSocket::writeSocket ( void *data, int size )
{
    int nResult;
    int nBytesWritten = 0;

    do
    {
        nResult = (int) send ( _socket, (char *) data + nBytesWritten, size - nBytesWritten, 0 );
        if ( nResult == -1 )
            break;

        nBytesWritten += nResult;
    }
    while ( nBytesWritten < size );

    return nResult == SOCKET_ERROR ? SOCKET_ERROR : nBytesWritten;
}

// Reads data from this TCP socket connection, or determines the number of bytes
// currently available to read from the connection.
// Pointer to buffer to receive data is (data).
// Size of buffer, i.e. maximum number of bytes to read, is (size).
// The function returns the number of bytes read from the socket connection.
// If the (data) parameter is NULL, the function determines the number of
// bytes currently available to read, rather than actually reading the data.
// For UDP sockets, see readUDPSocket().

int SSSocket::readSocket ( void *data, int size )
{
    int  nResult;
    int  nBytesRead = 0;
    unsigned long  lBytesToRead;

    do
    {
        // First, use the ioctlsocket() function to determine how many bytes
        // are available to be read from the socket.

        nResult = ioctlsocket ( _socket, FIONREAD, &lBytesToRead );
        if ( nResult == 0 )
        {
            // If there's nothing to read, stop now.  We do this
            // to prevent a subsequent call to recv() from blocking.

            if ( lBytesToRead == 0 )
                break;

            // If we have bytes available, but we've not been given a buffer
            // to read them into, return the number of bytes available without
            // actually reading them.

            if ( data == nullptr )
            {
                nBytesRead = (int) lBytesToRead;
                break;
            }
            else
            {
                // If we do have a buffer, actually read the available data
                // into it.  First make sure we don't overflow the buffer.
                // Then read into it, and break if we run into an error
                // condition or if the remote peer closes the connection.

                if ( lBytesToRead + nBytesRead > size )
                    lBytesToRead = size - nBytesRead;

                nResult = (int) recv ( _socket, (char *) data + nBytesRead, lBytesToRead, 0 );
                if ( nResult < 1 )
                    break;

                nBytesRead += nResult;
            }
        }

        // Repeat until we've filled the buffer provided, or run out of data.
    }
    while ( nBytesRead < size );

    // Return the number of bytes read from the socket connection,
    // if succesful, or an error code on failure.

    return nResult == SOCKET_ERROR ? SOCKET_ERROR : nBytesRead;
}

// Closes a socket connection and frees associated resources, and invalidates
// this SSSocket object's native socket handle/file descriptor.
// If ths is a TCP socket, the peer on the remote end of the connection
// can determine that the connection has been closed by calling socketOpen().

void SSSocket::closeSocket ( void )
{
    shutdown ( _socket, 2 );
    closesocket ( _socket );
    _socket = INVALID_SOCKET;
}

// Opens a TCP socket and places it a "listen" state, waiting for incoming connections
// from other network clients.
// IPv4 address of local interface on which to open server socket is (serverIP).
// TCP port number on which to listen for incoming connections is (wPort).
// Maximum number of incoming connections which will be accepted is (nMaxConnections).
// Returns a pointer to the open socket, if successful, or false on failure.
// The (serverIP) parameter must correspond to a valid local network interface; see getLocalIPs().

bool SSSocket::serverOpenSocket ( IPAddress serverIP, unsigned short wPort, int nMaxConnections )
{
    int                nResult;
    SOCKET             nSocket;
    struct sockaddr_in address;
    struct in_addr     addr = { serverIP };
    
    address.sin_family = AF_INET;
    address.sin_port = htons ( wPort );
    address.sin_addr = addr;
    memset(&(address.sin_zero), 0, sizeof ( address.sin_zero ) );

    nSocket = socket ( AF_INET, SOCK_STREAM, 0 );
    if ( nSocket == -1 )
        return false;

    nResult = ::bind ( nSocket, (struct sockaddr *) &address, (socklen_t) sizeof ( address ) );
    if ( nResult == -1 )
    {
        closesocket ( nSocket );
        return false;
    }

    nResult = listen ( nSocket, nMaxConnections );
    if ( nResult == -1 )
    {
        closesocket ( nSocket );
        return false;
    }

    _socket = nSocket;
    return true;
}

// Returns true if there are any incoming TCP connections pending
// on this server socket, or false if no connections are pending.
// To accept a pending connection, call serverAcceptConnection().
// Only call this function on a server socket that has been placed in a
// listening state, i.e. one that has been created with serverOpenSocket().

bool SSSocket::serverConnectionPending ( void )
{
    int             nResult;
    fd_set          readfds;
    struct timeval  tv;

    tv.tv_sec = 0;
    tv.tv_usec = 0;

    FD_ZERO ( &readfds );
    FD_SET ( _socket, &readfds );

    nResult = select ( _socket + 1, &readfds, NULL, NULL, &tv );
    return nResult == 1 ? true : false;
}

// Accepts an incoming connection on a listening TCP server socket.
// If this TCP server can accept the incoming connection request, the function returns
// the connected socket.  Otherwise, it returns INVALID_SOCKET.
// This socket must have been placed in the listening state; see serverOpenSocket().
//
// If there are no connections pending on the server socket, this function will
// block until an incoming connection request is made. To determine if there are
// any incoming connection requests pending on the socket, call serverConnectionPending()
// before calling this function.
//
// After accepting a connection, this listening server socket may still accept further
// incoming connection requests; the server is not required to wait until the remote
// client disconnects.  Once a connection has been established, you can determine the
// IP address of the machine on the remote end of the connection using getRemoteIP()
// on the socket returned by serverAcceptConnection(). You can send and receive data
// from the remote network client using with the returned socket's writeSocket()
// and readSocket() methods, and close the connectio with its closeSocket() method.

SSSocket SSSocket::serverAcceptConnection ( void )
{
    SOCKET              nResult;
    struct sockaddr_in  address;
    socklen_t           nSize = sizeof ( address );

    nResult = accept ( _socket, (struct sockaddr *) &address, &nSize );
    return SSSocket ( nResult );
}

// Opens a connectionless datagram socket, and optionally binds it to a local IPv4
// address and UDP port. The local IPv4 address on which to bind the socket is (localIP).
// The UDP port on which to bind, if nonzero, is (wPort).
// If successful, the function returns true. On failure, it returns false.
//
// Typically, the IP address and port number are only needed to listen for incoming
// UDP packets from a remote sender. The remote sender would send to the IP address
// and UDP port specified here. If you are only sending UDP packets, you ordinarily
// don't need to bind on a specific local IP address or port, because the receiver
// doesn't ordinarily care what IP address or port the UDP packets comes from.
//
// When finished using the socket, dispose of it with closeSocket().

bool SSSocket::openUDPSocket ( IPAddress ipAddress, unsigned short wPort )
{
    int                 nSocket = 0;
    struct sockaddr_in  address = { 0 };
    
    nSocket = socket ( PF_INET, SOCK_DGRAM, 0 );
    if ( nSocket == -1 )
        return false;
    
    address.sin_family = PF_INET;
    address.sin_port = htons ( wPort );
    address.sin_addr = ipAddress;
    memset(&(address.sin_zero), 0, sizeof ( address.sin_zero ) );
    
    if ( ipAddress.s_addr && wPort )
    {
        if ( ::bind ( nSocket, (struct sockaddr *) &address, (socklen_t) sizeof ( address ) ) == -1 )
        {
            closesocket ( nSocket );
            return false;
        }
    }
    
    _socket = nSocket;
    return true;
}

// Sends data over a connectionless datagram (UDP) socket.
// Buffer containing data to send over UDP is (lpvData)
// Number of bytes of data to send is (lLength)
// IPv4 address of intended destination is (destIP).
// UDP port number on which to send data on destination is (wDestPort).
// Returns the number of bytes actually sent over the UDP connection,
// or SOCKET_ERROR on failure.

int SSSocket::writeUDPSocket ( void *lpvData, int lLength, IPAddress destIP, unsigned short wDestPort )
{
    int        nResult;
    int        nBytesWritten = 0;
    struct sockaddr_in address = { 0 };
    struct in_addr addr = { destIP };
    
    address.sin_family = PF_INET;
    address.sin_port = htons ( wDestPort );
    address.sin_addr = addr;
    memset(&(address.sin_zero), 0, sizeof ( address.sin_zero ) );
    
    do
    {
        nResult = (int) sendto ( _socket, (char *) lpvData + nBytesWritten, lLength - nBytesWritten, 0, (sockaddr *) &address, sizeof ( address ) );
        if ( nResult == -1 )
            break;
        
        nBytesWritten += nResult;
    }
    while ( nBytesWritten < lLength );
    
    return nResult == SOCKET_ERROR ? SOCKET_ERROR : nBytesWritten;
}

// Recieves data from a connectionless datagram (UDP) socket.
// Buffer to receive data from UDP socket is (lpvData).
// Maximum number of bytes to recieve is (lLength), i.e. size of data buffer in bytes.
// IP address of remote sender is returned in (senderIP)
// Timeout value in milliseconds (timeout) is used if nonzero; see below.
// The function returns the number of bytes actually recieved before timeout,
// or SOCKET_ERROR on failure.
// This function will return when the bytes have been recieved from a remote sender;
// or when the specified number of milliseconds have elapsed (if timeout is non-zero).
// If a non-zero timeout has been specified, and the function has not recieved any data after
// timeout has elapsed, the function will return zero.
// If the timeout is zero or negative, the function will block "forever" waiting for data.
// If the sender transmits more data than will fit into the recieve buffer,
// that extra data will be discarded.

int SSSocket::readUDPSocket ( void *lpvData, int lLength, IPAddress &senderIP, int timeout_ms )
{
    int             nResult;
    struct          sockaddr_in address;
    socklen_t       addrlen = sizeof ( address );
    struct timeval  tv;
    
    // Set recieve timeout to the specified number of milliseconds
    
    if ( timeout_ms > 0 )
    {
        tv.tv_sec = timeout_ms / 1000;
        tv.tv_usec = 1000 * ( timeout_ms % 1000 );
    }
    else
    {
        tv.tv_sec = 1000000;
        tv.tv_usec = 0;
    }
    
    nResult = setsockopt ( _socket, SOL_SOCKET, SO_RCVTIMEO, (char *) &tv, sizeof ( tv ) );
    if ( nResult == -1 )
        return SOCKET_ERROR;
    
    // Wait to recieve UDP message, until timeout
    
    nResult = (int) recvfrom ( _socket, (char *) lpvData, lLength, 0, (sockaddr *) &address, &addrlen );
    if ( nResult == -1 )
    {
        if ( errno == EAGAIN || errno == EWOULDBLOCK )
            return 0;
        else
            return SOCKET_ERROR;
    }
    
    senderIP = address.sin_addr;
    return nResult;
}

// Returns true if this socket is a connectionless UDP socket, or false otherwise.

bool SSSocket::isUDPSocket ( void )
{
    int nType = 0;
    socklen_t nSize = sizeof ( nType );
    
    int nResult = getsockopt ( _socket, SOL_SOCKET, SO_TYPE, &nType, &nSize );
    if ( nResult != SOCKET_ERROR && nType == SOCK_DGRAM )
        return true;
    
    return false;
}
