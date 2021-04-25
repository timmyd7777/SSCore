// SSImportNGCIC.hpp
// SSCore
//
// Created by Tim DeBenedictis on 3/23/20.
// Copyright © 2020 Southern Stars. All rights reserved.
// Routines for importing Wolfgang Steinicke's Revised NGC-IC Catalogues.

#ifndef SSImportNGCIC_hpp
#define SSImportNGCIC_hpp

#include <map>
#include "SSStar.hpp"

int SSImportDAML02 ( const string &filename, SSIdentifierNameMap &nameMap, SSObjectVec &clusters );
int SSImportMWGC ( const string &filename, SSIdentifierNameMap &nameMap, SSObjectVec &clusters );
int SSImportPNG ( const string &main_filename, const string &dist_filename, const string &diam_filename, const string &vel_filename, SSIdentifierNameMap &nameMap, SSObjectVec &planebs );
int SSImportNGCIC ( const string &filename, SSIdentifierNameMap &nameMap, SSObjectVec &clusters, SSObjectVec &globulars, SSObjectVec &planNebs, SSObjectVec &objects, SSObjectFilter filter = nullptr, void *userData = nullptr );

#endif /* SSImportNGCIC_hpp */
