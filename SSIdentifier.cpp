//  SSIdentifier.cpp
//  SSCore
//
//  Created by Tim DeBenedictis on 3/20/20.
//  Copyright © 2020 Southern Stars. All rights reserved.

#include "SSIdentifier.hpp"
#include <vector>
#include <map>

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

static int string_to_bayer ( string str )
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
			if ( _bayvec[i].compare ( 0, len, str ) == 0 )
				return i + 1;
	}
	
	return 0;
}

static string bayer_to_string ( int bay )
{
	if ( bay > 50 )
		return string ( 1, bay - 51 + 'A' );
	else if ( bay > 24 )
		return string ( 1, bay - 25 + 'a' );
	else
		return _bayvec[ bay - 1 ];
}

static int string_to_variable ( string str )
{
	size_t len = str.length();
	int n1 = 0, n2 = 0;
	
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
		return stoi ( str.substr ( 1, len - 1 ) );
	}
		
	return 0;
}

string variable_to_string ( int n )
{
	int n0 = 0, n1 = 0, n2 = 0;
	
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

static void mapinit ( void )
{
	for ( int i = 0; i < _bayvec.size(); i++ )
		_baymap.insert ( { _bayvec[i], i + 1 } );

	for ( int i = 0; i < _convec.size(); i++ )
		_conmap.insert ( { _convec[i], i + 1 } );
}

SSIdentifier::SSIdentifier ( void )
{
	_id = 0;
}

SSIdentifier::SSIdentifier ( SSCatalog catalog, int64_t id )
{
	_id = ( (int64_t) catalog ) << 56 | ( id & 0x00ffffffffffffff );
}

SSCatalog SSIdentifier::catalog ( void )
{
	return static_cast<SSCatalog> ( _id >> 56 );
}

int64_t SSIdentifier::identifier ( void )
{
	return _id & 0x00ffffffffffffff;
}

SSIdentifier SSIdentifier::fromString ( string str )
{
	size_t len = str.length();
	
	if ( _conmap.size() == 0 || _baymap.size() == 0 )
		mapinit();

	// if string begins with "HR", attempt to parse a Harvard Revised (Bright Star) catalog identifier
	
	if ( str.find ( "HR" ) == 0 )
	{
		size_t pos = str.find_first_of ( "0123456789" );
		if ( pos != string::npos )
			return SSIdentifier ( kCatHR, stoi ( str.substr ( pos, len - pos ) ) );
	}
	
	// if string begins with "HD", attempt to parse a Henry Draper catalog identifier
	
	if ( str.find ( "HD" ) == 0 )
	{
		size_t pos = str.find_first_of ( "0123456789" );
		if ( pos != string::npos )
			return SSIdentifier ( kCatHD, stoi ( str.substr ( pos, len - pos ) ) );
	}

	// if string begins with "SAO", attempt to parse a Smithsonian Astrophyiscal Observatory catalog identifier
	
	if ( str.find ( "SAO" ) == 0 )
	{
		size_t pos = str.find_first_of ( "0123456789" );
		if ( pos != string::npos )
			return SSIdentifier ( kCatSAO, stoi ( str.substr ( pos, len - pos ) ) );
	}

	// if string begins with "HIP", attempt to parse a Hipparcos catalog identifier
	
	if ( str.find ( "HIP" ) == 0 )
	{
		size_t pos = str.find_first_of ( "0123456789" );
		if ( pos != string::npos )
			return SSIdentifier ( kCatHIP, stoi ( str.substr ( pos, len - pos ) ) );
	}

	// parse constellation abbreviation
	
	string constr = "";
	size_t consep = str.find_first_of ( " " );
	if ( consep != string::npos && consep + 3 <= str.length() )
		constr = str.substr ( consep + 1, 3 );
	
	// get constellation number; return unknown id if abbreviation not recognized
	
	int con = _conmap[ constr ];
	if ( ! con )
		return SSIdentifier ( kCatUnknown, 0 );
	
	// try parsing prefix as a variable star designation; return GCVS identifier if successful.
	
	string varstr = str.substr ( 0, consep );
	int var = string_to_variable ( varstr );
	if ( var > 0 )
		return SSIdentifier ( kCatGCVS, var * 100 + con );
	
	// Find numeric portion of string, if any, and convert to integer
	
	string numstr = "";
	size_t numsep = str.find_first_of ( "0123456789" );
	if ( numsep != string::npos )
		numstr = str.substr ( numsep, str.find_last_of ( "0123456789" ) - numsep + 1 );
	int num = numstr.empty() ? 0 : stoi ( numstr );
	
	// If string begins with a number, return a Flamsteed catalog identification
	
	if ( numsep == 0 )
		return SSIdentifier ( kCatFlamsteed, num * 100 + con );

	// Otherwise, extract first non-numeric part of string and parse as a Bayer designation
	// with the numeric portion (if any) as superscript
	
	string baystr = "";
	if ( numsep == string::npos )
		baystr = str.substr ( 0, consep );
	else
		baystr = str.substr ( 0, numsep );
	
	int bay = string_to_bayer ( baystr );
	if ( bay > 0 )
		return SSIdentifier ( kCatBayer, ( bay * 100 + num ) * 100 + con );
	
	// Return unknown identifier
	
	return SSIdentifier ( kCatUnknown, 0 );
}

string SSIdentifier::toString ( void )
{
	if ( _conmap.size() == 0 || _baymap.size() == 0 )
		mapinit();

	SSCatalog cat = catalog();
	int64_t id = identifier();
	string str = "";
	
	if ( cat == kCatBayer )
	{
		int bay = id / 10000;
		int num = ( id - bay * 10000 ) / 100;
		int con = id % 100;
		
		string baystr = bayer_to_string ( bay );
		string constr = _convec[con - 1];
		if ( num > 0 )
			str = baystr + to_string ( num ) + " " + constr;
		else
			str = baystr + " " + constr;
	}
	else if ( cat == kCatFlamsteed )
	{
		int num = id / 100;
		int con = id % 100;
		str = to_string ( num ) + " " + _convec[con - 1];
	}
	else if ( cat == kCatGCVS )
	{
		int num = id / 100;
		int con = id % 100;
		str = variable_to_string ( num ) + " " + _convec[con - 1];
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

	return str;
}

bool compareSSIdentifiers ( SSIdentifier id1, SSIdentifier id2 )
{
	return id1 < id2;
}
