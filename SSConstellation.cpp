// SSConstellation.cpp
// SSCore
//
// Created by Tim DeBenedictis on 3/25/20.
// Copyright © 2020 Southern Stars. All rights reserved.

#include <iostream>
#include <fstream>
#include <map>

#include "SSConstellation.hpp"

static vector<string> _convec =
{
	"And", "Ant", "Aps", "Aqr", "Aql", "Ara", "Ari", "Aur",
	"Boo", "Cae", "Cam", "Cnc", "CVn", "CMa", "CMi", "Cap",
	"Car", "Cas", "Cen", "Cep", "Cet", "Cha", "Cir", "Col",
	"Com", "CrA", "CrB", "Crv", "Crt", "Cru", "Cyg", "Del",
	"Dor", "Dra", "Equ", "Eri", "For", "Gem", "Gru", "Her",
	"Hor", "Hya", "Hyi", "Ind", "Lac", "Leo", "LMi", "Lep",
	"Lib", "Lup", "Lyn", "Lyr", "Men", "Mic", "Mon", "Mus",
	"Nor", "Oct", "Oph", "Ori", "Pav", "Peg", "Per", "Phe",
	"Pic", "Psc", "PsA", "Pup", "Pyx", "Ret", "Sge", "Sgr",
	"Sco", "Scl", "Sct", "Ser", "Sex", "Tau", "Tel", "Tri",
	"TrA", "Tuc", "UMa", "UMi", "Vel", "Vir", "Vol", "Vul"
};

static map<string,int> _conmap;

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
	if ( index >=1 && index <= 88 )
		return _convec[index];
	else
		return "";
}

// Given an official IAU constellation abbreviation ("And" ... "Vul"),
// returns corresponding index from 1 to 88, or 0 if abbreviation not recognized.

int SSConstellation::abbreviationToIndex ( string abbrev )
{
	// Initialize abbreviation-to-index mapping if it's empty
	
	if ( _conmap.size() == 0 )
		for ( int i = 0; i < _convec.size(); i++ )
			_conmap.insert ( { _convec[i], i + 1 } );

	return _conmap[abbrev];
}


// Downcasts generic SSObject pointer to SSConstellation pointer.
// Returns nullptr if input pointer is not an instance of SSPlanet!

SSConstellationPtr SSGetConstellationPtr ( SSObjectPtr ptr )
{
    return dynamic_cast<SSConstellation *> ( ptr.get() );
}

// Allocates a new SSConstellation and initializes it from a CSV-formatted string.
// Returns nullptr on error (invalid CSV string, heap allocation failure, etc.)

SSConstellation *SSConstellation::fromCSV ( string csv )
{
	vector<string> fields = split ( csv, "," );
	
	SSObjectType type = SSObject::codeToType ( fields[0] );
	if ( type < kTypeConstellation || type > kTypeAsterism || fields.size() < 8 )
		return nullptr;
	
	SSConstellationPtr pCon = new SSConstellation ( type );
	if ( pCon == nullptr )
		return pCon;
	
	double ra = degtorad ( strtofloat64 ( fields[1] ) * 15.0 );
	double dec = degtorad ( strtofloat64 ( fields[2] ) );
	SSVector center ( SSSpherical ( ra, dec, 1.0 ) );
	
	pCon->setDirection ( center );
	pCon->setArea ( degtorad ( degtorad ( strtofloat64 ( fields[3] ) ) ) );
	pCon->setRank ( strtoint ( fields[4] ) );
	pCon->setNames ( vector<string> ( &fields[5], &fields[8] ) );

	return pCon;
}

string SSConstellation::toCSV ( void )
{
	string csv = SSObject::typeToCode ( _type ) + ",";
	
	SSSpherical center ( _direction );
	double ra = radtodeg ( center.lon / 15.0 );
	double dec = radtodeg ( center.lat );
	double area = radtodeg ( radtodeg ( _area ) );
	
	csv += isinf ( ra ) ? "," : format ( "%.7f,", ra );
	csv += isinf ( dec ) ? "," : format ( "%+.6f,", dec );
	csv += isinf ( area ) ? "," : format ( "%.2f,", area );
	csv += _rank < 1 ? "," : format ( "%d,", _rank );
	
	for ( int i = 0; i < _names.size(); i++ )
		csv += _names[i] + ",";
	
	return csv;
}

// Reads constellation data from CSV-formatted text file.
// Imported data is appended to the input vector of SSObjects (constellations).
// Returns number of consteillations successfully imported (should be 88).

int SSImportConstellations ( const char *filename, SSObjectVec &constellations )
{
    // Open file; return on failure.

    ifstream file ( filename );
    if ( ! file )
        return ( 0 );

    // Read file line-by-line until we reach end-of-file

    string line = "";
    int numLines = 0, numCons = 0;

    while ( getline ( file, line ) )
    {
        numLines++;

		// Attempt to create constellation from CSV file line; continue on failure.
		
		SSConstellationPtr pCon = SSConstellation::fromCSV ( line );
		if ( pCon == nullptr )
			continue;
		
		// If successful, append new constellation to object vector.
		
		cout << pCon->toCSV() << endl;
        constellations.push_back ( shared_ptr<SSObject> ( pCon ) );
		numCons++;
	}
	
	return numCons;
}

// Reads constellation boundary data from CSV-formatted text file.
// Imported data is stored in each constellation in input vector of SSObjects (constellations).
// Assumes constellations in input vector are sorted by constellation name alphabetically,
// i.e. constellation[0] is Andromeda, [1] is Antlia, ... [87] is Vulpecula.
// Returns number of boundary vertices successfully imported (should be 1562).

int SSImportConstellationBoundaries ( const char *filename, SSObjectVec &constellations )
{
    // Open file; return on failure.

    ifstream file ( filename );
    if ( ! file )
        return ( 0 );

    // Read file line-by-line until we reach end-of-file

    string line = "", abbr = "", lastAbbr = "And";
    int numLines = 0, numVerts = 0;
	vector<SSVector> boundary ( 0 );
	SSConstellationPtr pCon = SSGetConstellationPtr ( constellations[0] );

    while ( getline ( file, line ) )
    {
        numLines++;

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
			// Store current boundary in current constellation
			
			if ( pCon != nullptr && boundary.size() > 0 )
			{
				pCon->setBoundary ( boundary );
				cout << "Imported " << boundary.size() << " vertices for " << lastAbbr << endl;
			}

			// Get pointer to new constellation, and start new boundary vertex.
			
			if ( index > 0 && index <= constellations.size() )
				pCon = SSGetConstellationPtr( constellations[index - 1] );

			boundary.clear();
		}
		
		// Extract vertex B1875 RA and Dec; skip if both are zero (no vertex at these coords).
		
		double ra = degtorad ( strtofloat64 ( fields[0] ) / 15.0 );
		double dec = degtorad ( strtofloat64 ( fields[1] ) );
		if ( ra == 0.0 && dec == 0.0 )
			continue;
		
		// Convert spherical coordinates to rectangular unit vector.
		// Append vertex to current boundary, increment vertex counter.
		
		SSVector vertex ( SSSpherical ( ra, dec, 1.0 ) );
		boundary.push_back ( vertex );

		lastAbbr = abbr;	
		numVerts++;
	}
	
	// After we reach end of file, add final boundary to last constellation.
	
	if ( pCon != nullptr && boundary.size() > 0 )
	{
		pCon->setBoundary ( boundary );
		cout << "Imported " << boundary.size() << " vertices for " << lastAbbr << endl;
	}

	return numVerts;
}

// Reads constellation shape data from CSV-formatted text file.
// Imported data is stored in input vector of SSConstellations (constellations).
// Assumes constellations in input vector are sorted by constellation name alphabetically,
// i.e. constellation[0] is Andromeda, [1] is Antlia, ... [87] is Vulpecula.
// Returns number of shape lines successfully imported (should be 765).

int SSImportConstellationShapes ( const char *filename, SSObjectVec &constellations )
{
    // Open file; return on failure.

    ifstream file ( filename );
    if ( ! file )
        return ( 0 );

    // Read file line-by-line until we reach end-of-file

    string line = "", abbr = "", lastAbbr = "And";
    int numLines = 0;
	vector<int> shape ( 0 );
	SSConstellationPtr pCon = SSGetConstellationPtr ( constellations[0] );

    while ( getline ( file, line ) )
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
				cout << "Imported " << shape.size() / 2 << " shape lines for " << lastAbbr << endl;
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
		cout << "Imported " << shape.size() / 2 << " shape lines for " << lastAbbr << endl;
	}

	return numLines;
}
