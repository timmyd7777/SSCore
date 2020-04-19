// SSEvent.hpp
// SSTest
//
// Created by Tim DeBenedictis on 4/18/20.
// Copyright © 2020 Southern Stars. All rights reserved.
//
// This class computes times and circumstances of astronomical events like
// object rising/transit/setting, satellite passes, conjuctions, elongations,
// and oppositions, etc.

#ifndef SSEvent_hpp
#define SSEvent_hpp

#include "SSCoordinates.hpp"
#include "SSObject.hpp"

class SSEvent
{
public:
    
    static const int kRise = -1;        // event sign for rising, to be used with riseTransSet, etc().
    static const int kTransit = 0;      // event sign for transit, to be used with riseTransSet, etc().
    static const int kSet = 1;          // event sign for setting, to be used with riseTransSet, etc().
    
    static SSAngle semiDiurnalArc ( SSAngle lat, SSAngle dec, SSAngle alt );
    
    static SSTime riseTransitSet ( SSTime jd, SSAngle ra, SSAngle dec, int sign, SSAngle lon, SSAngle lat, SSAngle alt );
    static SSTime riseTransitSet ( SSTime time, SSCoordinates &coords, SSObjectPtr pObj, int sign, SSAngle alt );
    static SSTime riseTransitSetSearch ( SSTime time, SSCoordinates coords, SSObjectPtr pObj, int sign, SSAngle alt );
    static SSTime riseTransitSetSearchDay ( SSTime today, SSCoordinates coords, SSObjectPtr pObj, int sign, SSAngle alt );
};

#endif /* SSEvent_hpp */
