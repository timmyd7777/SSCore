// SSConstellation.cpp
// SSCore
//
// Created by Tim DeBenedictis on 3/25/20.
// Copyright © 2020 Southern Stars. All rights reserved.

#include <iostream>
#include <fstream>

#include "SSConstellation.hpp"

SSConstellation::SSConstellation ( SSObjectType type ) : SSObject ( type )
{
	_area = 0.0;
	_rank = 0;
	_bounds = vector<SSVector> ( 0 );
	_figures = vector<int> ( 0 );
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
