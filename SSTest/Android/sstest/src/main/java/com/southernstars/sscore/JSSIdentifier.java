package com.southernstars.sscore;

/*
public enum JSSCatalog
{
    kCatUnknown(0),
    kCatUnknown(1),
    kCatUnknown(2);
}
*/

// This class represents object identifiers used in a wide variety of astronomical catalogs
// with a unified system of 64-bit integers, and contains methods for converting identifiers
// from string representations ("HR 7001", "NGC 1976", etc.) to 64-bit integers and vice-versa.

public class JSSIdentifier
{
    long id;

    public static final int kCatUnknown = 0;        // Unknown catalog

    public static final int kCatJPLanet = 1;        // JPL NAIF planet/moon identifiers (Mercury = 1, Venus = 2, etc.)
    public static final int kCatAstNum = 2;         // Numbered asteroids (Ceres = 1, Pallas = 2, etc.)
    public static final int kCatComNum = 3;         // Numbered periodic comets (Halley = 1, Encke = 2, etc.)
    public static final int kCatNORADSat = 4;       // NORAD satellite catalog (ISS = 25544, HST = 20580, etc.)

    public static final int kCatBayer = 10;         // Bayer star letters (Alpha CMa, etc.)
    public static final int kCatFlamsteed = 11;     // Flamsteed star numbers (9 CMa, etc.)
    public static final int kCatGCVS = 12;          // General Catalog of Variable Stars (R And, etc.)
    public static final int kCatHR = 13;            // Harvard Revised (Yale Bright Star) catalog
    public static final int kCatGJ = 14;            // Gliese-Jahreiss Catalog of Nearby Stars
    public static final int kCatHD = 15;            // Henry Draper star catalog
    public static final int kCatSAO = 16;           // Sminthsonian Astrophysical Observatory star catalog
    public static final int kCatBD = 17;            // Bonner Durchmusterung star catalog
    public static final int kCatCD = 18;            // Cordoba Durchmusterung star catalog
    public static final int kCatCP = 19;            // Cape Photographic Durchmusterung star catalog
    public static final int kCatHIP = 20;           // Hipparcos star catalog
    public static final int kCatWDS = 21;           // Washington Double Star catalog

    public static final int kCatMessier = 30;       // Messier deep sky objects
    public static final int kCatCaldwell = 31;      // Caldwell deep sky objects
    public static final int kCatNGC = 32;           // New General Catalog of deep sky objects
    public static final int kCatIC = 33;            // Index Catalog of deep sky objects
    public static final int kCatMel = 34;           // Melotte Catalog of open clusters
    public static final int kCatLBN = 35;           // Lynds Bright Nebula catalog
    public static final int kCatPNG = 36;           // Galactic Planetary Nebulae (Strasbourg-ESO)
    public static final int kCatPK = 37;            // Perek-Kohoutek Catalog (planetary nebulae)
    public static final int kCatPGC = 38;           // Principal Galaxy Catalog
    public static final int kCatUGC = 39;           // Uppsala Galaxy Catalog
    public static final int kCatUGCA = 40;          // Uppsala Galaxy Catalog Appendix

    public JSSIdentifier()
    {
        id = 0;
    }

    public JSSIdentifier ( long ident )
    {
        id = ident;
    }

    public JSSIdentifier ( int catalog, long number )
    {
        id = catalog * 10000000000000000L + number;
    }

    int catalog()
    {
        return (int) ( id / 10000000000000000L );
    }

    long identifier()
    {
        return (int) ( id % 10000000000000000L );
    }

    public static native JSSIdentifier fromString ( String s );
    public native String toString();
}
