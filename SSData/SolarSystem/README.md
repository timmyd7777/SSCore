Solar System Object Data
========================

This directory contains essential data on Solar System objects in various formats.  These files exit primarily for testing SSCore import/export functionality, but may be generally useful in their own right.

You may use this data free of charge for any purpose as you see fit.  If it's helpful to you, drop me a note!

_Tim DeBenedictis (timd@southernstars.com), 2 Apr 2020_

Planets.csv
-----------

This table of data on the Sun, the major planets, and Pluto is in Southern Stars CSV format was adapted from Table 1 in this paper by E.M. Standish (JPL Solar System Dynamics), ["Keplerian Elements for Approximate Positions of the Major Planets"](https://ssd.jpl.nasa.gov/txt/p_elem_t1.txt) and from JPL's ["Planets and Pluto: Physical Characteristics"](https://ssd.jpl.nasa.gov/?planet_phys_par) table.  Data for the Sun is from NASA's ["Sun Fact Sheet"](https://nssdc.gsfc.nasa.gov/planetary/factsheet/sunfact.html).

Columns are: Type, q, e, i, w, N, M, n, Epoch, H, G, Radius, ID, Name

- **_Type_** (PL) means planet
- **_q_** is perihelion distance (AU)
- **_e_** is eccentricity
- **_i_** is inclination (deg)
- **_w_** is argument of perihelion (deg)
- **_N_** is longitude of ascending node (deg)
- **_M_** is mean anomaly (deg)
- **_n_** is mean motion (deg/day)
- **_Epoch_** is epoch of orbital elements (Julian Ephemeris Date)
- **_H_** is absolute visual magnitude (i.e., V at 1.0 AU distance and 100% illumination)
- **_G_** is magnitude phase parameter
- **_Radius_** is equatorial radius (km)
- **_ID_** is JPL NAIF ID
- **_Name_** is planet name

Empty fields indicate unknown or missing values.

**Revision History**

Version 1.0 (31 Mar 2020): initial version

Moons.csv
---------

This table of moon data in Southern Stars CSV format was adapted from JPL's ["Planetary Satellite Mean Orbital Parameters"](https://ssd.jpl.nasa.gov/?sat_elem) and from JPL's ["Planetary Satellite Physical Parameters"](https://ssd.jpl.nasa.gov/?sat_phys_par) table.

Columns are: Type, q, e, i, w, N, M, n, Epoch, H, G, Radius, ID, Name

- **_Type_** (MN) means moon
- **_q_** is periapse distance (km)
- **_e_** is eccentricity
- **_i_** is inclination (deg)
- **_w_** is argument of periapse (deg)
- **_N_** is longitude of ascending node (deg)
- **_M_** is mean anomaly (deg)
- **_n_** is mean motion (deg/day)
- **_Epoch_** is epoch of orbital elements (Julian Ephemeris Date)
- **_H_** is absolute visual magnitude (i.e., V at 1.0 AU distance and 100% illumination)
- **_G_** is magnitude phase parameter
- **_Radius_** is equatorial radius (km)
- **_ID_** is JPL NAIF ID
- **_Name_** is moon name

Empty fields indicate unknown or missing values.

The orbital inclinations, arguments, and nodes are referred to their Laplace Plane (essentially their primary planet's equatorial plane), except for Earth's Moon, whose orbital elements are referred to the J2000 ecliptic.

**Revision History**

Version 1.0 (31 Mar 2020): initial version  
Version 1.1 (22 Apr 2020): fixed absolute magnitudes (H)

Asteroids.txt
-------------

This table of asteroid data in MPC Export Format was compiled from the [Minor Planet Center](https://www.minorplanetcenter.net/iau/mpc.html).  It includes the first 10,000 numbered asteroids from the [MPCORB database](https://www.minorplanetcenter.net/iau/MPCORB/MPCORB.DAT) and all of the asteroids on the MPC's ["Distant"  objects](https://www.minorplanetcenter.net/iau/MPCORB/Distant.txt) list.  This file includes all of the officially IAU-designated dwarf planets.  The file format is described [here](https://www.minorplanetcenter.net/iau/info/MPOrbitFormat.html).

**Revision History**

Version 1.0 (2 Apr 2020): initial version

Comets.txt
----------

This table of comet data in MPC Export Format is a copy of the Minor Planet Center's ["Observable Comets"](https://www.minorplanetcenter.net/iau/MPCORB/CometEls.txt) file.  The file format is described [here](https://www.minorplanetcenter.net/iau/info/CometOrbitFormat.html).

**Revision History**

Version 1.0 (2 Apr 2020): initial version

DE438
-----

This directory contains the [JPL DE438](ftp://ssd.jpl.nasa.gov/pub/eph/planets/ascii/de438/) planetary and lunar ephemeris, converted from ASCII to binary format using Bill Gray's [asc2eph](https://github.com/Bill-Gray/jpl_eph) utility (see documentation from [Project Pluto](https://www.projectpluto.com/jpl_eph.htm#asc2eph).).  The binary files are in little-endian (Intel/ARM) byte order.

There are two files here.  The larger, **1550_2650.438**, is about 103 MB in size and covers the entire thousand-year DE438 timespan.  The smaller file, **1950_2040.438**, is a 9.4 MB subset which convers the years 1950 to 2050 - the entire space age!

**Revision History**

Version 1.0 (3 Apr 2020): initial commit.

Satellites
----------

This directory contains a sample of TLE (Two/Three-Line Element) and other satellite data files from different sources.  These are test cases for SSCore satellite data import functionality, and may be useful in their own right!

- **_visual.txt_** is the [Visual](http://www.celestrak.com/NORAD/elements/visual.txt) (brightest) satellite list from [CelesTrak](http://www.celestrak.com)
- **_all.txt_** is the [complete](https://www.n2yo.com/tle2/3line.tle) satellite list from [N2YO](https://www.n2yo.com)
- **_mcnames.txt_** is Mike McCants' satellite [names file](https://www.prismnet.com/~mmccants/tles/mcnames.zip), which contains satellite size and magnitude information.
- **_je9pel.csv_** is Mineo Wakita's amateur radio satellite [frequency file](http://www.ne.jp/asahi/hamradio/je9pel/satslist.csv), in CSV format.

**Revision History**

Version 1.0 (4 Apr 2020): initial checkin.  
Version 1.1 (11 May 2020): added Mineo Wakita JE9PEL satellite radio frequency file.

Cities.csv
----------

The file contains data about cities on Earth, downloaded from the [GeoNames](http://download.geonames.org/export/dump/cities1000.zip) geographical database.  Using additional data on [daylight saving time observation](http://download.geonames.org/export/dump/timeZones.txt) and [admin1 code mapping](http://download.geonames.org/export/dump/admin1CodesASCII.txt), two additional columns (admin1_name, daylight_saving) are appended to the file.  Only cities with population at least 100,000 are present.  The columns are:

- **_name_**:  name of geographical point (utf8) varchar(200)
- **_asciiname_**:  name of geographical point in plain ascii characters, varchar(200)
- **_latitude_**:  latitude in decimal degrees (wgs84)
- **_longitude_**:  latitude in decimal degrees (wgs84)
- **_country code_**:  ISO-3166 2-letter country code, 2 characters
- **_admin1 code_**:  fipscode (subject to change to iso code), see exceptions below, see file admin1Codes.txt for display names of this code; varchar(20)
- **_population_**:  bigint (8 byte int) 
- **_elevation_**:  in meters, integer
- **_timezone_**:  the iana timezone id (see file timeZone.txt) varchar(40)
- **_admin1_name_**:  admin1 code, in English
- **_daylight_saving_**:  whether the geographical point observes daylight saving time (0: No, 1: Yes)

Except for the last two columns, the column description is copied from the [GeoNames ReadMe](http://download.geonames.org/export/dump/readme.txt).

**Revision History**

Version 1.0 (10 Nov 2020): initial checkin.

Features.csv
--------

This file contains non-Earth planetary surface feature names, from the USGS [Gazetteer of Planetary Nomenclature](https://planetarynames.wr.usgs.gov/AdvancedSearch).  The file was downloaded using the following options:

- **_System_**: All
- **_Target_**: All
- **_Coordinate System_**: +East, 0-360, Planetographic
- **_Feature Type_**: All
- **_Approval Status_**: Adopted by IAU
- **_Columns to Include_**:  Feature Name, Clean Feature Name, Target, Diameter, Center Lat/Lon, Feature Type Code, Origin
- **_Sorted By_**: Feature Name, Ascending
- **_Output Format_**: CSV

**Revision History**

Version 1.0 (10 Nov 2020): initial checkin.  