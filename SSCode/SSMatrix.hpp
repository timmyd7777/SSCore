// SSMatrix.hpp
// SSCore
//
// Created by Tim DeBenedictis on 2/24/20.
// Copyright © 2020 Southern Stars. All rights reserved.
//
// Represents a 3x3 matrix, with routines for performing simple matrix and vector-matrix arithmetic.

#ifndef SSMatrix_hpp
#define SSMatrix_hpp

#include "SSVector.hpp"

struct SSMatrix
{
    double m00, m01, m02;
    double m10, m11, m12;
    double m20, m21, m22;
    
    SSMatrix ( void );
    SSMatrix ( double m00, double m01, double m02, double m10, double m11, double m12, double m20, double m21, double m22 );
    
    SSMatrix transpose ( void );
    SSMatrix inverse ( void );
    double  determinant ( void );
    
    static SSMatrix identity ( void );
    static SSMatrix rotation ( int n, ... );
    SSMatrix negateMiddleRow ( void );

    SSVector multiply ( SSVector vec );
    SSMatrix multiply ( SSMatrix mat );
    SSMatrix rotate ( int axis, double angle );
    
    SSVector operator * ( SSVector other ) { return multiply ( other ); }
    SSMatrix operator * ( SSMatrix other ) { return multiply ( other ); }
};

#endif /* SSMatrix_hpp */
