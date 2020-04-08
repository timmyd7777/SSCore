#include <jni.h>
#include <string>

extern "C" JNIEXPORT jstring JNICALL
Java_com_southernstars_sstest_MainActivity_stringFromJNI(
        JNIEnv* env,
        jobject /* this */) {
    std::string hello = "Hello from C++ SSCore!";
    return env->NewStringUTF(hello.c_str());
}
