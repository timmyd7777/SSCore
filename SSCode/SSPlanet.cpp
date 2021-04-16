// SSPlanet.cpp
// SSCore
//
// Created by Tim DeBenedictis on 3/15/20.
// Copyright © 2020 Southern Stars. All rights reserved.

#include <mutex>

#include "SSPlanet.hpp"
#include "SSPSEphemeris.hpp"
#include "SSJPLDEphemeris.hpp"
#include "SSMoonEphemeris.hpp"
#include "SSTLE.hpp"

// This uses the 1979 Van Flandern - Pulkinnen low-precision planetary ephemeris when JPL DE is unavailable.
// After investigation, Paul Schlyter's formulae seem more accurate (esp. for Pluto and the Moon) and are
// much simpler/faster, so there's no need for the original VP version.  But it's here for safekeeping!

#ifndef USE_VPEPHEMERIS
#define USE_VPEPHEMERIS 0
#endif
#if USE_VPEPHEMERIS
#include "SSVPEphemeris.hpp"
#endif

// This uses the VSOP013 planetary and ELPMPP02 ephemeris when JPL DE is unavailable.
// These provide sub-arcsecond accuracy over a timespan from years -4000 to +8000,
// but are much slower than Paul Schlyter's formulae.

#ifndef USE_VSOP_ELP
#define USE_VSOP_ELP 1
#endif
#if USE_VSOP_ELP
#include "VSOP2013.hpp"
#include "ELPMPP02.hpp"
static bool _useVSOPELP = true;
static VSOP2013 _vsop;
static ELPMPP02 _elp;
#endif

SSPlanet::SSPlanet ( SSObjectType type ) : SSObject ( type )
{
    _id = SSIdentifier();
    _orbit = SSOrbit();
    _Hmag = _Gmag = INFINITY;
    _radius = 0.0;
    _mass = 0.0;
    _position = _velocity = SSVector ( INFINITY, INFINITY, INFINITY );
}

SSPlanet::SSPlanet ( SSObjectType type, SSPlanetID id ) : SSPlanet ( type )
{
    _id = SSIdentifier ( kCatJPLanet, id );
}

// Overrides SSObject::getIdentifier ( SSCatalog cat )

SSIdentifier SSPlanet::getIdentifier ( SSCatalog cat )
{
    if ( cat == kCatUnknown )
        return _id;
    else if ( _id.catalog() == cat )
        return _id;
    else
        return SSIdentifier ( kCatUnknown, 0 );
}

// Computes solar system object's heliocentric position and velocity vectors in AU and AU/day.
// Current time (jed) is Julian Ephemeris Date in dynamic time (TDT), not civil time (UTC).
// Light travel time to object (lt) is in days; may be zero for first approximation.
// Returned position (pos) and velocity (vel) vectors are both in fundamental J2000 equatorial frame.

void SSPlanet::computePositionVelocity ( double jed, double lt, SSVector &pos, SSVector &vel )
{
    if ( _type == kTypePlanet )
        computeMajorPlanetPositionVelocity ( (int) _id.identifier(), jed, lt, pos, vel );
    else if ( _type == kTypeMoon )
        computeMoonPositionVelocity ( jed, lt, pos, vel );
    else if ( _type == kTypeAsteroid || _type == kTypeComet )
        computeMinorPlanetPositionVelocity ( jed, lt, pos, vel );
    else if ( _type == kTypeSatellite )
    {
        SSSatellite *pSat = dynamic_cast<SSSatellite *> ( this );
        if ( pSat )
            pSat->computePositionVelocity ( jed, lt, pos, vel );
    }
}

// Wrapper for above which computes solar system object's position/velocity at the JED in the
// given SSCoordinates object without light time.  Overrides SSObject::computePositionVelocity().

void SSPlanet::computePositionVelocity  ( SSCoordinates &coords, SSVector &pos, SSVector &vel )
{
    computePositionVelocity ( coords.getJED(), 0.0, pos, vel );
}

// Computes major planet's heliocentric position and velocity vectors in AU and AU/day.
// Current time (jed) is Julian Ephemeris Date in dynamic time (TDT), not civil time (UTC).
// Light travel time to planet (lt) is in days; may be zero for first approximation.
// Returned position (pos) and velocity (vel) vectors are both in fundamental J2000 equatorial frame.

void SSPlanet::computeMajorPlanetPositionVelocity ( int id, double jed, double lt, SSVector &pos, SSVector &vel )
{
    // When planets or the Moon are more than 1 light day away, don't use JPL DE 408; VSOP/ELP is much faster in this case.

    if ( lt < 1.0 && SSJPLDEphemeris::compute ( id, jed - lt, false, pos, vel ) )
        return;

    // VSOP2013 is valid from years -4000 to +8000; use PS Ephemeris outside that range.
    
#if USE_VSOP_ELP
    double y = fabs ( jed - lt - SSTime::kJ2000 ) / 365.25;
    if ( _useVSOPELP && y < 6000.0 )
    {
        _vsop.computePositionVelocity ( id, jed - lt, pos, vel );
        
        // ELPMPP02 is valid within 3000 years of J2000; apply Earth-Moon barycenter correction
        // if within that range. If outside it, user is not likely to care about this small correction.
        
        if ( id == kEarth && y < 3000.0 )
        {
            SSVector mpos, mvel;
            _elp.computePositionVelocity ( jed - lt, mpos, mvel );
            pos -= mpos * _elp.kMoonEarthMassRatio;
            vel -= mvel * _elp.kMoonEarthMassRatio;
        }
    }
    else
    {
        computePSPlanetMoonPositionVelocity ( id, jed, lt, pos, vel );
    }
#elif USE_VPEPHEMERIS
    SSVPEphemeris::fundamentalPositionVelocity ( id, jed - lt, pos, vel );
#else
    computePSPlanetMoonPositionVelocity ( id, jed, lt, pos, vel );
#endif
}

void SSPlanet::computePSPlanetMoonPositionVelocity ( int id, double jed, double lt, SSVector &pos, SSVector &vel )
{
    static double orbMatJED = 0.0;
    static SSMatrix orbMat;
    
    if ( jed != orbMatJED )
    {
        SSMatrix eclMat = SSCoordinates::getEclipticMatrix ( SSCoordinates::getObliquity ( jed ) );
        SSMatrix preMat = SSCoordinates::getPrecessionMatrix ( jed ).transpose();
        orbMat = preMat * eclMat;
        orbMatJED = jed;
    }

    SSSpherical ecl;
    
    if ( id == kSun )
        ecl = SSPSEphemeris::sun ( jed - lt, pos, vel );
    if ( id == kMercury )
        ecl = SSPSEphemeris::mercury ( jed - lt, pos, vel );
    else if ( id == kVenus )
        ecl = SSPSEphemeris::venus ( jed - lt, pos, vel );
    else if ( id == kEarth )
        ecl = SSPSEphemeris::earth ( jed - lt, pos, vel );
    else if ( id == kMars )
        ecl = SSPSEphemeris::mars ( jed - lt, pos, vel );
    else if ( id == kJupiter )
        ecl = SSPSEphemeris::jupiter ( jed - lt, pos, vel );
    else if ( id == kSaturn )
        ecl = SSPSEphemeris::saturn ( jed - lt, pos, vel );
    else if ( id == kUranus )
        ecl = SSPSEphemeris::uranus ( jed - lt, pos, vel );
    else if ( id == kNeptune )
        ecl = SSPSEphemeris::neptune ( jed - lt, pos, vel );
    else if ( id == kPluto )
    {
        // use Keplerian orbit to compute Pluto's velocity since PSEphemeris does not.
        // If within 1000 years of J2000, use PS Ephemeris fo compute Pluto's position with better accuraacy.
        
        SSOrbit::getPlutoOrbit ( jed ).toPositionVelocity ( jed - lt, pos, vel );
        double y = fabs ( jed - lt - SSTime::kJ2000 ) / 365.25;
        if ( y < 1000.0 )
            ecl = SSPSEphemeris::pluto ( jed - lt, pos, vel );
    }
    else if ( id == kLuna )
    {
        ecl = SSPSEphemeris::moon ( jed - lt, pos, vel );
        pos *= SSCoordinates::kKmPerEarthRadii / SSCoordinates::kKmPerAU;
        vel *= SSCoordinates::kKmPerEarthRadii / SSCoordinates::kKmPerAU;
    }
    
    pos = orbMat * pos;
    vel = orbMat * vel;
}

// Computes asteroid or comet's heliocentric position and velocity vectors in AU and AU/day.
// Current time (jed) is Julian Ephemeris Date in dynamic time (TDT), not civil time (UTC).
// Light travel time to object (lt) is in days; may be zero for first approximation.
// Returned position (pos) and velocity (vel) vectors are both in fundamental J2000 equatorial frame.

void SSPlanet::computeMinorPlanetPositionVelocity ( double jed, double lt, SSVector &pos, SSVector &vel )
{
    static SSMatrix matrix = SSCoordinates::getEclipticMatrix ( SSCoordinates::getObliquity ( SSTime::kJ2000 ) );
    _orbit.toPositionVelocity ( jed - lt, pos, vel );
    pos = matrix.multiply ( pos );
    vel = matrix.multiply ( vel );
}

// Computes moon's heliocentric position and velocity vectors in AU and AU/day.
// Current time (jed) is Julian Ephemeris Date in dynamic time (TDT), not civil time (UTC).
// Light travel time to moon (lt) is in days; may be zero for first approximation.
// Returned position (pos) and velocity (vel) vectors are both in fundamental J2000 equatorial frame.

mutex moon_mutex;

void SSPlanet::computeMoonPositionVelocity ( double jed, double lt, SSVector &pos, SSVector &vel )
{
    static SSVector primaryPos[10], primaryVel[10];
    static double primaryJED[10] = { 0.0 };

    // Get moon and primary planet identifier.
    
    int m = (int) _id.identifier();
    int p = m / 100;
    if ( p < 0 || p > 9 )
        p = 0;

    // Special case for Moon: use JPL ephemeris to compute heliocentric position and velocity directly;
    // or if that fails, use PS ephemeris to compute Moon's geocentric position and velocity.
    
    if ( _id.identifier() == kLuna )
    {
        // When planets or the Moon are more than 1 light day away, don't use JPL DE 408; VSOP/ELP is much faster in this case.
        
        if ( lt < 1.0 && SSJPLDEphemeris::compute ( 10, jed - lt, false, pos, vel ) )
            return;

        // ELPMPP02 is valid within 3000 years of J2000; use PS Ephemeris if outside that range.

#if USE_VSOP_ELP
        double y = fabs ( jed - lt - SSTime::kJ2000 ) / 365.25;
        if ( _useVSOPELP && y < 3000.0 )
            _elp.computePositionVelocity ( jed - lt, pos, vel );
        else
            computePSPlanetMoonPositionVelocity ( kLuna, jed, lt, pos, vel );
#elif USE_VPEPHEMERIS
        SSVPEphemeris::fundamentalPositionVelocity ( 10, jed - lt, pos, vel );
        pos *= SSCoordinates::kKmPerEarthRadii / SSCoordinates::kKmPerAU;
        vel *= SSCoordinates::kKmPerEarthRadii / SSCoordinates::kKmPerAU;
#else
        computePSPlanetMoonPositionVelocity ( kLuna, jed, lt, pos, vel );
#endif
    }
    else
    {
        // Compute moon's position and velocity relative to its primary planet.
        
        bool result = false;
        
        if ( p == kMars )
            result = SSMoonEphemeris::marsMoonPositionVelocity ( m, jed - lt, pos, vel );
        else if ( p == kJupiter )
            result = SSMoonEphemeris::jupiterMoonPositionVelocity ( m, jed - lt, pos, vel );
        else if ( p == kSaturn )
            result = SSMoonEphemeris::saturnMoonPositionVelocity ( m, jed - lt, pos, vel );
        else if ( p == kUranus )
            result = SSMoonEphemeris::uranusMoonPositionVelocity ( m, jed - lt, pos, vel );
        else if ( p == kNeptune )
            result = SSMoonEphemeris::neptuneMoonPositionVelocity ( m, jed - lt, pos, vel );
        else if ( p == kPluto )
            result = SSMoonEphemeris::plutoMoonPositionVelocity ( m, jed - lt, pos, vel );

        // Fallback: compute moon's position using Keplerian orbit
        
        if ( ! result )
            computeMinorPlanetPositionVelocity ( jed, lt, pos, vel );
    }
    
    // If JED has changed since last time we computed primary's position and velocity, recompute them.
    // Add primary's position (antedated for light time) and velocity to moon's position and velocity.
    // If light time is less than 1 day, assume primary's velocity is constant over light time duration.
    // Mutex lock prevents multiple threads from modifying these shared resources simultaneously.
    
    moon_mutex.lock();
    if ( lt < 1.0 )
    {
        if ( primaryJED[p] != jed )
        {
            computeMajorPlanetPositionVelocity ( p, jed, 0.0, primaryPos[p], primaryVel[p] );
            primaryJED[p] = jed;
        }
        pos += primaryPos[p] - primaryVel[p] * lt;
        vel += primaryVel[p];
    }
    else
    {
        if ( primaryJED[p] != ( jed - lt ) )
        {
            computeMajorPlanetPositionVelocity ( p, jed, lt, primaryPos[p], primaryVel[p] );
            primaryJED[p] = jed - lt;
        }
        pos += primaryPos[p];
        vel += primaryVel[p];
    }
    moon_mutex.unlock();
}

// Given a point at planetographic longituade (lon) and latitude (lat) in radians,
// on the surface of this solar system object, computes apparent direction
// unit vector (dir) and distance in AU (dist) to that point, seen from the
// observer's current position.
// Returns true if the point is on the visible part of the planet's surface;
// returns false if the point is "over the horizon" on the far side the planet.
// Assumes planet's apparent direction and distance from observer have already been calculated!

bool SSPlanet::surfacePointDirection ( SSAngle lon, SSAngle lat, SSVector &dir, double &dist )
{
    SSVector point = SSSpherical ( lon, lat, getRadius() / SSCoordinates::kKmPerAU );
    point.z *= 1.0 - flattening();
    point = getPlanetographicMatrix() * point;
    dir = ( point + getDirection() * getDistance() ).normalize ( dist );
    return dir * point < 0.0;
}

// Returns distance to solar system object's limb (i.e. horizon) in arbitrary units.
// Object's physical radius (radius) and distance to object center (distance)
// must be in the same units - km, AU, etc.  Assumes object is spherical!

double SSPlanet::horizonDistance ( double radius, double distance )
{
    return distance > radius ? sqrt ( distance * distance - radius * radius ) : 0.0;
}

// Returns distance to this solar system object's limb (i.e. horizon) in AU.
// Object's distance must already be calculated!

double SSPlanet::horizonDistance ( void )
{
    return horizonDistance ( _radius / SSCoordinates::kKmPerAU, _distance );
}

// Returns solar system object's angular radius in radians.
// Object's physical radius (radius) and distance to object center (distance)
// must be in the same units - km, AU, etc.

double SSPlanet::angularRadius ( double radius, double distance )
{
    return distance >= radius ? asin ( radius / distance ) : SSAngle::kPi;
}

// Returns this solar system object's angular radius in radians,
// multiplied by an optional scale factor (s).
// Object's distance must already be calculated!

double SSPlanet::angularRadius ( float s )
{
    return angularRadius ( _radius * s, _distance * SSCoordinates::kKmPerAU );
}

// Returns solar system object's phase angle in radians.
// Object's heliocentric position vector (position) is in AU, but units don't matter.
// Object's apparent direction seen from observer (direction) must be a unit vector.

double SSPlanet::phaseAngle ( SSVector position, SSVector direction )
{
    double sundist = position.magnitude();
    return sundist > 0.0 ? acos ( ( position * direction ) / sundist ) : 0.0;
}

// Returns this solar system object's phase angle in radians.
// Object's heliocentric position and apparent direction vectors must already be calculated!

double SSPlanet::phaseAngle ( void )
{
    return phaseAngle ( _position, _direction );
}

// Returns solar system object's illuminated fraction (from 0.0 to 1.0)
// given its phase angle in radians (phase).

double SSPlanet::illumination ( double phase )
{
    return ( 1.0 + cos ( phase ) ) / 2.0;
}

// Returns this solar system object's illuminated fraction (from 0.0 to 1.0)
// Object's heliocentric position and apparent direction vectors must already be calculated!

double SSPlanet::illumination ( void )
{
    return illumination ( phaseAngle() );
}

// Returns this solar system object's elongation (i.e. difference in ecliptic longitude)
// from the Sun, in radians, reduced to the range -pi to +pi.
// Object's and Sun's apparent direction vectors must already be calculated!

double SSPlanet::elongation ( SSPlanetPtr pSun )
{
    static SSMatrix m = SSCoordinates::getEclipticMatrix ( SSCoordinates::getObliquity ( SSTime::kJ2000 ) ).transpose();
    
    SSSpherical suncoords = m * pSun->getDirection();
    SSSpherical coords = m * getDirection();
    
    return modpi ( coords.lon - suncoords.lon );
}

// Computes solar system object visual magnitude.
// Object's distance from sun (rad) and from observer (dist) are both in AU.
// Object's phase angle (phase) is in radians.
// Object's heliocentric position and apparent direction vectors must already be calculated!
// Formulae for major planets from Jean Meeus, "Astronomical Algorithms", pp. 269-270.

float SSPlanet::computeMagnitude ( double rad, double dist, double phase )
{
    int id = (int) _id.identifier();
    double b = radtodeg ( phase ), b2 = b * b, b3 = b2 * b;
    float mag = INFINITY;
    
    if ( _type == kTypePlanet )
    {
        if ( id == kSun )
            mag = -26.72 + 5.0 * log10 ( dist );
        else if ( id == kMercury )
            mag = -0.42 + 5.0 * log10 ( rad * dist ) + 0.0380 * b - 0.000273 * b2 + 0.000002 * b3;
        else if ( id == kVenus )
            mag = -4.40 + 5.0 * log10 ( rad * dist ) + 0.0009 * b + 0.000239 * b2 - 0.00000065 * b3;
        else if ( id == kEarth )
            mag = -3.86 + 5.0 * log10 ( rad * dist );
        else if ( id == kMars )
            mag = -1.52 + 5.0 * log10 ( rad * dist ) + 0.016 * b;
        else if ( id == kJupiter )
            mag = -9.40 + 5.0 * log10 ( rad * dist ) + 0.005 * b;
        else if ( id == kSaturn )
        {
            // Compute Saturn's ring plane inclination in radians from dot product of its apparent direction vector
            // and Saturn's north pole direction vector (both unit vectors in J2000 equatorial frame).
            
            static SSVector pole ( SSSpherical ( degtorad ( 40.589 ), degtorad ( 83.537 ) ) );
            double rinc = M_PI_2 - acos ( _direction * pole );
            mag = -8.88 + 5.0 * log10 ( rad * dist ) + 0.044 * b - 2.60 * fabs ( rinc ) + 1.25 * rinc * rinc;
        }
        else if ( id == kUranus )
            mag = -7.19 + 5.0 * log10 ( rad * dist ) + 0.0028 * b;
        else if ( id == kNeptune )
            mag = -6.87 + 5.0 * log10 ( rad * dist );
        else if ( id == kPluto )
            mag = -1.01 + 5.0 * log10 ( rad * dist ) + 0.041 * b;
    }
    else if ( _type == kTypeMoon )
    {
        if ( id == kLuna )
            mag = computeAsteroidMagnitude ( rad, dist, phase, 0.21, 0.25 );
        else
            mag = computeAsteroidMagnitude ( rad, dist, phase, _Hmag, isinf ( _Gmag ) ? 0.15 : _Gmag );
    }
    else if ( _type == kTypeAsteroid )
        mag = computeAsteroidMagnitude ( rad, dist, phase, _Hmag, _Gmag );
    else if ( _type == kTypeComet )
        mag = computeCometMagnitude ( rad, dist, _Hmag, _Gmag );
    else if ( _type == kTypeSatellite )
        mag = SSSatellite::computeSatelliteMagnitude ( dist * SSCoordinates::kKmPerAU, phase, _Hmag );
    
    return mag;
}

// Computes asteroid visual magnitude.
// Asteroid's distance from sun (rad) and from observer (dist) are both in AU.
// Asteroid's phase angle (phase) is in radians.
// Asteroid's absolute magnitude (h) is visual magnitude at 1 AU from Earth and Sun, 100% illumination.
// Asteroid's magnitude parameter (g) describes how it darkens as illumination decreases.
// Formula from Jean Meeus, "Astronomical Algorithms", p. 217.

float SSPlanet::computeAsteroidMagnitude ( double rad, double dist, double phase, double h, double g )
{
    double phi1 = exp ( -3.33 * pow ( tan ( phase / 2.0 ), 0.63 ) );
    double phi2 = exp ( -1.87 * pow ( tan ( phase / 2.0 ), 1.22 ) );
    float mag = ( 1.0 - g ) * phi1 + g * phi2;
    return mag > 0.0 ? h + 5.0 * log10 ( rad * dist ) - 2.5 * log10 ( mag ) : INFINITY;
}

// Computes comet visual magnitude.
// Comet's distance from sun (rad) and from observer (dist) are both in AU.
// Comet's absolute magnitude (h) is visual magnitude at 1 AU from Earth and Sun.
// Comet's magnitude parameter (k) defines how it darkens as distance from Sun increases.
// Formula from Jean Meeus, "Astronomical Algorithms", p. 216.

float SSPlanet::computeCometMagnitude ( double rad, double dist, double h, double k )
{
    return h + 5.0 * log10 ( dist ) + 2.5 * k * log10 ( rad );
}

// Computes this solar system object's position, direction, distance, and magnitude.
// The current Julian Ephemeris Date and observer position are input in the SSCoordinates object (coords).

void SSPlanet::computeEphemeris ( SSCoordinates &coords )
{
    // Compute planet's heliocentric position and velocity at current JED.
    // Compute distance and light time to planet.
    
    double lt = 0.0;
    double jed = coords.getJED();
    computePositionVelocity ( jed, lt, _position, _velocity );

    // If desired, recompute planet's position and velocity antedated for light time.
    // In theory we should iterate but in practice this gets us sub-arcsecond precision!
    
    if ( coords.getLightTime() )
    {
        lt = ( _position - coords.getObserverPosition() ).magnitude() / coords.kLightAUPerDay;
        computePositionVelocity ( jed, lt, _position, _velocity );
    }

    // We may fail to compute satellite position if TLE is significantly out of date.
    // If this happens, set direction/distance/magnitude to infinity to indicate invalid result.
    
    if ( _position.isnan() || isnan ( _distance ) )
    {
        _direction = SSVector ( INFINITY, INFINITY, INFINITY );
        _distance = _magnitude = INFINITY;
    }
    else
    {
        // Compute apparent direction vector and distance to planet from observer's position.
        // If desired, apply aberration of light.
        // Compute planet's phase angle and visual magnitude.

        _direction = coords.apparentDirection ( _position, _distance );
        double beta = phaseAngle();
        _magnitude = computeMagnitude ( _position.magnitude(), _distance, beta );
    }
    
    // Compute planetographic-to-fundamental transformation matrix.
    // For satellites, this has already been done in computePositionVelocity().
    
    if ( _type != kTypeSatellite )
        _pmatrix = setPlanetographicMatrix ( jed - lt );
}

// Returns this solar system object's apparent motion in the specified
// coordinate system (frame) as seen from the observer time and location
// that is stored in the provided SSCoordinates object (coords).
// Assumes planet's current heliocentric position and velocity have already
// been computed via computeEphemeris(). The apparent motion in RA (motion.lon)
// and Dec (motion.lat) are both in radians per year. Its radial velocity
// (motion.rad) is in AU per day and will be infinite if unknown.

SSSpherical SSPlanet::computeApparentMotion ( SSCoordinates &coords, SSFrame frame )
{
    SSVector pos = coords.transform ( kFundamental, frame, _position - coords.getObserverPosition() );
    SSVector vel = coords.transform ( kFundamental, frame, _velocity - coords.getObserverVelocity() );
    
    // Add this twist for the Horizon frame because it is rotating, not inertial.
    // See https://ocw.mit.edu/courses/aeronautics-and-astronautics/16-07-dynamics-fall-2009/lecture-notes/MIT16_07F09_Lec08.pdf
    
    if ( frame == kHorizon )
    {
        static SSVector omega ( 0.0, 0.0, SSAngle::kTwoPi * SSTime::kSiderealPerSolarDays );
        vel += coords.transform ( kEquatorial, kHorizon, omega ).crossProduct ( pos );
    }
    
    return pos.toSphericalVelocity ( vel );
}

// Downcasts generic SSObject pointer to SSPlanet pointer.
// Returns nullptr if input pointer is not an instance of SSPlanet!

SSPlanetPtr SSGetPlanetPtr ( SSObjectPtr ptr )
{
    return dynamic_cast<SSPlanet *> ( ptr );
}

// Downcasts generic SSObject pointer to SSSatellite pointer.
// Returns nullptr if input pointer is not an instance of SSPlanet!

SSSatellitePtr SSGetSatellitePtr ( SSObjectPtr ptr )
{
    return dynamic_cast<SSSatellite *> ( ptr );
}

// Returns CSV string from planet data, including identifier and names.

string SSPlanet::toCSV ( void )
{
    string csv = SSObject::typeToCode ( _type ) + ",";
    
    if ( _type == kTypeMoon )
        csv += isinf ( _orbit.q ) ? "," : format ( "%.0f,", _orbit.q * SSCoordinates::kKmPerAU );
    else
        csv += isinf ( _orbit.q ) ? "," : format ( "%.8f,", _orbit.q );

    csv += isinf ( _orbit.e ) ? "," : format ( "%.8f,", _orbit.e );
    csv += isinf ( _orbit.i ) ? "," : format ( "%.8f,", _orbit.i * SSAngle::kDegPerRad );
    csv += isinf ( _orbit.w ) ? "," : format ( "%.8f,", _orbit.w * SSAngle::kDegPerRad );
    csv += isinf ( _orbit.n ) ? "," : format ( "%.8f,", _orbit.n * SSAngle::kDegPerRad );
    csv += isinf ( _orbit.m ) ? "," : format ( "%.8f,", _orbit.m * SSAngle::kDegPerRad );
    csv += isinf ( _orbit.mm ) ? "," : format ( "%.8f,", _orbit.mm * SSAngle::kDegPerRad );
    csv += isinf ( _orbit.t ) ? "," : format ( "%.4f,", _orbit.t );
    
    csv += isinf ( _Hmag ) ? "," : format ( "%+.2f,", _Hmag );
    csv += isinf ( _Gmag ) ? "," : format ( "%+.2f,", _Gmag );
    csv += isinf ( _radius ) ? "," : format ( "%.1f,", _radius );
    csv += isinf ( _mass ) ? "," : format ( "%.6E,", _mass * SSCoordinates::kKgPerEarthMass );

    csv += _id ? _id.toString() + "," : ",";
        
    for ( int i = 0; i < _names.size(); i++ )
        csv += _names[i] + ",";

    return csv;
}

// Allocates a new SSPlanet and initializes it from a CSV-formatted string.
// Returns nullptr on error (invalid CSV string, heap allocation failure, etc.)

SSObjectPtr SSPlanet::fromCSV ( string csv )
{
    vector<string> fields = split ( csv, "," );
    
    SSObjectType type = SSObject::codeToType ( fields[0] );
    if ( type < kTypePlanet || type > kTypeComet || fields.size() < 15 )
        return nullptr;
    
    SSOrbit orbit;
    
    orbit.q = fields[1].empty() ? INFINITY : strtofloat64 ( fields[1] );
    orbit.e = fields[2].empty() ? INFINITY : strtofloat64 ( fields[2] );
    orbit.i = fields[3].empty() ? INFINITY : strtofloat64 ( fields[3] ) * SSAngle::kRadPerDeg;
    orbit.w = fields[4].empty() ? INFINITY : strtofloat64 ( fields[4] ) * SSAngle::kRadPerDeg;
    orbit.n = fields[5].empty() ? INFINITY : strtofloat64 ( fields[5] ) * SSAngle::kRadPerDeg;
    orbit.m = fields[6].empty() ? INFINITY : strtofloat64 ( fields[6] ) * SSAngle::kRadPerDeg;
    orbit.mm = fields[7].empty() ? INFINITY : strtofloat64 ( fields[7] ) * SSAngle::kRadPerDeg;
    orbit.t = fields[8].empty() ? INFINITY : strtofloat64 ( fields[8] );

    if ( orbit.q > 1000.0 )
        orbit.q /= SSCoordinates::kKmPerAU;
    
    float h = fields[9].empty() ? INFINITY : strtofloat ( fields[9] );
    float g = fields[10].empty() ? INFINITY : strtofloat ( fields[10] );
    float r = fields[11].empty() ? INFINITY : strtofloat ( fields[11] );
    float m = fields[12].empty() ? INFINITY : strtofloat ( fields[12] );

    SSIdentifier ident;
    if ( type == kTypePlanet || type == kTypeMoon )
        ident = SSIdentifier ( kCatJPLanet, strtoint ( fields[13] ) );
    else
        ident = SSIdentifier::fromString ( fields[13] );

    vector<string> names;
    for ( int i = 14; i < fields.size(); i++ )
        names.push_back ( trim ( fields[i] ) );
    
	SSObjectPtr pObject = SSNewObject ( type );
    SSPlanetPtr pPlanet = SSGetPlanetPtr ( pObject );
    if ( pPlanet == nullptr )
        return nullptr;
    
    pPlanet->setOrbit ( orbit );
    pPlanet->setHMagnitude ( h );
    pPlanet->setGMagnitude ( g );
    pPlanet->setRadius ( r );
    pPlanet->setMass ( m / SSCoordinates::kKgPerEarthMass );
    pPlanet->setIdentifier ( ident );
    pPlanet->setNames ( names );

    return pObject;
}

// Constructs a satellite object from an input Two-Line Element descriptor (tle).

SSSatellite::SSSatellite ( SSTLE &tle ) : SSPlanet ( kTypeSatellite )
{
    _tle = tle;

    _names.push_back ( tle.name );
    _names.push_back ( tle.desig );
    
    _id = SSIdentifier ( kCatNORADSat, tle.norad );
    
    _orbit = tle.toOrbit ( 0.0 );
    _orbit.q *= SSCoordinates::kKmPerEarthRadii / SSCoordinates::kKmPerAU;
    _orbit.mm *= SSTime::kMinutesPerDay;
}

// Computes satellite visual magnitude.
// Satellite's distance from observer (dist) is in kilometers.
// Satellite's phase angle (phase) is in radians.
// Standard magnitude is at 1000 km range, and 50% illumination.
// Formula from http://www.prismnet.com/~mmccants/tles/mccdesc.html

float SSSatellite::computeSatelliteMagnitude ( double dist, double phase, double stdmag )
{
    double mag = INFINITY;
    
    if ( phase < M_PI )
        mag = stdmag - 15.75 + 2.5 * log10 ( dist * dist / ( ( 1.0 + cos ( phase ) ) / 2.0 ) );
    
    return mag;
}

// Computes this satellite's magnitude using above formula.
// Satellite's distance from sun (rad) and from observer (dist) are both in AU.
// Satellite's phase angle (phase) is in radians.
// Satellite's heliocentric position and apparent direction vectors must already be calculated!

float SSSatellite::computeMagnitude ( double rad, double dist, double phase )
{
    return computeSatelliteMagnitude ( dist * SSCoordinates::kKmPerAU, phase, _Hmag );
}

// Computes Earth satellite's heliocentric position and velocity vectors in AU and AU/day.
// Current time (jed) is Julian Ephemeris Date in dynamic time (TDT), not civil time (UTC).
// Light travel time to satellite (lt) is in days; may be zero for first approximation.
// Returned position (pos) and velocity (vel) vectors are both in fundamental J2000 equatorial frame.
// Also computes satellite's "planetographic" orientation matrix, which describes how the
// satellite is oriented relative to the Earth's J2000 mean equatorial (fundamental) frame.

mutex sat_mutex;

void SSSatellite::computePositionVelocity ( double jed, double lt, SSVector &pos, SSVector &vel )
{
    static SSVector earthPos, earthVel;
    static SSMatrix earthMat;
    static double earthJED = 0.0, deltaT = 0.0;
    
    // Recompute Earth's position and velocity relative to Sun if JED has changed.
    // Asssume Earth's velocity is constant over light time duration.
    // Sat mutex prevents multiple threads from modifying these shared resources simultaneously.
    
    sat_mutex.lock();
    if ( jed != earthJED )
    {
        computeMajorPlanetPositionVelocity ( kEarth, jed, 0.0, earthPos, earthVel );
        earthJED = jed;
        deltaT = SSTime ( jed ).getDeltaT() / SSTime::kSecondsPerDay;
        earthMat = SSCoordinates::getPrecessionMatrix ( jed ).transpose();
    }
    sat_mutex.unlock();
    
    // Compute satellite position & velocity relative to Earth, antedated for light time.
    // Satellite's orbit epoch is Julian Date, not JED, so subtract Delta T.
    // If within 30 days of the epoch, use the full-precision SGP4/SDP4
    // orbit models to compute the satellite's position.
    // Otherwise compute the satellite's position using its Keplerian elements,
    // updated for secular variation according to the SGP orbit model.
    
    double tsince = ( jed - deltaT - lt - _tle.jdepoch ) * SSTime::kMinutesPerDay;
    if ( fabs ( tsince ) < 30 * SSTime::kMinutesPerDay )
    {
        // Output position and velocity vectors are in km and km/sec; convert to AU and AU/day;
        _tle.toPositionVelocity ( jed - deltaT - lt, pos, vel );
        pos /= SSCoordinates::kKmPerAU;
        vel /= SSCoordinates::kKmPerAU / SSTime::kSecondsPerDay;
    }
    else
    {
        // Output position and velocity vectors are in Earth-radii and Earth-radii/min; convert to AU and AU/day;
        _tle.toOrbit ( tsince ).toPositionVelocity ( jed - deltaT - lt, pos, vel );
        pos *= SSCoordinates::kKmPerEarthRadii / SSCoordinates::kKmPerAU;
        vel *= SSCoordinates::kKmPerEarthRadii / SSCoordinates::kKmPerAU / SSTime::kMinutesPerDay;
    }
    
    if ( pos.isnan() || vel.isnan() )
        return;
    
    // Satellite orbit elements are referred to current equator, not J2000 equator,
    // so transform output position and velocity from current to J2000 equatorial frame.
  
    pos = earthMat * pos;
    vel = earthMat * vel;
    
    // Set up orientation matrix so satellite's +Z axis points in orbit plane away from Earth's center,
    // +Y axis is perpendicular to orbit plane, +X axis points in orbit plane in direction of motion.
    
    SSVector zaxis = pos.normalize();
    SSVector yaxis = vel.crossProduct ( pos ).normalize();
    SSVector xaxis = zaxis.crossProduct ( yaxis ).normalize();

    _pmatrix.m00 = xaxis.x;
    _pmatrix.m10 = xaxis.y;
    _pmatrix.m20 = xaxis.z;
    
    _pmatrix.m01 = yaxis.x;
    _pmatrix.m11 = yaxis.y;
    _pmatrix.m21 = yaxis.z;

    _pmatrix.m02 = zaxis.x;
    _pmatrix.m12 = zaxis.y;
    _pmatrix.m22 = zaxis.z;
    
    // Add Earth's position (antedated for light time) and velocity to satellite position and velocity.
    
    pos += earthPos - earthVel * lt;
    vel += earthVel;
}

// Imports satellites from TLE-formatted text file (filename).
// Imported satellites are appended to the input vector of SSObjects (satellites).
// Returns number of satellites successfully imported.

int SSImportSatellitesFromTLE ( const string &filename, SSObjectVec &satellites )
{
    // Open file; return on failure.

    FILE *file = fopen ( filename.c_str(), "r" );
    if ( ! file )
        return 0;

    // Read file line-by-line until we reach end-of-file

    string line = "";
    int numSats = 0;
    SSTLE tle;
    
    while ( tle.read ( file ) == 0 )
    {
        // Attempt to create solar system object from TLE; if successful add to object vector.
        
        SSSatellite *pSat = new SSSatellite ( tle );
        if ( pSat )
        {
            satellites.push_back ( SSObjectPtr ( pSat ) );
            numSats++;
        }
    }
    
    // Close file. Return number of objects added to object vector.

    fclose ( file );
    return numSats;
}


// Imports a Mike McCants satellite names file, here:
// https://www.prismnet.com/~mmccants/tles/mcnames.zip
// into a map of McName structs indexed by NORAD number.
// Returns number of McNames imported from file.

int SSImportMcNames ( const string &filename, McNameMap &mcnames )
{
    // Open file; return on failure.

    FILE *file = fopen ( filename.c_str(), "r" );
    if ( ! file )
        return 0;

    // Read file line-by-line until we reach end-of-file

    string line = "";
    int nMcNames = 0;
    
    while ( fgetline ( file, line ) )
    {
        McName mcname = { 0, "", 0.0, 0.0, 0.0, 0.0 };
        
        // Attempt to read McName from line.
        
        size_t len = line.length();
        if ( len < 5 )
            continue;
        
        mcname.norad = strtoint ( line.substr ( 0, 5 ) );
        if ( mcname.norad == 0 )
            continue;

        mcname.name = len > 22 ? trim ( line.substr ( 6, 17 ) ) : "";
        mcname.len = len > 26 ? strtofloat ( line.substr ( 22, 4 ) ) : 0.0;
        mcname.wid = len > 31 ? strtofloat ( line.substr ( 27, 4 ) ) : 0.0;
        mcname.dep = len > 36 ? strtofloat ( line.substr ( 32, 4 ) ) : 0.0;
        mcname.mag = len > 41 ? strtofloat ( line.substr ( 37, 4 ) ) : 0.0;
        
        if ( mcname.mag == 0.0 )
            mcname.mag = INFINITY;
        
        mcnames.insert ( { mcname.norad, mcname } );
        nMcNames++;
    }
    
    // Close file. Return number of objects added to object vector.

    fclose ( file );
    return nMcNames;
}

// Imports satellite standard magnitudes and sizes from Mike McCants satellite
// names file, found here: https://www.prismnet.com/~mmccants/tles/mcnames.zip
// Magnitudes and sizes are inserted into the vector of SSObjects (objects),
// which may contain any solar system objects in addition to satellites.
// Returns number of McCants magnitudes & sizes successfully imported.

int SSImportMcNames ( const string &filename, SSObjectVec &objects )
{
    McNameMap mcnamemap;
    
    // First read the McNames file; return 0 if we fail.
    
    int n = SSImportMcNames ( filename, mcnamemap );
    if ( n == 0 || mcnamemap.size() == 0 )
        return 0;
    
    // For each object in the solar system object vector...
    
    n = 0;
    for ( int i = 0; i < objects.size(); i++ )
    {
        SSObjectPtr pObj = objects[i];
        
        // If the object is not a satellite, continue
        
        SSSatellite *pSat = SSGetSatellitePtr ( pObj );
        if ( pSat == nullptr )
            continue;
        
        // Get satellite's NORAD number. Look for McName record with same number.
        // If we find one, copy McName magnitude and size into satellite.
        
        int norad = pSat->getTLE().norad;
        McName mcname = mcnamemap[norad];
        if ( mcname.norad == norad )
        {
            pSat->setHMagnitude ( mcname.mag );
            pSat->setRadius ( mcname.len / 1000.0 );
            n++;
        }
    }
    
    // Return total number of McName records matched.
    
    return n;
}

// Imports a Mineo Wakita amateur radio satellite frequency data file:
// http://www.ne.jp/asahi/hamradio/je9pel/satslist.csv
// into a map of vectors of SatFreqData structs indexed by NORAD number.
// Returns number of SatFreqData records imported from file.

int SSImportSatelliteFrequencyData ( const string &filename, SatFreqMap &freqmap )
{
    // Open file; return on failure.

    ifstream file ( filename );
    if ( ! file )
        return 0;

    // Read file line-by-line until we reach end-of-file

    string line = "";
    int nFreqs = 0;
    vector<SatFreqData> freqvec;

    while ( getline ( file, line ) )
    {
        vector<string> fields = split ( line, ";" );
        if ( fields.size() < 8 )
            continue;
        
        SatFreqData freq = { 0, "", "", "", "", "", "", "" };
        
        freq.name = fields[0];
        freq.norad = strtoint ( fields[1] );
        freq.uplink = fields[2];
        freq.downlink = fields[3];
        freq.beacon = fields[4];
        freq.mode = fields[5];
        freq.callsign = fields[6];
        freq.status = fields[7];
        
        if ( freq.norad < 1 )
            continue;
        
        nFreqs++;

        if ( freqvec.size() > 0 )
        {
            if ( freq.norad != freqvec[0].norad )
            {
                freqmap.insert ( { freqvec[0].norad, freqvec } );
                freqvec.clear();
            }
        }

        freqvec.push_back ( freq );
    }
    
    // Return number of frequencies read from file.  File will close automatically.

    return nFreqs;
}

// Imports satellite amateur radio frequency data into a vector of SSObjects (objects),
// which may contain any solar system objects in addition to satellites.
// Returns number of satellite radio frequencies successfully imported.

int SSImportSatelliteFrequencyData ( const string &filename, SSObjectVec &objects )
{
    SatFreqMap freqmap;
    
    // First read the satellite amateur radio frequency file; return 0 if we fail.
    
    int n = SSImportSatelliteFrequencyData ( filename, freqmap );
    if ( n == 0 || freqmap.size() == 0 )
        return 0;
    
    // For each object in the solar system object vector...
    
    n = 0;
    for ( int i = 0; i < objects.size(); i++ )
    {
        SSObjectPtr pObj = objects[i];

        // If the object is not a satellite, continue
        
        SSSatellite *pSat = SSGetSatellitePtr ( pObj );
        if ( pSat == nullptr )
            continue;
        
        // Get satellite's NORAD number. Look for satellite frequency vector with same number.
        // If we find one, copy satellite frequency vector into satellite.
        
        int norad = pSat->getTLE().norad;
        vector<SatFreqData> freqvec = freqmap[norad];
        if ( freqvec.size() > 0 && freqvec[0].norad == norad )
        {
            // TODO: actually store the vector of SatFreqData into pSat!
            
            n += freqvec.size();
        }
    }
    
    // Return total number of satellites matched to amateur radio frequencies.
    
    return n;
}

#if USE_VSOP_ELP

void SSPlanet::useVSOPELP ( bool use )
{
    _useVSOPELP = use;
}

bool SSPlanet::useVSOPELP ( void )
{
    return _useVSOPELP;
}

#else

void SSPlanet::useVSOPELP ( bool use )
{
}

bool SSPlanet::useVSOPELP ( void )
{
    return false;
}

#endif

// Calculates planet's rotational elements at the specified Julian Ephemeris Date (jed).
// Returns J2000 right ascension (a0) and declination (d0) of planet's north pole in radians;
// argument of planet's prime meridian (w) and rotation rate (wd) in radians and rad/day.
// Formulae from Report of the IAU Working Group on Cartographic Coordinates and Rotational Elements:
// 2015 https://astrogeology.usgs.gov/search/map/Docs/WGCCRE/WGCCRE2015reprint
// 2009 https://astrogeology.usgs.gov/search/map/Docs/WGCCRE/WGCCRE2009reprint (for Earth and Moon)
// Small periodic terms with amplitudes less than 0.001 degree omitted for Mercury, Mars, Jupiter.
// Rotation rates are System II for Jupiter and System III for Saturn.

void SSPlanet::rotationElements ( double jed, double &a0, double &d0, double &w, double &wd )
{
    int id = (int) _id.identifier();
    double d = jed - 2451545.0;
    double T = d / 36525.0;
    
    if ( _type == kTypePlanet )
    {
        if ( id == kSun )
        {
            a0 = 286.13;
            d0 = 63.87;
            wd = 14.1844000;
            w  = 84.176 + wd * d;
        }
        else if ( id == kMercury )
        {
            double M1 = degtorad ( 174.791086 + 4.092335 * d );
            double M2 = degtorad ( 349.582171 + 8.184670 * d );
            a0 = 281.0097 - 0.0328 * T;
            d0 = 61.4143 - 0.0049 * T;
            wd = 6.1385025;
            w  = 329.5469 + wd * d
                          + 0.00993822 * sin ( M1 )
                          - 0.00104581 * sin ( M2 );
        }
        else if ( id == kVenus )
        {
            a0 = 272.76;
            d0 = 67.16;
            wd = -1.4813688;
            w  = 160.20 + wd * d;
        }
        else if ( id == kEarth )
        {
            a0 = 0.00 - 0.641 * T;
            d0 = 90.00 - 0.557 * T;
            wd = 360.9856235;
            w  = 190.147 + wd * d;
        }
        else if ( id == kMars )
        {
            a0 = 317.269202 - 0.10927547 * T + 0.419057 * sindeg ( 79.398797 + 0.5042615 * T );
            d0 =  54.432516 - 0.05827105 * T + 1.591274 * cosdeg ( 166.325722 + 0.5042615 * T );
            wd = 350.891982443297;
            w  = 176.049863 + wd * d + 0.584542 * sindeg ( 95.391654 + 0.5042615 * T );
        }
        else if ( id == kJupiter )
        {
            a0 = 268.056595 - 0.006499 * T;
            d0 = 64.495303 + 0.002413 * T;
            wd = 870.5360000;
            // w = 67.1 + 877.900 * d; // System I
            w = 43.3 + 870.270 * d; // System II
            // w = 284.95 + wd * d; // System III
        }
        else if ( id == kSaturn )
        {
            a0 = 40.589 - 0.036 * T;
            d0 = 83.537 - 0.004 * T;
            wd = 810.7939024;
            w  = 38.90 + wd * d;
        }
        else if ( id == kUranus )
        {
            a0 = 257.311;
            d0 = -15.175;
            wd = -501.1600928;
            w  = 203.81 + wd * d;
        }
        else if ( id == kNeptune )
        {
            double N = degtorad ( 357.85 + 52.316 * T );
            a0 = 299.36 + 0.70 * sin ( N );
            d0 = 43.46 - 0.51 * cos ( N );
            wd = 541.1397757;
            w  = 249.978 + wd * d - 0.48 * sin ( N );
        }
        else if ( id == kPluto )
        {
            a0 = 132.993;
            d0 = -6.163;
            wd = 56.3625225;
            w  = 302.695 + wd * d;
        }
    }
    else if ( _type == kTypeMoon )
    {
        if ( id == kLuna )
        {
            double E1 = degtorad ( 125.045 - 0.0529921 * d );
            double E2 = degtorad ( 250.089 - 0.1059842 * d );
            double E3 = degtorad ( 260.008 + 13.0120009 * d );
            double E4 = degtorad ( 176.625 + 13.3407154 * d );
            double E5 = degtorad ( 357.529 +  0.9856003 * d );
            double E6 = degtorad ( 311.589 + 26.4057084 * d );
            double E7 = degtorad ( 134.963 + 13.0649930 * d );
            double E8 = degtorad ( 276.617 +  0.3287146 * d );
            double E9 = degtorad (  34.226 +  1.7484877 * d );
            double E10 = degtorad ( 15.134 -  0.1589763 * d );
            double E11 = degtorad ( 119.743 + 0.0036096 * d );
            double E12 = degtorad ( 239.961 + 0.1643573 * d );
            double E13 = degtorad (  25.053 + 12.9590088 * d );
            
            a0 = 269.9949 + 0.0031 * T
                          - 3.8787 * sin ( E1 )
                          - 0.1204 * sin ( E2 )
                          + 0.0700 * sin ( E3 )
                          - 0.0172 * sin ( E4 )
                          + 0.0072 * sin ( E6 )
                          - 0.0052 * sin ( E10 )
                          + 0.0043 * sin ( E13 );
            
            d0 = 66.5392 + 0.0130 * T
                         + 1.5419 * cos ( E1 )
                         + 0.0239 * cos ( E2 )
                         - 0.0278 * cos ( E3 )
                         + 0.0068 * cos ( E4 )
                         - 0.0029 * cos ( E6 )
                         + 0.0009 * cos ( E7 )
                         + 0.0008 * cos ( E10 )
                         - 0.0009 * cos ( E13 );

            wd = 13.17635815;
            
            w = 38.3213 + wd * d - 1.4e-12 * d * d
                        + 3.5610 * sin ( E1 )
                        + 0.1208 * sin ( E2 )
                        - 0.0642 * sin ( E3 )
                        + 0.0158 * sin ( E4 )
                        + 0.0252 * sin ( E5 )
                        - 0.0066 * sin ( E6 )
                        - 0.0047 * sin ( E7 )
                        - 0.0046 * sin ( E8 )
                        + 0.0028 * sin ( E9 )
                        + 0.0052 * sin ( E10 )
                        + 0.0040 * sin ( E11 )
                        + 0.0019 * sin ( E12 )
                        - 0.0044 * sin ( E13 );
        }
        else if ( id == kPhobos )
        {
            double M1 = degtorad ( 190.72646643 + 15917.10818695 * T );
            double M2 = degtorad (  21.46892470 + 31834.27934054 * T );
            double M3 = degtorad ( 332.86082793 + 19139.89694742 * T );
            double M4 = degtorad ( 394.93256437 + 38280.79631835 * T );
            double M5 = degtorad ( 189.63271560 + 41215158.18420050 * T + 12.71192322 * T * T );
            
            a0 = 317.67071657 - 0.10844326 * T
                              - 1.78428399 * sin ( M1 ) + 0.02212824 * sin ( M2 )
                              - 0.01028251 * sin ( M3 ) - 0.00475595 * sin ( M4 );
            
            d0 = 52.88627266 - 0.06134706 * T
                             - 1.07516537 * cos ( M1 ) + 0.00668626 * cos ( M2 )
                             - 0.00648740 * cos ( M3 ) + 0.00281576 * cos ( M4 );
            
            wd = 1128.84475928;
            
             w = 34.9964842535 + wd * d + 12.72192797 * T * T
                               + 1.42421769 * sin ( M1 ) - 0.02273783 * sin ( M2 )
                               + 0.00410711 * sin ( M3 ) + 0.00631964 * sin ( M4 )
                               + 1.143 * sin ( M5 );
        }
        else if ( id == kDeimos )
        {
            double M6 = degtorad ( 121.46893664 + 660.22803474 * T );
            double M7 = degtorad ( 231.05028581 + 660.99123540 * T );
            double M8 = degtorad ( 251.37314025 + 1320.50145245 * T );
            double M9 = degtorad ( 217.98635955 + 38279.96125550 * T );
            double M10 = degtorad ( 196.19729402 + 19139.83628608 * T );
            
            a0 = 316.65705808 - 0.10518014 * T
                              + 3.09217726 * sin ( M6 ) + 0.22980637 * sin ( M7 )
                              + 0.06418655 * sin ( M8 ) + 0.02533537 * sin ( M9 )
                              + 0.00778695 * sin ( M10 );
            
            d0 = 53.50992033 - 0.05979094 * T
                             + 1.83936004 * cos ( M6 ) + 0.14325320 * cos ( M7 )
                             + 0.01911409 * cos ( M8 ) - 0.01482590 * cos ( M9 )
                             + 0.00192430 * cos ( M10 );
            
            wd = 285.16188899;
            
             w = 79.39932954 + wd * d
                             - 2.73954829 * sin ( M6 ) - 0.39968606 * sin ( M7 )
                             - 0.06563259 * sin ( M8 ) - 0.02912940 * sin ( M9 )
                             + 0.01699160 * sin ( M10 );
        }
        else if ( id == kIo )
        {
            double J3 = degtorad ( 283.90 + 4850.7 * T );
            double J4 = degtorad ( 355.80 + 1191.3 * T );
            
            a0 = 268.05 - 0.009 * T + 0.094 * sin ( J3 ) + 0.024 * sin ( J4 );
            d0 =  64.50 + 0.003 * T + 0.040 * cos ( J3 ) + 0.011 * cos ( J4 );
            wd = 203.4889538;
            w  = 200.39 + wd * d - 0.085 * sin ( J3 ) - 0.022 * sin ( J4 );
        }
        else if ( id == kEuropa )
        {
            double J4 = degtorad ( 355.80 + 1191.3 * T );
            double J5 = degtorad ( 119.90 + 262.1 * T );
            double J6 = degtorad ( 229.80 + 64.3 * T );
            double J7 = degtorad ( 352.25 + 2382.6 * T );
            
            a0 = 268.08 - 0.009 * T + 1.086 * sin ( J4 ) + 0.060 * sin ( J5 ) + 0.015 * sin ( J6 ) + 0.009 * sin ( J7 );
            d0 =  64.51 + 0.003 * T + 0.468 * cos ( J4 ) + 0.026 * cos ( J5 ) + 0.007 * cos ( J6 ) + 0.002 * cos ( J7 );
            wd = 101.3747235;
            w  =  36.022 + wd * d - 0.980 * sin ( J4 ) - 0.054 * sin ( J5 ) - 0.014 * sin ( J6 ) - 0.008 * sin ( J7 );
        }
        else if ( id == kGanymede )
        {
            double J4 = degtorad ( 355.80 + 1191.3 * T );
            double J5 = degtorad ( 119.90 + 262.1 * T );
            double J6 = degtorad ( 229.80 + 64.3 * T );

            a0 = 268.20 - 0.009 * T - 0.037 * sin ( J4 ) + 0.431 * sin ( J5 ) + 0.091 * sin ( J6 );
            d0 =  64.57 + 0.003 * T - 0.016 * cos ( J4 )  +0.186 * cos ( J5 ) + 0.039 * cos ( J6 );
            wd =  50.3176081;
            w  =  44.064 + wd * d + 0.033 * sin ( J4 ) - 0.389 * sin ( J5 ) - 0.082 * sin ( J6 );
        }
        else if ( id == kCallisto )
        {
            double J5 = degtorad ( 119.90 + 262.1 * T );
            double J6 = degtorad ( 229.80 + 64.3 * T );
            double J8 = degtorad ( 113.35 + 6070.0 * T );

            a0 = 268.72 - 0.009 * T - 0.068 * sin ( J5 ) + 0.590 * sin ( J6 ) + 0.010 * sin ( J8 );
            d0 =  64.83 + 0.003 * T - 0.029 * cos ( J5 ) + 0.254 * cos ( J6 ) - 0.004 * cos ( J8 );
            wd =  21.5710715;
            w  = 259.51 + wd * d + 0.061 * sin ( J5 ) - 0.533 * sin ( J6 ) - 0.009 * sin ( J8 );
        }
        else if ( id == kMimas )
        {
            double S3 = degtorad ( 177.40 - 36505.5 * T );
            double S5 = degtorad ( 316.45 + 506.2 * T );
            a0 = 40.66 - 0.036 * T + 13.56 * sin ( S3 );
            d0 = 83.52 - 0.004 * T - 1.53 * cos ( S3 );
            wd = 381.9945550;
            w  = 333.46 + wd * d - 13.48 * sin ( S3 ) - 44.85 * sin ( S5 );
        }
        else if ( id == kEnceladus )
        {
            a0 =  40.66 - 0.036 * T;
            d0 =  83.52 - 0.004 * T;
            wd = 262.7318996;
            w  =   6.32 + wd * d;
        }
        else if ( id == kTethys )
        {
            double S4 = degtorad ( 300.00 - 7225.9 * T );
            double S5 = degtorad ( 316.45 + 506.2 * T );
            a0 = 40.66 - 0.036 * T + 9.66 * sin ( S4 );
            d0 = 83.52 - 0.004 * T - 1.09 * cos ( S4 );
            wd = 190.6979085;
            w  =   8.95 + wd * d - 9.60 * sin ( S4 ) + 2.23 * sin ( S5 );
        }
        else if ( id == kDione )
        {
            a0 = 40.66 - 0.036 * T;
            d0 = 83.52 - 0.004 * T;
            wd = 131.5349316;
            w  = 357.6 + wd * d;
        }
        else if ( id == kRhea )
        {
            double S6 = degtorad ( 345.20 - 1016.3 * T );
            a0 =  40.38 - 0.036 * T + 3.10 * sin ( S6 );
            d0 =  83.55 - 0.004 * T - 0.35 * cos ( S6 );
            wd =  79.6900478;
            w  = 235.16 + wd * d - 3.08 * sin ( S6 );
        }
        else if ( id == kTitan )
        {
            a0 = 39.4827;
            d0 = 83.4279;
            wd = 22.5769768;
            w  = 186.5855 + wd * d;
        }
        else if ( id == kIapetus )
        {
            a0 = 318.16 - 3.949 * T;
            d0 =  75.03 - 1.143 * T;
            wd =   4.5379572;
            w  = 355.2 + wd * d;
        }
        else if ( id == kPhoebe )
        {
            a0 = 356.90;
            d0 =  77.80;
            wd = 931.639;
            w  = 178.58 + wd * d;
        }
        else if ( id == kMiranda )
        {
            double U11 = degtorad ( 102.23 - 2024.22 * T );
            double U12 = degtorad ( 316.41 + 2863.96 * T );
            a0 = 257.43 + 4.41 * sin ( U11 ) - 0.04 * sin ( 2 * U11 );
            d0 = -15.08 + 4.25 * cos ( U11 ) - 0.02 * cos ( 2 * U11 );
            wd = -254.6906892;
            w  = 30.70 + wd * d - 1.27 * sin ( U12 ) + 0.15 * sin ( 2 * U12 )
                                + 1.15 * sin ( U11 ) - 0.09 * sin ( 2 * U11 );
        }
        else if ( id == kAriel )
        {
            double U12 = degtorad ( 316.41 + 2863.96 * T );
            double U13 = degtorad ( 304.01 - 51.94 * T );
            a0 = 257.43 + 0.29 * sin ( U13 );
            d0 = -15.10 + 0.28 * cos ( U13 );
            wd = -142.8356681;
            w  = 156.22 + wd * d + 0.05 * sin ( U12 ) + 0.08 * sin ( U13 );
        }
        else if ( id == kUmbriel )
        {
            double U12 = degtorad ( 316.41 + 2863.96 * T );
            double U14 = degtorad ( 308.71 - 93.17 * T );
            a0 = 257.43 + 0.21 * sin ( U14 );
            d0 = -15.10 + 0.2  * cos ( U14 );
            wd = -86.8688923;
            w  = 108.05 + wd * d - 0.09 * sin ( U12 ) + 0.06 * sin ( U14 );
        }
        else if ( id == kTitania )
        {
            double U15 = degtorad ( 340.82 - 75.32 * T );
            a0 = 257.43 + 0.29 * sin ( U15 );
            d0 = -15.10 + 0.28 * cos ( U15 );
            wd = -41.3514316;
            w  =  77.74 + wd * d + 0.08 * sin ( U15 );
        }
        else if ( id == kOberon )
        {
            double U16 = degtorad ( 259.14 - 504.81 * T );
            a0 = 257.43 + 0.16 * sin ( U16 );
            d0 = -15.10 + 0.16 * cos ( U16 );
            wd = -26.7394932;
            w  =   6.77 + wd * d + 0.04 * sin ( U16 );
        }
        else if ( id == kTriton )
        {
            double N7 = degtorad ( 177.85 + 52.316 * T );
            a0 = 299.36 - 32.35 * sin ( N7 ) - 6.28 * sin ( 2 * N7 ) -2.08 * sin ( 3 * N7 )
                        -  0.74 * sin ( 4 * N7 ) - 0.28 * sin ( 5 * N7 ) - 0.11 * sin ( 6 * N7 )
                        -  0.07 * sin ( 7 * N7 ) - 0.02 * sin ( 8 * N7 ) - 0.01 * sin ( 9 * N7 );
            d0 = 41.17 + 22.55 * cos ( N7 ) + 2.10 * cos ( 2 * N7 ) + 0.55 * cos ( 3 * N7 )
                       +  0.16 * cos ( 4 * N7 ) + 0.05 * cos ( 5 * N7 ) + 0.02 * cos ( 6 * N7 )
                       + 0.01 * cos ( 7 * N7 );
            wd = -61.2572637;
            w  = 296.53 + wd * d + 22.25 * sin ( N7 ) + 6.73 * sin ( 2 * N7 ) + 2.05 * sin ( 3 * N7 )
                        + 0.74 * sin ( 4 * N7 ) + 0.28 * sin ( 5 * N7 ) + 0.11 * sin ( 6 * N7 )
                        + 0.05 * sin ( 7 * N7 ) + 0.02 * sin ( 8 * N7 ) + 0.01 * sin ( 9 * N7 );
        }
        else if ( id == kCharon )
        {
            a0 = 132.993;
            d0 =  -6.163;
            wd =  56.3625225;
            w  = 122.695 + d;
        }
    }
    
    a0 = degtorad ( a0 );
    d0 = degtorad ( d0 );
    wd = degtorad ( wd );
    w = mod2pi ( degtorad ( w ) );
}

// Computes matrix which transforms coordinates from planetographic
// frame to J2000 equatorial frame.

SSMatrix SSPlanet::setPlanetographicMatrix ( double jed )
{
    double a0, d0, w, dw;
    
    rotationElements ( jed, a0, d0, w, dw );
    _pmatrix = SSMatrix::rotation ( 3, 2, w, 0, SSAngle::kHalfPi - d0, 2, a0 + SSAngle::kHalfPi );
    
    return _pmatrix;
}

// Returns flattening factor (i.e. difference between polar and equatorial radii divided by equatorial radius)
// Data from "Report of the IAU Working Group on Cartographic Coordinates and Rotational Elements: 2015", page 28:
// https://astrogeology.usgs.gov/search/map/Docs/WGCCRE/WGCCRE2015reprint

double SSPlanet::flattening ( void )
{
    double f = 0.0;
    int id = (int) _id.identifier();

    if ( _type == kTypePlanet )
    {
        if ( id == kMercury )
            f = ( 2440.53 - 2438.26 ) / 2440.53;
        else if ( id == kEarth )
            f = ( 6378.1366 - 6356.7519 ) / 6378.1366;
        else if ( id == kMars )
            f = ( 3396.19 - 3376.20 ) / 3396.19;
        else if ( id == kJupiter )
            f = ( 71492.0 - 66854.0 ) / 71492.0;
        else if ( id == kSaturn )
            f = ( 60268.0 - 54634.0 ) / 60268.0;
        else if ( id == kUranus )
            f = ( 25559.0 - 24973.0 ) / 25559.0;
        else if ( id == kNeptune )
            f = ( 24764.0 - 24341.0 ) / 24764.0;
    }
    
    return f;
}

// Returns planetographic longitude and latitude of central point on solar system object's apparent disk.
// Assumes object's ephemeris (apparent direction and planetographic matrix) has already been calculated.

SSSpherical SSPlanet::centralCoordinates ( void )
{
    SSVector direction = getDirection() * -1.0;
    SSSpherical coords = _pmatrix.transpose() * direction;
    return coords;
}

// Returns planetographic longitude and latitude of sub-solar point on solar system object.
// Assumes object's ephemeris (heliocentric position and planetographic matrix) has already been calculated.

SSSpherical SSPlanet::subsolarCoordinates ( void )
{
    SSVector position = getPosition().normalize() * -1.0;
    SSSpherical coords = _pmatrix.transpose() * position;
    return coords;
}

// Returns System II longitude of Jupiter's Gred Red Spot, in degrees.
// Formula is a manual linear fit to 2019-2021 data published here:
// http://jupos.privat.t-online.de/img/Grs.GIF

double SSPlanet::getGRSLongitude ( double jd )
{
    double y = SSTime ( jd ).toJulianYear();
    double l = 350.0 + 25.5 * ( y - 2021.0 );
    return mod360 ( l );
}

// Determines if a ray from an external point (p) extending in direction of the unit vector (r) intersects
// this planet's oblate ellipsoid surface, with planet's radius multiplied by a scale factor (s).
// Returns true if the ray intersects planet, and finds distance (d) from (p) to intersection point (q).
// Assumes vectors p, q, r are all in fundamental J2000 mean equatorial reference frame.
// Adapted from https://gis.stackexchange.com/questions/20780/point-of-intersection-for-a-ray-and-earths-surface

bool SSPlanet::rayIntersect ( SSVector p, SSVector r, double &d, SSVector &q, float s )
{
    // First transform vectors from fundamental to planetograhic frame
    // and get heliocentric position vector planet's center in planetographic frame.
    // Get planet radius and flattening factor.
    
    SSMatrix tmatrix = _pmatrix.transpose();
    p = tmatrix * p;
    r = tmatrix * r;
    SSVector c = tmatrix * _position;
    double re = getRadius() * s / SSCoordinates::kKmPerAU;
    double f = flattening();
    
    // Define some variables
    
    double x = p.x - c.x;
    double y = p.y - c.y;
    double z = p.z - c.z;
    double x2 = x * x, y2 = y * y, z2 = z * z;
    double u = r.x, v = r.y, w = r.z;
    double u2 = u * u, v2 = v * v, w2 = w * w;
    double a = re, a2 = a * a;
    double b = re * ( 1.0 - f ), b2 = b * b;
    double t = b2 * ( u * x + v * y ) + a2 * w * z;
    
    // Compute vector from exterior point to center of ellipsoid.
    // If unit vector points away from center of ellipsoid, line doesn't intersect.
    
    q = c - p;
    d = q * r;
    if ( d < 0.0 )
        return false;

    // Any point (x,y,z) along a line from the external point (x0,y0,z0)
    // satisfies the equation (x,y,z) = (x0,y0,z0) + t * (u,v,w).
    // The ellipsoid's equatorial and polar radii are a and b, and the ellipsoid
    // is described by the equation (x^2/a^2) + (y^2/a^2) + (z^2/b^2) = 1.
    // Plug the first equation into the second, and we get a quadratic equation,
    // which we solve for t. There is no solution if the line does not intersect
    // the ellipsoid at all, one solution if the line is exactly tangent to the
    // ellipsoid, and two solutions for most intersection cases (where the line
    // enters the ellipsoid on one side, and exits out the opposite side).
    
    t = t * t - ( b2 * ( u2 + v2 ) + a2 * w2 ) * ( b2 * ( -a2 + x2 + y2 ) + a2 * z2 );
    if ( t < 0 )
        return false;
    
    // We choose the solution which is closest to the external point (x0,y0,z0).
    
    t = ( -1.0 / ( b2 * ( u2 + v2 ) + a2 * w2 ) )
      * ( b2 * ( u * x + v * y ) + a2 * w * z + sqrt ( t ) );
    
    d = t;
    
    // Plug this value into the first equation to obtain the intersection point.
    
    q.x = p.x + t * u;
    q.y = p.y + t * v;
    q.z = p.z + t * w;
    
    // Transform intersection point from planetographic to fundamental frame.
    
    q = _pmatrix * q;
    return true;
}

// Returns length of this solar system object's umbral shadow cone, in AU,
// with the object's physical radius multipled by a scale factor (s).
// Uses hard-coded Sun radius of 695500 km.

double SSPlanet::umbraLength ( float s )
{
    return _position.magnitude() * _radius * s / ( 695500.0 - _radius * s );
}

// Returns radius of this solar system object's umbral shadow cone, in AU,
// with the object's physical radius multipled by a scale factor (s),
// at a distance (d) from the object's center along the shadow cone axis, away from the Sun, in AU.

double SSPlanet::umbraRadius ( double d, float s )
{
    double u = umbraLength ( s );
    double r = _radius * s * ( u - d ) / u;
    return r / SSCoordinates::kKmPerAU;
}

// Returns radius of this solar system object's penumbral shadow cone, in AU,
// with the object's physical radius multipled by a scale factor (scale),
// at a distance (d) from the object's center along the shadow cone axis, away from the Sun, in AU.

double SSPlanet::penumbraRadius ( double d, float s )
{
    double u = umbraLength ( s );
    double r = _radius * s * ( u + d ) / u;
    return r / SSCoordinates::kKmPerAU;
}
