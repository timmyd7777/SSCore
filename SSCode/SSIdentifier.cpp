//  SSIdentifier.cpp
//  SSCore
//
//  Created by Tim DeBenedictis on 3/20/20.
//  Copyright © 2020 Southern Stars. All rights reserved.

#include <algorithm>
#include <iostream>
#include <fstream>

#include "SSAngle.hpp"
#include "SSIdentifier.hpp"

static map<SSCatalog,string> _catNameMap =
{
    { kCatJPLanet, "JPL" },
    { kCatAstNum, "AST" },
    { kCatComNum, "COM" },
    { kCatNORADSat, "NORAD" },
    { kCatBayer, "Bayer" },
    { kCatFlamsteed, "Flamsteed" },
    { kCatGCVS, "GCVS" },
    { kCatHR, "HR" },
    { kCatGJ, "GJ" },
    { kCatHD, "HD" },
    { kCatSAO, "SAO" },
    { kCatBD, "BD" },
    { kCatCD, "CD" },
    { kCatCP, "CP" },
    { kCatHIP, "HIP" },
    { kCatTYC, "TYC" },
    { kCatGAIA, "GAIA" },
    { kCat2MASS, "2MASS" },
    { kCatGiclas, "G" },
    { kCatLuyten, "L" },
    { kCatLP, "LP" },
    { kCatWDS, "WDS" },
    { kCatMessier, "M" },
    { kCatCaldwell, "C" },
    { kCatNGC, "NGC" },
    { kCatIC, "IC" },
    { kCatMel, "Mel" },
    { kCatSh2, "Sh2" },
    { kCatLBN, "LBN" },
    { kCatLDN, "LDN" },
    { kCatPNG, "PNG" },
    { kCatPK, "PK" },
    { kCatPGC, "PGC" },
    { kCatUGC, "UGC" },
    { kCatUGCA, "UGCA" },
};

static map<string,SSCatalog> _nameCatMap =
{
    { "JPL", kCatJPLanet },
    { "AST", kCatAstNum },
    { "COM", kCatComNum },
    { "NORAD", kCatNORADSat },
    { "Bayer", kCatBayer },
    { "Flamsteed", kCatFlamsteed },
    { "GCVS", kCatGCVS },
    { "HR", kCatHR },
    { "GJ", kCatGJ },
    { "HD", kCatHD },
    { "SAO", kCatSAO },
    { "BD", kCatBD },
    { "CD", kCatCD },
    { "CP", kCatCP },
    { "HIP", kCatHIP },
    { "TYC", kCatTYC },
    { "GAIA", kCatGAIA },
    { "2MASS", kCat2MASS },
    { "G", kCatGiclas },
    { "L", kCatLuyten },
    { "LP", kCatLP },
    { "WDS", kCatWDS },
    { "M", kCatMessier },
    { "C", kCatCaldwell },
    { "NGC", kCatNGC },
    { "IC", kCatIC },
    { "Mel", kCatMel },
    { "Sh2", kCatSh2 },
    { "LBN", kCatLBN },
    { "LDN", kCatLDN },
    { "PNG", kCatPNG },
    { "PK", kCatPK },
    { "PGC", kCatPGC },
    { "UGC", kCatUGC },
    { "UGCA", kCatUGCA },
};

string catalog_to_string ( SSCatalog cat )
{
    return _catNameMap[cat];
}

SSCatalog string_to_catalog ( string str )
{
    return _nameCatMap[str];
}

static vector<string> _bayvec =
{
    "alpha",
    "beta",
    "gamma",
    "delta",
    "epsilon",
    "zeta",
    "eta",
    "theta",
    "iota",
    "kappa",
    "lambda",
    "mu",
    "nu",
    "xi",
    "omicron",
    "pi",
    "rho",
    "sigma",
    "tau",
    "upsilon",
    "phi",
    "chi",
    "psi",
    "omega",
};

static vector<string> _convec =
{
    "And", "Ant", "Aps", "Aqr", "Aql", "Ara", "Ari", "Aur",
    "Boo", "Cae", "Cam", "Cnc", "CVn", "CMa", "CMi", "Cap",
    "Car", "Cas", "Cen", "Cep", "Cet", "Cha", "Cir", "Col",
    "Com", "CrA", "CrB", "Crv", "Crt", "Cru", "Cyg", "Del",
    "Dor", "Dra", "Equ", "Eri", "For", "Gem", "Gru", "Her",
    "Hor", "Hya", "Hyi", "Ind", "Lac", "Leo", "LMi", "Lep",
    "Lib", "Lup", "Lyn", "Lyr", "Men", "Mic", "Mon", "Mus",
    "Nor", "Oct", "Oph", "Ori", "Pav", "Peg", "Per", "Phe",
    "Pic", "Psc", "PsA", "Pup", "Pyx", "Ret", "Sge", "Sgr",
    "Sco", "Scl", "Sct", "Ser", "Sex", "Tau", "Tel", "Tri",
    "TrA", "Tuc", "UMa", "UMi", "Vel", "Vir", "Vol", "Vul"
};

static map<string,int> _conmap;
static map<string,int> _baymap;

static void mapinit ( void )
{
    for ( int i = 0; i < _bayvec.size(); i++ )
        _baymap.insert ( { _bayvec[i], i + 1 } );

    for ( int i = 0; i < _convec.size(); i++ )
        _conmap.insert ( { _convec[i], i + 1 } );
}

string con_to_string ( int con )
{
    return con > 0 && con < _convec.size() ? _convec[con - 1] : "";
}

static int string_to_con ( const string &str, bool casesens = true )
{
    if ( _conmap.size() == 0 )
        mapinit();
    
    if ( casesens )
        return _conmap[ str ];
    
    for ( int i = 0; i < _convec.size(); i++ )
        if ( compare ( _convec[i], str, 0, casesens ) == 0 )
            return i + 1;
    
    return 0;
}

static int string_to_bayer ( const string &str, bool casesens = true )
{
    size_t len = str.length();
    
    if ( len == 1 )
    {
        if ( str[0] >= 'a' && str[0] <= 'z' )
            return str[0] - 'a' + 25;
        else if ( str[0] >= 'A' && str[0] < 'R' )
            return str[0] - 'A' + 51;
    }
    else
    {
        for ( int i = 0; i < _bayvec.size(); i++ )
            if ( compare ( _bayvec[i], str, len, casesens ) == 0 )
                return i + 1;
    }
    
    return 0;
}

string bayer_to_string ( uint64_t bay )
{
    if ( bay > 50 )
        return string ( 1, bay - 51 + 'A' );
    else if ( bay > 24 )
        return string ( 1, bay - 25 + 'a' );
    else
        return _bayvec[ bay - 1 ];
}

static uint64_t string_to_gcvs ( string str, bool casesens = true )
{
    size_t len = str.length();
    int n1 = 0, n2 = 0;
    
    if ( ! casesens )
        toUpper ( str );
    
    // Sequence R, S, T ... Z
        
    if ( len == 1 && str[0] >= 'R' && str[0] <= 'Z' )
    {
        return str[0] - 'R' + 1;
    }

    // Sequence RR, RS, RT ... SS, ST, SU, ... TT, TU ... ZZ
        
    else if ( len == 2 && str[0] >= 'R' && str[0] <= 'Z' && str[1] >= str[0] && str[1] <= 'Z' )
    {
        n1 = str[0] - 'R';
        n2 = str[1] - 'R';
        
        return n1 * 9 - ( n1 - 1 ) * n1 / 2 + ( n2 - n1 ) + 10;
    }
        
    // Sequence AA, AB, AC, ... BB, BC, BD, ... CC, CD, .... QZ
        
    else if ( len == 2 && str[0] >= 'A' && str[0] < 'R' && str[0] != 'J' && str[1] >= str[0] && str[1] <= 'Z' && str[1] != 'J'  )
    {
        n1 = str[0] - 'A';
        n2 = str[1] - 'A';
        
        // J is skipped!
        
        if ( str[0] >= 'K' )
            n1--;

        if ( str[1] >= 'K' )
            n2--;
        
        return n1 * 25 - ( n1 - 1 ) * n1 / 2 + ( n2 - n1 ) + 55;
    }

    // Sequence V335, V336, V337, V338, ...
        
    if ( len > 3 && str[0] == 'V' && str[1] >= '0' && str[1] <= '9' )
    {
        return strtoint ( str.substr ( 1, len - 1 ) );
    }
        
    return 0;
}

string gcvs_to_string ( uint64_t n )
{
    uint64_t n0 = 0, n1 = 0, n2 = 0;
    
    // Sequence R, S, T, ... Z
    
    if ( n < 10 )
    {
        return string ( 1, 'R' + n - 1 );
    }
    
    // Sequence RR, RS, RT, ... RZ, SS, ST, ... SZ, TT, TU, ... ZZ ***/
    
    else if ( n < 55 )
    {
        for ( n0 = n - 10, n1 = 0, n2 = 9; n2 <= n0; n1++, n2 += 9 - n1 )
            ;
        n2 = n0 - n2 + 9;

        return string ( 1, 'R' + n1 ) + string ( 1, 'R' + n2 );
    }
    
    // Sequence AA, AB, AC, ... AZ, BB, BC, ... BZ, CC, CD, ... QZ
    
    else if ( n < 335 )
    {
        for ( n0 = n - 55, n1 = 0, n2 = 25; n2 <= n0; n1++, n2 += 25 - n1 )
            ;
        n2 = n0 - n2 + 25;

        // J is skipped!
        
        if ( n1 >= 'J' - 'A' )
            n1++;
        
        if ( n2 >= 'J' - 'A' )
            n2++;
            
        return string ( 1, 'A' + n1 ) + string ( 1, 'A' + n2 );
    }
    
    // Sequence V335, V336, V337, V338, etc.
    
    else
    {
        return "V" + to_string ( n );
    }
    
    return "";
}

uint64_t string_to_dm ( string str )
{
    char     sign = 0, suffix = 0;
    int      zone = 0, num = 0;
    
    sscanf ( str.c_str(), "%c%d%d%c", &sign, &zone, &num, &suffix );

    if ( sign == '+' )
        sign = 1;
    else
        sign = 0;

    if ( suffix == 'a' || suffix == 'A' )
        suffix = 1;
    else if ( suffix == 'b' || suffix == 'B' )
        suffix = 2;
    else if ( suffix == 'n' || suffix == 'N' )
        suffix = 3;
    else if ( suffix == 'p' || suffix == 'P' )
        suffix = 4;
    else if ( suffix == 's' || suffix == 'S' )
        suffix = 5;
    else
        suffix = 0;
    
    return sign * 100000000 + zone * 1000000 + num * 10 + suffix;
}

string dm_to_string ( uint64_t dm )
{
    uint64_t sign = dm / 100000000;
    uint64_t zone = ( dm - sign * 100000000 ) / 1000000;
    uint64_t num = ( dm - sign * 100000000 - zone * 1000000 ) / 10;
    uint64_t suffix = dm - sign * 100000000 - zone * 1000000 - num * 10;
    
    if ( sign )
        sign = '+';
    else
        sign = '-';

    if ( suffix == 1 )
        suffix = 'a';
    else if ( suffix == 2 )
        suffix = 'b';
    else if ( suffix == 3 )
        suffix = 'n';
    else if ( suffix == 4 )
        suffix = 'p';
    else if ( suffix == 5 )
        suffix = 's';

    if ( suffix > 0 )
        return format ( "%c%02d %d%c", (int) sign, (int) zone, (int) num, (int) suffix );
    else
        return format ( "%c%02d %d", (int) sign, (int) zone, (int) num );
}

string tyc_to_string ( uint64_t tyc )
{
    uint64_t r = tyc / 1000000;
    uint64_t n = ( tyc - r * 1000000 ) / 10;
    uint64_t c = ( tyc - r * 1000000 - n * 10 );
    
    return format ( "%04d-%04d-%d", (int) r, (int) n, (int) c );
}

uint64_t string_to_tyc ( string str )
{
    char sep;
    int rgn = 0, num = 0, com = 0;

    sscanf ( str.c_str(), "%d%c%d%c%d", &rgn, &sep, &num, &sep, &com );

    if ( rgn >= 1 && rgn <= 9537 && num >= 1 && num <= 12121 && com >= 0 && com <= 4 )
        return (uint64_t) rgn * 1000000 + num * 10 + com;
    else
        return 0;
}

string gj_to_string ( uint64_t gj )
{
    uint64_t d = gj / 10;
    uint64_t c = gj - d * 10;
    
    static vector<string> compvec = { "", "A", "B", "C", "D" };
    string comps = compvec[ c ];

    if ( d % 10 == 0 )
        return format ( "%d", (int) d / 10 ) + comps;
    else
        return format ( "%.1f", d / 10.0 ) + comps;
}

uint64_t string_to_gj ( string str )
{
    size_t pos = str.find_first_of ( "ABCD" );
    int d = strtofloat64 ( str ) * 10.0 + 0.1;
    int c = pos == string::npos ? 0 : str[pos] - 'A' + 1;

    if ( d == 0 )
        return 0;
    else
        return 10 * d + c;
}

string glp_to_string ( uint64_t glp )
{
    uint64_t r = glp / 1000;
    uint64_t n = ( glp - r * 1000 );
    
    return format ( "%d-%d", (int) r, (int) n );
}

uint64_t string_to_glp ( string str )
{
    char    sign = 0;
    int     r = 0, n = 0;
    
    sscanf ( str.c_str(), "%d%c%d", &r, &sign, &n );
    if ( r > 0 && r < 1000 & n > 0 && n < 1000 )
        return r * 1000 + n;
    else
        return 0;
}

uint64_t string_to_wds ( string str )
{
    char    sign = 0;
    int     ra = 0, dec = 0;
    
    sscanf ( str.c_str(), "%d%c%d", &ra, &sign, &dec );
    
    if ( sign == '+' )
        sign = 1;
    else
        sign = 0;
        
    if ( ra >= 0 && ra < 24000 && dec >= 0 && dec < 9000 )
        return ra * 100000LL + sign * 10000 + dec;
    else
        return 0;
}

string wds_to_string ( uint64_t wds )
{
    uint64_t ra = wds / 100000;
    uint64_t sign = ( wds - ra * 100000 ) / 10000;
    uint64_t dec = wds - ra * 100000 - sign * 10000;
    
    if ( sign )
        sign = '+';
    else
        sign = '-';

    return format ( "%05d%c%04d", (int) ra, (int) sign, (int) dec );
}

uint64_t string_to_ngcic ( string str )
{
    int     num = 0;
    char    ext = 0;

    sscanf ( str.c_str(), "%d%c", &num, &ext );
    
    if ( ext >= 'A' && ext <= 'I' )
        ext = ext - 'A' + 1;
    else if ( ext >= 'a' && ext <= 'i' )
        ext = ext - 'a' + 1;
    else
        ext = 0;
            
    if ( num >= 0 && num <= 7840 )
        return num * 10 + ext;
    else
        return 0;
}

string ngcic_to_string ( uint64_t ngcic )
{
    uint64_t num = ngcic / 10;
    uint64_t ext = ngcic - num * 10;
    
    if ( ext > 0 )
        return format ( "%d%c", (int) num, (int) ext + 'A' - 1 );
    else
        return format ( "%d", (int) num );
}

uint64_t string_to_pngpk ( string str )
{
    double    lon = 0, lat = 0;
    int       londec = 0, latdec = 0;
    char      sign = 0;
    
    // Comvert whitespace in penultimate position to period.
    
    size_t pos = str.length() - 2;
    if ( str[pos] == ' ' )
        str[pos] = '.';
    
    sscanf ( str.c_str(), "%lf%c%lf", &lon, &sign, &lat );

    londec = lon * 10.0 + 0.1;
    latdec = lat * 10.0 + 0.1;
    
    if ( sign == '+' )
        sign = 1;
    else
        sign = 0;
    
    if ( londec >= 0 && londec < 3600 && latdec >= 0 && latdec < 900 )
        return londec * 10000 + sign * 1000 + latdec;
    else
        return 0;
}

string pngpk_to_string ( uint64_t pngpk, SSCatalog cat )
{
    uint64_t londec = pngpk / 10000;
    uint64_t sign = ( pngpk - londec * 10000 ) / 1000;
    uint64_t latdec = pngpk - londec * 10000 - sign * 1000;
    
    if ( sign )
        sign = '+';
    else
        sign = '-';
    
    if ( cat == kCatPNG )
        return format ( "%05.1f%c%04.1f", londec / 10.0, sign, latdec / 10.0 );
    else // kCatPK
        return format ( "%03.0f%c%04.1f", londec / 10.0, sign, latdec / 10.0 );
}

SSIdentifier::SSIdentifier ( void )
{
    _id = 0;
}

SSIdentifier::SSIdentifier ( uint64_t id )
{
    _id = id;
}

SSIdentifier::SSIdentifier ( SSCatalog catalog, uint64_t ident )
{
    if ( catalog == kCatGAIA )
        _id = ident | 0x8000000000000000;
    else
        _id = catalog * 10000000000000000LL + ident;
}

SSCatalog SSIdentifier::catalog ( void )
{
    if ( _id & 0x8000000000000000 )
        return kCatGAIA;
    else
        return static_cast<SSCatalog> ( _id / 10000000000000000LL );
}

uint64_t SSIdentifier::identifier ( void )
{
    if ( _id & 0x8000000000000000 )
        return _id & 0x7FFFFFFFFFFFFFFF;
    else
        return _id % 10000000000000000LL;
}

// Attempts to convert an indentifer in string form ("M 42", "alpha CMa", "HR 7001", "NGC 7992", etc.)
// to numeric form. The object type code, if other than kTypeNonexistent, may be used as a hint to
// resolve ambiguities. If Case Sensitivity matters (for example, if "M42" should convert but not "m42")
// then set (casesens) to true. Caution: case is important for many star identifiers. For example, "mu Cep"
// (Bayer star mu Cephei) is different from "MU Cep" (variable star MU Cephei).

SSIdentifier SSIdentifier::fromString ( const string &str, SSObjectType type, bool casesens )
{
    if ( _conmap.size() == 0 || _baymap.size() == 0 )
        mapinit();

    size_t len = str.length();

    // if string begins with "M", attempt to parse a Messier number
    
    if ( compare ( str, "M", 1, casesens ) == 0 && len > 1 )
    {
        uint64_t m = strtoint ( str.substr ( 1, len - 1 ) );
        if ( m > 0 && m <= 110 )
            return SSIdentifier ( kCatMessier, m );
    }

    // if string begins with "C", attempt to parse a Caldwell number
    
    if ( compare ( str, "C", 1, casesens ) == 0 && len > 1 )
    {
        uint64_t c = strtoint ( str.substr ( 1, len - 1 ) );
        if ( c > 0 && c <= 109 )
            return SSIdentifier ( kCatCaldwell, c );
    }

    // if string begins with "NGC", attempt to parse a New General Catalog identifier
    
    if ( compare ( str, "NGC", 3, casesens ) == 0 && len > 3 )
    {
        uint64_t ngc = string_to_ngcic ( str.substr ( 3, len - 3 ) );
        if ( ngc )
            return SSIdentifier ( kCatNGC, ngc );
    }

    // if string begins with "IC", attempt to parse an Index Catalog identifier
    
    if ( compare ( str, "IC", 2, casesens ) == 0 && len > 2 )
    {
        uint64_t ic = string_to_ngcic ( str.substr ( 2, len - 2 ) );
        if ( ic )
            return SSIdentifier ( kCatIC, ic );
    }

    // if string begins with "Mel", attempt to parse a Melotte open cluster identifier
    
    if ( compare ( str, "Mel", 3, casesens ) == 0 && len > 3 )
    {
        size_t mel = strtoint ( str.substr ( 3, len - 2 ) );
        if ( mel > 0 )
            return SSIdentifier ( kCatMel, mel );
    }

    // if string begins with "Sh2", attempt to parse a Sharpless bright nebula identifier
    
    if ( compare ( str, "Sh2", 3, casesens ) == 0 && len > 3 )
    {
        uint64_t sh2 = strtoint ( str.substr ( 3, len - 2 ) );
        if ( sh2 > 0 )
            return SSIdentifier ( kCatSh2, sh2 );
    }

    // if string begins with "LBN", attempt to parse a Lynds Bright Nebula identifier
    
    if ( compare ( str, "LBN", 3, casesens ) == 0 && len > 3 )
    {
        uint64_t lbn = strtoint ( str.substr ( 3, len - 2 ) );
        if ( lbn > 0 )
            return SSIdentifier ( kCatLBN, lbn );
    }

    // if string begins with "LDN", attempt to parse a Lynds Dark Nebula identifier
    
    if ( compare ( str, "LDN", 3, casesens ) == 0 && len > 3 )
    {
        uint64_t ldn = strtoint ( str.substr ( 3, len - 2 ) );
        if ( ldn > 0 )
            return SSIdentifier ( kCatLDN, ldn );
    }

    // if string begins with "PNG", attempt to parse a Galactic Planetary Nebula number
    
    if ( compare ( str, "PNG", 3, casesens ) == 0 && len > 3 )
    {
        uint64_t png = string_to_pngpk ( str.substr ( 3, len - 3 ) );
        if ( png )
            return SSIdentifier ( kCatPNG, png );
    }

    // if string begins with "PK", attempt to parse a Perek-Kohoutek planetary nebula number
    
    if ( compare ( str, "PK", 2, casesens ) == 0 && len > 2 )
    {
        uint64_t pk = string_to_pngpk ( str.substr ( 2, len - 2 ) );
        if ( pk )
            return SSIdentifier ( kCatPK, pk );
    }

    // if string begins with "PGC", attempt to parse a Principal Galaxy Catalog identifier
    
    if ( compare ( str, "PGC", 3, casesens ) == 0 && len > 3 )
    {
        uint64_t pgc = strtoint ( str.substr ( 3, len - 3 ) );
        if ( pgc )
            return SSIdentifier ( kCatPGC, pgc );
    }

    // if string begins with "UGCA", attempt to parse a Uppsala Galaxy Catalog Appendix identifier
    
    if ( compare ( str, "UGCA", 4, casesens ) == 0 && len > 4 )
    {
        uint64_t ugca = strtoint ( str.substr ( 4, len - 4 ) );
        if ( ugca )
            return SSIdentifier ( kCatUGCA, ugca );
    }
    
    // if string begins with "UGC", attempt to parse a Uppsala Galaxy Catalog identifier
    
    if ( compare ( str, "UGC", 3, casesens ) == 0 && len > 3 )
    {
        uint64_t ugc = strtoint ( str.substr ( 3, len - 3 ) );
        if ( ugc )
            return SSIdentifier ( kCatUGC, ugc );
    }

    // if string begins with "HR", attempt to parse a Harvard Revised (Bright Star) catalog identifier
    
    if ( compare ( str, "HR", 2, casesens ) == 0 )
    {
        size_t pos = str.find_first_of ( "0123456789" );
        if ( pos != string::npos )
            return SSIdentifier ( kCatHR, stoi ( str.substr ( pos, len - pos ) ) );
    }
    
    // if string begins with "HD", attempt to parse a Henry Draper catalog identifier
    
    if ( compare ( str, "HD", 2, casesens ) == 0 )
    {
        size_t pos = str.find_first_of ( "0123456789" );
        if ( pos != string::npos )
            return SSIdentifier ( kCatHD, stoi ( str.substr ( pos, len - pos ) ) );
    }

    // if string begins with "SAO", attempt to parse a Smithsonian Astrophyiscal Observatory catalog identifier
    
    if ( compare ( str, "SAO", 3, casesens ) == 0 )
    {
        size_t pos = str.find_first_of ( "0123456789" );
        if ( pos != string::npos )
            return SSIdentifier ( kCatSAO, stoi ( str.substr ( pos, len - pos ) ) );
    }

    // if string begins with "HIP", attempt to parse a Hipparcos catalog identifier
    
    if ( compare ( str, "HIP", 3, casesens ) == 0 )
    {
        size_t pos = str.find_first_of ( "0123456789" );
        if ( pos != string::npos )
            return SSIdentifier ( kCatHIP, stoi ( str.substr ( pos, len - pos ) ) );
    }

    // if string begins with "TYC", attempt to parse a Tycho catalog identifier
    
    if ( compare ( str, "TYC", 3, casesens ) == 0 )
    {
        size_t pos = str.find_first_of ( "0123456789" );
        if ( pos != string::npos )
            return SSIdentifier ( kCatTYC, string_to_tyc ( str.substr ( pos, len - pos ) ) );
    }

    // if string begins with "GAIA", attempt to parse a GAIA catalog identifier
    
    if ( compare ( str, "GAIA", 4, casesens ) == 0 )
    {
        size_t pos = str.find_first_of ( "0123456789" );
        if ( pos != string::npos )
            return SSIdentifier ( kCatGAIA, stoll ( str.substr ( pos, len - pos ) ) );
    }

    // if string begins with "BD" or "SD", attempt to parse a Bonner Durchmusterung catalog identifier
    // Note: "SD" is abbrevieation for Southern Durchmusterung, found in SKY2000 Master Star Catalog.
    
    if ( compare ( str, "BD", 2, casesens ) == 0 || compare ( str, "SD", 2, casesens ) == 0 )
    {
        size_t pos = str.find_first_of ( "+-" );
        if ( pos != string::npos )
            return SSIdentifier ( kCatBD, string_to_dm ( str.substr ( pos, len - pos ) ) );
    }

    // if string begins with "CD", attempt to parse a Bonner Durchmusterung catalog identifier
    
    if ( compare ( str, "CD", 2, casesens ) == 0 )
    {
        size_t pos = str.find_first_of ( "+-" );
        if ( pos != string::npos )
            return SSIdentifier ( kCatCD, string_to_dm ( str.substr ( pos, len - pos ) ) );
    }

    // if string begins with "CP", attempt to parse a Bonner Durchmusterung catalog identifier
    
    if ( compare ( str, "CP", 2, casesens ) == 0 )
    {
        size_t pos = str.find_first_of ( "+-" );
        if ( pos != string::npos )
            return SSIdentifier ( kCatCP, string_to_dm ( str.substr ( pos, len - pos ) ) );
    }

    // if string begins with "WDS", attempt to parse a Washington Double Star catalog identifier
    
    if ( compare ( str, "WDS", 3, casesens ) == 0 && len > 3 )
    {
        uint64_t wds = string_to_wds ( str.substr ( 3, len - 3 ) );
        if ( wds )
            return SSIdentifier ( kCatWDS, wds );
    }
    
    // if string begins with "GJ", "Gl", "Wo", or "NN", attempt to parse a Gliese-Jahreiss Nearby Star Catalog identifier
    
    if ( ( compare ( str, "GJ", 2, casesens ) == 0 || compare ( str, "Gl", 2, casesens ) == 0 || compare ( str, "NN", 2, casesens ) == 0 || compare ( str, "Wo", 2, casesens ) == 0 ) && len > 2 )
    {
        uint64_t gj = string_to_gj ( str.substr ( 2, len - 2 ) );
        if ( gj )
            return SSIdentifier ( kCatGJ, gj );
    }

    // if string begins with "G", attempt to parse a Giclas catalog identifier
    
    if ( compare ( str, "G", 1, casesens ) == 0 && len > 3 )
    {
        uint64_t glp = string_to_glp ( str.substr ( 1, len - 1 ) );
        if ( glp )
            return SSIdentifier ( kCatGiclas, glp );
    }

    // if string begins with "LP", attempt to parse a Luyten-Palomar catalog identifier
    
    if ( compare ( str, "LP", 2, casesens ) == 0 && len > 3 )
    {
        uint64_t glp = string_to_glp ( str.substr ( 2, len - 2 ) );
        if ( glp )
            return SSIdentifier ( kCatLP, glp );
    }

    // if string begins with "L", attempt to parse a Luyten catalog identifier
    
    if ( compare ( str, "L", 1, casesens ) == 0 && len > 3 )
    {
        uint64_t glp = string_to_glp ( str.substr ( 1, len - 1 ) );
        if ( glp )
            return SSIdentifier ( kCatLuyten, glp );
    }

    // Tokenize string into words separated by whitespace.
    // if last token is a constellation abbrevation,
    // attempt to parse Bayer/Flamsteed/GCVS identifier.

    vector<string> tokens = tokenize ( str, " " );
    int con = tokens.size() >= 2 ? string_to_con ( tokens[ tokens.size() - 1 ], casesens ) : 0;
    if ( con )
    {
        string constr = tokens[1];

        // try parsing first token as a variable star designation; return GCVS identifier if successful.
        
        uint64_t var = string_to_gcvs ( tokens[0], casesens );
        if ( var > 0 )
            return SSIdentifier ( kCatGCVS, con * 10000 + var );
        
        // If first token begins with a number, return a Flamsteed catalog identification
        
        size_t pos = tokens[0].find_first_of ( "0123456789" );
        if ( pos == 0 )
            return SSIdentifier ( kCatFlamsteed, con * 10000 + strtoint ( tokens[0] ) );

        // If first token contains a number, convert numeric portion of token to integer,
        // then erase numeric portion of token. If we have 3 tokens, convert middle to integer.
        // This is the (optional) superscript after a Bayer letter.
        
        int num = 0;
        if ( pos != string::npos )
        {
            num = strtoint ( tokens[0].substr ( pos, string::npos ) );
            tokens[0].erase ( pos, string::npos );
        }
        else if ( tokens.size() == 3 )
            num = strtoint ( tokens[1] );
        
        // Try parsing first token as a Bayer letter.  If successful, return
        // a Bayer designation with the numeric portion (if any) as superscript
        
        int bay = string_to_bayer ( tokens[0], casesens );
        if ( bay > 0 )
            return SSIdentifier ( kCatBayer, con * 10000 + bay * 10 + num );
    }
    
    // if string is a number inside paratheses, attempt to parse as an asteroid number
    
    if ( str[0] == '(' && str[len - 1] == ')' )
    {
        uint64_t n = strtoint ( str.substr ( 1, len - 2 ) );
        if ( n > 0 )
            return SSIdentifier ( kCatAstNum, n );
    }
    
    // if string is a number followed by "P" (or "p" if case-insensitive),
    // parse as a periodic comet number
    
    size_t pos = str.find ( "P" );
    if ( ! casesens && pos == string::npos )
        pos = str.find ( "p" );
    
    if ( pos != string::npos )
    {
        uint64_t n = strtoint ( str.substr ( 0, pos ) );
        if ( n > 0 )
            return SSIdentifier ( kCatComNum, n );
    }

    // If we have a numeric string and a solar system object type,
    // deduce identifier from number and type code.

    if ( isNumeric ( str ) )
    {
        uint64_t n = strtoint ( str );
        
        if ( ( type == kTypePlanet || type == kTypeMoon ) && n >= 0 )
            return SSIdentifier ( kCatJPLanet, n );
        
        if ( type == kTypeAsteroid && n > 0 )
            return SSIdentifier ( kCatAstNum, n );

        if ( type == kTypeComet && n > 0 )
            return SSIdentifier ( kCatComNum, n );

        if ( type == kTypeSatellite && n > 0 )
            return SSIdentifier ( kCatNORADSat, n );
    }
    
    // We give up!  Return unknown identifier.
    
    return SSIdentifier ( kCatUnknown, 0 );
}

string SSIdentifier::toString ( void )
{
    if ( _conmap.size() == 0 || _baymap.size() == 0 )
        mapinit();

    SSCatalog cat = catalog();
    uint64_t id = identifier();
    string str = "";
    
    if ( cat == kCatBayer )
    {
        uint64_t con = ( id / 10000 );
        uint64_t bay = ( id - 10000 * con ) / 10;
        uint64_t num = ( id - 10000 * con ) - bay * 10;
        
        string baystr = bayer_to_string ( bay );
        string constr = _convec[con - 1];
        if ( num > 0 )
            str = baystr + to_string ( num ) + " " + constr;
        else
            str = baystr + " " + constr;
    }
    else if ( cat == kCatFlamsteed )
    {
        uint64_t con = id / 10000;
        uint64_t num = id - 10000 * con;
        str = to_string ( num ) + " " + _convec[con - 1];
    }
    else if ( cat == kCatGCVS )
    {
        uint64_t con = id / 10000;
        uint64_t num = id - 10000 * con;
        str = gcvs_to_string ( num ) + " " + _convec[con - 1];
    }
    else if ( cat == kCatHR )
    {
        str = "HR " + to_string ( id );
    }
    else if ( cat == kCatHD )
    {
        str = "HD " + to_string ( id );
    }
    else if ( cat == kCatSAO )
    {
        str = "SAO " + to_string ( id );
    }
    else if ( cat == kCatHIP )
    {
        str = "HIP " + to_string ( id );
    }
    else if ( cat == kCatTYC )
    {
        str = "TYC " + tyc_to_string ( id );
    }
    else if ( cat == kCatGAIA )
    {
        str = "GAIA " + to_string ( id );
    }
    else if ( cat == kCatBD )
    {
        str = "BD " + dm_to_string ( id );
    }
    else if ( cat == kCatCD )
    {
        str = "CD " + dm_to_string ( id );
    }
    else if ( cat == kCatCP )
    {
        str = "CP " + dm_to_string ( id );
    }
    else if ( cat == kCatWDS )
    {
        str = "WDS " + wds_to_string ( id );
    }
    else if ( cat == kCatGJ )
    {
        str = "GJ " + gj_to_string ( id );
    }
    else if ( cat == kCatGiclas )
    {
        str = "G " + glp_to_string ( id );
    }
    else if ( cat == kCatLuyten )
    {
        str = "L " + glp_to_string ( id );
    }
    else if ( cat == kCatLP )
    {
        str = "LP " + glp_to_string ( id );
    }
    else if ( cat == kCatMessier )
    {
        str = "M " + to_string ( id );
    }
    else if ( cat == kCatCaldwell )
    {
        str = "C " + to_string ( id );
    }
    else if ( cat == kCatNGC )
    {
        str = "NGC " + ngcic_to_string ( id );
    }
    else if ( cat == kCatIC )
    {
        str = "IC " + ngcic_to_string ( id );
    }
    else if ( cat == kCatMel )
    {
        str = "Mel " + to_string ( id );
    }
    else if ( cat == kCatSh2 )
    {
        str = "Sh2 " + to_string ( id );
    }
    else if ( cat == kCatLBN )
    {
        str = "LBN " + to_string ( id );
    }
    else if ( cat == kCatLDN )
    {
        str = "LDN " + to_string ( id );
    }
    else if ( cat == kCatPNG )
    {
        str = "PNG " + pngpk_to_string ( id, cat );
    }
    else if ( cat == kCatPK )
    {
        str = "PK " + pngpk_to_string ( id, cat );
    }
    else if ( cat == kCatPGC )
    {
        str = "PGC " + to_string ( id );
    }
    else if ( cat == kCatUGC )
    {
        str = "UGC " + to_string ( id );
    }
    else if ( cat == kCatUGCA )
    {
        str = "UGCA " + to_string ( id );
    }
    else if ( cat == kCatAstNum )
    {
        str = "(" + to_string ( id ) + ")";
    }
    else if ( cat == kCatComNum )
    {
        str = to_string ( id ) + "P";
    }
    else if ( cat == kCatJPLanet )
    {
        str = to_string ( id );
    }
    else if ( cat == kCatNORADSat )
    {
        str = to_string ( id );
    }
    
    return str;
}

// Strips Bayer superscripts (alpha1 Cen -> alpha Cen),
// GJ components (GJ 559B -> GJ 559)
// DM suffixes (BD+04 3561a -> BD+04 3561), and
// NGC and IC suffixes (NGC 7318A -> NGC 7318, IC 1318B -> IC 1318).
// All other identifiers are returned unchanged.

SSIdentifier SSIdentifier::strip ( void )
{
    SSCatalog cat = catalog();
    uint64_t id = identifier();
    
    if ( cat == kCatBayer )
    {
        uint64_t con = ( id / 10000 );
        uint64_t bay = ( id - 10000 * con ) / 10;
        return SSIdentifier ( cat, con * 10000 + bay * 10 );
    }
    else if ( cat == kCatGJ )
    {
        uint64_t gj = id / 10;
        return SSIdentifier ( cat, 10 * gj );
    }
    else if ( cat == kCatBD || cat == kCatCD || cat == kCatCP )
    {
        uint64_t sign = id / 100000000;
        uint64_t zone = ( id - sign * 100000000 ) / 1000000;
        uint64_t num = ( id - sign * 100000000 - zone * 1000000 ) / 10;
        return SSIdentifier ( cat, sign * 100000000 + zone * 1000000 + num * 10 );
    }
    else if ( cat == kCatNGC || cat == kCatIC )
    {
        uint64_t num = id / 10;
        return SSIdentifier ( cat, num * 10 );
    }
    
    return *this;
}

bool compareSSIdentifiers ( const SSIdentifier &id1, const SSIdentifier &id2 )
{
    return id1 < id2;
}

// Reads identifier-to-name map from filename and stores results in nameMap.
// Returns number of identifer-to-name pairs inserted into nameMap.

int SSImportIdentifierNameMap ( const string &filename, SSIdentifierNameMap &nameMap )
{
    // Open file; return on failure.

    ifstream file ( filename );
    if ( ! file )
        return 0;

    // Read file line-by-line until we reach end-of-file

    string line = "";
    int paircount = 0;
    
    while ( getline ( file, line ) )
    {
        // Split line into tokens separated by commas.
        // Require at least 2 tokens.  First token is name.
        
        vector<string> tokens = split ( line, "," );
        if ( tokens.size() < 2 )
            continue;
        
        // For each token after the first, attempt to generate an identifier.
        // If successful, insert an identifier-name pair into the map.
        
        for ( int k = 1; k < tokens.size(); k++ )
        {
            SSIdentifier ident = SSIdentifier::fromString ( tokens[k] );
            if ( ident )
            {
                nameMap.insert ( { ident, tokens[0] } );
                paircount++;
            }
        }
    }
    
    return paircount;
}

// Given a vector of identifiers, returns vector of all corresponding name strings
// from the input identifier-to-name map.  If no names correspond to any identifier,
// returns a zero-length vector.

vector<string> SSIdentifiersToNames ( SSIdentifierVec &idents, SSIdentifierNameMap &nameMap )
{
    vector<string> names;

    for ( SSIdentifier ident : idents )
    {
        auto nameRange = nameMap.equal_range ( ident );
        for ( auto i = nameRange.first; i != nameRange.second; i++ )
        {
            string name = i->second;
            
            if ( name.length() > 0 && find ( names.begin(), names.end(), name ) == names.end() )
                names.push_back ( name );
        }
    }

    return names;
}

// Returns identifier in a specific catalog from a vector of identifiers/
// If not present in the vector, returns null identifier (i.e. zero).

SSIdentifier SSGetIdentifier ( SSCatalog cat, SSIdentifierVec &identVec )
{
    for ( int i = 0; i < identVec.size(); i++ )
        if ( identVec[i].catalog() == cat )
            return identVec[i];
    
    return SSIdentifier();
}

// Adds a new identifier to a vector of identifiers,
// if the new identifier is valid and not already present in the vector.
// Returns true if identifier was added, false otherwise;

bool SSAddIdentifier ( SSIdentifier ident, SSIdentifierVec &identVec )
{
    if ( ident && find ( identVec.begin(), identVec.end(), ident ) == identVec.end() )
    {
        identVec.push_back ( ident );
        return true;
    }
    
    return false;
}

// Given a key identifier (key) and a mapping of identifiers to other identifiers (map),
// adds other idenfiers corresponding to key to a vector of identifiers (idents)
// Adds identifiers only if valid and not already present in the vector.
// Returns number of new identifiers added to idents vector.

int SSAddIdentifiers ( SSIdentifier key, const SSIdentifierMap &map, SSIdentifierVec &idents )
{
    int n = 0;
    
    auto range = map.equal_range ( key );
    for ( auto i = range.first; i != range.second; i++ )
        if ( SSAddIdentifier ( i->second, idents ) )
            n++;
    
    return n;
}

// Adds a new common name string to a vector of names,
// if the new name is valid and not already present in the vector.
// Returns true if name string was added, false otherwise;

bool SSAddCommonName ( const string &name, vector<string> &names )
{
    if ( name.length() && find ( names.begin(), names.end(), name ) == names.end() )
    {
        names.push_back ( name );
        return true;
    }
    
    return false;
}
