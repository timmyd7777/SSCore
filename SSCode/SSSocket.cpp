// SSSocket.cpp
// SSCore
//
// Created by Tim DeBenedictis on 9/12/22.
// Copyright © 2022 Southern Stars Group, LLC. All rights reserved.
//
// This class implements low-level IPv4 network socket communication.

#include "SSSocket.hpp"

#ifdef _WINDOWS

#include <winsock.h>

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

bool SSSocket::finalize ( void )
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

bool SSSocket::finalize ( void )
{
    return false; // Nothing to do on MacOS/Linux
}

#endif

vector<SSSocket::IPAddress> SSSocket::hostNameToIPs ( const string &hostName, bool useDNS )
{
    int               i, nIPs = 0;
    IPAddress         ip;
    struct hostent    *pHostEnt;
    vector<IPAddress> vIPs;
    
    // First, try to parse the host name string as an IPv4 dotted address
    // (e.g. "192.168.1.1").  If we fail, then try to resolve the host name
    // string using DNS, if so requested by the caller. ***/

    ip = inet_addr ( hostName.c_str() );
    if ( ip != INADDR_NONE )
    {
        vIPs.push_back ( ip );
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

string SSSocket::IPtoHostName ( IPAddress ip, bool useDNS )
{
    struct in_addr address = { ip };

    if ( useDNS )
    {
        struct hostent *pHostEnt = gethostbyaddr ( &address, sizeof ( address ), AF_INET );
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

/*
SSSocket::IPAddress SSSocket::getLocalIP ( void )
{
    char szHost[256] = { 0 };
    
    if ( gethostname ( szHost, sizeof ( szHost ) ) == 0 )
        return ( GHostNameToIPAddress ( szHost, pIPs, nMaxIPs ) );
    else
        return ( 0 );
}
*/

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
        
        vIPs.push_back ( ( (sockaddr_in *) pifa->ifa_addr )->sin_addr.s_addr );
        nIPs++;
    }
    
    return vIPs;
}

SSSocket::IPAddress SSSocket::getRemoteIP ( void )
{
    struct sockaddr_in address;
    socklen_t nLength = sizeof ( address );

    if ( getpeername ( _socket, (struct sockaddr *) &address, &nLength ) == 0 )
        return address.sin_addr.s_addr;
    else
        return 0;
}

bool SSSocket::socketOpen ( void )
{
    int    nFlag = 0;
    char   cData = 0;
    int    nResult = 0;

    if ( _socket == INVALID_SOCKET )
        return false;
    
    // The "right" way to do this is rather obscure.  First put the socket into
    // non-blocking mode and call recv() to see if there's any data waiting to be
    // read, without actually reading it.

    nFlag = true;
    if ( ioctlsocket ( _socket, FIONBIO, &nFlag ) == 0 )
    {
        nResult = (int) recv ( _socket, &cData, 1, MSG_PEEK );

        // Finally put the socket back into blocking mode. We have to do it this way
        // because select() and ioctlsocket() only tell us if there's unread data on the
        // connection, not whether the connection is still closed.

        nFlag = false;
        ioctlsocket ( _socket, FIONBIO, &nFlag );
    }

    // If the connection has been closed and all data received, recv() returns zero.
    // Otherwise, recv() returns the number of bytes it could have read (if there's
    // any data remaining in the buffer), or an error code indicating that the recv()
    // call would block (because there's nothing in the buffer but the remote peer
    // hasn't yet closed the connection).

    return nResult == 0 ? false : true;
}

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

int SSSocket::readSocket ( void *data, int size )
{
    int  nResult;
    int  nBytesRead = 0;
    int  nBytesToRead;

    do
    {
        // First, use the ioctlsocket() function to determine how many bytes
        // are available to be read from the socket.

        nResult = ioctlsocket ( _socket, FIONREAD, &nBytesToRead );
        if ( nResult == 0 )
        {
            // If there's nothing to read, stop now.  We do this
            // to prevent a subsequent call to recv() from blocking.

            if ( nBytesToRead == 0 )
                break;

            // If we have bytes available, but we've not been given a buffer
            // to read them into, return the number of bytes available without
            // actually reading them.

            if ( data == nullptr )
            {
                nBytesRead = nBytesToRead;
                break;
            }
            else
            {
                // If we do have a buffer, actually read the available data
                // into it.  First make sure we don't overflow the buffer.
                // Then read into it, and break if we run into an error
                // condition or if the remote peer closes the connection.

                if ( nBytesToRead + nBytesRead > size )
                    nBytesToRead = size - nBytesRead;

                nResult = (int) recv ( _socket, (char *) data + nBytesRead, nBytesToRead, 0 );
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

void SSSocket::closeSocket ( void )
{
    shutdown ( _socket, 2 );
    closesocket ( _socket );
    _socket = INVALID_SOCKET;
}

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

SSSocket SSSocket::serverAcceptConnection ( void )
{
    SOCKET              nResult;
    struct sockaddr_in  address;
    socklen_t           nSize = sizeof ( address );

    nResult = accept ( _socket, (struct sockaddr *) &address, &nSize );
    return SSSocket ( nResult );
}

bool SSSocket::openUDPSocket ( IPAddress ipAddress, unsigned short wPort )
{
    int                 nSocket = 0;
    struct sockaddr_in  address = { 0 };
    struct in_addr      addr = { ipAddress };
    
    nSocket = socket ( PF_INET, SOCK_DGRAM, 0 );
    if ( nSocket == -1 )
        return false;
    
    if ( ipAddress && wPort )
    {
        address.sin_family = PF_INET;
        address.sin_port = htons ( wPort );
        address.sin_addr = addr;
        memset(&(address.sin_zero), 0, sizeof ( address.sin_zero ) );
        
        if ( ::bind ( nSocket, (struct sockaddr *) &address, (socklen_t) sizeof ( address ) ) == -1 )
        {
            closesocket ( nSocket );
            return false;
        }
    }
    
    _socket = nSocket;
    return true;
}

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

int SSSocket::readUDPSocket ( void *lpvData, int lLength, IPAddress &senderIP, long timeout_ms )
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
    
    nResult = setsockopt ( _socket, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof ( tv ) );
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
    
    senderIP = address.sin_addr.s_addr;
    return nResult;
}
