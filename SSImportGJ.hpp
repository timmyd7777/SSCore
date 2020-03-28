// SSImportGJ.hpp
// SSCore
//
// Created by Tim DeBenedictis on 3/27/20.
// Copyright © 2020 Southern Stars. All rights reserved.
// Routins for importing the Gliese-Jahreiss Catalog of Nearby Stars, 3rd ed.

#ifndef SSImportGJ_hpp
#define SSImportGJ_hpp

#include "SSStar.hpp"

int SSImportGJ ( const char *filename, SSIdentifierNameMap &nameMap, SSObjectVec &stars );

#endif /* SSImportGJ_hpp */
