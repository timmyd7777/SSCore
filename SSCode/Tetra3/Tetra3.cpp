//  Tetra3.cpp
//  Tetra
//
//  Created by Tim DeBenedictis on 5/30/23.

#include <math.h>
#include <string.h>
#include <chrono>
#include <thread>
#include <mutex>
#include <sys/stat.h>

#include "Tetra3.hpp"
#include "svdcmp.h"
#include "cnpy.h"

// Calculates the binomial cumulative distribution function (CDF) using the formula:
// CDF(k; n, p) = sum(coef * p^i * q^(n-i)) for i = 0 to k
// where k is the number of successes, n is the number of trials, p is the probability of success,
// and q = 1 - p is the probability of failure.
// This implementation assumes k, n, and p are non-negative values.
// Confirmed working successfully 2023-06-05 TCD

double binomialCDF (int k, int n, double p)
{
    if (k < 0) {
        return 0.0;
    }
    if (k >= n) {
        return 1.0;
    }
    double q = 1.0 - p;
    double sum = 0.0;
    for (int i = 0; i <= k; ++i) {
        double coef = std::exp(std::lgamma(n + 1) - std::lgamma(i + 1) - std::lgamma(n - i + 1));
        double term = coef * std::pow(p, i) * std::pow(q, n - i);
        sum += term;
    }
    return sum;
}

void print ( const SSVector vec )
{
    printf ( "%11.8f %11.8f %11.8f\n", vec.x, vec.y, vec.z );
}

void print ( const std::vector<double> vec )
{
    for ( const double &d : vec )
        printf ( "%11.8f ", d );
    printf ( "\n" );
}

void print ( const T3HashCode &code )
{
    for ( const int &i : code )
        printf ( "%3d ", i );
    printf ( "\n" );
}

void print ( const std::vector<SSVector> &vectors )
{
    for ( const SSVector &vec : vectors )
        print ( vec );
    printf ( "\n" );
}

SSVector mean ( const std::vector<SSVector> &vectors )
{
    SSVector sum;
    
    for ( const SSVector &vec : vectors )
        sum = sum.add ( vec );
    
    return sum / (double) vectors.size();
}


void indexDistanceFromCenter ( const std::vector<SSVector> &vectors, std::vector<size_t> &indices )
{
    // find the centroid, or average position, of the star vectors
    SSVector centroid = mean ( vectors );

    std::vector<double> radii;
    for ( SSVector vector : vectors )
        radii.push_back ( vector.distance ( centroid ) );

    // use the radii to uniquely order the pattern's star vectors so they can be
    // matched with the catalog vectors

    indices = std::vector<size_t> ( vectors.size() );
    std::iota ( indices.begin(), indices.end(), 0 );
    std::sort ( indices.begin(), indices.end(),
              [&radii](size_t i, size_t j) { return radii[i] < radii[j]; } );
}

void sortByDistanceFromCenter ( const std::vector<SSVector> &vectors, std::vector<SSVector> &sorted_vectors )
{
    std::vector<size_t> indices;
    indexDistanceFromCenter ( vectors, indices );
    
    sorted_vectors = std::vector<SSVector> ( vectors.size() );
    for (size_t i = 0; i < vectors.size(); ++i)
        sorted_vectors[i] = vectors[indices[i]];
}

// Sort star indices in order of increasing distance from centroid of pattern.

void sortByDistanceFromCenter ( T3Pattern &p, const T3PatternVectors &pv )
{
    std::vector<size_t> indices;
    indexDistanceFromCenter ( pv.vectors, indices );
    std::vector<uint32_t> stars = std::vector<uint32_t> ( &p.stars[0], &p.stars[4] );
    for ( size_t i = 0; i < stars.size(); ++i )
        p.stars[i] = stars[indices[i]];
}

std::vector<T3HashCode> generate_hash_codes ( const std::vector<std::vector<int>> &hash_space )
{
    std::vector<T3HashCode> hash_codes;

    for ( int i0 = 0; i0 < hash_space[0].size() - 1; i0++ )
        for ( int i1 = 0; i1 < hash_space[1].size() - 1; i1++ )
            for ( int i2 = 0; i2 < hash_space[2].size() - 1; i2++ )
                for ( int i3 = 0; i3 < hash_space[3].size() - 1; i3++ )
                    for ( int i4 = 0; i4 < hash_space[4].size() - 1; i4++ )
                    {
                        T3HashCode code;
                        code.push_back ( hash_space[0][i0] );
                        code.push_back ( hash_space[1][i1] );
                        code.push_back ( hash_space[2][i2] );
                        code.push_back ( hash_space[3][i3] );
                        code.push_back ( hash_space[4][i4] );
                        hash_codes.push_back ( code );
                    }
    
    return hash_codes;
}

// Projects source (x,y) as 3D unit vector (X,Y,Z) on unit sphere
// tangent to image plane at X axis; right on image (+x) is -Y;
// down on image (+y) is -Z. Image angular width (fov) in radians;
// image dimensions (width,height) in pixels.

SSVector T3Source::project ( float fov, float width, float height ) const
{
    float scale_factor = tan ( fov / 2.0 ) / ( width / 2.0 );
    float img_center[2] = { width / 2.0f, height / 2.0f };
    SSVector v = { 1.0, 1.0, 1.0 };
    v.y = (img_center[0] - x) * scale_factor;
    v.z = (img_center[1] - y) * scale_factor;
    return v.normalize();
}

// Performs the inverse transformation. Projects 3D unit vector (X,Y,Z) on unit sphere
// tangent to image plane at X axis to (x,y) in 2D image plane. Image angular width (fov)
// in radians; image dimensions (width,height) in pixels.

T3Source T3Source::deproject ( const SSVector &v, float fov, float width, float height )
{
    float scale_factor = tan ( fov / 2.0 ) / ( width / 2.0 );
    float img_center[2] = { width / 2.0f, height / 2.0f };
    float x = img_center[0] - ( v.y / v.x ) / scale_factor;
    float y = img_center[1] - ( v.z / v.x ) / scale_factor;
    return T3Source ( x, y );
}

bool T3Results::imageXYtoRADec ( float x, float y, float width, float height, double &ra, double &dec )
{
    if ( fov == 0.0 )
        return false;
    
    SSVector v = T3Source ( x, y ).project ( degtorad ( fov ), width, height );
    SSSpherical sph ( rmat * v );
    ra = sph.lon;
    dec = sph.lat;
    return true;
}

bool T3Results::raDectoImageXY ( double ra, double dec, float width, float height, float &x, float &y )
{
    if ( fov == 0.0 )
        return false;
    
    SSVector v ( SSSpherical ( ra, dec ) );
    v = rmat.transpose() * v;
    T3Source s = T3Source::deproject ( v, degtorad ( fov ), width, height );
    x = s.x; y = s.y;
    return true;
}

void T3Database::addPattern ( const T3Pattern &pat )
{
    // retrieve the vectors of the stars in the pattern
    T3Pattern p = pat;
    T3PatternVectors pv = getStarPatternVectors ( p );
    pv.computeEdgeRatios();
    p.largest_edge = pv.largestEdge();
    sortByDistanceFromCenter ( p, pv );
    
    // convert edge ratio float to hash code by binning
    
    std::vector<int> hash_code;
    for (size_t i = 0; i < pv.edge_ratios.size(); ++i) {
        int bin = pv.edge_ratios[i] * pattern_bins;
        hash_code.push_back ( bin );
    }
    
    uint32_t hash_index = keyToIndex( hash_code, pattern_bins );
    insertAtIndex ( p, hash_index );
}

size_t T3Database::numPatterns ( void )
{
    if ( fp == NULL )
        return patterns.size();
    else
        return npatterns;
}

T3Pattern T3Database::getPattern ( size_t i )
{
    i = patindex[i];
    if ( i == 0 )
        return T3Pattern();
    i--;
    
    if ( fp == NULL )
        return patterns[i];
    
    T3Pattern pattern;
    if ( fseek ( fp, pattern_offset + sizeof ( T3Pattern ) * i, SEEK_SET ) == 0
      && fread ( &pattern, sizeof ( pattern ), 1, fp ) == 1 )
        return pattern;
    else
        return T3Pattern();
}

// Inserts to pattern table with quadratic probing.

void T3Database::insertAtIndex ( const T3Pattern &p, uint32_t index )
{
    size_t max_ind = patindex.size();
    for (unsigned int c = 0;; ++c) {
        unsigned int i = (index + c*c) % max_ind;
        T3Pattern pattern = getPattern ( i );
        if ( pattern.empty() ) { // if the current slot is empty, add the pattern
            patterns.push_back ( p );
            patindex[i] = patterns.size();
            return;
        }
    }
}

// Gets from pattern table with quadratic probing, returns list of all matches.

std::vector<T3Pattern> T3Database::getAtIndex ( uint32_t index )
{
    size_t max_ind = patindex.size();
    std::vector<T3Pattern> found;
    for (unsigned int c = 0;; ++c) {
        unsigned int i = (index + c*c) % max_ind;
        T3Pattern pattern = getPattern ( i );
        if ( pattern.empty() ) {
            return found;
        }
        else {
            found.push_back ( pattern );
        }
    }
}

T3PatternVectors T3Database::getStarPatternVectors ( const T3Pattern &p )
{
    T3PatternVectors pv;
    for ( int i = 0; i < 4; i++ )
    {
        uint32_t index = p.stars[i];
        T3Star star = getStar ( index );
        pv.vectors[i] = SSVector ( star.xyz[0], star.xyz[1], star.xyz[2] ); // T3Vector ( star.ra, star.dec );
    }
    return pv;
}

size_t T3Database::getStarPatternVectors ( const std::vector<T3Pattern> &patterns, std::vector<T3PatternVectors> &star_vectors )
{
     star_vectors = std::vector<T3PatternVectors> ( patterns.size() );
    
    for ( const T3Pattern &p : patterns )
        star_vectors.push_back ( getStarPatternVectors ( p ) );
    
    return star_vectors.size();
}

// Loads tetra3 database in Python .npz format, generated by Python version of tetra3.
// Returns true if successful or false on failure

bool T3Database::loadFromNumPy ( const std::string &path )
{
    // load the entire npz file

    cnpy::npz_t database;
    try { database = cnpy::npz_load(path); } catch ( ... ) { return false; }
    if ( database.size() < 3 )
        return false;
    
    // Get the pattern_catalog array and make sure it's got 4 columns of integer data
    
    cnpy::NpyArray &pattern_catalog = database["pattern_catalog"];
    if ( pattern_catalog.shape.size() < 2 || pattern_catalog.shape[1] != 4 || pattern_catalog.type != 'u')
        return false;
    
    // Get the star_table array and make sure it's got 6 columns of 32-bit floating point data

    cnpy::NpyArray &star_table = database["star_table"];
    if ( star_table.shape.size() < 2 || star_table.shape[1] != 6 || star_table.type != 'f'  || star_table.word_size != 4 )
        return false;

    // Allocate vectors to store the pattern catalog and star table
    
    patindex = std::vector<uint32_t> ( pattern_catalog.shape[0] );
    stars = std::vector<T3Star> ( star_table.shape[0] );
    
    // Copy data from pattern_catalog array into patterns vector
    
    uint8_t *pattern_data = pattern_catalog.data<uint8_t>();
    for ( int i = 0; i < pattern_catalog.shape[0]; i++ )
    {
        T3Pattern p;
        for ( int j = 0; j < 4; j++ )
        {
            if ( pattern_catalog.word_size == 1 )
                p.stars[j] = ((uint8_t *)pattern_data)[j];
            else if ( pattern_catalog.word_size == 2 )
                p.stars[j] = ((uint16_t *)pattern_data)[j];
            else if ( pattern_catalog.word_size == 4 )
                p.stars[j] = ((uint32_t *)pattern_data)[j];
        }
        
        if ( p.empty() )
            patindex[i] = 0;
        else {
            patterns.push_back ( p );
            patindex[i] = patterns.size();
        }
        
        pattern_data += 4 * pattern_catalog.word_size;
    }
    
    // Copy data from star_table array into stars vector

    float *star_data = star_table.data<float>();
    for ( int i = 0; i < star_table.shape[0]; i++ )
    {
        T3Star &s = stars[i];
        //s.ra = star_data[0];
        //s.dec = star_data[1];
        memcpy ( s.xyz, star_data + 2, sizeof (float) * 3 );
        //s.mag = star_data[5];
        star_data += 6;
    }

    // Read database properties from the props_packed table.
    // This depends on the binary layout of the table fields,
    // so make sure the field definitions agree with what we expect.
    
    cnpy::NpyArray props = database["props_packed"];
    const std::vector<cnpy::NpyField> &fields = props.fields;
    if ( fields.size() != 13 )
        return false;
    if ( fields[0].name != "pattern_mode" || fields[0].size != 4 || fields[0].type != 'u' )
        return false;
    if ( fields[1].name != "pattern_size" || fields[1].size != 2 || fields[1].type != 'u' )
        return false;
    if ( fields[2].name != "pattern_bins" || fields[2].size != 2 || fields[2].type != 'u' )
        return false;
    if ( fields[3].name != "pattern_max_error" || fields[3].size != 4 || fields[3].type != 'f' )
        return false;
    if ( fields[4].name != "max_fov" || fields[4].size != 4 || fields[4].type != 'f' )
        return false;
    if ( fields[5].name != "min_fov" || fields[5].size != 4 || fields[5].type != 'f' )
        return false;
    if ( fields[6].name != "star_catalog" || fields[6].size != 4 || fields[6].type != 'u' || fields[6].count < 64 )
        return false;
    if ( fields[7].name != "pattern_stars_per_fov" || fields[7].size != 2 || fields[7].type != 'u' )
        return false;
    if ( fields[8].name != "verification_stars_per_fov" || fields[8].size != 2 || fields[8].type != 'u' )
        return false;
    if ( fields[9].name != "star_max_magnitude" || fields[9].size != 4 || fields[9].type != 'f' )
        return false;
    if ( fields[10].name != "simplify_pattern" || fields[10].size != 1 || fields[10].type != 'b' )
        return false;
    if ( fields[11].name != "range_ra" || fields[11].size != 4 || fields[11].type != 'f' || fields[11].count < 2 )
        return false;
    if ( fields[12].name != "range_dec" || fields[12].size != 4 || fields[12].type != 'f' || fields[12].count < 2 )
        return false;

    char *data = props.data<char>();
    
    pattern_mode = props.read_string_field ( data );
    pattern_size = props.read_uint16_field ( data );
    pattern_bins = props.read_uint16_field ( data );
    pattern_max_error = props.read_float32_field ( data );
    max_fov = props.read_float32_field ( data );
    min_fov = props.read_float32_field ( data );
    star_catalog = props.read_string_field ( data );
    pattern_stars_per_fov = props.read_uint16_field ( data );
    verification_stars_per_fov = props.read_uint16_field ( data );
    star_max_magnitude = props.read_float32_field ( data );
    simplify_pattern = props.read_bool_field ( data );
    range_ra[0] = props.read_float32_field ( data );
    range_ra[1] = props.read_float32_field ( data );
    range_dec[0] = props.read_float32_field ( data );
    range_dec[1] = props.read_float32_field ( data );
    
    optimize();
    loaded = true;
    return true;
}

// Optimizes database loaded from python tetra NumPy .npz format.
// Saves pattern largest edge angle (in radians) in pattern,
// and sorts pattern star indices by their distance from the pattern centroid,
// so the solver does not have to do this.

void T3Database::optimize ( void )
{
    for ( T3Pattern &p : patterns )
    {
        if ( p.empty() )
            continue;
        
        // Save largest edge angle (in radians) with each pattern.
        
        T3PatternVectors pv = getStarPatternVectors ( p );
        pv.computeEdgeRatios();
        p.largest_edge = pv.largestEdge();
        sortByDistanceFromCenter( p, pv );
    }
}

// Reads optimized version of Tetra3 database from binary data file.

static const char *tetra3_db_tag = "Tetra3DB";  // no more than 8 characters!

bool T3Database::loadOptimized ( const std::string &filename, bool loadPatterns )
{
    // Open file; return error code on failure.
    
    uint32_t npatindex = 0;
    bool success = false;
    fp = fopen ( filename.c_str(), "rb" );
    if ( fp == NULL )
        return false;
    
    // Read and validate "Tetra3DB" tag
    
    char tag[8] = { 0 };
    if ( fread ( &tag, 8, 1, fp ) != 1 || strncmp ( tag, tetra3_db_tag, 8 ) != 0 )
        goto end;
    
    // Read metadata
    
    if ( fread ( &nstars, sizeof ( nstars ), 1, fp ) != 1 || nstars < 1 )
        goto end;

    if ( fread ( &npatindex, sizeof ( npatindex ), 1, fp ) != 1 || npatindex < 1 )
        goto end;
    
    if ( fread ( &npatterns, sizeof ( npatterns ), 1, fp ) != 1 || npatterns < 1 )
        goto end;

    if ( fread ( &pattern_size, sizeof ( pattern_size ), 1, fp ) != 1 )
        goto end;

    if ( fread ( &pattern_bins, sizeof ( pattern_bins ), 1, fp ) != 1 )
        goto end;

    if ( fread ( &pattern_max_error, sizeof ( pattern_max_error ), 1, fp ) != 1 )
        goto end;

    if ( fread ( &max_fov, sizeof ( max_fov ), 1, fp ) != 1 )
        goto end;

    if ( fread ( &min_fov, sizeof ( min_fov ), 1, fp ) != 1 )
        goto end;

    if ( fread ( &pattern_stars_per_fov, sizeof ( pattern_stars_per_fov ), 1, fp ) != 1 )
        goto end;

    if ( fread ( &verification_stars_per_fov, sizeof ( verification_stars_per_fov ), 1, fp ) != 1 )
        goto end;

    if ( fread ( &star_max_magnitude, sizeof ( star_max_magnitude ), 1, fp ) != 1 )
        goto end;

    // Allocate storage for stars, read stars.
    
    stars = std::vector<T3Star> ( nstars );
    if ( fread ( &stars[0], sizeof ( stars[0] ), nstars, fp ) != nstars )
        goto end;
    
    // Allocate storage for pattern index, read index
    
    patindex = std::vector<uint32_t> ( npatindex );
    if ( fread ( &patindex[0], sizeof ( patindex[0] ), npatindex, fp ) != npatindex )
        goto end;
    
    // if loading patterns into RAM, allocate storage, read patterns, close file.

    if ( loadPatterns )
    {
        patterns = std::vector<T3Pattern> ( npatterns );
        size_t n = fread ( &patterns[0], sizeof ( patterns[0] ), npatterns, fp );
        if ( n != npatterns )
            goto end;
        
        fclose ( fp );
        fp = NULL;
    }
    else
    {
        // save offset to first pattern, but don't read patterns into RAM.
        // leave file open so we can read patterns later; destructor will close it.

        pattern_offset = ftell ( fp );
    }
    
    success = true;
    
end:
    
    // close file if we failed to read it properly.
    
    if ( ! success )
    {
        fclose ( fp );
        fp = NULL;
    }
    
    loaded = success;
    return success;
}

// Saves optimized version of Tetra3 database to a binary data file

bool T3Database::saveOptimized ( const std::string &filename )
{
    // Open file
    
    bool success = false;
    FILE *fp = fopen ( filename.c_str(), "wb" );
    if ( fp == NULL )
        return false;
    
    // Write "Tetra3DB" tag
    
    if ( fwrite ( tetra3_db_tag, 8, 1, fp ) != 1 )
        return false;

    // Write metadata
    
    nstars = stars.size();
    npatterns = patterns.size();
    uint32_t npatindex = patindex.size();

    if ( fwrite ( &nstars, sizeof ( nstars ), 1, fp ) != 1 )
        goto end;

    if ( fwrite ( &npatindex, sizeof ( npatindex ), 1, fp ) != 1 )
        goto end;
    
    if ( fwrite ( &npatterns, sizeof ( npatterns ), 1, fp ) != 1 )
        goto end;

    if ( fwrite ( &pattern_size, sizeof ( pattern_size ), 1, fp ) != 1 )
        goto end;

    if ( fwrite ( &pattern_bins, sizeof ( pattern_bins ), 1, fp ) != 1 )
        goto end;

    if ( fwrite ( &pattern_max_error, sizeof ( pattern_max_error ), 1, fp ) != 1 )
        goto end;

    if ( fwrite ( &max_fov, sizeof ( max_fov ), 1, fp ) != 1 )
        goto end;

    if ( fwrite ( &min_fov, sizeof ( min_fov ), 1, fp ) != 1 )
        goto end;

    if ( fwrite ( &pattern_stars_per_fov, sizeof ( pattern_stars_per_fov ), 1, fp ) != 1 )
        goto end;

    if ( fwrite ( &verification_stars_per_fov, sizeof ( verification_stars_per_fov ), 1, fp ) != 1 )
        goto end;

    if ( fwrite ( &star_max_magnitude, sizeof ( star_max_magnitude ), 1, fp ) != 1 )
        goto end;

    // Write stars and patterns
    
    if ( fwrite ( &stars[0], sizeof ( stars[0] ), stars.size(), fp ) != stars.size() )
        goto end;

    if ( fwrite ( &patindex[0], sizeof ( patindex[0] ), patindex.size(), fp ) != patindex.size() )
        goto end;
    
    if ( fwrite ( &patterns[0], sizeof ( patterns[0] ), patterns.size(), fp ) != patterns.size() )
        goto end;

    success = true;
    
end:
    
    fclose ( fp );
    return success;
}

// Get hash index for a given key.
// Note use of 128-bit integer. We need this because we are multiplying a 64-bit integer
// (index) by a 32-bit integer (_MAGIC_RAND) and this can overflow 2^64. This actually
// happens when bin_factor is greater than 64, corresponding to pattern_max_arr < 0.0039!
// Note: bins = 1 / ( 4 * max_err ) and max_err = 1 / ( 4 * bins ) exactly.

uint32_t T3Database::keyToIndex(std::vector<int> key, uint32_t bin_factor)
{
    size_t max_index = patindex.size();
    __uint128_t index = 0, bin_factor_pow_i = 1;
    for (size_t i = 0; i < key.size(); ++i) {
        index += key[i] * bin_factor_pow_i;
        bin_factor_pow_i *= bin_factor;
    }
    
    // Randomise by magic constant and modulo to maximum index
    return (index * _MAGIC_RAND) % max_index;
}

// Get unit vectors from star centroids (pinhole camera).
// Compute list of (i,j,k) vectors given list of (y,x) star centroids and
// an estimate of the image's field-of-view in the x dimension in radians
// by applying the pinhole camera equations.
// Near-clone of SSSource::project().

std::vector<SSVector> Tetra3::computeVectors ( const std::vector<T3Source> &sources, float fov, float width, float height )
{
    float scale_factor = tan(fov / 2.0) / width * 2.0;
    float img_center[2] = { width / 2.0f, height / 2.0f };

    std::vector<SSVector> star_vectors;
    for ( const T3Source &source : sources) {
        
        SSVector v = { 1.0, 1.0, 1.0 };
        v.y = (img_center[0] - source.x) * scale_factor;
        v.z = (img_center[1] - source.y) * scale_factor;
        star_vectors.push_back( v.normalize() );
    }

    return star_vectors;
}

std::vector<T3Pattern> Tetra3::generatePatternsFromCentroids ( const std::vector<T3Source> &star_centroids, int pattern_size )
{
    std::vector<T3Pattern> patterns;
    
    // Iterate over centroids in order of brightness.
    // Break if there aren't enough centroids to make even one pattern

    if ( star_centroids.size() < pattern_size )
        return patterns;
    
    int pattern_indices[pattern_size + 2];
    pattern_indices[0] = -1;
    for (unsigned int i = 1; i <= pattern_size; ++i) {
        pattern_indices[i] = i - 1;
    }
    pattern_indices[pattern_size + 1] = star_centroids.size();
    patterns.push_back( T3Pattern ( &pattern_indices[1] ) );
    
    while (pattern_indices[1] < star_centroids.size() - pattern_size) {
        for (unsigned int index_to_change = 1; index_to_change <= pattern_size; ++index_to_change) {
            ++pattern_indices[index_to_change];
            if (pattern_indices[index_to_change] < pattern_indices[index_to_change + 1]) {
                break;
            }
            else {
                pattern_indices[index_to_change] = pattern_indices[index_to_change - 1] + 1;
            }
        }
        patterns.push_back( T3Pattern ( &pattern_indices[1] ) );
    }
                       
    return patterns;
}

// calculate the least-squares rotation matrix from image frame to catalog using singular value decomposition.

SSMatrix Tetra3::findRotationMatrix ( const std::vector<SSVector> &image_vectors, const std::vector<SSVector> &catalog_vectors )
{
    SSMatrix u, v;
    
    double **a = dmatrix ( 3, 3 );
    double **vt = dmatrix ( 3, 3 );
    double *w = dvector ( 3 );

    for ( int i = 0; i < image_vectors.size() && i < catalog_vectors.size(); i++ )
    {
        SSVector u = image_vectors[i], v = catalog_vectors[i];
        a[0][0] += u.x * v.x;    a[0][1] += u.x * v.y;    a[0][2] += u.x * v.z;
        a[1][0] += u.y * v.x;    a[1][1] += u.y * v.y;    a[1][2] += u.y * v.z;
        a[2][0] += u.z * v.x;    a[2][1] += u.z * v.y;    a[2][2] += u.z * v.z;
    }
    
    svdcmp ( a, 3, 3, w, vt );
    
    u = SSMatrix ( a[0][0], a[0][1], a[0][2],
                   a[1][0], a[1][1], a[1][2],
                   a[2][0], a[2][1], a[2][2] );
    
    v = SSMatrix ( vt[0][0], vt[0][1], vt[0][2],
                   vt[1][0], vt[1][1], vt[1][2],
                   vt[2][0], vt[2][1], vt[2][2] ).transpose();
    
    free_dmatrix ( a );
    free_dmatrix ( vt );
    free_dvector ( w );

    // Note we are returning the transpose!  The python code returns the original matrix,
    // but then transposes it later when rotating the star vectors.

    SSMatrix r = u * v;
    return r.transpose();
}

// Get stars within radius radians of the vector.

std::vector<SSVector> Tetra3::getNearbyStarVectors ( const SSVector &vector, double radius, int max_stars )
{
    std::vector<SSVector> nearby_star_vectors;
    
    double cosrad = cos ( radius );
    for ( size_t i = 0; i < db.numStars(); i++ )
    {
        T3Star star = db.getStar ( i );
        SSVector star_vector ( star.xyz[0], star.xyz[1], star.xyz[2] );
        if ( star_vector.dotProduct ( vector ) > cosrad )
            nearby_star_vectors.push_back ( star_vector );
        if ( nearby_star_vectors.size() >= max_stars )
            break;
    }
    
    return nearby_star_vectors;
}

// Multiplies all elements in a vector of vectors (vecs) by a 3x3 rotation matrix (rmat),
// and return a vector of the rotated input vectors.

std::vector<SSVector> rotateVectors ( SSMatrix rmat, std::vector<SSVector> &vecs )
{
    std::vector<SSVector> rvecs;
    for ( SSVector &vec : vecs )
        rvecs.push_back ( rmat * vec );
    return rvecs;
}

// Solve for the sky location of an image using source locations (centroids) of stars found in the image.
// The image's dimensions in pixels are width (x) and height (y).
// The function returns true if it can successfully solve the image, or false if it fails.
// If successful, details of the solution are returned in the T3Results struct provided.
// Every combination of the args.pattern_checking_stars brightest stars found is checked against the database
// before giving up.

bool Tetra3::solveFromSources ( const std::vector<T3Source> &sources, float width, float height, const T3Options &args, T3Results &results )
{
    if ( db.numPatterns() < 1 || db.numStars() < 1 )
        return false;
    
    // If no FoV estimate provided, sweep over the database FoV range from widest to narrowest,
    // reducing 20% each step. Try solving at each FoV estimate with 10% allowable FoV error.
    // Choose the solution (if any) with the lowest false-match probability.
    
    if ( args.fov_estimate == 0.0 )
    {
        T3Options opts = args;
        T3Results res = results;
        results.prob = 1.0;

        std::chrono::time_point t0_solve = std::chrono::high_resolution_clock::now();
        for ( float fov = db.max_fov; fov >= db.min_fov; fov *= 0.8 )
        {
            opts.fov_estimate = fov;
            opts.fov_max_error = fov * 0.1;
            if ( solveFromSources ( sources, width, height, opts, res ) )
                if ( res.prob < results.prob )
                    results = res;
        }
        
        std::chrono::duration<double> t_solve = std::chrono::high_resolution_clock::now() - t0_solve;
        results.t_solve = t_solve.count() * 1000.0;
        return results.prob <= args.match_threshold ? true : false;
    }
    
    // If no FOV given at all, guess middle of the range for a start
    float fov_initial = degtorad ( args.fov_estimate == 0.0 ? ( db.max_fov + db.min_fov ) / 2.0 : args.fov_estimate );
    float pattern_max_error = args.pattern_max_error == 0.0 ? db.pattern_max_error : args.pattern_max_error;
    float match_radius = args.match_radius;

    std::vector<T3Source> pattern_sources = sources;
    if ( sources.size() > args.pattern_checking_stars )
        pattern_sources = std::vector<T3Source> ( sources.begin(), sources.begin() + args.pattern_checking_stars );

    std::vector<T3Source> verification_sources = sources;
    if ( sources.size() > db.verification_stars_per_fov )
        verification_sources = std::vector<T3Source> ( sources.begin(), sources.begin() + db.verification_stars_per_fov );

    std::vector<T3Pattern> image_patterns = generatePatternsFromCentroids ( pattern_sources, db.pattern_size );

    // If we don't have a field of view estimate, calculate the largest
    // distance in pixels between centroids, for future FOV estimation.
    
    float pattern_largest_distance = 0.0;
    if ( args.fov_estimate == 0.0 )
        for ( int i = 0; i < pattern_sources.size(); i++ )
            for ( int j = i + 1; j < pattern_sources.size(); j++ )
                pattern_largest_distance = std::max ( pattern_largest_distance, pattern_sources[i].distance ( pattern_sources[j] ) );
    
    // This internal lambda function does the real work. It tests a single pattern of four sources in the input image
    // and returns true if the pattern results in a sucessful solution. It can be called in parallel, by multiple threads.
    
    auto solveFromPattern = [&] ( const T3Pattern &pattern ) -> bool
    {
        std::vector<T3Source> image_centroids;
        for ( int i = 0; i < db.pattern_size; i++ )
            image_centroids.push_back ( sources[ pattern.stars[i] ] );
        
        // Compute star vectors using an estimate for the field-of-view in the x dimension
        T3PatternVectors pattern_vectors;
        pattern_vectors.vectors = computeVectors(image_centroids, fov_initial, width, height);
        pattern_vectors.computeEdgeRatios();
        double pattern_largest_edge = pattern_vectors.largestEdge();
        
        // Possible hash codes to look up
        std::vector<std::vector<int>> hash_code_space;
        for (size_t i = 0; i < pattern_vectors.edge_ratios.size(); ++i) {
            double low = (pattern_vectors.edge_ratios[i] - pattern_max_error) * db.pattern_bins;
            double high = (pattern_vectors.edge_ratios[i] + pattern_max_error) * db.pattern_bins;
            int low_index = std::clamp(static_cast<int>(low), 0, db.pattern_bins);
            int high_index = std::min(static_cast<int>(high) + 1, db.pattern_bins);
            std::vector<int> range(high_index - low_index + 1);
            std::iota(range.begin(), range.end(), low_index);
            hash_code_space.push_back(range);
        }
        
        std::vector<T3HashCode> hash_codes = generate_hash_codes(hash_code_space);
        for ( T3HashCode &hash_code : hash_codes )
        {
            uint32_t hash_index = db.keyToIndex(hash_code, db.pattern_bins );
            std::vector<T3Pattern> matches = db.getAtIndex ( hash_index );
         
            if ( matches.size() == 0 )
                continue;

            // Calculate difference to observed pattern and find sufficiencly close ones

            std::vector<size_t> valid_patterns;
            std::vector<T3PatternVectors> catalog_star_vectors;

            for ( size_t i = 0; i < matches.size(); i++ )
            {
                if ( args.fov_estimate == 0.0 )
                {
                    // Calculate actual fov from pattern pixel distance and catalog edge angle
                    
                    double f = pattern_largest_distance / 2.0 / tan ( matches[i].largest_edge / 2.0 );
                    double fov = 2.0 * atan ( width / 2.0 / f );
                }
                else
                {
                    // Calculate actual fov by scaling estimate
                    // If the FOV is incorrect we can skip this immediately

                    double fov = matches[i].largest_edge / pattern_largest_edge * fov_initial;
                    if ( args.fov_max_error != 0.0 && fabs ( radtodeg ( fov ) - args.fov_estimate ) > args.fov_max_error )
                        continue;
                }
                
                T3PatternVectors pv = db.getStarPatternVectors ( matches[i] );
                pv.computeEdgeRatios();
                
                // Calculate difference to observed pattern and find sufficiencly close ones

                double max_edge_error = 0;
                for (size_t j = 0; j < pv.edge_ratios.size(); j++ )
                    max_edge_error = std::max ( max_edge_error, fabs ( pv.edge_ratios[j] - pattern_vectors.edge_ratios[j]));

                if ( max_edge_error < pattern_max_error )
                {
                    valid_patterns.push_back ( i );
                    catalog_star_vectors.push_back ( pv );
                }
            }

            for ( int i = 0; i < valid_patterns.size(); i++ )
            {
                size_t index = valid_patterns[i];
                T3PatternVectors catalog_vectors = catalog_star_vectors[i];
                double fov = catalog_vectors.largestEdge() / pattern_largest_edge * fov_initial;

                // Recalculate vectors and uniquely sort them by distance from centroid
                // so they can be uniqely matched with the catalog vectors
                
                pattern_vectors.vectors = computeVectors(image_centroids, fov, width, height);
                std::vector<SSVector> pattern_sorted_vectors, catalog_sorted_vectors;
                sortByDistanceFromCenter ( pattern_vectors.vectors, pattern_sorted_vectors );
                catalog_sorted_vectors = catalog_vectors.vectors;   // stars in pattern are already sorted by distance from center

                // Use the pattern match to find an estimate for the image's rotation matrix
                
                SSMatrix rotation_matrix = findRotationMatrix ( pattern_sorted_vectors, catalog_sorted_vectors );
                std::vector<SSVector> all_star_vectors = computeVectors ( verification_sources, fov, width, height );
                std::vector<SSVector> rotated_star_vectors = rotateVectors ( rotation_matrix, all_star_vectors );
                
                SSVector image_center_vector = rotation_matrix.col ( 0 );
                double fov_diagonal_rad = fov * hypot ( width, height ) / width / 2.0;
                std::vector<SSVector> nearby_star_vectors = getNearbyStarVectors ( image_center_vector, fov_diagonal_rad, db.verification_stars_per_fov );
                
                // Match the nearby star vectors to the proposed measured star vectors
                
                double cosrad = cos ( args.match_radius * fov );
                std::vector<SSVector> match_image_sources, match_catalog_stars;
                for ( int i = 0; i < rotated_star_vectors.size(); i++ )
                {
                    int sum = 0, jmatch = 0;
                    for ( int j = 0; j < nearby_star_vectors.size(); j++ )
                    {
                        if ( nearby_star_vectors[j].dotProduct ( rotated_star_vectors[i] ) > cosrad )
                        {
                            sum++;
                            jmatch = j;
                        }
                    }
                
                    if ( sum == 1 )
                    {
                        match_image_sources.push_back ( all_star_vectors[i] );
                        match_catalog_stars.push_back ( nearby_star_vectors[jmatch] );
                    }
                }
                
                // Statistical reasoning for probability that current match is incorrect:
                
                int num_extracted_stars = all_star_vectors.size();
                int num_nearby_catalog_stars = nearby_star_vectors.size();
                int num_star_matches = match_catalog_stars.size();
                
                // Probability that a single star is a mismatch
                double prob_single_star_mismatch = 1.0 - (1.0 - num_nearby_catalog_stars * match_radius * match_radius);
                
                // Two matches can always be made using the degrees of freedom of the pattern
                double prob_mismatch = binomialCDF(num_extracted_stars - (num_star_matches - 2), num_extracted_stars, 1.0 - prob_single_star_mismatch);
                if ( prob_mismatch < args.match_threshold )
                {
                    // if a match has been found, recompute rotation with all matched vectors
                    rotation_matrix = findRotationMatrix(match_image_sources, match_catalog_stars );
                    double det = rotation_matrix.determinant();

                    // Residuals calculation
                    double residual = 0.0;
                    rotated_star_vectors = rotateVectors ( rotation_matrix, match_image_sources );
                    for ( int i = 0; i < rotated_star_vectors.size(); i++ )
                    {
                        double angle = rotated_star_vectors[i].angularSeparation ( match_catalog_stars[i] );
                        residual += angle * angle;
                    }
                    residual = sqrt ( residual / rotated_star_vectors.size() ); // radians
                    
                    // extract right ascension, declination, and roll from rotation matrix
                    double ra  = atan2pi ( rotation_matrix.m10, rotation_matrix.m00 );
                    double dec = atan2 ( rotation_matrix.m20, hypot ( rotation_matrix.m21, rotation_matrix.m22 ) );
                    double roll = atan2pi ( rotation_matrix.m21, rotation_matrix.m22 );

                    results.ra  = radtodeg ( ra );
                    results.dec = radtodeg ( dec );
                    results.roll = radtodeg ( roll ) * ( det < 0 ? -1 : 1 );
                    results.fov = radtodeg ( fov );
                    results.matches = match_image_sources.size();
                    results.prob = prob_mismatch;
                    results.rmse = radtodeg ( residual ) * 3600.0;   // arcseconds
                    results.rmat = rotation_matrix;
                    return true;
                }
            }
        }
        
        return false;
    };
    
    // This lambda processes part of vector of patterns, so the entire vector of patterns can be
    // divided among multiple threads that process them in parallel.
    // The "solved" flag indicates when one thread has solved successfully. At that point, all threads can exit.
    
    std::atomic_bool solved = false;
    auto solveFromPatterns = [&] ( const std::vector<T3Pattern> &patterns, int start, int step ) -> bool
    {
        for ( size_t i = start; i < patterns.size() && solved == false; i += step )
        {
            std::this_thread::yield();
            if ( solveFromPattern ( patterns[i] ) )
                solved = true;
        }
        
        return solved;
    };
    
    // If no threading specified, process all patterns found in the image synchronously.
    // Otherwise, process them in parallel, divided among the number of threads specified.
    
    std::chrono::time_point t0_solve = std::chrono::high_resolution_clock::now();
    if ( args.num_threads == 0 )
    {
        solved = solveFromPatterns ( image_patterns, 0, 1 );
    }
    else
    {
        int num_threads = args.num_threads;
        std::vector<std::thread> threads;
        for ( int i = 0; i < num_threads; i++ )
            threads.push_back ( std::thread ( solveFromPatterns, image_patterns, i, num_threads ) );
        for ( int i = 0; i < num_threads; i++ )
            threads[i].join();
    }
    
    // Solved or failed in this time
    std::chrono::duration<double> t_solve = std::chrono::high_resolution_clock::now() - t0_solve;
    results.t_solve = t_solve.count() * 1000.0;
    return solved;
}
