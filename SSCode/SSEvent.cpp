// SSEvent.cpp
// SSTest
//
// Created by Tim DeBenedictis on 4/18/20.
// Copyright © 2020 Southern Stars. All rights reserved.

#include "SSEvent.hpp"

// Computes the hour angle when an object with declination (dec)
// as seen from latitude (lat) reaches an altitude (alt) above
// or below th horison.  All angles are in radians.
// If the object's altitude is always greater than (alt), returns kPi.
// If the altitude is always less than (alt), returns zero.

SSAngle SSEvent::semiDiurnalArc ( SSAngle lat, SSAngle dec, SSAngle alt )
{
    double cosha = ( sin ( alt ) - sin ( dec ) * sin ( lat ) ) / ( cos ( dec ) * cos ( lat ) );
    
    if ( cosha >= 1.0 )
        return 0.0;
    else if ( cosha <= -1.0 )
        return SSAngle::kPi;
    else
        return acos ( cosha );
}

// Given an object's equatorial coordinates (ra,dec) at a particular instant (time),
// returns the object's time of rising, transit, or setting above a horizon altitude (alt),
// as seen from a particular geographic longitude (lon) and latitude (lat).
// The event to compute (sign) is -1 = rising, 0 = transit, +1 = setting.
// All angles are in radians; north and east are positive.
// The returned time will always be within 0.5 days of the input time, except:
// if the object does set below the horizon altitude, return +INFINITY;
// if the object does rise above the horizon altitude, returns -INFINITY.
// The object's equatorial coordinates, (ra,dec), should be given for the same
// precessional epoch as the input time!
// This function ignores the object's motion during day, OK for stars;
// but for moving solar system objects, use riseTransitSetSearch().
// For point objects, use horizon altitude -0.5 degrees to account for refraction.
// For the Sun and Moon, use horizon altitude -50 arcminutes.
// For civil, nautical, and astronomical twilight, use a horizon
// altitude of -6, -12, and -18 degrees, respectively.

SSTime SSEvent::riseTransitSet ( SSTime time, SSAngle ra, SSAngle dec, int sign, SSAngle lon, SSAngle lat, SSAngle alt )
{
    // Compute the object's hour angle when it reaches the rise/set altitude.

    SSAngle ha = semiDiurnalArc ( lat, dec, alt );

    // If the object never sets, return infinity;
    // if it never rises, return negative infinity.
         
    if ( ha == SSAngle::kPi && sign != 0 )
        return ( INFINITY );
    
    if ( ha == 0.0 )
        return ( -INFINITY );
    
    // Compute the local sidereal time
    
    SSAngle lst = time.getSiderealTime ( lon );
    
    // Now compute the angular distance that the earth needs to turn
    // through to make the object reach the horizon altitude.
    // Reduce it to the range -kPi to +kPi
    
    SSAngle theta = ( ra - lst + sign * ha ).modPi();
    
    // Obtain the time of rising or setting by adding the amount of time
    // the earth takes to rotate through the angle calculated above to the
    // current time.
        
    return time + ( theta / SSAngle::kTwoPi / SSTime::kSiderealPerSolarDays );
}

// Wrapper for the above which takes input location in the SSCoordinates object (obj)
// and gets equatorial RA and Dec from an object (obj); other params (sign,alt) are as above.

SSTime SSEvent::riseTransitSet ( SSTime time, SSCoordinates &coords, SSObjectPtr pObj, int sign, SSAngle alt )
{
    SSSpherical loc = coords.getLocation();
    SSSpherical equ ( coords.transform ( kFundamental, kEquatorial, pObj->getDirection() ) );
    return riseTransitSet ( time, equ.lon, equ.lat, sign, loc.lon, loc.lat, alt );
}

// Computes the time of an object's rise, transit, or set that is closest to an initial starting time (time).
// All other parameters (coords, pObj, sign, alt) are the same as for riseTransitSet().
// If the object does not rise, returns -INFINITY; if it does not set, returns +INFINITY.
// After return, both coords and pObj will be modified (recomputed for the time of the event).
// Will not work for objects which rise and set multiple times per day, e.g. artifical satellites.

SSTime SSEvent::riseTransitSetSearch ( SSTime time, SSCoordinates &coords, SSObjectPtr pObj, int sign, SSAngle alt )
{
    SSTime lasttime = time;
    int i = 0, imax = 10;
    double precision = 1.0 / SSTime::kSecondsPerDay;
             
    // Iteratively compute the object's position and predict increasingly accurate
    // estimates of the object's rise/transit/set time, until the estimate of the time
    // converges to the specified precision, or we perform the maximum number of iterations.

    do
    {
        lasttime = time;
        coords.setTime ( time );
        pObj->computeEphemeris ( coords );
        time = riseTransitSet ( time, coords, pObj, sign, alt );
        i++;
    }
    while ( fabs ( time - lasttime ) > precision && ! isinf ( time ) && i < imax );
            
    return ( time );
}

// Computes the time an object's rises, transits, or sets on a particular local day (today).
// All other parameters (coords, pObj, sign, alt) are the same as for riseTransitSet().
// If the object does not rise or set on the specified local day, returns -INFINITY or +INFINITY.
// After return, both coords and pObj will be modified (recomputed for the time of the event).
// Will not work for objects which rise and set multiple times per day, e.g. artifical satellites.

SSTime SSEvent::riseTransitSetSearchDay ( SSTime today, SSCoordinates &coords, SSObjectPtr pObj, int sign, SSAngle alt )
{
    // Find the julian dates that correspond to the start and end of the local day.
         
    SSTime start = today.getLocalMidnight();
    SSTime end = start + 1.0;

    // Search for the object's exact rise/set time, starting from the middle of the local day.

    SSTime time = riseTransitSetSearch ( start + 0.5, coords, pObj, sign, alt );
    
    // If the resulting rise/set time is after the end of the day,
    // start searching again from the middle of the previous day;
    // similarly, if the resulting rise/set time is before the start
    // of the current day, start searching again from the middle
    // of the following day.

    if ( time > end )
        time = riseTransitSetSearch ( start - 0.5, coords, pObj, sign, alt );
    else if ( time < start )
        time = riseTransitSetSearch ( end + 0.5, coords, pObj, sign, alt );

    // If the resulting rise/set time is still before the beginning or
    // after the end of the local day, the object does not rise or set
    // on that day; return positive/negative infinity to indicate this.
    
    if ( time > end || time < start )
    {
        if ( sign == -1 )
            time = -INFINITY;
        else
            time = INFINITY;
    }
                
    return ( time );
}

// Returns circumstances of an object's overhead pass on a given local day (today) as seen from a
// particular location (coords), with the altitude for rising and setting (alt) in radians.
// Transit time and elevation are stored in the returned pass's transit struct.
// If the object does not rise or set on the specified local day, returns -INFINITY or +INFINITY.
// After return, both coords and pObj will be restored to their original states.
// Will not work for objects which rise and set multiple times per day, e.g. artifical satellites.

SSPass SSEvent::riseTransitSet ( SSTime today, SSCoordinates &coords, SSObjectPtr pObj, SSAngle alt )
{
    SSTime savetime = coords.getTime();
    SSPass pass = { 0.0 };
    SSSpherical hor = { INFINITY, INFINITY, INFINITY };
    
    pass.rising.time = riseTransitSetSearchDay ( today, coords, pObj, kRise, alt );
    if ( ! isinf ( pass.rising.time ) )
    {
        hor = coords.transform ( kFundamental, kHorizon, pObj->getDirection() );
        pass.rising.azm = hor.lon;
        pass.rising.alt = hor.lat;
    }
    
    pass.transit.time = riseTransitSetSearchDay ( today, coords, pObj, kTransit, 0.0 );
    if ( ! isinf ( pass.transit.time ) )
    {
        hor = coords.transform ( kFundamental, kHorizon, pObj->getDirection() );
        pass.transit.azm = hor.lon;
        pass.transit.alt = hor.lat;
    }

    pass.setting.time = riseTransitSetSearchDay ( today, coords, pObj, kSet, alt );
    if ( ! isinf ( pass.setting.time ) )
    {
        hor = coords.transform ( kFundamental, kHorizon, pObj->getDirection() );
        pass.setting.azm = hor.lon;
        pass.setting.alt = hor.lat;
    }

    // Reset original time and restore object's original ephemeris
    
    coords.setTime ( savetime );
    pObj->computeEphemeris ( coords );
    
    return pass;
}

// Searches for satellite passes seen from a location (coords) between two Julian dates (start to stop).
// Passes start when satellite's apparent altitude rises above a minimum threshold (minAlt) in radians;
// passes end when satellite's elevation falls below that threshold.  Peak elevation and time thereof are
// also recorded in each pass's transit struct. The method returns the total number of passes found, and
// returns all pass circumstances in the vector of SSPass structs.
// After return, both coords and pObj will be restored to their original states.

int SSEvent::findSatellitePasses ( SSCoordinates &coords, SSObjectPtr pSat, SSTime start, SSTime stop, double minAlt, vector<SSPass> &passes )
{
    SSTime  savetime = coords.getTime();
    SSTime  time = 0, step = 0;
    SSPass  pass = { 0 };
    SSAngle azm = 0, alt = 0, maxAlt = 0, oldAlt = 0;
    SSSpherical hor = { INFINITY, INFINITY, INFINITY };
    
    for ( time = start; time <= stop; time += step )
    {
        // Advance time and recompute satellite's position;
        // obtain its current elevation and azimuth.
        
        coords.setTime ( time );
        pSat->computeEphemeris ( coords );
        hor = coords.transform ( kFundamental, kHorizon, pSat->getDirection() );
        azm = hor.lon;
        alt = hor.lat;
        
        // When the satellite reaches 1 degree below the horizon,
        // change time step to 1 second for more precision.
        // Otherwise use a coarse 1-minute time step for speed.
        
        if ( hor.lat > -1.0 * SSAngle::kDegPerRad )
            step = 1.0 / SSTime::kSecondsPerDay;
        else
            step = 1.0 / SSTime::kMinutesPerDay;
        
        if ( time > start )
        {
            // If satellite is above elevation threshold now,
            // but below it on previous step, pass starts now.
            
            if ( alt > minAlt && oldAlt < minAlt )
            {
                pass.rising.time = time;
                pass.rising.azm = azm;
                pass.rising.alt = alt;
            }
            
            // Search for peak elevation.

            if ( alt > maxAlt )
            {
                pass.transit.time = time;
                pass.transit.azm = azm;
                pass.transit.alt = alt;
                maxAlt = alt;
            }
            
            // If satellite is below elevation threshold now,
            // but above it on previous step, pass starts now.

            if ( oldAlt > minAlt && alt < minAlt )
            {
                pass.setting.time = time;
                pass.setting.azm = azm;
                pass.setting.alt = alt;
                passes.push_back ( pass );
                maxAlt = 0.0;
            }
        }
        
        oldAlt = alt;
    }
    
    // Reset original time and restore satellite's original ephemeris
    
    coords.setTime ( savetime );
    pSat->computeEphemeris ( coords );

    return (int) passes.size();
}

// Returns the Juliam Date of the next moon phase after the current time (time).
// Objects pSun and pMoon are pointers to the SUn and Moon, respectively.
// The angular value (phase) corresponds to the desired moon phase in radians:
// new = 0.0, first quarter = kHalfPi, full = kPi, last quarter = 3 * kPi / 2.
// The moon's and sun's positions will be recomputed/modified by this function!

SSTime SSEvent::nextMoonPhase ( SSTime time, SSObjectPtr pSun, SSObjectPtr pMoon, double phase )
{
    SSSpherical ecl;
    SSAngle sunlon, moonlon;
    double dellon = 0.0, deltime = 0.0;
    int i = 0, imax = 10;
    
    // Moon phases are defined from a geocentric perspective,
    // so set location to the center of the Earth.
    
    SSSpherical loc ( 0.0, 0.0, -SSCoordinates::kKmPerEarthRadii );
    SSCoordinates coords ( time, loc );
    
    // Iteratively recompute Sun and Moon's ecliptic longitude until
    // difference between them equals the desired phase angle.
    
    do
    {
        coords.setTime ( time );
        pSun->computeEphemeris ( coords );
        ecl = coords.transform ( kFundamental, kEcliptic, pSun->getDirection() );
        sunlon = ecl.lon;
        
        pMoon->computeEphemeris ( coords );
        ecl = coords.transform ( kFundamental, kEcliptic, pMoon->getDirection() );
        moonlon = ecl.lon;

        // On first iteration, ensure ecliptic longitude delta is negative
        // so time change on next iteration must be ahead of starting time.
        
        dellon = modpi ( moonlon - sunlon - phase );
        if ( i == 0 && dellon > 0.0 )
            dellon -= SSAngle::kTwoPi;
        else
            dellon = modpi ( moonlon - sunlon - phase );
        
        // Estimate time delta needed for ecliptic longitude delta
        // assuming Moon's synodic period is 29.5 days. Iterate until
        // time delta is less than 1 minute, or 10 iterations max.
        
        deltime = dellon / ( SSAngle::kTwoPi / 29.5 );
        time -= deltime;
        i++;
    }
    while ( fabs ( deltime ) > ( 1.0 / SSTime::kMinutesPerDay ) && i < imax );
    
    return time;
}

double object_distance ( SSCoordinates &coords, SSObjectPtr pObj1, SSObjectPtr pObj2 )
{
    SSVector pos1 = pObj1->getDirection() * pObj1->getDistance();
    SSVector pos2 = pObj2->getDirection() * pObj2->getDistance();
    
    return pos1.distance( pos2 );
}

double object_separation ( SSCoordinates &coords, SSObjectPtr pObj1, SSObjectPtr pObj2 )
{
    SSVector dir1 = pObj1->getDirection();
    SSVector dir2 = pObj2->getDirection();
    
    return dir1.angularSeparation ( dir2 );
}

double object_altitude ( SSCoordinates &coords, SSObjectPtr pObj1, SSObjectPtr pObj2 )
{
    SSVector dir = pObj1->getDirection();
    SSSpherical hor = coords.transform ( kFundamental, kHorizon, dir );
    return hor.lat;
}

// Generic event-finding method for "maximum and minimum"-type events. This type of event occurrs when a value
// (physical distance, angular separation, etc.) reaches a local maximum or minimum above (or below) a certain threshold value (limit).
// The geographic location from which the event(s) are being sought is in the coordinates object (coords).
// The object(s) involved in the event are pObj1 and pObj2.
// The time range to search over (start to stop), is in Julian Dates.
// The initial search step (step) is in days.
// The boolean flag (min) instructs whether to search for local minima (true) or maxima (false) of the value.
// The function (func) returns the value for those objects at a given time.
// The coordinates (coords) and objects' (pObj1,pObj2) positions will be recomputed/modified by this function!

void SSEvent::findEvents ( SSCoordinates &coords, SSObjectPtr pObj1, SSObjectPtr pObj2, SSTime start, SSTime stop, double step, bool min, double limit, SSEventFunc func, vector<SSEventTime> &events, int maxEvents )
{
    double newVal = INFINITY, curVal = INFINITY, oldVal = INFINITY;
    
    for ( SSTime time = start; time <= stop && events.size() < maxEvents; time += step )
    {
        // Compute the ephemerides of the objects at the current time,
        // then the value of the event function.

        coords.setTime ( time );
        
        if ( pObj1 )
            pObj1->computeEphemeris ( coords );
        
        if ( pObj2 )
            pObj2->computeEphemeris ( coords );
        
        // Save the current value into the old value, and the new value into the current value,
        // so that when we compute a new distance, we will have three different values we can
        // search for a maximum or minimum.
        
        if ( ! isinf ( curVal ) )
            oldVal = curVal;
            
        if ( ! isinf ( newVal ) )
            curVal = newVal;

        // Find the new value of the event function at the current time.

        newVal = func ( coords, pObj1, pObj2 );
        
        // If we have an old, current, and new value, see if we have a minimum
        // or maximum bracketed between the old, current, and new times. If so,
        // call this method recursively to search the interval between those times
        // with a search step 10x smaller, until the step is less than 1 second.
        // When we reach that precision, save the time and value, and return.

        if ( ! isinf ( oldVal ) && ! isinf ( curVal ) && ! isinf ( newVal ) )
        {
            if ( ( min && ( newVal > curVal && curVal < oldVal ) && curVal <= limit )
            || ( ! min && ( newVal < curVal && curVal > oldVal ) && curVal >= limit ) )
            {
                if ( step < 1.0 / SSTime::kSecondsPerDay )
                {
                    SSEventTime event = { time - step, curVal };
                    events.push_back ( event );
                    return;
                }
                else
                {
                    findEvents ( coords, pObj1, pObj2, time - step * 2.0, time, step / 10.0, min, limit, func, events, maxEvents );
                }
            }
        }
    }
}

// Generic event-finding method for "equality" events. This type of event ocurrs when a value (distance, altitude, declination, etc.)
// equals a desired target value (target). There are two sub-types of equality event: 1) when the value reaches the target from below,
// and 2) when the value reaches the target from above.
// The boolean flag (below) defines which to search for: true = equality from below, false = from above.
// All other parameters are the same as for findEvents().
// The coordinates (coords) and objects' (pObj1,pObj2) positions will be recomputed/modified by this function!

void SSEvent::findEqualityEvents ( SSCoordinates &coords, SSObjectPtr pObj1, SSObjectPtr pObj2, SSTime start, SSTime stop, double step, bool below, double target, SSEventFunc func, vector<SSEventTime> &events, int maxEvents )
{
    double curVal = INFINITY, oldVal = INFINITY;
    
    for ( SSTime time = start; time <= stop && events.size() < maxEvents; time += step )
    {
        // Compute the ephemerides of the objects at the current time,
        // then the value of the event function.

        coords.setTime ( time );
        
        if ( pObj1 )
            pObj1->computeEphemeris ( coords );
        
        if ( pObj2 )
            pObj2->computeEphemeris ( coords );
        
        // Save the current value into the old value, then find
        // the value of the event function at the current time.

        if ( ! isinf ( curVal ) )
            oldVal = curVal;
            
        curVal = func ( coords, pObj1, pObj2 );
        
        // If we have an old and current, and new value, see if we have a minimum
        // or maximum bracketed between the old, current, and new times. If so,
        // call this method recursively to search the interval between those times
        // with a search step 10x smaller, until the step is less than 1 second.
        // When we reach that precision, save the time and value, and return.

        if ( ! isinf ( oldVal ) && ! isinf ( curVal ) )
        {
            if ( ( below && ( curVal >= target && oldVal < target ) )
            || ( ! below && ( curVal <= target && oldVal > target ) ) )
            {
                if ( step < 1.0 / SSTime::kSecondsPerDay )
                {
                    SSEventTime event = { time, curVal };
                    events.push_back ( event );
                    return;
                }
                else
                {
                    findEqualityEvents ( coords, pObj1, pObj2, time - step, time, step / 10.0, below, target, func, events, maxEvents );
                }
            }
        }
    }
}

void SSEvent::findConjunctions ( SSCoordinates &coords, SSObjectPtr pObj1, SSObjectPtr pObj2, SSTime start, SSTime stop, vector<SSEventTime> &events, int maxEvents )
{
    findEvents ( coords, pObj1, pObj2, start, stop, 1.0, true, INFINITY, object_separation, events, maxEvents );
}

void SSEvent::findOppositions ( SSCoordinates &coords, SSObjectPtr pObj1, SSObjectPtr pObj2, SSTime start, SSTime stop, vector<SSEventTime> &events, int maxEvents )
{
    findEvents ( coords, pObj1, pObj2, start, stop, 1.0, false, 0.0, object_separation, events, maxEvents );
}

void SSEvent::findNearestDistances ( SSCoordinates &coords, SSObjectPtr pObj1, SSObjectPtr pObj2, SSTime start, SSTime stop, vector<SSEventTime> &events, int maxEvents )
{
    findEvents ( coords, pObj1, pObj2, start, stop, 1.0, true, INFINITY, object_distance, events, maxEvents );
}

void SSEvent::findFarthestDistances ( SSCoordinates &coords, SSObjectPtr pObj1, SSObjectPtr pObj2, SSTime start, SSTime stop, vector<SSEventTime> &events, int maxEvents )
{
    findEvents ( coords, pObj1, pObj2, start, stop, 1.0, false, 0.0, object_distance, events, maxEvents );
}

// Searches for satellite passes seen from a location (coords) between two Julian dates (start to stop).
// Passes start when satellite's apparent altitude rises above a minimum threshold (minAlt) in radians;
// passes end when satellite's elevation falls below that threshold.  Peak elevation and time thereof are
// also recorded in each pass's transit struct. The method returns the total number of passes found, and
// returns all pass circumstances in the vector of SSPass structs.
// After return, both coords and pObj will be restored to their original states.

int SSEvent::findSatellitePasses2 ( SSCoordinates &coords, SSObjectPtr pSat, SSTime start, SSTime stop, double minAlt, vector<SSPass> &passes, int maxPasses )
{
    SSTime  savetime = coords.getTime();
    
    while ( true )
    {
        // First search for the next satellite rising. Save satellite horizon coords at end of search. Quit if we find none.
        
        vector<SSEventTime> risings;
        findEqualityEvents ( coords, pSat, nullptr, start, stop, 1.0 / SSTime::kMinutesPerDay, true, minAlt, object_altitude, risings, 1 );
        SSSpherical risingCoords = coords.transform ( kFundamental, kHorizon, pSat->getDirection() );
        if ( risings.size() == 0 )
            break;

        // Now search for the next satellite setting, within 1 day after the rising time. Save satellite horizon coords at end of search. Quit if we find none.
        
        vector<SSEventTime> settings;
        findEqualityEvents ( coords, pSat, nullptr, risings[0].time, risings[0].time + 1.0, 1.0 / SSTime::kMinutesPerDay, false, minAlt, object_altitude, settings, 1 );
        SSSpherical settingCoords = coords.transform ( kFundamental, kHorizon, pSat->getDirection() );
        if ( settings.size() == 0 )
            break;
        
        // Finally search for the next transit time after rising but before setting. Save satellite horizon coords at end of search. Quit if we find none.
        
        vector<SSEventTime> transits;
        findEvents ( coords, pSat, nullptr, risings[0].time, settings[0].time, ( settings[0].time - risings[0].time ) / 10.0, false, minAlt, object_altitude, transits, 1 );
        SSSpherical transitCoords = coords.transform ( kFundamental, kHorizon, pSat->getDirection() );
        if ( transits.size() == 0.0 )
            break;
        
        // We found a complete pass!
        
        SSPass pass;
        
        pass.rising.time = risings[0].time;
        pass.rising.azm = risingCoords.lon;
        pass.rising.alt = risingCoords.lat;
        
        pass.transit.time = transits[0].time;
        pass.transit.azm = transitCoords.lon;
        pass.transit.alt = transitCoords.lat;

        pass.setting.time = settings[0].time;
        pass.setting.azm = settingCoords.lon;
        pass.setting.alt = settingCoords.lat;

        // Save and add to pass vector.  Quit if we've saved max desired number of passes.
        // Otherwise start search for next satellite rising when satellite sets in current pass.
        
        passes.push_back ( pass );
        if ( passes.size() >= maxPasses )
            break;
        
        start = pass.setting.time;
    }

    // Reset original time and restore satellite's original ephemeris
    
    coords.setTime ( savetime );
    pSat->computeEphemeris ( coords );

    return (int) passes.size();
}
