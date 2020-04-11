package com.southernstars.sscore;

import com.southernstars.sscore.JSSVector;

// This class reads JPL's binary DE43x series of ephemeris files
// and computes very fast, accurate lunar and planetary positions from them.

public class JSSJPLDEphemeris
{
    // Opens and closes ephemeris file

    public static native boolean open ( String filename );
    public static native boolean isOpen();
    public static native void close();

    // Gets number of contants, name and value of i-th constant.

    public static native int getConstantNumber();
    public static native String getConstantName ( int i );
    public static native double getConstantValue ( int i );

    // Gets start and stop Julian Ephemeris Date, and time step in days

    public static native double getStartJED();
    public static native double getStopJED();
    public static native double getStep();

    // Computes object position and velocity at a given JED.

    public static native boolean compute ( int id, double jde, boolean bary, JSSVector position, JSSVector velocity );
}
