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

Type, RA, Dec, pmRA, pmDec, V, B, Dist, RV, Type, Maj, Min, PA, Ident1, Ident2, Ident3, Common1, Common2

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
- **_Ident1, Ident2, etc._** are catalog identifiers in preferred order: Messier, Caldwell, NGC, IC
- **_Common1, Common2, etc._** are the objects common name(s), if any.

Empty fields indicate unknown or missing values.

**Revision History**

- Version 1.0 (27 Mar 2020): initial version
- Version 1.1 (30 Mar 2020): corrected erroneous proper motions for open clusters with RV.
- Version 1.2 (6 Apr 2020): Moved comments from CSV file header into this README.
- Version 1.2.1 (18 Jun 2020): added 90 arcmin size to M 24 from Wikipedia.
- Version 1.2.2 (12 Aug 2020): correct coordinates for M 20 from Wikipedia.

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

Names.csv
---------

This deep sky object name list is based primarily on Wolfgang Steinicke's NGC-IC name list in Table 6 on [this page](http://www.klima-luft.de/steinicke/index_e.htm).  I added a few additional names for Messier and Caldwell objects from Wikipedia:

- [Caldwell Catalog](https://en.wikipedia.org/wiki/Caldwell_catalogue)
- [Messier Catalog](https://en.wikipedia.org/wiki/Messier_object)

**Revision History**

- Version 1.0 (27 Mar 2020): initial version
