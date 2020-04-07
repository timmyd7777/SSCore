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

func test ( ) -> String
{
    let ctime = CSSTimeFromSystem()
    let cdate = CSSTimeToCSSDate ( ctime )

    var str = "Hello, SSCore!\n"

    str.append ( String ( format: "Current local date is %04d/%02hd/%02.0f\n", cdate.year, cdate.month, floor ( cdate.day ) ) )
    str.append ( String ( format: "Current local time is %02hd:%0h2d:%04.1f\n", cdate.hour, cdate.min, cdate.sec ) )
    str.append ( String ( format: "Local time zone is %+.2f hours east of UTC\n", cdate.zone ) )
    str.append ( String ( format: "Julian Date is %.6f\n", ctime.jd ) )

    str.append ( String ( format: "pi = %.6f\n", kSSPi ) );
    str.append ( String ( format: "1 radian = %.6f deg\n", kSSDegPerRad ) );
    
    let cdms = CSSDegMinSecFromRadians ( 1.0 );
    let dmsstr = String ( cString: CSSDegMinSecToString ( cdms ) )
    str.append ( String ( format: "1 radian = %s deg min sec\n", ( dmsstr as NSString ).utf8String ?? "" ) )
    
    let hmsstr = "12 34 56.7"
    let chms = CSSHourMinSecFromString ( hmsstr );
    str.append ( String ( format: "%@ hour min sec = %.6f radian\n", hmsstr, CSSHourMinSecToRadians ( chms ) ) )

    var cmat = CSSMatrixIdentity();
    cmat = CSSMatrixRotate ( cmat, 1, 1.0 );
    cmat = CSSMatrixRotate ( cmat, 2, 2.0 );
    cmat = CSSMatrixRotate ( cmat, 3, 3.0 );

    str.append ( "Rotation Matrix:\n" )
    str.append ( CSSMatrixToString ( mat: cmat ) )
    
    str.append ( "Inverse Matrix:\n" )
    let cinv = CSSMatrixInverse ( cmat );
    str.append ( CSSMatrixToString ( mat: cinv ) )
    
    str.append ( "Identity Matrix:\n" )
    let cidm = CSSMatrixMultiplyMatrix ( cinv, cmat );
    str.append ( CSSMatrixToString ( mat: cidm ) )

    return str
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
