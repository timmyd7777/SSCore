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
    let hms = CSSHourMinSecFromRadians ( gst )
    
    // Print local date, time, time zone, JD, JED, GST
    
    str.append ( String ( format: "Current local date is %04d/%02hd/%02.0f\n", cdate.year, cdate.month, floor ( cdate.day ) ) )
    str.append ( String ( format: "Current local time is %02hd:%0h2d:%04.1f\n", cdate.hour, cdate.min, cdate.sec ) )
    str.append ( String ( format: "Local time zone is %+.2f hours east of UTC\n", cdate.zone ) )
    str.append ( String ( format: "Julian Date is %.12f\n", ctime.jd ) )
    str.append ( String ( format: "Julian Ephemeris Date is %.6f\n", jed ) )
    str.append ( String ( format: "Greenwich Sidereal Time is %s\n", CSSHourMinSecToString ( hms ) ) )

    // Print some angular constants and do some angle conversions
    
    str.append ( String ( format: "pi = %.6f\n", kSSPi ) );
    str.append ( String ( format: "1 radian = %.12f deg\n", kSSDegPerRad ) );
    
    let cdms = CSSDegMinSecFromRadians ( 1.0 );
    let dmsstr = String ( cString: CSSDegMinSecToString ( cdms ) )
    str.append ( String ( format: "1 radian = %s deg min sec\n", ( dmsstr as NSString ).utf8String ?? "" ) )
    
    let hmsstr = "12 34 56.7"
    let chms = CSSHourMinSecFromString ( hmsstr );
    str.append ( String ( format: "%@ hour min sec = %.6f radian\n", hmsstr, CSSHourMinSecToRadians ( chms ) ) )

    // Do some matrix operations which should generate an identity matrix at the end
    
    var cmat = CSSMatrixIdentity();
    
    cmat = CSSMatrixRotate ( cmat, 1, 1.0 );
    cmat = CSSMatrixRotate ( cmat, 2, 2.0 );
    cmat = CSSMatrixRotate ( cmat, 3, 3.0 );

    let cinv = CSSMatrixInverse ( cmat );
    let cidm = CSSMatrixMultiplyMatrix ( cinv, cmat );

    str.append ( "Identity Matrix:\n" )
    str.append ( CSSMatrixToString ( mat: cidm ) )

    // get path to DE438 file within SSData folder within main bundle
    // open DE438.  If succesful, compute and display Earth's barycentric position & velocity
    
    var path = Bundle.main.bundlePath.appending ( "/SSData/SolarSystem/DE438/1950_2050.438" )
    if CSSJPLDEphemerisOpen ( ( path as NSString ).utf8String )
    {
        str.append ( "Opened DE438 ephemeris file.\n" )
        var pos = CSSVectorFromXYZ ( 0.0, 0.0, 0.0 )
        var vel = pos
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

    // Open and read bright stars csv data file into object array in memory.
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
    
    for i in 0..<n
    {
        let pObj = CSSObjectGetFromArray ( pObjArr, i )
        str.append ( String ( cString: CSSObjectGetName ( pObj, 0 ) ) )
        if i < n - 1 {
            str.append ( ", " )
        } else {
            str.append ( "\n" )
        }
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
