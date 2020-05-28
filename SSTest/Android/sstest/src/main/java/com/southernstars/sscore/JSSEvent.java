package com.southernstars.sscore;

import java.util.ArrayList;
import com.southernstars.sscore.JSSAngle;
import com.southernstars.sscore.JSSEventTime;
import com.southernstars.sscore.JSSPass;
import com.southernstars.sscore.JSSRTS;
import com.southernstars.sscore.JSSObject;

// This class computes times and circumstances of astronomical events like
// object rising/transit/setting, satellite passes, moon phases, conjuctions,
// oppositions, etc.

public class JSSEvent
{
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

    public static native double semiDiurnalArc ( double lat, double dec, double alt );
    
    public static native JSSTime riseTransitSet ( JSSTime jd, double ra, double dec, int sign, double lon, double lat, double alt );
    public static native JSSTime riseTransitSet ( JSSTime time, JSSCoordinates coords, JSSObject pObj, int sign, double alt );
    public static native JSSTime riseTransitSetSearch ( JSSTime time, JSSCoordinates coords, JSSObject pObj, int sign, double alt );
    public static native JSSTime riseTransitSetSearchDay ( JSSTime today, JSSCoordinates coords, JSSObject pObj, int sign, double alt );

    public static native JSSPass riseTransitSet ( JSSTime today, JSSCoordinates coords, JSSObject pObj, double alt );
    public static native int findSatellitePasses ( JSSCoordinates coords, JSSObject pSat, JSSTime start, JSSTime stop, double minAlt, ArrayList<JSSPass> passes, int maxPassses );

    public static native JSSTime nextMoonPhase ( JSSTime time, JSSObject pSun, JSSObject pMoon, double phase );
    
    public static native void findConjunctions ( JSSCoordinates coords, JSSObject pObj1, JSSObject pObj2, JSSTime start, JSSTime stop, ArrayList<JSSEventTime> events, int maxEvents );
    public static native void findOppositions ( JSSCoordinates coords, JSSObject pObj1, JSSObject pObj2, JSSTime start, JSSTime stop, ArrayList<JSSEventTime> events, int maxEvents );
    public static native void findNearestDistances ( JSSCoordinates coords, JSSObject pObj1, JSSObject pObj2, JSSTime start, JSSTime stop, ArrayList<JSSEventTime> events, int maxEvents );
    public static native void findFarthestDistances ( JSSCoordinates coords, JSSObject pObj1, JSSObject pObj2, JSSTime start, JSSTime stop, ArrayList<JSSEventTime> events, int maxEvents );
}
