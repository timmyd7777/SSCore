// SSImportGJ.hpp
// SSCore
//
// Created by Tim DeBenedictis on 3/27/20.
// Copyright © 2020 Southern Stars. All rights reserved.
// Routins for importing the Gliese-Jahreiss Catalog of Nearby Stars, 3rd ed.

#ifndef SSImportGJ_hpp
#define SSImportGJ_hpp

#include "SSStar.hpp"

int SSImportGJCNS3 ( const string &filename, SSIdentifierNameMap &nameMap, SSObjectVec &acStars, SSObjectVec &gjStars );
int SSImportGJAC ( const string &filename, SSObjectVec &hipStars, SSObjectVec &acStars );

#endif /* SSImportGJ_hpp */
