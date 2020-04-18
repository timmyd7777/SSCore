//
//  ContentView.swift
//  SSTest
//
//  Created by Tim DeBenedictis on 4/5/20.
//  Copyright © 2020 Southern Stars. All rights reserved.
//

import SwiftUI

func CSSMatrixToString ( mat: CSSMatrix ) -> String
{
    var str = ""
    
    str.append ( String ( format: "%+f %+f %+f\n", mat.m00, mat.m01, mat.m02 ) );
    str.append ( String ( format: "%+f %+f %+f\n", mat.m10, mat.m11, mat.m12 ) );
    str.append ( String ( format: "%+f %+f %+f\n", mat.m20, mat.m21, mat.m22 ) );

    return str
}

func CSSVectorToString ( vec: CSSVector ) -> String
{
    return String ( format: "%+f %+f %+f\n", vec.x, vec.y, vec.z )
}

func test ( ) -> String
{
    var str = "Hello, SSCore!\n"

    // Get current time from system as Julian date, and convert fo calendar date
    // Get JED and Greenwich Sidereal Time
    
    let ctime = CSSTimeFromSystem()
    let cdate = CSSTimeToCSSDate ( ctime )
    let jed = CSSTimeGetJulianEphemerisDate ( ctime );
    let gst = CSSTimeGetSiderealTime ( ctime, 0.0 );
    var hms = CSSHourMinSecFromRadians ( gst )
    
    // Print local date, time, time zone, JD, JED, GST
    
    str.append ( String ( format: "Current local date is %04d/%02hd/%02.0f\n", cdate.year, cdate.month, floor ( cdate.day ) ) )
    str.append ( String ( format: "Current local time is %02hd:%0h2d:%04.1f\n", cdate.hour, cdate.min, cdate.sec ) )
    str.append ( String ( format: "Local time zone is %+.2f hours east of UTC\n", cdate.zone ) )
    str.append ( String ( format: "Julian Date is %.12f\n", ctime.jd ) )
    str.append ( String ( format: "Julian Ephemeris Date is %.6f\n", jed ) )
    str.append ( String ( format: "Greenwich Sidereal Time is %s\n", CSSHourMinSecToString ( hms ) ) )

    // Print some angular constants and do some angle conversions
    
    str.append ( String ( format: "pi = %.6f\n", kSSPi ) )
    str.append ( String ( format: "1 radian = %.12f deg\n", kSSDegPerRad ) )
    
    var dms = CSSDegMinSecFromRadians ( 1.0 )
    let dmsstr = String ( cString: CSSDegMinSecToString ( dms ) )
    str.append ( String ( format: "1 radian = %s deg min sec\n", ( dmsstr as NSString ).utf8String ?? "" ) )
    
    let hmsstr = "12 34 56.7"
    let chms = CSSHourMinSecFromString ( hmsstr );
    str.append ( String ( format: "%@ hour min sec = %.6f radian\n", hmsstr, CSSHourMinSecToRadians ( chms ) ) )

    // Compute some angular separations and position angles using spherical and rectangular coordinates

    hms = CSSHourMinSecFromString ( "06 45 08.92" )
    dms = CSSDegMinSecFromString ( "-16 42 58.0" )
    let sirius = CSSSpherical ( lon:CSSHourMinSecToRadians ( hms ), lat:CSSDegMinSecToRadians ( dms ), rad:2.637 )
    let siriusXYZ = CSSVectorNormalize ( CSSSphericalToCSSVector ( sirius ) )

    hms = CSSHourMinSecFromString ( "07 39 18.12" )
    dms = CSSDegMinSecFromString ( "+05 13 30.0" )
    let procyon = CSSSpherical ( lon:CSSHourMinSecToRadians ( hms ), lat:CSSDegMinSecToRadians ( dms ), rad:3.497 )
    let procyonXYZ = CSSVectorNormalize ( CSSSphericalToCSSVector ( procyon ) )

    dms = CSSDegMinSecFromRadians ( CSSSphericalAngularSeparation ( sirius, procyon ) )
    var pa = CSSSphericalPositionAngle ( sirius, procyon )
    let d = CSSVectorDistance ( siriusXYZ, procyonXYZ )
    str.append ( String ( format:"Sirius to Procyon (sph): %02d° %02d' %04.1f\" @ %.3f°, %.3f pc\n", dms.deg, dms.min, dms.sec, pa * kSSDegPerRad, d ) )

    dms = CSSDegMinSecFromRadians ( CSSVectorAngularSeparation ( siriusXYZ, procyonXYZ ) )
    pa = CSSVectorPositionAngle ( siriusXYZ, procyonXYZ )
    str.append ( String ( format:"Sirius to Procyon (vec): %02d° %02d' %04.1f\" @ %.3f°, %.3f pc\n", dms.deg, dms.min, dms.sec, pa * kSSDegPerRad, d ) )

    // Do some matrix operations which should generate an identity matrix at the end
    
    var cmat = CSSMatrixIdentity();
    
    cmat = CSSMatrixRotate ( cmat, 0, 1.0 );
    cmat = CSSMatrixRotate ( cmat, 1, 2.0 );
    cmat = CSSMatrixRotate ( cmat, 2, 3.0 );

    let cinv = CSSMatrixInverse ( cmat );
    let cidm = CSSMatrixMultiplyMatrix ( cinv, cmat );

    str.append ( "Identity Matrix:\n" )
    str.append ( CSSMatrixToString ( mat: cidm ) )

    // Create a coordinate transformation object initialized to the longitude and latitude of San Francisco at current time
    // Wouldn't it be great to have a common API that gets current location on all platforms? (hint, hint)
    
    let lon = CSSDegMinSecToRadians ( CSSDegMinSecFromString ( "-122 25 09.9" ) )
    let lat = CSSDegMinSecToRadians ( CSSDegMinSecFromString ( "+37 46 29.7" ) )
    let location = CSSSphericalFromLonLatRad ( lon, lat, 0.026 )
    let coords = CSSCoordinatesCreate ( ctime, location )
    
    str.append ( String ( format:"Test lon:%+.3f° lat:%+.3f° alt:%.3f km\n",
        CSSCoordinatesGetLocation ( coords ).lon * kSSDegPerRad,
        CSSCoordinatesGetLocation ( coords ).lat * kSSDegPerRad,
        CSSCoordinatesGetLocation ( coords ).rad ) )

    // Now compute some coordinate-related values.

    var dl:Double = 0.0
    var de:Double = 0.0
    
    let obq = CSSCoordinatesGetObliquity ( ctime.jd )
    CSSCoordinatesGetNutationConstants ( ctime.jd, &de, &dl )
    
    str.append ( String ( format:"Ecliptic obq: %.6f°\n", obq * kSSDegPerRad ) )
    str.append ( String ( format:"Nutation lon: %+.1f\" obq: %+.1f\"\n", dl * kSSArcsecPerRad, de * kSSArcsecPerRad ) )
    
    var zeta:Double = 0.0
    var z:Double = 0.0
    var theta:Double = 0.0

    CSSCoordinatesGetPrecessionConstants ( ctime.jd, &zeta, &z, &theta )
    str.append ( String ( format:"Precession zeta:%+.1f\" z:%+.1f\" theta:%+.1f\"\n", zeta * kSSArcsecPerRad, z * kSSArcsecPerRad, theta * kSSArcsecPerRad ) )

    let lst = CSSCoordinatesGetLST ( coords );
    str.append ( String ( format:"Local Sidereal Time: %s\n", CSSHourMinSecToString ( CSSHourMinSecFromRadians ( lst ) ) ) )
    
    let geo = CSSCoordinatesToGeocentric ( location, kCSSKmPerEarthRadii, kCSSEarthFlattening )
    str.append ( String ( format:"Geo X:%+.3f Y:%+.3f Z:%+.3f km\n", geo.x, geo.y, geo.z ) )
    let loc = CSSCoordinatesToGeodetic ( geo, kCSSKmPerEarthRadii, kCSSEarthFlattening )
    str.append ( String ( format:"Geo lon:%+.3f° lat:%+.3f° alt:%.3f km\n",
        loc.lon * kSSDegPerRad, loc.lat * kSSDegPerRad, loc.rad ) )
    
    // Set horizon coordinates to due north; then do some coordinate transformations to equatorial, ecliptic, galactic frames.
    
    let hor = CSSSphericalFromLonLatRad ( 0.0, 0.0, 1.0 );
    let equ = CSSCoordinatesTransformSpherical ( coords, kCSSHorizon, kCSSEquatorial, hor );
    let ecl = CSSCoordinatesTransformSpherical ( coords, kCSSHorizon, kCSSEcliptic, hor );
    let gal = CSSCoordinatesTransformSpherical ( coords, kCSSHorizon, kCSSGalactic, hor );
    
    str.append ( String ( format:"Horizon Az:%.3f° Alt:%+.3f°\n", hor.lon * kSSDegPerRad, hor.lat * kSSDegPerRad ) )
    str.append ( String ( format:"Equatorial RA:%s Dec:%s\n",
        CSSHourMinSecToString ( CSSHourMinSecFromRadians ( equ.lon ) ),
        CSSDegMinSecToString ( CSSDegMinSecFromRadians ( equ.lat ) ) ) )
    str.append ( String ( format:"Ecliptic lon:%.3f° lat:%+.3f°\n", ecl.lon * kSSDegPerRad, ecl.lat * kSSDegPerRad ) )
    str.append ( String ( format:"Galactic lon:%.3f° lat:%+.3f°\n", gal.lon * kSSDegPerRad, gal.lat * kSSDegPerRad ) )

    // Finally destroy coordinate transformations object
    
    CSSCoordinatesDestroy ( coords );
    
    // get path to DE438 file within SSData folder within main bundle
    // open DE438.  If succesful, compute and display Earth's barycentric position & velocity
    
    var path = Bundle.main.bundlePath.appending ( "/SSData/SolarSystem/DE438/1950_2050.438" )
    if CSSJPLDEphemerisOpen ( ( path as NSString ).utf8String )
    {
        str.append ( "Opened DE438 ephemeris file.\n" )
        let jed0 = CSSJPLDEphemerisGetStartJED()
        let jed1 = CSSJPLDEphemerisGetStopJED()
        str.append ( String ( format:"JED %.1f to %.1f\n", jed0, jed1 ) )
        let cnum = CSSJPLDEphemerisGetConstantCount()
        let cname0 = String ( cString:CSSJPLDEphemerisGetConstantName ( 0 ) )
        let cval0 = CSSJPLDEphemerisGetConstantValue ( 0 );
        str.append ( String ( format:"%d constants; %@=%f\n", cnum, cname0, cval0 ) )
        var pos = CSSVectorFromXYZ ( 0.0, 0.0, 0.0 )
        var vel = CSSVectorFromXYZ ( 0.0, 0.0, 0.0 )
        CSSJPLDEphemerisCompute ( 3, jed, true, &pos, &vel );
        str.append ( "Earth position: " )
        str.append ( CSSVectorToString ( vec: pos ) )
        str.append ( "Earth velocity: " )
        str.append ( CSSVectorToString ( vec: vel ) )
        CSSJPLDEphemerisClose();
    }
    else
    {
        str.append ( "Failed to open DE438 ephemeris file.\n" )
    }
    
    // Open and read bright stars csv data file into object array in memory.
    // Display number of objects imported; then release object array memory.
    
    path = Bundle.main.bundlePath.appending ( "/SSData/Stars/Brightest.csv" )
    var pObjArr = CSSObjectArrayCreate();
    var n = CSSImportObjectsFromCSV ( ( path as NSString ).utf8String, pObjArr )
    str.append ( String ( format: "Imported %d bright stars.\n", n ) )
    CSSObjectArrayDestroy ( pObjArr )

    // Open and Messier and Caldwell csv data files into object array in memory.
    // Display number of objects imported; then release object array memory.
    
    pObjArr = CSSObjectArrayCreate()
    path = Bundle.main.bundlePath.appending ( "/SSData/DeepSky/Messier.csv" )
    let nM = CSSImportObjectsFromCSV ( ( path as NSString ).utf8String, pObjArr )
    path = Bundle.main.bundlePath.appending ( "/SSData/DeepSky/Caldwell.csv" )
    let nC = CSSImportObjectsFromCSV ( ( path as NSString ).utf8String, pObjArr )
    let nMC = CSSObjectArraySize ( pObjArr );
    str.append ( String ( format: "Imported %d Messier & %d Caldwell objects.\n", nM, nC ) )
    str.append ( String ( format: "Imported %d total deep sky objects.\n", nMC ) )
    CSSObjectArrayDestroy ( pObjArr )

    // Open and read Planets.csv data file into object array in memory.
    
    path = Bundle.main.bundlePath.appending ( "/SSData/SolarSystem/Planets.csv" )
    pObjArr = CSSObjectArrayCreate();
    n = CSSImportObjectsFromCSV ( ( path as NSString ).utf8String, pObjArr );
    str.append ( String ( format: "Imported %d planets:\n", n ) );

    // Print names of all objects in the array, then destroy.
    // Deliberately overrun the array to test handling this error condition.

    for i in 0...n
    {
        let pObj = CSSObjectGetFromArray ( pObjArr, i )
        str.append ( pObj == nil ? "null 11th successfully caught" : String ( cString: CSSObjectGetName ( pObj, 0 ) ) )
        str.append ( i < n ? ", " : "\n" )
    }
    
    CSSObjectArrayDestroy ( pObjArr );
    
    return str;
}

struct ContentView: View {
    var body: some View {
        Text ( test() )
    }
}

struct ContentView_Previews: PreviewProvider {
    static var previews: some View {
        ContentView()
    }
}
