SSCode MacOS Test App
=====================

This is the SSCore test app for MacOS.

Building and Running
--------------------

Open **SSTest.xcodeproj** in the MacOS directory with Xcode 10 or later. From Xcode's **Product** menu, select **Run**.  This is a console app; there is no GUI. All output goes into the Xcode terminal (if you're running from with Xcode) or to your Terminal window (if you're running it from the command line, standalone.)

NOTE if you're running SSTest within Xcode: By default, Xcode places the SSTest executable under `~/Library/Developer/Xcode/DerivedData`, so SSTest won't know where to find the `SSData` folder.  To fix this, from Xcode's **File** menu, select **Project Settings...**, then change **Derived Data** to **Project-relative Location** (DerivedData). This puts the `DerivedData` folder, which contains the SSTest executable, into the `SSTest/MacOS` folder adjacent to `SSData`.

Version History
---------------

Version 1.0, 12 Apr 2020: Initial public release.
