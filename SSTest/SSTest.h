// SSTest.h
// SSTest
//
// Created by Tim DeBenedictis on 4/5/20.
// Copyright © 2020 Southern Stars. All rights reserved.
//
// This is the entry point to SSTest when called from the Swift iOS environment
// or from the Android JNI environment. Not needed on other platforms.

#ifndef SSTest_h
#define SSTest_h

#ifdef __cplusplus
extern "C" {
#endif

int SSTestMain ( const char *ssdatapath, const char *outpath );

#ifdef __cplusplus
}
#endif

#endif /* SSTest_h */
