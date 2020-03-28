// SSNGCIC.hpp
// SSCore
//
// Created by Tim DeBenedictis on 3/23/20.
// Copyright © 2020 Southern Stars. All rights reserved.
// Routines for importing Wolfgang Steinicke's Revised NGC-IC Catalogues.

#ifndef SSNGCIC_hpp
#define SSNGCIC_hpp

#include <map>
#include "SSStar.hpp"

void addNGCICClusterData ( SSObjectVec &clusters, SSObjectVec &objects );

int SSImportNGCIC ( const char *filename, SSIdentifierNameMap &nameMap, SSObjectVec &objects );
int SSImportDAML02 ( const char *filename, SSIdentifierNameMap &nameMap, SSObjectVec &clusters );
int SSImportMWGC ( const char *filename, SSIdentifierNameMap &nameMap, SSObjectVec &clusters );
int SSImportPNG ( const char *main_filename, const char *dist_filename, const char *diam_filename, const char *vel_filename, SSIdentifierNameMap &nameMap, SSObjectVec &planebs );

#endif /* SSNGCIC_hpp */
