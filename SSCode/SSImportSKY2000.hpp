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

#include "SSHTM.hpp"
#include "SSStar.hpp"

int SSImportIAUStarNames ( const string &filename, SSIdentifierNameMap &nameMap );
int SSImportSKY2000 ( const string &filename, SSIdentifierNameMap &nameMap, SSObjectVec &hipStars, SSObjectVec &gjStars, SSObjectVec &gcvsStars, SSHTM &wdsHTM, SSObjectVec &stars, SSObjectFilter filter = nullptr, void *userData = nullptr );
int SSMergeHIPTYCtoSKY2000 ( SSObjectVec &hipStars, SSObjectVec &skyStars );

#endif /* SSImportSKY2000_hpp */
