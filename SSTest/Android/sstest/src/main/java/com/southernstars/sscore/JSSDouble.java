package com.southernstars.sscore;

// This class is simply a double-precision floating point value wrapped in a Java class.
// It's needed for JNI interop because the "value" field of java.lang.Double does not exist on Android 10.

public class JSSDouble
{
    public double value;

    public JSSDouble ( double val )
    {
        value = val;
    }
}
