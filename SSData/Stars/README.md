Star Data
=========

This directory contains essential data on the brightest and nearest stars in Southern Stars CSV format. These files exist primarily for testing SSCore import/export functionality, but may be generally useful in their own right.

Even in this day and age of multi-billion-star catalogs (e.g. GAIA) it's still useful to have all the most pertinent data gathered in one place, and GAIA is missing several hundred of the brightest stars in the sky.

You may use this data free of charge for any purpose as you see fit.  If it's helpful to you, drop me a note!

We've been exceedingly careful to generate these data files from the latest and most accurate sources. If you find errors, please let us know.

_Tim DeBenedictis (timd@southernstars.com), 6 Apr 2020_

Brightest.csv
-----------

This bright star catalog was extracted from NASA's [SKY2000 Master Star Catalog](https://cdsarc.unistra.fr/ftp/V/145), updated with data from the [GAIA DR3](https://www.cosmos.esa.int/web/gaia/dr3), [Tycho-2](https://www.cosmos.esa.int/web/hipparcos/tycho-2), [Hipparcos](https://cdsarc.unistra.fr/ftp/I/239), and the Gliese-Jahreiss [Catalog of Nearby Stars, 3rd Edition](ftp://cdsarc.u-strasbg.fr/cats/V/70A/) (CNS3). It contains all 9092 stars from SKY2000 with HR (Harvard Revised = Bright Star) catalog numbers.

Binary star orbital data was added from the [Sixth Catalog of Orbits of Visual Binary Stars](http://www.astro.gsu.edu/wds/orb6.html). Variable star data was added from the [General Catalogue of Variable Stars version 5.1](http://www.sai.msu.su/gcvs/gcvs/intr.htm).

The original HR catalog has 9110 entries. Four HR numbers missing from SKY2000 (928, 4374, 4969, 5977) are double star secondary components; these were added to HR 927 (52 Ari), HR 4375 (xi UMa), HR 4968 (alpha Com), and HR 5978 (xi Sco). The other missing HR numbers (92, 95, 182, 1057, 1841, 2472, 2496, 3515, 3671, 6309, 6515, 7189, 7539, 8296) are all "bad" HR objects which are novae, supernovae, or deep sky objects. 

HIP and additional Bayer latin identifiers were added from Hipparcos, and GJ numbers added from the CNS3, using HD identifiers as a cross reference. Star common names are taken from the official [IAU star name list](http://www.pas.rochester.edu/~emamajek/WGSN/IAU-CSN.txt)

The column format is:

Type, RA, Dec, pmRA, pmDec, V, B, Dist, RV, Spec, [Comp, Delta, Sep, PA, Year, T, a, e, i, w, n, P], [Vtype, Vmin, Vmax, Vper, Vepoch], Ident1, Ident2, Ident3..., Common1, Common2...

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
- **_Comp_** is component(s) of double star: A, B, C, etc. See note below for binary stars.
- **_Delta_** is magnitude difference between components.
- **_Sep_** is angular separation between components in arcseconds.
- **_PA_** is position angle in degrees
- **_Year_** is Julian year of separation and PA measurement
- **_T_** is Julian year of periastron for binary stars
- **_a_** is semimajor axis in arcseconds for binary stars
- **_e_** is orbital eccentricity for binary stars
- **_i_** is orbital plane inclination to sky plane, in degrees, for binary stars
- **_w_** is argument of periastron, in degrees, for binary stars
- **_n_** is position angle of ascending node, in degrees, for binary stars
- **_P_** is orbital period, in years, for binary stars
- **_Vtype_** is variability type code from GCVS
- **_Vmin_** is minimum (brightest) magnitude
- **_Vmax_** is maximum (faintest) magnitude 
- **_Vper_** is variability period in days
- **_Vepoch_** is variability epoch as Julian Date
- **_Ident1, Ident2, etc._** are catalog identifiers in preferred order: Bayer, Flamsteed, GCVS, HR, HD, SAO, BD, CD, CP, WDS
- **_Common1, Common2, etc._** are star common names.

Fields **_[Comp ... P]_** are only present for star types DS and DV. 
Fields **_[Vtype ... Vepoch]_** are only present for object types VS and DV. 

Empty fields indicate unknown or missing values. 

For binary stars with orbits, the **_Comp_** field contains two characters: first this component's identifier, then the other component's identifier.  So, "AB" means "this component is A, and the other component in the binary pair is B", and "DC" means "this component is D, and the other component in the binary pair is C". This convention resolves ambiguities in the original binary star orbit catalog, which contains orbits for pairs ("AB", "CD", "AB,C") rather than individual components ("A", "B", "C", "D", etc.)

**Revision History**

- Version 1.0 (27 Mar 2020): initial version
- Version 1.1 (30 Mar 2020): corrected erroneous proper motions for open clusters with RV.
- Version 1.2 (6 Apr 2020): Moved comments from CSV file header into this README.
- Version 2.0 (4 May 2021): Added binary star orbital data from WDS, variable star data from GCVS, a few nearby star proper names.
- Version 3.0 (9 Feb 2023): Added identifiers, positions, motions, and distances fom GAIA DR3.
- Version 3.1 (11 Feb 2023): Keep original SKY2000 positions to preserve double stars. Use identifiers, motions, radial velocities, and parallaxes fom GAIA DR3.

Nearest.csv
-----------

This list of the 377 nearest stars is primarily based on data from [The 10-parsec Sample in the GAIA Era](https://gruze.org/10pc_v2/), version 2. It includes all known stars closer than 10 parsecs; brown dwarfs and exoplanets are not included. Three problematic stars (41 Ara Bb, Xi UMa Ab, Xi UMa Bb) were removed from the original 10-pc sample, which contains 380 stars. 

Missing names, identifiers, and magnitudes were added from the [Gliese-Jahreiss Catalog of Nearby Stars](https://heasarc.gsfc.nasa.gov/W3Browse/star-catalog/cns3.html) 3rd Edition, the [RECONS](http://www.recons.org/TOP100.posted.htm) list of 160 nearest stars, cross-referenced with [this list](http://www.ieti.org/articles/crossref.pdf) by Gerald Nordley, and NASA's [SKY2000 Master Star Catalog](https://cdsarc.unistra.fr/ftp/V/145). Binary star orbital data was added from the [Sixth Catalog of Orbits of Visual Binary Stars](http://www.astro.gsu.edu/wds/orb6.html). Double star identifiers and data were added from the [Washington Double Star Catalog](http://www.astro.gsu.edu/wds/). Variable star data was added from the [General Catalogue of Variable Stars version 5.1](http://www.sai.msu.su/gcvs/gcvs/intr.htm). 

The column format is identical to that of **Stars.csv**, above. The preferred order for the catalog identifiers and names is: Bayer or GCVS, Flamsteed, GJ, HD, DM, HIP, then common names. 

**Revision History**

- Version 1.0 (27 Mar 2020): initial version
- Version 1.2 (6 Apr 2020): Moved comments from CSV file header into this README.
- Version 1.3 (3 Apr 2021): Fixed bad data for Sigma Dra, e Eri, and HIP 113020.
- Version 1.4 (11 Apr 2021): Added names for Luyten 726-8 A & B, moved many names in the GJ column to the name column.
- Version 2.0 (4 May 2021): Added binary star orbital data from WDS, variable star data from GCVS. Fixed a few common names assigned to the wrong stars.
- Version 3.0 (9 Feb 2023): Complete rework based on the 10-parsec sample described above.
- Version 3.1 (11 Feb 2023): Add missing Giclas (G), Luyten (L), Luyten-Palomar (LP) identifiers and B, V magnitudes from CNS3. Add WDS identifiers.

Names.csv
---------

This list of nearby star common names was compiled primarily from:

- [RECONS](http://www.recons.org/TOP100.posted.htm)
- [Gerald Nordley](http://www.ieti.org/articles/crossref.pdf)

**Revision History**

- Version 1.0 (27 Mar 2020): initial version
- Version 1.1 (6 Apr 2020): Moved comments from CSV file header into this README.

Tetra3.npz
----------

This is a star pattern database for the Tetra3 plate solver. It was compiled from the [5th Bright Star Catalog](http://tdc-www.harvard.edu/catalogs/bsc5.html) using the modified version of `tetra3.py` included in the **SSCode/Tetra3** directory. 

**Revision History**

- Version 1.0 (27 Nov 2023): initial version. 
