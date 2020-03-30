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

int SSImportDAML02 ( const char *filename, SSIdentifierNameMap &nameMap, SSObjectVec &clusters );
int SSImportMWGC ( const char *filename, SSIdentifierNameMap &nameMap, SSObjectVec &clusters );
int SSImportPNG ( const char *main_filename, const char *dist_filename, const char *diam_filename, const char *vel_filename, SSIdentifierNameMap &nameMap, SSObjectVec &planebs );
int SSImportNGCIC ( const char *filename, SSIdentifierNameMap &nameMap, SSObjectVec &clusters, SSObjectVec &globulars, SSObjectVec &planNebs, SSObjectVec &objects );

#endif /* SSImportNGCIC_hpp */
