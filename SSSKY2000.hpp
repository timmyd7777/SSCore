// SSSKY2000.hpp
// SSCore
//
// Created by Tim DeBenedictis on 3/23/20.
// Copyright © 2020 Southern Stars. All rights reserved.
//
// Functions for importing the SKY2000 Master Star Catalog
// and IAU table of official star names.

#ifndef SSSKY2000_hpp
#define SSSKY2000_hpp

#include <map>
#include <string>

#include "SSStar.hpp"

typedef map<SSIdentifier,string> SSStarNameMap;

SSStarNameMap importIAUStarNames ( const char *filename );
vector<string> getStarNames ( vector<SSIdentifier> &idents, SSStarNameMap nameMap );

vector<SSStar> importSKY2000 ( const char *filename, SSStarNameMap &nameMap );

#endif /* SSSKY2000_hpp */
