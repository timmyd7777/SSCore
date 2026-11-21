JUPOS GREAT RED SPOT (GRS) LONGITUDE DATA 1964-2027
Generated on 21 November 2026 from original data at https://jupos.org/

These files document the construction of historical and future extrapolated System II longitudes for Jupiter's Great Red Spot (GRS) at 30-day intervals from 1964 through 2027.

jupos_grs_csv.py
----------------
Standalone Python 3 command-line tool, requiring no external packages. Reads .mea files from either a directory (recursively) or directly from a JUPOS MEA ZIP archive, selects GRS-center measurements, computes mean System II longitudes over N-day bins, and writes a CSV output file. Recognizes historical DC/WC and modern RC center-tag series. Optionally extrapolates future longitude using a recent linear drift fit. The script has its own extensive usage instructions and supports --help.

GRS_1964_2027.csv
------------------------------------
The final 30-day longitude table assembled from the 1964-2007 GIF (https://jupos.org/img/oRS19b.gif), and the 2026-08-14 MEA data after that (https://jupos.org/etc/PosDataMEA-2026-08-14.zip). Covers January 1964 through December 2027 (780 rows), with initial empty dates before the first usable historical plot point. Uses digitized values from oRS19b.gif before 2007 and means of original MEA measurements from 2007 onward. Missing historical longitudes were interpolated, and longitudes after 2026-08-14 were extrapolated.

CSV columns:
date_utc                 UTC date at start of 30-day interval
jd                       Julian Date at 00:00 UTC on that date
system_II_longitude_deg  GRS System II longitude, degrees (0..360)
n_observations           See conventions below

n_observations conventions in this merged file:

>0   A source-derived point; for GIF-derived points, 1 means one digitized graphical value (NOT one physical observation). For MEA-derived points it counts actual MEA measurements.
0    If longitude is present: interpolated or extrapolated value. If longitude is blank: insufficient data to supply a value.

GRS_oRS19b_overlay.png
----------------------
Sanity-check overlay graphic, comparing the final merged 30-day CSV with JUPOS's 1964-2007 historical GIF.

HOW TO REGENERATE NEW MEA DATA NEXT YEAR
---------------------------------------_
1. Download the newest MEA ZIP archive from https://jupos.org/rPosiData.htm

2. Save the ZIP alongside jupos_grs_csv.py.

3. Run one of the following (replace the archive filename):

# 30-day binned measurements, no extrapolation:
python3 jupos_grs_csv.py PosDataMEA-2026-08-14.zip grs_new.csv -n 30

# 30-day measurements plus up to 365 days of extrapolation:
python3 jupos_grs_csv.py PosDataMEA-2026-08-14.zip grs_new.csv -n 30 -e 365

# 30-day measurements and extrapolation through a specified date:
python3 jupos_grs_csv.py PosDataMEA-2026-08-14.zip grs_new.csv -n 30 --through 2028-12-31

# Full options and explanations:
python3 jupos_grs_csv.py --help

The MEA extractor outputs the same CSV format as jupos_grs_merged_30day_1964_2027.csv:
date_utc,jd,system_II_longitude_deg,n_observations

IMPORTANT LIMITATIONS / PRECAUTIONS
-----------------------------------
* jupos_grs_csv.py processes MEA files only. It does NOT regenerate the pre-2007 GIF digitization or automatically update/merge the historical master CSV. Keep the master file intact and save fresh results separately.

* The extractor DOES NOT fill missing historical intervals: with no observations they are blank with n_observations=0. The merged master CSV was produced in a separate merge/interpolation step.

* MEA observations are averaged using circular longitude arithmetic, to handle a 359-to-1-degree wrap correctly.

* Forecasts are linear extrapolations of a drift fitted to recent daily observational means (default fitting window: 365 days). They are not new observations, and accuracy deteriorates if the GRS drift changes.

* The binary MEA layout was reverse-engineered from this 2026 archive, not taken from an official file-format specification. Spot-check new archives and compare with JUPOS's published plots before relying on updated output.

* The 30-day values refer to interval bins, whose dates are bin STARTS; measured-bin longitudes are means over the interval, not instantaneous longitudes measured exactly at midnight on the start date.

* Do not overwrite jupos_grs_merged_30day_1964_2027.csv by accident!
