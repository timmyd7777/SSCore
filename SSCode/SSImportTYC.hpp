// SSImportTYC.hpp
// SSCore
//
// Created by Tim DeBenedictis on 6/1/21.
// Copyright ©2021 Southern Stars Group, LLC. All rights reserved.

#ifndef SSImportTYC_hpp
#define SSImportTYC_hpp

#include "SSStar.hpp"

struct TYC2HD
{
    SSIdentifier tyc;
    SSIdentifier hd;
    string spectrum;
};

typedef multimap<SSIdentifier,TYC2HD> TYC2HDMap;

int SSImportTYC ( const string &filename, TYC2HDMap &tyc2hdmap, SSObjectVec &stars );
int SSImportTYC2 ( const string &filename, TYC2HDMap &tyc2hdmap, SSObjectVec &stars );
int SSImportTYC2HD ( const string &filename, TYC2HDMap &tyc2hdmap );

#endif /* SSImportTYC_hpp */

