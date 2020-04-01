# SSCore

**SSCore** is Southern Stars' astronomical core code library. It contains routines for astronomical calculations like time conversion, coordinate transformation, orbital mechanics, lunar and planetary position computation. It also contains functions for importing, storing, and combining data from a variety of astronomical catalogs.

SSCore is written in highly portable, modern C++.  It has been tested on MacOS, iOS, Android, Linux, and Windows.  It uses common STL types and language features (string, vector, map; operator overloading) and stack-based memory management wherever possible to produce compact, highly efficient, optimized code.

# SSData

**SSData** is a collection of data on well-known astronomical objects, compiled and carefully vetted from a variety of modern astronomical catalogs and data sources.  It includes:

- **_Solar System Objects:_** the major planets (including Pluto!) and their natural satellites, with orbital and physical characteristics from [JPL Solar System Dynamics.](https://ssd.jpl.nasa.gov)
- **_Stars:_** the brightest and nearest stars, compiled from NASA's [SKY2000 Master Star Catalog](https://ui.adsabs.harvard.edu/abs/2015yCat.5145....0M/abstract), Hipparcos, RECONS, and other sources.
- **_Deep Sky Objects:_** the Messier and Caldwell objects, with data from Wolfgang Steinicke's [Revised NGC and IC](http://www.klima-luft.de/steinicke/index_e.htm) catalogs, and enhanced using data from a few other sources.
- **_Constellations:_** the 88 IAU-sanctioned constellations, including official IAU [boundary](https://watcheroftheskies.net/constellations/bound_18.txt) and [shape]( http://mkweb.bcgsc.ca/constellations/constellation.shapes.iau.txt) data.

These files are all CSV-formatted text.  They can easily be imported into a spreadsheet or edited manually.  SSCore contains routines for reading and writing them as well.

# Version History

Version 1.0, 10 Apr 2020: Initial public release.
