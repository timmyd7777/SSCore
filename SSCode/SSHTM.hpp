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
// into recursive triangular regions, used by the Guide Star Catalog 2.x and Sloan Digitial Sky Survey.
// For more information: http://www.skyserver.org/HTM/Old_default.aspx
// Our HTM includes an "origin region" named O0, with HTM ID 0, that covers the entire sky.
// Its 8 children are the HTM root triangles named S0, S1, S2, S3, N0, N1, N2, N3,
// with HTM ID numbers 8, 9, 10, 11, 12, 13, 14, 15.
// This class also contains methods for loading, saving, and storing objects in the regions.
// Regions can be loaded synchronously on the current thread, or asynchronously on a background thread.

class SSHTM
{
    map<uint64_t,SSObjectVec *> _regions;
    vector<float> _magLevels;
    string _rootpath;
    
    map<uint64_t,thread *> _loadThreads;
    SSObjectVec *_loadRegion ( uint64_t htmID );
    
public:
    
    SSHTM();
    SSHTM ( const vector<float> &magLevels, const string &rootpath );
    virtual ~SSHTM ( void );
    
    int magLevel ( float mag );
    bool store ( SSStar *pStar );
    int store ( SSObjectVec &objects );
    
    int countRegions ( void ) { return (int) _regions.size(); }
    int countStars ( void );
    int countStars ( uint64_t htmID );
    
    int saveRegions ( void );
    int saveRegion ( uint64_t id );
    int loadRegions ( uint64_t htmID = 0 );
    SSObjectVec *loadRegion ( uint64_t htmID, bool sync );
    void dumpRegions ( void );
    void dumpRegion ( uint64_t htmID );
    
    bool regionLoaded ( uint64_t id );
    SSObjectVec *getObjects ( uint64_t id );
    
    vector<uint64_t> subRegionIDs ( uint64_t id );
    bool magLimits ( uint64_t id, float &min, float &max );
    
    static uint64_t vector2ID ( const SSVector &vector, int depth );
    static uint64_t name2ID ( const string &name );
    static int IDlevel ( uint64_t id );
    static string ID2name ( uint64_t id );
    static bool name2Triangle ( const string &name, SSVector &v0, SSVector &v1, SSVector &v2 );
    static bool isinside ( const SSVector &p, const SSVector &v0, SSVector &v1, SSVector &v2 );
};

// This function is called after regions are loaded asynchronously.

typedef void (* SSHTMRegionLoadCallback) ( SSHTM *pHTM, uint64_t htmID );

void SSHTMSetRegionLoadCallback ( SSHTMRegionLoadCallback pCallback );
SSHTMRegionLoadCallback SSHTMGetRegionLoadCallback ( void );

#endif /* SSHTM_HPP */
