// SSSerial.hpp
// SSCore
//
// Created by Tim DeBenedictis on 9/12/22.
// Copyright © 2022 Southern Stars. All rights reserved.
//
// This class contains routines for low-level serial (RS-232) communication
// on MacOS, Windows, and Linux. Currently not supported on iOS or Android.

#ifndef SSSerial_hpp
#define SSSerial_hpp

#ifdef _WINDOWS
#include <windows.h>
#endif

#include <string>
#include <vector>

using namespace std;

class SSSerial
{
public:
#ifdef _WINDOWS
    typedef HANDLE PortRef;
    static constexpr HANDLE kError = INVALID_HANDLE_VALUE;
#else
    typedef int PortRef;
    static constexpr int kError = -1;
#endif

protected:
    PortRef _port = kError;

public:
    enum Baud     // Supported baud rates (bits per second)
    {
        k300 = 300,
        k600 = 600,
        k1200 = 1200,
        k2400 = 2400,
        k4800 = 4800,
        k9600 = 9600,
        k14400 = 14400,  // not supported on Linux
        k19200 = 19200,
        k38400 = 38400,
        k57600 = 57600,
        k115200 = 115200,
        k230400 = 230400,
        k460800 = 460800,  // not supported on MacOS
        k921600 = 921600   // not supported on MacOS
    };
    
    enum Parity     // Supported parity settings
    {
        kNone = 0,
        kOdd = 1,
        kEven = 2
    };
    
    enum DataBits   // Suppored data bits
    {
        k5 = 5,
        k6 = 6,
        k7 = 7,
        k8 = 8
    };

    enum StopBits  // Suppored stop bits
    {
        k1 = 1,
        k15 = 15, // 1.5 stop bits not supported in MacOS!
        k2 = 2
    };

    SSSerial ( void ) { _port = kError; }
    virtual ~SSSerial ( void ) { closePort(); }
    
    int listPorts ( vector<string> &names, vector<string> &paths );
    bool openPort ( const string &path );
    bool portOpen ( void );
    bool closePort ( void );
    int writePort ( void *data, size_t size );
    int readPort ( void *data, size_t size );
    int inputBytes ( void );
    int outputBytes ( void );
    bool getPortConfig ( Baud &baud, Parity &parity, DataBits &data, StopBits &stop );
    bool setPortConfig ( Baud baud, Parity parity, DataBits data, StopBits stop );
};

#endif /* SSSerial_hpp */
