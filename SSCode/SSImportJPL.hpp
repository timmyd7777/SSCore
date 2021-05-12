// SSImportJPL.hpp
// Nanoverse
//
// Created by Tim DeBenedictis on 5/11/21.
// Copyright © 2021 Southern Stars Group, LLC. All rights reserved.
// Routines for importing asteroid and comet data from JPL's DASTCOM export CSV files.

#ifndef SSImportJPL_hpp
#define SSImportJPL_hpp

#include "SSPlanet.hpp"

SSPlanetPtr SSImportJPLAstCom ( const string &line, SSObjectType type );
int SSImportJPLDASTCOM ( const string &filename, SSObjectType type, SSObjectVec &objects );

#endif /* SSImportJPL_hpp */
