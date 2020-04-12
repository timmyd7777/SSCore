package com.southernstars.sscore;

public class JSSObjectArray
{
    long pObjectVec;

    public JSSObjectArray()
    {
        pObjectVec = 0;
    }

    public static native JSSObjectArray create();
    public native void destroy();
    public native int importFromCSV ( String path );
    public native int exportToCSV ( String path );
}
