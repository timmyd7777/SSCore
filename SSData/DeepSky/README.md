Deep Sky Object Data
====================

This directory contains essential data on the most prominent and well known deep sky objects in Southern Stars CSV format.  These files exist primarily for testing SSCore import/export functionality, but may be generally useful in their own right.

You may use this data free of charge for any purpose as you see fit.  If it's helpful to you, drop me a note!

_Tim DeBenedictis (timd@southernstars.com), 6 Apr 2020_

Messier.csv
-----------

This Messier catalog is primarily based on data from Wolfgang Steinicke's [Revised NGC and IC](http://www.klima-luft.de/steinicke/index_e.htm), with additions from the following catalogs:

- **_Open clusters:_** added proper motions, distances, radial velocities from Wilton Dias' catalog of [Open Clusters and Galactic Structure](https://wilton.unifei.edu.br/ocdb/clusters.txt).
- **_Globular clusters:_** added distances, radial velocities, spectral types from William Harris' catalog of [Globular Clusters in the Milky Way](http://physwww.mcmaster.ca/~harris/mwgc.dat).
- **_Planetary nebulae:_** added distances, radial velocities, PNG numbers from the [Strasbourg-ESO catalog of Galactic Planetary Nebulae](https://cdsarc.unistra.fr/ftp/V/84)

A few object-specific edits were made:

- M 1 (Crab Nebula): added 2.0 kpc distance from Wikipedia.
- M 8 (Lagoon Nebula): added 1.25 kpc distance from Wikipedia.
- M 16 (Eagle Nebula): changed type from OC to BN.
- M 20 (Trifid Nebula): obtained RA and Dec from Wikipedia.
- M 24 (Sagittarius Star Cloud): added 3.0 kpc distance and 90 arcmin size from Wikipedia.
- M 40 (Winnecke 4): added data from Wikipedia for A component.
- M 42 (Orion Nebula): distance 412 pc from Wikipedia.
- M 43 (Orion Nebula): distance 412 pc from Wikipedia.
- M 45 (Pleiades): data added from Wilton Dias Open Cluster catalog, with V magnitude +1.6 from Wikipedia.
- M 73: changed type to OC and added 770 pc distance from Wikipedia.
- M 76: added NGC 651 designation.
- M 78: added distance 415 pc from Wikipedia.

The column format is:

Type, RA, Dec, pmRA, pmDec, V, B, Dist, RV, Type, Maj, Min, PA, IDs, Names

- **_Type_** is object type: open cluster (OC), globular cluster (GC), bright nebula (BN), dark nebula (DN), planetary nebula (PN), galaxy (GX)
- **_RA_** is J2000 right ascension
- **_Dec_** is J2000 declination
- **_pmRA_** is proper motion in R.A., seconds per year
- **_pmDec_** is proper motion in Dec., arcsec per year
- **_V_** is Johnson Visual magnitude
- **_B_** is Johnson Blue magnitude
- **_Dist_** is distance in parsecs
- **_RV_** is radial velocity in km/sec
- **_Type_** is galaxy morphological type, or cluster type.
- **_Maj_** is apparent ellipse major axis in arcminutes.
- **_Min_** is apparent ellipse minor axis in aarcminutes.
- **_PA_** is position angle of major axis from North in degrees.
- **_IDs** are semicolon-delimited catalog identifiers in preferred order: Messier; Caldwell; NGC; IC
- **_Names_** are the object's semicolon-delimited common name(s), if any.

Empty fields indicate unknown or missing values.

**Revision History**

- Version 1.0 (27 Mar 2020): initial version
- Version 1.1 (30 Mar 2020): corrected erroneous proper motions for open clusters with RV.
- Version 1.2 (6 Apr 2020): Moved comments from CSV file header into this README.
- Version 1.2.1 (18 Jun 2020): added 90 arcmin size to M 24 from Wikipedia.
- Version 1.2.2 (12 Aug 2020): correct coordinates for M 20 from Wikipedia.
- Version 2.0 (9 Jul 2026): add column headers, combine IDs and Names into semicolon-delimited columns; fix distance to M42 and M43.

Caldwell.csv
------------

This Caldwell catalog is based on the same data sources as **Messier.csv**, with edits for the  following specific objects. I've also added the Magellanic Clouds; while these objects are not part of Caldwell's list, they are too important to ignore.

- C 4 (Iris Nebula): distance 400 pc from Wikipedia.
- C 9 (Cave Nebula): added with data from Wikipedia.
- C 11 (Bubble Nebula): distance 3400 pc from Wikipedia.
- C 14 (Double Cluster): has two entries; NGC 884 was added from Steinicke's NGC-IC.
- C 20 (North America Nebula): distance 675 pc from Wikipedia.
- C 27 (Crescent Nebula): distance 1500 pc from Wikipedia.
- C 31 (Flaming Star Nebula): distance 460 pc from Wikipedia.
- C 33 (Western Veil Nebula): distance 740 pc from Wikipedia.
- C 34 (Eastern Veil Nebula): distance 740 pc from Wikipedia.
- C 41 (Hyades) was added with data from Wilton Dias Open Cluster catalog, with magnitude +0.5 from Wikipedia
- C 46 (Hubble's Variable Nebula): V magnitude +9.0 and distance 770 pc from Wikipedia.
- C 49 (Rosette Nebula): V magnitude +9.0 and distance 1600 pc from Wikipedia.
- C 50 (NGC 2244): removed NGC 2239 and Rosette Nebula designation removed
- C 68 (NGC 6279): missing V magnitude and distance.
- C 69 (Bug Nebula): distance 1.04 kpc from Wikipedia.
- C 92 (Eta Carinae Nebula): distance 2600 pc from Wikipedia.
- C 99 (Coalsack): added with data from Wikipedia
- C 103 (Tarantula Nebula): distance 49 kpc from Wikipedia.
- C 106 (47 Tucanae): removed extraneous 47 Tuc designation.
- C 109 (NGC 3195): distance 1.7 kpc from Wikipedia.
- LMC: data from Principal Galaxy Catalog

The column format is identical to that of **Messer.csv**, above.

**Revision History**

- Version 1.0 (27 Mar 2020): initial version
- Version 1.1 (30 Mar 2020): corrected erroneous proper motions for open clusters with RV; added LMC and SMC.
- Version 1.2 (6 Apr 2020): Moved comments from CSV file header into this README.
- Version 1.2.1 (19 June 2020): Added dimensions of C99 (Coalsack) from Wikipedia.

MCNGCIC.csv
-----------

This file is the Messier and Caldwell catalogs above, supplemented with all other NGC and IC objects from Wolfgang Steinicke's [Revised NGC and IC](http://www.klima-luft.de/steinicke/index_e.htm). This combined catalog contains 13,033 entries.

NGC-IC objects which are duplicates of other objects are discarded; instead, the other object receives an additional NGC or IC identifier: that of the duplicate. Nonexistent NGC-IC objects have type "NO" in the first column. The goal is to have every NGC-IC entry represented once, but not more than once. This catalog adds distances, proper motions, and radial velocities from other catalogs for open and globulars clusters and planetary nebulae, as described above for **Messier.csv**.

The column format is identical to that of **Messer.csv**, above.

**Revision History**

- Version 1.0 (23 Jun 2026): initial version

Names.csv
---------

This deep sky object name list is based primarily on Wolfgang Steinicke's NGC-IC name list in Table 6 on [this page](http://www.klima-luft.de/steinicke/index_e.htm).  I added a few additional names for Messier and Caldwell objects from Wikipedia:

- [Caldwell Catalog](https://en.wikipedia.org/wiki/Caldwell_catalogue)
- [Messier Catalog](https://en.wikipedia.org/wiki/Messier_object)

**Revision History**

- Version 1.0 (27 Mar 2020): initial version

MilkyWay.csv
------------

This file was adapted from original data provided by Charles Turner in 2002, used in the venerable [Voyager](https://www.carinasoft.com/voyager.html) desktop planetarium. Turner's data was based on digitizations of Milky Way boundaries in modern star atlases. The original author of these Milky Way boundaries is unclear, but is probably [Wil Tirion](https://www.wil-tirion.com/wil-tirion.com/Homepage.html) based on comparisons with the Milky Way in many Tirion star atlases.

This boundary data divides the Milky Way into 85 regions, including the Large and Small Magellanic Clouds, and the Coalsack dark nebula. Each region is a polygon defined by a set of points. Each point may be an edge, or an inner polygon-closing point. Use all points to draw a region as a filled polygon. Use only the edge points to draw an outline of the Milky Way (no fill).

Region 83 is the SMC, 84 is the LMC, and 85 is the Coalsack. 

The column format is RA, Dec, Region, Edge:

- **_RA_** is J2000 right ascension in decimal degrees
- **_Dec_** is J2000 declination in decimal egrees
- **_Region_** is Milky Way region number
- **_Edge_** is 1 if this is a Milky Way boundary edge point, or 0 if this is an internal polygon-closing point.

**Revision History**

- Version 1.0 (23 June 2026): initial version

Nebula Contours
---------------

The **Nebulae** folder contains two files adapted from original data provided by Charles Turner in 2002, also used in Voyager. Turner digitized the contours of 127 bright and dark nebulae drawn on printed star atlases. The original author of those nebula contours is unclear, but they closely resemble the contours drawn by Wil Tirion (in e.g. Sky Atlas 2000).

The first file, **Contours.csv**, contains the points which make up each of the 127 nebula contours. Its column format is RA, Dec, Region:

- **_RA_** is J2000 right ascension in decimal degrees
- **_Dec_** is J2000 declination in decimal egrees
- **_Contour_** is contour number

The second file, **Index.csv**, contains the catalog number(s) of the object which each contour represents. Its column format is Countour, IDs:

- **_Contour_** is contour number
- **_IDs_** are the catalog number(s) of the object which the contour represents, separated by semicolons

For example, contour 96 represents NGC 2237, 2238, and 2239 (the Rosette Nebula). Most contours only represent a single catalog number. Note that some objects have multiple contours. For example, M 42 (the Orion Nebula) has two contours, which represent the bright inner region, and fainter surrounding extended nebulosity.

**Revision History**

- Version 1.0 (1 July 2026): initial version
