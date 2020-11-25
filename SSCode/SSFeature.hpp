// SSFeature.hpp
// SSCore
//
// Created by Tim DeBenedictis on 11/18/20.
//
// Copyright © 2020 Southern Stars. All rights reserved.
//
// This subclass of SSObject represents planetary surface features.

#ifndef SSFeature_hpp
#define SSFeature_hpp

#include "SSObject.hpp"

#pragma pack ( push, 1 )

// This subclass of SSObject stores basic data for planetary surface features
// from the IAU Gazetteer of Planetary Nomenclature:
// https://planetarynames.wr.usgs.gov/AdvancedSearch
// For details, see https://planetarynames.wr.usgs.gov/Page/Specifics

class SSFeature : public SSObject
{
protected:

    string _name, _clean_name;              // feature name & clean feature name
    string _target;                         // planetary object to which the feature belongs; if empty string, implies Earth.
    string _type_code;                      // feature type code; see https://planetarynames.wr.usgs.gov/DescriptorTerms
    string _origin;                         // feature origin (short explanation of name)
    float  _diameter;                       // diameter in kilometers
    float  _lat, _lon;                      // center latitude & longitude in degrees. East and North are positive.
    
public:
    
    SSFeature ( void );

    // modifiers
    
    void setName ( string name ) { _name = name; }
    void setCleanName ( string clean_name ) { _clean_name = clean_name; }
    void setTarget ( string target ) { _target = target; }
    void setFeatureTypeCode ( string type_code ) { _type_code = type_code; }
    void setOrigin ( string origin ) { _origin = origin; }
    void setDiameter ( double diameter ) { _diameter = diameter; }
    void setLat ( double lat ) { _lat = lat; }
    void setLon ( double lon ) { _lon = lon; }

    // imports/exports from/to CSV-format text string
    
    static SSObjectPtr fromCSV ( string csv );
    virtual string toCSV ( void );
};

// This subclass of SSFeature stores data for cities obtained from:
// http://download.geonames.org/export/dump/cities1000.zip
// For details see http://download.geonames.org/export/dump/readme.txt

class SSCity : public SSFeature
{
protected:

    string _country_code;                   // 2-letter country code
    string _admin1_code, _admin1_name;      // administrative division 1:  2-letter code & name
    string _timezone;                       // time zone name (e.g. "America/Los_Angeles")
    float _elevation;                       // elevation in meters, -1 means unknown
    int _population;                        // population, -1 means unknown
    bool _daylight_saving;                  // whether the city observes daylight saving time
    float _timezone_raw_offset;             // offset from GMT in hours, independent of daylight saving time; east is positive
    
public:
    
    SSCity ( void );

    // modifiers
    
    void setCountryCode ( string country_code ) { _country_code = country_code; }
    void setAdmin1Code ( string admin1_code ) { _admin1_code = admin1_code; }
    void setAdmin1Name ( string admin1_name ) { _admin1_name = admin1_name; }
    void setTimezone ( string timezone ) { _timezone = timezone; }
    void setElevation ( int elevation ) { _elevation = elevation; }
    void setPopulation ( int population ) { _population = population; }
    void setDaylightSaving ( bool daylight_saving ) { _daylight_saving = daylight_saving; }
    void setTimezoneRawOffset ( double timezone_raw_offset ) { _timezone_raw_offset = timezone_raw_offset; }

    // imports/exports from/to CSV-format text string
    
    static SSObjectPtr fromCSV ( string csv );
    virtual string toCSV ( void );
};

#pragma pack ( pop )

// convenient aliases for pointers to various subclasses of SSFeature

typedef SSFeature *SSFeaturePtr;
typedef SSCity *SSCityPtr;

// These functions downcast a pointer from the SSObject base class to its various SSFeature subclasses.
// They all return null pointers if the input object pointer is not an instance of the expected derived class.

SSFeaturePtr SSGetFeaturePtr ( SSObjectPtr ptr );
SSCityPtr SSGetCityPtr ( SSObjectPtr ptr );

#endif /* SSFeature_hpp */
