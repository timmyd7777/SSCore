SSCode iOS Test App
===================

This is the SSCore test app for iOS. It includes a plain C wrapper for the C++ classes which makes them useable from Swift. There is no need for this wrapper for Objective-C iOS apps, since Objective-C can can call C++ code directly.

Building and Running
--------------------

Open **SSTest.xcodeproj** in this directory with Xcode 10 or later. From Xcode's **Product** menu, select **Run**.  This will launch a test program in the iPhone Simulator.  There is no GUI, just text output.

The entire **SSData** folder is copied into the iOS SSTest app bundle as part of the build process. Several of the tests read files from this folder, so it needs to be copied to the iPhone file system.

Wrapping SSCore C++ in C
------------------------

**ContentView.swift** shows how to call the SSCore C++ classes from Swift using the plain C wrapper in **SSCore.cpp** and **SSCore.h** This is a work in progress, but contains wrappers around the essential C++ API classes. It's enough to demonstrate the design pattern.

The wrapper works like this: for every C++ classe (`SSTime, SSAngle`, etc.) there is a corresponding C struct, named with a ‘C’ in front (`CSSTime, CSSAngle`, etc.). That struct is always passed as the first argument to C functions which encapsulate the equivalent methods of the C++ classes.  C++ constructors or static methods are implemented as C functions which take no such struct input as the first argument.
 
Here an example in C++:
 
`SSTime now = SSTime::fromSystem();
`double jed = now.getJulianEphemerisDate();
 
In C, it becomes the following C wrapper calls:
 
`CSSTime now = CSSTimeFromSystem();
`double jed = CSSTimeGetJulianEphemerisDate ( now );
 
which look like this in Swift:
 
`let now = CSSTimeFromSystem()
`let jed = CSSTimeGetJulianEphemerisDate ( now )

Objects and Object Arrays
-------------------------

The hairiest part of this is the `SSObject` class hierarchy, which represents various kinds of celestial objects (stars, planets, moons, galaxies, etc.).  These are implemented as `shared_ptr<SSObject>` in C++, and cast as pointers to opaque structs (`CSSObjectPtr`) in C. You can use these opaque pointers from Swift without needing the internal C++ internals.

The `CSSObjectArray` object is an opaque C struct wrapper around a vector of pointers to C++ SSObject. To get a CSSObjectPtr from the array at a particular index (from 0 to number of array elements - 1), call CSSObjectGetFromArray(). This functions returns a pointer to the object, or a null pointer if the array index is out of bounds. To drill down to specfic properties of the object, call the appropriate accessor method wrapper.

Here is a C++ example which reads an vector of objects from a data file in CSV format, gets a pointer to the first object, obtains the apparent direction vector to that object, and finally releases memory for the entire object vector.

`SSObjectVec objvec;
`int n = SSImportObjectsFromCSV ( "path/to/data/file.csv", objvec )
`SSObjectPtr pObject = objvec[0];
`SSVector dir = pObject->getDirection();
`// memory is freed automatically when objvec goes out of scope`

Here is the equivalent C wrapper code:

`CSSObjectArray pObjArr = CSSObjectArrayCreate();
`int n = CSSImportObjectsFromCSV ( "path/to/data/file.csv", pObjArr );
`CSSObjectPtr pObj = CSSObjectGetFromArray ( pObjArr, 0 );
`CSSVector dir = CSSObjectGetDirection ( pObj );
`CSSObjectArrayDestroy ( pObjArr )

And finally Swift calls to the C wrapper:

`var pObjArr:CSSObjectVec? = CSSObjectArrayCreate()
`var n = CSSImportObjectsFromCSV ( "path/to/data/file.csv", pObjArr );
`var pObj:CSSObject? = CSSObjectGetFromArray ( pObjArr, 0 )
`var dir = CSSObjectGetDirection ( pObj )
`CSSObjectArrayDestroy ( pObjArr )

After the object array is freed, the object pointer previously extracted from it will be invalid. The C wrapper functions should handle this sitation gracefully, rather than crashing. For example, `CSSObjectGetType ( pObj )` will return `kCSSTypeNonexistent`, if the object pointer passed into it is invalid.

Version History
---------------

Version 1.0, 12 Apr 2020: Initial public release.
