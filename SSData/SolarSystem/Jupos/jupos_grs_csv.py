#!/usr/bin/env python3
"""Export JUPOS Great Red Spot (GRS) System II longitudes to CSV.

QUICK START (Python 3; no pip installs or external packages needed)
------------------------------------------------------------------
1. Download the latest *MEA positional measurements* ZIP from JUPOS:
      https://jupos.org/rPosiData.htm
   The archive filename is dated and may change from year to year.
2. Save this script as jupos_grs_csv.py alongside that ZIP.
3. Open Terminal, cd to the directory containing both files, and run:

   # 30-day averages from the ZIP (no need to unzip it):
   python3 jupos_grs_csv.py PosDataMEA-2026-08-14.zip grs.csv -n 30

   # Same output, but forecast up to 365 days after the LAST observation:
   python3 jupos_grs_csv.py PosDataMEA-2026-08-14.zip grs.csv -n 30 -e 365

   # Or forecast through an absolute date (inclusive):
   python3 jupos_grs_csv.py PosDataMEA-2026-08-14.zip grs.csv -n 30 --through 2027-12-31

   # Use a 180-day window instead of the default 365 days to fit the drift:
   python3 jupos_grs_csv.py PosDataMEA-2026-08-14.zip grs.csv -n 30 -e 365 --fit-days 180

   # Read already-extracted .mea files in a directory (also scans subfolders):
   python3 jupos_grs_csv.py /path/to/mea_directory grs.csv -n 30

   # Daily averages; restrict historical dates; omit unobserved date bins:
   python3 jupos_grs_csv.py PosDataMEA-2026-08-14.zip daily.csv -n 1 \
       --start 2020-01-01 --end 2026-07-07 --skip-empty

   # Show complete command-line help:
   python3 jupos_grs_csv.py --help

Replace the dated ZIP filename in these examples with the new download's name.
Output CSV is overwritten if it already exists, so choose an output name
that doesn't overwrite your separately merged historical master CSV.

OUTPUT COLUMNS
--------------
  date_utc                 UTC date at the START of the averaging interval
  jd                       Julian Date at 00:00 UTC on date_utc
  system_II_longitude_deg  Circular mean of GRS-center longitude, degrees
  n_observations           Count of actual MEA observations in that interval

Examples of how to interpret rows:
  - n_observations > 0: observed mean from that many binary MEA records.
  - n_observations = 0 and blank longitude: no observations in the bin.
  - n_observations = 0 and populated longitude: extrapolated prediction.
    The script does NOT interpolate missing historical bins.

Each bin covers [date_utc, date_utc + N days); successive bin *start* dates
are N days apart. The observation mean describes that whole bin, not an
instantaneous value on the date_utc timestamp. The grid starts at the first
observed UTC date unless --start specifies a different anchor date.

EXTRAPOLATION
-------------
-e DAYS extends the grid no more than DAYS after the latest observation;
--through YYYY-MM-DD extends through the chosen date. Use one or the other.
A straight-line least-squares trend is fitted to *daily* circular means over
the last --fit-days calendar days (default 365), so multiple observations on
the same day do not overweight that date. Some outliers are rejected.
Predictions may become inaccurate if the GRS changes its drift rate.

ABOUT THIS FORMAT
-----------------
The WinJUPOS MEA binary record layout was reverse-engineered against the
PosDataMEA-2026-08-14 archive, NOT obtained from an official format specification.
Accepted GRS-center feature tags include historical DC/WC and modern RC series;
feature boundaries are excluded. Verify results when JUPOS changes its format.
"""

import argparse
import csv
from datetime import date, timedelta
from collections import defaultdict
from statistics import median
import math
from pathlib import Path
import struct
import sys
import zipfile

HEADER_SIZE = 630
RECORD_SIZE = 182
RECORD_JD_OFFSET = 3            # little-endian IEEE-754 double
RECORD_CODE_LENGTH_OFFSET = 11  # length of following ASCII tag
RECORD_CODE_OFFSET = 12
RECORD_SYSTEM_II_OFFSET = 28     # little-endian IEEE-754 float
# 'C' identifies the feature center; D/W/R are JUPOS measurement series.
# Historical GRS data are principally DC/WC, while recent measurements are RC.
# Exclude F/P (preceding/following boundaries) and N/S (north/south edges).
GRS_CENTERS = frozenset(
    ('DC RS', 'DC1RS', 'DC2RS', 'DC3RS',
     'WC RS', 'WC1RS', 'WC2RS', 'WC3RS',
     'RC1RS', 'RC2RS', 'RC3RS'))
UNIX_EPOCH_JD = 2440587.5
UNIX_EPOCH_DATE = date(1970, 1, 1)


def utc_date_to_jd(day):
    """Julian Date at 00:00 UTC on the given calendar date."""
    return UNIX_EPOCH_JD + (day - UNIX_EPOCH_DATE).days


def jd_to_utc_date(jd):
    """Convert JD to UTC calendar date. JD starts at noon UT, not midnight."""
    days_since_epoch = math.floor(jd - UNIX_EPOCH_JD)
    return UNIX_EPOCH_DATE + timedelta(days=days_since_epoch)


def sources(path):
    """Yield (name, content bytes), from a directory or a .zip archive."""
    if path.is_dir():
        files = sorted(p for p in path.rglob('*') if p.is_file() and p.suffix.lower() == '.mea')
        if not files:
            raise ValueError(f'No .mea files found under {path}')
        for p in files:
            yield str(p), p.read_bytes()
    elif path.is_file() and zipfile.is_zipfile(path):
        with zipfile.ZipFile(path) as z:
            names = sorted(n for n in z.namelist() if n.lower().endswith('.mea'))
            if not names:
                raise ValueError(f'No .mea files in {path}')
            for n in names:
                yield n, z.read(n)
    else:
        raise ValueError(f'Input must be a directory of .mea files or a ZIP archive: {path}')


def measurements(path, stats):
    """Yield (UTC date, longitude) for verified GRS-center tags."""
    for name, content in sources(path):
        stats['files'] += 1
        if not content.startswith(b'**WJOFHM**'):
            raise ValueError(f'{name}: unexpected MEA signature')
        if len(content) < HEADER_SIZE or (len(content) - HEADER_SIZE) % RECORD_SIZE:
            raise ValueError(f'{name}: unexpected MEA record alignment/size')
        for offset in range(HEADER_SIZE, len(content), RECORD_SIZE):
            record = memoryview(content)[offset:offset + RECORD_SIZE]
            stats['records'] += 1
            # MEA files contain an initial all-zero placeholder record.
            if record[0:RECORD_SIZE] == bytes(RECORD_SIZE):
                continue
            code_len = record[RECORD_CODE_LENGTH_OFFSET]
            if not 1 <= code_len <= 20:
                continue
            try:
                code = bytes(record[RECORD_CODE_OFFSET:RECORD_CODE_OFFSET + code_len]).decode('ascii').strip()
            except UnicodeDecodeError:
                continue
            if code not in GRS_CENTERS:
                continue
            jd = struct.unpack_from('<d', record, RECORD_JD_OFFSET)[0]
            lon = struct.unpack_from('<f', record, RECORD_SYSTEM_II_OFFSET)[0]
            if not (math.isfinite(jd) and 2300000 <= jd <= 2600000
                    and math.isfinite(lon) and 0 <= lon < 360):
                stats['invalid'] += 1
                continue
            stats['selected'] += 1
            yield jd_to_utc_date(jd), float(lon)


def circular_mean(longitudes):
    """Correct mean on a circle: 359 and 1 degrees average to 0, not 180."""
    sx = sum(math.sin(math.radians(x)) for x in longitudes)
    cx = sum(math.cos(math.radians(x)) for x in longitudes)
    if math.hypot(sx, cx) < 1e-10:
        return None  # e.g. equal numbers of 0 and 180 degrees; undefined mean
    return math.degrees(math.atan2(sx, cx)) % 360.0


def fit_recent_drift(data, fit_days):
    """Fit deg/day from daily circular means in the last fit_days days.

    Each observing DATE has equal weight, independent of the number of images.
    Longitude is unwrapped before fitting. Reject large residual outliers in
    two passes using a median-absolute-deviation threshold.
    Returns (last observation date, predicted longitude on that date, deg/day,
             number of dates used, fit start date).
    """
    last_date = max(d for d, _ in data)
    fit_start = last_date - timedelta(days=fit_days)
    by_day = defaultdict(list)
    for d, lon in data:
        if d >= fit_start:
            by_day[d].append(lon)
    samples = [(d, circular_mean(by_day[d])) for d in sorted(by_day)]
    samples = [(d, lon) for d, lon in samples if lon is not None]
    if len(samples) < 3 or (samples[-1][0] - samples[0][0]).days < 30:
        raise ValueError('Not enough recent GRS observations for extrapolation '
                         '(need at least 3 observing dates spanning 30 days; '
                         'try increasing --fit-days)')

    xs = [(d - last_date).days for d, _ in samples]
    ys = [samples[0][1]]
    for _, lon in samples[1:]:
        # Select the nearest full-circle equivalent to the previous value.
        ys.append(ys[-1] + ((lon - ys[-1] + 180) % 360 - 180))

    def line_fit(indices):
        count = len(indices)
        xbar = sum(xs[i] for i in indices) / count
        ybar = sum(ys[i] for i in indices) / count
        denom = sum((xs[i] - xbar) ** 2 for i in indices)
        if denom == 0:
            raise ValueError('Cannot fit GRS drift to identical observation dates')
        slope = sum((xs[i] - xbar) * (ys[i] - ybar) for i in indices) / denom
        return ybar - slope * xbar, slope

    keep = list(range(len(samples)))
    for _ in range(2):
        intercept, slope = line_fit(keep)
        residuals = [ys[i] - (intercept + slope * xs[i]) for i in keep]
        center = median(residuals)
        mad = median(abs(r - center) for r in residuals)
        threshold = max(3.0, 4 * 1.4826 * mad)
        retained = [i for i in keep if abs(ys[i] - intercept - slope * xs[i] - center) <= threshold]
        if len(retained) < 3 or (xs[retained[-1]] - xs[retained[0]]) < 30:
            break
        if retained == keep:
            break
        keep = retained
    intercept, slope = line_fit(keep)
    return last_date, intercept % 360, slope, len(keep), samples[0][0]


def create_csv(args):
    stats = {'files': 0, 'records': 0, 'selected': 0, 'invalid': 0}
    data = list(measurements(args.input, stats))
    if not data:
        raise ValueError('No valid GRS-center observations found')
    data.sort(key=lambda p: p[0])

    first_obs, last_obs = data[0][0], data[-1][0]
    first = args.start or first_obs
    last = args.end or last_obs
    if args.through and args.extrapolate:
        raise ValueError('Use either --through or -e, not both')
    forecast_end = args.through or (last_obs + timedelta(days=args.extrapolate))
    if args.through and forecast_end < last_obs:
        raise ValueError('--through must be on or after the last observation')
    if first > last:
        raise ValueError('--start must not be after --end')
    if (args.extrapolate or args.through) and last != last_obs:
        raise ValueError('With -e, --end must be omitted or equal the last observation date')

    # Align intervals to the requested start date. Each bin is [start, start+N).
    # Observations outside the requested output window are ignored.
    n_bins = (last - first).days // args.interval + 1
    bins = [[] for _ in range(n_bins)]
    for obs_date, longitude in data:
        delta = (obs_date - first).days
        if 0 <= delta and obs_date <= last:
            bins[delta // args.interval].append(longitude)

    if str(args.output) == '-':
        f = sys.stdout
        close = False
    else:
        args.output.parent.mkdir(parents=True, exist_ok=True)
        f = args.output.open('w', newline='', encoding='utf-8')
        close = True
    try:
        writer = csv.writer(f)
        writer.writerow(('date_utc', 'jd', 'system_II_longitude_deg', 'n_observations'))
        nonempty = 0
        for i, values in enumerate(bins):
            if not values and args.skip_empty:
                continue
            day = first + timedelta(days=i * args.interval)
            mean = circular_mean(values) if values else None
            writer.writerow((day.isoformat(), f'{utc_date_to_jd(day):.1f}', f'{mean:.6f}' if mean is not None else '', len(values)))
            nonempty += bool(values)

        if args.extrapolate or args.through:
            anchor_date, anchor_lon, drift, fit_count, fit_start = fit_recent_drift(data, args.fit_days)
            # Continue the exact same bin-start date grid. A point is emitted
            # only if its date is after the last measured date AND no more
            # than -e days afterward. No interpolation is invented for gaps.
            extrapolated = 0
            forecast_days = (forecast_end - anchor_date).days
            for i in range(n_bins, n_bins + (forecast_days // args.interval) + 3):
                day = first + timedelta(days=i * args.interval)
                ahead = (day - anchor_date).days
                if day > forecast_end:
                    break
                if ahead <= 0:
                    continue
                lon = (anchor_lon + drift * ahead) % 360.0
                writer.writerow((day.isoformat(), f'{utc_date_to_jd(day):.1f}', f'{lon:.6f}', 0))
                extrapolated += 1
            print(f'Extrapolation: {extrapolated} points, last measurement {anchor_date}, '
                  f'fit since {fit_start} ({fit_count} observing dates), '
                  f'drift {drift * 365.2425:+.3f} deg/year; '
                  'extrapolated rows have n_observations=0', file=sys.stderr)
    finally:
        if close:
            f.close()
    print(f'MEA files: {stats["files"]:,}; records: {stats["records"]:,}; '
          f'GRS-center records: {stats["selected"]:,}; invalid selected records: {stats["invalid"]:,}', file=sys.stderr)
    print(f'Observation range: {first_obs} to {last_obs}; interval: {args.interval} days; '
          f'nonempty bins: {nonempty:,}/{n_bins:,}; output: {args.output}', file=sys.stderr)


def iso_date(s):
    try:
        return date.fromisoformat(s)
    except ValueError as exc:
        raise argparse.ArgumentTypeError(str(exc)) from exc


def main():
    p = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    p.add_argument('input', type=Path, help='directory containing MEA files (recursively), or JUPOS MEA ZIP')
    p.add_argument('output', type=Path, help='output CSV path; use - for stdout')
    p.add_argument('-n', '--interval', type=int, default=1, choices=range(1, 366),
                   metavar='1..365', help='number of UTC calendar days per averaging bin (default: 1)')
    p.add_argument('--start', type=iso_date, help='first bin start date, YYYY-MM-DD (default: earliest observation)')
    p.add_argument('--end', type=iso_date, help='last included date, YYYY-MM-DD (default: latest observation)')
    p.add_argument('-e', '--extrapolate', type=int, default=0, metavar='DAYS',
                   help='forecast up to DAYS after latest measurement, on the -n date grid (default: 0)')
    p.add_argument('--through', type=iso_date, metavar='YYYY-MM-DD',
                   help='extrapolate through this UTC date (inclusive), maintaining the -n grid')
    p.add_argument('--fit-days', type=int, default=365, metavar='DAYS',
                   help='lookback in days for linear extrapolation fit (default: 365)')
    p.add_argument('--skip-empty', action='store_true', help='omit intervals without observations (default: output blank longitude and count 0)')
    args = p.parse_args()
    if not 1 <= args.interval <= 365:
        p.error('--interval must be between 1 and 365')
    if args.extrapolate < 0:
        p.error('-e/--extrapolate must be >= 0 days')
    if args.fit_days < 30:
        p.error('--fit-days must be >= 30 days')
    try:
        create_csv(args)
    except (ValueError, OSError, zipfile.BadZipFile, OverflowError) as exc:
        p.exit(1, f'Error: {exc}\n')


if __name__ == '__main__':
    main()
