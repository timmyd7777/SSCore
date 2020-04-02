// SSImportMPC.hpp
// SSCore
//
// Created by Tim DeBenedictis on 3/24/20.
// Copyright © 2020 Southern Stars. All rights reserved.
// Routines for importing asteroid and comet data from Minor Planet Center export files.

#ifndef SSImportMPC_hpp
#define SSImportMPC_hpp

#include "SSPlanet.hpp"

int SSImportMPCComets ( const string &filename, SSObjectVec &comets );
int SSImportMPCAsteroids ( const string &filename, SSObjectVec &comets );

#endif /* SSImportMPC_hpp */
