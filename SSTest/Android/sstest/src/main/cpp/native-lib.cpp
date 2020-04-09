#include <jni.h>
#include <string>

#include "SSTest.h"

extern "C" JNIEXPORT jstring JNICALL
Java_com_southernstars_sstest_MainActivity_stringFromJNI(
        JNIEnv* env,
        jobject /* this */) {

    // This calls our main test program, but we have graduated beyond this test!
    // SSTestMain ( "", "" );

    std::string hello = "Hello from SSCore!";
    return env->NewStringUTF(hello.c_str());
}
