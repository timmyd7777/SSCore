//
//  ContentView.swift
//  SSTest
//
//  Created by Tim DeBenedictis on 4/5/20.
//  Copyright © 2020 Southern Stars. All rights reserved.
//

import SwiftUI

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
    str.append ( String ( format: "%@ hour min sec = %.6f radian", hmsstr, CSSHourMinSecToRadians ( chms ) ) )

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
