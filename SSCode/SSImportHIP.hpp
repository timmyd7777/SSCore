// SSImportHIP.hpp
// SSCore
//
// Created by Tim DeBenedictis on 3/23/20.
// Copyright © 2020 Southern Stars. All rights reserved.
//
// Functions for importing the Hipparcos and related star catalogs
// (Hipparcos Input Catalog, Hipparcos New Reduction).

#ifndef SSHipparcos_hpp
#define SSHipparcos_hpp

#include <map>
#include <string>

#include "SSStar.hpp"

void SSUpdateStarCoordsAndMotion ( double jyear, SSMatrix *pMatrix, SSSpherical &position, SSSpherical &velocity );

int SSImportHIPHRIdentifiers ( const string &filename, SSIdentifierMap &map );
int SSImportHIPBayerIdentifiers ( const string &filename, SSIdentifierMap &map );
int SSImportHIPGCVSIdentifiers ( const string &filename, SSIdentifierMap &map );
int SSImportHIPNames ( const string &filename, SSIdentifierNameMap &nameMap );

int SSImportHIC ( const string &filename, SSObjectVec &stars );
int SSImportHIP2 ( const string &filename, SSObjectVec &stars );
int SSImportHIP ( const string &filename, SSIdentifierMap &hrMap, SSIdentifierMap &bayMap, SSIdentifierMap &varMap, SSIdentifierNameMap &nameMap, SSObjectVec &hicStars, SSObjectVec &hip2Stars, SSObjectVec &stars );

#endif /* SSHipparcos_hpp */
