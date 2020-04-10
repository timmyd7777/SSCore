package com.southernstars.sstest

import androidx.appcompat.app.AppCompatActivity
import android.os.Bundle
import kotlinx.android.synthetic.main.activity_main.*
import kotlin.math.*
import com.southernstars.sscore.*

class MainActivity : AppCompatActivity() {

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)

        // Example of a call to a native method

        var str = stringFromJNI() + "\n"

        // Get current time from system as Julian date, and convert fo calendar date
        // Get JED and Greenwich Sidereal Time

        val time = JSSTime.fromSystem()
        val date = JSSDate.fromJulianDate ( time )
        val jed = time.getJulianEphemerisDate()
        val gst = time.getSiderealTime ( 0.0 )
        var hms = JSSHourMinSec.fromRadians ( gst )

        // Print local date, time, time zone, JD, JED, GST

        str += "Current local date is %02d/%02d/%02.0f\n".format ( date.year, date.month, floor ( date.day ) )
        str += "Current local time is %02d:%02d:%04.1f\n".format ( date.hour, date.min, date.sec )
        str += "Local time zone is %+.2f hours east of UTC\n".format ( date.zone )
        str += "Julian Date is %.6f\n".format ( time.jd )
        str += "Julian Ephemeris Date is %.6f\n".format ( jed )
        str += "Greenwich Sidereal Time is %s\n".format ( hms.toString() )

        // Print some angular constants and do some angle conversions

        str += "pi = %.12f\n".format ( JSSAngle.kPi )
        str += "1 radian = %.12f deg\n".format ( JSSAngle.kDegPerRad )

        var dms = JSSDegMinSec.fromRadians ( 1.0 )
        str += "1 radian = %s deg min sec\n".format ( dms.toString() )

        val hmsstr = "12 34 56.7"
        hms = JSSHourMinSec.fromString ( hmsstr )
        str += "%s hour min sec = %.6f radian\n".format ( hmsstr, hms.toRadians() )

        // Compute some angular separations and position angles using spherical and rectangular coordinates

        hms = JSSHourMinSec.fromString ( "06 45 08.92" )
        dms = JSSDegMinSec.fromString ( "-16 42 58.0" )
        val sirius = JSSSpherical ( hms.toRadians(), dms.toRadians(), 2.637 )
        val siriusXYZ = sirius.toVector().normalize()

        hms = JSSHourMinSec.fromString ( "07 39 18.12" )
        dms = JSSDegMinSec.fromString ( "+05 13 30.0" )
        val procyon = JSSSpherical ( hms.toRadians(), dms.toRadians(), 3.497 )
        val procyonXYZ = procyon.toVector().normalize()

        dms = JSSDegMinSec.fromRadians ( sirius.angularSeparation ( procyon ) )
        var pa = sirius.positionAngle ( procyon )
        val d = siriusXYZ.distance ( procyonXYZ )
        str += "Sirius to Procyon (sph): %02d° %02d' %04.1f\" @ %.3f°, %.3f pc\n".format ( dms.deg, dms.min, dms.sec, JSSAngle ( pa ).toDegrees(), d )

        dms = JSSDegMinSec.fromRadians ( siriusXYZ.angularSeparation ( procyonXYZ ) )
        pa = siriusXYZ.positionAngle ( procyonXYZ )
        str += "Sirius to Procyon (vec): %02d° %02d' %04.1f\" @ %.3f°, %.3f pc\n".format ( dms.deg, dms.min, dms.sec, JSSAngle ( pa ).toDegrees(), d )

        sample_text.text = str
    }

    /**
     * A native method that is implemented by the 'sscore-lib' native library,
     * which is packaged with this application.
     */
    external fun stringFromJNI(): String

    companion object {

        // Used to load the 'sscore-lib' library on application startup.
        init {
            System.loadLibrary("sscore-lib")
        }
    }
}
