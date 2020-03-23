// SSHipparcos.hpp
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

typedef multimap<int,SSIdentifier> HIPMap;
typedef multimap<int,string> HIPNameMap;
typedef map<int,SSStar> SSStarMap;

SSStarMap importHIC ( const char *filename );
SSStarMap importHIP ( const char *filename, HIPMap mapHIPtoHR, HIPMap mapHIPtoBF, HIPMap mapHIPtoVar, SSStarMap mapHIC, SSStarMap mapHIP2, HIPNameMap mapNames );
HIPMap importHIPtoHRMap ( const char *filename );
HIPMap importHIPtoBayerFlamsteedMap ( const char *filename );
HIPMap importHIPtoVarMap ( const char *filename );
HIPNameMap importHIPNameMap ( const char *filename );
SSStarMap importHIP2 ( const char *filename );

#endif /* SSHipparcos_hpp */
