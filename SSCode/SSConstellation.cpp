// SSConstellation.cpp
// SSCore
//
// Created by Tim DeBenedictis on 3/25/20.
// Copyright © 2020 Southern Stars. All rights reserved.

#include <iostream>
#include <fstream>
#include <map>

#include "SSConstellation.hpp"
#include "SSCoordinates.hpp"

SSObjectVec _constellationVec;    // filled by SSImportConstellations(); used in SSIdentifier string_to_con()

SSConstellation::SSConstellation ( SSObjectType type ) : SSObject ( type )
{
    _area = 0.0;
    _rank = 0;
    _bounds = vector<SSVector> ( 0 );
    _figures = vector<int> ( 0 );
}

// Given a constellation index from 1 (Andromeda) to 88 (Vulpecula),
// returns the offical 3-letter IAU abbreviation for that constellation.
// Returns empty string for invalid constellation index.

string SSConstellation::indexToAbbreviation ( int index )
{
    return con_to_string ( index );
}

// Given an official IAU constellation abbreviation ("And" ... "Vul"),
// returns corresponding index from 1 to 88, or 0 if abbreviation not recognized.

int SSConstellation::abbreviationToIndex ( string abbrev )
{
    return string_to_con ( abbrev, false );
}

// Downcasts generic SSObject pointer to SSConstellation pointer.
// Returns nullptr if input pointer is not an instance of SSPlanet!

SSConstellationPtr SSGetConstellationPtr ( SSObjectPtr ptr )
{
    return dynamic_cast<SSConstellation *> ( ptr );
}

// Allocates a new SSConstellation and initializes it from a CSV-formatted string.
// Returns nullptr on error (invalid CSV string, heap allocation failure, etc.)

SSObjectPtr SSConstellation::fromCSV ( string csv )
{
    vector<string> fields = split ( csv, "," );
    
    SSObjectType type = SSObject::codeToType ( fields[0] );
    if ( type < kTypeConstellation || type > kTypeAsterism || fields.size() < 8 )
        return nullptr;
    
	SSObjectPtr pObject = SSNewObject ( type );
    SSConstellationPtr pCon = SSGetConstellationPtr ( pObject );
    if ( pCon == nullptr )
        return nullptr;
    
    double ra = degtorad ( strtofloat64 ( fields[1] ) * 15.0 );
    double dec = degtorad ( strtofloat64 ( fields[2] ) );
    SSVector center ( SSSpherical ( ra, dec, 1.0 ) );
    
    pCon->setDirection ( center );
    pCon->setArea ( degtorad ( degtorad ( strtofloat64 ( fields[3] ) ) ) );
    pCon->setRank ( strtoint ( fields[4] ) );
    pCon->setNames ( vector<string> ( &fields[5], &fields[8] ) );

    return pObject;
}

string SSConstellation::toCSV ( void )
{
    string csv = SSObject::typeToCode ( _type ) + ",";
    
    SSSpherical center ( _direction );
    double ra = radtodeg ( center.lon / 15.0 );
    double dec = radtodeg ( center.lat );
    double area = radtodeg ( radtodeg ( _area ) );
    
    csv += ::isinf ( ra ) ? "," : formstr ( "%.7f,", ra );
    csv += ::isinf ( dec ) ? "," : formstr ( "%+.6f,", dec );
    csv += ::isinf ( area ) ? "," : formstr ( "%.2f,", area );
    csv += _rank < 1 ? "," : formstr ( "%d,", _rank );
    
    for ( int i = 0; i < _names.size(); i++ )
        csv += _names[i] + ",";
    
    return csv;
}

// Reads constellation data from CSV-formatted text file.
// Imported data is appended to the input vector of SSObjects (constellations).
// Returns number of consteillations successfully imported (should be 88).

int SSImportConstellations ( const string &filename, SSObjectVec &constellations )
{
    // Open file; return on failure.

    FILE *file = fopen ( filename.c_str(), "rb" );
    if ( ! file )
        return ( 0 );

    // Read file line-by-line until we reach end-of-file

    string line = "";
    int numCons = 0;

    while ( fgetline ( file, line ) )
    {
        // Attempt to create constellation from CSV file line; continue on failure.
        
		SSObjectPtr pObject = SSConstellation::fromCSV ( line );
		SSConstellationPtr pCon = SSGetConstellationPtr ( pObject );
        if ( pCon == nullptr )
            continue;
        
        // If successful, append new constellation to object vector.
        
        // cout << pCon->toCSV() << endl;
        constellations.append ( pObject );
        numCons++;
    }
    
    fclose ( file );
    return numCons;
}

// If no constellation vector supplied, reads constellations into our local copy.

int SSImportConstellations ( const string &filename )
{
    return SSImportConstellations ( filename, _constellationVec );
}

// Interpolates constellation boundary from point (ra0,dec0) to (ra1,dec1) in radians
// with a maximum step size (res) in dedgrees.
// If (close) is true, interpolation will include last point (ra1,dec); if false,
// boundary will be inerpolated up to but not including (ra1,dec1)

void interpolateBoundary ( double ra0, double dec0, double ra1, double dec1, bool close, double res, vector<SSVector> &bound )
{
    double dra = modpi ( ra1 - ra0 );
    double ddec = dec1 - dec0;
    
    int nsteps = 1;
    if ( res != 0.0 )
        nsteps = ceil ( ( fabs ( dra ) + fabs ( ddec ) ) / degtorad ( res ) );
    
    dra /= nsteps;
    ddec /= nsteps;
    
    double ra = ra0;
    double dec = dec0;
    
    if ( close )
        nsteps++;
    
    for ( int i = 0; i < nsteps; i++ )
    {
        SSVector vertex ( SSSpherical ( ra, dec, 1.0 ) );
        static SSMatrix precess = SSCoordinates::getPrecessionMatrix ( SSTime::fromBesselianYear ( 1875.0 ) ).transpose();
        vertex = precess * vertex;
        bound.push_back ( vertex );

        ra += dra;
        dec += ddec;
    }
}

// Reads constellation boundary data from CSV-formatted text file.
// Imported data is stored in each constellation in input vector of SSObjects (constellations).
// Assumes constellations in input vector are sorted by constellation name alphabetically,
// i.e. constellation[0] is Andromeda, [1] is Antlia, ... [87] is Vulpecula.
// Returns number of boundary vertices successfully imported (should be 1562).

int SSImportConstellationBoundaries ( const string &filename, SSObjectVec &constellations )
{
    // Open file; return on failure.

    FILE *file = fopen ( filename.c_str(), "rb" );
    if ( ! file )
        return ( 0 );

    // Read file line-by-line until we reach end-of-file

    string line = "", abbr = "", lastAbbr = "And";
    int numVerts = 0;
    
    double ra0 = 0.0, dec0 = 0.0, ra1 = 0.0, dec1 = 0.0;
    double ra00 = 0.0, dec00 = 0.0;
    
    vector<SSVector> boundary ( 0 );
    SSConstellationPtr pCon = SSGetConstellationPtr ( constellations[0] );

    while ( fgetline ( file, line ) )
    {
        // Require 3 fields per line; skip if we don't have em.
        
        vector<string> fields = split ( line, "," );
        if ( fields.size() < 3 )
            continue;
        
        // Get constellation abbreviation. Truncate to first 3 characters (to handle "Ser1" and "Ser2").
        // Skip line if abbreviation not recognized.
        
        abbr = ( fields[2].length() <= 3 ) ? fields[2] : fields[2].substr ( 0, 3 );
        int index = SSConstellation::abbreviationToIndex ( abbr );
        if ( index < 1 )
            continue;
        
        // If different than the previous vertex abbreviation...
        
        if ( abbr.compare ( lastAbbr ) != 0 )
        {
            // Close boundary by interpolating to first vertex,
            // then store boundary in current constellation.
            
            if ( pCon != nullptr && boundary.size() > 0 )
            {
                interpolateBoundary ( ra0, dec0, ra00, dec00, true, 5.0, boundary );
                pCon->setBoundary ( boundary );
                // cout << "Imported " << boundary.size() << " vertices for " << lastAbbr << endl;
                ra0 = dec0 = 0.0;
            }

            // Get pointer to new constellation, and start new boundary vertex.
            
            if ( index > 0 && index <= constellations.size() )
                pCon = SSGetConstellationPtr( constellations[index - 1] );

            boundary.clear();
        }
        
        // Extract vertex B1875 RA and Dec; skip if both are zero (no vertex at these coords).
        
        ra1 = degtorad ( strtofloat64 ( fields[0] ) * 15.0 );
        dec1 = degtorad ( strtofloat64 ( fields[1] ) );
        if ( ra1 == 0.0 && dec1 == 0.0 )
            continue;

        // Save first vertex in boundary; interpolate to subsequent vertices.

        if ( ra0 == 0.0 && dec0 == 0.0 )
        {
            ra00 = ra1;
            dec00 = dec1;
        }
        else
        {
            interpolateBoundary ( ra0, dec0, ra1, dec1, false, 5.0, boundary );
        }
        
        // save current vertex for interpolation to next vertex
        
        ra0 = ra1;
        dec0 = dec1;

        // increment vertex counter.
        
        lastAbbr = abbr;
        numVerts++;
    }
    
    // After we reach end of file, add final boundary to last constellation.
    
    if ( pCon != nullptr && boundary.size() > 0 )
    {
        pCon->setBoundary ( boundary );
        // cout << "Imported " << boundary.size() << " vertices for " << lastAbbr << endl;
    }

    fclose ( file );
    return numVerts;
}

// Reads constellation shape data from CSV-formatted text file.
// Imported data is stored in input vector of SSConstellations (constellations).
// Assumes constellations in input vector are sorted by constellation name alphabetically,
// i.e. constellation[0] is Andromeda, [1] is Antlia, ... [87] is Vulpecula.
// Returns number of shape lines successfully imported (should be 765).

int SSImportConstellationShapes ( const string &filename, SSObjectVec &constellations )
{
    // Open file; return on failure.

    FILE *file = fopen ( filename.c_str(), "rb" );
    if ( ! file )
        return ( 0 );

    // Read file line-by-line until we reach end-of-file

    string line = "", abbr = "", lastAbbr = "And";
    int numLines = 0;
    vector<int> shape ( 0 );
    SSConstellationPtr pCon = SSGetConstellationPtr ( constellations[0] );

    while ( fgetline ( file, line ) )
    {
        // Require 3 fields per line; skip if we don't have enough.
        
        vector<string> fields = split ( line, "," );
        if ( fields.size() < 3 )
            continue;
        
        // Get constellation abbreviation. Skip line if abbreviation not recognized.
        
        abbr = fields[0];
        int index = SSConstellation::abbreviationToIndex ( abbr );
        if ( index < 1 )
            continue;
        
        // If different than the previous vertex abbreviation...
        
        if ( abbr.compare ( lastAbbr ) != 0 )
        {
            // Store current boundary in current constellation
            
            if ( pCon != nullptr && shape.size() > 0 )
            {
                pCon->setFigure ( shape );
                // cout << "Imported " << shape.size() / 2 << " shape lines for " << lastAbbr << endl;
            }

            // Get pointer to new constellation, and start new boundary vertex.
            
            if ( index > 0 && index <= constellations.size() )
                pCon = SSGetConstellationPtr( constellations[index - 1] );

            shape.clear();
        }
        
        // Extract shape line endpoint star HR numbers; skip if either are zero.
        
        int hr1 = strtoint ( fields[1]);
        int hr2 = strtoint ( fields[2]);
        if ( hr1 == 0 || hr2 == 0 )
            continue;
        
        // Append line endpoints to current shape, increment line counter.
        
        shape.push_back ( hr1 );
        shape.push_back ( hr2 );
        
        lastAbbr = abbr;
        numLines++;
    }
    
    // After we reach end of file, add final shape to last constellation.
    
    if ( pCon != nullptr && shape.size() > 0 )
    {
        pCon->setFigure ( shape );
        // cout << "Imported " << shape.size() / 2 << " shape lines for " << lastAbbr << endl;
    }

    fclose ( file );
    return numLines;
}

// This struct contains data used to identify a constellation from a position.

struct CData
{
    float ral;      // right ascension lower boundary, B1875.0 [decimal hours]
    float rau;      // right ascention upper boundard, B1875.0 [decimal hours]
    float decl;     // declination lower boundary, B1875.0 [decimal degrees]
    char  con[4];   // constellation IAU three-letter abbrevation plus terminating NUL
};

// Data table from "Identification of a Constellation from a Position", Nancy G. Roman, PASP 99:695-699, July 1987.
// Machine-readable version here: https://cdsarc.unistra.fr/viz-bin/cat/VI/42

static CData _table[357] =
{
    {  0.0000, 24.0000,  88.0000, "UMi" },
    {  8.0000, 14.5000,  86.5000, "UMi" },
    { 21.0000, 23.0000,  86.1667, "UMi" },
    { 18.0000, 21.0000,  86.0000, "UMi" },
    {  0.0000,  8.0000,  85.0000, "Cep" },
    {  9.1667, 10.6667,  82.0000, "Cam" },
    {  0.0000,  5.0000,  80.0000, "Cep" },
    { 10.6667, 14.5000,  80.0000, "Cam" },
    { 17.5000, 18.0000,  80.0000, "UMi" },
    { 20.1667, 21.0000,  80.0000, "Dra" },
    {  0.0000,  3.5083,  77.0000, "Cep" },
    { 11.5000, 13.5833,  77.0000, "Cam" },
    { 16.5333, 17.5000,  75.0000, "UMi" },
    { 20.1667, 20.6667,  75.0000, "Cep" },
    {  7.9667,  9.1667,  73.5000, "Cam" },
    {  9.1667, 11.3333,  73.5000, "Dra" },
    { 13.0000, 16.5333,  70.0000, "UMi" },
    {  3.1000,  3.4167,  68.0000, "Cas" },
    { 20.4167, 20.6667,  67.0000, "Dra" },
    { 11.3333, 12.0000,  66.5000, "Dra" },
    {  0.0000,  0.3333,  66.0000, "Cep" },
    { 14.0000, 15.6667,  66.0000, "UMi" },
    { 23.5833, 24.0000,  66.0000, "Cep" },
    { 12.0000, 13.5000,  64.0000, "Dra" },
    { 13.5000, 14.4167,  63.0000, "Dra" },
    { 23.1667, 23.5833,  63.0000, "Cep" },
    {  6.1000,  7.0000,  62.0000, "Cam" },
    { 20.0000, 20.4167,  61.5000, "Dra" },
    { 20.5367, 20.6000,  60.9167, "Cep" },
    {  7.0000,  7.9667,  60.0000, "Cam" },
    {  7.9667,  8.4167,  60.0000, "UMa" },
    { 19.7667, 20.0000,  59.5000, "Dra" },
    { 20.0000, 20.5367,  59.5000, "Cep" },
    { 22.8667, 23.1667,  59.0833, "Cep" },
    {  0.0000,  2.4333,  58.5000, "Cas" },
    { 19.4167, 19.7667,  58.0000, "Dra" },
    {  1.7000,  1.9083,  57.5000, "Cas" },
    {  2.4333,  3.1000,  57.0000, "Cas" },
    {  3.1000,  3.1667,  57.0000, "Cam" },
    { 22.3167, 22.8667,  56.2500, "Cep" },
    {  5.0000,  6.1000,  56.0000, "Cam" },
    { 14.0333, 14.4167,  55.5000, "UMa" },
    { 14.4167, 19.4167,  55.5000, "Dra" },
    {  3.1667,  3.3333,  55.0000, "Cam" },
    { 22.1333, 22.3167,  55.0000, "Cep" },
    { 20.6000, 21.9667,  54.8333, "Cep" },
    {  0.0000,  1.7000,  54.0000, "Cas" },
    {  6.1000,  6.5000,  54.0000, "Lyn" },
    { 12.0833, 13.5000,  53.0000, "UMa" },
    { 15.2500, 15.7500,  53.0000, "Dra" },
    { 21.9667, 22.1333,  52.7500, "Cep" },
    {  3.3333,  5.0000,  52.5000, "Cam" },
    { 22.8667, 23.3333,  52.5000, "Cas" },
    { 15.7500, 17.0000,  51.5000, "Dra" },
    {  2.0417,  2.5167,  50.5000, "Per" },
    { 17.0000, 18.2333,  50.5000, "Dra" },
    {  0.0000,  1.3667,  50.0000, "Cas" },
    {  1.3667,  1.6667,  50.0000, "Per" },
    {  6.5000,  6.8000,  50.0000, "Lyn" },
    { 23.3333, 24.0000,  50.0000, "Cas" },
    { 13.5000, 14.0333,  48.5000, "UMa" },
    {  0.0000,  1.1167,  48.0000, "Cas" },
    { 23.5833, 24.0000,  48.0000, "Cas" },
    { 18.1750, 18.2333,  47.5000, "Her" },
    { 18.2333, 19.0833,  47.5000, "Dra" },
    { 19.0833, 19.1667,  47.5000, "Cyg" },
    {  1.6667,  2.0417,  47.0000, "Per" },
    {  8.4167,  9.1667,  47.0000, "UMa" },
    {  0.1667,  0.8667,  46.0000, "Cas" },
    { 12.0000, 12.0833,  45.0000, "UMa" },
    {  6.8000,  7.3667,  44.5000, "Lyn" },
    { 21.9083, 21.9667,  44.0000, "Cyg" },
    { 21.8750, 21.9083,  43.7500, "Cyg" },
    { 19.1667, 19.4000,  43.5000, "Cyg" },
    {  9.1667, 10.1667,  42.0000, "UMa" },
    { 10.1667, 10.7833,  40.0000, "UMa" },
    { 15.4333, 15.7500,  40.0000, "Boo" },
    { 15.7500, 16.3333,  40.0000, "Her" },
    {  9.2500,  9.5833,  39.7500, "Lyn" },
    {  0.0000,  2.5167,  36.7500, "And" },
    {  2.5167,  2.5667,  36.7500, "Per" },
    { 19.3583, 19.4000,  36.5000, "Lyr" },
    {  4.5000,  4.6917,  36.0000, "Per" },
    { 21.7333, 21.8750,  36.0000, "Cyg" },
    { 21.8750, 22.0000,  36.0000, "Lac" },
    {  6.5333,  7.3667,  35.5000, "Aur" },
    {  7.3667,  7.7500,  35.5000, "Lyn" },
    {  0.0000,  2.0000,  35.0000, "And" },
    { 22.0000, 22.8167,  35.0000, "Lac" },
    { 22.8167, 22.8667,  34.5000, "Lac" },
    { 22.8667, 23.5000,  34.5000, "And" },
    {  2.5667,  2.7167,  34.0000, "Per" },
    { 10.7833, 11.0000,  34.0000, "UMa" },
    { 12.0000, 12.3333,  34.0000, "CVn" },
    {  7.7500,  9.2500,  33.5000, "Lyn" },
    {  9.2500,  9.8833,  33.5000, "LMi" },
    {  0.7167,  1.4083,  33.0000, "And" },
    { 15.1833, 15.4333,  33.0000, "Boo" },
    { 23.5000, 23.7500,  32.0833, "And" },
    { 12.3333, 13.2500,  32.0000, "CVn" },
    { 23.7500, 24.0000,  31.3333, "And" },
    { 13.9583, 14.0333,  30.7500, "CVn" },
    {  2.4167,  2.7167,  30.6667, "Tri" },
    {  2.7167,  4.5000,  30.6667, "Per" },
    {  4.5000,  4.7500,  30.0000, "Aur" },
    { 18.1750, 19.3583,  30.0000, "Lyr" },
    { 11.0000, 12.0000,  29.0000, "UMa" },
    { 19.6667, 20.9167,  29.0000, "Cyg" },
    {  4.7500,  5.8833,  28.5000, "Aur" },
    {  9.8833, 10.5000,  28.5000, "LMi" },
    { 13.2500, 13.9583,  28.5000, "CVn" },
    {  0.0000,  0.0667,  28.0000, "And" },
    {  1.4083,  1.6667,  28.0000, "Tri" },
    {  5.8833,  6.5333,  28.0000, "Aur" },
    {  7.8833,  8.0000,  28.0000, "Gem" },
    { 20.9167, 21.7333,  28.0000, "Cyg" },
    { 19.2583, 19.6667,  27.5000, "Cyg" },
    {  1.9167,  2.4167,  27.2500, "Tri" },
    { 16.1667, 16.3333,  27.0000, "CrB" },
    { 15.0833, 15.1833,  26.0000, "Boo" },
    { 15.1833, 16.1667,  26.0000, "CrB" },
    { 18.3667, 18.8667,  26.0000, "Lyr" },
    { 10.7500, 11.0000,  25.5000, "LMi" },
    { 18.8667, 19.2583,  25.5000, "Lyr" },
    {  1.6667,  1.9167,  25.0000, "Tri" },
    {  0.7167,  0.8500,  23.7500, "Psc" },
    { 10.5000, 10.7500,  23.5000, "LMi" },
    { 21.2500, 21.4167,  23.5000, "Vul" },
    {  5.7000,  5.8833,  22.8333, "Tau" },
    {  0.0667,  0.1417,  22.0000, "And" },
    { 15.9167, 16.0333,  22.0000, "Ser" },
    {  5.8833,  6.2167,  21.5000, "Gem" },
    { 19.8333, 20.2500,  21.2500, "Vul" },
    { 18.8667, 19.2500,  21.0833, "Vul" },
    {  0.1417,  0.8500,  21.0000, "And" },
    { 20.2500, 20.5667,  20.5000, "Vul" },
    {  7.8083,  7.8833,  20.0000, "Gem" },
    { 20.5667, 21.2500,  19.5000, "Vul" },
    { 19.2500, 19.8333,  19.1667, "Vul" },
    {  3.2833,  3.3667,  19.0000, "Ari" },
    { 18.8667, 19.0000,  18.5000, "Sge" },
    {  5.7000,  5.7667,  18.0000, "Ori" },
    {  6.2167,  6.3083,  17.5000, "Gem" },
    { 19.0000, 19.8333,  16.1667, "Sge" },
    {  4.9667,  5.3333,  16.0000, "Tau" },
    { 15.9167, 16.0833,  16.0000, "Her" },
    { 19.8333, 20.2500,  15.7500, "Sge" },
    {  4.6167,  4.9667,  15.5000, "Tau" },
    {  5.3333,  5.6000,  15.5000, "Tau" },
    { 12.8333, 13.5000,  15.0000, "Com" },
    { 17.2500, 18.2500,  14.3333, "Her" },
    { 11.8667, 12.8333,  14.0000, "Com" },
    {  7.5000,  7.8083,  13.5000, "Gem" },
    { 16.7500, 17.2500,  12.8333, "Her" },
    {  0.0000,  0.1417,  12.5000, "Peg" },
    {  5.6000,  5.7667,  12.5000, "Tau" },
    {  7.0000,  7.5000,  12.5000, "Gem" },
    { 21.1167, 21.3333,  12.5000, "Peg" },
    {  6.3083,  6.9333,  12.0000, "Gem" },
    { 18.2500, 18.8667,  12.0000, "Her" },
    { 20.8750, 21.0500,  11.8333, "Del" },
    { 21.0500, 21.1167,  11.8333, "Peg" },
    { 11.5167, 11.8667,  11.0000, "Leo" },
    {  6.2417,  6.3083,  10.0000, "Ori" },
    {  6.9333,  7.0000,  10.0000, "Gem" },
    {  7.8083,  7.9250,  10.0000, "Cnc" },
    { 23.8333, 24.0000,  10.0000, "Peg" },
    {  1.6667,  3.2833,   9.9167, "Ari" },
    { 20.1417, 20.3000,   8.5000, "Del" },
    { 13.5000, 15.0833,   8.0000, "Boo" },
    { 22.7500, 23.8333,   7.5000, "Peg" },
    {  7.9250,  9.2500,   7.0000, "Cnc" },
    {  9.2500, 10.7500,   7.0000, "Leo" },
    { 18.2500, 18.6622,   6.2500, "Oph" },
    { 18.6622, 18.8667,   6.2500, "Aql" },
    { 20.8333, 20.8750,   6.0000, "Del" },
    {  7.0000,  7.0167,   5.5000, "CMi" },
    { 18.2500, 18.4250,   4.5000, "Ser" },
    { 16.0833, 16.7500,   4.0000, "Her" },
    { 18.2500, 18.4250,   3.0000, "Oph" },
    { 21.4667, 21.6667,   2.7500, "Peg" },
    {  0.0000,  2.0000,   2.0000, "Psc" },
    { 18.5833, 18.8667,   2.0000, "Ser" },
    { 20.3000, 20.8333,   2.0000, "Del" },
    { 20.8333, 21.3333,   2.0000, "Equ" },
    { 21.3333, 21.4667,   2.0000, "Peg" },
    { 22.0000, 22.7500,   2.0000, "Peg" },
    { 21.6667, 22.0000,   1.7500, "Peg" },
    {  7.0167,  7.2000,   1.5000, "CMi" },
    {  3.5833,  4.6167,   0.0000, "Tau" },
    {  4.6167,  4.6667,   0.0000, "Ori" },
    {  7.2000,  8.0833,   0.0000, "CMi" },
    { 14.6667, 15.0833,   0.0000, "Vir" },
    { 17.8333, 18.2500,   0.0000, "Oph" },
    {  2.6500,  3.2833, -01.7500, "Cet" },
    {  3.2833,  3.5833, -01.7500, "Tau" },
    { 15.0833, 16.2667, -03.2500, "Ser" },
    {  4.6667,  5.0833, -04.0000, "Ori" },
    {  5.8333,  6.2417, -04.0000, "Ori" },
    { 17.8333, 17.9667, -04.0000, "Ser" },
    { 18.2500, 18.5833, -04.0000, "Ser" },
    { 18.5833, 18.8667, -04.0000, "Aql" },
    { 22.7500, 23.8333, -04.0000, "Psc" },
    { 10.7500, 11.5167, -06.0000, "Leo" },
    { 11.5167, 11.8333, -06.0000, "Vir" },
    {  0.0000, 00.3333, -07.0000, "Psc" },
    { 23.8333, 24.0000, -07.0000, "Psc" },
    { 14.2500, 14.6667, -08.0000, "Vir" },
    { 15.9167, 16.2667, -08.0000, "Oph" },
    { 20.0000, 20.5333, -09.0000, "Aql" },
    { 21.3333, 21.8667, -09.0000, "Aqr" },
    { 17.1667, 17.9667, -10.0000, "Oph" },
    {  5.8333,  8.0833, -11.0000, "Mon" },
    {  4.9167,  5.0833, -11.0000, "Eri" },
    {  5.0833,  5.8333, -11.0000, "Ori" },
    {  8.0833,  8.3667, -11.0000, "Hya" },
    {  9.5833, 10.7500, -11.0000, "Sex" },
    { 11.8333, 12.8333, -11.0000, "Vir" },
    { 17.5833, 17.6667, -11.6667, "Oph" },
    { 18.8667, 20.0000, -12.0333, "Aql" },
    {  4.8333,  4.9167, -14.5000, "Eri" },
    { 20.5333, 21.3333, -15.0000, "Aqr" },
    { 17.1667, 18.2500, -16.0000, "Ser" },
    { 18.2500, 18.8667, -16.0000, "Sct" },
    {  8.3667,  8.5833, -17.0000, "Hya" },
    { 16.2667, 16.3750, -18.2500, "Oph" },
    {  8.5833,  9.0833, -19.0000, "Hya" },
    { 10.7500, 10.8333, -19.0000, "Crt" },
    { 16.2667, 16.3750, -19.2500, "Sco" },
    { 15.6667, 15.9167, -20.0000, "Lib" },
    { 12.5833, 12.8333, -22.0000, "Crv" },
    { 12.8333, 14.2500, -22.0000, "Vir" },
    {  9.0833,  9.7500, -24.0000, "Hya" },
    {  1.6667,  2.6500, -24.3833, "Cet" },
    {  2.6500,  3.7500, -24.3833, "Eri" },
    { 10.8333, 11.8333, -24.5000, "Crt" },
    { 11.8333, 12.5833, -24.5000, "Crv" },
    { 14.2500, 14.9167, -24.5000, "Lib" },
    { 16.2667, 16.7500, -24.5833, "Oph" },
    {  0.0000,  1.6667, -25.5000, "Cet" },
    { 21.3333, 21.8667, -25.5000, "Cap" },
    { 21.8667, 23.8333, -25.5000, "Aqr" },
    { 23.8333, 24.0000, -25.5000, "Cet" },
    {  9.7500, 10.2500, -26.5000, "Hya" },
    {  4.7000,  4.8333, -27.2500, "Eri" },
    {  4.8333,  6.1167, -27.2500, "Lep" },
    { 20.0000, 21.3333, -28.0000, "Cap" },
    { 10.2500, 10.5833, -29.1667, "Hya" },
    { 12.5833, 14.9167, -29.5000, "Hya" },
    { 14.9167, 15.6667, -29.5000, "Lib" },
    { 15.6667, 16.0000, -29.5000, "Sco" },
    {  4.5833,  4.7000, -30.0000, "Eri" },
    { 16.7500, 17.6000, -30.0000, "Oph" },
    { 17.6000, 17.8333, -30.0000, "Sgr" },
    { 10.5833, 10.8333, -31.1667, "Hya" },
    {  6.1167,  7.3667, -33.0000, "CMa" },
    { 12.2500, 12.5833, -33.0000, "Hya" },
    { 10.8333, 12.2500, -35.0000, "Hya" },
    {  3.5000,  3.7500, -36.0000, "For" },
    {  8.3667,  9.3667, -36.7500, "Pyx" },
    {  4.2667,  4.5833, -37.0000, "Eri" },
    { 17.8333, 19.1667, -37.0000, "Sgr" },
    { 21.3333, 23.0000, -37.0000, "PsA" },
    { 23.0000, 23.3333, -37.0000, "Scl" },
    {  3.0000,  3.5000, -39.5833, "For" },
    {  9.3667, 11.0000, -39.7500, "Ant" },
    {  0.0000,  1.6667, -40.0000, "Scl" },
    {  1.6667,  3.0000, -40.0000, "For" },
    {  3.8667,  4.2667, -40.0000, "Eri" },
    { 23.3333, 24.0000, -40.0000, "Scl" },
    { 14.1667, 14.9167, -42.0000, "Cen" },
    { 15.6667, 16.0000, -42.0000, "Lup" },
    { 16.0000, 16.4208, -42.0000, "Sco" },
    {  4.8333,  5.0000, -43.0000, "Cae" },
    {  5.0000,  6.5833, -43.0000, "Col" },
    {  8.0000,  8.3667, -43.0000, "Pup" },
    {  3.4167,  3.8667, -44.0000, "Eri" },
    { 16.4208, 17.8333, -45.5000, "Sco" },
    { 17.8333, 19.1667, -45.5000, "CrA" },
    { 19.1667, 20.3333, -45.5000, "Sgr" },
    { 20.3333, 21.3333, -45.5000, "Mic" },
    {  3.0000,  3.4167, -46.0000, "Eri" },
    {  4.5000,  4.8333, -46.5000, "Cae" },
    { 15.3333, 15.6667, -48.0000, "Lup" },
    {  0.0000,  2.3333, -48.1667, "Phe" },
    {  2.6667,  3.0000, -49.0000, "Eri" },
    {  4.0833,  4.2667, -49.0000, "Hor" },
    {  4.2667,  4.5000, -49.0000, "Cae" },
    { 21.3333, 22.0000, -50.0000, "Gru" },
    {  6.0000,  8.0000, -50.7500, "Pup" },
    {  8.0000,  8.1667, -50.7500, "Vel" },
    {  2.4167,  2.6667, -51.0000, "Eri" },
    {  3.8333,  4.0833, -51.0000, "Hor" },
    {  0.0000,  1.8333, -51.5000, "Phe" },
    {  6.0000,  6.1667, -52.5000, "Car" },
    {  8.1667,  8.4500, -53.0000, "Vel" },
    {  3.5000,  3.8333, -53.1667, "Hor" },
    {  3.8333,  4.0000, -53.1667, "Dor" },
    {  0.0000,  1.5833, -53.5000, "Phe" },
    {  2.1667,  2.4167, -54.0000, "Eri" },
    {  4.5000,  5.0000, -54.0000, "Pic" },
    { 15.0500, 15.3333, -54.0000, "Lup" },
    {  8.4500,  8.8333, -54.5000, "Vel" },
    {  6.1667,  6.5000, -55.0000, "Car" },
    { 11.8333, 12.8333, -55.0000, "Cen" },
    { 14.1667, 15.0500, -55.0000, "Lup" },
    { 15.0500, 15.3333, -55.0000, "Nor" },
    {  4.0000,  4.3333, -56.5000, "Dor" },
    {  8.8333, 11.0000, -56.5000, "Vel" },
    { 11.0000, 11.2500, -56.5000, "Cen" },
    { 17.5000, 18.0000, -57.0000, "Ara" },
    { 18.0000, 20.3333, -57.0000, "Tel" },
    { 22.0000, 23.3333, -57.0000, "Gru" },
    {  3.2000,  3.5000, -57.5000, "Hor" },
    {  5.0000,  5.5000, -57.5000, "Pic" },
    {  6.5000,  6.8333, -58.0000, "Car" },
    {  0.0000,  1.3333, -58.5000, "Phe" },
    {  1.3333,  2.1667, -58.5000, "Eri" },
    { 23.3333, 24.0000, -58.5000, "Phe" },
    {  4.3333,  4.5833, -59.0000, "Dor" },
    { 15.3333, 16.4208, -60.0000, "Nor" },
    { 20.3333, 21.3333, -60.0000, "Ind" },
    {  5.5000,  6.0000, -61.0000, "Pic" },
    { 15.1667, 15.3333, -61.0000, "Cir" },
    { 16.4208, 16.5833, -61.0000, "Ara" },
    { 14.9167, 15.1667, -63.5833, "Cir" },
    { 16.5833, 16.7500, -63.5833, "Ara" },
    {  6.0000,  6.8333, -64.0000, "Pic" },
    {  6.8333,  9.0333, -64.0000, "Car" },
    { 11.2500, 11.8333, -64.0000, "Cen" },
    { 11.8333, 12.8333, -64.0000, "Cru" },
    { 12.8333, 14.5333, -64.0000, "Cen" },
    { 13.5000, 13.6667, -65.0000, "Cir" },
    { 16.7500, 16.8333, -65.0000, "Ara" },
    {  2.1667,  3.2000, -67.5000, "Hor" },
    {  3.2000,  4.5833, -67.5000, "Ret" },
    { 14.7500, 14.9167, -67.5000, "Cir" },
    { 16.8333, 17.5000, -67.5000, "Ara" },
    { 17.5000, 18.0000, -67.5000, "Pav" },
    { 22.0000, 23.3333, -67.5000, "Tuc" },
    {  4.5833,  6.5833, -70.0000, "Dor" },
    { 13.6667, 14.7500, -70.0000, "Cir" },
    { 14.7500, 17.0000, -70.0000, "TrA" },
    {  0.0000,  1.3333, -75.0000, "Tuc" },
    {  3.5000,  4.5833, -75.0000, "Hyi" },
    {  6.5833,  9.0333, -75.0000, "Vol" },
    {  9.0333, 11.2500, -75.0000, "Car" },
    { 11.2500, 13.6667, -75.0000, "Mus" },
    { 18.0000, 21.3333, -75.0000, "Pav" },
    { 21.3333, 23.3333, -75.0000, "Ind" },
    { 23.3333, 24.0000, -75.0000, "Tuc" },
    {  0.7500,  1.3333, -76.0000, "Tuc" },
    {  0.0000,  3.5000, -82.5000, "Hyi" },
    {  7.6667, 13.6667, -82.5000, "Cha" },
    { 13.6667, 18.0000, -82.5000, "Aps" },
    {  3.5000,  7.6667, -85.0000, "Men" },
    {  0.0000, 24.0000, -90.0000, "Oct" }
};

// identifies constellation from position in B1875 equatorial cooordinates
// (ra,dec) both in radians; returns 3-letter constellation abbreviation string.

string SSConstellation::identify ( double ra, double dec )
{
    ra *= SSAngle::kHourPerRad;
    dec *= SSAngle::kDegPerRad;
    
    int i = 0;
    while ( ra < _table[i].ral || ra >= _table[i].rau || dec < _table[i].decl )
        i++;
    return string ( _table[i].con );
}

// identifies constellation from unit position vector in J2000 equatorial cooordinates.
// Returns 3-letter constellation abbreviation string.

string SSConstellation::identify ( SSVector position )
{
    static SSMatrix precess = SSCoordinates::getPrecessionMatrix ( SSTime::fromBesselianYear ( 1875.0 ) );
    SSSpherical coords = precess * position;
    return identify ( coords.lon, coords.lat );
}
