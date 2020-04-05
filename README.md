SSCore
======

**SSCore** is Southern Stars' astronomical core code library. It contains routines for astronomical calculations like time conversion, coordinate transformation, orbital mechanics, lunar and planetary position computation. It also contains functions for importing, storing, and combining data from a variety of astronomical catalogs.

SSCore is written in highly portable, modern C++.  It has been tested on MacOS, iOS, Android, Linux, and Windows.  It uses common STL types and language features (string, vector, map; operator overloading) and stack-based memory management wherever possible to produce compact, highly efficient, optimized code.

License
-------

SSCore is Copyright © 2020 Southern Stars Group, LLC. All rights reserved.  Southern Stars grants you a license to use the SSCore code for any personal, private, research, or other **non-commercial** purpose.

If you wish to include any part of the SSCore code in a commercial product, please contact Southern Stars Group, LLC (timd@southernstars.com) for a commercial license.

SSCode
------

This directory contains the source code.  Here's an overview of the C++ classes inside the source files:

- **_SSAngle:_** Classes for converting angular values from radians to degress/hours, minutes, seconds; and vice-versa.
- **_SSConstellation:_** This subclass of SSObject stores information for constellations and asterisms, including the official IAU constellation names, abbreviations, boundaries; and shape/figure data.
- **_SSCoords:_** This class converts rectangular and spherical coordinates between different astronomical reference frames (fundamental/ICRS, equatorial, ecliptic, galactic, local horizon) at a particular time and geographic location.
- **_SSIdentifier:_** This class represents object identifiers used in a wide variety of astronomical catalogs with a unified system of 64-bit integers, and contains methods for converting identifiers from string representations ("HR 7001", "NGC 1976", etc.) to 64-bit integers and vice-versa.
- **_SSJPLDEphemeris:_** This class reads JPL's binary DE43x series of ephemeris files and computes very fast, accurate lunar and planetary positions from them.
- **_SSDynamics:_** Implements solar system and stellar dynamics. Computes position, motion, and visual magnitude of all types of celestial objects, as seen by an observer from a particular time and location, in a unified fashion.
- **_SSMatrix:_** Represents a 3x3 matrix, with routines for performing matrix and vector-matrix arithmetic.
- **_SSObject:_** Base class for all types of celestial objects (stars, planets, constellations, etc.)
- **_SSOrbit:_** This class stores Keplerian orbital elements, computes position/velocity at a given time from them, and vice-versa.
- **_SSPlanet:_** This subclass of SSObject represents all solar system objects (not just planets, but also moons, asteroids, comets, satellites, etc.)
- **_SSStar:_** This subclass of SSObject represents all objects outside the solar system, including stars, star clusters, nebulae, and galaxies. SSStar has special subclasses for double and variable stars, and for deep sky objects.
- **_SSTime:_** Classes for converting between Julian Dates and calendar dates/times; and between civil (UTC) and dynamic time (TDT).
- **_SSTLE:_** Routines for reading satellite orbital elements from TLE (Two/Three-Line Element) files, and computing satellite position/velocity from them using the SGP, SGP4, and SDP4 orbit models; and vice-versa.
- **_SSUtilities:_** A few useful string manipulation, angle conversion, and other utility functions that are not present in standard C++11.
- **_SSVector:_** Classes for converting points between spherical and rectangular coordinates, and for performing vector arithmetic operations.


SSData
------

**SSData** is a collection of data on well-known astronomical objects, compiled and carefully vetted from a variety of modern astronomical catalogs and data sources.  It includes:

- **_Solar System Objects:_** the major planets (including Pluto!) and their natural satellites, with orbital and physical characteristics from [JPL Solar System Dynamics.](https://ssd.jpl.nasa.gov). Also current asteroid and comet data from the [Minor Planet Center](https://www.minorplanetcenter.net/iau/mpc.html), a selection of satellite TLE files from [CelesTrak](http://www.celestrak.com) and [N2YO](https://www.n2yo.com), and JPL's latest planetary and lunar [ephemeris](https://ssd.jpl.nasa.gov/?planet_eph_export).
- **_Stars:_** the brightest and nearest stars, compiled from NASA's [SKY2000 Master Star Catalog](https://ui.adsabs.harvard.edu/abs/2015yCat.5145....0M/abstract), Hipparcos, RECONS, and other sources; with official IAU [star names](http://www.pas.rochester.edu/~emamajek/WGSN/IAU-CSN.txt).
- **_Deep Sky Objects:_** the Messier and Caldwell objects, with data from Wolfgang Steinicke's [Revised NGC and IC](http://www.klima-luft.de/steinicke/index_e.htm) catalogs, and enhanced using data from a few other sources.
- **_Constellations:_** the 88 IAU-sanctioned constellations, including official IAU [boundary](https://watcheroftheskies.net/constellations/bound_18.txt) and [shape]( http://mkweb.bcgsc.ca/constellations/constellation.shapes.iau.txt) data.

These files are mostly CSV-formatted text.  They can easily be imported into a spreadsheet or edited manually.  SSCore contains routines for reading and writing them as well.

SSTest
------

This directory contains a test program (SSTest.cpp), which hopefully serves as an example of how to use the C++ classes in the SSCode directory.  It also contains build environments for MacOS, iOS, Android, Windows, and Linux.  Build instructions for each platform:

- **_Android:_** instructions TBD
- **_MacOS:_** open SSTest.codeproj in Xcode directory with Xcode 10 or later. Select Product -> Run from Xcode.
- **_iOS:_** instructions TBD
- **_Linux:_** cd to SSTest/Linux directory; then type "make" (without quotes!)  After build completes, type "./SSTest ../../SSData ." (without quotes!)
- **_Windows:_** instructions TBD

Version History
---------------

Version 1.0, 10 Apr 2020: Initial public release.
