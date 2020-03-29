// SSImportSKY2000.hpp
// SSCore
//
// Created by Tim DeBenedictis on 3/23/20.
// Copyright © 2020 Southern Stars. All rights reserved.
//
// Functions for importing the SKY2000 Master Star Catalog
// and IAU table of official star names.

#ifndef SSImportSKY2000_hpp
#define SSImportSKY2000_hpp

#include <map>
#include <string>

#include "SSStar.hpp"

int SSImportSKY2000 ( const char *filename, SSIdentifierNameMap &nameMap, SSObjectVec &stars );
int SSImportIAUStarNames ( const char *filename, SSIdentifierNameMap &nameMap );

#endif /* SSImportSKY2000_hpp */