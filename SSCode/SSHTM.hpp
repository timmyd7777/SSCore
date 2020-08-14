// SSHTM.hpp
// SSCore
//
// Created by Tim DeBenedictis on 8/10/20.
// Copyright © 2020 Southern Stars. All rights reserved.

#ifndef SSHTM_HPP
#define SSHTM_HPP

#include <thread>

#include "SSObject.hpp"
#include "SSStar.hpp"
#include "SSVector.hpp"

// No, not Hypertext Markup Language!
// This class implements the Heirarchial Triangle Mesh, a method for subdividing the celestial sphere
// into recursive triangular regions. Used by the Guide Star Catalog 2.x and Sloan Digital Sky Survey.
// For more information see: http://www.skyserver.org/HTM/Old_default.aspx
// Our HTM includes an "origin region" at level 0 named O0, with HTM ID 0, that covers the entire sky.
// Its eight children at level 1 are the HTM root triangles named S0, S1, S2, S3, N0, N1, N2, N3,
// with HTM ID numbers 8, 9, 10, 11, 12, 13, 14, 15. Each of those has four childred at level 2,
// named S00, S01, S02, S02, etc. with HTM ID numbers 32, 33, 34, 35 etc., and so on down the mesh tree.
// This class also contains methods for loading, saving, and storing objects in the regions to files.
// Regions can be loaded synchronously on the current thread, or asynchronously on a background thread.

class SSHTM
{
    map<uint64_t,SSObjectVec *> _regions;           // arrays of objects loaded into memory, indexed by HTM region ID
    vector<float>               _magLevels;         // faintest magnitude of objects at each HTM level; vector size is depth of mesh tree
    string                      _rootpath;          // directory containing object data files on filesystem.
    
    map<uint64_t,thread *>      _loadThreads;       // background threads currently loading region objects from data files, indexed by HTM region ID
    SSObjectVec *_loadRegion ( uint64_t htmID );    // private method to load object data file for a given HTM region ID
    
public:
    
    // constructors and destructor
    
    SSHTM();
    SSHTM ( const vector<float> &magLevels, const string &rootpath );
    virtual ~SSHTM ( void );
    
    // get magnitude limits for a particular HTM region ID;
    // get HTM level corresponding to a particular magnitude

    bool magLimits ( uint64_t id, float &min, float &max );
    int magLevel ( float mag );

    // store an individual object or an antire array of objects in this HTM

    bool store ( SSStar *pStar );
    int store ( SSObjectVec &objects );
 
    // Count number of regions and objects in HTM or in a region therein.
    
    int countRegions ( void ) { return (int) _regions.size(); }
    int countStars ( void );
    int countStars ( uint64_t htmID );
    
    // save region objects to file(s), load them from file(s), dump them from memory.
    
    int saveRegions ( void );
    int saveRegion ( uint64_t id );
    int loadRegions ( uint64_t htmID = 0 );
    SSObjectVec *loadRegion ( uint64_t htmID, bool sync );
    void dumpRegions ( void );
    void dumpRegion ( uint64_t htmID );
    
    // test whether region objects are loaded into memory, get array of pointers to loaded region objects
    
    bool regionLoaded ( uint64_t id );
    SSObjectVec *getObjects ( uint64_t id );
    
    // Get child HTM region IDs of a particular region; gets empty vector if region has no children.
    
    vector<uint64_t> subRegionIDs ( uint64_t id );

    // wrappers around functions in original Johns Hopkins C HTM implementation, cc_aux.c
    
    static uint64_t vector2ID ( const SSVector &vector, int depth );
    static uint64_t name2ID ( const string &name );
    static int IDlevel ( uint64_t id );
    static string ID2name ( uint64_t id );
    static bool name2Triangle ( const string &name, SSVector &v0, SSVector &v1, SSVector &v2 );
    static bool isinside ( const SSVector &p, const SSVector &v0, SSVector &v1, SSVector &v2 );
};

// Callback function to notify external HTM user when regions are loaded asynchronously.

typedef void (* SSHTMRegionLoadCallback) ( SSHTM *pHTM, uint64_t htmID );
void SSHTMSetRegionLoadCallback ( SSHTMRegionLoadCallback pCallback );
SSHTMRegionLoadCallback SSHTMGetRegionLoadCallback ( void );

#endif /* SSHTM_HPP */
