#include "com_southernstars_sscore_JSSObject.h"
#include "JNIUtilities.h"
#include "SSObject.hpp"

jobject SSObjectToJSSObject ( JNIEnv *pEnv, SSObject *pObject )
{
    if ( pObject == nullptr )
        return NULL;

    jobject pJSSObject = CreateJObject ( pEnv, "com/southernstars/sscore/JSSObject" );
    if ( pJSSObject )
        SetLongField ( pEnv, pJSSObject, "pObject", (jlong) pObject );

    return pJSSObject;
}

SSObject *JSSObjectToSSObject (JNIEnv *pEnv, jobject pJSSObject )
{
    return (SSObject *) ( GetLongField ( pEnv, pJSSObject, "pObject" ) );
}

/*
 * Class:     com_southernstars_sscore_JSSObject
 * Method:    typeToCode
 * Signature: (I)Ljava/lang/String;
 */

JNIEXPORT jstring JNICALL Java_com_southernstars_sscore_JSSObject_typeToCode ( JNIEnv *pEnv, jclass pClassJSSObject, jint type )
{
    string code = SSObject::typeToCode ( (SSObjectType) type );
    return pEnv->NewStringUTF ( code.c_str() );
}

/*
 * Class:     com_southernstars_sscore_JSSObject
 * Method:    codeToType
 * Signature: (Ljava/lang/String;)I
 */

JNIEXPORT jint JNICALL Java_com_southernstars_sscore_JSSObject_codeToType ( JNIEnv *pEnv, jclass pClassJSSObject, jstring pJCodeString )
{
    const char *pCodeString = pEnv->GetStringUTFChars ( pJCodeString, nullptr );
    int type = SSObject::codeToType ( pCodeString );
    pEnv->ReleaseStringUTFChars ( pJCodeString, pCodeString );
    return type;
}

/*
 * Class:     com_southernstars_sscore_JSSObject
 * Method:    getType
 * Signature: ()I
 */

JNIEXPORT jint JNICALL Java_com_southernstars_sscore_JSSObject_getType ( JNIEnv *pEnv, jobject pJObject )
{
    SSObject *pObj = (SSObject *) GetLongField ( pEnv, pJObject, "pObject" );
    return pObj ? pObj->getType() : kTypeNonexistent;
}

/*
 * Class:     com_southernstars_sscore_JSSObject
 * Method:    getName
 * Signature: (I)Ljava/lang/String;
 */

JNIEXPORT jstring JNICALL Java_com_southernstars_sscore_JSSObject_getName ( JNIEnv *pEnv, jobject pJObject, jint i )
{
    SSObject *pObj = (SSObject *) GetLongField ( pEnv, pJObject, "pObject" );
    const char *pName = pObj ?  pObj->getName ( i ).c_str() : "";
    return pEnv->NewStringUTF ( pName );
}

/*
 * Class:     com_southernstars_sscore_JSSObject
 * Method:    getIdentifier
 * Signature: (I)Lcom/southernstars/sscore/JSSIdentifier;
 */

JNIEXPORT jobject JNICALL Java_com_southernstars_sscore_JSSObject_getIdentifier ( JNIEnv *pEnv, jobject pJObject, jint catalog )
{
    SSObject *pObj = (SSObject *) GetLongField ( pEnv, pJObject, "pObject" );
    SSIdentifier ident = pObj ? pObj->getIdentifier ( (SSCatalog) catalog ) : SSIdentifier ( 0 );
    return SSIdentifierToJSSIdentifier ( pEnv, ident );
}

/*
 * Class:     com_southernstars_sscore_JSSObject
 * Method:    getDirection
 * Signature: ()Lcom/southernstars/sscore/JSSVector;
 */

JNIEXPORT jobject JNICALL Java_com_southernstars_sscore_JSSObject_getDirection ( JNIEnv *pEnv, jobject pJObject )
{
    SSObject *pObj = (SSObject *) GetLongField ( pEnv, pJObject, "pObject" );
    SSVector direction = pObj ? pObj->getDirection() : SSVector ( HUGE_VAL, HUGE_VAL, HUGE_VAL );
    return SSVectorToJSSVector ( pEnv, direction );
}

/*
 * Class:     com_southernstars_sscore_JSSObject
 * Method:    getDistance
 * Signature: ()D
 */

JNIEXPORT jdouble JNICALL Java_com_southernstars_sscore_JSSObject_getDistance ( JNIEnv *pEnv, jobject pJObject )
{
    SSObject *pObj = (SSObject *) GetLongField ( pEnv, pJObject, "pObject" );
    return pObj ? pObj->getDistance() : HUGE_VAL;
}

/*
 * Class:     com_southernstars_sscore_JSSObject
 * Method:    getMagnitude
 * Signature: ()F
 */

JNIEXPORT jfloat JNICALL Java_com_southernstars_sscore_JSSObject_getMagnitude ( JNIEnv *pEnv, jobject pJObject )
{
    SSObject *pObj = (SSObject *) GetLongField ( pEnv, pJObject, "pObject" );
    return pObj ? pObj->getMagnitude() : HUGE_VAL;
}

/*
 * Class:     com_southernstars_sscore_JSSObject
 * Method:    setDirection
 * Signature: (Lcom/southernstars/sscore/JSSVector;)V
 */

JNIEXPORT void JNICALL Java_com_southernstars_sscore_JSSObject_setDirection ( JNIEnv *pEnv, jobject pJObject, jobject pJDirection )
{
    SSObject *pObj = (SSObject *) GetLongField ( pEnv, pJObject, "pObject" );
    if ( pObj )
        pObj->setDirection ( JSSVectorToSSVector ( pEnv, pJDirection ) );
}

/*
 * Class:     com_southernstars_sscore_JSSObject
 * Method:    setDistance
 * Signature: (D)V
 */

JNIEXPORT void JNICALL Java_com_southernstars_sscore_JSSObject_setDistance ( JNIEnv *pEnv, jobject pJObject, jdouble distance )
{
    SSObject *pObj = (SSObject *) GetLongField ( pEnv, pJObject, "pObject" );
    if ( pObj )
        pObj->setDistance ( distance );
}

/*
 * Class:     com_southernstars_sscore_JSSObject
 * Method:    setMagnitude
 * Signature: (F)V
 */

JNIEXPORT void JNICALL Java_com_southernstars_sscore_JSSObject_setMagnitude ( JNIEnv *pEnv, jobject pJObject, jfloat magnitude )
{
    SSObject *pObj = (SSObject *) GetLongField ( pEnv, pJObject, "pObject" );
    if ( pObj )
        pObj->setMagnitude ( magnitude );
}
