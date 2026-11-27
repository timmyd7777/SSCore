// SSContour.hpp
// SSCore
//
// Created by Tim DeBenedictis on 11/27/26.
//
// Copyright © 2026 Southern Stars. All rights reserved.
//
// Contours are outlines of bright and dark nebulae on the celestial sphere,
// with functions to import them from CSV files and find them from deep sky object identifiers.

#ifndef SSContour_hpp
#define SSContour_hpp

#include <map>
#include <string>
#include <vector>

#include "SSIdentifier.hpp"
#include "SSVector.hpp"

using namespace std;

// Represents the outline of a bright or dark nebula on the celestial sphere as a closed polygon.
// One object may be outlined by several contours, and one contour may outline several objects.
// Contours are imported from CSV files with SSImportContours(), which also computes each one's enclosing circle.

struct SSContour
{
    vector<SSVector> points;        // unit vectors to outline points, in the fundamental (J2000 mean equatorial) frame
    vector<SSIdentifier> idents;    // identifiers of the deep sky objects this contour represents; may be empty
    vector<string> names;           // names of outlined objects which could not be parsed as identifiers, e.g. "Coalsack"
    SSVector center;                // unit vector to center of circle enclosing all points, in the fundamental frame
    double radius = 0.0;            // angular radius of circle around center which encloses all points, in radians
};

typedef SSContour *SSContourPtr;
typedef vector<SSContour> SSContourVec;
typedef multimap<SSIdentifier,int> SSContourMap;

int SSImportContours ( const string &contourPath, const string &indexPath, SSContourVec &contours );
SSContourMap SSMakeContourMap ( SSContourVec &contours );

#endif /* SSContour_hpp */
