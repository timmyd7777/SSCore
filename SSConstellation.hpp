// SSConstellation.hpp
// SSCore
//
// Created by Tim DeBenedictis on 3/25/20.
// Copyright © 2020 Southern Stars. All rights reserved.
// This subclass of SSObject stores information for constellations and asterisms.

#ifndef SSConstellation_hpp
#define SSConstellation_hpp

#include "SSObject.hpp"

class SSConstellation : public SSObject
{
protected:
    
    double _area;                // square radians
    int _rank;                   // by area, largest-to-smallest
    
    vector<SSVector> _bounds;    // unit vectors to boundary vertices in fundamental (J2000 mean equatorial) frame
    vector<int> _figures;        // HR numbers of stars making up figure.
    
public:
    
    // constructor
    
    SSConstellation ( SSObjectType type );
    
    // accessors
    
    double getArea ( void ) { return _area; }
    int getRank ( void ) { return _rank; }
    vector<SSVector> getBoundary ( void ) { return _bounds; }
    vector<int> getFigure ( void ) { return _figures; }

    // modifiers
    
    void setArea ( double area ) { _area = area; }
    void setRank ( int rank ) { _rank = rank; }
    void setBoundary ( vector<SSVector> bounds ) { _bounds = bounds; }
    void setFigure ( vector<int> figure ) { _figures = figure; }

    // converts IAU abbreviation ("And", "Ant", ... "Vul") to index number (1, 2, ... 88) and vice-versa.
    
    static int abbreviationToIndex ( string abbrev );
    static string indexToAbbreviation ( int index );
    
    // imports/exports from/to CSV-format text string
    
    static SSObjectPtr fromCSV ( string csv );
    string toCSV ( void );
};

// convenient alias for pointer to SSConstellation

typedef SSConstellation *SSConstellationPtr;

// Downcasts a pointer from SSObject base class to SSConstellation subclass.
// Returns nullptr if input object pointer is not actually an SSConstellation.

SSConstellationPtr SSGetConstellationPtr ( SSObjectPtr ptr );

// Imports constellations, boundaries, shapes from CSV-format text files into vector of SSObjectPtr.

int SSImportConstellations ( const string &filename, SSObjectVec &constellations );
int SSImportConstellationBoundaries ( const string &filename, SSObjectVec &constellations );
int SSImportConstellationShapes ( const string &filename, SSObjectVec &constellations );

#endif /* SSConstellation_hpp */
