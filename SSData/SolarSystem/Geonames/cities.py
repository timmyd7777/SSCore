"""
This script augments the cities file by admin1 English name and daylight saving time observation.
Follow these steps to run:

1. Set the minimum population(POPULATION_MIN) for a city to be included in the result CSV
2. Makes sure the paths to all files are correct:
   - cities1000.txt (unzipped from cities1000.zip)
   - admin1CodesASCII.txt
   - timeZones.txt
   - output Cities.csv
3. You shouldn't need to change this often, but double check the (selected) column headers:
   - cities1000_cols
   - cities1000_cols_selected
   - admin1CodesASCII_cols
4. On the command line, run:  python cities.py

See the SolarSystem README for the locations of the source files.
"""
import csv

POPULATION_MIN = 100000

TYPE_CODE = 'CT'

CITIES_PATH = 'cities1000.txt'
ADMIN1_PATH = 'admin1CodesASCII.txt'
TIMEZONES_PATH = 'timeZones.txt'
OUTPUT_CITIES_PATH = '../Cities.csv'

cities1000_cols = [
    'geonameid','name','asciiname','alternatenames','latitude','longitude','feature class','feature code','country code','cc2',
    'admin1 code','admin2 code','admin3 code','admin4 code','population','elevation','dem','timezone','modification date'
]

cities1000_cols_selected = [
    'name','asciiname','latitude','longitude','country code','admin1 code','population','elevation','timezone'
]

admin1CodesASCII_cols = [
    'country_admin1','admin1_unicode','admin1_ascii','unknown_number'
]


def readFile(path, delimiter, columns=None):
    result = []
    num_skipped = 0
    with open(path) as f:
        reader = csv.reader(f, delimiter=delimiter)
        if not columns:
            columns = next(reader)
        for row in reader:
            if not row:
                continue
            if len(row) != len(columns):
                # Not observed yet, but we want to call out any rows that can't match header to value perfectly 
                print('HEADER/VALUE MISMATCH; SKIPPED: ', row)
                num_skipped += 1
            result.append({header: value for header, value in zip(columns, row)})
    print('-----------------------------------------------------------------')
    print(path)
    print('SKIPPED: ', num_skipped)
    print('IMPORTED: ', len(result))
    print('-----------------------------------------------------------------')
    return result


def getCities():
    cities1000 = readFile(CITIES_PATH, '\t', cities1000_cols)
    cities1000 = [city for city in cities1000 if int(city['population']) >= POPULATION_MIN]
    print('FILTERED CITIES ON POPULATION >=', POPULATION_MIN)
    print('NUMBER OF CITIES: ', len(cities1000))
    return cities1000


def getAdmin1Map():
    admin1CodesASCII = readFile(ADMIN1_PATH, '\t', admin1CodesASCII_cols)
    admin1CodesASCII_map = {row['country_admin1']: row['admin1_ascii'] for row in admin1CodesASCII}
    return admin1CodesASCII_map


def getDaylightSaving():
    """
    The timezone file currently contains unique timezones.  Since it also contains country code, we perform
    timezone matching using both country code and timezone.  The cities file can contain country code/timezone
    combos that don't appear in the timezone file, in which case we fall back to matching on timezone only.  As
    long as the timezone file contains unique timezones it should be fine.  But do look out for invalid
    country code/timezone combos to make sure the fallback is valid.
    """
    timeZones = readFile(TIMEZONES_PATH, '\t')

    daylight_saving = {}
    for row in timeZones:
        is_daylight_saving = int(row['GMT offset 1. Jan 2020'] != row['DST offset 1. Jul 2020'])

        if row['TimeZoneId'] in daylight_saving:
            print('DUPLICATE TIMEZONE; ONLY KEEPING FIRST OCCURRENCE: ', row['TimeZoneId'])
        else:
            daylight_saving[row['TimeZoneId']] = (is_daylight_saving, row['rawOffset (independant of DST)'])

        if (row['CountryCode'], row['TimeZoneId']) in daylight_saving:
            print('DUPLICATE COUNTRY/TIMEZONE; ONLY KEEPING FIRST OCCURRENCE: ', (row['CountryCode'], row['TimeZoneId']))
        else:
            daylight_saving[(row['CountryCode'], row['TimeZoneId'])] = (is_daylight_saving, row['rawOffset (independant of DST)'])
    
    return daylight_saving


def augmentCities(cities, admin1CodesASCII_map, daylight_saving):
    invalid_country_admin1 = []
    invalid_country_admin1_non00 = []
    invalid_country_timezone = []
    invalid_timezone = []
    timezone_fallbacks = 0

    for city in cities:
        city['type'] = TYPE_CODE
        country_timezone = (city['country code'], city['timezone'])

        if city['timezone'] not in daylight_saving:
            invalid_timezone.append(city['timezone'])
        elif country_timezone not in daylight_saving:
            city['daylight_saving'] = daylight_saving[city['timezone']][0]
            city['timezone_raw_offset'] = daylight_saving[city['timezone']][1]
            invalid_country_timezone.append(country_timezone)
            timezone_fallbacks += 1
        else:
            city['daylight_saving'] = daylight_saving[country_timezone][0]
            city['timezone_raw_offset'] = daylight_saving[country_timezone][1]

        if not city['admin1 code']:                         # e.g. country code 'AQ' (Antartica) has no admin1
            continue

        country_admin1 = city['country code'] + '.' + city['admin1 code']

        if country_admin1 not in admin1CodesASCII_map:      # e.g. 'AQ.00' (Aruba has no entries in admin1CodesASCII.txt)
            invalid_country_admin1.append(country_admin1)

            # Some small cities may have admin1 code '00'.  Those cities don't seem to need admin1 code.
            if city['admin1 code'] != '00':
                invalid_country_admin1_non00.append(country_admin1)

        city['admin1_name'] = admin1CodesASCII_map.get(country_admin1, '')

    print('-----------------------------------------------------------------')
    print('INVALID TIMEZONES: ', sorted(set(invalid_timezone)))
    print('NUMBER OF CITIES AFFECTED: ', len(invalid_timezone))
    print('-----------------------------------------------------------------')
    print('INVALID COUNTRY CODE/TIMEZONE COMBOS: ', sorted(set(invalid_country_timezone)))
    print('FALLBACK TO TIMEZONE ONLY; NUMBER OF CITIES AFFECTED: ', timezone_fallbacks)
    print('-----------------------------------------------------------------')
    print('INVALID COUNTRY CODE/ADMIN1 CODE COMBOS: ', sorted(set(invalid_country_admin1)))
    print('NUMBER OF CITIES AFFECTED: ', len(invalid_country_admin1))
    print()
    print('INVALID COUNTRY CODE/ADMIN1 CODE COMBOS (NON-00): ', sorted(set(invalid_country_admin1_non00)))
    print('NUMBER OF CITIES AFFECTED: ', len(invalid_country_admin1_non00))
    print('-----------------------------------------------------------------')


def saveFile(content, headers, filename):
    with open(filename, 'w') as f:
        writer = csv.DictWriter(f, fieldnames=headers, extrasaction='ignore')   # ignore columns not selected
        for row in content:
            writer.writerow(row)

    print('FILE SAVED: ', filename)


def main():
    cities = getCities()
    admin1Map = getAdmin1Map()
    daylight_saving = getDaylightSaving()
    augmentCities(cities, admin1Map, daylight_saving)

    output_cols = ['type'] + cities1000_cols_selected + ['admin1_name', 'daylight_saving', 'timezone_raw_offset']
    saveFile(cities, output_cols, OUTPUT_CITIES_PATH)


if __name__ == '__main__':
    main()
