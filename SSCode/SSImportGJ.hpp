// SSImportGJ.hpp
// SSCore
//
// Created by Tim DeBenedictis on 3/27/20.
// Copyright © 2020 Southern Stars. All rights reserved.
// Routins for importing the Gliese-Jahreiss Catalog of Nearby Stars, 3rd ed.

#ifndef SSImportGJ_hpp
#define SSImportGJ_hpp

#include "SSStar.hpp"
#include "SSHTM.hpp"

int SSImportGJCNS3 ( const string &filename, SSIdentifierNameMap &nameMap, SSObjectVec &acStars, SSObjectVec &gjStars );
int SSImportGJAC ( const string &filename, SSObjectVec &hipStars, SSObjectVec &acStars );

int SSImport10pcSample ( const string &filename, SSIdentifierNameMap &starNames, SSObjectVec &skyStars, SSObjectVec &gcvsStars, SSHTM &wdsHTM, SSObjectVec &stars );
int SSMergeNearbyStars ( SSObjectVec &skyStars, SSObjectVec &nearStars );

#endif /* SSImportGJ_hpp */
