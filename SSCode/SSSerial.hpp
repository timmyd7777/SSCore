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

    // Standard supported baud rates (bits per second)

    static constexpr int k300Baud = 300;
    static constexpr int k600Baud = 600;
    static constexpr int k1200Baud = 1200;
    static constexpr int k2400Baud = 2400;
    static constexpr int k4800Baud = 4800;
    static constexpr int k9600Baud = 9600;
    static constexpr int k14400Baud = 14400;
    static constexpr int k19200Baud = 19200;
    static constexpr int k38400Baud = 38400;
    static constexpr int k57600Baud = 57600;
    static constexpr int k115200Baud = 115200;
    static constexpr int k230400Baud = 230400;
    static constexpr int k460800Baud = 460800;
    static constexpr int k921600Baud = 921600;
    
    // Supported parity settings

    static constexpr int kNoParity = 0;
    static constexpr int kOddParity = 1;
    static constexpr int kEvenParity = 2;
    
    // Suppored data bits

    static constexpr int k5DataBits = 5;
    static constexpr int k6DataBits = 6;
    static constexpr int k7DataBits = 7;
    static constexpr int k8DataBits = 8;

    // Suppored stop bits

    static constexpr float k1StopBits = 1.0f;
    static constexpr float k15StopBits = 1.5f; // 1.5 stop bits not supported in MacOS!
    static constexpr float k2StopBits = 2.0f;

    SSSerial ( void ) { _port = kError; }
    virtual ~SSSerial ( void ) { closePort(); }
    
    static int listPorts ( vector<string> &names, vector<string> &paths );
    bool openPort ( const string &path );
    bool portOpen ( void );
    bool closePort ( void );
    int writePort ( const void *data, size_t size );
    int readPort ( void *data, size_t size );
    int inputBytes ( void );
    int outputBytes ( void );
    bool getPortConfig ( int &baud, int &parity, int &data, float &stop );
    bool setPortConfig ( int baud, int parity, int data, float stop );
};

#endif /* SSSerial_hpp */
