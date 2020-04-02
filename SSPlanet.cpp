// SSPlanet.cpp
// SSCore
//
// Created by Tim DeBenedictis on 3/15/20.
// Copyright © 2020 Southern Stars. All rights reserved.

#include "SSDynamics.hpp"
#include "SSPlanet.hpp"

SSPlanet::SSPlanet ( SSObjectType type ) : SSObject ( type )
{
    _id = SSIdentifier();
    _orbit = SSOrbit();
    _Hmag = _Gmag = _radius = HUGE_VAL;
    _position = _velocity = SSVector ( HUGE_VAL, HUGE_VAL, HUGE_VAL );
}

SSPlanet::SSPlanet ( SSObjectType type, SSPlanetID id ) : SSPlanet ( type )
{
    _id = SSIdentifier ( kCatJPLanet, id );
}

void SSPlanet::computeEphemeris ( SSDynamics &dyn )
{
    double lt = 0.0;
    SSPlanetID planetID = static_cast<SSPlanetID> ( _id.identifier() );
    
    dyn.getPlanetPositionVelocity ( planetID, dyn.jde, _position, _velocity );
    lt = ( _position - dyn.obsPos ).magnitude() / dyn.kLightAUPerDay;

    dyn.getPlanetPositionVelocity ( planetID, dyn.jde - lt, _position, _velocity );
    _direction = ( _position - dyn.obsPos ).normalize ( _distance );
}

// Downcasts generic SSObject pointer to SSPlanet pointer.
// Returns nullptr if input pointer is not an instance of SSPlanet!

SSPlanetPtr SSGetPlanetPtr ( SSObjectPtr ptr )
{
    return dynamic_cast<SSPlanet *> ( ptr.get() );
}

// Returns CSV string from planet data, including identifier and names.

string SSPlanet::toCSV ( void )
{
    string csv = SSObject::typeToCode ( _type ) + ",";
    
    if ( _type == kTypeMoon )
        csv += isinf ( _orbit.q ) ? "," : format ( "%.0f,", _orbit.q * SSDynamics::kKmPerAU );
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

    csv += _id ? _id.toString() + "," : ",";
        
    for ( int i = 0; i < _names.size(); i++ )
        csv += _names[i] + ",";

    return csv;
}

// Allocates a new SSPlanet and initializes it from a CSV-formatted string.
// Returns nullptr on error (invalid CSV string, heap allocation failure, etc.)

SSPlanet *SSPlanet::fromCSV ( string csv )
{
    vector<string> fields = split ( csv, "," );
    
    SSObjectType type = SSObject::codeToType ( fields[0] );
    if ( type < kTypePlanet || type > kTypeComet || fields.size() < 14 )
        return nullptr;
    
    SSOrbit orbit;
    
    orbit.q = fields[1].empty() ? HUGE_VAL : strtofloat64 ( fields[1] );
    orbit.e = fields[2].empty() ? HUGE_VAL : strtofloat64 ( fields[2] );
    orbit.i = fields[3].empty() ? HUGE_VAL : strtofloat64 ( fields[3] ) * SSAngle::kRadPerDeg;
    orbit.w = fields[4].empty() ? HUGE_VAL : strtofloat64 ( fields[4] ) * SSAngle::kRadPerDeg;
    orbit.n = fields[5].empty() ? HUGE_VAL : strtofloat64 ( fields[5] ) * SSAngle::kRadPerDeg;
    orbit.m = fields[6].empty() ? HUGE_VAL : strtofloat64 ( fields[6] ) * SSAngle::kRadPerDeg;
    orbit.mm = fields[7].empty() ? HUGE_VAL : strtofloat64 ( fields[7] ) * SSAngle::kRadPerDeg;
    orbit.t = fields[8].empty() ? HUGE_VAL : strtofloat64 ( fields[8] );

    if ( orbit.q > 1000.0 )
        orbit.q /= SSDynamics::kKmPerAU;
    
    float h = fields[9].empty() ? HUGE_VAL : strtofloat ( fields[9] );
    float g = fields[10].empty() ? HUGE_VAL : strtofloat ( fields[10] );
    float r = fields[11].empty() ? HUGE_VAL : strtofloat ( fields[11] );

    SSIdentifier ident;
    if ( type == kTypePlanet || type == kTypeMoon )
        ident = SSIdentifier ( kCatJPLanet, strtoint ( fields[12] ) );
    else
        ident = SSIdentifier::fromString ( fields[12] );

    vector<string> names;
    for ( int i = 13; i < fields.size(); i++ )
        names.push_back ( trim ( fields[i] ) );
    
    SSPlanetPtr pPlanet = new SSPlanet ( type );
    if ( pPlanet == nullptr )
        return nullptr;
    
    pPlanet->setOrbit ( orbit );
    pPlanet->setHMagnitude ( h );
    pPlanet->setGMagnitude ( g );
    pPlanet->setRadius ( r );
    pPlanet->setIdentifier ( ident );
    pPlanet->setNames ( names );

    return pPlanet;
}
