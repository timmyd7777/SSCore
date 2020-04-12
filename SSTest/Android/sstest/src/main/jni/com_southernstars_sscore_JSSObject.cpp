#include "com_southernstars_sscore_JSSObject.h"
#include "JNIUtilities.h"
#include "SSObject.hpp"

/*
 * Class:     com_southernstars_sscore_JSSObject
 * Method:    typeToCode
 * Signature: (I)Ljava/lang/String;
 */

JNIEXPORT jstring JNICALL Java_com_southernstars_sscore_JSSObject_typeToCode ( JNIEnv *pEnv, jclass pClassJSSObject, jint type )
{
    return nullptr;
}

/*
 * Class:     com_southernstars_sscore_JSSObject
 * Method:    codeToType
 * Signature: (Ljava/lang/String;)I
 */

JNIEXPORT jint JNICALL Java_com_southernstars_sscore_JSSObject_codeToType ( JNIEnv *pEnv, jclass pClassJSSObject, jstring pJCode )
{
    return 0;
}

/*
 * Class:     com_southernstars_sscore_JSSObject
 * Method:    getType
 * Signature: ()I
 */

JNIEXPORT jint JNICALL Java_com_southernstars_sscore_JSSObject_getType ( JNIEnv *pEnv, jobject pJObject )
{
    return 0;
}

/*
 * Class:     com_southernstars_sscore_JSSObject
 * Method:    getName
 * Signature: (I)Ljava/lang/String;
 */

JNIEXPORT jstring JNICALL Java_com_southernstars_sscore_JSSObject_getName ( JNIEnv *pEnv, jobject pJObject, jint i )
{
    return nullptr;
}

/*
 * Class:     com_southernstars_sscore_JSSObject
 * Method:    getIdentifier
 * Signature: (I)Lcom/southernstars/sscore/JSSIdentifier;
 */

JNIEXPORT jobject JNICALL Java_com_southernstars_sscore_JSSObject_getIdentifier ( JNIEnv *pEnv, jobject pJObject, jint i )
{
    return nullptr;
}

/*
 * Class:     com_southernstars_sscore_JSSObject
 * Method:    getDirection
 * Signature: ()Lcom/southernstars/sscore/JSSVector;
 */

JNIEXPORT jobject JNICALL Java_com_southernstars_sscore_JSSObject_getDirection ( JNIEnv *pEnv, jobject pJObject )
{
    return nullptr;
}

/*
 * Class:     com_southernstars_sscore_JSSObject
 * Method:    getDistance
 * Signature: ()D
 */

JNIEXPORT jdouble JNICALL Java_com_southernstars_sscore_JSSObject_getDistance ( JNIEnv *pEnv, jobject pJObject )
{
    return 0.0;
}


/*
 * Class:     com_southernstars_sscore_JSSObject
 * Method:    getMagnitude
 * Signature: ()F
 */

JNIEXPORT jfloat JNICALL Java_com_southernstars_sscore_JSSObject_getMagnitude ( JNIEnv *pEnv, jobject pJObject )
{
    return 0.0;
}

/*
 * Class:     com_southernstars_sscore_JSSObject
 * Method:    setDirection
 * Signature: (Lcom/southernstars/sscore/JSSVector;)V
 */

JNIEXPORT void JNICALL Java_com_southernstars_sscore_JSSObject_setDirection ( JNIEnv *pEnv, jobject pJObject )
{

}

/*
 * Class:     com_southernstars_sscore_JSSObject
 * Method:    setDistance
 * Signature: (D)V
 */

JNIEXPORT void JNICALL Java_com_southernstars_sscore_JSSObject_setDistance ( JNIEnv *pEnv, jobject pJObject, jdouble distance )
{

}

/*
 * Class:     com_southernstars_sscore_JSSObject
 * Method:    setMagnitude
 * Signature: (F)V
 */

JNIEXPORT void JNICALL Java_com_southernstars_sscore_JSSObject_setMagnitude ( JNIEnv *pEnv, jobject pJObject, jfloat magnitude )
{

}
