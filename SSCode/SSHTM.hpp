// SSHTM.hpp
// SSCore
//
// Created by Tim DeBenedictis on 8/10/20.
// Copyright © 2020 Southern Stars. All rights reserved.

#ifndef SSHTM_HPP
#define SSHTM_HPP

#ifndef USE_THREADS
#if defined(__EMSCRIPTEN__) && !defined(__EMSCRIPTEN_PTHREADS__)
#define USE_THREADS 0
#else
#define USE_THREADS 1
#endif
#endif

#if USE_THREADS
#include <thread>
#endif

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

// Callback function to notify external HTM user when regions are loaded asynchronously.

class SSHTM
{
public:
    typedef void (* RegionLoadCallback) ( SSHTM *pHTM, uint64_t htmID );
    typedef int (* DataFileFunc) ( SSHTM *pHTM, uint64_t htmID, SSObjectArray *objects, void *userData );
    typedef bool (* RegionTestCallback) ( SSHTM *pHTM, uint64_t htmID, void *userData );
    
protected:
    DataFileFunc                _readFunc = nullptr;    // custom function for reading region data files
    DataFileFunc                _writeFunc = nullptr;   // custom function for writing region data files
    map<uint64_t,SSObjectVec *> _regions;               // arrays of objects loaded into memory, indexed by HTM region ID
    vector<float>               _magLevels;             // faintest magnitude of objects at each HTM level; vector size is depth of mesh tree
    string                      _rootpath;              // directory containing object data files on filesystem.
    
#if USE_THREADS
    map<uint64_t,thread *>      _loadThreads;           // background threads currently loading region objects from data files, indexed by HTM region ID
#endif
    SSObjectVec *_loadRegion ( uint64_t htmID, RegionLoadCallback callback, void *userData );    // private method to load object data file for a given HTM region ID
    
public:
    
    // constructors and destructor
    
    SSHTM();
    SSHTM ( const vector<float> &magLevels, const string &rootpath );
    virtual ~SSHTM ( void );
    
    // return path to directory containing region data files
    
    string rootPath ( void ) { return _rootpath; }
    
    // get magnitude limits for a particular HTM region ID;
    // get HTM level corresponding to a particular magnitude

    bool magLimits ( uint64_t id, float &min, float &max );
    int magLevel ( float mag );

    // store an individual object or an antire array of objects in this HTM

    virtual bool store ( SSStar *pStar );
    virtual int store ( SSObjectVec &objects );
 
    // Count number of regions and objects in HTM or in a region therein.
    
    int countRegions ( void ) { return (int) _regions.size(); }
    int countStars ( void );
    int countStars ( uint64_t htmID );
    
    // save region objects to file(s), load them from file(s), dump them from memory.
    
    int saveRegions ( void *userData = nullptr );
    int saveRegion ( uint64_t id, void *userData = nullptr );
    int loadRegions ( uint64_t htmID = 0, bool sync = true, void *userData = nullptr );
    SSObjectVec *loadRegion ( uint64_t htmID, bool sync = true, void *userData = nullptr );
    void dumpRegions ( RegionTestCallback callback = nullptr, void *userData = nullptr );
    void dumpRegion ( uint64_t htmID );
    void clearRegions ( void );
    
    // test whether region objects are loaded into memory, get array of pointers to loaded region objects
    
    bool regionLoaded ( uint64_t id );
    SSObjectVec *getObjects ( uint64_t id );
    
    // Get child HTM region IDs of a particular region; gets empty vector if region has no children.
    
    virtual vector<uint64_t> subRegionIDs ( uint64_t id );

    // wrappers around functions in original Johns Hopkins C HTM implementation, cc_aux.c
    
    virtual uint64_t vector2ID ( const SSVector &vector, int depth );
    virtual uint64_t name2ID ( const string &name );
    virtual int IDlevel ( uint64_t id );
    virtual string ID2name ( uint64_t id );
    virtual bool name2Triangle ( const string &name, SSVector &v0, SSVector &v1, SSVector &v2 );
    virtual bool isinside ( const SSVector &p, const SSVector &v0, SSVector &v1, SSVector &v2 );
    
    // Describes the location of particular object inside an HTM
    
    struct ObjectLoc
    {
        uint64_t region;    // HTM ID of the region containing the object.
        size_t   offset;    // Position of object within region's object vector, counting from zero.
    };
    
    typedef multimap<string,ObjectLoc>       NameMap;
    typedef multimap<SSIdentifier,ObjectLoc> IdentMap;
    
    map<SSCatalog,NameMap>  _nameIndex;
    map<SSCatalog,IdentMap> _identIndex;

    typedef int (* IdentMapFunc) ( SSHTM *pHTM, SSCatalog cat, IdentMap *pMap, void *userData );

    size_t loadObjectMap ( SSCatalog cat, IdentMapFunc loadFunc = nullptr, void *userData = nullptr );
    size_t saveObjectMap ( SSCatalog cat, IdentMapFunc saveFunc = nullptr, void *userData = nullptr );

    size_t makeObjectMap ( SSCatalog cat );
    size_t makeObjectMap ( SSCatalog cat, uint64_t regionID, NameMap &nameMap, IdentMap &identMap );
    
    size_t objectMapSize ( SSCatalog cat ) { return cat == kCatUnknown ? _nameIndex[cat].size() : _identIndex[cat].size(); }
    
    int findObjectLocs ( const string &name, vector<ObjectLoc> &locs, bool casesens = true, bool begins = false );
    int findObjectLocs ( SSIdentifier ident, vector<ObjectLoc> &locs );
    
    SSObjectPtr loadObject ( const ObjectLoc &loc );

    void setDataFileReadFunc ( DataFileFunc func ) { _readFunc = func; }
    DataFileFunc getDataFileReadFunc ( void ) { return _readFunc; }
    
    void setDataFileWriteFunc ( DataFileFunc func ) { _writeFunc = func; }
    DataFileFunc getDataFileWriteFunc ( void ) { return _writeFunc; }

    int search ( uint64_t htmID, SSVector center, SSAngle rad, vector<SSObjectPtr> &results );
};

// Callback function to notify external HTM user when regions are loaded asynchronously.

void SSHTMSetRegionLoadCallback ( SSHTM::RegionLoadCallback pCallback );
SSHTM::RegionLoadCallback SSHTMGetRegionLoadCallback ( void );

#endif /* SSHTM_HPP */
