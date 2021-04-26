// SSImportGCVS.hpp
// SSTest
//
// Created by Tim DeBenedictis on 4/26/21.
// Copyright © 2021 Southern Stars. All rights reserved.
//
// Functions for importing the General Catalog of Variable Stars
// and its cross-index.

#ifndef SSImportGCVS_hpp
#define SSImportGCVS_hpp

#include "SSObject.hpp"

int SSImportGCVSCrossIdentifiers ( const string &filename, SSIdentifierMap &identmap );
int SSImportGCVS ( const string &filename, SSIdentifierMap &identmap, SSObjectArray &stars );

#endif /* SSImportGCVS_hpp */
