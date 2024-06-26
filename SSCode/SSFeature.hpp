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
#include "SSPlanet.hpp"

#pragma pack ( push, 1 )

// This subclass of SSObject stores basic data for planetary surface features
// from the IAU Gazetteer of Planetary Nomenclature:
// https://planetarynames.wr.usgs.gov/AdvancedSearch
// For details, see https://planetarynames.wr.usgs.gov/Page/Specifics

class SSFeature : public SSObject
{
protected:
//  string _name;                           // feature name; may contain funky UTF8 characters. Stored in SSObject::_names[0].
//  string _clean_name;                     // clean feature name; only contains ASCII characters. Stored in SSObject::_names[1].
    string _target;                         // name of planetary object to which the feature belongs.
    string _type_code;                      // feature type code; see https://planetarynames.wr.usgs.gov/DescriptorTerms
    string _origin;                         // feature origin (short explanation of name)
    float  _diameter;                       // diameter in kilometers
    float  _lat, _lon;                      // center latitude & longitude in degrees. East and North are positive.
    
public:
    
    SSFeature ( void );

    // accessors

    string getName ( void ) { return _names[0]; }
    string getCleanName ( void ) { return _names[1]; }
    string getTarget ( void ) { return _target; }
    string getFeatureTypeCode ( void ) { return _type_code; }
    string getOrigin ( void ) { return _origin; }
    double getDiameter ( void ) { return _diameter; }
    double getLatitude ( void ) { return _lat; }
    double getLongitude ( void ) { return _lon; }

    // modifiers
    
    void setName ( string name ) { _names[0] = name; }
    void setCleanName ( string clean_name ) { _names[1] = clean_name; }
    void setTarget ( string target ) { _target = target; }
    void setFeatureTypeCode ( string type_code ) { _type_code = type_code; }
    void setOrigin ( string origin ) { _origin = origin; }
    void setDiameter ( double diameter ) { _diameter = diameter; }
    void setLatitude ( double lat ) { _lat = lat; }
    void setLongitude ( double lon ) { _lon = lon; }

    // imports/exports from/to CSV-format text string
    
    static SSObjectPtr fromCSV ( string csv );
    virtual string toCSV ( void );
    
    // computes apparent direction and distance; planet must already have ephemeris computed.
    
    void computeEphemeris ( SSPlanet *pPlanet );
};

// This subclass of SSFeature stores data for cities obtained from:
// http://download.geonames.org/export/dump/cities1000.zip
// For details see http://download.geonames.org/export/dump/readme.txt

class SSCity : public SSFeature
{
protected:

    string _country_code;                   // 2-letter country code
    string _admin1_code, _admin1_name;      // administrative division 1:  2-letter code & name
    string _timezone_name;                  // time zone name (e.g. "America/Los_Angeles")
    float _elevation;                       // elevation in meters, -1 means unknown
    int _population;                        // population, -1 means unknown
    bool _daylight_saving;                  // whether the city observes daylight saving time
    float _timezone_raw_offset;             // offset from GMT in hours, independent of daylight saving time; east is positive
    
public:
    
    SSCity ( void );

    // accessors

    string getCountryCode ( void ) { return _country_code; }
    string getAdmin1Code ( void ) { return _admin1_code; }
    string getAdmin1Name ( void ) { return _admin1_name; }
    string getTimezoneName ( void ) { return _timezone_name; }
    float getElevation ( void ) { return _elevation; }
    int getPopulation ( void ) { return _population; }
    bool getDaylightSaving ( void ) { return _daylight_saving; }
    float getTimezoneRawOffset ( void ) { return _timezone_raw_offset; }

    // modifiers
    
    void setCountryCode ( string country_code ) { _country_code = country_code; }
    void setAdmin1Code ( string admin1_code ) { _admin1_code = admin1_code; }
    void setAdmin1Name ( string admin1_name ) { _admin1_name = admin1_name; }
    void setTimezoneName ( string timezone_name ) { _timezone_name = timezone_name; }
    void setElevation ( float elevation ) { _elevation = elevation; }
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

typedef map<string,int> SSPlanetFeatureMap;

int SSMakePlanetFeatureMap ( SSObjectVec &features, SSPlanetFeatureMap &map );

#endif /* SSFeature_hpp */
