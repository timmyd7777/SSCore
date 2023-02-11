// SSImportWDS.hpp
// SSCore
//
// Created by Tim DeBenedictis on 4/26/21.
// Copyright © 2021 Southern Stars Group, LLC. All rights reserved.
//
// Functions for importing data from the Washington Double Star catalog
// and the Sixth Catalog of Orbits of Visual Binary Stars.

#ifndef SSImportWDS_hpp
#define SSImportWDS_hpp

#include "SSHTM.hpp"
#include "SSStar.hpp"

int SSImportORB6  ( const string &filename, const SSIdentifierMap &identmap, SSObjectArray &stars );
int SSImportWDS  ( const string &filename, const SSIdentifierMap &identmap, SSObjectArray &stars );
int SSImportORB6toHTM  ( const string &filename, const SSIdentifierMap &identmap, SSHTM &htm );
int SSImportWDStoHTM ( const string &filename, const SSIdentifierMap &identmap, SSHTM &htm );
int SSImportWDSHIPCrossIndex ( const string &filename, SSIdentifierMap &identmap );
int SSImportWDSBFCrossIndex ( const string &filename, SSIdentifierMap &identmap );
SSDoubleStarPtr SSFindWDSStar ( SSHTM &wdsHTM, SSSpherical coords, char comp, char &prim, float sep );
bool SSCopyDoubleStarData ( SSDoubleStarPtr pWDStar, char comp, char primComp, SSStarPtr pStar );

#endif /* SSImportWDS_hpp */
