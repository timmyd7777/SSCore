// SSHTM.cpp
// SSCore
//
// Created by Tim DeBenedictis on 8/10/20.
// Copyright © 2020 Southern Stars. All rights reserved.

#include "SSHTM.hpp"

uint64_t cc_vector2ID ( double x, double y, double z, int depth );
int cc_IDlevel ( uint64_t htmid );
int cc_isinside ( const double *p, const double *v1, const double *v2, const double *v3 );
int cc_ID2name ( char *name, uint64_t id );
uint64_t cc_name2ID ( const char *name );
int cc_name2Triangle ( const char *name, double *v0, double *v1, double *v2 );

// Constructor specifies array of magnitude limits for each HTM level,
// and root path to directory containing region data files in CSV format.

SSHTM::SSHTM ( const vector<float> &magLevels, const string &rootpath )
{
    _magLevels = magLevels;
    
    _rootpath = rootpath;
    if ( _rootpath[ _rootpath.length() - 1 ] != '/' )
        _rootpath += '/';
}

// Destructor frees memory for all loaded regions and all objects therein.

SSHTM::~SSHTM ( void )
{
    dumpRegions();
}

// Returns the HTM level corresponding to a specific stellar magnitude,
// or -1 if the magnitude does not correspond to any HTM level.

int SSHTM::magLevel ( float mag )
{
    for ( int l = 0; l < _magLevels.size(); l++ )
        if ( mag <= _magLevels[l] )
            return l;
    
    return -1;
}

// For a specific HTM region ID, returns a vector of HTM region IDs of all
// sub-regions contained within this region. If the input HTM region is at
// the bottom level, returns an empty vector because it has no sub-regions.
// If the input region is the origin, returns eight HTM root triangle IDs;
// otherwise returns four HTM sub-triangle IDs.

vector<uint64_t> SSHTM::subRegionIDs ( uint64_t htmID )
{
    int level = 0;
    if ( htmID > 0 )
        level = IDlevel ( htmID ) + 1;
    
    if ( level >= _magLevels.size() )
        return vector<uint64_t> ( 0 );
    
    if ( level == 0 )
        return vector<uint64_t> ( { 8, 9, 10, 11, 12, 13, 14, 15 } );
    
    uint64_t subID = htmID * 4;
    return  vector<uint64_t> ( { subID, subID + 1, subID + 2, subID + 3 } );
}

// Stores a pointer to a star or deep sky object in this HTM, creating an HTM region to store it in, if needed.
// Returns true if successful or false if the star cannot be stored.

bool SSHTM::store ( SSStar *pStar )
{
    float mag = pStar->getVMagnitude();
    if ( isinf ( mag ) )
        mag = pStar->getBMagnitude();
    
    SSVector pos = pStar->getFundamentalPosition();
    
    int level = magLevel ( mag );
    if ( level < 0 )
        return false;
    
    uint64_t htmID = 0;
    if ( level > 0 )
        htmID = SSHTM::vector2ID ( pos, level - 1 );

    if ( _regions.count ( htmID ) == 0 )
        _regions[htmID] = new SSObjectVec();
    
    _regions[htmID]->push_back ( pStar );
    return true;
}

// Stores all stars and deep sky objects in an array of object pointers (objects)
// into this HTM, and returns the total number of pointers stored.

int SSHTM::store ( SSObjectVec &objects )
{
    int n = 0;
    
    for ( int i = 0; i < objects.size(); i++ )
    {
        SSStar *pStar = SSGetStarPtr ( objects[i] );
        if ( pStar == nullptr )
            continue;
        
        if ( store ( pStar ) )
            n++;
    }
    
    return n;
}

// Saves all regions of this HTM as CSV-formatted files in its root directory.
// Root directory must already exist, and root path must end with a '/' character.
// CSV files within directory will be named for individual HTM regions and will overwrite
// any existing files with the same names.
// Returns the total number of objects written to the file(s).

int SSHTM::saveRegions ( void )
{
    int n = 0;
    
    for ( auto it = _regions.begin(); it != _regions.end(); it++ )
    {
        uint64_t htmID = it->first;
        SSObjectVec *objects = it->second;
        string name = SSHTM::ID2name ( htmID );
        if ( ! name.empty() )
            n += SSExportObjectsToCSV ( _rootpath + name + ".csv", *objects );
    }
    
    return n;
}

// Saves a single region of this HTM as a CSV-formatted files in its root directory.
// Root directory must already exist, and root path must end with a '/' character.
// CSV file will be named for its HTM region, and overwrites any existing file with same name.
// Returns the total number of objects written to the file.

int SSHTM::saveRegion ( uint64_t htmID )
{
    int n = 0;
    
    if ( _regions.count ( htmID ) )
    {
        string name = ID2name ( htmID );
        if ( ! name.empty() )
            n = SSExportObjectsToCSV ( _rootpath + name + ".csv", *_regions[htmID] );
    }
    
    return n;
}

// Loads star data for a specific region in this HTM and recursively for its sub-regions.
// Returns the total number of regions loaded.

int SSHTM::loadRegions ( uint64_t htmID )
{
    int n = 0;
    
    if ( loadRegion ( htmID ) )
        n++;
    
    vector<uint64_t> subIDs = subRegionIDs ( htmID );
    for ( int i = 0; i < subIDs.size(); i++ )
       n += loadRegions ( subIDs[i] );
        
    return n;
}

// Loads star data for a single region in this HTM from a CSV-formatted file in the HTM root directory.
// Returns the number of stars loaded. If region is already loaded, returns number of stars already in region.

int SSHTM::loadRegion ( uint64_t htmID )
{
    int n = 0;
    
    if ( regionLoaded ( htmID ) )
        return countStars ( htmID );
    
    string name = ID2name ( htmID );
    if ( ! name.empty() )
    {
        SSObjectVec *objects = new SSObjectVec();
        n = SSImportObjectsFromCSV ( _rootpath + name + ".csv", *objects );
        if ( n > 0 )
            _regions[htmID] = objects;
    }
    
    return n;
}

// Tests whether star data for a specific region in this HTM has been
// loaded into memory, i.e. if that region exists in this HTM.

bool SSHTM::regionLoaded ( uint64_t htmID )
{
    return _regions.count ( htmID );
}

// Returns pointer to array of objects stored in the region
// with the specified HTM triangle ID. If region is not present
// in this HTM or objects have not been loaded, returns nullptr.

SSObjectVec *SSHTM::getObjects ( uint64_t htmID )
{
    return _regions[htmID];
}

// Deletes all star data for a specific region in this HTM from memory.

void SSHTM::dumpRegion ( uint64_t htmID )
{
    if ( _regions.count ( htmID ) )
    {
        if ( _regions[htmID] != nullptr )
            delete _regions[htmID];
        _regions.erase ( htmID );
    }
}

// Deletes all star data for all regions in this HTM from memory.

void SSHTM::dumpRegions ( void )
{
    _regions.clear();
}

// Counts total number of stars stored in all regions in this HTM.

int SSHTM::countStars ( void )
{
    int count = 0;
    
    for ( auto it = _regions.begin(); it != _regions.end(); it++ )
        if ( it->second != nullptr )
            count += it->second->size();
    
    return count;
}

// Counts number of stars stored in a single region in this HTM.

int SSHTM::countStars ( uint64_t htmID )
{
    int count = 0;
    
    if ( _regions.count ( htmID ) && _regions[htmID] != nullptr )
        count = (int) _regions[htmID]->size();

    return count;
}

// Given a unit vector to a point on the celestial sphere, returns the HTM ID
// of the triangle containing that vector at a specific HTM depth level.

uint64_t SSHTM::vector2ID ( const SSVector &vector, int depth )
{
    return cc_vector2ID ( vector.x, vector.y, vector.z, depth );
}

// Given an HTM triangle name, returns the HTM ID of that triangle,
// or zero if the name is not a valid HTM name.

uint64_t SSHTM::name2ID ( const string &name )
{
    return cc_name2ID ( name.c_str() );
}

// Given an HTM triangle ID, returns the depth level of that triangle.

int SSHTM::IDlevel ( uint64_t id )
{
    return cc_IDlevel ( id );
}

// Given an HTM triangle ID, returns the triangle's HTM name string,
// or an empty string if the ID is not a valid HTM ID.

string SSHTM::ID2name ( uint64_t id )
{
    if ( id == 0 )
        return string ( "O0" );
    
    char name[64] = { 0 };
    cc_ID2name ( name, id );
    return string ( name );
}

// Given an HTM triangle name, computes unit vectors to the triangle's three vertices
// on the unit sphere (v0, v1, v2).  Returns true if the name is valid, or false otherwise.

bool SSHTM::name2Triangle ( const string &name, SSVector &v0, SSVector &v1, SSVector &v2 )
{
    return cc_name2Triangle ( name.c_str(), &v0.x, &v1.x, &v2.x );
}

// Given a unit vector to a point on the celestial sphere (p), determines if p is inside
// the triangle on the celestial sphere whose vertices are the unit vectors (v0, v1, v2).

bool SSHTM::isinside ( const SSVector &p, const SSVector &v0, SSVector &v1, SSVector &v2 )
{
    return cc_isinside ( &p.x, &v0.x, &v1.x, &v2.x );
}

// Original code from cc_aux.c was downloaded from:
// http://www.skyserver.org/htm/implementation.aspx#download

#include <stdio.h>
#include <math.h>
#include <string.h>

/*
 * cc_name2ID
 * cc_isinside
 * cc_startpane
 * cc_parseVectors
 * cc_vector2ID
 * cc_radec2ID
 * cc_ID2name
 */

#define IDSIZE 64

typedef double                float64;

#ifdef _WIN32
typedef __int64                int64;
typedef unsigned __int64    uint64_t;

typedef __int32 int32;
typedef unsigned __int32 uint32;
#else
typedef long long            int64;
typedef unsigned long long    uint64_t;

typedef long int32;
typedef unsigned long uint32;
#define IDHIGHBIT  0x8000000000000000LL
#define IDHIGHBIT2 0x4000000000000000LL
#endif

#define HTMNAMEMAX         32

static const double gEpsilon = 1.0E-15;

double anchor[][3] = {
  {0.0L,  0.0L,  1.0L}, // 0
  {1.0L,  0.0L,  0.0L}, // 1
  {0.0L,  1.0L,  0.0L}, // 2
  {-1.0L,  0.0L,  0.0L}, // 3
  {0.0L, -1.0L,  0.0L}, // 4
  {0.0L,  0.0L, -1.0L}  // 5
};

struct _bases {
    const char *name;
    int ID;
    int v1, v2, v3;
} bases[] = {
    {"S2", 10, 3, 5, 4},
    {"N1", 13, 4, 0, 3},
    {"S1", 9, 2, 5, 3},
    {"N2", 14, 3, 0, 2},
    {"S3", 11, 4,5,1},
    {"N0", 12, 1, 0, 4},
    {"S0", 8, 1, 5, 2},
    {"N3", 15, 2, 0, 1}
};

int S_indexes[][3] = {
  {1, 5, 2}, //S0
  {2, 5, 3}, //S1
  {3, 5, 4}, //S2
  {4, 5, 1} //S3
};
int N_indexes[][3] = {
  {1, 0, 4}, //N0
  {4, 0, 3}, //N1
  {3, 0, 2}, //N2
  {2, 0, 1} //N3
};
  

int cc_startpane(
         double *v1, double *v2, double *v3,
         double xin, double yin, double zin, char *name)
{
  int ix = (xin > 0 ? 4 : 0) + (yin > 0 ? 2 : 0) + (zin > 0 ? 1 : 0);
  double *tvec;
  const char *s;
  int baseID;

  // printf("Startpane %f %f %f, (%d)\n", xin, yin, zin, ix);

  baseID = bases[ix].ID;
    
  tvec = anchor[bases[ix].v1];
  v1[0] = tvec[0];
  v1[1] = tvec[1];
  v1[2] = tvec[2];

  tvec = anchor[bases[ix].v2];
  v2[0] = tvec[0];
  v2[1] = tvec[1];
  v2[2] = tvec[2];

  tvec = anchor[bases[ix].v3];
  v3[0] = tvec[0];
  v3[1] = tvec[1];
  v3[2] = tvec[2];

  s = bases[ix].name;
  name[0] = *s++;
  name[1] = *s++;
  name[2] = '\0';
  return baseID;
}

double cc_Epsilon = 1.0E-15;
double cc_sqrt3    = 1.7320508075688772935;

#define m4_midpoint(v1, v2, w, tmp){\
    w[0] = v1[0] + v2[0]; w[1] = v1[1] + v2[1]; w[2] = v1[2] + v2[2]; \
    tmp = sqrt(w[0] * w[0] + w[1] * w[1] + w[2]*w[2]);\
    w[0] /= tmp; w[1] /= tmp; w[2] /= tmp;}

#define copy_vec(d, s) { d[0] = s[0]; d[1] = s[1]; d[2] = s[2]; }




uint64_t cc_vector2ID(double x, double y, double z, int depth)
{
  uint64_t rstat = 0;
  int startID;
  char name[80];
  int len = 0;

  double v1[3], v2[3], v0[3];
  double w1[3], w2[3], w0[3];
  double p[3];
  double dtmp;

  p[0] = x;
  p[1] = y;
  p[2] = z;

  //
  // Get the ID of the level0 triangle, and its starting vertices
  //

  startID = cc_startpane(v0, v1, v2, x, y, z, name);
  len = 2;
  //
  // Start searching for the children
  ///
  while(depth-- > 0){
    m4_midpoint(v0, v1, w2, dtmp);
    m4_midpoint(v1, v2, w0, dtmp);
    m4_midpoint(v2, v0, w1, dtmp);

    if (cc_isinside(p, v0, w2, w1)) {
      name[len++] = '0';
      copy_vec(v1, w2);
      copy_vec(v2, w1);
    }
    else if (cc_isinside(p, v1, w0, w2)) {
      name[len++] = '1';
      copy_vec(v0, v1);
      copy_vec(v1, w0);
      copy_vec(v2, w2);
    }
    else if (cc_isinside(p, v2, w1, w0)) {
      name[len++] = '2';
      copy_vec(v0, v2);
      copy_vec(v1, w1);
      copy_vec(v2, w0);
    }
    else if (cc_isinside(p, w0, w1, w2)) {
      name[len++] ='3';
      copy_vec(v0, w0);
      copy_vec(v1, w1);
      copy_vec(v2, w2);
    }
    else {
      fprintf(stderr, "PANIC\n");
    }
  }
  name[len] = '\0';
  rstat = cc_name2ID(name);
  return rstat;
}

int cc_isinside ( const double *p, const double *v1, const double *v2, const double *v3 )
{
  double crossp[3];
  // if (v1 X v2) . p < epsilon then false
  // same for v2 X v3 and v3 X v1.
  // else return true..
  crossp[0] = v1[1] * v2[2] - v2[1] * v1[2];
  crossp[1] = v1[2] * v2[0] - v2[2] * v1[0];
  crossp[2] = v1[0] * v2[1] - v2[0] * v1[1];
  if (p[0] * crossp[0] + p[1] * crossp[1] + p[2] * crossp[2] < -gEpsilon)
    return 0;


  crossp[0] = v2[1] * v3[2] - v3[1] * v2[2];
  crossp[1] = v2[2] * v3[0] - v3[2] * v2[0];
  crossp[2] = v2[0] * v3[1] - v3[0] * v2[1];
  if (p[0] * crossp[0] + p[1] * crossp[1] + p[2] * crossp[2] < -gEpsilon)
    return 0;


  crossp[0] = v3[1] * v1[2] - v1[1] * v3[2];
  crossp[1] = v3[2] * v1[0] - v1[2] * v3[0];
  crossp[2] = v3[0] * v1[1] - v1[0] * v3[1];
  if (p[0] * crossp[0] + p[1] * crossp[1] + p[2] * crossp[2] < -gEpsilon)
    return 0;

  return 1;
}

uint64_t cc_name2ID(const char *name)
{
  uint64_t out=0, i;
  size_t siz = 0;

  if(name == 0)              // null pointer-name
    return 0;
  if(name[0] != 'N' && name[0] != 'S')  // invalid name
    return 0;

  siz = strlen(name);       // determine string length
  // at least size-2 required, don't exceed max
  if(siz < 2)
    return 0;
  if(siz > HTMNAMEMAX)
    return 0;

  for(i = siz-1; i > 0; i--) {// set bits starting from the end
    if(name[i] > '3' || name[i] < '0') {// invalid name
      return 0;
    }
    out += ( (uint64_t)(name[i]-'0')) << 2*(siz - i -1);
  }

  i = 2;                     // set first pair of bits, first bit always set
  if(name[0]=='N') i++;      // for north set second bit too
  out += (i << (2*siz - 2) );

  
  /************************
               // This code may be used later for hashing !
               if(size==2)out -= 8;
               else {
               size -= 2;
               uint32 offset = 0, level4 = 8;
               for(i = size; i > 0; i--) { // calculate 4 ^ (level-1), level = size-2
               offset += level4;
               level4 *= 4;
               }
               out -= level4 - offset;
               }
  **************************/
  return out;
}

#define HTM_INVALID_ID 1

/*
 * cc_IDlevel is a trusting method (assumes that the id is well formed and
 * valid) that returns the level of the trixel represented by the given
 * htm id
 */
int cc_IDlevel(uint64_t htmid)
{
  uint32 size=0, i;

#if defined(_WIN32)
  uint64_t IDHIGHBIT = 1;
  uint64_t IDHIGHBIT2= 1;
  IDHIGHBIT = IDHIGHBIT << 63;
  IDHIGHBIT2 = IDHIGHBIT2 << 62;
#endif


  // determine index of first set bit
  for(i = 0; i < IDSIZE; i+=2) {
    if ( (htmid << i) & IDHIGHBIT ) break;
    /*
      if ( (id << i) & IDHIGHBIT2 )  // invalid id
      return HTM_INVALID_ID;
      // but we trust you now...
      */
  }
/*   if(id == 0) */
/*     return HTM_INVALID_ID; */

  size=(IDSIZE-i) >> 1;
  /* Size is the length of the string representing the name of the
     trixel, the level is size - 2
  */
  return size-2;
}

int cc_ID2name(char *name, uint64_t id)
{

  uint32 size=0, i;
  int c; // a spare character;

#if defined(_WIN32)
  uint64_t IDHIGHBIT = 1;
  uint64_t IDHIGHBIT2= 1;
  IDHIGHBIT = IDHIGHBIT << 63;
  IDHIGHBIT2 = IDHIGHBIT2 << 62;
#endif



  // determine index of first set bit
  for(i = 0; i < IDSIZE; i+=2) {
    if ( (id << i) & IDHIGHBIT ) break;
    if ( (id << i) & IDHIGHBIT2 )  // invalid id
      return HTM_INVALID_ID;
  }
  if(id == 0)
    return HTM_INVALID_ID;

  size=(IDSIZE-i) >> 1;

  // fill characters starting with the last one
  for(i = 0; i < size-1; i++) {
    c =  '0' + (int) ((id >> i*2) & (uint32) 3);
    name[size-i-1] = (char ) c;
  }

  // put in first character
  if( (id >> (size*2-2)) & 1 ) {
    name[0] = 'N';
  } else {
    name[0] = 'S';
  }
  name[size] = 0; // end string

  return 0;
}

int cc_name2Triangle(const char *name, double *v0, double *v1, double *v2)
{
  int rstat = 0;
  const char *s;
  double w1[3], w2[3], w0[3];
  double dtmp;


  //
  // Get the top level hemi-demi-semi space
  //
  int k;
  int anchor_offsets[3];
  k = (int) name[1] - '0';

  if (name[0] == 'S') {
    anchor_offsets[0] = S_indexes[k][0];
    anchor_offsets[1] = S_indexes[k][1];
    anchor_offsets[2] = S_indexes[k][2];
  } else {
    anchor_offsets[0] = N_indexes[k][0];
    anchor_offsets[1] = N_indexes[k][1];
    anchor_offsets[2] = N_indexes[k][2];
  }
  s = name+2;
  copy_vec(v0, anchor[anchor_offsets[0]]);
  copy_vec(v1, anchor[anchor_offsets[1]]);
  copy_vec(v2, anchor[anchor_offsets[2]]);

  while(*s){
    m4_midpoint(v0, v1, w2, dtmp);
    m4_midpoint(v1, v2, w0, dtmp);
    m4_midpoint(v2, v0, w1, dtmp);
    switch(*s) {
    case '0':
      copy_vec(v1, w2);
      copy_vec(v2, w1);
      break;
    case '1':
      copy_vec(v0, v1);
      copy_vec(v1, w0);
      copy_vec(v2, w2);
      break;
    case '2':
      copy_vec(v0, v2);
      copy_vec(v1, w1);
      copy_vec(v2, w0);
      break;
    case '3':
      copy_vec(v0, w0);
      copy_vec(v1, w1);
      copy_vec(v2, w2);
      break;
    }
    s++;
  }
  return rstat;
}

