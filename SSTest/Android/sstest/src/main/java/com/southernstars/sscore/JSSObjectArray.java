package com.southernstars.sscore;

import android.content.res.AssetManager;
import com.southernstars.sscore.JSSObject;

public class JSSObjectArray
{
    long pObjectVec;    // pointer to native C++ SSObjectVec

    public JSSObjectArray()
    {
        create();
    }

    public void finalize()
    {
        destroy();
    }

    public native void create();
    public native void destroy();
    public native int size();
    public native JSSObject getObject ( int i );
    public native int importFromCSV ( String path );
    public native int exportToCSV ( String path );
    public native int importFromTLE ( String path );
    public native int importMcNames ( String path );

    public static native boolean initAssetManager ( AssetManager mgr );
}
