// SSSocket.cpp
// SSCore
//
// Created by Tim DeBenedictis on 9/12/22.
// Copyright © 2022 Southern Stars Group, LLC. All rights reserved.
//
// This class implements basic IPv4 network TCP and UDP socket communication.
// TCP server sockets are supported. IPv6 and SSL and not supported.
// On Windows, make sure to link with WS2_32.LIB!
// Based primarily on example code from Beej's Guide to Network Programming:
// https://beej.us/guide/bgnet/html/

#include <string.h>

#include "SSSocket.hpp"
#include "SSUtilities.hpp"

#ifdef _MSC_VER

typedef int socklen_t;
typedef ULONG in_addr_t;

// Initializes socket communcation module.
// Returns true if successful or false on failure.
// Call once at application startup; SSSocket constructor also calls initialize().
// SSSocket constructor also calls initialize(), if not already initialized.

static bool _initialized = false;

bool SSSocket::initialize ( void )
{
    if ( _initialized )
        return true;

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

    _initialized = true;
    return true;
}

// Finalizes socket communcation module.
// Call once before application shutdown.

void SSSocket::finalize ( void )
{
    if ( _initialized )
        WSACleanup();
    _initialized = false;
}

#else   // MacOS/Linux implementation

#include <arpa/inet.h>
#include <errno.h>
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

// SSIP default constructor sets IPv4 address to 0.0.0.0

SSIP::SSIP ( void )
{
    ipv6 = false;
    memset ( &add6, 0, sizeof ( add6 ) );
}

// SSIP constructor from IPv4 or APv6 address string in dotted notation
// (like "192.168.0.1" or "2001:0db8:c9d2:aee5:73e3:934a:a5ae:9551").
// Sets address to 0.0.0.0 if string invalid.

SSIP::SSIP ( const string &str ) : SSIP()
{
    if ( inet_pton ( AF_INET, str.c_str(), &addr ) )
        ipv6 = false;
    else if ( inet_pton ( AF_INET6, str.c_str(), &add6 ) )
        ipv6 = true;
}

// SSIP constructor from platform-native IPv4 address struct

SSIP::SSIP ( const struct in_addr &add )
{
    addr = add;
    ipv6 = false;
}

// SSIP constructor from platform-native IPv6 address struct

SSIP::SSIP ( const struct in6_addr &add )
{
    add6 = add;
    ipv6 = true;
}

// SSIP constructor from unsigned 32-bit integer value;
// generates IPv4 address.

SSIP::SSIP ( const uint32_t val )
{
    addr.s_addr = val;
    ipv6 = false;
}

// Returns string containing IPv4 or IPv6 address in dotted notation
// (like "192.168.0.1" or "2001:0db8:c9d2:aee5:73e3:934a:a5ae:9551")

string SSIP::toString ( void )
{
    char str[256] = { 0 };
    if ( inet_ntop ( ipv6 ? AF_INET6 : AF_INET, &addr, str, sizeof ( str ) ) != nullptr )
        return string ( str );
    else
        return "";
}

// Factory method constructs IPv4 or IPv6 address from string in dotted notation.
// Returned address is 0.0.0.0 if string invalid.

SSIP SSIP::fromString ( const string &str )
{
    return SSIP ( str );
}

// Returns IPv4 address as 32-bit unsigned integer on all platforms
// If this is an IPv6 address, returns zero.

SSIP::operator uint32_t() const
{
    return ipv6 ? 0 : addr.s_addr;
}

// Populates sockaddr_in (addr) or sockaddr_in6 (add6) struct with contents of IP address and port.
// Returns pointer the provided addr or add6 struct, depending on whether the provided SSIP is a v4 or v6 address.
// Returns length of the filled-in addr or addd6 struct as the final parameter, len.
// Yes, we know about struct sockaddr_storage, but this routine solves the same problem more cleanly.

static sockaddr *fill_sockaddr ( const SSIP &ip, uint16_t port, sockaddr_in *addr, sockaddr_in6 *add6, socklen_t *len )
{
    if ( ip.ipv6 )
    {
        add6->sin6_family = AF_INET6;
        add6->sin6_port = htons ( port );
        add6->sin6_addr = ip.add6;
    }
    else
    {
        addr->sin_family = AF_INET;
        addr->sin_port = htons ( port );
        addr->sin_addr = ip.addr;
    }

    *len = ip.ipv6 ? sizeof ( sockaddr_in6 ) : sizeof ( sockaddr_in );
    return ip.ipv6 ? (sockaddr *) add6 : (sockaddr *) addr;
}

// Attempts to resolve a fully-qualified domain name (like "www.southernstars.com") using DNS.
// If successul, returns a vector of IPv4 or IPv6 addresses (if argument ipv6 is true)
// corresponding to the host name string. Returns an empty vector on failure.
// TODO: we are returning the same address twice. Why?

vector<SSIP> SSSocket::hostNameToIPs ( const string &hostName, bool ipv6 )
{
    vector<SSIP> vIPs;
    addrinfo hint = { 0 }, *results = nullptr;

    hint.ai_family = ipv6 ? AF_INET6 : AF_INET;

    if ( getaddrinfo ( hostName.c_str(), nullptr, &hint, &results ) == 0 )
    {
        for ( addrinfo *pinfo = results; pinfo != nullptr; pinfo = pinfo->ai_next )
        {
            if ( ipv6 )
            {
                sockaddr_in6 *addr = (sockaddr_in6 *) pinfo->ai_addr;
                vIPs.push_back ( SSIP ( addr->sin6_addr ) );
            }
            else
            {
                sockaddr_in *addr = (sockaddr_in *) pinfo->ai_addr;
                vIPs.push_back ( SSIP ( addr->sin_addr ) );
            }
        }
        freeaddrinfo ( results );
    }

    return vIPs;
}

// Determines the host name corresponding to an IPv4 or IPv6 address using DNS.
// This method will attempt to determine the fully-qualified domain name
// (like "cnn.com") corresponding to the IP address.
// If the method fails to find the host's fully-qualified domain name,
// it will return an empty string.

string SSSocket::IPtoHostName ( const SSIP &ip )
{
    char hostname[1024] = { 0 };
    sockaddr_in addr = { 0 };
    sockaddr_in6 add6 = { 0 };
    socklen_t len = 0;
    sockaddr *add = fill_sockaddr ( ip, 0, &addr, &add6, &len );

    if ( getnameinfo ( add, len, hostname, sizeof ( hostname ), nullptr, 0, NI_NAMEREQD ) == 0 )
        return string ( hostname );
    else    
        return string ( "" );
}

#ifdef _MSC_VER

// Returns the IPv4 or IPv6 address corresponding to the local machine's host name.

vector<SSIP> SSSocket::getLocalIPs ( bool ipv6 )
{
    char szHost[256] = { 0 };
    
    if ( gethostname ( szHost, sizeof ( szHost ) ) == 0 )
        return hostNameToIPs ( string ( szHost ), ipv6 );
    else
        return vector<SSIP> ( 0 );
}

#else

// Returns vector of IPv4 or IPv6 addresses of all network interfaces on the local system.

vector<SSIP> SSSocket::getLocalIPs ( bool ipv6 )
{
    vector<SSIP> vIPs;
    struct ifaddrs *ifa = NULL, *pifa = ifa;
    int nIPs = 0;
    
    // Enumerate interfaces

    if ( getifaddrs ( &ifa ) != 0 )
        return vIPs;

    // Walk the linked list of interfaces

    for ( pifa = ifa; pifa != NULL; pifa = pifa->ifa_next )
    {
        if ( pifa->ifa_addr->sa_family != ( ipv6 ? AF_INET6 : AF_INET ) )
            continue;
        
        if ( ipv6 )
            vIPs.push_back ( ( (sockaddr_in6 *) pifa->ifa_addr )->sin6_addr );
        else
            vIPs.push_back ( ( (sockaddr_in *) pifa->ifa_addr )->sin_addr );

        nIPs++;
    }
    
    return vIPs;
}

#endif

// Obtains the IPv4 or IPv6 address of the remote (i.e. peer) machine to which
// this TCP socket is connected. Will not work for UDP sockets.
// To return IPv6 address, set peer.ipv6 true before calling.
// Returns true if successful, or false on failure.
// TODO: needs testing!

bool SSSocket::getRemoteIP ( SSIP &peer )
{
    sockaddr_in addr = { 0 };
    sockaddr_in6 add6 = { 0 };
    socklen_t len = 0;
    sockaddr *add = fill_sockaddr ( peer, 0, &addr, &add6, &len );

    if ( getpeername ( _socket, add, &len ) == 0 )
    {
        if ( peer.ipv6 )
            peer.add6 = add6.sin6_addr;
        else
            peer.addr = addr.sin_addr;
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

bool SSSocket::openSocket ( SSIP serverIP, uint16_t port, int nTimeout )
{
    int                   nResult;
    SOCKET                nSocket;
    unsigned long         dwValue;
    int                   nError;
    socklen_t             nSize;
    fd_set                writefds;
    struct timeval        timeout;
    sockaddr_in           addr = { 0 };
    sockaddr_in6          add6 = { 0 };
    socklen_t             len = 0;
    sockaddr             *add = fill_sockaddr ( serverIP, port, &addr, &add6, &len );

    nSocket = socket ( serverIP.ipv6 ? AF_INET6 : AF_INET, SOCK_STREAM, 0 );
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
        
        nResult = connect ( nSocket, add, len );

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
        nResult = getsockopt ( nSocket, SOL_SOCKET, SO_ERROR, (char *) &nError, &nSize );
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
        
        nResult = connect ( nSocket, add, len );
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

    // If the socket handle or file descriptor is invalid, return false.
    
    if ( _socket == INVALID_SOCKET )
        return false;
    
    // The test below only works for TCP sockets!
    
    if ( isUDPSocket() )
        return true;
    
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

int SSSocket::writeSocket ( const void *data, int size )
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

bool SSSocket::serverOpenSocket ( SSIP serverIP, uint16_t port, int nMaxConnections )
{
    int                res;
    SOCKET             sock;
    sockaddr_in        addr = { 0 };
    sockaddr_in6       add6 = { 0 };
    socklen_t          len = 0;
    sockaddr           *add = fill_sockaddr ( serverIP, port, &addr, &add6, &len );

    sock = socket ( AF_INET, SOCK_STREAM, 0 );
    if ( sock == INVALID_SOCKET )
        return false;

    res = ::bind ( sock, add, len );
    if ( res == -1 )
    {
        closesocket ( sock );
        return false;
    }

    res = listen ( sock, nMaxConnections );
    if ( res == -1 )
    {
        closesocket ( sock );
        return false;
    }

    _socket = sock;
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

bool SSSocket::openUDPSocket ( SSIP localIP, uint16_t port )
{
    int                sock = 0;
    sockaddr_in        addr = { 0 };
    sockaddr_in6       add6 = { 0 };
    socklen_t          len = 0;
    sockaddr           *add = fill_sockaddr ( localIP, port, &addr, &add6, &len );

    sock = socket ( localIP.ipv6 ? PF_INET6 : PF_INET, SOCK_DGRAM, 0 );
    if ( sock == -1 )
        return false;
    
    // TODO: this will break on IPv6; need a better test to determine if IPv6 address is valid!
    
    if ( localIP && port )
    {
        if ( ::bind ( sock, add, len ) == -1 )
        {
            closesocket ( sock );
            return false;
        }
    }
    
    _socket = sock;
    return true;
}

// Sends data over a connectionless datagram (UDP) socket.
// Buffer containing data to send over UDP is (lpvData)
// Number of bytes of data to send is (lLength)
// IPv4 address of intended destination is (destIP).
// UDP port number on which to send data on destination is (wDestPort).
// Returns the number of bytes actually sent over the UDP connection,
// or SOCKET_ERROR on failure.

int SSSocket::writeUDPSocket ( const void *lpvData, int lLength, SSIP destIP, uint16_t destPort )
{
    int             nResult;
    int             nBytesWritten = 0;
    sockaddr_in     addr = { 0 };
    sockaddr_in6    add6 = { 0 };
    socklen_t       len = 0;
    sockaddr        *add = fill_sockaddr ( destIP, destPort, &addr, &add6, &len );

    do
    {
        nResult = (int) sendto ( _socket, (char *) lpvData + nBytesWritten, lLength - nBytesWritten, 0, add, len );
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

int SSSocket::readUDPSocket ( void *lpvData, int lLength, SSIP &senderIP, int timeout_ms )
{
    int             nResult;
    struct          sockaddr_in address;
    socklen_t       addrlen = sizeof ( address );
    struct timeval  tv;

    // Set recieve timeout to the specified number of milliseconds

#ifdef _MSC_VER
    DWORD dwTimeout = timeout_ms;
    nResult = setsockopt ( _socket, SOL_SOCKET, SO_RCVTIMEO, (char *) &dwTimeout, sizeof ( dwTimeout ) );
#else
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
#endif
    if ( nResult == -1 )
        return SOCKET_ERROR;
    
    // Wait to recieve UDP message, until timeout

    // TODO: add ability to return IPv6 address?
    
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
    
    int nResult = getsockopt ( _socket, SOL_SOCKET, SO_TYPE, (char *) &nType, &nSize );
    if ( nResult != SOCKET_ERROR && nType == SOCK_DGRAM )
        return true;
    
    return false;
}

// SSHTTP constructor parses URL strings like "http://www.southernstars.com:8080/updates/asteroids.txt".
// Communication timeout is milliseconds. All other class members are set to default values.

SSHTTP::SSHTTP ( const string &url, uint32_t timeout )
{
    setURL ( url );
    _timeout = timeout;
    _respCode = 0;
    _contLen = 0;
}

// Destructor closes socket, if open.

SSHTTP::~SSHTTP ( void )
{
    if ( _socket.socketOpen() )
        _socket.closeSocket();
}

// Sets and parses URL for future requests.

void SSHTTP::setURL ( const string &url )
{
    // extract resource type
    
    size_t s = url.find ( "://" );
    _type = s == string::npos ? "" : url.substr ( 0, s );
    
    // extract hostname and/or resource path
    
    _url = url.substr ( s + 3, string::npos );
    size_t t = _url.find ( "/" );
    if ( t == string::npos )
    {
        _host = _url;
        _port = 80;
        _path = "/";
    }
    else    // extract host and port if present
    {
        size_t p = _url.find ( ":" );
        if ( p == string::npos )
        {
            _port = 80;   // Just default port for http
            _host = _url.substr ( 0, t );
        }
        else
        {
            _port = strtoint ( _url.substr ( p + 1, t ) );
            _host = _url.substr ( 0, p );
        }

        // extract resource path
        
        _path = _url.substr ( t, string::npos );
    }
    
    // save original URL
    
    _url = url;
}

// Opens socket to remote server at the specified URL and writes HTTP request header to that socket.
// If postSize == 0, writes a GET request; otherwise, writes a POST request.
// Leaves socket open. Returns number of bytes of header data sent to server.

int SSHTTP::sendRequestHeader ( size_t postSize )
{
    if ( ! _socket.socketOpen() )
    {
        // Parse server IP address from path string, first as dotted form,
        // then as fully-qualified domain name string using DNS.
        
        SSIP addr ( _host );
        vector<SSIP> addrs;
        if ( addr )
            addrs.push_back ( addr );
        else
            addrs = SSSocket::hostNameToIPs ( _host );
        
        // Try connecting to all IP addresses and save the first that succeeds.
        
        for ( int i = 0; i < addrs.size(); i++ )
            if ( _socket.openSocket ( addrs[i], _port, _timeout ) )
                break;
    }

    if ( ! _socket.socketOpen() )
        return 0;

    // format HTTP POST or GET command.
    
    string header;
    if ( postSize > 0 && ! _contType.empty() )
        header = format ( "POST %s HTTP/1.1\r\nHost: %s\r\nContent-Length: %ld\r\nContent-Type: %s\r\n\r\n", _path.c_str(), _host.c_str(), postSize, _contType.c_str() );
    else if ( postSize > 0 )
        header = format ( "POST %s HTTP/1.1\r\nHost: %s\r\nContent-Length: %ld\r\n\r\n", _path.c_str(), _host.c_str(), postSize );
    else
        header = format ( "GET %s HTTP/1.1\r\nHost: %s\r\n\r\n", _path.c_str(), _host.c_str() );

    // Sender header to the server via socket

    return _socket.writeSocket( &header[0], (int) header.length() );
}

// Returns value string corresponding to specified keyword in HTTP response header.
// If keyword is not found, returns empty string.

string SSHTTP::getHeaderValue ( const string &key )
{
    size_t keylen = key.length();
    size_t start = _respHead.find ( key );
    if ( start == string::npos )
        return "";
    
    start += keylen;
    size_t end = _respHead.find ( "\r\n", start );
    if ( end == string::npos )
        return "";
    
    return trim ( _respHead.substr ( start, end - start ) );
}

// Reads HTTP response headers from server socket into _respHead, until double newline is received,
// or timeout occurs. If complete header is received, the method parses response code, content length,
// content type, etc. from the header. Returns number of bytes read from server.

int SSHTTP::readResponseHeader ( void )
{
    int     n = 0;
    double  timeout = clocksec() + _timeout / 1000.0;
    string  header;
    
    while ( clocksec() < timeout )
    {
        char c;
        int bytes = _socket.readSocket ( &c, 1 );
        if ( bytes < 1 )
        {
            msleep ( 1 );
            continue;
        }

        header += c;
        n += bytes;
        timeout = clocksec() + _timeout / 1000.0;
        if ( endsWith ( header, "\n\n" ) || endsWith ( header, "\r\n\r\n" ) )
            break;
    }
    
    _respHead = header;
    if ( clocksec() < timeout )
    {
        _respCode = strtoint ( getHeaderValue ( "HTTP/1.1" ) );
        _contLen = strtoint ( getHeaderValue ( "Content-Length:" ) );
        _contType = getHeaderValue ( "Content-Type:" );
        _location = getHeaderValue ( "Location:" );
        _date = SSDate ( "%a, %d %b %Y %H:%M:%S", getHeaderValue ( "Date:" ) );
    }
    else
    {
        _respCode = 0;
        _contLen = 0;
        _contType = "";
        _location = "";
        _date = SSDate();
    }
    
    _content = vector<char> ( 0 );
    return n;
}

// Reads content from server into internal content buffer.
// Returns number of bytes read from server.
// TODO: what if _contentLength is not specified?

int SSHTTP::readContent ( void )
{
    // Make sure socket is open and content length is valid
    
    if ( ! _socket.socketOpen() || _contLen < 1 )
        return 0;
    
    // read content until timeout, error, or content buffer filled.
    
    int pos = 0;
    double timeout = clocksec() + _timeout / 1000.0;
    while ( clocksec() < timeout )
    {
        _content = vector<char> ( _contLen );
        memset ( &_content[0], 0, _contLen );
        int bytes = _socket.readSocket ( &_content[pos], (int) _contLen - pos );
        if ( bytes < 0 )
            break;
        
        if ( bytes < 1 )
        {
            msleep ( 1 );
            continue;
        }
        
        timeout = clocksec() + _timeout / 1000.0;

        pos += bytes;
        if ( pos == _contLen )
            break;
    }

    // Return number of bytes actually read
    
    return pos;
}

// Writes content up to len bytes in size to server.
// Returns number of bytes written to server.

int SSHTTP::sendContent ( const void *content, size_t len )
{
    if ( ! _socket.socketOpen() || len < 1 )
        return 0;
    
    int bytes = _socket.writeSocket ( content, (int) len );
    if ( bytes == len && _socket.socketOpen() )
        bytes += _socket.writeSocket ( "\r\n", 2 );
    
    return bytes;
}

// GETs content from server to internal _content buffer.
// Returns HTTP response code or 0 on failure.

int SSHTTP::get ( void )
{
    _respCode = 0;
    
    if ( sendRequestHeader() > 1 )
        if ( readResponseHeader() > 1 )
            readContent();
    
    return _respCode;
}

// POSTs content in postData buffer of postSize length in bytes.
// Returns HTTP response code or 0 on failure.

int SSHTTP::post ( const void *postData, size_t postSize )
{
    _respCode = 0;
    
    if ( sendRequestHeader ( postSize ) > 1 )
        if ( sendContent ( postData, postSize ) > 1 )
            if ( readResponseHeader() > 1 )
                readContent();
    
    return _respCode;
}

// POSTs content that has previously been set with setContent().

int SSHTTP::post ( void )
{
    return post ( &_content[0], _content.size() );
}

// Copies content of size bytes into internal _content vector

void SSHTTP::setContent ( const void *content, size_t size )
{
    _content = vector<char> ( size );
    memcpy ( &_content[0], content, size );
}

// Returns string copied from internal _content buffer

string SSHTTP::getContentString ( void )
{
    _content.push_back ( '\0' );
    string s ( &_content[0] );
    _content.pop_back();
    return s;
}

// Copies string into internal _content buffer

void SSHTTP::setContentString ( const string &s )
{
    setContent ( &s[0], s.length() );
}

// Obtains geographic location from local IP address using SSHTTP API request
// to free ip-api.com service. Runs synchronously on current thread; may block
// for several seconds if no internet connection.
// Returns true if successful or false on failure.

bool SSLocationFromIP ( SSSpherical &loc )
{
    SSHTTP request ( "http://ip-api.com/csv/?fields=lat,lon", 1000 );
    request.get();
    if ( request.getResponseCode() == SSHTTP::kOK )
    {
        double lat = 0.0, lon = 0.0;
        if ( sscanf ( request.getContentString().c_str(), "%lf,%lf", &lat, &lon ) == 2 )
        {
            loc = SSSpherical ( SSAngle::fromDegrees ( lon ), SSAngle::fromDegrees ( lat ) );
            return true;
        }
    }
    
    return false;
}
