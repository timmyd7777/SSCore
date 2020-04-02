Solar System Object Data
========================

This directory contains essential data on Solar System objects in CSV format and Minor Planet Center export format.  These files exit primarily for testing SSCore import/export functionality, but may be generally useful in their own right.

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
