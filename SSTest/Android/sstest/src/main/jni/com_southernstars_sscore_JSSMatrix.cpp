#include "com_southernstars_sscore_JSSMatrix.h"
#include "JNIUtilities.h"
#include "SSMatrix.hpp"

jobject SSMatrixToJSSMatrix ( JNIEnv *pEnv, SSMatrix &matrix )
{
    jobject pJSSMatrix = CreateJObject ( pEnv, "com/southernstars/sscore/JSSMatrix" );

    if ( pJSSMatrix != nullptr )
    {
        SetDoubleField ( pEnv, pJSSMatrix, "m00", matrix.m00 );
        SetDoubleField ( pEnv, pJSSMatrix, "m01", matrix.m01 );
        SetDoubleField ( pEnv, pJSSMatrix, "m02", matrix.m02 );
        SetDoubleField ( pEnv, pJSSMatrix, "m10", matrix.m10 );
        SetDoubleField ( pEnv, pJSSMatrix, "m11", matrix.m11 );
        SetDoubleField ( pEnv, pJSSMatrix, "m12", matrix.m12 );
        SetDoubleField ( pEnv, pJSSMatrix, "m20", matrix.m20 );
        SetDoubleField ( pEnv, pJSSMatrix, "m21", matrix.m21 );
        SetDoubleField ( pEnv, pJSSMatrix, "m22", matrix.m22 );
    }

    return pJSSMatrix;
}

SSMatrix JSSMatrixToSSMatrix ( JNIEnv *pEnv, jobject pJSSMatrix )
{
    double m00 = GetDoubleField ( pEnv, pJSSMatrix, "m00" );
    double m01 = GetDoubleField ( pEnv, pJSSMatrix, "m01" );
    double m02 = GetDoubleField ( pEnv, pJSSMatrix, "m02" );
    double m10 = GetDoubleField ( pEnv, pJSSMatrix, "m10" );
    double m11 = GetDoubleField ( pEnv, pJSSMatrix, "m11" );
    double m12 = GetDoubleField ( pEnv, pJSSMatrix, "m12" );
    double m20 = GetDoubleField ( pEnv, pJSSMatrix, "m20" );
    double m21 = GetDoubleField ( pEnv, pJSSMatrix, "m21" );
    double m22 = GetDoubleField ( pEnv, pJSSMatrix, "m22" );

    return SSMatrix ( m00, m01, m02,
                      m10, m11, m12,
                      m20, m21, m22 );
}

/*
 * Class:     com_southernstars_sscore_JSSMatrix
 * Method:    inverse
 * Signature: ()Lcom/southernstars/sscore/JSSMatrix;
 */

JNIEXPORT jobject JNICALL Java_com_southernstars_sscore_JSSMatrix_inverse ( JNIEnv *pEnv, jobject pJMatrix )
{
    SSMatrix matrix = JSSMatrixToSSMatrix ( pEnv, pJMatrix );
    SSMatrix invmat = matrix.inverse();
    return SSMatrixToJSSMatrix ( pEnv, invmat );
}

/*
 * Class:     com_southernstars_sscore_JSSMatrix
 * Method:    determinant
 * Signature: ()D
 */

JNIEXPORT jdouble JNICALL Java_com_southernstars_sscore_JSSMatrix_determinant ( JNIEnv *pEnv, jobject pJMatrix )
{
    SSMatrix matrix = JSSMatrixToSSMatrix ( pEnv, pJMatrix );
    return matrix.determinant();
}

/*
 * Class:     com_southernstars_sscore_JSSMatrix
 * Method:    multiply
 * Signature: (Lcom/southernstars/sscore/JSSVector;)Lcom/southernstars/sscore/JSSVector;
 */

JNIEXPORT jobject JNICALL Java_com_southernstars_sscore_JSSMatrix_multiply__Lcom_southernstars_sscore_JSSVector_2 ( JNIEnv *pEnv, jobject pJMatrix, jobject pJVector )
{
    SSVector vector = JSSVectorToSSVector ( pEnv, pJVector );
    SSMatrix matrix = JSSMatrixToSSMatrix ( pEnv, pJMatrix );
    vector = matrix.multiply ( vector );
    return SSVectorToJSSVector ( pEnv, vector );
}

/*
 * Class:     com_southernstars_sscore_JSSMatrix
 * Method:    multiply
 * Signature: (Lcom/southernstars/sscore/JSSMatrix;)Lcom/southernstars/sscore/JSSMatrix;
 */

JNIEXPORT jobject JNICALL Java_com_southernstars_sscore_JSSMatrix_multiply__Lcom_southernstars_sscore_JSSMatrix_2 ( JNIEnv *pEnv, jobject pJMatrix1, jobject pJMatrix2 )
{
    SSMatrix matrix1 = JSSMatrixToSSMatrix ( pEnv, pJMatrix1 );
    SSMatrix matrix2 = JSSMatrixToSSMatrix ( pEnv, pJMatrix2 );
    SSMatrix matrix3 = matrix1.multiply ( matrix2 );
    return SSMatrixToJSSMatrix ( pEnv, matrix3 );
}

/*
 * Class:     com_southernstars_sscore_JSSMatrix
 * Method:    rotate
 * Signature: (ID)Lcom/southernstars/sscore/JSSMatrix;
 */

JNIEXPORT jobject JNICALL Java_com_southernstars_sscore_JSSMatrix_rotate ( JNIEnv *pEnv, jobject pJMatrix, jint axis, jdouble angle )
{
    SSMatrix matrix = JSSMatrixToSSMatrix ( pEnv, pJMatrix );
    matrix = matrix.rotate ( axis, angle );
    return SSMatrixToJSSMatrix ( pEnv, matrix );
}
