// SSUtilities.cpp
// SSCore
//
// Created by Tim DeBenedictis on 3/23/20.
// Copyright © 2020 Southern Stars. All rights reserved.

#include <cstdarg>
#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include <time.h>

#ifdef _MSC_VER
#include <windows.h>
#include <direct.h>
#else
#include <sys/stat.h>
#include <sys/time.h>
#include <dirent.h>
#include <errno.h>
#include <glob.h>
#include <unistd.h>
#endif

#include "SSUtilities.hpp"

// Returns path to current working directory as a string

#ifdef _MSC_VER

string getcwd ( void )
{
    char path[_MAX_PATH] = { 0 };
    _getcwd ( path, _MAX_PATH );
    return string ( path );
}

bool setcwd ( const string &dir )
{
    return _chdir ( dir.c_str() ) == 0;
}

#define S_ISDIR(st_mode) ( st_mode & _S_IFDIR )
#define S_ISREG(st_mode) ( st_mode & _S_IFREG )
#define S_ISLNK(st_mode) ( st_mode & 0 )   // _stat() function does not work with symlinks on Windows; see https://learn.microsoft.com/en-us/cpp/c-runtime-library/reference/stat-functions?view=msvc-170

#else

string getcwd(void)
{
    char path[PATH_MAX] = { 0 };
    return ( getcwd ( path, PATH_MAX ) == NULL ? "" : string ( path ) );
}

bool setcwd ( const string &dir )
{
    return chdir ( dir.c_str() ) == 0;
}

#endif

bool isdir ( const string &path )
{
    struct stat st;
    return stat ( path.c_str(), &st ) == 0 && S_ISDIR ( st.st_mode );
}

bool isfile ( const string &path )
{
    struct stat st;
    return stat ( path.c_str(), &st ) == 0 && S_ISREG ( st.st_mode );
}

bool islink ( const string &path )
{
    struct stat st;
    return stat ( path.c_str(), &st ) == 0 && S_ISLNK ( st.st_mode );
}

// Reads into a C++ string (line) from a C FILE pointer (file) opened for reading in binary mode.
// Handles lines endings in LF (Unix/Mac), CRLF (Windows), or CR (Classic MacOS). In all cases,
// dicards line ending characters. Returns true if successful or false on failure (end-of-file, etc.)

bool fgetline ( FILE *file, string &line )
{
    line = "";
    char c = 0;

    while ( true )
    {
        if ( fread ( &c, 1, 1, file ) != 1 )
            return false;
        
        if ( c == '\n' )
            return true;

        if ( c == '\r' )
        {
            if ( fread ( &c, 1, 1, file ) == 1 && c != '\n' )
                fseek ( file, -1, SEEK_CUR );
            return true;
        }
        
        line += c;
    }
}

// Returns a C++ string which has leading and trailing whitespace
// trimmed from the input string (does not modify input string).

string trim ( string str )
{
    auto start = str.find_first_not_of ( " \t\r\n" );
    auto end = str.find_last_not_of ( " \t\r\n" );

    if ( start == string::npos )
        return string ( "" );
    else
        return str.substr ( start, ( end - start ) + 1 );
}

// Returns C++ string constructed from printf()-style input arguments.

string format ( const char *fmt, ... )
{
    char buf[1024] = { 0 };

    va_list args;
    va_start ( args, fmt );
    vsnprintf ( buf, sizeof buf, fmt, args );
    va_end ( args );

    return string ( buf );
}

// Robust string comparison function. Returns an integer less than zero if str1 is less than str2;
// zero if str1 equals str2; or an integer greater than zero if str1 equals str2. If n > 0, only
// the first n characters are compared. Performs a case-sensitive comparison if casesens is true;
// performs a case insensitive comparison otherwise.

int compare ( const string &str1, const string &str2, size_t n, bool casesens )
{
    if ( casesens )
    {
        if ( n > 0 )
            return strncmp ( str1.c_str(), str2.c_str(), n );
        else
            return strcmp ( str1.c_str(), str2.c_str() );
    }
    else
    {
#ifdef _MSC_VER
        if ( n > 0 )
            return _strnicmp ( str1.c_str(), str2.c_str(), n );
        else
            return _stricmp ( str1.c_str(), str2.c_str() );
#else
        if ( n > 0 )
            return strncasecmp ( str1.c_str(), str2.c_str(), n );
        else
            return strcasecmp ( str1.c_str(), str2.c_str() );
#endif
    }
}

// Returns a string converted to lower case. Does not modify the input string.

string toLower ( const string &str )
{
    string lower;
    for ( char c : str )
        lower.push_back ( tolower ( c ) );
    return lower;
}

// Returns a string converted to upper case. Does not modify the input string.

string toUpper ( const string &str )
{
    string upper;
    for ( char c : str )
        upper.push_back ( toupper ( c ) );
    return upper;
}

// Tests whether a string contains only numeric characters (digits, '-', '+', '.').
// Returns false for empty strings.

bool isNumeric ( const string &str )
{
    for ( auto it = str.begin(); it != str.end(); it++ )
        if ( ! ( isascii(*it) && ( *it == '-' || *it == '+' || *it == '.' || isdigit ( *it ) ) ) )
            return false;
    
    return str.empty() ? false : true;
}

// Tests whether a string (str) ends with another string (suffix)
// Adapted from https://stackoverflow.com/questions/874134/find-out-if-string-ends-with-another-string-in-c

bool endsWith ( const string &str, const string &suffix )
{
    return str.size() >= suffix.size() && str.compare ( str.size() - suffix.size(), suffix.size(), suffix ) == 0;
}

// Tests whether a string (str) starts with another string (prefix)
// Adapted from https://stackoverflow.com/questions/874134/find-out-if-string-ends-with-another-string-in-c

bool startsWith ( const string& str, const string &prefix )
{
    return str.size() >= prefix.size() && str.compare ( 0, prefix.size(), prefix ) == 0;
}

void replaceAll ( string& str, const std::string& from, const std::string& to)
{
    if ( from.empty() )
        return;
    size_t start_pos = 0;
    while ( ( start_pos = str.find ( from, start_pos ) ) != string::npos )
    {
        str.replace ( start_pos, from.length(), to );
        start_pos += to.length();
    }
}

// Tests whether a string (haystack) contains another string (needle).
// If case sensitivity matters, pass true for (casesens).

bool contains ( const string &haystack, const string &needle, bool casesens ) {
    if ( casesens )
        return haystack.find ( needle ) != string::npos;
    else
        return toUpper ( haystack ).find ( toUpper ( needle ) ) != string::npos;
}

// Given a file path string reference, returns the filename component (after the last slash).
// If the path contains no slash, the path is assumed to be a filename and is returned verbatim.

string getFileName ( const string &path )
{
    size_t slash = path.find_last_of ( '/' );
    if ( slash == string::npos )
        return path;
    else
        return path.substr ( slash + 1, string::npos );
}

// Given a file path string and a file type extension (".jpg", ".png", ".tif", etc.)
// returns file path changed to that extension. Input path string is not modified.

string setFileExt ( const string &path, const string &ext )
{
    size_t dot = path.rfind ( "." );
    if ( dot == string::npos )
        return path + ext;
    else
        return path.substr ( 0, dot ) + ext;
}

// Returns file type extension (".jpg", ".png", ".tif", etc.) from a file path.

string getFileExt ( const string &path )
{
    size_t dot = path.find_last_of ( '.' );
    if ( dot == string::npos )
        return "";
    else
        return path.substr ( dot, string::npos );
}

// Given a file path string and a vector of file extension strings,
// returns true if the file path ends with any of the extensions or false otherwise.
// File path is internally converted to lower case so extensions must be lowercase.

bool hasFileExt ( const string &path, const vector<string> &exts )
{
    string lowerPath = toLower ( path );
    for ( const std::string &ext : exts )
        if ( endsWith ( lowerPath, ext ) )
            return true;
    
    return false;
}

// Appends file name (name) to an input directory path (path).
// Returns appended path string; input path is not modified.

string appendPath ( const string &path, const string &name )
{
    string outPath = path;
#ifdef _MSC_VER
    if (outPath.back() != '\\')
        outPath += "\\";
#else
    if ( outPath.back() != '/' )
        outPath += "/";
#endif
    outPath += name;
    return outPath;
}

// Lists names of all contents in a directory (dirPath), except the "." and ".." entries.
// Content names are returned in the vector of string (contentPaths), sorted alphabetically.
// Returns zero if successful or nonzero error code on failure.

#ifdef _MSC_VER

int listDirectory ( const string &dirPath, vector<string> &contentPaths, bool prefixPath )
{
    HANDLE hFind;
    WIN32_FIND_DATA fileData;

    hFind = FindFirstFile ( appendPath ( dirPath, "*" ).c_str(), &fileData );
    if ( hFind == INVALID_HANDLE_VALUE )
        return -2;

    do
    {
        if ( strcmp ( fileData.cFileName, "." ) == 0 || strcmp ( fileData.cFileName, ".." ) == 0 )
            continue;

        contentPaths.push_back ( prefixPath ? appendPath ( dirPath, fileData.cFileName ) : fileData.cFileName );
    }
    while ( FindNextFile ( hFind, &fileData ) );
    FindClose ( hFind );

    sort ( contentPaths.begin(), contentPaths.end() );
    return 0;
}

#else

int listDirectory ( const string &dirPath, vector<string> &contentPaths, bool prefixPath )
{
    DIR *dir = opendir ( dirPath.c_str() );
    if ( dir == NULL )
        return -2;

    while ( true )
    {
        struct dirent *entry = readdir ( dir );
        if ( entry == NULL )
            break;

        if ( strcmp ( entry->d_name, "." ) == 0 || strcmp ( entry->d_name, ".." ) == 0 || strcmp ( entry->d_name, ".DS_Store") == 0 )
            continue;

        contentPaths.push_back ( prefixPath ? appendPath ( dirPath, entry->d_name ) : entry->d_name );
    }

    sort ( contentPaths.begin(), contentPaths.end() );
    closedir ( dir );
    return 0;
}

#endif

// Lists all existing files which match a pattern containing wildcard characters * or ?.
// Returns the number of matchine files found, and their paths are appended to the vector of strings (paths).
// Asterisk means match any number of characters, question mark means match a single character.
// For example, if the pattern string is "/dev/tty*", then the following files would match:
// "/dev/ttyS0", "/dev/ttyS1", "/dev/ttyS2", "/dev/ttyUSB0", "/dev/ttyUSB1", etc.
// If the pattern string is "/dev/ttyS?", then the following files would match:
// "/dev/ttyS0", "/dev/ttyS1", "/dev/ttyS2", but not "/dev/ttyUSB0", "/dev/ttyUSB1".
// Microsoft Windows implemented with FindFirstFile(), FindNextFile() is not yet tested as of 2025-03-08!
// See https://learn.microsoft.com/en-us/windows/win32/api/fileapi/nf-fileapi-findfirstfilea

int listWildcardFiles ( const string &pattern, vector<string> &paths )
{
#ifdef _MSC_VER
    WIN32_FIND_DATAA findData;
    HANDLE hFind = FindFirstFileA ( pattern.c_str(), &findData );
    if ( hFind == INVALID_HANDLE_VALUE )
        return 0;
    
    do
    {
        paths.push_back ( string ( findData.cFileName ) );
    }
    while ( FindNextFileA ( hFind, &findData ) );
    
    FindClose ( hFind );
    return paths.size();
#else
    glob_t glob_result;

    if ( glob (pattern.c_str(), 0, nullptr, &glob_result ) != 0 )
        return 0;

    for ( int i = 0; i < glob_result.gl_pathc; i++ )
        paths.push_back ( string ( glob_result.gl_pathv[i] ) );

    globfree (&glob_result);
    return paths.size();
#endif
}

// Returns a sanitized version of a string (filename) that contains no filesystem-forbidden characters.
// See https://stackoverflow.com/questions/1976007/what-characters-are-forbidden-in-windows-and-linux-directory-names
string sanitizeFilename ( const string &filename )
{
    string sanitized = "";
    string forbidden = "<>:\"/\\|?*";

    for ( char c : filename )
        if ( c > 31 && forbidden.find ( c ) == string::npos )
            sanitized += c;

    return sanitized;
}

// Splits a string into a vector of token strings separated by the specified delimiter.
// Two adjacent delimiters generate an empty token string (unlike C's strtok()).
// The original string is not modified.

vector<string> split ( string str, string delim )
{
    vector<string> tokens;
    
    size_t start = 0;
    size_t end = str.find ( delim );
    while ( end != std::string::npos )
    {
        tokens.push_back ( str.substr ( start, end - start ) );
        start = end + delim.length();
        end = str.find ( delim, start );
    }

    tokens.push_back ( str.substr ( start, end ) );
    return tokens;
}

// Splits a string into a vector of token strings separated by the specified delimiter.
// Adjacent delimiters are ignored so tokens can never be empty (as with C's strtok()).
// The original string is not modified.

vector<string> tokenize ( string str, string delim )
{
    std::vector<std::string> tokens;

    size_t start;
    size_t end = 0;
    while ( ( start = str.find_first_not_of ( delim, end ) ) != std::string::npos )
    {
        end = str.find ( delim, start );
        tokens.push_back ( str.substr ( start, end - start )) ;
    }

    return tokens;
}

// Splits a string containing comma-separated values into a vector of field strings.
// Handles quoted fields, commas within fields, and double quotes.
// The original string is not modified.  Based on original code found in:
// https://stackoverflow.com/questions/1120140/how-can-i-read-and-parse-csv-files-in-c

enum CSVState
{
    kCSVUnquotedField,
    kCSVQuotedField,
    kCSVQuotedQuote
};

vector<string> split_csv ( const string &csv )
{
    CSVState state = kCSVUnquotedField;
    vector<string> fields {""};
    size_t i = 0; // index of the current field
    
    for ( char c : csv )
    {
        switch ( state )
        {
            case kCSVUnquotedField:
                switch ( c )
                {
                    case ',': // end of field
                        fields.push_back("");
                        i++;
                        break;
                    case '"':
                        state = kCSVQuotedField;
                        break;
                    default:
                        fields[i].push_back(c);
                        break;
                }
                break;
            case kCSVQuotedField:
                switch ( c )
                {
                    case '"':
                        state = kCSVQuotedQuote;
                        break;
                    default:
                        fields[i].push_back(c);
                        break;
                }
                break;
            case kCSVQuotedQuote:
                switch ( c )
                {
                    case ',': // , after closing quote
                        fields.push_back(""); i++;
                        state = kCSVUnquotedField;
                        break;
                    case '"': // "" -> "
                        fields[i].push_back('"');
                        state = kCSVQuotedField;
                        break;
                    default:  // end of quote
                        state = kCSVUnquotedField;
                        break;
                }
                break;
        }
    }
    
    return fields;
}

// Converts string to 32-bit signed integer.
// Avoids throwing exceptions, unlike stoi().
// Returns zero if string cannot be converted.

int strtoint ( string str )
{
    return atoi ( str.c_str() );
}

// Converts string to 64-bit signed integer.
// Avoids throwing exceptions, unlike stoll().
// Returns zero if string cannot be converted.

int64_t strtoint64 ( string str )
{
    return atoll ( str.c_str() );
}

// Converts string to 32-bit single precision floating point value.
// Avoids throwing exceptions, unlike stof().
// Returns zero if string cannot be converted.

float strtofloat ( string str )
{
    return strtof ( str.c_str(), nullptr );
}

// Converts string to 64-bit double precision floating point value.
// Avoids throwing exceptions, unlike stod().
// Returns zero if string cannot be converted.

double strtofloat64 ( string str )
{
    return strtod ( str.c_str(), nullptr );
}

// Converts hexadecimal string to binary data.
// See https://stackoverflow.com/questions/7363774/c-converting-binary-data-to-a-hex-string-and-back

void hexstring_to_binary ( const std::string &source, uint8_t *destination, size_t length )
{
    size_t effective_length = min ( length, source.length() / 2 );
    for( size_t b = 0; b < effective_length; b++)
    {
        unsigned int ui;
        sscanf (source.data() + ( b * 3 ), "%02x", &ui);
        destination[b] = (uint8_t) ui;
    }
}

// Converts binary data to hexadecimal string
// See https://stackoverflow.com/questions/7363774/c-converting-binary-data-to-a-hex-string-and-back

void binary_to_hexstring ( const uint8_t *source, size_t length, std::string& destination )
{
    destination.clear();
    for ( unsigned int i = 0; i < length; i++ )
    {
        char digit[4] = { 0 };
        snprintf(digit, sizeof ( digit ), "%02x ", source[i] );
        destination.append (digit );
    }
}

// Convenience wrapper for binary_to_hexstring() that accepts a void * and returns the hex sting

string hexstring ( const void *data, size_t size )
{
    string hexstr;
    binary_to_hexstring ( (const uint8_t *) data, size, hexstr );
    return hexstr;
}

// Converts a string representing an angle in deg min sec to decimal degrees.
// Works with angle strings in any format (DD MM SS.S, DD MM.M, DD.D, etc.)
// Assumes leading whitespace has been removed from string!
// Returns INFINITY if string cannot be parsed as an angle.

double strtodeg ( string str )
{
    bool valid = false;
    double deg = INFINITY, min = 0.0, sec = 0.0;
    vector<string> tokens = tokenize ( str, " " );

    if ( tokens.size() > 0 )
        valid = sscanf ( tokens[0].c_str(), "%lf", &deg );
    
    if ( tokens.size() > 1 )
        valid = sscanf ( tokens[1].c_str(), "%lf", &min ) && valid;

    if ( tokens.size() > 2 )
        valid = sscanf ( tokens[2].c_str(), "%lf", &sec ) && valid;
    
    if ( ! valid )
        return INFINITY;
    
    deg = fabs ( deg ) + min / 60.0 + sec / 3600.0;
    return str[0] == '-' ? -deg : deg;
}

// Converts angle in degrees to radians.

double degtorad ( double deg )
{
    return deg * M_PI / 180.0;
}

// Converts angle in radians to degrees.

double radtodeg ( double rad )
{
    return rad * 180.0 / M_PI;
}

// Returns sine of angle in degrees.

double sindeg ( double deg )
{
    return sin ( degtorad ( deg ) );
}

// Returns cosine of angle in degrees.

double cosdeg ( double deg )
{
    return cos ( degtorad ( deg ) );
}

// Returns tangent of angle in degrees.

double tandeg ( double deg )
{
    return tan ( degtorad ( deg ) );
}

// Returns arcsine in degrees.

double asindeg ( double y )
{
    return radtodeg ( asin ( y ) );
}

// Returns arccosine in degrees.

double acosdeg ( double x )
{
    return radtodeg ( acos ( x ) );
}

// Returns arctangent in degrees.

double atandeg ( double x )
{
    return radtodeg ( atan ( x ) );
}

// Returns arctangent of y / x in radians in the range 0 to 2.0 * M_PI.

double atan2pi ( double y, double x )
{
    if ( y < 0.0 )
        return atan2 ( y, x ) + 2.0 * M_PI;
    else
        return atan2 ( y, x );
}

// Returns arctangent of y / x in degress in the range 0 to 360.

double atan2pideg ( double y, double x )
{
    return radtodeg ( atan2pi ( y, x ) );
}

// Reduces an angle in degrees to the range 0 to 2.0 * M_PI.

double mod2pi ( double rad )
{
    return rad - M_2PI * floor ( rad / M_2PI );
}

// Reduces an angle in degrees to the range -M_PI to +M_PI.

double modpi ( double rad )
{
    rad = mod2pi ( rad );
    
    if ( rad > M_PI )
        rad -= M_2PI;

    return rad;
}

// Reduces an angle in degrees to the range 0 to 360.

double mod360 ( double deg )
{
    return deg - 360 * floor ( deg / 360 );
}

// Reduces an angle in degrees to the range -180 to +180.

double mod180 ( double deg )
{
    deg = mod360 ( deg );
    
    if ( deg > 180 )
        deg -= 360;

    return deg;
}

// Reduces an angle in hours to the range 0 to 24.

double mod24h ( double h )
{
    return h - 24 * floor ( h / 24 );
}

// Replacements for POSIX sleep() and usleep() functions in MSVC/Windows

#ifdef _MSC_VER

unsigned int sleep ( unsigned int secs )
{
    Sleep ( secs * 1000 );
    return 0;
}

// see https://gist.github.com/ngryman/6482577
int usleep ( uint32_t usec )
{
    HANDLE timer;
	LARGE_INTEGER ft;

	ft.QuadPart = -(10 * (__int64) usec);
	timer = CreateWaitableTimer(NULL, TRUE, NULL);
	SetWaitableTimer(timer, &ft, 0, NULL, NULL, 0);
	WaitForSingleObject(timer, INFINITE);
	CloseHandle(timer);

    return 0;
}

#endif

// Sleep the current thread for the specified number of milliseconds.

void msleep ( uint32_t ms )
{
#ifdef _MSC_VER
    Sleep ( ms );
#else
    usleep ( (useconds_t) ms * 1000 );
#endif
}

// Returns unix time (seconds since 1 January 1970) with microsecond precision.
// Time may not increate monotonically because of system clock adjustments (leap seconds, etc.) 
// From https://stackoverflow.com/questions/5404277/porting-clock-gettime-to-windows

double unixtime ( void )
{
#ifdef _MSC_VER
	int64_t wintime = 0;
	GetSystemTimeAsFileTime ( (FILETIME*) &wintime );
	wintime -= 116444736000000000LL;  // 1 Jan 1601 to 1 Jan 1970
	return wintime / 1.0e7;
#else
	struct timeval tv = { 0 };
	gettimeofday ( &tv, nullptr );
	return tv.tv_sec + tv.tv_usec / 1.0e6;
#endif
}

// Returns elapsed seconds since the given start unix time,
// and resets start time to current unix time.
// Returned value may be negative due to system time adjustments!

double unixtime_since ( double &start )
{
	double now = unixtime();
	double since = now - start;
	start = now;
	return since;
}

// Returns a timestamp in seconds with nanosecond precision for performance timing.
// Time increases monotonically and is not affected by system clock adjustments.
// Starting point is undefined, and returned value may reset to zero at midnight.
// From https://create.stephan-brumme.com/windows-and-linux-code-timing/

double clocksec ( void )
{
#ifdef _MSC_VER
    static LARGE_INTEGER frequency;
    if ( frequency.QuadPart == 0 )
        QueryPerformanceFrequency ( &frequency );
    
    LARGE_INTEGER now;
    QueryPerformanceCounter ( &now );
    return now.QuadPart / double ( frequency.QuadPart );
#else
    struct timespec now;
    clock_gettime ( CLOCK_MONOTONIC, &now );
    return now.tv_sec + now.tv_nsec / 1.0e9;
#endif
}

// Returns elapsed seconds since the given start timestamp,
// and resets start timestamp to current monotonic clock time.

double clocksec_since ( double &start )
{
    double now = clocksec();
    double since = now - start;
    start = now;
    return since;
}

// Returns current system time zone offset in hours east of UTC.
// This is always standard time, not adjusted for daylight savings;
// see https://www.gnu.org/software/libc/manual/html_node/Time-Zone-Functions.html

double get_timezone ( void )
{
#ifdef _MSC_VER
    return _timezone / -3600.0;
#else
    return ::timezone / -3600.0;
#endif
}

// Modifies current system standard time zone offset in hours east of UTC

void set_timezone ( double zone )
{
#ifdef _MSC_VER
    _timezone = zone * -3600.0;
#else
    ::timezone = zone * -3600.0;
#endif
}

// Modifies current time zone to an IANA time zone name ("America/Los_Angeles", "Australia/Sydney", etc.)
// This will modify the current time zone offset and daylight saving time rules.
// If zone name string is empty, sets zone to system time zone.

void set_timezonename ( string zonename )
{
#ifdef _MSC_VER
    _putenv_s ( "TZ", zonename.c_str() );
    _tzset();
#else
    if ( zonename.empty() )
        unsetenv ( "TZ" );
    else
        setenv ( "TZ", zonename.c_str(), 1 );
    tzset();
#endif
}

// Returns current IANA time zone name string ("America/Los_Angeles", "Australia/Sydney", etc.)
// An empty string returned means the system is using default time zone.

string get_timezonename ( void )
{
    char *tz = getenv ( "TZ" );
    return string ( tz ? tz : "" );
}

// Returns file size in bytes, or zero if file does not exist.

size_t filesize ( const string &path )
{
    struct stat st;
    if ( stat ( path.c_str(), &st ) == 0 )
        return st.st_size;
    else
        return 0;
}

// Returns file modification time in seconds since 1 Jan 1970 UTC,
// or zero if file does not exist.

time_t filetime ( const string &path )
{
    struct stat st;
    if ( stat ( path.c_str(), &st ) == 0 )
        return st.st_mtime;
    else
        return 0;
}

// Returns a string which is the URL-encoded form of the source string (src).
// See https://stackoverflow.com/questions/154536/encode-decode-urls-in-c

string urlEncode ( const string &src )
{
    string dst;
    
    for ( unsigned char c : src )
    {
        if ( ::isalnum ( c ) || c == '-' || c == '_' || c == '.' || c == '~' )
            dst += c;
        else if ( c == ' ' )
            dst += '+';
        else
            dst += format ( "%%%02X", c );
    }
        
    return dst;
}

// Returns a string which is the URL-decoded form of the source string (src).
// See https://stackoverflow.com/questions/154536/encode-decode-urls-in-c

string urlDecode ( const string &src )
{
    string dst;
    size_t len = src.length();
    
    for ( int i = 0; i < len; i++ )
    {
        char c = src[i];
        if ( c == '%' && i + 2 < len )
        {
            int ii = 0;
            if ( sscanf ( src.substr ( i + 1, 2 ).c_str(), "%x", &ii ) )
                dst += ii;
            i += 2;
        }
        else if ( c == '+' )
            dst += ' ';
        else
            dst += c;
    }
    
    return dst;
}

// Creates a directory, including parent directories.
// Returns 0 on success.
// Based on https://stackoverflow.com/a/2336245

#ifdef _MSC_VER
#define mkdir(name, mode) _mkdir(name)
#endif

int mkdir_p(const char *dir, mode_t mode)
{
    char tmp[4096];
    char *p = NULL;
    size_t len;

    snprintf(tmp, sizeof(tmp), "%s", dir);
    len = strlen(tmp);
    if (tmp[len - 1] == '/')
        tmp[len - 1] = 0;
    for (p = tmp + 1; *p; p++)
    {
        if (*p == '/')
        {
            *p = 0;
            if (mkdir(tmp, mode) != 0 && errno != EEXIST)
                return -1;
            *p = '/';
        }
    }
    if (mkdir(tmp, mode) != 0 && errno != EEXIST)
        return -1;
    else
        return 0;
}
