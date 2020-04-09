package com.southernstars.sstest

import androidx.appcompat.app.AppCompatActivity
import android.os.Bundle
import kotlinx.android.synthetic.main.activity_main.*

import com.southernstars.sscore.*

class MainActivity : AppCompatActivity() {

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)

        // Example of a call to a native method

        var str = stringFromJNI() + "\n"

        var now = JSSTime.fromSystem()
        var jed = now.getJulianEphemerisDate()

        str += "Current Julian Date is " + now.jd + "\n"
        str += "Current Julian Ephemeris Date is " + jed + "\n"

        sample_text.text = str
    }

    /**
     * A native method that is implemented by the 'native-lib' native library,
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
