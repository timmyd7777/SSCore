// MountSimualtor.cpp
//
// Created by Tim DeBenedictis on 9/15/22.
// Copyright © 2022 Southern Stars. All rights reserved.
//
// Command-line test program for simulating mount model behavior
// using input from real Origin log files.

#include <cstdio>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <map>
#include <thread>

#include "SSMount.hpp"
#include "SSMountModel.hpp"
#include "SSUtilities.hpp"

void ParseImagingData ( const string &logpath )
{
    ifstream logfile ( logpath );
    if ( ! logfile.good() )
    {
        cout << "Can't open log file " << logpath << endl;
        return;
    }
    
    // Set up location and time. These will be overwriten when we process the log file.
    SSAngle lon = SSAngle::fromDMS( "-118 22 58.2" );
    SSAngle lat = SSAngle::fromDMS( "+33 50 27.9" );
    SSSpherical loc ( lon, lat, 0.0 );
    SSTime now = SSTime::fromSystem();
    SSDate date ( now ), imagedate;
    
    // Set up variables for imaging session data.
    string astrodir;
    double mountRA = 0.0, mountDec = 0.0;
    double mountAz = 0.0, mountAlt = 0.0;
    double plateRA = 0.0, plateDec = 0.0;
    double plateAz = 0.0, plateAlt = 0.0;
    double rate0 = 0.0, rate1 = 0.0;
    SSTime startTime = 0.0, imageTime = 0.0;
    int numCaptures = 0;
    
    // Read log file line by line
    size_t pos;
    string line;
    while ( getline ( logfile, line ) )
    {
        // Read local time from start of line
        if ( sscanf ( line.c_str(), "[%hd:%hd:%lf]", &date.hour, &date.min, &date.sec ) != 3 )
            continue;
        
        // If log file sets mount location, set our location to match
        if ( line.find ( "Set mount location to longitude" ) != string::npos )
        {
            pos = line.find ( "longitude" ) + 10;
            lon = SSAngle::fromDMS ( line.substr ( pos, 12 ) );
            
            pos = line.find ( "latitude" ) + 9;
            lat = SSAngle::fromDMS ( line.substr ( pos, 11 ) );
            
            loc = SSSpherical ( lon, lat, 0.0 );
            printf ( "Set mount location to longitude %s, latitude %s\n",
                    radToDMS ( loc.lon ).c_str(), radToDMS ( loc.lat ).c_str() );
        }
        
        if ( line.find ( "Set system clock to" ) != string::npos )
        {
            pos = line.find ( "clock to" ) + 9;
            sscanf ( line.substr ( pos, string::npos ).c_str(), "%d/%hd/%hd %hd:%hd:%lf UTC%lf",
                    &date.year, &date.month, &date.day, &date.hour, &date.min, &date.sec, &date.zone );
            printf ( "Set local date and time to %04d/%02hd/%02hd %02hd:%02hd:%02.0f UTC%+.1f\n\n",
                    date.year, date.month, date.day, date.hour, date.min, date.sec, date.zone );
        }
        
        // When a new imaging session starts, parse name of astrophotography dir
        // Print CSV column headers
        if ( line.find ( "Generating Astrophotograhy Directory") != string::npos )
        {
            pos = line.find ( "Astrophotography/" ) + 17;
            astrodir = line.substr ( pos, string::npos );
            printf ( "\n%s\n", astrodir.c_str() );
            printf ( "%3s,%7s,", "N", "tsince" );
            printf ( "%12s,", "HH:MM:SS.SSS" );
            printf ( "%10s,%10s,", "mountRA", "mountDec" );
            printf ( "%10s,%10s,", "mountAz", "mountAlt" );
            printf ( "%10s,%10s,", "plateRA", "plateDec" );
            printf ( "%10s,%10s,", "plateAz", "plateAlt" );
            printf ( "%10s,%10s", "rate0", "rate1" );
            printf ( "\n" );
            numCaptures = 0;
        }
        
        // If we are inside an imaging session...
        if ( ! astrodir.empty() )
        {
            // First save date and time each image was captured.
            if ( line.find ( "Image captured" ) != string::npos )
            {
                imagedate = date;
                imageTime = SSTime ( imagedate );

                // Save capture time for first image in session.
                if ( numCaptures == 0 )
                    startTime = imageTime;
                
                numCaptures++;
            }
            
            // For each image capture...
            if ( imageTime > 0.0 )
            {
                // Get mount predicted RA, Dec, Az, Alt
                if ( mountRA == 0.0 && mountDec == 0.0 && line.find ( "Got mount RA" ) != string::npos )
                {
                    pos = line.find ( "Got mount RA " ) + 13;
                    mountRA = strtofloat64 ( line.substr ( pos, 10 ) );
                    pos = line.find ( "Dec " ) + 4;
                    mountDec = strtofloat64 ( line.substr ( pos, 10 ) );
                    pos = line.find ( "Az " ) + 3;
                    mountAz = strtofloat64 ( line.substr ( pos, 10 ) );
                    pos = line.find ( "Alt " ) + 4;
                    mountAlt = strtofloat64 ( line.substr ( pos, 10 ) );
                }
                
                // Get plate-solve coordinates
                if ( plateRA == 0.0 && plateDec == 0.0 && line.find ( "Tetra3 solved image" ) != string::npos )
                {
                    getline ( logfile, line );
                    pos = line.find ( "RA = " ) + 5;
                    plateRA = strtofloat64 ( line.substr ( pos, 10 ) );
                    pos = line.find ( "Dec = " ) + 6;
                    plateDec = strtofloat64 ( line.substr ( pos, 10 ) );
                    pos = line.find ( "Az = " ) + 5;
                    plateAz = strtofloat64 ( line.substr ( pos, 10 ) );
                    pos = line.find ( "Alt = " ) + 6;
                    plateAlt = strtofloat64 ( line.substr ( pos, 10 ) );
                }
                
                // Get sidereal tracking rates
                if ( line.find ( "Sidereal tracking rates" ) != string::npos )
                {
                    pos = line.find ( "Sidereal tracking rates are " ) + 27;
                    rate0 = strtofloat64 ( line.substr ( pos, 10 ) );
                    pos = line.find ( "and " ) + 4;
                    rate1 = strtofloat64 ( line.substr ( pos, 9 ) );
                }
                
                // When new image notification is sent, or stacking failed, output all tracking data for this image
                if ( line.find ( "Sent NewImageReady" ) != string::npos
                || line.find ( "Image stacking failed" ) != string::npos )
                {

                    // print elapsed seconds since start of image capture, plus timestamp
                    double tsince = SSTime::kSecondsPerDay * ( imageTime - startTime );
                    printf ( "%3d,%7.3f,", numCaptures, tsince );
                    printf ( "%02hd:%02hd:%06.3f,", imagedate.hour, imagedate.min, imagedate.sec );
                    
                    // print mount RA/Dec
                    printf ( "%10.6f,%+010.6f,", mountRA, mountDec );
                    printf ( "%10.6f,%+010.6f,", mountAz, mountAlt );
                    
                    // if plate-solve RA/Dec differs from mount RA/Dec by more than 1 degree.
                    // assume false plate solve and display zeros.
                    SSSpherical mount ( degtorad ( mountRA ), degtorad ( mountDec ) );
                    SSSpherical plate ( degtorad ( plateRA ), degtorad ( plateDec ) );
                    if ( mount.angularSeparation( plate ) > degtorad ( 1.0 ) )
                        plateRA = plateDec = plateAz = plateAlt = 0.0;
                    
                    printf ( "%10.6f,%+010.6f,", plateRA, plateDec );
                    printf ( "%10.6f,%+010.6f,", plateAz, plateAlt );
                    
                    // print encoder rates
                    printf ( "%+10.6f,%+10.6f", rate0, rate1 );
                    printf ( "\n" );

                    // Then reset data for next image in session
                    imageTime = 0.0;
                    mountRA = mountDec = 0.0;
                    mountAz = mountAlt = 0.0;
                    plateRA = plateDec = 0.0;
                    plateAz = plateAlt = 0.0;
                }
            }
        }
        
        // When imaging session ends, reset name of astrophotography dir
        if ( line.find ( "Task RunImaging CANCELLED" ) != string::npos
        || line.find ( "Task RunImaging STOPPED" ) != string::npos )
        {
            astrodir = "";
            numCaptures = 0;
            printf ( "\n" );
        }
    }
}

// Computes polar alignment error in azimuth and altitude (both radians).
// Mount model must be aligned to an equatorial coordinate frame.
void GetPolarAlignError(SSCoordinates &coords, SSMountModel &model, double &azm, double &alt) {
    // get declination encoder position in radians when scope is pointed parallel to mount polar axis;
    double lat = coords.getLocation().lat; // note in southern hemisphere, dec decreases as enc1 increases.
    double enc1 = SSAngle::kHalfPi + (lat < 0.0 ? 1 : -1) * model.getParameter(MODEL_ALT_ZERO);
    
    // Compute RA/Dec where scope is pointed (should be very close to pole!); transform from equatorial to horizon coordinates.
    // True celestial pole should be at azimuth 0 in northerh hemisphere or 180 in southern, altitude same as observer latitude.
    // Polar alignment errors are differences in mount's actual azimuth/altitude from true celestial pole.
    SSAngle ha, dec;
    model.encodersToCelestial(0.0, enc1, ha, dec); // note hour angle/RA encoder value is irrelevant at pole.
    SSAngle ra = mod2pi(coords.getLST() - ha);     // convert hour angle to right ascension
    SSSpherical pole = coords.transform(kEquatorial, kHorizon, SSSpherical(ra, dec));
    printf("Along polar axis, enc1 = %+.4f, RA = %.3f (%s), Dec = %+.3f (%s), Azm = %.3f, Alt = %+.3f\n", radtodeg(enc1),
           ra.toHours(), radToHMS(ra).c_str(), dec.toDegrees(), radToDMS(dec).c_str(), pole.lon.toDegrees(), pole.lat.toDegrees());
    azm = modpi(pole.lon - (lat > 0 ? 0.0 : SSAngle::kPi));
    alt = pole.lat - fabs(lat);
}

// Computes zenith error in azimuth and altitude (both radians).
// Returns true if successful or false if not valid (mount not aligned, or not alt-azimuth).
void GetZenithError(SSCoordinates &coords, SSMountModel &model, double &azm, double &alt) {
    // get altitude encoder position in radians when scope is pointed parallel to mount azimuth axis.
    // Compute Azm/Alt where scope is actually pointed (should be very close to zenith!).
    double enc1 = SSAngle::kHalfPi - model.getParameter(MODEL_ALT_ZERO);
    SSAngle az, al;
    model.encodersToCelestial(0.0, enc1, az, al); // note azimuth encoder value is irrelevant at zenith.
    alt = SSAngle::kHalfPi - al;
    azm = az;
}

void TestMountModel ( const string &logpath )
{
    ifstream logfile ( logpath );
    if ( ! logfile.good() )
    {
        cout << "Can't open log file " << logpath << endl;
        return;
    }
    
    // Set up location and time. These will be overwriten when we process the log file.
    SSAngle lon = SSAngle::fromDMS( "-118 22 58.2" );
    SSAngle lat = SSAngle::fromDMS( "+33 50 27.9" );
    SSSpherical loc ( lon, lat, 0.0 );
    SSTime now = SSTime::fromSystem();
    SSDate date ( now );
    
    // Set up coordinate transforms and mount model
    SSCoordinates coords ( now, loc );
    SSMountModel model ( M_2PI, M_2PI );
    bool isEquatorial = false;
    
    // Read log file line by line
    string line;
    while ( getline ( logfile, line ) )
    {
        // Read local time from start of line
        if ( sscanf ( line.c_str(), "[%hd:%hd:%lf]", &date.hour, &date.min, &date.sec ) != 3 )
            continue;
        
        // If log file shows initialization, reset mount model
        if ( line.find ( "Task RunInitialize STARTED") != string::npos )
        {
            printf ( "Task RunInitialize STARTED %s\n", date.format("%Y-%m-%d %H:%M:%S").c_str() );
            model.reset();
        }
        
        if ( line.find ( "Mount configuration: equatorial") != string::npos )
        {
            printf ( "Mount is equatorial\n");
            isEquatorial = true;
        }
        
        // If log file sets mount location, set our location to match
        if ( line.find ( "Set mount location to longitude" ) != string::npos )
        {
            size_t pos = line.find ( "longitude" ) + 10;
            lon = SSAngle::fromDMS ( line.substr ( pos, 12 ) );
            
            pos = line.find ( "latitude" ) + 9;
            lat = SSAngle::fromDMS ( line.substr ( pos, 11 ) );
            
            loc = SSSpherical ( lon, lat, 0.0 );
            coords.setLocation ( loc );
            printf ( "Set mount location to longitude %s, latitude %s\n",
                    radToDMS ( loc.lon ).c_str(), radToDMS ( loc.lat ).c_str() );
        }
        
        if ( line.find ( "Set system clock to" ) != string::npos )
        {
            size_t pos = line.find ( "clock to" ) + 9;
            char zonename[256] = { 0 };
            sscanf ( line.substr ( pos, string::npos ).c_str(), "%d/%hd/%hd %hd:%hd:%lf, %s UTC%lf",
                    &date.year, &date.month, &date.day, &date.hour, &date.min, &date.sec, zonename, &date.zone );
            printf ( "Set local date and time to %04d/%02hd/%02hd %02hd:%02hd:%02.0f %s UTC%+.1f\n\n",
                    date.year, date.month, date.day, date.hour, date.min, date.sec, zonename, date.zone );
        }
            
        // If log file shows alignment attempt...
        if ( line.find ( "Attempting alignment at encoder positions" ) != string::npos )
        {
            double enc0 = 0.0, enc1 = 0.0;
            string ra = "", dec = "";
         
            // parse encoder positions in radians   
            size_t pos = line.find ( "positions") + 10;
            sscanf ( line.c_str() + pos, "%lf, %lf", &enc0, &enc1 );
            enc0 = degtorad ( enc0 );
            enc1 = degtorad ( enc1 );
            
            // parse RA, Dec
            pos = line.find ( "RA" ) + 3;
            ra = line.substr ( pos, 11 );
            
            pos = line.find ( "Dec" ) + 4;
            dec = line.substr ( pos, 11 );
            
            SSSpherical radec( SSAngle::fromHMS ( ra ), SSAngle::fromDMS ( dec ) ), ref;
            printf("Attempting alignment at encoder positions %+.4f, %+.4f deg = Reference J2000 RA %s, Dec %s\n",
                  radtodeg(enc0), radtodeg(enc1),
                  radToHMS(radec.lon).c_str(), radToDMS(radec.lat).c_str());
            
            // In southern hemisphere on equatorial mounts, encoders increase in opposite direction from RA and Dec.
            // In northern hemisphere and on alt-az mounts, encoders increase in same direction as RA/Dec and Az/Alt.
            if (model.numStars() == 0)
            {
                if (isEquatorial && coords.getLocation().lat < 0.0)
                    model = SSMountModel(-M_2PI, -M_2PI);
                else
                    model = SSMountModel(M_2PI, M_2PI);
            }
            
            // set local time, compute local sidereal time
            coords.setTime ( date );
            SSAngle lst = coords.getLST();
            printf("Local Date: %04d-%02hd-%02hd Time: %02hd:%02hd:%05.3f LST: %s\n",
                   date.year, date.month, date.day, date.hour, date.min, date.sec, radToHMS(lst).c_str());

            // Convert J2000 RA,Dec coordinates to JNow HA,Dec or Azm,Alt reference coordinates depending on mount type.
            if (isEquatorial) {
                ref = coords.transform(kFundamental, kEquatorial, radec);
                ref.lon = mod2pi(coords.getLST() - ref.lon); // convert JNow right ascension to local hour angle
                printf("Reference Local HA: %s (%+.4f deg) Dec: %s (%+.4f deg)\n",
                       radToHMS(ref.lon).c_str(), radtodeg(ref.lon),
                       radToDMS(ref.lat).c_str(), radtodeg(ref.lat));
            } else {
                ref = coords.transform(kFundamental, kHorizon, radec);
                printf("Reference Az: %+.4f Alt: %+.4f\n", radtodeg(ref.lon), radtodeg(ref.lat));
            }
            
            // If further than minimum distance from nearest existing reference, add new reference to mount model.
            int index = 0;
            SSAngle distance, minDistRad = SSAngle::fromDegrees(1.0);
            if (model.numStars() > 0) {
                index = model.nearestStar(ref, distance);
                printf("Nearest mount model reference is %.3f degrees away at index %d.\n", distance.toDegrees(), index);
                if (distance < minDistRad) {
                    printf("Must be > %.3f degrees away; rejecting new reference; mount model is unchanged with %d references\n\n",
                          radtodeg(minDistRad), model.numStars());
                    continue;
                }
            }
            
            // Save current mount model, then add new alignment star.
            SSMountModel oldModel = model;
            bool result = model.addStar(enc0, enc1, ref.lon, ref.lat);
            
            model.adjustable ( MODEL_TILT1, model.numStars() > 1 );
            model.adjustable ( MODEL_TILT2, model.numStars() > 1 );
            //model.adjustable ( MODEL_MISALIGN, model.numStars() > 2 );
            //model.adjustable ( MODEL_FLEXURE, model.numStars() > 2 );
            
            // Recompute best-fit alignment model.
            double rmsErr = model.align();
            double altRate = radtodeg(model.getParameter(MODEL_ALT_RATE));
            double azmRate = radtodeg(model.getParameter(MODEL_AZM_RATE));
            double altZero = radtodeg(model.getParameter(MODEL_ALT_ZERO));
            double azmZero = radtodeg(model.getParameter(MODEL_AZM_ZERO));
            double tilt1 = radtodeg(model.getParameter(MODEL_TILT1));
            double tilt2 = radtodeg(model.getParameter(MODEL_TILT2));
            //double misalign = radtodeg(model.getParameter(MODEL_MISALIGN));
            //double flexure = radtodeg(model.getParameter(MODEL_FLEXURE));
            
            // display RMS residual and best-fit parameters
            printf("Aligned mount model with %d references; RMS error = %.1f arcsec\n", model.numStars(), rmsErr * SSAngle::kArcsecPerRad);
            printf("MODEL_ALT_RATE = %+.4f deg/rad\n", altRate);
            printf("MODEL_ALT_ZERO = %+.4f deg\n", altZero);
            printf("MODEL_AZM_RATE = %+.4f deg/rad\n", azmRate);
            printf("MODEL_AZM_ZERO = %+.4f deg\n", azmZero);
            printf("MODEL_TILT1    = %+.4f deg\n", tilt1);
            printf("MODEL_TILT2    = %+.4f deg\n", tilt2);
            //printf("MODEL_MISALIGN = %+.4f deg\n", misalign);
            //printf("MODEL_FLEXURE  = %+.4f deg\n\n", flexure);
            
            // If we have enough alignment references for a non-trivial model, restore the old mount model if the new one is crazy.
            // Degenerate mount models can occur with (for example) a 2-star alt-az alignment where the 2 stars have (nearly) the same azimuth,
            if ((isEquatorial && model.numStars() > 3) || (!isEquatorial && model.numStars() > 1))
            {
                double maxTilt = isEquatorial ? 180.0 : 5.0;
                if (fabs(tilt1) > maxTilt || fabs(tilt2) > maxTilt || fabs(altZero) > 360.0 || fabs(azmZero) > 360.0)
                {
                    model = oldModel;
                    printf("Rejecting new model fit due to unacceptable tilt angles or zero points; "
                          "mount model is unchanged with %d references",
                          model.numStars());
                    continue;
                }
            }
            
            // Compute and display polar alignment error for equatorial mounts, or zenith error for alt-azimuth mounts.
            double azmErr = 0.0, altErr = 0.0;
            if (isEquatorial) {
                GetPolarAlignError(coords, model, azmErr, altErr);
                printf("Polar alignment errors: azm %+.3f, alt %+.3f deg\n", radtodeg(azmErr), radtodeg(altErr));
            } else {
                GetZenithError(coords, model, azmErr, altErr);
                printf("Zenith errors: azm %+.3f, alt %+.3f deg\n", radtodeg(azmErr), radtodeg(altErr));
            }
        }
    }
}

int main ( int argc, const char * argv[] )
{
    // path to origin log file we want to take input from.
    string logpath = "/Users/timmyd/Desktop/originCoreLog_2025-06-26.txt";
    TestMountModel( logpath );
    //ParseImagingData( logpath );
    return 0;
}
