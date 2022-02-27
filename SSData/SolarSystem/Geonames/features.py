"""
This script adds a type column to the features file
"""
TYPE_CODE = 'FT'

INPUT_PATH = 'Features_download.csv'
OUTPUT_PATH = '../Features.csv'


def main():
    content = []

    with open(INPUT_PATH, 'r') as fi:
        with open(OUTPUT_PATH, 'w') as fo:
            fi.readline()                               # skip header
            for line in fi:
                if line.strip():                        # skip blank line
                    fo.write(TYPE_CODE + ',' + line)

    print('FILE SAVED: ', OUTPUT_PATH)


if __name__ == '__main__':
    main()
