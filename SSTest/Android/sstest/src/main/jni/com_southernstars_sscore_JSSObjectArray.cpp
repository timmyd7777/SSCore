#include "com_southernstars_sscore_JSSObjectArray.h"
#include "JNIUtilities.h"
#include "SSObject.hpp"
#include "SSPlanet.hpp"

/*
 * Class:     com_southernstars_sscore_JSSObjectArray
 * Method:    create
 * Signature: ()V
 */

JNIEXPORT void JNICALL Java_com_southernstars_sscore_JSSObjectArray_create ( JNIEnv *pEnv, jobject pJObjectArray )
{
    SSObjectVec *pObjectVec = new SSObjectVec;
    SetLongField ( pEnv, pJObjectArray, "pObjectVec", (long) pObjectVec );
}

/*
 * Class:     com_southernstars_sscore_JSSObjectArray
 * Method:    destroy
 * Signature: ()V
 */

JNIEXPORT void JNICALL Java_com_southernstars_sscore_JSSObjectArray_destroy ( JNIEnv *pEnv, jobject pJObjectArray )
{
    SSObjectVec *pObjectVec = (SSObjectVec *) GetLongField ( pEnv, pJObjectArray, "pObjectVec" );
    SetLongField ( pEnv, pJObjectArray, "pObjectVec", 0 );
    delete pObjectVec;
}

/*
 * Class:     com_southernstars_sscore_JSSObjectArray
 * Method:    size
 * Signature: ()I
 */

JNIEXPORT jint JNICALL Java_com_southernstars_sscore_JSSObjectArray_size ( JNIEnv *pEnv, jobject pJObjectArray )
{
    SSObjectVec *pObjectVec = (SSObjectVec *) GetLongField ( pEnv, pJObjectArray, "pObjectVec" );
    return pObjectVec ? (size_t) pObjectVec->size() : 0;
}

/*
 * Class:     com_southernstars_sscore_JSSObjectArray
 * Method:    getObject
 * Signature: (I)Lcom/southernstars/sscore/JSSObject;
 */
JNIEXPORT jobject JNICALL Java_com_southernstars_sscore_JSSObjectArray_getObject ( JNIEnv *pEnv, jobject pJSSObjectArray, jint index )
{
    SSObjectVec *pObjectVec = (SSObjectVec *) GetLongField ( pEnv, pJSSObjectArray, "pObjectVec" );
    SSObject *pObject = pObjectVec ? pObjectVec->get ( index ) : nullptr;
    return SSObjectToJSSObject ( pEnv, pObject );
}

/*
 * Class:     com_southernstars_sscore_JSSObjectArray
 * Method:    importFromCSV
 * Signature: (Ljava/lang/String;)I
 */

JNIEXPORT jint JNICALL Java_com_southernstars_sscore_JSSObjectArray_importFromCSV ( JNIEnv *pEnv, jobject pJObjectArray, jstring pJPath )
{
    const char *pPath = pEnv->GetStringUTFChars ( pJPath, nullptr );
    SSObjectVec *pObjectVec = (SSObjectVec *) GetLongField ( pEnv, pJObjectArray, "pObjectVec" );
    int n = SSImportObjectsFromCSV ( string ( pPath ), *pObjectVec );
    pEnv->ReleaseStringUTFChars ( pJPath, pPath );
    return n;
}

/*
 * Class:     com_southernstars_sscore_JSSObjectArray
 * Method:    exportToCSV
 * Signature: (Ljava/lang/String;)I
 */

JNIEXPORT jint JNICALL Java_com_southernstars_sscore_JSSObjectArray_exportToCSV ( JNIEnv *pEnv, jobject pJObjectArray, jstring pJPath )
{
    const char *pPath = pEnv->GetStringUTFChars ( pJPath, nullptr );
    SSObjectVec *pObjectVec = (SSObjectVec *) GetLongField ( pEnv, pJObjectArray, "pObjectVec" );
    int n = SSExportObjectsToCSV ( string ( pPath ), *pObjectVec );
    pEnv->ReleaseStringUTFChars ( pJPath, pPath );
    return n;
}

/*
 * Class:     com_southernstars_sscore_JSSObjectArray
 * Method:    importFromTLE
 * Signature: (Ljava/lang/String;)I
 */

JNIEXPORT jint JNICALL Java_com_southernstars_sscore_JSSObjectArray_importFromTLE ( JNIEnv *pEnv, jobject pJObjectArray, jstring pJPath )
{
    const char *pPath = pEnv->GetStringUTFChars ( pJPath, nullptr );
    SSObjectVec *pObjectVec = (SSObjectVec *) GetLongField ( pEnv, pJObjectArray, "pObjectVec" );
    int n = SSImportSatellitesFromTLE ( string ( pPath ), *pObjectVec );
    pEnv->ReleaseStringUTFChars ( pJPath, pPath );
    return n;
}

/*
 * Class:     com_southernstars_sscore_JSSObjectArray
 * Method:    importMcNames
 * Signature: (Ljava/lang/String;)I
 */

JNIEXPORT jint JNICALL Java_com_southernstars_sscore_JSSObjectArray_importMcNames ( JNIEnv *pEnv, jobject pJObjectArray, jstring pJPath )
{
    const char *pPath = pEnv->GetStringUTFChars ( pJPath, nullptr );
    SSObjectVec *pObjectVec = (SSObjectVec *) GetLongField ( pEnv, pJObjectArray, "pObjectVec" );
    int n = SSImportMcNames ( string ( pPath ), *pObjectVec );
    pEnv->ReleaseStringUTFChars ( pJPath, pPath );
    return n;
}

/*
 * Class:     com_southernstars_sscore_JSSObjectArray
 * Method:    initAssetManager
 * Signature: (Landroid/content/res/AssetManager;)Z
 */

JNIEXPORT jboolean JNICALL Java_com_southernstars_sscore_JSSObjectArray_initAssetManager ( JNIEnv *pEnv, jclass pJObjectArray, jobject pAssetMgr )
{
    return initAssetManager ( pEnv, pJObjectArray, pAssetMgr );
}
