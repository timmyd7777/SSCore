SSCore Android Test App
=======================

This is the SSCore test app for Android. It includes Java wrappers and JNI code for the C++ classes which makes them useable from Java and/or Kotlin-based Android apps.

Building and Running
--------------------

Open the **SSTest/Android** project with Android Studio 3.6 or later. Wait a minute for Gradle sync to complete. Then from the **Build** menu, select **Make Project.** After the build completes, go to the **Run** menu and select **Debug 'sstest'**. This will launch a test program in the Android Emulator or on your device, if you have one connected.  There is no GUI, just text output.

Selected files from the **SSData** folder are copied into the Android SSTest app APK **assets** directory as part of the build process (see **sstest/build.gradle**). Several of the tests read files from this folder, so they needs to be copied to the APK. Functions for reading files in the assets folder from native code using the Android Asset Manager are in **sstest/src/main/jni/JNIUtilities.cpp**.

Wrapping SSCore C++ in Java
---------------------------

**MainActivity.kt** shows how to call the SSCore C++ classes from Kotlin using Java wrapper classes in **sstest/src/main/java/com/southernstars/sscore**. This is a work in progress, but there are enough Java and JNI wrappers around the essential C++ classes to demonstrate the design pattern.

The wrapper works like this: for every C++ class (`SSTime, SSAngle`, etc.) there is a corresponding Java class, named with a ‘J’ in front (`JSSTime, JSSAngle`, etc.). Those Java classes instantiate the equivalent C++ classes, and call the corresponding methods, via a JNI bridge.
 
Here an example in C++:
 
    SSTime now = SSTime::fromSystem();
    double jed = now.getJulianEphemerisDate();
 
In Java, it becomes the following:
 
    JSSTime now = JSSTime.fromSystem();
    double jed = now.getJulianEphemerisDate());
 
which look like this in Kotlin:
 
    let now = CSSTimeFromSystem()
    let jed = CSSTimeGetJulianEphemerisDate ( now )

A few trivial methods are implemented directly in Java, where the overhead of passing data back-and-forth through JNI would be greater than the slowdown introduced by the java runtime. **JSSVector.java** contains some good examples.  For the most part, the Java SSCore API is closer to the original C++ API than the C wrapper API needed for the Swift iOS test app.

The **sstest/build.gradle** file contains a **'buildJNIHeaders'** task. This task generates JNI headers from the SSCore Java classes, and outputs them in the **sstest/src/main/jni** directory. You can run this task from Android Studio to regenerate the JNI headers after adding to or changing the SSCore Java classes. To run **buildJNIHeaders**, you'll need JDK version 8 or earlier installed (Oracle removed the 'javah' tool in later JDK versions). None of this matters if you are just _using_ the existing Java SSCore classes, without _modifying_ them.

Objects and Object Arrays
-------------------------

The stickiest part of this wrapper is the `SSObject` class hierarchy, which represents various kinds of celestial objects (stars, planets, moons, galaxies, etc.).  These are implemented as `shared_ptr<SSObject>` in C++, and stored as private pointers to native objects inside the java **JSSObject** class. You can use these pointers from Java without needing the native C++ internals.

The `JSSObjectArray` Java object is a wrapper around a vector of pointers to C++ SSObjects. To get a JSSObject from the array at a particular index (from 0 to number of array elements - 1), call CSSObjectGetFromArray(). This functions returns a pointer to the object, or a null pointer if the array index is out of bounds. To drill down to specfic properties of the object, call the appropriate accessor method.

Here is a C++ example which reads an vector of objects from a data file in CSV format, gets a pointer to the first object, obtains the apparent direction vector to that object, and finally releases memory for the entire object vector.

    SSObjectVec objvec;
    int n = SSImportObjectsFromCSV ( "path/to/data/file.csv", objvec )
    SSObjectPtr pObject = objvec[0];
    SSVector dir = pObject->getDirection();
    // memory is freed automatically when objvec goes out of scope`

Here is the equivalent Java wrapper code:

    JSSObjectArray jObjArr = JSSObjectArray();
    int n = jObjArr.importFromCSV ( "path/to/data/file.csv", jObjArr );
    JSSObject jObj = pObjArr.getObject ( 0 );
    JSSVector dir = jObj.getDirection();
    // memory is freed automatically when jObjArr goes out of scope`

And finally Kotlin calls to the Java wrapper:

    var objarr = JSSObjectArray()
    var n = objarr.importFromCSV ( "path/to/data/file.csv" )
    var obj = objarr.getObject ( 0 )
    var dir = obj?.getDirection() ?: JSSVector()
    // memory is freed automatically when objarr goes out of scope`

After the object array is freed, the object pointer previously extracted from it will be invalid. The Java wrapper classes should handle this sitation gracefully, rather than crashing. For example, `JSSObject.getType()` will return `kTypeNonexistent`, if the underlying pointer is invalid.

Version History
---------------

Version 1.0, 12 Apr 2020: Initial public release.
