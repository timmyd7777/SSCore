//  SSStar.cpp
//  SSCore
//
//  Created by Tim DeBenedictis on 3/15/20.
//  Copyright © 2020 Southern Stars. All rights reserved.

#include <algorithm>
#include <map>

#include "SSCoordinates.hpp"
#include "SSStar.hpp"

// Constructs single star with a specific object type code.
// All fields except type code are set to empty strings or infinity,
// signifying unknown/undefined values.

SSStar::SSStar ( SSObjectType type ) : SSObject ( type )
{
    _names = vector<string> ( 0 );
    _idents = vector<SSIdentifier> ( 0 );

    _parallax = 0.0;
    _radvel = HUGE_VAL;
    _position = _velocity = SSVector ( HUGE_VAL, HUGE_VAL, HUGE_VAL );
    _Vmag = HUGE_VAL;
    _Bmag = HUGE_VAL;
    
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
    _varMaxMag = HUGE_VAL;
    _varMinMag = HUGE_VAL;
    _varPeriod = HUGE_VAL;
    _varEpoch = HUGE_VAL;
}

// Constructs double star with all fields except type code
// set to empty strings or infinity, signifying "unknown".

SSDoubleStar::SSDoubleStar ( void ) : SSStar ( kTypeDoubleStar )
{
    _comps = "";
    _magDelta = HUGE_VAL;
    _sep = HUGE_VAL;
    _PA = HUGE_VAL;
    _PAyr = HUGE_VAL;
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
    _majAxis = HUGE_VAL;
    _minAxis = HUGE_VAL;
    _PA = HUGE_VAL;
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

// Compute star's apparent direction, distance, and magnitude at the Julian Ephemeris Date
// specified inside the SSCoordinates object.

void SSStar::computeEphemeris ( SSCoordinates &coords )
{
    // Start by assuming star's current apparent direction vector is unchanged from J2000.
    
    _direction = _position;

    // If applying stellar space motion, and the star's space motion is known, add its space velocity
    // (times years since J2000) to its J2000 position.

    if ( coords.getStarMotion() && ! isinf ( _velocity.x ) )
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
        _distance = _parallax > 0.0 ? coords.kAUPerParsec / _parallax : HUGE_VAL;
        _magnitude = _Vmag;
    }
    else
    {
        // If we applied stellar space motion or parallax, compute "delta" (ratio of star's current distance
        // to its J2000 distance). Then normalize direction to unit vector. If star's J2000 parallax is known,
        // get its current distance in AU. Get current visual magnitude by adjusting J2000 magnitude for delta.

        double delta = _direction.magnitude();
        _direction = _direction / delta;
        _distance = _parallax > 0.0 ? delta * coords.kAUPerParsec / _parallax : HUGE_VAL;
        _magnitude = _Vmag + 5.0 * log10 ( delta );
    }

    // Finally apply aberration of light, if desired.
    
    if ( coords.getAberration() )
        _direction = coords.applyAberration ( _direction );
}

// Sets this star's spherical coordinates and distance in the fundamental frame,
// i.e. the star's mean equatorial J2000 coordinates at epoch 2000.
// The star's RA (coords.lon) and Dec (coords.lat) are in radians.
// The star's distance in light years (coords.rad) may be infinite if unknown.

void SSStar::setFundamentalCoords ( SSSpherical coords )
{
    _parallax = isinf ( coords.rad ) ? 0.0 : SSCoordinates::kLYPerParsec / coords.rad;
    _position = SSVector ( coords.lon, coords.lat, 1.0 );
}

// Sets this star's spherical coordinates and proper motion in the fundamental frame
// i.e. the star's mean equatorial J2000 coordinates and proper motion at epoch 2000.
// The star's RA (coords.lon) and Dec (coords.lat) are in radians.
// The stars proper motion in RA (motion.ra) and dec (motion.dec) are in radians per Julian year.
// The star's distance in light years (coords.rad) may be infinite if unknown.
// The star's radial velocity in light years per year (motion.rad) may be infinite if unknown.
// Mathematically, both coordinates and motion are required to compute the star's rectangular
// heliocentric position and motion; practically, if you have its motion you'll also have its position,
// so we pass them both here.  You can extract them separately (see below).

void SSStar::setFundamentalMotion ( SSSpherical coords, SSSpherical motion )
{
    _parallax = isinf ( coords.rad ) ? 0.0 : SSCoordinates::kLYPerParsec / coords.rad;
    _radvel = motion.rad;

    // if distance or radial velocity are unknown, treat them as zero;
    // otherwise divide radial velocity by distance; set unit distance.
    
    if ( isinf ( coords.rad ) || isinf ( motion.rad ) )
        motion.rad = 0.0;
    else
        motion.rad /= coords.rad;
    
    coords.rad = 1.0;
    
    _position = coords.toVectorPosition();
    
    if ( isinf ( motion.lon ) || isinf ( motion.lat ) )
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
    coords.rad = ( isinf ( _parallax ) || _parallax == 0.0 ) ? HUGE_VAL : SSCoordinates::kLYPerParsec / _parallax;
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

// Returns the brightness ratio that corresponds to the difference between
// two different magnitudes. If mag2 > mag1, the ratio is > 1;
// if mag2 < mag1, the ratio is < 1.  If mag1 is infinite, the
// ratio iszero; if mag2 is infinite, the ratio is infinite.

double SSStar::brightnessRatio ( double mag1, double mag2 )
{
    if ( isinf ( mag2 ) )
        return INFINITY;
    else if ( isinf ( mag1 ) )
        return 0.0;
    else
        return pow ( 10.0, ( mag2 - mag1 ) / 2.5 );
}

// Given the brightness ratio between two objects, returns their difference in magnitudes.
// If the ratio is < 1, the magnitude difference is postive; if > 1, it is negative.

double SSStar::magnitudeDifference ( double ratio )
{
    return -2.5 * log10 ( ratio + 1.0 );
}

// Returns the combined magnitude of two stars with individual magnitudes
// mag1 and mag2. If either magnitude is infinite, the function returns the
// other magnitude.

double SSStar::magnitudeSum ( double mag1, double mag2 )
{
    double r = brightnessRatio ( mag1, mag2 );
    
    if ( isinf ( mag2 ) )
        return mag1;
    else if ( isinf ( mag1 ) )
        return mag2;
    else
        return mag1 - 2.5 * log10 ( r + 1.0 );
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
    
    csv += isnan ( motion.lon ) ? "," : format ( "%+.5f,", ( motion.lon / 15.0 ).toArcsec() );
    csv += isnan ( motion.lat ) ? "," : format ( "%+.4f,", motion.lat.toArcsec() );
    
    csv += isinf ( _Vmag ) ? "," : format ( "%+.2f,", _Vmag );
    csv += isinf ( _Bmag ) ? "," : format ( "%+.2f,", _Bmag );
    
    csv += isinf ( distance ) ? "," : format ( "%.3E,", distance * SSCoordinates::kParsecPerLY );
    csv += isinf ( _radvel ) ? "," : format ( "%+.1f,", _radvel * SSCoordinates::kLightKmPerSec );
    
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

// Returns CSV string from double-star data (but not SStar base class).

string SSDoubleStar::toCSVD ( void )
{
    string csv = "";
    
    csv += _comps + ",";
    csv += isinf ( _magDelta ) ? "," : format ( "%+.2f,", _magDelta );
    csv += isinf ( _sep ) ? "," : format ( "%.1f,", _sep * SSAngle::kArcsecPerRad );
    csv += isinf ( _PA ) ? "," : format ( "%.1f,", _PA * SSAngle::kDegPerRad );
    csv += isinf ( _PAyr ) ? "," : format ( "%.2f,", _PAyr );

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
    
    csv += _varType + ",";
    csv += isinf ( _varMinMag ) ? "," : format ( "%+.2f,", _varMinMag );
    csv += isinf ( _varMaxMag ) ? "," : format ( "%+.2f,", _varMaxMag );
    csv += isinf ( _varPeriod ) ? "," : format ( "%.2f,", _varPeriod );
    csv += isinf ( _varEpoch )  ? "," : format ( "%.2f,", _varEpoch );

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

    csv += isinf ( _majAxis ) ? "," : format ( "%.2f,", _majAxis * SSAngle::kArcminPerRad );
    csv += isinf ( _minAxis ) ? "," : format ( "%.2f,", _minAxis * SSAngle::kArcminPerRad );
    csv += isinf ( _PA ) ? "," : format ( "%.1f,", _PA * SSAngle::kDegPerRad );

    return csv;
}

// Returns CSV string including base star data, double-star data,
// plus names and identifiers. Overrides SSStar::toCSV().

string SSDeepSky::toCSV ( void )
{
    return toCSV1() + toCSVDS() + toCSV2();
}

// Allocates a new SSStar and initializes it from a CSV-formatted string.
// Returns nullptr on error (invalid CSV string, heap allocation failure, etc.)

SSObjectPtr SSStar::fromCSV ( string csv )
{
    // split string into comma-delimited fields,
    // remove leading & trailing whitespace from each field.
    
    vector<string> fields = split ( csv, "," );
    for ( int i = 0; i < fields.size(); i++ )
        fields[i] = trim ( fields[i] );
    
    SSObjectType type = SSObject::codeToType ( fields[0] );
    if ( type < kTypeStar || type > kTypeGalaxy )
        return nullptr;
    
    // Set expected field index for first identifier based on object type.
    // Verify that we have the required number if fiels and return if not.
    
    int fid = 0;
    if ( type == kTypeStar )
        fid = 10;
    else if ( type == kTypeDoubleStar )
        fid = 15;
    else if ( type == kTypeVariableStar )
        fid = 15;
    else if ( type == kTypeDoubleVariableStar )
        fid = 20;
    else
        fid = 13;
    
    if ( fields.size() < fid )
        return nullptr;
    
    SSHourMinSec ra ( fields[1] );
    SSDegMinSec dec ( fields[2] );
    
    double pmRA = fields[3].empty() ? HUGE_VAL : SSAngle::kRadPerArcsec * strtofloat64 ( fields[3] ) * 15.0;
    double pmDec = fields[4].empty() ? HUGE_VAL : SSAngle::kRadPerArcsec * strtofloat64 ( fields[4] );
    
    float vmag = fields[5].empty() ? HUGE_VAL : strtofloat ( fields[5] );
    float bmag = fields[6].empty() ? HUGE_VAL : strtofloat ( fields[6] );
    
    float dist = fields[7].empty() ? HUGE_VAL : strtofloat ( fields[7] ) * SSCoordinates::kLYPerParsec;
    float radvel = fields[8].empty() ? HUGE_VAL : strtofloat ( fields[8] ) / SSCoordinates::kLightKmPerSec;
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
        float dmag = fields[11].empty() ? HUGE_VAL : strtofloat ( fields[11] );
        float sep = fields[12].empty() ? HUGE_VAL : strtofloat ( fields[12] ) / SSAngle::kArcsecPerRad;
        float pa = fields[13].empty() ? HUGE_VAL : strtofloat ( fields[13] ) / SSAngle::kDegPerRad;
        float year = fields[14].empty() ? HUGE_VAL : strtofloat ( fields[14] );

        pDoubleStar->setComponents ( comps );
        pDoubleStar->setMagnitudeDelta( dmag );
        pDoubleStar->setSeparation ( sep );
        pDoubleStar->setPositionAngle ( pa );
        pDoubleStar->setPositionAngleYear ( year );
    }

    if ( pVariableStar )
    {
        int fv = ( type == kTypeVariableStar ) ? 10 : 15;
            
        string vtype = fields[fv];
        float vmin = fields[fv+1].empty() ? HUGE_VAL : strtofloat ( fields[fv+1] );
        float vmax = fields[fv+2].empty() ? HUGE_VAL : strtofloat ( fields[fv+2] );
        float vper = fields[fv+3].empty() ? HUGE_VAL : strtofloat ( fields[fv+3] );
        double vep = fields[fv+4].empty() ? HUGE_VAL : strtofloat64 ( fields[fv+4] );
        
        pVariableStar->setVariableType ( vtype );
        pVariableStar->setMaximumMagnitude ( vmax );
        pVariableStar->setMinimumMagnitude ( vmin );
        pVariableStar->setPeriod ( vper );
        pVariableStar->setEpoch ( vep );
    }
    
    if ( pDeepSkyObject )
    {
        float major = fields[10].empty() ? HUGE_VAL : strtofloat ( fields[10] ) / SSAngle::kArcminPerRad;
        float minor = fields[11].empty() ? HUGE_VAL : strtofloat ( fields[11] ) / SSAngle::kArcminPerRad;
        float pa = fields[12].empty() ? HUGE_VAL : strtofloat ( fields[12] ) / SSAngle::kDegPerRad;
        
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
