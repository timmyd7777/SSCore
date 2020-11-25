//  SSFeature.cpp
//  SSCore
//
//  Created by Tim DeBenedictis on 11/18/20.
//  Copyright © 2020 Southern Stars. All rights reserved.

#include "SSCoordinates.hpp"
#include "SSFeature.hpp"

// Constructs single Feature with a specific object type code.
// All fields except type code are set to empty strings or infinity,
// signifying unknown/undefined values.

SSFeature::SSFeature ( void ) : SSObject ( kTypeFeature )
{
    _name = "";
    _clean_name = "";
    _target = "";
    _type_code = "";
    _origin = "";
    _diameter = INFINITY;
    _lat = INFINITY;
    _lon = INFINITY;
}

// Allocates a new SSFeature and initializes it from a CSV-formatted string.
// Returns nullptr on error (invalid CSV string, heap allocation failure, etc.)

SSObjectPtr SSFeature::fromCSV ( string csv )
{
    if ( csv.size() == 0 )
        return nullptr;

    // split string into comma-delimited fields,
    // remove leading & trailing whitespace/line breaks from each field.
    
    vector<string> fields = split_csv ( csv );
    for ( int i = 0; i < fields.size(); i++ )
        fields[i] = trim ( fields[i] );
    
    SSObjectType type = SSObject::codeToType ( fields[0] );
    if ( type != kTypeFeature && type != kTypeCity )
        return nullptr;
    
    // Set expected field index for first identifier based on object type.
    // Verify that we have the required number if fiels and return if not.
    
    int fid;
    if ( type == kTypeCity )
        fid = 13;
    else
        fid = 9;
    
    if ( fields.size() < fid )
        return nullptr;

    SSObjectPtr pObject = SSNewObject ( type );
    SSFeaturePtr pFeature = SSGetFeaturePtr ( pObject );
    SSCityPtr pCity = SSGetCityPtr ( pObject );

    if ( pFeature == nullptr )
        return nullptr;
    
    pFeature->setName ( fields[1] );
    pFeature->setCleanName ( fields[2] );

    if ( type == kTypeFeature )
    {
        pFeature->setTarget ( fields[3] );
        pFeature->setDiameter ( strtofloat64 ( fields[4] ) );
        pFeature->setLatitude ( strtofloat64 ( fields[5] ) );
        pFeature->setLongitude ( strtofloat64 ( fields[6] ) );
        pFeature->setFeatureTypeCode ( fields[7] );
        pFeature->setOrigin ( fields[8] );
    }

    if ( type == kTypeCity )
    {
        pFeature->setLatitude ( strtofloat64 ( fields[3] ) );
        pFeature->setLongitude ( strtofloat64 ( fields[4] ) );
        pCity->setCountryCode ( fields[5] );
        pCity->setAdmin1Code ( fields[6] );
        pCity->setPopulation ( strtoint ( fields[7] ) );
        if ( fields[8] != "" ) pCity->setElevation ( strtofloat ( fields[8] ) );
        pCity->setTimezoneName ( fields[9] );
        pCity->setAdmin1Name ( fields[10] );
        pCity->setDaylightSaving ( !!strtoint( fields[11] ) );
        pCity->setTimezoneRawOffset ( strtofloat64 ( fields[12] ) );
    }

    return pObject;
}

string SSFeature::toCSV ( void )
{
    string csv = SSObject::typeToCode ( _type ) + ",";

    csv += "\"" + _name + "\",";
    csv += "\"" + _clean_name + "\",";
    csv += "\"" + _target + "\",";
    csv += format ( "%.2f", _diameter ) + ",";
    csv += format ( "%.5f", _lat ) + ",";
    csv += format ( "%.5f", _lon ) + ",";
    csv += _type_code + ",";
    csv += "\"" + _origin + "\",";

    return csv;
}

// Constructs single city.
// All fields except type code are set to empty strings or infinity,
// signifying unknown/undefined values.

SSCity::SSCity ( void ) : SSFeature()
{
    _type = kTypeCity;
    _country_code = "";
    _admin1_code = "";
    _admin1_name = "";
    _timezone_name = "";
    _elevation = -1;
    _population = -1;
    _daylight_saving = false;
    _timezone_raw_offset = INFINITY;
}

// Returns the CSV string of the SSCity object.
// Values should match the source CSV file for easy comparison

string SSCity::toCSV ( void )
{
    string csv = SSObject::typeToCode ( _type ) + ",";
    
    csv += _name + ",";
    csv += _clean_name + ",";
    csv += format ( "%.5f,", _lat );
    csv += format ( "%.5f,", _lon );
    csv += _country_code + ",";
    csv += _admin1_code + ",";
    csv += to_string( _population ) + ",";

    if ( _elevation != -1 )
        csv += to_string( _elevation );
    csv += ",";

    csv += _timezone + ",";
    
    size_t pos;
    if ( ( pos = _admin1_name.find ( "," ) ) != string::npos ) {
        csv += "\"" + _admin1_name + "\",";
    } else {
        csv += _admin1_name + ",";
    }
    
    csv += _daylight_saving ? "1," : "0,";

    if ( int (_timezone_raw_offset * 100 ) % 10 != 0 ) {
        csv += format ( "%.2f", _timezone_raw_offset );
    } else {
        csv += format ( "%.1f", _timezone_raw_offset );
    }

    return csv;
}

// Downcasts generic SSObject pointer to SSFeature pointer.
// Returns nullptr if pointer is not an instance of SSFeature!

SSFeaturePtr SSGetFeaturePtr ( SSObjectPtr ptr )
{
    return dynamic_cast<SSFeaturePtr> ( ptr );
}

// Downcasts generic SSObject pointer to SSCity pointer.
// Returns nullptr if pointer is not an instance of SSCity!

SSCityPtr SSGetCityPtr ( SSObjectPtr ptr )
{
    return dynamic_cast<SSCityPtr> ( ptr );
}
