//  Tetra3.hpp
//  Tetra
//
//  Created by Tim DeBenedictis on 5/30/23.

#ifndef TETRA3_HPP
#define TETRA3_HPP

#include <iostream>
#include <fstream>
#include <vector>
#include <set>
#include <algorithm>
#include <iterator>
#include <cmath>

#include "SSMatrix.hpp"
#include "cnpy.h"

typedef std::vector<int> T3HashCode;

#pragma pack ( push, 1 )

// Defines a catalog star in the database, used in a pattern, or for verification.
// Commented-out fields are not used in the solver, only for database generation.

struct T3Star
{
    float xyz[3];     // (ra,dec) converted to rectangular (cartesian) coordinates
    
    T3Star ( void )
    {
        xyz[0] = xyz[1] = xyz[2] = 0.0;
    }
    
    T3Star ( const SSVector &v )
    {
        xyz[0] = v.x; xyz[1] = v.y; xyz[2] = v.z;
    }
    
    T3Star ( double ra, double dec ) : T3Star ( SSVector ( SSSpherical ( ra, dec ) ) ) { }
};

// Defines a pattern of four stars in a star catalog.

struct T3Pattern
{
    uint32_t stars[4];      // zero-based indices of four stars that make up this pattern
    float   largest_edge;   // largest edge angle in patters in radians
    
    T3Pattern ( void )
    {
        stars[0] = stars[1] = stars[2] = stars[3] = 0;
        largest_edge = 0.0;
    }
    
    T3Pattern ( int i0, int i1, int i2, int i3 ) : T3Pattern()
    {
        stars[0] = i0;
        stars[1] = i1;
        stars[2] = i2;
        stars[3] = i3;
        largest_edge = 0.0;
    }

    T3Pattern ( int indices[4] ) : T3Pattern()
    {
        for ( int i = 0; i < 4; i++ )
            stars[i] = indices[i];
        largest_edge = 0.0;
    }
    
    bool empty ( void ) {
        return stars[0] == 0 && stars[1] == 0 && stars[2] == 0 && stars[3] == 0;
    }
    
    bool contains ( int index ) { return stars[0] == index || stars[1] == index || stars[2] == index || stars[3] == index; }
    bool equals ( const T3Pattern &p ) { return contains ( p.stars[0] ) && contains ( p.stars[1] ) && contains ( p.stars[2] ) && contains ( p.stars[3] ); }

    void print ( void ) { printf ( "%9u %9u %9u %9u\n", stars[0], stars[1], stars[2], stars[3] ); }
};

#pragma pack ( pop )

// Contains geometric information about a patter of four stars.
// Can be known stars in a star catalog, or sources found in an image.

struct T3PatternVectors
{
    std::vector<SSVector> vectors;    // has 4 elements, vectors to stars making the pattern
    std::vector<double> edge_angles;  // has 6 elements, angular distances between stars in patter, sorted smallest to largest, in radians
    std::vector<double> edge_ratios;  // has 5 elements, ratios of edges to largest edge

    T3PatternVectors ( void )
    {
        vectors = std::vector<SSVector> ( 4 );
        edge_angles = std::vector<double> ( 6 );
        edge_ratios = std::vector<double> ( 5 );
    }

    double largestEdge ( void ) { return edge_angles.back(); }

    void computeEdgeRatios ( void )
    {
        int edge = 0;
        for ( int i = 0; i < vectors.size(); i++ )
            for ( int j = i + 1; j < vectors.size(); j++ )
                edge_angles[edge++] = vectors[i].angularSeparation ( vectors[j] );
        
        std::sort ( edge_angles.begin(), edge_angles.end() );
        for ( int i = 0; i < edge_ratios.size(); i++ )
            edge_ratios[i] = edge_angles[i] / edge_angles.back();
    }
};

// Contains centroid information for a source found in an image.

struct T3Source
{
    float x;                // centroid horizontal (x) coordinate in image
    float y;                // centroid vertical (y) coordinate in image
    
    T3Source ( void ) { x = y = 0.0; }
    T3Source ( float xf, float yf ) { x = xf; y = yf; }
    
    float distance ( const T3Source &s ) { return hypot ( s.x - x, s.y - y ); }
    SSVector project ( float fov, float width, float height ) const;
    static T3Source deproject ( const SSVector &v, float fov, float width, float height );
};

// Arguments to Tetra3::solveFromSources() method.

struct T3Options
{
    float fov_estimate;             // Estimated field of view of the image in degrees, or zero if unknown.
    float fov_max_error;            // Maximum difference in field of view from the estimate allowed for a match in degrees; zero if unknown.
    float pattern_max_error;        // Maximum difference allowed in pattern for a match. If zero, use database pattern_max_error.
    int pattern_checking_stars;     // Number of sources used to create possible patterns to look up in database.
    float match_radius;             // Maximum distance to a star to be considered a match as a fraction of the image field of view.
    float match_threshold;          // Maximum allowed mismatch probability to consider a tested pattern a valid match.
    uint8_t num_threads;            // Number of parallel threads to run; if zero, run synchronously on current thread.
};

// Results of an attempt to solve a set of sources.
// If unsuccessful in finding a match, zero is returned for all fields of this
// struct except prob, t_solve, and t_extract

struct T3Results
{
    float ra = 0.0f;          // Right ascension of centre of image in degrees.
    float dec = 0.0f;         // Declination of centre of image in degrees.
    float roll = 0.0f;        // Rotation of image relative to north celestial pole, negative if image is flipped/inverted.
    float fov = 0.0f;         // Calculated field of view width of the provided image in degrees
    float rmse = 0.0f;        // RMS residual of matched stars in arcseconds.
    int matches = 0;          // Number of stars in the image matched to the database.
    float prob = 1.0;         // Probability that the solution is a mismatch.
    float t_solve = 0.0f;     // Time spent searching for a match in milliseconds.
    float t_extract = 0.0f;   // Time spent extracting star centroids in milliseconds.
    SSMatrix rmat;            // Best-fit rotation matrix for transforming from image frame to RA/Dec frame. Determinant is -1 if image is flipped/inverted.
    
    // Converts (x,y) in image of dimensions (width, height) to (ra,dec) in radians, and vice-versa.
    // T3Results must be initialized; functions return true if successful or false on failure.
    
    bool imageXYtoRADec ( float x, float y, float width, float height, double &ra, double &dec );
    bool raDectoImageXY ( double ra, double dec, float width, float height, float &x, float &y );
};

// Contains a Tetra3 database of patterns and stars, and associated metadata.
// Patterns can be loaded into RAM (faster), or read on-demand from file (slower).
// Currently multi-threaded execution only works when the database is loaded into RAM!

struct T3Database
{
private:
    static constexpr unsigned int _MAGIC_RAND = 2654435761;
    static constexpr unsigned int _PATTERN_MULT = 2;
    
    FILE *fp = NULL;                    // pointer to pattern file, NULL if patterns are loaded into RAM.
    std::vector<T3Star> stars;          // vector of stars
    std::vector<T3Pattern> patterns;    // vector of patterns loaded into RAM, empty if fp is valid.
    std::vector<uint32_t> patindex;     // 1-based index to valid patterns in patvec; zeros indicate empty patterns
    size_t pattern_offset = 0;          // offset to first pattern in pattern file, in bytes from start of file
    uint32_t npatterns = 0;             // number of patterns in database
    uint32_t nstars = 0;                // number of stars in database
    bool loaded = false;                // true when database has been completely and successfully loaded.
    
public:

    std::string pattern_mode = "";      // Method used to identify star patterns.
    int pattern_size = 0;               // Number of stars in each pattern.
    int pattern_bins = 0;               // Number of bins per dimension in pattern catalog.
    float pattern_max_error = 0.0;      // Maximum difference allowed in pattern for a match. Default .005.
    float max_fov = 0.0;                // Maximum angle (in degrees) between stars in the same pattern.
    float min_fov = 0.0;                // Minimum FOV considered when the catalogue density is trimmed to size.
    std::string star_catalog = "";      // Name of star catalog used to generate the database.
    int pattern_stars_per_fov = 0;      // Number of stars used for pattern matching in each region of size 'max_fov'. Default 10.
    int verification_stars_per_fov = 0; // Number of stars used for verification of the solution in each region of size 'max_fov'. Default 30.
    float star_max_magnitude = 0.0;     // Dimmest apparent magnitude of stars in database. Default 7.
    bool simplify_pattern = false;      // If set to true, the patterns generated have maximum
                                        // size of FOV/2 from the centre star, and will be generated much faster.
                                        // If set to false (the default) the maximum separation of all stars in the pattern is FOV.
    float range_ra[2] = { 0.0 };        // only stars within the given right ascension range (min_ra, max_ra) in degrees (0 to 360) will be kept in the database.
    float range_dec[2] = { 0.0 };       // only stars within the give declination range (min_dec, max_dec) in degrees (-90 to 90)will be kept in the database.

    T3Database ( void ) { };
    ~T3Database ( void ) { if ( fp ) fclose ( fp ); }
    
    void newPatterns ( size_t max_patterns ) { patindex = std::vector<uint32_t> ( max_patterns * _PATTERN_MULT ); }
    void addPattern ( const T3Pattern &p );
    void addStar ( const T3Star &s ) { stars.push_back ( s ); }
    
    size_t numPatterns ( void );
    size_t numStars ( void ) { return stars.size(); }
    
    T3Pattern getPattern ( size_t i );
    T3Star getStar ( size_t i ) { return stars[i]; }
    uint32_t keyToIndex(std::vector<int> key, uint32_t bin_factor);
    std::vector<T3Pattern> getAtIndex ( uint32_t index );
    void insertAtIndex ( const T3Pattern &p, uint32_t index );

    bool loadFromNumPy ( const std::string &path );
    void optimize ( void );
    bool loadOptimized ( const std::string &path, bool loadPatterns = false );
    bool saveOptimized ( const std::string &path );
    bool isLoaded ( void ) { return loaded; }
    
    T3PatternVectors getStarPatternVectors ( const T3Pattern &pattern );
    size_t getStarPatternVectors ( const std::vector<T3Pattern> &patterns, std::vector<T3PatternVectors> &pattern_vectors );
};

// The main Tetra3 class which contains routines for loading the database
// and solving an image from a set of sources found in it.

class Tetra3
{
private:
    T3Database db;                  // The associated pattern and star database.
        
    std::vector<SSVector> computeVectors ( const std::vector<T3Source> &sources, float fov, float width, float height );
    std::vector<T3Pattern> generatePatternsFromCentroids ( const std::vector<T3Source> &sources, int pattern_size );
    SSMatrix findRotationMatrix ( const std::vector<SSVector> &image_vectors, const std::vector<SSVector> &catalog_vectors );
    std::vector<SSVector> getNearbyStarVectors ( const SSVector &vector, double radius, int max_stars );

public:
    
    Tetra3 ( void ) { };
    
    size_t numPatterns ( void ) { return db.numPatterns(); }
    size_t numStars ( void ) { return db.numStars(); }
    
    bool loadDatabase ( const std::string &path ) { return db.loadFromNumPy ( path ); }
    bool loadOptimizedDatabase ( const std::string &path, bool loadPatterns = false ) { return db.loadOptimized ( path, loadPatterns ); }
    bool saveOptimizedDatabase ( const std::string &path ) { return db.saveOptimized ( path ); }
    bool databaseLoaded ( void ) { return db.isLoaded(); }
    
    bool solveFromSources ( const std::vector<T3Source> &sources, float width, float height, const T3Options &options, T3Results &results );
};

#endif // TETRA3_HPP
