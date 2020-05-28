package com.southernstars.sscore;

import com.southernstars.sscore.JSSIdentifier;
import com.southernstars.sscore.JSSVector;

// This is the base class for all astronomical objects (planets, stars, deep sky objects, constellations, etc.)

public class JSSObject
{
    private long pObject;   // pointer to native C++ SSObject

    public static final int kTypeNonexistent = 0;            // Nonexistent/unknown object or erroneous catalog entry
    public static final int kTypePlanet = 1;                 // Major planet (Mercury, Venus, etc.)
    public static final int kTypeMoon = 2;                   // Natural satellite (Moon, Io, Europa, etc.)
    public static final int kTypeAsteroid = 3;               // Minor planet (Ceres, Pallas, etc.)
    public static final int kTypeComet = 4;                  // Comet (Halley, Encke, etc.)
    public static final int kTypeSatellite = 5;              // Artificial satellite (ISS, HST, etc.)
    public static final int kTypeSpacecraft = 6;             // Interplanetary spacecraft (Voyager, Cassini, etc.)
    public static final int kTypeStar = 10;                  // Single star (Canopus, Vega, etc.)
    public static final int kTypeDoubleStar = 12;            // Double star (Alpha Cen, Sirius, etc.)
    public static final int kTypeVariableStar = 13;          // Variable single star (Mira, etc.)
    public static final int kTypeDoubleVariableStar = 14;    // Double star with variable component (Betelgeuse, Algol, etc.)
    public static final int kTypeOpenCluster = 20;           // Open star cluster (M45, Hyades, etc.)
    public static final int kTypeGlobularCluster = 21;       // Globular star cluster (M13, etc.)
    public static final int kTypeBrightNebula = 22;          // Emission, reflection nebula or supernova remnant (M42, M78, M1, etc.)
    public static final int kTypeDarkNebula = 23;            // Dark nebula (Coalsack, Horsehead, etc.)
    public static final int kTypePlanetaryNebula = 24;       // Planetary nebula (M57, M27, etc.)
    public static final int kTypeGalaxy = 25;                // Galaxy (M31, LMC, SMC, etc.)
    public static final int kTypeConstellation = 30;         // Constellation officially recognized by IAU (Andromeda, Antlia, etc.)
    public static final int kTypeAsterism = 31;              // Common but informally recognized star pattern (Big Dipper, Summer Triangle, etc.)

    public JSSObject()
    {
        pObject = 0;
    }

    public static native String typeToCode ( int type );
    public static native int codeToType ( String code );

    public native int getType();
    public native String getName ( int i );
    public native JSSIdentifier getIdentifier ( int catalog );

    public native JSSVector getDirection();
    public native double getDistance();
    public native float getMagnitude();

    public native void setDirection ( JSSVector direction );
    public native void setDistance ( double distance );
    public native void setMagnitude ( float magnitude );
}
