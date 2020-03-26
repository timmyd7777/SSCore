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

typedef multimap<SSIdentifier,string> SSIdentifierNameMap;

void importNGCICNameMap ( const char *filename, SSIdentifierNameMap &nameMap );
vector<string> getNamesFromIdentifiers ( vector<SSIdentifier> &idents, SSIdentifierNameMap &nameMap );

int SSImportNGCIC ( const char *filename, SSIdentifierNameMap &nameMap, SSObjectVec &objects );
int SSImportDAML02 ( const char *filename, SSIdentifierNameMap &nameMap, SSObjectVec &clusters );
int SSImportMWGC ( const char *filename, SSIdentifierNameMap &nameMap, SSObjectVec &clusters );

#endif /* SSNGCIC_hpp */
