Star Data
=========

This directory contains essential data on the brightest and nearest stars in Southern Stars CSV format. These files exist primarily for testing SSCore import/export functionality, but may be generally useful in their own right.

Even in this day and age of multi-billion-star catalogs (e.g. GAIA) it's still useful to have all the most pertinent data gathered in one place, and GAIA is missing several thousand of the brightest stars in the sky.

You may use this data free of charge for any purpose as you see fit.  If it's helpful to you, drop me a note!

_Tim DeBenedictis (timd@southernstars.com), 6 Apr 2020_

Brightest.csv
-----------

This bright star catalog was extracted from NASA's [SKY2000 Master Star Catalog](https://cdsarc.unistra.fr/ftp/V/145), updated with data from [Hipparcos](https://cdsarc.unistra.fr/ftp/I/239) and the Gliese-Jahreiss [Catalog of Nearby Stars, 3rd Edition](ftp://cdsarc.u-strasbg.fr/cats/V/70A/) (CNS3). It contains all 9092 stars from SKY2000 with HR (Harvard Revised = Bright Star) catalog numbers.

The original HR catalog has 9110 entries. Four HR numbers missing from SKY2000 (928, 4374, 4969, 5977) are double star secondary components; these were added to HR 927 (52 Ari), HR 4375 (xi UMa), HR 4968 (alpha Com), and HR 5978 (xi Sco). The other missing HR numbers (92, 95, 182, 1057, 1841, 2472, 2496, 3515, 3671, 6309, 6515, 7189, 7539, 8296) are all "bad" HR objects which are novae, supernovae, or deep sky objects. 

HIP and additional Bayer latin identifiers were added from Hipparcos, and GJ numbers added from the CNS3, using HD identifiers as a cross reference. Star common names are taken from the official [IAU star name list](http://www.pas.rochester.edu/~emamajek/WGSN/IAU-CSN.txt)

The column format is:

Type, RA, Dec, pmRA, pmDec, V, B, Dist, RV, Spec, [Comp, Delta, Sep, PA, Year], [Vtype, Vmin, Vmax, Vper, Vepoch], Ident1, Ident2, Ident3..., Common1, Common2...

- **_Type_** is object type: single star (SS), double star (DS), variable star (VS), double variable star (DV)
- **_RA_** is J2000 right ascension
- **_Dec_** is J2000 declination
- **_pmRA_** is proper motion in R.A., seconds per year
- **_pmDec_** is proper motion in Dec., arcsec per year
- **_V_** is Johnson Visual magnitude
- **_B_** is Johnson Blue magnitude
- **_Dist_** is distance in parsecs
- **_RV_** is radial velocity in km/sec
- **_Type_** is stellar spectral type.
- **_Comp_** is component(s) of double star: A, B, C, etc.
- **_Delta_** is magnitude difference between components.
- **_Sep_** is angular separation between components in arcseconds.
- **_PA_** is position angle in degrees
- **_Year_** is year of separation and PA measurement
- **_Vtype_** is variability type code from GCVS
- **_Vmin_** is minimum (brightest) magnitude
- **_Vmax_** is maximum (faintest) magnitude 
- **_Vper_** is variability period in days
- **_Vepoch_** is variability epoch as Julian Date
- **_Ident1, Ident2, etc._** are catalog identifiers in preferred order: Bayer, Flamsteed, GCVS, HR, HD, SAO, BD, CD, CP, WDS
- **_Common1, Common2, etc._** are star common names.

Fields **_[Comp ... Year]_** are only present for star types DS and DV.  
Fields **_[Vtype ... Vepoch]_** are only present for object types VS and DV.  
Empty fields indicate unknown or missing values.

**Revision History**

- Version 1.0 (27 Mar 2020): initial version
- Version 1.1 (30 Mar 2020): corrected erroneous proper motions for open clusters with RV.
- Version 1.2 (6 Apr 2020): Moved comments from CSV file header into this README.

Nearest.csv
-----------

This list of the 160 nearest stars is primarily based on data from [RECONS](http://www.recons.org/TOP100.posted.htm). Wherever possible, accurate coordinates and proper motion are from [Accurate Coordinates for Gliese Catalog Stars](https://cdsarc.unistra.fr/ftp/J/PASP/122/885). Johnson B (blue) magnitudes and radial velocities are preferentially from [Hipparcos](https://cdsarc.unistra.fr/ftp/I/239), the [Hipparcos Input Catalog](https://cdsarc.unistra.fr/ftp/I/196), or the Gliese [Catalog of Nearby Stars](ftp://cdsarc.u-strasbg.fr/cats/V/70A/).

Identifiers and common names are from RECONS and cross-referenced with [this list](http://www.ieti.org/articles/crossref.pdf) by Gerald Nordley.

The column format is identical to that of **Stars.csv**, above.

**Revision History**

Version 1.0 (27 Mar 2020): initial version
Version 1.1 (30 Mar 2020): corrected erroneous proper motions for open clusters with RV; added LMC and SMC.
Version 1.2 (6 Apr 2020): Moved comments from CSV file header into this README.

Names.csv
---------

This list of nearby star common names was compiled primarily from:

- [RECONS](http://www.recons.org/TOP100.posted.htm)
- [Gerald Nordley](http://www.ieti.org/articles/crossref.pdf)

**Revision History**

- Version 1.0 (27 Mar 2020): initial version
- Version 1.1 (6 Apr 2020): Moved comments from CSV file header into this README.
