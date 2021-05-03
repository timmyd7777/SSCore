//  SSStar.cpp
//  SSCore
//
//  Created by Tim DeBenedictis on 3/15/20.
//  Copyright © 2020 Southern Stars. All rights reserved.

#include <algorithm>
#include <map>

#include "SSCoordinates.hpp"
#include "SSStar.hpp"

// Table of absolute magnitudes by spectral and luminosity class from
// "Stellar Spectral Classification", Gray & Corbally, 2009, Appendix B.
// Magnitude values interpolated by Bruce MacEvoy here:
// http://www.handprint.com/ASTRO/specclass.html

struct SpecClass
{
    string spec;        // MK spectral classification
    float temp;         // main-sequence temperature (Kelvins)
    float MvV;          // absolute visual (V) magnitude for luminosity class V (main sequence)
    float MvIV;         // absolute visual (V) magnitude for luminosity class IV (sub-giants)
    float MvIII;        // absolute visual (V) magnitude for luminosity class III (giants)
    float MvII;         // absolute visual (V) magnitude for luminosity class II (bright giants)
    float MvIb;         // absolute visual (V) magnitude for luminosity class Ib (less luminous supergiants)
    float MvIa;         // absolute visual (V) magnitude for luminosity class Ia (more luminous supergiants)
};

static vector<SpecClass> _speclass =
{
    { "O1", INFINITY, INFINITY, INFINITY, INFINITY, INFINITY, INFINITY, INFINITY },
    { "O2", INFINITY, -5.6, INFINITY, INFINITY, INFINITY, INFINITY, INFINITY },
    { "O3", 44850, -5.6, INFINITY, INFINITY, INFINITY, INFINITY, INFINITY },
    { "O4", 42860, -5.5, INFINITY, INFINITY, INFINITY, INFINITY, INFINITY },
    { "O5", 40860, -5.5, INFINITY, INFINITY, INFINITY, INFINITY, INFINITY },
    { "O6", 38870, -5.3, INFINITY, INFINITY, INFINITY, -7, INFINITY },
    { "O7", 36870, -4.8, INFINITY, INFINITY, -6.3, INFINITY, INFINITY },
    { "O8", 34880, -4.4, INFINITY, INFINITY, -6.2, -6.5, INFINITY },
    { "O9", 32880, -4.3, -5, -5.6, -5.9, -6.2, -7 },
    { "B0", 29000, -4.1, -4.6, -5, -5.6, -5.8, -7 },
    { "B1", 24500, -3.5, -3.9, -4.4, -5.1, -5.7, -7 },
    { "B2", 19500, -2.5, -3, -3.6, -4.4, -5.7, -7 },
    { "B3", 16500, -1.7, -2.3, -2.9, -3.9, -5.7, -7 },
    { "B4", INFINITY, -1.4, -2, -2.6, -3.9, -5.7, -7 },
    { "B5", 15000, -1.1, -1.6, -2.2, -3.7, -5.7, -7 },
    { "B6", INFINITY, -0.9, -1.3, -1.9, -3.7, -5.7, -7.1 },
    { "B7", 13000, -0.4, -1.3, -1.6, -3.6, -5.6, -7.1 },
    { "B8", 11500, 0, -1, -1.4, -3.4, -5.6, -7.1 },
    { "B9", 10700, 0.7, -0.5, -0.8, -3.1, -5.5, -7.1 },
    { "A0", 9800, 1.4, 0.3, -0.8, -2.8, -5.2, -7.1 },
    { "A1", 9500, 1.6, 0.3, -0.4, -2.6, -5.1, -7.3 },
    { "A2", 8900, 1.9, 0.5, -0.2, -2.4, -5, -7.5 },
    { "A3", 8520, 2, 0.7, 0, -2.3, -4.8, -7.6 },
    { "A4", INFINITY, 2.05, 0.95, 0.15, -2.2, -4.8, -7.65 },
    { "A5", 8150, 2.1, 1.2, 0.3, -2.1, -4.8, -7.7 },
    { "A6", INFINITY, 2.2, 1.35, 0.4, -2.05, -4.8, -7.75 },
    { "A7", 7830, 2.3, 1.5, 0.5, -2, -4.8, -8 },
    { "A8", INFINITY, 2.4, 1.55, 0.55, -2, -4.8, -8.15 },
    { "A9", 7380, 2.5, 1.6, 0.6, -2, -4.8, -8.3 },
    { "F0", 7250, 2.6, 1.7, 0.6, -2, -4.7, -8.5 },
    { "F1", 7120, 2.8, 1.8, 0.6, -2, -4.7, -8.5 },
    { "F2", 7000, 3, 1.9, 0.6, -2, -4.6, -8.4 },
    { "F3", 6750, 3.1, 1.9, 0.6, -2, -4.6, -8.3 },
    { "F4", INFINITY, 3.3, 2, 0.7, -2, -4.6, -8.3 },
    { "F5", 6550, 3.4, 2.1, 0.7, -2, -4.4, -8.2 },
    { "F6", INFINITY, 3.7, 2.2, 0.7, -2, -4.4, -8.1 },
    { "F7", 6250, 3.8, 2.3, 0.6, -2, -4.4, -8.1 },
    { "F8", 6170, 4, 2.4, 0.6, -2, -4.3, -8 },
    { "F9", 6010, 4.2, 2.6, 0.6, -2, -4.2, -8 },
    { "G0", 5900, 4.4, 2.8, 0.6, -2, -4.1, -8 },
    { "G1", 5800, 4.5, 2.9, 0.5, -2, -4.1, -8 },
    { "G2", 5750, 4.7, 3, 0.4, -2, -4, -8 },
    { "G3", INFINITY, 4.9, 3, 0.4, -1.9, -4, -8 },
    { "G4", INFINITY, 5, 3.1, 0.4, -1.9, -3.9, -8 },
    { "G5", 5580, 5.2, 3.2, 0.4, -1.9, -3.9, -8 },
    { "G6", INFINITY, 5.3, 3.2, 0.4, -1.9, -3.8, -8 },
    { "G7", INFINITY, 5.5, 3.2, 0.3, -1.9, -3.8, -8 },
    { "G8", 5430, 5.6, 3.2, 0.3, -1.9, -3.7, -8 },
    { "G9", 5350, 5.7, 3.2, 0.25, -2, -3.7, -8 },
    { "K0", 5280, 5.9, 3.2, 0.2, -2, -3.6, -8 },
    { "K1", 5110, 6.1, INFINITY, 0.1, -2.1, -3.6, -8 },
    { "K2", 4940, 6.3, INFINITY, 0.1, -2.1, -3.6, -8 },
    { "K3", 4700, 6.9, INFINITY, -0.1, -2.2, -3.6, -8 },
    { "K4", INFINITY, 7.4, INFINITY, -0.2, -2.3, -3.7, -8 },
    { "K5", 4400, 8, INFINITY, -0.4, -2.5, -3.8, -8 },
    { "K6", INFINITY, 8.2, INFINITY, -0.45, -2.5, -3.8, -7.85 },
    { "K7", 4130, 8.5, INFINITY, -0.5, -2.5, -3.8, -7.7 },
    { "K8", INFINITY, 8.7, INFINITY, -0.57, -2.53, -3.83, -7.6 },
    { "K9", INFINITY, 9, INFINITY, -0.64, -2.56, -3.86, -7.45 },
    { "M0", 3760, 9.2, INFINITY, -0.7, -2.6, -3.9, -7.3 },
    { "M1", 3625, 9.7, INFINITY, -0.8, -2.7, -4.1, -7.3 },
    { "M2", 3490, 10.6, INFINITY, -1.1, -2.9, -4.2, -7 },
    { "M3", 3355, 11.6, INFINITY, -1.3, INFINITY, INFINITY, INFINITY },
    { "M4", 3220, 12.9, INFINITY, -1.6, INFINITY, INFINITY, INFINITY },
    { "M5", 3085, 14.5, INFINITY, INFINITY, INFINITY, INFINITY, INFINITY },
    { "M6", 2950, 16.1, INFINITY, INFINITY, INFINITY, INFINITY, INFINITY },
    { "M7", 2815, INFINITY, INFINITY, INFINITY, INFINITY, INFINITY, INFINITY },
    { "M8", 2680, INFINITY, INFINITY, INFINITY, INFINITY, INFINITY, INFINITY },
    { "M9", 2545, INFINITY, INFINITY, INFINITY, INFINITY, INFINITY, INFINITY },
};

// Table of main-sequence stellar properties from:
// http://www.pas.rochester.edu/%7Eemamajek/EEM_dwarf_UBVIJHK_colors_Teff.txt

static vector<SSStar::SpecInfo> _specinfo =
{
    { "O3V", 44900, -4.01, 5.82, 13.43, -5.8, -0.33, INFINITY },
    { "O4V", 42900, -3.89, 5.65, 12.13, -5.5, -0.326, INFINITY },
    { "O5V", 41400, -3.76, 5.54, 11.45, -5.35, -0.323, INFINITY },
    { "O6V", 39500, -3.57, 5.36, 10.27, -5.1, -0.321, INFINITY },
    { "O7V", 37100, -3.41, 5.18, 9.42, -4.8, -0.318, 27 },
    { "O8V", 35100, -3.24, 4.99, 8.47, -4.5, -0.315, 23 },
    { "O9V", 33300, -3.11, 4.82, 7.72, -4.2, -0.312, 19.8 },
    { "B0V", 31400, -2.99, 4.65, 7.16, -3.9, -0.301, 17.7 },
    { "B1V", 26000, -2.58, 4.13, 5.71, -3, -0.278, 11 },
    { "B2V", 20600, -2.03, 3.43, 4.06, -1.8, -0.215, 7.3 },
    { "B3V", 17000, -1.54, 2.99, 3.61, -1.2, -0.178, 5.4 },
    { "B4V", 16400, -1.49, 2.89, 3.46, -1, -0.165, 5.1 },
    { "B5V", 15700, -1.34, 2.77, 3.36, -0.85, -0.156, 4.7 },
    { "B6V", 14500, -1.13, 2.57, 3.27, -0.55, -0.14, 4.3 },
    { "B7V", 14000, -1.05, 2.48, 2.94, -0.4, -0.128, 3.92 },
    { "B8V", 12300, -0.73, 2.19, 2.86, 0, -0.109, 3.38 },
    { "B9V", 10700, -0.42, 1.86, 2.49, 0.5, -0.07, 2.75 },
    { "A0V", 9700, -0.21, 1.58, 2.193, 0.99, 0, 2.18 },
    { "A1V", 9300, -0.14, 1.49, 2.136, 1.16, 0.035, 2.05 },
    { "A2V", 8800, -0.07, 1.38, 2.117, 1.35, 0.07, 1.98 },
    { "A3V", 8600, -0.04, 1.23, 1.861, 1.7, 0.1, 1.86 },
    { "A4V", 8250, -0.02, 1.13, 1.794, 1.94, 0.14, 1.93 },
    { "A5V", 8100, 0, 1.09, 1.785, 2.01, 0.16, 1.88 },
    { "A6V", 7910, 0.005, 1.05, 1.775, 2.12, 0.185, 1.83 },
    { "A7V", 7760, 0.01, 1, 1.75, 2.23, 0.21, 1.77 },
    { "A8V", 7590, 0.02, 0.96, 1.747, 2.32, 0.25, 1.81 },
    { "A9V", 7400, 0.02, 0.92, 1.747, 2.43, 0.27, 1.75 },
    { "F0V", 7220, 0.01, 0.86, 1.728, 2.57, 0.295, 1.61 },
    { "F1V", 7020, 0.005, 0.79, 1.679, 2.76, 0.33, 1.5 },
    { "F2V", 6820, -0.005, 0.71, 1.622, 2.97, 0.37, 1.46 },
    { "F3V", 6750, -0.01, 0.67, 1.578, 3.08, 0.39, 1.44 },
    { "F4V", 6670, -0.015, 0.62, 1.533, 3.2, 0.41, 1.38 },
    { "F5V", 6550, -0.02, 0.56, 1.473, 3.37, 0.44, 1.33 },
    { "F6V", 6350, -0.03, 0.43, 1.359, 3.69, 0.486, 1.25 },
    { "F7V", 6280, -0.035, 0.39, 1.324, 3.8, 0.5, 1.21 },
    { "F8V", 6180, -0.04, 0.29, 1.221, 4.05, 0.53, 1.18 },
    { "F9V", 6050, -0.05, 0.22, 1.167, 4.25, 0.56, 1.13 },
    { "G0V", 5930, -0.065, 0.13, 1.1, 4.48, 0.595, 1.06 },
    { "G1V", 5860, -0.073, 0.08, 1.06, 4.62, 0.622, 1.03 },
    { "G2V", 5770, -0.085, 0.01, 1.012, 4.8, 0.65, 1 },
    { "G3V", 5720, -0.095, -0.01, 1.002, 4.87, 0.66, 0.99 },
    { "G4V", 5680, -0.1, -0.04, 0.991, 4.93, 0.67, 0.985 },
    { "G5V", 5660, -0.105, -0.05, 0.977, 4.98, 0.68, 0.98 },
    { "G6V", 5600, -0.115, -0.1, 0.949, 5.1, 0.7, 0.97 },
    { "G7V", 5550, -0.125, -0.13, 0.927, 5.2, 0.71, 0.95 },
    { "G8V", 5480, -0.14, -0.17, 0.914, 5.3, 0.73, 0.94 },
    { "G9V", 5380, -0.16, -0.26, 0.853, 5.55, 0.775, 0.9 },
    { "K0V", 5270, -0.195, -0.34, 0.813, 5.78, 0.816, 0.88 },
    { "K1V", 5170, -0.23, -0.39, 0.797, 5.95, 0.857, 0.86 },
    { "K2V", 5100, -0.26, -0.43, 0.783, 6.07, 0.884, 0.82 },
    { "K3V", 4830, -0.375, -0.55, 0.755, 6.5, 0.99, 0.78 },
    { "K4V", 4600, -0.52, -0.69, 0.713, 6.98, 1.09, 0.73 },
    { "K5V", 4440, -0.63, -0.76, 0.701, 7.28, 1.15, 0.7 },
    { "K6V", 4300, -0.75, -0.86, 0.669, 7.64, 1.24, 0.69 },
    { "K7V", 4100, -0.93, -1, 0.63, 8.16, 1.34, 0.64 },
    { "K8V", 3990, -1.03, -1.06, 0.615, 8.43, 1.363, 0.62 },
    { "K9V", 3930, -1.07, -1.1, 0.608, 8.56, 1.4, 0.59 },
    { "M0V", 3850, -1.15, -1.16, 0.588, 8.8, 1.42, 0.57 },
    { "M1V", 3660, -1.42, -1.39, 0.501, 9.64, 1.485, 0.5 },
    { "M2V", 3560, -1.62, -1.54, 0.446, 10.21, 1.505, 0.44 },
    { "M3V", 3430, -1.93, -1.79, 0.361, 11.15, 1.53, 0.37 },
    { "M4V", 3210, -2.51, -2.14, 0.274, 12.61, 1.65, 0.23 },
    { "M5V", 3060, -3.11, -2.52, 0.196, 14.15, 1.83, 0.162 },
    { "M6V", 2810, -4.13, -2.98, 0.137, 16.32, 2.01, 0.102 },
    { "M7V", 2680, -4.99, -3.19, 0.12, 17.7, 2.12, 0.09 },
    { "M8V", 2570, -5.65, -3.28, 0.114, 18.6, 2.15, 0.085 },
    { "M9V", 2380, -5.86, -3.52, 0.102, 19.4, 2.17, 0.079 },
    { "L0V", 2270, -6.25, -3.6, 0.102, 20, INFINITY, 0.077 },
    { "L1V", 2160, -6.48, -3.71, 0.0995, 20.5, INFINITY, 0.076 },
    { "L2V", 2060, -6.62, -3.82, 0.097, 20.9, INFINITY, 0.075 },
    { "L3V", 1920, -7.05, -3.96, 0.0942, 21.7, INFINITY, INFINITY },
    { "L4V", 1870, -7.53, -4.01, 0.094, 22.3, INFINITY, INFINITY },
    { "L5V", 1710, -7.87, -4.2, 0.0909, INFINITY, INFINITY, INFINITY },
    { "L6V", 1550, INFINITY, -4.38, 0.0891, INFINITY, INFINITY, INFINITY },
    { "L7V", 1530, INFINITY, -4.41, 0.0886, INFINITY, INFINITY, INFINITY },
    { "L8V", 1420, INFINITY, -4.55, 0.0875, INFINITY, INFINITY, INFINITY },
    { "L9V", 1370, INFINITY, -4.61, 0.0877, INFINITY, INFINITY, INFINITY },
    { "T0V", 1255, INFINITY, -4.66, 0.098, INFINITY, INFINITY, INFINITY },
    { "T1V", 1240, INFINITY, -4.69, 0.1, INFINITY, INFINITY, INFINITY },
    { "T2V", 1220, INFINITY, -4.73, 0.1, INFINITY, INFINITY, INFINITY },
    { "T3V", 1200, INFINITY, -4.77, 0.102, INFINITY, INFINITY, INFINITY },
    { "T4V", 1180, INFINITY, -4.84, 0.101, INFINITY, INFINITY, INFINITY },
    { "T5V", 1160, INFINITY, -4.95, 0.101, INFINITY, INFINITY, INFINITY },
    { "T6V", 950, INFINITY, -5.12, 0.1, INFINITY, INFINITY, INFINITY },
    { "T7V", 825, INFINITY, -5.37, 0.098, INFINITY, INFINITY, INFINITY },
    { "T8V", 680, INFINITY, -5.71, 0.095, INFINITY, INFINITY, INFINITY },
    { "T9V", 560, INFINITY, -6.15, 0.1, INFINITY, INFINITY, INFINITY }
};

// Constructs single star with a specific object type code.
// All fields except type code are set to empty strings or infinity,
// signifying unknown/undefined values.

SSStar::SSStar ( SSObjectType type ) : SSObject ( type )
{
    _names = vector<string> ( 0 );
    _idents = vector<SSIdentifier> ( 0 );

    _parallax = 0.0;
    _radvel = INFINITY;
    _position = _velocity = SSVector ( INFINITY, INFINITY, INFINITY );
    _Vmag = INFINITY;
    _Bmag = INFINITY;
    
    _spectrum = "";
}

// Constructs single star with type code set to indicate "single star".
// All other fields set to values, signifying unknown/undefined.

SSStar::SSStar ( void ) : SSStar ( kTypeStar )
{

}

// Constructs variable star with all fields except type code
// set to empty strings or infinity, signifying "unknown".

SSVariableStar::SSVariableStar ( void ) : SSStar ( kTypeVariableStar )
{
    _varType = "";
    _varMaxMag = INFINITY;
    _varMinMag = INFINITY;
    _varPeriod = INFINITY;
    _varEpoch = INFINITY;
}

// Constructs double star with all fields except type code
// set to empty strings or infinity, signifying "unknown".

SSDoubleStar::SSDoubleStar ( void ) : SSStar ( kTypeDoubleStar )
{
    _comps = "";
    _magDelta = INFINITY;
    _sep = INFINITY;
    _PA = INFINITY;
    _PAyr = INFINITY;
    _pOrbit = nullptr;
    _pPrimary = nullptr;
}

// Destructs double star. Deletes binary star orbit data if present.

SSDoubleStar::~SSDoubleStar ( void )
{
    delete _pOrbit;
}

// Copy constuctor clones binary orbit data record, copies all other fields.
// Prevents double-deletes when freeing structs.

SSDoubleStar::SSDoubleStar ( const SSDoubleStar &other ) : SSStar ( other )
{
    *this = other;
    _pOrbit = _pOrbit ? new SSOrbit ( *_pOrbit ) : nullptr;
}

// Constructs double variable star with all fields except type code
// set to empty strings or infinity, signifying "unknown".

SSDoubleVariableStar::SSDoubleVariableStar ( void ) : SSDoubleStar(), SSVariableStar()
{
    _type = kTypeDoubleVariableStar;
}

// Constructs deep sky object with the specified type code;
// all other fields are set to unknown/undefined values.

SSDeepSky::SSDeepSky ( SSObjectType type ) : SSStar ( type )
{
    _majAxis = INFINITY;
    _minAxis = INFINITY;
    _PA = INFINITY;
}

// Returns i-th identifier in this star's ident vector,
// or null identifier (i.e. zero) if i is out of range

SSIdentifier SSStar::getIdentifier ( int i )
{
    if ( i >= 0 && i < _idents.size() )
        return _idents[i];
    else
        return SSIdentifier();
}

// Returns this star's identifier in a specific catalog.
// If not present, returns null identifier (i.e. zero).

SSIdentifier SSStar::getIdentifier ( SSCatalog cat )
{
    for ( int i = 0; i < _idents.size(); i++ )
        if ( _idents[i].catalog() == cat )
            return _idents[i];
    
    return SSIdentifier();
}

bool SSStar::addIdentifier ( SSIdentifier ident )
{
    return SSAddIdentifier ( ident, _idents );
}

void SSStar::sortIdentifiers ( void )
{
    sort ( _idents.begin(), _idents.end(), compareSSIdentifiers );
}

// Compute star's heliocentric position and velocity in AU and AU per day in the fundamental (J2000 mean equatorial)
// reference frame at the Julian Ephemeris Date specified inside the SSCoordinates object.
// If star's parallax is unknown, returned position will be approximately a unit vector.
// If parallax is known, position vector magnitude will be > 206265 (i.e., 1 parsec in AU).

void SSStar::computePositionVelocity ( SSCoordinates &coords, SSVector &pos, SSVector &vel )
{
    // Start by assuming star's heliocentric position is unchanged from J2000.
    
    pos = _position;
    vel = _velocity;
    
    // If applying stellar space motion, and the star's space motion is known, add its space velocity
    // (times years since J2000) to its J2000 position.

    if ( coords.getStarMotion() && ! ::isinf ( _velocity.x ) )
        pos += _velocity * ( coords.getJED() - SSTime::kJ2000 ) / SSTime::kDaysPerJulianYear;
    
    // If star's parallax is known, scale position and velocity by parallax to AU and AU/day.
    
    if ( _parallax > 0.0 )
    {
        pos *= coords.kAUPerParsec / _parallax;
        vel *= SSTime::kDaysPerJulianYear * coords.kAUPerParsec / _parallax;
    }
}

// Compute star's apparent direction, distance, and magnitude at the Julian Ephemeris Date
// specified inside the SSCoordinates object.

void SSStar::computeEphemeris ( SSCoordinates &coords )
{
    // Start by assuming star's current apparent direction vector is unchanged from J2000.
    
    _direction = _position;

    // If applying stellar space motion, and the star's space motion is known, add its space velocity
    // (times years since J2000) to its J2000 position.

    if ( coords.getStarMotion() && ! ( _velocity.isinf() || _velocity.isnan() ) )
        _direction += _velocity * ( coords.getJED() - SSTime::kJ2000 ) / SSTime::kDaysPerJulianYear;
    
    // If applying heliocentric parallax, and the star's parallax is known, subtract the observer's
    // position divided by the star's J2000 distance.
    
    if ( coords.getStarParallax() && _parallax > 0.0 )
        _direction -= coords.getObserverPosition() * ( _parallax / coords.kAUPerParsec );

    // If star's apparent direction is the same as in J2000, we ignored both its space motion and parallax.
    // If star's parallax is known, convert to distance in AU; otherwise set distance to infinity.
    // Star's current visual magnitude equals its J2000 magnitude.

    if ( _direction == _position )
    {
        _distance = _parallax > 0.0 ? coords.kAUPerParsec / _parallax : INFINITY;
        _magnitude = _Vmag < INFINITY ? _Vmag : _Bmag;
    }
    else
    {
        // If we applied stellar space motion or parallax, compute "delta" (ratio of star's current distance
        // to its J2000 distance). Then normalize direction to unit vector. If star's J2000 parallax is known,
        // get its current distance in AU. Get current visual magnitude by adjusting J2000 magnitude for delta.

        double delta = _direction.magnitude();
        _direction = _direction / delta;
        _distance = _parallax > 0.0 ? delta * coords.kAUPerParsec / _parallax : INFINITY;
        _magnitude = ( _Vmag < INFINITY ? _Vmag : _Bmag ) + 5.0 * log10 ( delta );
    }

    // Finally apply aberration of light, if desired.
    
    if ( coords.getAberration() )
        _direction = coords.applyAberration ( _direction );
}

// Compute double star's apparent direction, distance, and magnitude at the Julian Ephemeris Date
// specified inside the SSCoordinates object.

void SSDoubleStar::computeEphemeris ( SSCoordinates &coords )
{
    if ( _pPrimary == nullptr || _pOrbit == nullptr || _pPrimary == this )
    {
        SSStar::computeEphemeris ( coords );
        return;
    }
    
    // NOTE: this only works when viewed from within Solar System
    
    SSVector pos, vel;
    _pOrbit->toPositionVelocity ( coords.getJED(), pos, vel );
    SSVector dir = _pPrimary->getDirection() + pos / SSAngle::kArcsecPerRad;
    
    _direction = dir.normalize();
    _distance = _pPrimary->getDistance();
    _magnitude = _Vmag < INFINITY ? _Vmag : _Bmag;  // TODO: correct for distance
    
    // Don't apply aberration, was already applied to primary star
}

// Returns this star's apparent proper motion in the coordinate system (frame)
// at the observer time and location stored in the SSCoordinates object (coords).
// Assumes star's apparent direction and current distance have already
// been computed via computeEphemeris(). The proper motion in RA (motion.lon)
// and Dec (motion.lat) are both in radians per year, EXCEPT if frame == kHorizon;
// then angular motion is in radians per day. The radial velocity (motion.rad) is
// always returned light years per year (i.e. fraction of light speed) and will be
// infinite if unknown.

SSSpherical SSStar::computeApparentMotion ( SSCoordinates &coords, SSFrame frame )
{
    SSSpherical motion ( INFINITY, INFINITY, _radvel );
    
    // Special case for Horizon frame: ignore annual proper motion,
    // return angular motion in radians per day.
    
    if ( frame == kHorizon )
    {
        SSVector pos = coords.transform ( kFundamental, kHorizon, _direction );
        SSVector vel;  // assume star is stationary w/ respect to fundamental frame
        
        // Add this twist for the Horizon frame because it is rotating, not inertial.
        // See https://ocw.mit.edu/courses/aeronautics-and-astronautics/16-07-dynamics-fall-2009/lecture-notes/MIT16_07F09_Lec08.pdf
        
        static SSVector omega ( 0.0, 0.0, SSAngle::kTwoPi * SSTime::kSiderealPerSolarDays );
        vel += coords.transform ( kEquatorial, kHorizon, omega ).crossProduct ( pos );

        motion = pos.toSphericalVelocity ( vel );
        motion.rad = _radvel;
    }
    else if ( ! _velocity.isinf() )
    {
        SSVector vel = _velocity;
        SSVector pos = _position;
        
        // If current distance and parallax are known, compute space velocity and relative direction.
        
        if ( ! ::isinf ( _distance ) && _parallax > 0.0 )
        {
            vel = vel * SSCoordinates::kAUPerParsec / (double) _parallax; // [AU/year]
            pos = _direction * _distance;                                 // [AU]
        }
        
        // Transform star's position and velocity to the desired output coordinate frame.
        
        pos = coords.transform ( kFundamental, frame, pos );
        vel = coords.transform ( kFundamental, frame, vel );
        
        // Overwrite J2000 radial velocity (if known) with current radial velocity in light years per year
        
        motion = pos.toSphericalVelocity ( vel );
        motion.rad = ::isinf ( _radvel ) || _parallax == 0.0 ? _radvel : motion.rad / SSCoordinates::kAUPerLY;
    }
    
    return motion;
}

// Sets this star's spherical coordinates and distance in the fundamental frame,
// i.e. the star's mean equatorial J2000 coordinates at epoch 2000.
// The star's RA (coords.lon) and Dec (coords.lat) are in radians.
// The star's distance in light years (coords.rad) may be infinite if unknown.

void SSStar::setFundamentalCoords ( SSSpherical coords )
{
    _parallax = ::isinf ( coords.rad ) ? 0.0 : SSCoordinates::kLYPerParsec / coords.rad;
    _position = SSSpherical ( coords.lon, coords.lat, 1.0 );
}

// Sets this star's spherical coordinates and proper motion in the fundamental frame
// i.e. the star's mean equatorial J2000 coordinates and proper motion at epoch 2000.
// The star's RA (coords.lon) and Dec (coords.lat) are in radians.
// The stars proper motion in RA (motion.ra) and dec (motion.dec) are in radians per Julian year.
// The star's distance in light years (coords.rad) may be infinite if unknown.
// The star's radial velocity in light years per year (motion.rad) may be infinite if unknown.
// Mathematically, both coordinates and motion are required to compute the star's rectangular
// heliocentric position and motion; practically, if you have its motion you'll also have its position,
// so we pass them both here. You can extract them separately (see below).

void SSStar::setFundamentalMotion ( SSSpherical coords, SSSpherical motion )
{
    _parallax = ::isinf ( coords.rad ) ? 0.0 : SSCoordinates::kLYPerParsec / coords.rad;
    _radvel = motion.rad;

    // if distance or radial velocity are unknown, treat them as zero;
    // otherwise divide radial velocity by distance; set unit distance.
    
    if ( ::isinf ( coords.rad ) || ::isinf ( motion.rad ) )
        motion.rad = 0.0;
    else
        motion.rad /= coords.rad;
    
    coords.rad = 1.0;
    
    _position = coords.toVectorPosition();
    
    if ( ::isinf ( motion.lon ) || ::isinf ( motion.lat ) )
        _velocity = SSVector ( INFINITY, INFINITY, INFINITY );
    else
        _velocity = coords.toVectorVelocity ( motion );
}

// Returns this star's heliocentric spherical coordinates in the fundamental
// J2000 mean equatorial frame at epoch J2000.  The star's RA (coords.lon)
// and Dec (coords.lat) are in radians.  Its distance in light years (coords.rad)
// will be infinite if unknown.

SSSpherical SSStar::getFundamentalCoords ( void )
{
    SSSpherical coords = _position.toSpherical();
    coords.rad = ( ::isinf ( _parallax ) || _parallax == 0.0 ) ? INFINITY : SSCoordinates::kLYPerParsec / _parallax;
    return coords;
}

// Returns this star's heliocentric proper motion in the fundamental J2000
// mean equatorial frame at epoch J2000.  The proper motion in RA (motion.lon)
// and Dec (motion.lat) are both in radians per year.  Its radial velocity
// (motion.rad) is in light years per year and will be infinite if unknown.

SSSpherical SSStar::getFundamentalMotion ( void )
{
    SSSpherical motion = _position.toSphericalVelocity ( _velocity );
    motion.rad = _radvel;
    return motion;
}

// Converts B-V color index (bv) to RGB color.
// B-V will be clamped to range -0.4 to +2.0.
// RGB values will be returned in the range 0.0 to 1.0.
// from https://stackoverflow.com/questions/21977786/star-b-v-color-index-to-apparent-rgb-color

void SSStar::bmv2rgb ( float bv, float &r, float &g, float &b )
{
    double t = r = g = b = 0.0;

    if ( bv < -0.4 )
        bv = -0.4;
    
    if ( bv > 2.0 )
        bv = 2.0;

    // red
    
    if ( bv >= -0.40 && bv < 0.00 )
    {
        t = ( bv + 0.40 ) / ( 0.00 + 0.40 );
        r = 0.61 + ( 0.11 * t ) + ( 0.1 * t * t );
    }
    else if ( bv >= 0.00 && bv < 0.40 )
    {
        t = ( bv - 0.00 ) / ( 0.40 - 0.00 );
        r = 0.83 + ( 0.17 * t );
    }
    else if ( bv >= 0.40 && bv < 2.10 )
    {
        t = ( bv - 0.40 ) / ( 2.10 - 0.40 );
        r = 1.00;
    }
    
    // green
    
    if ( bv >= -0.40 && bv < 0.00 )
    {
        t = ( bv + 0.40 ) / ( 0.00 + 0.40 );
        g = 0.70 + ( 0.07 * t ) + ( 0.1 * t * t );
    }
    else if ( bv >= 0.00 && bv < 0.40 )
    {
        t = ( bv - 0.00 ) / ( 0.40 - 0.00 );
        g = 0.87 + ( 0.11 * t );
    }
    else if ( bv >= 0.40 && bv < 1.60 )
    {
        t = ( bv - 0.40 ) / ( 1.60 - 0.40 );
        g = 0.98 - ( 0.16 * t );
    }
    else if ( bv >= 1.60 && bv < 2.00 )
    {
        t = ( bv - 1.60 ) / ( 2.00 - 1.60 );
        g = 0.82 - ( 0.5 * t * t );
    }
    
    // blue
    
    if ( bv >= -0.40 && bv < 0.40 )
    {
        t = ( bv + 0.40 ) / ( 0.40 + 0.40 );
        b = 1.00;
    }
    else if ( bv >= 0.40 && bv < 1.50 )
    {
        t = ( bv - 0.40 ) / ( 1.50 - 0.40 );
        b = 1.00 - ( 0.47 * t ) + ( 0.1 * t * t );
    }
    else if ( bv >= 1.50 && bv < 1.94 )
    {
        t = ( bv - 1.50 ) / ( 1.94 - 1.50 );
        b = 0.63 - ( 0.6 * t * t );
    }
}

// Converts B-V color index (bv) to temperature in Kelvin.
// from https://en.wikipedia.org/wiki/Color_index
// Superseded by colorTemperature(), below.

float SSStar::bmv2temp ( float bv )
{
   return 4600.0 * ( ( 1.0 / ( ( 0.92 * bv ) + 1.7 ) ) + ( 1.0 / ( ( 0.92 * bv ) + 0.62 ) ) );
}

// Converts B-V index to stellar surface effective temperature in Kelvins.
// From Table 2 of "ON THE USE OF EMPIRICAL BOLOMETRIC CORRECTIONS FOR STARS",
// Guillermo Torres, Harvard-Smithsonian Center for Astrophysics, 2010.
// https://iopscience.iop.org/article/10.1088/0004-6256/140/5/1158/pdf

float SSStar::colorTemperature ( float bv, int lumclass )
{
    float t = 0.0;
    
    if ( lumclass <= LumClass::Ib )
    {
        t = 4.012559732366214
          - 1.055043117465989 * bv
          + 2.133394538571825 * bv * bv
          - 2.459769794654992 * bv * bv * bv
          + 1.349423943497744 * bv * bv * bv * bv
          - 0.283942579112032 * bv * bv * bv * bv * bv;
    }
    else
    {
        t = 3.979145106714099
          - 0.654992268598245 * bv
          + 1.740690042385095 * bv * bv
          - 4.608815154057166 * bv * bv * bv
          + 6.792599779944473 * bv * bv * bv * bv
          - 5.396909891322525 * bv * bv * bv * bv * bv
          + 2.192970376522490 * bv * bv * bv * bv * bv * bv
          - 0.359495739295671 * bv * bv * bv * bv * bv * bv * bv;
    }
    
    return pow ( 10.0, t );
}

// Converts B-V index to bolometric correction in magnitudes.
// From Table 1 of "ON THE USE OF EMPIRICAL BOLOMETRIC CORRECTIONS FOR STARS",
// Guillermo Torres, Harvard-Smithsonian Center for Astrophysics, 2010.
// https://iopscience.iop.org/article/10.1088/0004-6256/140/5/1158/pdf

float SSStar::bolometricCorrection ( float t )
{
    float bc = INFINITY;
    
    t = log10 ( t );
    if ( t > 3.9 )
    {
        bc = -0.118115450538963E+06
           + 0.137145973583929E+06 * t
           - 0.636233812100225E+05 * t * t
           + 0.147412923562646E+05 * t * t * t
           - 0.170587278406872E+04 * t * t * t * t
           + 0.788731721804990E+02 * t * t * t * t * t;
    }
    else if ( t > 3.7 )
    {
        bc = -0.370510203809015E+05
           + 0.385672629965804E+05 * t
           - 0.150651486316025E+05 * t * t
           + 0.261724637119416E+04 * t * t * t
           - 0.170623810323864E+03 * t * t * t * t;
    }
    else
    {
        bc = -0.190537291496456E+05
           + 0.155144866764412E+05 * t
           - 0.421278819301717E+04 * t * t
           + 0.381476328422343E+03 * t * t * t;
    }

    return bc;
}

// Returns a star's absolute magnitude, given its apparent magnitude (appMag)
// and distance in parsecs (dist). If the distance is zero or infinite, returns infinity.

double SSStar::absoluteMagnitude ( double appMag, double dist )
{
    if ( dist > 0.0 && dist < INFINITY )
        return appMag - 5.0 * ( log10 ( dist ) - 1.0 );
    else
        return -INFINITY;
}

// Returns a star's apparent magnitude, given its absolute magnitude (absMag)
// and distance in parsecs (dist). If the distance is zero or infinite, returns infinity.

double SSStar::apparentMagnitude ( double absMag, double dist )
{
    if ( dist > 0.0 && dist < INFINITY )
        return absMag + 5.0 * ( log10 ( dist ) - 1.0 );
    else
        return dist <= 0.0 ? -INFINITY : INFINITY;
}

// Returns a star's distance in parsecs from the difference between
// its apparent and absolute magnitudes.

double SSStar::distanceFromMagnitude ( double appMag, double absMag )
{
    return pow ( 10.0, ( appMag - absMag ) / 5.0 ) + 1.0;
}

// Returns the brightness ratio that corresponds to the magnitude difference (magDiff)
// between two stars. If magDiff < 0, the ratio is > 1; if magDiff > 0, the ratio is < 1.
// If magDiff positive infinite, the ratio is inifite; if mgative infinite, the ratio is zero.

double SSStar::brightnessRatio ( double magDiff )
{
    if ( ::isinf ( magDiff ) )
        return magDiff > 0.0 ? INFINITY : 0.0;
    else
        return pow ( 10.0, magDiff / 2.5 );
}

// Given the brightness ratio between two objects, returns their difference in magnitudes.
// If the ratio is < 1, the magnitude difference is postive; if > 1, it is negative.

double SSStar::magnitudeDifference ( double ratio )
{
    return -2.5 * log10 ( ratio );
}

// Returns the combined magnitude of two stars with individual magnitudes
// mag1 and mag2. If either magnitude is infinite, the function returns the
// other magnitude.

double SSStar::magnitudeSum ( double mag1, double mag2 )
{
    if ( ::isinf ( mag2 ) )
        return mag1;
    else if ( ::isinf ( mag1 ) )
        return mag2;
    else
        return mag2 + magnitudeDifference ( 1.0 + brightnessRatio ( mag1 - mag2 ) );
}

// A Moffat function describes a stellar image profile on a CCD image:
// https://ned.ipac.caltech.edu/level5/Stetson/Stetson2_2_1.html
// Maximum intensity at center of star image, in arbitrary units, is (max).
// Square of distance in pixels from star image center is (r2).
// Power law exponent (beta) describes rate intensity declines as distance from center increases.
// For real stars beta values from 2.5 to 4.675 are used, see:
// https://www.startools.org/modules/decon/usage/the-point-spread-function-psf

double SSStar::moffatFunction ( double max, double r2, double beta )
{
    return max / pow ( 1.0 + r2, beta );
}

// Computes radius in pixels from center of a Moffat-function star image profile
// where intensity equals a given value (z). Other Moffat function parameters,
// (max) and (beta) are as described above for moffatFunction().

double SSStar::moffatRadius ( double z, double max, double beta )
{
    return sqrt ( pow ( max / z, 1.0 / beta ) - 1.0 );
}

// Given a stellar spectral class string, returns integer code for spectral type.
// Note obsolete types R and N are now considered part of C but are still parsed
// separately since these are found in the SKY2000 Master Star Catalog.
// Assumes leading and trailing whitespace has been removed.
// See https://en.wikipedia.org/wiki/Stellar_classification

int SSStar::spectralType ( const string &spectrum )
{
    int spectype = 0;
    static char types[14] = { 'W', 'O', 'B', 'A', 'F', 'G', 'K', 'M', 'L', 'T', 'R', 'N', 'S', 'C'  };

    for ( int i = 0; i < spectrum.length(); i++ )
    {
        for ( int k = 0; k < 14; k++ )
        {
            if ( spectrum[i] == types[k] )
            {
                spectype = k * 10;
                i++;
                if ( spectrum[i] >= '0' && spectrum[i] <= '9' )
                    spectype += spectrum[i] - '0';
                    
                return spectype;
            }
        }
    }
    
    return 0;
}

// Given a stellar spectral class string, returns integer code for luminosity class.
// Assumes leading and trailing whitespace has been removed.
// See https://en.wikipedia.org/wiki/Stellar_classification

int SSStar::luminosityClass ( const string &spectrum )
{
    int i, lumclass = 0;
    
    // Mt. Wilson luminosity classes c, g, sg, d, sd, and white dwarfs (D) are prefixed.
    
    if ( spectrum[0] == 'c' )
        lumclass = LumClass::Iab;

    if ( spectrum[0] == 'g' )
        lumclass = LumClass::III;
    
    if ( spectrum[0] == 's' && spectrum.size() >= 2 )
    {
        if ( spectrum[1] == 'g' )
            lumclass = LumClass::IV;
        else if ( spectrum[1] == 'd' )
            lumclass = LumClass::VI;
    }

    if ( spectrum[0] == 'd' )
        lumclass = LumClass::V;
    
    if ( spectrum[0] == 'D' )
        lumclass = LumClass::VII;
    
    if ( lumclass > 0 )
        return lumclass;
    
    // Yerkes (Morgan-Keenan) luminosity classes are suffixed after spectral type.
    // Find the first luminosity class character in the string.
    // If none, return zero to indicate unknown luminosity class.
    
    i = (int) spectrum.find_first_of ( "IV" );
    if ( i == string::npos )
        return 0;
    
    // Class V, VI, VII
    
    if ( spectrum.compare ( i, 3, "VII", 3 ) == 0 )
        lumclass = LumClass::VII;
    else if ( spectrum.compare ( i, 2, "VI", 2 ) == 0 )
        lumclass = LumClass::VI;
    else if ( spectrum.compare ( i, 1, "V", 1 ) == 0 )
        lumclass = LumClass::V;

    // Class Ia0, Ia, Iab, Ib

    else if ( spectrum.compare ( i, 3, "Iab", 3 ) == 0 )
        lumclass = LumClass::Iab;
    else if ( spectrum.compare ( i, 3, "IAB", 3 ) == 0 )
        lumclass = LumClass::Iab;
    else if ( spectrum.compare ( i, 3, "Ia0", 3 ) == 0 )
        lumclass = LumClass::Ia0;
    else if ( spectrum.compare ( i, 3, "IA0", 3 ) == 0 )
        lumclass = LumClass::Ia0;
    else if ( spectrum.compare ( i, 3, "IA+", 3 ) == 0 )
        lumclass = LumClass::Ia0;
    else if ( spectrum.compare ( i, 2, "Ia", 2 ) == 0 )
        lumclass = LumClass::Ia;
    else if ( spectrum.compare ( i, 2, "IA", 2 ) == 0 )
        lumclass = LumClass::Ia;
    else if ( spectrum.compare ( i, 2, "Ib", 2 ) == 0 )
        lumclass = LumClass::Ib;
    else if ( spectrum.compare ( i, 2, "IB", 2 ) == 0 )
        lumclass = LumClass::Ib;

    // Class II, III, and IV

    else if ( spectrum.compare ( i, 3, "III", 3 ) == 0 )
        lumclass = LumClass::III;
    else if ( spectrum.compare ( i, 2, "II", 2 ) == 0 )
        lumclass = LumClass::II;
    else if ( spectrum.compare ( i, 2, "IV", 2 ) == 0 )
        lumclass = LumClass::IV;
    
    return lumclass;
}

// Given a stellar spectral class string, parses integer code for spectral type
// and luminosity class. Assumes leading and trailing whitespace has been removed.
// See https://en.wikipedia.org/wiki/Stellar_classification

bool SSStar::parseSpectrum ( const string &spectrum, int &spectype, int &lumclass )
{
    spectype = spectralType ( spectrum );
    lumclass = luminosityClass ( spectrum );
    return spectype || lumclass;
}

// Given an integer spectral type and luminosity class code,
// formats and returns equivalent spectral class string.
// Note obsolete types R and N are now considered part of C.

string SSStar::formatSpectrum ( int spectype, int lumclass )
{
    string spectrum = "";
    static char types[14] = { 'W', 'O', 'B', 'A', 'F', 'G', 'K', 'M', 'L', 'T', 'R', 'N', 'S', 'C'  };
    
    if ( lumclass == LumClass::VII )
        spectrum.append ( 1, 'D' );
    
    if ( spectype > SpecType::W0 && spectype < SpecType::T0 + 9 )
    {
        spectrum.append ( 1, types[ spectype / 10 ] );
        spectrum.append ( 1, '0' + spectype % 10 );
    }
    
    if ( lumclass == LumClass::Ia0 )
        spectrum.append ( "Ia0" );
    else if ( lumclass == LumClass::Ia )
        spectrum.append ( "Ia" );
    else if ( lumclass == LumClass::Iab )
        spectrum.append ( "Iab" );
    else if ( lumclass == LumClass::Ib )
        spectrum.append ( "Ib" );
    else if ( lumclass == LumClass::II )
        spectrum.append ( "II" );
    else if ( lumclass == LumClass::III )
        spectrum.append ( "III" );
    else if ( lumclass == LumClass::IV )
        spectrum.append ( "IV" );
    else if ( lumclass == LumClass::V )
        spectrum.append ( "V" );
    else if ( lumclass == LumClass::VI )
        spectrum.append ( "VI" );

    return spectrum;
}

// Returns information for the given spectral type and luminosity class;
// values that cannot be determined will be INFINITY.

SSStar::SpecInfo SSStar::spectralClassInfo ( int spectype, int lumclass )
{
    SpecInfo info = { "", INFINITY, INFINITY, INFINITY, INFINITY, INFINITY, INFINITY, INFINITY };
    
    // Use E. Mamajek's table for main sequence stars
    
    int i  = spectype - ( SpecType::O0 + 3 );
    if ( i >= 0 && i < _specinfo.size() )
        info = _specinfo[i];

    if ( lumclass == LumClass::V )
        return info;
    
    // get absolute magnitudes from "Stellar Spectral Classifications" table for other luminosity classes.
    
    i = spectype - ( SpecType::O0 + 1 );
    if ( i >= 0 && i < _specinfo.size() )
    {
        if ( lumclass == LumClass::IV )
        {
            info.spec = _speclass[i].spec + "IV";
            info.Mv = _speclass[i].MvIV;
        }
        else if ( lumclass == LumClass::III )
        {
            info.spec = _speclass[i].spec + "III";
            info.Mv = _speclass[i].MvIII;
        }
        else if ( lumclass == LumClass::II )
        {
            info.spec = _speclass[i].spec + "II";
            info.Mv = _speclass[i].MvII;
        }
        else if ( lumclass == LumClass::Ib || lumclass == LumClass::Iab )
        {
            info.spec = _speclass[i].spec + "Ib";
            info.Mv = _speclass[i].MvIb;
        }
        else if ( lumclass == LumClass::Ia || lumclass == LumClass::Ia0 )
        {
            info.spec = _speclass[i].spec + "Ia";
            info.Mv = _speclass[i].MvIa;
        }
        else    // unknown luminosity class; can't compute lumosity, radius, mass.
        {
            info.spec = _speclass[i].spec;
            info.Mv = info.Rsun = info.logL = info.Msun = INFINITY;
            return info;
        }
        
        // compute luminosity based on absolute magnitude and bolometric correction using 4.725 as the Sun's
        // absolute bolometric magnitude, calculate radius from temperature and luminosity, but set mass unknown.

        float lum = luminosity ( info.Mv, info.BCv );
        info.Rsun = radius ( info.Teff, lum );
        info.logL = log10 ( lum );
        info.Msun = INFINITY;
    }
    
    return info;
}

// Returns star's total luminosity from absolute visual magnitude (mv)
// and bolometric correction (bc); assumes Sun's absolute bolometric magnitude is 4.72.

float SSStar::luminosity ( float mv, float bc )
{
    return brightnessRatio ( 4.725 - mv - bc );
}

// Returns star's radius in solar radii from its total bolometric luminosity (lum)
// and effective surface temperature in Kelvins (temp).

float SSStar::radius ( float lum, float temp )
{
    temp = 5770.0 / temp;
    return temp * temp * sqrt ( lum );
}

// Returns CSV string from base data (excluding names and identifiers).

string SSStar::toCSV1 ( void )
{
    SSSpherical coords = getFundamentalCoords();
    SSSpherical motion = getFundamentalMotion();
    
    SSHourMinSec ra = coords.lon;
    SSDegMinSec dec = coords.lat;
    double distance = coords.rad;
    
    string csv = SSObject::typeToCode ( _type ) + ",";
    
    csv += ra.toString() + ",";
    csv += dec.toString() + ",";
    
    csv += ::isnan ( motion.lon ) || ::isinf ( motion.lon ) ? "," : format ( "%+.5f,", ( motion.lon / 15.0 ).toArcsec() );
    csv += ::isnan ( motion.lat ) || ::isinf ( motion.lat ) ? "," : format ( "%+.4f,", motion.lat.toArcsec() );
    
    csv += ::isinf ( _Vmag ) ? "," : format ( "%+.2f,", _Vmag );
    csv += ::isinf ( _Bmag ) ? "," : format ( "%+.2f,", _Bmag );
    
    csv += ::isinf ( distance ) ? "," : format ( "%.3E,", distance * SSCoordinates::kParsecPerLY );
    csv += ::isinf ( _radvel ) ? "," : format ( "%+.1f,", _radvel * SSCoordinates::kLightKmPerSec );
    
    // If spectrum contains a comma, put it in quotes.
    
    csv += _spectrum.find ( "," ) == string::npos ? _spectrum + "," : "\"" + _spectrum + "\",";
    
    return csv;
}

// Returns CSV string from identifiers and names (excluding base data).

string SSStar::toCSV2 ( void )
{
    string csv = "";
    
    for ( int i = 0; i < _idents.size(); i++ )
        csv += _idents[i].toString() + ",";
    
    for ( int i = 0; i < _names.size(); i++ )
        csv += _names[i] + ",";

    return csv;
}

// Returns CSV string including base star data plus names and identifiers.

string SSStar::toCSV ( void )
{
    return toCSV1() + toCSV2();
}

// Stores orbital elements (orbit) referenced to sky plane centered at (ra,dec),
// transforming them to fundamental J2000 mean equatorial reference plane internally.

void SSDoubleStar::setOrbit ( SSOrbit orbit, SSAngle ra, SSAngle dec )
{
    SSMatrix m = SSMatrix::rotation ( 2, 1, dec + SSAngle::kHalfPi, 2, ra );
    setOrbit ( orbit.transform ( m ) );
}

// Returns orbital elements (orbit) referenced to sky plane centered at (ra,dec),
// transforming them from internal fundamental J2000 mean equatorial reference plane.

SSOrbit SSDoubleStar::getOrbit ( SSAngle ra, SSAngle dec )
{
    SSMatrix m = SSMatrix::rotation ( 2, 2, -ra, 1, -dec - SSAngle::kHalfPi );
    return getOrbit().transform ( m );
}

// Returns CSV string from double-star data (but not SStar base class).

string SSDoubleStar::toCSVD ( void )
{
    string csv = "";
    
    // If components contains a comma, put them in quotes.

    csv += _comps.find ( "," ) == string::npos ? _comps + "," : "\"" + _comps + "\",";
    csv += ::isinf ( _magDelta ) ? "," : format ( "%+.2f,", _magDelta );
    csv += ::isinf ( _sep ) ? "," : format ( "%.1f,", _sep * SSAngle::kArcsecPerRad );
    csv += ::isinf ( _PA ) ? "," : format ( "%.1f,", _PA * SSAngle::kDegPerRad );
    csv += ::isinf ( _PAyr ) ? "," : format ( "%.2f,", _PAyr );

    if ( _pOrbit == nullptr )
        return csv + ",,,,,,,";
    
    SSSpherical coords = getFundamentalCoords();
    SSOrbit orbit = getOrbit ( coords.lon, coords.lat );
    
    csv += format ( "%.4f,", SSTime ( orbit.t ).toJulianYear() );
    csv += format ( "%.4f,", orbit.semiMajorAxis() );
    csv += format ( "%.4f,", orbit.e );
    csv += format ( "%.2f,", radtodeg ( orbit.i ) );
    csv += format ( "%.2f,", radtodeg ( orbit.w ) );
    csv += format ( "%.2f,", radtodeg ( orbit.n ) );
    csv += format ( "%.6f,", ( SSAngle::kTwoPi / orbit.mm ) / SSTime::kDaysPerJulianYear );

    return csv;
}

// Returns CSV string including base star data, double-star data,
// plus names and identifiers. Overrides SSStar::toCSV().

string SSDoubleStar::toCSV ( void )
{
    return toCSV1() + toCSVD() + toCSV2();
}

// Returns CSV string from variable-star data (but not SStar base class).

string SSVariableStar::toCSVV ( void )
{
    string csv = "";
    
    // If variable type contains a comma, put it in quotes.

    csv += _varType.find ( "," ) == string::npos ? _varType + "," : "\"" + _varType + "\",";
    csv += ::isinf ( _varMinMag ) ? "," : format ( "%+.2f,", _varMinMag );
    csv += ::isinf ( _varMaxMag ) ? "," : format ( "%+.2f,", _varMaxMag );
    csv += ::isinf ( _varPeriod ) ? "," : format ( "%.2f,", _varPeriod );
    csv += ::isinf ( _varEpoch )  ? "," : format ( "%.2f,", _varEpoch );

    return csv;
}

// Returns CSV string including base star data, variable-star data, plus names and identifiers.
// Overrides SSStar::toCSV().

string SSVariableStar::toCSV ( void )
{
    return toCSV1() + toCSVV() + toCSV2();
}

// Returns CSV string including base star data, double-star data, variable-star data,
// plus names and identifiers.  Overrides SSStar::toCSV().

string SSDoubleVariableStar::toCSV ( void )
{
    return toCSV1() + toCSVD() + toCSVV() + toCSV2();
}

// Returns CSV string from deep sky object data (but not SStar base class).

string SSDeepSky::toCSVDS ( void )
{
    string csv = "";

    csv += ::isinf ( _majAxis ) ? "," : format ( "%.2f,", _majAxis * SSAngle::kArcminPerRad );
    csv += ::isinf ( _minAxis ) ? "," : format ( "%.2f,", _minAxis * SSAngle::kArcminPerRad );
    csv += ::isinf ( _PA ) ? "," : format ( "%.1f,", _PA * SSAngle::kDegPerRad );

    return csv;
}

// Returns CSV string including base star data, double-star data,
// plus names and identifiers. Overrides SSStar::toCSV().

string SSDeepSky::toCSV ( void )
{
    if ( _type == kTypeStar )
        return toCSV1() + toCSV2();
    else
        return toCSV1() + toCSVDS() + toCSV2();
}

// Allocates a new SSStar and initializes it from a CSV-formatted string.
// Returns nullptr on error (invalid CSV string, heap allocation failure, etc.)

SSObjectPtr SSStar::fromCSV ( string csv )
{
    // split string into comma-delimited fields,
    // remove leading & trailing whitespace from each field.
    
    vector<string> fields = split_csv ( csv );
    for ( int i = 0; i < fields.size(); i++ )
        fields[i] = trim ( fields[i] );
    
    SSObjectType type = SSObject::codeToType ( fields[0] );
    if ( type < kTypeStar || type > kTypeGalaxy )
        if ( type != kTypeNonexistent )
            return nullptr;
    
    // Set expected field index for first identifier based on object type.
    // Verify that we have the required number if fiels and return if not.
    
    int fid = 0;
    if ( type == kTypeStar )
        fid = 10;
    else if ( type == kTypeDoubleStar )
        fid = 22;
    else if ( type == kTypeVariableStar )
        fid = 15;
    else if ( type == kTypeDoubleVariableStar )
        fid = 27;
    else
        fid = 13;
    
    if ( fields.size() < fid )
        return nullptr;
    
    SSHourMinSec ra ( fields[1] );
    SSDegMinSec dec ( fields[2] );
    
    double pmRA = fields[3].empty() ? INFINITY : SSAngle::kRadPerArcsec * strtofloat64 ( fields[3] ) * 15.0;
    double pmDec = fields[4].empty() ? INFINITY : SSAngle::kRadPerArcsec * strtofloat64 ( fields[4] );
    
    float vmag = fields[5].empty() ? INFINITY : strtofloat ( fields[5] );
    float bmag = fields[6].empty() ? INFINITY : strtofloat ( fields[6] );
    
    float dist = fields[7].empty() ? INFINITY : strtofloat ( fields[7] ) * SSCoordinates::kLYPerParsec;
    float radvel = fields[8].empty() ? INFINITY : strtofloat ( fields[8] ) / SSCoordinates::kLightKmPerSec;
    string spec = trim ( fields[9] );
    
    // For remaining fields, attempt to parse an identifier.
    // If we succeed, add it to the identifier vector; otherwise add it to the name vector.
    
    vector<string> names;
    vector<SSIdentifier> idents;
    
    for ( int i = fid; i < fields.size(); i++ )
    {
        if ( fields[i].empty() )
            continue;
        
        SSIdentifier ident = SSIdentifier::fromString ( fields[i] );
        if ( ident )
            idents.push_back ( ident );
        else
            names.push_back ( fields[i] );
    }
    
    SSObjectPtr pObject = SSNewObject ( type );
    SSStarPtr pStar = SSGetStarPtr ( pObject );
    SSDoubleStarPtr pDoubleStar = SSGetDoubleStarPtr ( pObject );
    SSVariableStarPtr pVariableStar = SSGetVariableStarPtr ( pObject );
    SSDeepSkyPtr pDeepSkyObject = SSGetDeepSkyPtr ( pObject );

    if ( pStar == nullptr )
        return nullptr;

    SSSpherical coords ( ra, dec, dist );
    SSSpherical motion ( pmRA, pmDec, radvel );
    
    pStar->setFundamentalMotion ( coords, motion );
    pStar->setVMagnitude ( vmag );
    pStar->setBMagnitude ( bmag );
    pStar->setSpectralType ( spec );
    pStar->setIdentifiers( idents );
    pStar->setNames ( names );
    
    if ( pDoubleStar )
    {
        string comps = fields[10];
        float dmag = fields[11].empty() ? INFINITY : strtofloat ( fields[11] );
        float sep = fields[12].empty() ? INFINITY : strtofloat ( fields[12] ) / SSAngle::kArcsecPerRad;
        float pa = fields[13].empty() ? INFINITY : strtofloat ( fields[13] ) / SSAngle::kDegPerRad;
        float year = fields[14].empty() ? INFINITY : strtofloat ( fields[14] );

        pDoubleStar->setComponents ( comps );
        pDoubleStar->setMagnitudeDelta( dmag );
        pDoubleStar->setSeparation ( sep );
        pDoubleStar->setPositionAngle ( pa );
        pDoubleStar->setPositionAngleYear ( year );
        
        if ( fields[15].length() && fields[16].length() && fields[17].length() )
        {
            SSOrbit orbit;
            
            orbit.t = SSTime::fromJulianYear ( strtofloat64 ( fields[15] ) );
            orbit.e = strtofloat ( fields[17] );
            orbit.q = strtofloat ( fields[16] ) * ( 1.0 - orbit.e );
            orbit.i = degtorad ( strtofloat ( fields[18] ) );
            orbit.w = degtorad ( strtofloat ( fields[19] ) );
            orbit.n = degtorad ( strtofloat ( fields[20] ) );
            orbit.m = 0.0;
            orbit.mm = SSAngle::kTwoPi / ( strtofloat64 ( fields[21] ) * SSTime::kDaysPerJulianYear );
            
            pDoubleStar->setOrbit ( orbit, coords.lon, coords.lat );
        }
    }
    
    if ( pVariableStar )
    {
        int fv = ( type == kTypeVariableStar ) ? 10 : 22;
            
        string vtype = fields[fv];
        float vmin = fields[fv+1].empty() ? INFINITY : strtofloat ( fields[fv+1] );
        float vmax = fields[fv+2].empty() ? INFINITY : strtofloat ( fields[fv+2] );
        float vper = fields[fv+3].empty() ? INFINITY : strtofloat ( fields[fv+3] );
        double vep = fields[fv+4].empty() ? INFINITY : strtofloat64 ( fields[fv+4] );
        
        pVariableStar->setVariableType ( vtype );
        pVariableStar->setMaximumMagnitude ( vmax );
        pVariableStar->setMinimumMagnitude ( vmin );
        pVariableStar->setPeriod ( vper );
        pVariableStar->setEpoch ( vep );
    }
    
    if ( pDeepSkyObject )
    {
        float major = fields[10].empty() ? INFINITY : strtofloat ( fields[10] ) / SSAngle::kArcminPerRad;
        float minor = fields[11].empty() ? INFINITY : strtofloat ( fields[11] ) / SSAngle::kArcminPerRad;
        float pa = fields[12].empty() ? INFINITY : strtofloat ( fields[12] ) / SSAngle::kDegPerRad;
        
        pDeepSkyObject->setMajorAxis ( major );
        pDeepSkyObject->setMinorAxis ( minor );
        pDeepSkyObject->setPositionAngle ( pa );
    }
    
    return ( pObject );
}

// Downcasts generic SSObject pointer to SSStar pointer.
// Returns nullptr if pointer is not an instance of SSStar!

SSStarPtr SSGetStarPtr ( SSObjectPtr ptr )
{
    return dynamic_cast<SSStarPtr> ( ptr );
}

// Downcasts generic SSObject pointer to SSDoubleStar pointer.
// Returns nullptr if pointer is not an instance of SSDoubleStar
// or SSDoubleVariableStar!

SSDoubleStarPtr SSGetDoubleStarPtr ( SSObjectPtr ptr )
{
    return dynamic_cast<SSDoubleStarPtr> ( ptr );
}

// Downcasts generic SSObject pointer to SSVariableStar pointer.
// Returns nullptr if pointer is not an instance of SSVariableStar
// or SSDoubleVariableStar!

SSVariableStarPtr SSGetVariableStarPtr ( SSObjectPtr ptr )
{
    return dynamic_cast<SSVariableStarPtr> ( ptr );
}

// Downcasts generic SSObject pointer to SSDeepSkyStar pointer.
// Returns nullptr if pointer is not an instance of SSDeepSky!

SSDeepSkyPtr SSGetDeepSkyPtr ( SSObjectPtr ptr )
{
    return dynamic_cast<SSDeepSkyPtr> ( ptr );
}
