// SSCore.h
// SSTest
//
// Created by Tim DeBenedictis on 4/7/20.
// Copyright © 2020 Southern Stars. All rights reserved.
//
// This file defines C wrappers around the C++ SSCore definitions, classes, and methods.
// It's primarily intended as a bridge to call SSCore functionality from Swift.

#ifndef SSCore_h
#define SSCore_h

#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <math.h>

#ifdef __cplusplus
extern "C" {
#endif

// C wrappers for C++ SSTime definitions, classes, and methods

const int kSSGregorian = 0;             // Gregorian calendar, used after 15 October 1582 (JD 2299161).  Leap years every 4th year, but not every 100th, unless also 400th.
const int kSSJulian = 1;                // Julian calendar, used before 4 October 1582 (JD 2299161).  Leap years every 4 years.

const double kSSJ2000 = 2451545.0;      // JD of standard Julian epoch J2000
const double kSSJ1970 = 2440587.5;      // JD of standard UNIX time base 1.0 January 1970 UTC
const double kSSB1950 = 2433282.423;    // JD of standard Besselian epoch B1950
const double kSSB1900 = 2433282.423;    // JD of standard Besselian epoch B1900

const double kSSSecondsPerDay = 86400.0;
const double kSSMinutesPerDay = 1440.0;
const double kSSHoursPerDay = 24.0;

const double kSSDaysPerJulianYear = 365.25;
const double kSSDaysPerBesselianYear = 365.242198781;

const double kSSSiderealPerSolarDays = 1.00273790934;
const double kSSSolarPerSiderealDays = 0.99726957;

typedef struct CSSTime
{
    double jd;          // Julian date in civil time (NOT epehemeris time!)
    double zone;        // Local time zone to use for converting to local calendar date/time, hours east of Greenwich
    int    calendar;    // Calendar system to use for converting to calendar date/time
}
CSSTime;

typedef struct CSSDate
{
    int    calendar;    // calendar system: kGregorian or kJulian.
    double zone;        // local time zone offset from UTC in hours east of Greenwich; wwst is negative!
    int    year;        // calendar year; note 0 = 1 BC, -1 = 2 BC, etc.
    short  month;       // month; 1 = Jan, 2 = Feb, etc.
    double day;         // day including fractional part; 1.0 to 31.999...
    short  hour;        // hour of day; 0 to 23
    short  min;         // minute of hour; 0 to 59
    double sec;         // seconds of minute including fractional part; 0 to 59.999...
}
CSSDate;

CSSDate CSSTimeToCSSDate ( CSSTime time );
CSSTime CSSDateToCSSTime ( CSSDate date );

CSSTime CSSTimeFromSystem ( void );
CSSTime CSSTimeFromUnixTime ( time_t utime );
CSSTime CSSTimeFromJulianYear ( double year );
CSSTime CSSTimeFromBesselianYear ( double year );

time_t CSSTimeToUnixTime ( CSSTime ctime );
double CSSTimeToJulianYear ( CSSTime ctime );
double CSSTimeToBesselianYear ( CSSTime ctime );

double CSSTimeGetDeltaT ( CSSTime ctime );
double CSSTimeGetJulianEphemerisDate ( CSSTime ctime );
double CSSTimeGetSiderealTime ( CSSTime ctime, double lon );

// C wrappers for C++ SSAngle constants, classes, and methods

const double kSSPi = M_PI;
const double kSSTwoPi = M_PI * 2.0;
const double kSSHalfPi = M_PI_2;

const double kSSDegPerRad = 180.0 / kSSPi;
const double kSSRadPerDeg = kSSPi / 180.0;

const double kSSHourPerRad = kSSDegPerRad / 15.0;
const double kSSRadPerHour = 1.0 / kSSHourPerRad;

const double kSSArcminPerRad = 60.0 * kSSDegPerRad;
const double kSSRadPerArcmin = kSSArcminPerRad;

const double kSSArcsecPerRad = 3600 * kSSDegPerRad;
const double kSSRadPerArcsec = 1.0 / kSSArcsecPerRad;

typedef struct CSSDegMinSec
{
    char   sign;      // sign of angle, either '+' or '-'
    short  deg;       // degrees portion of angle, 0 - 360, always positive
    short  min;       // minutes portion of angle, 0 - 59, always positive
    double sec;       // seconds portion of angle, 0 - 59.999..., always positive
}
CSSDegMinSec;

typedef struct CSSHourMinSec
{
    char   sign;      // sign of angle, either '+' or '-'
    short  hour;      // hours portion of angle, 0 - 23, always positive
    short  min;       // minutes portion of angle, 0 - 59, always positive
    double sec;       // seconds portion of angle, 0 - 59.999..., always positive
}
CSSHourMinSec;

typedef double CSSAngle;

CSSDegMinSec CSSDegMinSecFromRadians ( CSSAngle rad );
CSSDegMinSec CSSDegMinSecFromString ( const char *str );

CSSAngle CSSDegMinSecToRadians ( CSSDegMinSec dms );
const char *CSSDegMinSecToString ( CSSDegMinSec dms );

CSSHourMinSec CSSHourMinSecFromRadians ( CSSAngle rad );
CSSHourMinSec CSSHourMinSecFromString ( const char *str );

CSSAngle CSSHourMinSecToRadians ( CSSHourMinSec hms );
const char *CSSHourMinSecToString ( CSSHourMinSec hms );

// C wrappers for C++ SSVector classes

// Represents a point in a spherical coordinate system (lon/lat, RA/Dec, Az/Alt)

typedef struct CSSSpherical
{
    CSSAngle lon;    // Longitude coordinate, in radians from 0 to kTwoPi.
    CSSAngle lat;    // Latitude coordinate, in radians from -kHalfPi to +kHalfPi.
    double   rad;    // Radial distance from origin of coordinate system, in arbitrary units; 1.0 is a unit vector.
}
CSSSpherical;

// Represents a point in a rectangular coordinate system.

typedef struct CSSVector
{
    double x, y, z;    // Point's distance from origin along X, Y, Z axes, in arbitary units.
}
CSSVector;

CSSVector CSSVectorFromXYZ ( double x, double y, double z );
CSSSpherical CSSSphericalFromLonLatRad ( double lon, double lat, double rad );

CSSVector CSSSphericalToCSSVector ( CSSSpherical csph );
CSSSpherical CSSVectorToCSSSpherical ( CSSVector cvec );

CSSVector CSSSphericalCSSVectorVelocity ( CSSSpherical pos, CSSSpherical vel );
CSSSpherical CSSVectorToCSSSphericalVelocity ( CSSVector pos, CSSVector vel );

double CSSSphericalAngularSeparation ( CSSSpherical csph1, CSSSpherical csph2 );
double CSSSphericalPositionAngle ( CSSSpherical csph1, CSSSpherical csph2 );

double CSSVectorMagnitude ( CSSVector cvec );
double CSSVectorDistance ( CSSVector cvec1, CSSVector cvec2 );
CSSVector CSSVectorNormalize ( CSSVector cvec );

CSSVector CSSVectorAdd ( CSSVector cvec1, CSSVector cvec2 );
CSSVector CSSVectorSubtract ( CSSVector cvec1, CSSVector cvec2 );
CSSVector CSSVectorMultiplyBy ( CSSVector cvec, double s );
CSSVector CSSVectorDivideBy ( CSSVector cvec, double s );

double CSSVectorDotProduct ( CSSVector v1, CSSVector v2 );
CSSVector CSSVectorCrossProduct ( CSSVector v1, CSSVector v2 );

double CSSVectorAngularSeparation ( CSSVector cvec1, CSSVector cvec2 );
double CSSVectorPositionAngle ( CSSVector cvec1, CSSVector cvec2 );

// C wrappers for C++ SSMatrix classes and methods

typedef struct CSSMatrix
{
    double m00, m01, m02;
    double m10, m11, m12;
    double m20, m21, m22;
}
CSSMatrix;

CSSMatrix CSSMatrixIdentity ( void );
CSSMatrix CSSMatrixTranspose ( CSSMatrix mat );
CSSMatrix CSSMatrixInverse ( CSSMatrix mat );
double CSSMatrixDeterminant ( CSSMatrix mat );

CSSMatrix CSSMatrixMultiplyMatrix ( CSSMatrix mat1, CSSMatrix mat2 );
CSSVector CSSMatrixMultiplyVector ( CSSMatrix mat, CSSVector vec );
CSSMatrix CSSMatrixRotate ( CSSMatrix mat, int axis, double angle );

// C wrappers for C++ SSIdentifier classes and methods

static int kCSSCatUnknown = 0;        // Unknown catalog

static int kCSSCatJPLanet = 1;        // JPL NAIF planet/moon identifiers (Mercury = 1, Venus = 2, etc.)
static int kCSSCatAstNum = 2;         // Numbered asteroids (Ceres = 1, Pallas = 2, etc.)
static int kCSSCatComNum = 3;         // Numbered periodic comets (Halley = 1, Encke = 2, etc.)
static int kCSSCatNORADSat = 4;       // NORAD satellite catalog (ISS = 25544, HST = 20580, etc.)

static int kCSSCatBayer = 10;         // Bayer star letters (Alpha CMa, etc.)
static int kCSSCatFlamsteed = 11;     // Flamsteed star numbers (9 CMa, etc.)
static int kCSSCatGCVS = 12;          // General Catalog of Variable Stars (R And, etc.)
static int kCSSCatHR = 13;            // Harvard Revised (Yale Bright Star) catalog
static int kCSSCatGJ = 14;            // Gliese-Jahreiss Catalog of Nearby Stars
static int kCSSCatHD = 15;            // Henry Draper star catalog
static int kCSSCatSAO = 16;           // Sminthsonian Astrophysical Observatory star catalogstatic int kCSS
static int kCSSCatBD = 17;            // Bonner Durchmusterung star catalog
static int kCSSCatCD = 18;            // Cordoba Durchmusterung star catalog
static int kCSSCatCP = 19;            // Cape Photographic Durchmusterung star catalog
static int kCSSCatHIP = 20;           // Hipparcos star catalog
static int kCSSCatWDS = 21;           // Washington Double Star catalog

static int kCSSCatMessier = 30;       // Messier deep sky objects
static int kCSSCatCaldwell = 31;      // Caldwell deep sky objects
static int kCSSCatNGC = 32;           // New General Catalog of deep sky objects
static int kCSSCatIC = 33;            // Index Catalog of deep sky objects
static int kCSSCatMel = 34;           // Melotte Catalog of open clusters
static int kCSSCatLBN = 35;           // Lynds Bright Nebula catalog
static int kCSSCatPNG = 36;           // Galactic Planetary Nebulae (Strasbourg-ESO)
static int kCSSCatPK = 37;            // Perek-Kohoutek Catalog (planetary nebulae)
static int kCSSCatPGC = 38;           // Principal Galaxy Catalog
static int kCSSCatUGC = 39;           // Uppsala Galaxy Catalog
static int kCSSCatUGCA = 40;          // Uppsala Galaxy Catalog Appendix

typedef int64_t CSSIdentifier;

CSSIdentifier CSSIdentifierFromString ( const char *str );
const char *StringFromCSSIdentifier ( CSSIdentifier ident );

CSSIdentifier CSSIdentifierFromCatalogNumber ( int catalog, int64_t number );
char CSSIdentifierGetCatalog ( CSSIdentifier ident );
int64_t CSSIdentifierGetNumber ( CSSIdentifier ident );

// C wrappers for C++ SSJPLDEphemeris classes and methods

bool CSSJPLDEphemerisOpen ( const char *filename );
bool CSSJPLDEphemerisIsOpen ( void );
void CSSJPLDEphemerisClose ( void );

int CSSJPLDEphemerisGetConstantCount ( void );
const char *CSSJPLDEphemerisGetConstantName ( int i );
double CSSJPLDEphemerisGetConstantValue ( int i );

double CSSJPLDEphemerisGetStartJED ( void );
double CSSJPLDEphemerisGetStopJED ( void );
double CSSJPLDEphemerisGetStep ( void );

bool CSSJPLDEphemerisCompute ( int planet, double jd, bool bary, CSSVector *pos, CSSVector *vel );

// C wrappers for C++ SSObject definitions, classes and methods

static int kCSSTypeNonexistent = 0;            // Nonexistent/unknown object or erroneous catalog entry
static int kCSSTypePlanet = 1;                 // Major planet (Mercury, Venus, etc.)
static int kCSSTypeMoon = 2;                   // Natural satellite (Moon, Io, Europa, etc.)
static int kCSSTypeAsteroid = 3;               // Minor planet (Ceres, Pallas, etc.)
static int kCSSTypeComet = 4;                  // Comet (Halley, Encke, etc.)
static int kCSSTypeSatellite = 5;              // Artificial satellite (ISS, HST, etc.)
static int kCSSTypeSpacecraft = 6;             // Interplanetary spacecraft (Voyager, Cassini, etc.)
static int kCSSTypeStar = 10;                  // Single star (Canopus, Vega, etc.)
static int kCSSTypeDoubleStar = 12;            // Double star (Alpha Cen, Sirius, etc.)
static int kCSSTypeVariableStar = 13;          // Variable single star (Mira, etc.)
static int kCSSTypeDoubleVariableStar = 14;    // Double star with variable component (Betelgeuse, Algol, etc.)
static int kCSSTypeOpenCluster = 20;           // Open star cluster (M45, Hyades, etc.)
static int kCSSTypeGlobularCluster = 21;       // Globular star cluster (M13, etc.)
static int kCSSTypeBrightNebula = 22;          // Emission, reflection nebula or supernova remnant (M42, M78, M1, etc.)
static int kCSSTypeDarkNebula = 23;            // Dark nebula (Coalsack, Horsehead, etc.)
static int kCSSTypePlanetaryNebula = 24;       // Planetary nebula (M57, M27, etc.)
static int kCSSTypeGalaxy = 25;                // Galaxy (M31, LMC, SMC, etc.)
static int kCSSTypeConstellation = 30;         // Constellation officially recognized by IAU (Andromeda, Antlia, etc.)
static int kCSSTypeAsterism = 31;              // Common but informally recognized star pattern (Big Dipper, Summer Triangle, etc.)

typedef struct CSSObject CSSObject;
typedef CSSObject *CSSObjectPtr;

const char *CSSObjectTypeToCode ( int type );
int CSSObjectTypeFromCode ( const char *string );

int CSSObjectGetType ( CSSObjectPtr pObject );
const char *CSSObjectGetName ( CSSObjectPtr pObject, int i );           // returns copy of i-th name string
CSSIdentifier CSSObjectGetIdentifier ( CSSObjectPtr pObject, int cat ); // returns identifier in the specified catalog, or null identifier if object has none in that catalog.

CSSVector CSSObjectGetDirection ( CSSObjectPtr pObject );
double CSSObjectGetDistance ( CSSObjectPtr pObject );
float CSSObjectGetMagnitude ( CSSObjectPtr pObject );

void CSSObjectSetDirection ( CSSObjectPtr pObject, CSSVector dir );
void CSSObjectSetDistance ( CSSObjectPtr pObject, double dist );
void CSSObjectSetMagnitude ( CSSObjectPtr pObject, float mag );

typedef struct CSSObjectArray CSSObjectArray;
typedef CSSObjectArray *CSSObjectArrayPtr;

CSSObjectArray *CSSObjectArrayCreate ( void );
void CSSObjectArrayDestroy ( CSSObjectArray *pObjArr );

int CSSImportObjectsFromCSV ( const char *filename, CSSObjectArray *pObjArr );
size_t CSSObjectArraySize ( CSSObjectArray *pObjArr );
CSSObjectPtr CSSObjectGetFromArray ( CSSObjectArray *pObjArr, int i );

#ifdef __cplusplus
}
#endif

#endif /* SSCore_h */
