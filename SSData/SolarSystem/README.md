Solar System Object Data
========================

This directory contains essential data on Solar System objects in various formats.  These files exit primarily for testing SSCore import/export functionality, but may be generally useful in their own right.

You may use this data free of charge for any purpose as you see fit.  If it's helpful to you, drop me a note!

_Tim DeBenedictis (timd@southernstars.com), 2 Apr 2020_

Planets.csv
-----------

This table contains data on the Sun, the major planets, and the officially recognized dwarf planets, in Southern Stars CSV format. It was adapted from Table 1 in the original paper by E.M. Standish, [Approximate Positions of the Planets](https://ssd.jpl.nasa.gov/planets/approx_pos.html), and from JPL's [Planetary Physical Parameters](https://ssd.jpl.nasa.gov/?planet_phys_par) table. Data for the Sun is from NASA's [Sun Fact Sheet](https://nssdc.gsfc.nasa.gov/planetary/factsheet/sunfact.html). Orbital data for dwarf planets Ceres, Eris, Makemake, and Haumea was extracted from [JPL Horizons](https://ssd.jpl.nasa.gov/horizons/).

All orbital elements in this table are referenced against the J2000 ecliptic.

Columns are: Type, q, e, i, w, N, M, n, wrate, Nrate, RA, Dec, Epoch, H, G, Radius, Mass, RotPer, Albedo, ID, Name

- **_Type_** is object type; PL = planet, MN = moon
- **_q_** is perihelion distance (AU)
- **_e_** is eccentricity
- **_i_** is inclination (deg)
- **_w_** is argument of perihelion (deg)
- **_N_** is longitude of ascending node (deg)
- **_M_** is mean anomaly (deg)
- **_n_** is mean motion (deg/day)
- **_wrate_** is rate of argument of perihelion (deg/year) where 1 year = 365.25 days
- **_Nrate_** is rate of longitude of ascending node (deg/year) where 1 year = 365.35 days
- **_RA_** is J2000 Right Ascension of north pole of reference plane (deg)
- **_Dec_** is J2000 Declination of north pole of reference plane (deg)
- **_Epoch_** is epoch of orbital elements (Julian Ephemeris Date)
- **_H_** is absolute visual magnitude (i.e., V at 1.0 AU distance and 100% illumination)
- **_G_** is magnitude phase parameter
- **_Radius_** is equatorial radius (km)
- **_Mass_** is planetary mass (kg)
- **_RotPer_** is sidereal rotation period (days); negative if retrograde
- **_Albedo_** is geometric albedo
- **_ID_** is JPL NAIF ID, or asteroid number for dwarf planets Ceres, Eris, Makemake, Haumea
- **_Name_** is planet name

Empty fields indicate unknown or missing values.

**Revision History**

- Version 1.0 (31 Mar 2020): initial version
- Version 1.1 (10 Apr 2021): added planetary masses
- Version 1.2 (16 May 2021): added rotation periods and albedos
- Version 2.0 (30 Jun 2026): added dwarf planets, column headers, rates of motion for argument of perihelion and longitude of ascending node; added J2000 RA and Dec of reference plane.

Moons.csv
---------

This table contains data for all of the Solar System's 461 confirmed moons as of June 2026. It was adapted from the following sources:

- JPL's [Planetary Satellite Mean Elements](https://ssd.jpl.nasa.gov/?sat_elem) and [Planetary Satellite Physical Parameters](https://ssd.jpl.nasa.gov/?sat_phys_par) tables. Data for the major moons was taken from older (March 2020) versions of these sources, which are more complete and accurate than the current versions. 
- Wikipedia articles for moons of [Earth](https://en.wikipedia.org/wiki/Moon), [Mars](https://en.wikipedia.org/wiki/Moons_of_JupiterMars), [Jupiter](https://en.wikipedia.org/wiki/Moons_of_Jupiter), [Saturn](https://en.wikipedia.org/wiki/Moons_of_Saturn), [Uranus](https://en.wikipedia.org/wiki/Moons_of_Uranus), [Neptune](https://en.wikipedia.org/wiki/Moons_of_Neptune), and [Pluto](https://en.wikipedia.org/wiki/Moons_of_Pluto).

The orbital inclinations, arguments, and nodes for most moons are refererenced againt their Laplace plane (essentially their primary planet's equatorial plane), except for Earth's Moon, and the irregular outer satellites of Jupiter, Saturn, Uranus, and Neptune. These moons' orbital elements are referred to the J2000 ecliptic. For all moons, the J2000 RA and Dec of the pole of the reference plane is now provided, along with precession rates for the node and periapse around that pole.

JPL's satellite orbit data contains two entries for Uranus's moon Puck; we used the data from the more recent solution. Orbit data for Saturn's B-ring shepherd moon S/2009 S1 was missing from the JPL data; we replaced with Wikipedia's values. 

Columns are identical to Planets.csv, above. 

**Revision History**

- Version 1.0 (31 Mar 2020): initial version  
- Version 1.1 (22 Apr 2020): fixed absolute magnitudes (H)
- Version 1.2 (10 Apr 2021): added moon masses
- Version 1.3 (16 May 2021): added rotation periods and albedos
- Version 1.4 (9  Jan 2023): added data for all 177 natural satellites in the original JPL data tables.
- Version 1.5 (17 Jan 2023): added data for 39 additional Jupiter and Saturn moons from Wikipedia and Scott Sheppard's tables.
- Version 1.6 (27 Dec 2023): added data for 74 additional Jupiter and Saturn moons from Wikipedia and Scott Sheppard's tables, and corrected numerous errors in the previous version.
- Version 1.61 (30 Dec 2023): use Sheppard & Wikipedia as primary sources for all outer planet moons; use JPL data only for Earth's Moon, Phobos, Deimos, and albedos.
- Version 1.62 (31 Dec 2023): use JPL orbit data and albedos for all moons except S/2009 S1; use Wikipedia physical data except albedo for all moons. 
- version 1.7 (27 Jun 2026): correct absolute magnitudes for Pluto's moons (they were incorrectly entered as apparent magnitudes previously). Add column headers.
- Version 2.0 (30 Jun 2026): added 171 new moons discovered since 2023, rates of motion for argument of perihelion and longitude of ascending node; added J2000 RA and Dec of reference plane.
- Version 2.01 (8 Jul 2026): corrected rates of ascending node and mean motion for moons with inclination < 90 deg.

Asteroids.txt
-------------

This table of asteroid data in MPC Export Format was compiled from the [Minor Planet Center](https://www.minorplanetcenter.net/iau/mpc.html).  It includes the first 10,000 numbered asteroids from the [MPCORB database](https://www.minorplanetcenter.net/iau/MPCORB/MPCORB.DAT) and all of the asteroids on the MPC's ["Distant"  objects](https://www.minorplanetcenter.net/iau/MPCORB/Distant.txt) list.  This file includes all of the officially IAU-designated dwarf planets.  The file format is described [here](https://www.minorplanetcenter.net/iau/info/MPOrbitFormat.html).

**Revision History**

- Version 1.0 (2 Apr 2020): initial version
- Version 1.1 (21 Mar 2021): updated with latest data from MPC

Comets.txt
----------

This table of comet data in MPC Export Format is a copy of the Minor Planet Center's ["Observable Comets"](https://www.minorplanetcenter.net/iau/MPCORB/CometEls.txt) file.  The file format is described [here](https://www.minorplanetcenter.net/iau/info/CometOrbitFormat.html).

**Revision History**

Version 1.0 (2 Apr 2020): initial version
Version 1.1 (21 Mar 2021): updated with latest data from MPC

DE442
-----

This directory contains the [JPL DE442](https://ssd.jpl.nasa.gov/ftp/eph/planets/ascii/de442/) planetary and lunar ephemeris, converted from ASCII to binary format using Bill Gray's [asc2eph](https://github.com/Bill-Gray/jpl_eph) utility (see documentation from [Project Pluto](https://www.projectpluto.com/jpl_eph.htm#asc2eph).).  The binary files are in little-endian (Intel/ARM) byte order.

There are two files here.  The larger, **1550_2650.442**, is about 103 MB in size and covers the entire thousand-year DE442 timespan.  The smaller file, **1950_2050.442**, is a 9.4 MB subset which covers the years 1950 to 2050 - the entire space age!

**Revision History**

- Version 1.0 (3 Apr 2020): initial commit.
- Version 2.0 (26 Jul 2026): update from DE438 to DE442

Satellites.csv
--------------

This file consolidates satellite data from several sources. [Celestrak](https://www.celestak.org/) and [N2YO](https://n2yo.com) are the primary sources for satellite orbital elements and satellite categories. Launch and decay dates, launch sites, countries, and descriptions are provided by [Space-Track](https://www.space-track.org) and n2yo.com.

Satellite standard visual magnitudes and sizes were originally from Mike McCant's [McNames](https://www.mmccants.org/tles/mcnames.zip) file. Mike stopped updating this file in July 2019, but most of its satellites are still in orbit. More recent satellite magnitudes are derived from the IAU [SCORE](https://score.cps.iau.org/about) observation [database](https://score.cps.iau.org/download). For satellites belonging to a well-observed constellation (like Starlink, Kuiper, Iridium), a constellation-average standard magnitude is assigned.

Satellite radio frequency data comes from [Mineo Wakita](https://www.ne.jp/asahi/hamradio/je9pel/satslist.htm).

In the consolidated CSV file, the first 15 fields are identical to Celestrak's OMM (Orbital Mean-Element Message) CSV export format. The remaining fields contain data from these other sources. The columns are:

- **_OBJECT_NAME_** is satellite name. Multiple names from different sources are concatenated with semicolons, like `SPACE STATION;ISS (ZARYA)`
- **_OBJECT_ID_** is the satellite's international designator, like `1998-067A`
- **_EPOCH_** is the epoch of orbital elements, like `2026-08-03T19:06:47.841984`
- **_MEAN_MOTION_** is mean motion in revolutions per day, like `15.49332738`
- **_ECCENTRICITY_** is orbital eccentricity like `.00072249`
- **_INCLINATION_** is inclination of orbit plane to Earth's equatorial plane in degrees, like `51.6316`
- **_RA_OF_ASC_NODE_** is right ascension of ascending node at epoch (not J2000) in degrees, like `64.4821`
- **_ARG_OF_PERICENTER_** is the argument of pericenter in degrees, like `9.2337`
- **_MEAN_ANOMALY_** is the mean anomalty at epoch in degrees, like `350.8783`
- **_EPHEMERIS_TYPE_** indicates the orbit propogator to be used with these elements, and is always `0` for SGP4/SDP4
- **_CLASSIFICATION_TYPE_** is the classification status, `U` = unclassified
- **_NORAD_CAT_ID_** is the satellite's NORAD catalog ID. The ISS is `25544`
- **_ELEMENT_SET_NO_** is the sequential element set number for this satellite, maximum `999`
- **_REV_AT_EPOCH_** is the number of revolutions around Earth the the satellite has completed since launch, like `57913`
- **_BSTAR_** is the B* atmospheric drag coefficient (see SGP4 documetation), like `.14146099E-3`
- **_MEAN_MOTION_DOT_** is 1/2 of the first derivative of the mean motion in revolutions/day^2, like `.14146099E-3`
- **_MEAN_MOTION_DDOT_** is 1/6 of the second derivative of the mean motion in revolutions/day^3, like `.7444E-4`
- **_STD_MAG_** is the satellite's standard visual magnitude as defined by McCants: its apparent magnitude at 1000 km range and 50% ilumination, like `-0.50` for the ISS.
- **_MAG_SRC_** is the standard magnitude source: `M` = McCants, `C` = constellation average, `S` = SCORE observations
- **_DIAMETER_** is the satellite's largest physical size, i.e. diameter of enclosing sphere, in meters, like `30.0` for the ISS.
- **_DIAMETER_SRC_** is the source of the diameter: `M` = McCants, `R` = radar cross section
- **_COUNTRY_** is the abbreviation for the satellite owner/operator's country or other source/organization, used by space-track. The multinational `ISS` is its own country!
- **_LAUNCH_SITE_** is the abbreviation for the satellite's launch site used by space-track. `TTMTR` for the ISS Zarya module, launched from Tyuratam.
- **_LAUNCH_DATE_** is the satellite's launch date, YYYY-MM-DD. For the first ISS module, `1998-11-20`
- **_DECAY_DATE_** is the satellite's re-entry date, YYYY-MM-DD. Empty for currently-orbiting satellites like the ISS.
- **_RADIO_UPLINK_FREQ_** is the satellite's uplink radio communication frequency, multiple frequencies are semicolon-separated, like `145.825;437.825`
- **_RADIO_DOWNLINK_FREQ_** is the satellite's downlink radio communication frequency, multiple frequencies are semicolon-separated.
- **_RADIO_BEACON_FREQ_** is the satellite's radio beacon frequency, multiple frequencies are semicolon-separated.
- **_RADIO_MODE_** is the radio modulation scheme, multiple modes are semicolon-separated. The ISS uses `SSTV;1k2 AFSK;1k2 AFSK SSTV;1k2 AFSK APRS;9k6 GFSK FM;FM;Voice(Reg 1);Voice(Reg 2 3)`
- **_RADIO_CALLSIGN_** is the satellite radio station's international call sign for ; multiple callsigns are semicolon-delimited. The ISS call signs are all `RS0ISS ARISS`.
- **_RADIO_STATUS_** is the satellite's radio communication status. Statuses for multiple radio systems are semicolon-delimited.
- **_CATEGORIES_** is a semicolon-delimited list of numerical category identifiers to which the satellite belongs. See list below. The ISS belongs to categories `18;1;2;9`.
- **_DESCRIPTION_** is a free-text description of the satellite from n2yo.com or the NSSDC master catalog.

The first 57 category identifiers are from n2yo.com, and many are shared with Celestrak. Category 99 (space debris) is from n2yo, although this is not formally listed as a category there. Categories 100-107 are unique to Celestrak. Here is a complete list:

|  ID | Category |
|-----|----------|
|   1 | Brightest |
|   2 | International Space Station |
|   3 | Weather |
|   4 | NOAA |
|   5 | kGOES |
|   6 | Earth Resources |
|   7 | Search and Rescue |
|   8 | Disaster Monitoring (DMC) |
|   9 | Tracking and Data Relay Satellite System (TDRSS) |
|  10 | Geostationary |
|  11 | Intelsat |
|  12 | Gorizont |
|  13 | Raduga |
|  14 | Molniya |
|  15 | Iridium and Iridium-NEXT |
|  16 | Orbcomm |
|  17 | Globalstar |
|  18 | Amateur Radio |
|  19 | Experimental |
|  20 | GPS Operational |
|  21 | GLONASS Operational |
|  22 | Galileo |
|  23 | Satellite-Based Augmentation System (SBAS) |
|  24 | Navy Navigation Satellite System (NNSS) |
|  25 | Russian LEO Navigation (Musson) |
|  26 | Space & Earth Science |
|  27 | Geodetic |
|  28 | Engineering |
|  29 | Education |
|  30 | Military |
|  31 | Radar Calibration |
|  32 | CubeSats |
|  33 | XM & Sirius |
|  34 | Television |
|  35 | BeiDou Navigation |
|  36 | Yaogan |
|  37 | Westford Needles |
|  38 | Parus |
|  39 | Strela |
|  40 | Gonets |
|  41 | Tsiklon |
|  42 | Tsikada |
|  43 | O3b Communication |
|  44 | Tselina |
|  45 | Celestis |
|  46 | Indian Regional Navigation Satellite System (IRNSS / NavIC) |
|  47 | Quasi-Zenith Satellite System (QZSS) |
|  48 | Planet (Flock) |
|  49 | Spire (Lemur) |
|  50 | GPS |
|  51 | GLONASS |
|  52 | Starlink |
|  53 | OneWeb |
|  54 | Chinese Space Station |
|  55 | Qianfan (Thousand Sails) |
|  56 | Amazon Leo (Kuiper), not to be confused with LEO = Low Earth Orbit |
|  57 | GeeSAT |
|  99 | Space junk / debris |
| 100 | SES |
| 101 | ARGOS Data Collection System |
| 102 | Eutelsat |
| 103 | Hulianwang (Guowang) National Internet Satellites |
| 104 | SatNOGS |
| 105 | GEO Protected Zone |
| 106 | Synthetic Aperture Radar (SAR) |
| 107 | TeleSat |
| 108 | Analyst (Unknown) satellites |

**IMPORTANT:** The `Satellites.csv` file checked into this repository is an example only, for illistrative and testing purposes. For production uses, the current version is updated daily and can be downloaded from:

[https://www.southernstars.com/updates/satellites/satellites.csv](https://www.southernstars.com/updates/satellites/satellites.csv)

**Revision History**

- Version 2.0 (8 Aug 2026): initial version.

Satellites
----------

This directory contains a sample of TLE (Two/Three-Line Element) and other satellite data files from different sources.  These are test cases for SSCore satellite data import functionality, and may be useful in their own right! These files are examples only, and should not be used for production.

- **_CeleTrak_** contains a selection of satellite orbit files in CSV format from [CelesTrak](http://www.celestrak.com)
- **_N2YO_** contains a selection of satellite orbit files in TLE format from [N2YO](https://www.n2yo.com)
- **_Other_** contains a number of miscellaneous other satellite data files, including...
- **_mcnames.txt_** is Mike McCants' satellite [names file](https://www.prismnet.com/~mmccants/tles/mcnames.zip), which contains satellite size and magnitude information. Mike last updated this file in July 2019.
- **_score.csv_** is the [SCORE](https://score.cps.iau.org/download) observation database, which contains more recent satellite magnitude observations. 
- **_je9pel.csv_** is Mineo Wakita's amateur radio satellite [frequency file](http://www.ne.jp/asahi/hamradio/je9pel/satslist.csv), in CSV format.

**Revision History**

- Version 1.0 (4 Apr 2020): initial checkin.  
- Version 1.1 (11 May 2020): added Mineo Wakita JE9PEL satellite radio frequency file.
- Version 1.2 (21 Mar 2021): updated with latest data from all sources; added n2yo "brightest" satellites.
- Version 1.3 (10 Oct 2022): updated with latest data from all sources. 
- Version 1.4 (2 Apr 2024): added test data from celestrak.org in new CSV and TLE format.
- Version 2.0 (8 Aug 2026): reorganized, added lstest CelesTrak and N2YO files, added SCORE database.

Cities.csv
----------

The file contains data about cities on Earth, downloaded from the [GeoNames](http://download.geonames.org/export/dump/cities1000.zip) geographical database.  Using additional data on [daylight saving time observation](http://download.geonames.org/export/dump/timeZones.txt) and [admin1 code mapping](http://download.geonames.org/export/dump/admin1CodesASCII.txt), two additional columns (admin1_name, daylight_saving) are appended to the file.  All cities with population at least 100,000 and/or country capitals are present.  The columns are:

- **_Type_** (CT) means city
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
- **_timezone_raw_offset_**:  GMT offset, independent of daylight saving time

Except for the last two columns, the column description is copied from the [GeoNames ReadMe](http://download.geonames.org/export/dump/readme.txt).

**Revision History**

- Version 1.0 (10 Nov 2020): initial checkin.
- Version 1.1 (24 Nov 2020): added type column.
- Version 1.2 (23 Jun 2026): added country capitals.

Features.csv
------------

This file contains non-Earth planetary surface feature names, from the USGS [Gazetteer of Planetary Nomenclature](https://planetarynames.wr.usgs.gov/AdvancedSearch).  The file was downloaded using the following options:

- **_Type_** (FT) means feature
- **_System_**: All
- **_Target_**: All
- **_Coordinate System_**: +East, 0-360, Planetographic
- **_Feature Type_**: All
- **_Approval Status_**: Adopted by IAU
- **_Columns to Include_**:  Feature Name, Clean Feature Name, Target, Diameter, Center Lat/Lon, Feature Type Code, Origin
- **_Sorted By_**: Feature Name, Ascending
- **_Output Format_**: CSV

Geonames
--------

This folder contains the original files from GeoNames and the Gazetteer of Planetary Nomenclature, on which the above two .csv files are based.  It also contains two python scripts (developed by Kam Seto) for converting the original files to the CSV format described above. 

**Revision History**

- Version 1.0 (10 Nov 2020): initial checkin.
- Version 1.1 (24 Nov 2020): added type column.
