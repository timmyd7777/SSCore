package com.southernstars.sstest

import androidx.appcompat.app.AppCompatActivity
import android.os.Bundle
import android.content.res.AssetManager
import kotlinx.android.synthetic.main.activity_main.*
import kotlin.math.*
import com.southernstars.sscore.*

fun JSSVectorToString ( vec: JSSVector ): String
{
    return "%+f %+f %+f\n".format ( vec.x, vec.y, vec.z )
}

fun JSSMatrixToString ( mat: JSSMatrix ): String
{
    var str = ""

    str += "%+f %+f %+f\n".format ( mat.m00, mat.m01, mat.m02 )
    str += "%+f %+f %+f\n".format ( mat.m10, mat.m11, mat.m12 )
    str += "%+f %+f %+f\n".format ( mat.m20, mat.m21, mat.m22 )

    return str
}

class MainActivity : AppCompatActivity() {

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)

        // Example of a call to a native method

        var str = stringFromJNI() + "\n"

        // needed to read files from assets from native code

        val assetMgr = getResources().assets
        if ( initAssetManager ( assetMgr ) )
            str += "Successfully initialized asset manager.\n"
        else
            str += "Failed to initialize asset manager!\n"

        // Get current time from system as Julian date, and convert fo calendar date
        // Get JED and Greenwich Sidereal Time

        val time = JSSTime.fromSystem()
        val date = JSSDate.fromJulianDate ( time, JSSDate.kGregorian )
        val jed = time.getJulianEphemerisDate()
        val gst = time.getSiderealTime ( 0.0 )
        var hms = JSSHourMinSec.fromRadians ( gst )

        // Print local date, time, time zone, JD, JED, GST

        str += "Current local date is %02d/%02d/%02d\n".format ( date.year, date.month, date.day )
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

        // Do some matrix operations which should generate an identity matrix at the end

        var mat = JSSMatrix.identity()

        mat = mat.rotate (0, 1.0 )
        mat = mat.rotate (1, 2.0 )
        mat = mat.rotate (2, 3.0 )

        val inv = mat.inverse()
        val prd = inv.multiply ( mat )

        str += "Identity Matrix:\n"
        str += JSSMatrixToString ( prd )

        // Create a coordinate transformation object initialized to the longitude and latitude of San Francisco at current time
        // Wouldn't it be great to have a common API that gets current location on all platforms? (hint, hint)

        val lon = JSSDegMinSec.fromString ( "-122 25 09.9" ).toRadians()
        val lat = JSSDegMinSec.fromString ( "+37 46 29.7" ).toRadians()
        val location = JSSSpherical ( lon, lat, 0.026 )
        val coords = JSSCoordinates ( time, location )

        str += "Test lon:%+.3f° lat:%+.3f° alt:%.3f km\n".format (
            coords.getLocation().lon * JSSAngle.kDegPerRad,
            coords.getLocation().lat * JSSAngle.kDegPerRad,
            coords.getLocation().rad )

        // Now compute some coordinate-related values.

        val dl = JSSDouble ( 0.0 )
        val de = JSSDouble ( 0.0 )

        val obq = JSSCoordinates.getObliquity ( time.jd )
        JSSCoordinates.getNutationConstants ( time.jd, de, dl )

        str += "Ecliptic obq: %.6f°\n".format ( obq * JSSAngle.kDegPerRad )
        str += "Nutation lon: %+.1f\" obq: %+.1f\"\n". format ( dl.value * JSSAngle.kArcsecPerRad, de.value * JSSAngle.kArcsecPerRad )

        val zeta = JSSDouble ( 0.0 )
        val z = JSSDouble ( 0.0 )
        val theta = JSSDouble ( 0.0 )

        JSSCoordinates.getPrecessionConstants ( time.jd, zeta, z, theta )
        str += "Precession zeta:%+.1f\" z:%+.1f\" theta:%+.1f\"\n".format ( zeta.value * JSSAngle.kArcsecPerRad, z.value * JSSAngle.kArcsecPerRad, theta.value * JSSAngle.kArcsecPerRad )

        val lst = coords.getLST()
        str += "Local Sidereal Time: %s\n".format ( JSSHourMinSec.fromRadians ( lst ).toString() )

        val geo = JSSCoordinates.toGeocentric ( location, JSSCoordinates.kKmPerEarthRadii, JSSCoordinates.kEarthFlattening )
        str += "Geo X:%+.3f Y:%+.3f Z:%+.3f km\n".format ( geo.x, geo.y, geo.z )
        val loc = JSSCoordinates.toGeodetic ( geo, JSSCoordinates.kKmPerEarthRadii, JSSCoordinates.kEarthFlattening )
        str += "Geo lon:%+.3f° lat:%+.3f° alt:%.3f km\n".format (loc.lon * JSSAngle.kDegPerRad, loc.lat * JSSAngle.kDegPerRad, loc.rad )

        // Set horizon coordinates to due north; then do some coordinate transformations to equatorial, ecliptic, galactic frames.

        val hor = JSSSpherical ( 0.0, 0.0, 1.0 )
        val equ = coords.transform ( JSSCoordinates.kHorizon, JSSCoordinates.kEquatorial, hor )
        val ecl = coords.transform ( JSSCoordinates.kHorizon, JSSCoordinates.kEcliptic, hor )
        val gal = coords.transform ( JSSCoordinates.kHorizon, JSSCoordinates.kGalactic, hor )

        str += "Horizon Az:%.3f° Alt:%+.3f°\n".format ( hor.lon * JSSAngle.kDegPerRad, hor.lat * JSSAngle.kDegPerRad )
        str += "Equatorial RA:%s Dec:%s\n".format ( JSSHourMinSec.fromRadians ( equ.lon ).toString(), JSSDegMinSec.fromRadians ( equ.lat ).toString() )
        str += "Ecliptic lon:%.3f° lat:%+.3f°\n".format ( ecl.lon * JSSAngle.kDegPerRad, ecl.lat * JSSAngle.kDegPerRad )
        str += "Galactic lon:%.3f° lat:%+.3f°\n".format ( gal.lon * JSSAngle.kDegPerRad, gal.lat * JSSAngle.kDegPerRad )

        // Open JPL ephemeris file.  if successful, compute Earth's current position and velocity.

        val path = "SSData/SolarSystem/DE438/1950_2050.438"
        if ( JSSJPLDEphemeris.open ( path ) )
        {
            str += "Opened DE438 ephemeris file.\n"
            val jed0 = JSSJPLDEphemeris.getStartJED()
            val jed1 = JSSJPLDEphemeris.getStopJED()
            str += "JED %.1f to %.1f\n".format ( jed0, jed1 )
            val nconst = JSSJPLDEphemeris.getConstantNumber()
            val cname0 = JSSJPLDEphemeris.getConstantName ( 0 )
            val cval0 = JSSJPLDEphemeris.getConstantValue ( 0 )
            str += "%d constants; %s=%f\n".format ( nconst, cname0, cval0 )
            val pos = JSSVector ( 0.0, 0.0, 0.0 )
            val vel = JSSVector ( 0.0, 0.0, 0.0 )
            JSSJPLDEphemeris.compute ( 3, jed, true, pos, vel )
            str += "Earth position: " + JSSVectorToString ( pos )
            str += "Earth velocity: " + JSSVectorToString ( vel )
            JSSJPLDEphemeris.close()
        }
        else
        {
            str += "Failed to open DE438 ephemeris file.\n"
        }

        // Open and read bright stars csv data file into object array in memory.
        // Display number of objects imported; then release object array memory.

        val stars = JSSObjectArray()
        val nS = stars.importFromCSV ( "SSData/Stars/Brightest.csv" )
        str += "Imported %d bright stars.\n".format ( nS )
        stars.destroy()

        // Open and Messier and Caldwell csv data files into object array in memory.
        // Display number of objects imported; then release object array memory.

        val deepsky = JSSObjectArray()
        val nM = deepsky.importFromCSV ( "SSData/DeepSky/Messier.csv" )
        val nC = deepsky.importFromCSV ( "SSData/DeepSky/Caldwell.csv" )
        val nMC = deepsky.size()
        str += "Imported %d Messier & %d Caldwell objects.\n".format ( nM, nC )
        str += "Imported %d total deep sky objects.\n".format ( nMC )
        deepsky.destroy()

        // Read planetary data file

        val planets = JSSObjectArray()
        val nP = planets.importFromCSV ( "SSData/SolarSystem/Planets.csv" )
        str += "Imported %d planets: ".format ( nP )

        // Print names of all objects in the array, then destroy.
        // Deliberately overrun the array to test handling this error condition.

        for ( i in 0 .. nP )
        {
            val planet = planets.getObject ( i )
            str += planet?.getName ( 0 ) ?: "null 11th successfully caught."
            val ident = planet?.getIdentifier ( JSSIdentifier.kCatUnknown ) ?: JSSIdentifier ( JSSIdentifier.kCatUnknown, 0 )
 //           if ( ident.equals ( JSSIdentifier ( JSSIdentifier.kCatJPLanet, 3 ) ) )
 //               str += "I found the Earth!\n"
            str += if ( i < nP ) ", " else "\n"
        }

        planets.destroy()

        // Test some JSSEvent functions.

        var sda = JSSEvent.semiDiurnalArc ( lat, sirius.lat, JSSEvent.kDefaultRiseSetAlt )
        hms = JSSHourMinSec.fromRadians ( sda )

        str += "SDA of Sirius: " + hms.toString() + "\n"

        var risetime = JSSEvent.riseTransitSet ( time, sirius.lon, sirius.lat, JSSEvent.kRise, lon, lat, JSSEvent.kDefaultRiseSetAlt )
        var risedate = JSSDate.fromJulianDate ( risetime, JSSDate.kGregorian )

        str += "Sirius rises at %02d:%02d:%02.0f\n".format ( risedate.hour, risedate.min, risedate.sec )

        val nearest = JSSObjectArray()
        val numS = nearest.importFromCSV ( "SSData/Stars/Nearest.csv" )
        val siriusA = nearest.getObject ( 6 )

        var pass = JSSEvent.riseTransitSet ( time, coords, siriusA, JSSEvent.kDefaultRiseSetAlt )
        var risedate2 = JSSDate.fromJulianDate ( pass.rising.time, JSSDate.kGregorian )

        str += "Sirius also rises at %02d:%02d:%02.0f\n".format ( risedate2.hour, risedate2.min, risedate2.sec )
        nearest.destroy()

        // Solar system import code lifted & translated from TestEvents in SSTest.cpp

        val solsys = JSSObjectArray()

        val numPlanets = solsys.importFromCSV( "SSData/SolarSystem/Planets.csv" )
        str += "Imported %02d planets.\n".format ( numPlanets )

        val numMoons = solsys.importFromCSV( "SSData/SolarSystem/Moons.csv" )
        str += "Imported %02d moons.\n".format ( numMoons )

        str += "Total solar system objects imported:  %02d.\n".format ( solsys.size() )

        val numSats = solsys.importFromTLE( "SSData/SolarSystem/Satellites/visual.txt" )
        str += "Imported %02d artificial satellites.\n".format ( numSats )

        val numMcNames = solsys.importMcNames( "SSData/SolarSystem/Satellites/mcnames.txt" )
        str += "Imported %02d McCants satellite names.\n".format ( numMcNames )

        // JSSEvent.nextMoonPhase

        val now = JSSTime.fromSystem()
        val date_now = JSSDate.fromJulianDate ( now, 0 ) // kGregorian = 0

        val sun = solsys.getObject ( 0 )
        val moon = solsys.getObject ( 10 )

        val new_moon_time = JSSEvent.nextMoonPhase ( now, sun, moon, JSSEvent.kNewMoon )
        val nmd = JSSDate.fromJulianDate( new_moon_time, 0 )

        str += "New Moon:       %02d/%02d/%02d %02d:%02d:%02.0f\n".format ( nmd.year, nmd.month, nmd.day, nmd.hour, nmd.min, nmd.sec )
        
        // Find Jupiter-Saturn conjunctions in the next year
        
        val jupiter = solsys.getObject ( 5 )
        val saturn = solsys.getObject ( 6 )

        val conjunctions: ArrayList<JSSEventTime> = ArrayList()
        JSSEvent.findConjunctions ( coords, jupiter, saturn, now, JSSTime( now.jd + 365.25, now.zone ), conjunctions, 10 )

        val conj_size = conjunctions.size
        str += "%02d Jupiter-Saturn Conjunctions in the next year:\n".format ( conj_size )

        for ( i in 0 .. ( conj_size - 1) )
        {
            val conj_date = JSSDate.fromJulianDate ( conjunctions.get( i ).time, 0 )
            val sep = JSSDegMinSec.fromRadians ( conjunctions.get( i ).value )
            str += "%2d° %2d' %4.1f\"".format( sep.deg, sep.min, sep.sec ) + " on %02d/%02d/%02d %02d:%02d:%02.0f\n".format ( conj_date.year, conj_date.month, conj_date.day, conj_date.hour, conj_date.min, conj_date.sec )
        }

        // findOppositions

        val oppositions: ArrayList<JSSEventTime> = ArrayList()
        JSSEvent.findOppositions ( coords, jupiter, saturn, now, JSSTime( now.jd + 365.25, now.zone ), oppositions, 10 )

        val opp_size = oppositions.size
        str += "%02d Jupiter-Saturn Oppositions in the next year:\n".format ( opp_size )

        // findNearestDistances

        val nearestDistances: ArrayList<JSSEventTime> = ArrayList()
        JSSEvent.findNearestDistances ( coords, jupiter, saturn, now, JSSTime( now.jd + 365.25, now.zone ), nearestDistances, 10 )

        val nd_size = nearestDistances.size
        str += "%02d Jupiter-Saturn Nearest Distances in the next year:\n".format ( nd_size )

        // findFarthestDistances

        val farthesttDistances: ArrayList<JSSEventTime> = ArrayList()
        JSSEvent.findFarthestDistances ( coords, jupiter, saturn, now, JSSTime( now.jd + 365.25, now.zone ), farthesttDistances, 10 )

        val fd_size = farthesttDistances.size
        str += "%02d Jupiter-Saturn Farthest Distances in the next year:\n".format ( fd_size )

        for ( i in 0 .. ( solsys.size() - 1) )
        {
            val jobject = solsys.getObject ( i )
            val ident = jobject?.getIdentifier ( JSSIdentifier.kCatNORADSat ) ?: JSSIdentifier ( JSSIdentifier.kCatUnknown, 0 )

            if ( ident.equals ( JSSIdentifier ( JSSIdentifier.kCatNORADSat, 25544 ) ) )
            {
                coords.setTime ( now );
                val passes: ArrayList<JSSPass> = ArrayList();
            
                val numpasses = JSSEvent.findSatellitePasses ( coords, jobject, now, JSSTime( now.jd + 1.0, now.zone ), 0.0, passes, 10 );
                str += "%02d ISS passes in the next day:\n".format ( numpasses )
                for ( i in 0 .. ( numpasses - 1) )
                {
                    val pass = passes.get( i )
                    var date = JSSDate.fromJulianDate ( pass.rising.time, 0 )
                    str += "Rise:  %02d:%02d:%02.0f @ %.1f°\n".format( date.hour, date.min, date.sec, pass.rising.azm * JSSAngle.kDegPerRad )

                    date = JSSDate.fromJulianDate ( pass.transit.time, 0 )
                    str += "Peak:  %02d:%02d:%02.0f @ %.1f°\n".format( date.hour, date.min, date.sec, pass.transit.alt * JSSAngle.kDegPerRad )

                    date = JSSDate.fromJulianDate ( pass.setting.time, 0 )
                    str += "Set:   %02d:%02d:%02.0f @ %.1f°\n".format( date.hour, date.min, date.sec, pass.setting.azm * JSSAngle.kDegPerRad )
                }

                break
            }
        }

        sample_text.text = str

        solsys.destroy()
    }

    external fun initAssetManager ( mgr:AssetManager ): Boolean

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
