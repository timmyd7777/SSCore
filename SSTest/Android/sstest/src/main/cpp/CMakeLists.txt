# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html

# Sets the minimum version of CMake required to build the native library.

cmake_minimum_required(VERSION 3.4.1)

# Creates and names a library, sets it as either STATIC
# or SHARED, and provides the relative paths to its source code.
# You can define multiple libraries, and CMake builds them for you.
# Gradle automatically packages shared libraries with your APK.

add_library( # Sets the name of the library.
             sscore-lib

             # Sets the library as a shared library.
             SHARED

             # Provides a relative path to your source file(s).
             native-lib.cpp
             ../../../../../../SSCode/SSAngle.cpp
             ../../../../../../SSCode/SSConstellation.cpp
             ../../../../../../SSCode/SSCoordinates.cpp
             ../../../../../../SSCode/SSEvent.cpp
             ../../../../../../SSCode/SSIdentifier.cpp
             ../../../../../../SSCode/SSImportMPC.cpp
             ../../../../../../SSCode/SSJPLDEphemeris.cpp
             ../../../../../../SSCode/SSMatrix.cpp
             ../../../../../../SSCode/SSMoonEphemeris.cpp
             ../../../../../../SSCode/SSObject.cpp
             ../../../../../../SSCode/SSOrbit.cpp
             ../../../../../../SSCode/SSPlanet.cpp
             ../../../../../../SSCode/SSPSEphemeris.cpp
             ../../../../../../SSCode/SSStar.cpp
             ../../../../../../SSCode/SSTime.cpp
             ../../../../../../SSCode/SSTLE.cpp
             ../../../../../../SSCode/SSUtilities.cpp
             ../../../../../../SSCode/SSVector.cpp
             ../../../../../../SSCode/VSOP2013/ELPMPP02.cpp
             ../../../../../../SSCode/VSOP2013/VSOP2013.cpp
             ../../../../../../SSCode/VSOP2013/VSOP2013p1.cpp
             ../../../../../../SSCode/VSOP2013/VSOP2013p2.cpp
             ../../../../../../SSCode/VSOP2013/VSOP2013p3.cpp
             ../../../../../../SSCode/VSOP2013/VSOP2013p4.cpp
             ../../../../../../SSCode/VSOP2013/VSOP2013p5.cpp
             ../../../../../../SSCode/VSOP2013/VSOP2013p6.cpp
             ../../../../../../SSCode/VSOP2013/VSOP2013p7.cpp
             ../../../../../../SSCode/VSOP2013/VSOP2013p8.cpp
             ../../../../../../SSCode/VSOP2013/VSOP2013p9.cpp
             ../../../../../../SSTest/SSTest.cpp
             ../jni/com_southernstars_sscore_JSSAngle.cpp
             ../jni/com_southernstars_sscore_JSSCoordinates.cpp
             ../jni/com_southernstars_sscore_JSSDate.cpp
             ../jni/com_southernstars_sscore_JSSDegMinSec.cpp
             ../jni/com_southernstars_sscore_JSSEvent.cpp
             ../jni/com_southernstars_sscore_JSSEventTime.cpp
             ../jni/com_southernstars_sscore_JSSHourMinSec.cpp
             ../jni/com_southernstars_sscore_JSSIdentifier.cpp
             ../jni/com_southernstars_sscore_JSSJPLDEphemeris.cpp
             ../jni/com_southernstars_sscore_JSSMatrix.cpp
             ../jni/com_southernstars_sscore_JSSObject.cpp
             ../jni/com_southernstars_sscore_JSSObjectArray.cpp
             ../jni/com_southernstars_sscore_JSSPass.cpp
             ../jni/com_southernstars_sscore_JSSRTS.cpp
             ../jni/com_southernstars_sscore_JSSSpherical.cpp
             ../jni/com_southernstars_sscore_JSSTime.cpp
             ../jni/com_southernstars_sscore_JSSVector.cpp
             ../jni/JNIUtilities.cpp
        )

# For CMake to locate your header files during compile time,
# you need to use the include_directories() command and include the path to your header files:

include_directories (
        ../../../../../../SSCode/
        ../../../../../../SSCode/VSOP2013
        ../../../../../../SSTest/
        ../jni/
    )

# Searches for a specified prebuilt library and stores the path as a
# variable. Because CMake includes system libraries in the search path by
# default, you only need to specify the name of the public NDK library
# you want to add. CMake verifies that the library exists before
# completing its build.

find_library( # Sets the name of the path variable.
              log-lib
              # Specifies the name of the NDK library that
              # you want CMake to locate.
              log )

find_library(
        android-lib
        android )

# Specifies libraries CMake should link to your target library. You
# can link multiple libraries, such as libraries you define in this
# build script, prebuilt third-party libraries, or system libraries.

target_link_libraries( # Specifies the target library.
                       sscore-lib

                       # Links the target library to the log library
                       # and android library included in the NDK.
                       ${log-lib}
                       ${android-lib}
        )