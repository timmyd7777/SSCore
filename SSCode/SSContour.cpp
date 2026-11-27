//  SSContour.cpp
//  SSCore
//
//  Created by Tim DeBenedictis on 11/27/26.
//  Copyright © 2026 Southern Stars. All rights reserved.

#include <algorithm>
#include <cmath>
#include <fstream>

#include "SSContour.hpp"
#include "SSUtilities.hpp"

// Computes the center (center) and angular radius (radius) of a circle which encloses all of a contour's points.
// The center is the normalized midpoint of the points' bounding box in rectangular coordinates; the radius is
// the angular distance from there to the farthest point. This is fast (two passes over the points), but not
// the smallest enclosing circle: on the imported nebula contours, it averages 4% larger, and at most 14% larger.
// A contour with no points gets a zero center and radius. Fails for points spread over most of the sphere,
// whose bounding box midpoint may be near the origin; nebula contours are much smaller than that.

static void SSComputeContourBounds ( SSContour &contour )
{
    contour.center = SSVector ( 0.0, 0.0, 0.0 );
    contour.radius = 0.0;
    if ( contour.points.empty() )
        return;

    // Find the bounding box of all points, and normalize its midpoint to get the center.

    SSVector vmin = contour.points[0], vmax = contour.points[0];
    for ( SSVector &p : contour.points )
    {
        vmin = SSVector ( min ( vmin.x, p.x ), min ( vmin.y, p.y ), min ( vmin.z, p.z ) );
        vmax = SSVector ( max ( vmax.x, p.x ), max ( vmax.y, p.y ), max ( vmax.z, p.z ) );
    }

    contour.center = vmin.add ( vmax ).normalize();

    // Find the largest straight-line (chord) distance from the center to any point,
    // then convert that to an angle once, rather than computing an angle for every point.

    double maxChord = 0.0;
    for ( SSVector &p : contour.points )
        maxChord = max ( maxChord, contour.center.subtract ( p ).magnitude() );

    contour.radius = 2.0 * asin ( min ( maxChord / 2.0, 1.0 ) );
}

// Imports deep sky object contours (outlines of bright and dark nebulae) from two CSV files:
// The contour file (contourPath) and the index file (indexPath).
// Imported contours are appended to the vector (contours), in the order they appear in the contour file.
// Returns the number of contours imported, or zero if the contour file cannot be read.
// If the index file cannot be read, contours are still imported, but without identifiers.
// Each imported contour's enclosing circle (center, radius) is computed from its points.

int SSImportContours ( const string &contourPath, const string &indexPath, SSContourVec &contours )
{
    // Open contour file; return on failure. Skip its header line.

    ifstream contourFile ( contourPath );
    if ( ! contourFile )
        return ( 0 );

    string line = "";
    getline ( contourFile, line );

    // Read outline points. Start a new contour whenever the contour number changes, and remember
    // where each contour number's contour is in the output vector, to attach identifiers to it below.

    map<int,size_t> contourIndex;
    size_t first = contours.size();
    int lastNum = 0;

    while ( getline ( contourFile, line ) )
    {
        // Require 3 fields per line, and a positive contour number; skip line if we don't have them.
        
        vector<string> fields = split ( line, "," );
        if ( fields.size() < 3 )
            continue;

        int num = strtoint ( fields[2] );
        if ( num < 1 )
            continue;

        if ( num != lastNum )
        {
            contourIndex[num] = contours.size();
            contours.push_back ( SSContour() );
            lastNum = num;
        }

        // Convert R.A. and Dec. from degrees to a unit vector, and add it to the current contour.

        SSAngle ra = SSAngle::fromDegrees ( strtofloat64 ( fields[0] ) );
        SSAngle dec = SSAngle::fromDegrees ( strtofloat64 ( fields[1] ) );
        contours.back().points.push_back ( SSVector ( SSSpherical ( ra, dec ) ) );
    }

    // Compute the enclosing circle of each newly imported contour, now that all its points are read.

    for ( size_t i = first; i < contours.size(); i++ )
        SSComputeContourBounds ( contours[i] );

    // Open index file; if we can't, return contours without identifiers. Skip its header line.

    int numContours = (int) ( contours.size() - first );
    ifstream indexFile ( indexPath );
    if ( ! indexFile )
        return numContours;

    getline ( indexFile, line );

    // Read each contour's identifiers, and attach them to the contour with that number:
    // those we can parse as identifiers, and those we cannot parse as names.
    
    while ( getline ( indexFile, line ) )
    {
        vector<string> fields = split ( line, "," );
        if ( fields.size() < 2 )
            continue;

        auto it = contourIndex.find ( strtoint ( fields[0] ) );
        if ( it == contourIndex.end() )
            continue;

        for ( string str : split ( fields[1], ";" ) )
        {
            // Convert Sharpless identifiers to the form SSIdentifier expects.
            
            str = trim ( str );
            if ( str.compare ( 0, 12, "Sharpless 2-" ) == 0 )
                str = "Sh2 " + str.substr ( 12 );

            SSIdentifier ident = SSIdentifier::fromString ( str, kTypeNonexistent, true );
            if ( ident )
                contours[ it->second ].idents.push_back ( ident );
            else if ( ! str.empty() )
                contours[ it->second ].names.push_back ( str );
        }
    }

    return numContours;
}

// Makes a map from the identifiers in a vector of nebula contours (contours)
// to the indices of those contours in the vector (counting from zero). 
// One identifier may map to several contours, and one contour may be mapped from several identifiers.
// Contours with no identifiers are not mapped. Returns the map; see SSContourMap.

SSContourMap SSMakeContourMap ( SSContourVec &contours )
{
    SSContourMap contourMap;
    for ( int i = 0; i < contours.size(); i++ )
        for ( SSIdentifier ident : contours[i].idents )
            contourMap.insert ( { ident, i } );
    
    return contourMap;
}
