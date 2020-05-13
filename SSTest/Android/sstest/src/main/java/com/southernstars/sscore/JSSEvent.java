package com.southernstars.sscore;

import java.util.ArrayList;
import com.southernstars.sscore.JSSAngle;

// This class computes times and circumstances of astronomical events like
// object rising/transit/setting, satellite passes, moon phases, conjuctions,
// oppositions, etc.

public class JSSRTS
{
    public JSSTime time;
    public JSSAngle azm;
    public JSSAngle alt;
}

public class JSSPass
{
    public JSSRTS rising;
    public JSSRTS transit;
    public JSSRTS setting;
}

public class JSSEventTime
{
    public JSSTime time;
    public double value;
}

// No need convert this coz it's not public?  typedef double (*SSEventFunc) ( SSCoordinates &coords, SSObjectPtr pObj1, SSObjectPtr pObj2 );
// TIM: yeah, this is a C function pointer.  This will not translate to Java at all so just leave it out.

public class JSSEvent
{   // Lines copied over from from JSSCoordinates.java....
    //
    // private long pCoords;   // pointer to native C++ SSObject    <-- ... what does this do?
    // TIM: this points to the underlying instance of the C++ class that this Java class is wrapping.
    // TIM: However, since all methods of JSSEvent and SSEvent are static ("class") methods, shared by all instances of the class,
    // TIM: you never actually need an instance of the C++ class. So this can go away!

    // private JSSCoordinates()     <-- related to the pointer above
    // {
    //     pCoords = 0;
    // }
    // TIM: This can also go away.  It just initializes the C++ class instance pointer.  But no instance pointer is needed here!

    // public void finalize()       <-- what does this do?
    // {
    //     destroy();
    // }
    // TIM: when the JVM runs its garbage collector to free up memory, it calls the finalize() method of the Java class,
    // TIM: which in turn calls a native destroy() method, which destroys the underlying C++ SSEvent class instance.
    // TIM: Again you don't need any of this here.

    // public JSSEvent ( JSSTime time, JSSSpherical loc )
    // {
    //     create ( time, loc );
    // }
    // TIM: this would call a native create() method to construct the C++ SSEvent class instance when the JSSEvent java class is constructed.
    // TIM: Again not needed here

    // public native void create ( JSSTime time, JSSSpherical loc );
    // public native void destroy();
    // TIM: These are not needed for the same reason as all the above.

    public static final int kRise = -1;        // event sign for rising, to be used with riseTransSet, etc().
    public static final int kTransit = 0;      // event sign for transit, to be used with riseTransSet, etc().
    public static final int kSet = 1;          // event sign for setting, to be used with riseTransSet, etc().

    public static final double kDefaultRiseSetAlt = -30.0 / JSSAngle.kArcminPerRad;        // geometric altitude of point object when rising/setting [radians]
    public static final double kSunMoonRiseSetAlt = -50.0 / JSSAngle.kArcminPerRad;        // geometric altitude of Sun/Moon's apparent disk center when rising/setting [radians]
    public static final double kSunCivilDawnDuskAlt = -6.0 / JSSAngle.kDegPerRad;          // geometric altitude of Sun's apparent disk center at civil dawn/dusk [radians]
    public static final double kSunNauticalDawnDuskAlt = -12.0 / JSSAngle.kDegPerRad;      // geometric altitude of Sun's apparent disk center at nautical dawn/dusk [radians]
    public static final double kSunAstronomicalDawnDuskAlt = -18.0 / JSSAngle.kDegPerRad;  // geometric altitude of Sun's apparent disk center at astronomical dawn/dusk [radians]

    public static final double kNewMoon = 0.0;
    public static final double kFirstQuarterMoon = JSSAngle.kHalfPi;
    public static final double kFullMoon = JSSAngle.kPi;
    public static final double kLastQuarterMoon = 3.0 * JSSAngle.kHalfPi;

    public static native JSSAngle semiDiurnalArc ( JSSAngle lat, JSSAngle dec, JSSAngle alt );

    // TIM: In all the methods below, the JSSObjectPtr arguments should just be a JSSObject.

    public static native JSSTime riseTransitSet ( JSSTime jd, JSSAngle ra, JSSAngle dec, int sign, JSSAngle lon, JSSAngle lat, JSSAngle alt );
    public static native JSSTime riseTransitSet ( JSSTime time, JSSCoordinates coords, JSSObjectPtr pObj, int sign, JSSAngle alt );
    public static native JSSTime riseTransitSetSearch ( JSSTime time, JSSCoordinates coords, JSSObjectPtr pObj, int sign, JSSAngle alt );
    public static native JSSTime riseTransitSetSearchDay ( JSSTime today, JSSCoordinates coords, JSSObjectPtr pObj, int sign, JSSAngle alt );

    public static native JSSPass riseTransitSet ( JSSTime today, JSSCoordinates coords, JSSObjectPtr pObj, JSSAngle alt );
    public static native int findSatellitePaJsses ( JSSCoordinates coords, JSSObjectPtr pSat, JSSTime start, JSSTime stop, double minAlt, ArrayList<JSSPass> passes, int maxPassses );

    public static native JSSTime nextMoonPhase ( JSSTime time, JSSObjectPtr pSun, JSSObjectPtr pMoon, double phase );

    // TIM: you can skip the first to methods below. These take a C function pointer, which cannot translate to Java.
    // TIM: but keep the last four methods, which don't take a function pointer.
    
    public static native void findEvents ( JSSCoordinates coords, JSSObjectPtr pObj1, JSSObjectPtr pObj2, JSSTime start, JSSTime stop, double step, bool max, double limit, JSSEventFunc func, ArrayList<JSSEventTime> events, int maxEvents );
    public static native void findEqualityEvents ( JSSCoordinates coords, JSSObjectPtr pObj1, JSSObjectPtr pObj2, JSSTime start, JSSTime stop, double step, bool max, double value, JSSEventFunc func, ArrayList<JSSEventTime> events, int maxEvents );
    public static native void findConjunctions ( JSSCoordinates coords, JSSObjectPtr pObj1, JSSObjectPtr pObj2, JSSTime start, JSSTime stop, ArrayList<JSSEventTime> events, int maxEvents );
    public static native void findOppositions ( JSSCoordinates coords, JSSObjectPtr pObj1, JSSObjectPtr pObj2, JSSTime start, JSSTime stop, ArrayList<JSSEventTime> events, int maxEvents );
    public static native void findNearestDistances ( JSSCoordinates coords, JSSObjectPtr pObj1, JSSObjectPtr pObj2, JSSTime start, JSSTime stop, ArrayList<JSSEventTime> events, int maxEvents );
    public static native void findFarthestDistances ( JSSCoordinates coords, JSSObjectPtr pObj1, JSSObjectPtr pObj2, JSSTime start, JSSTime stop, ArrayList<JSSEventTime> events, int maxEvents );
}
