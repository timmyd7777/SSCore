// SSMountModel.hpp
// SSCore
//
// Created by Tim DeBenedictis on 3/21/23.
// Copyright © 2023 Southern Stars. All rights reserved.
//
// This class implements a telescope mount pointing model.
// It is based on original code provided by Project Pluto.

#ifndef SSMountModel_hpp
#define SSMountModel_hpp

#include "SSAngle.hpp"
#include "SSVector.hpp"

#define MODEL_ALT_RATE     0    // Altitude encoder rate [radians/step]
#define MODEL_ALT_ZERO     1    // Altitude encoder zero point [radians]
#define MODEL_AZM_RATE     2    // Azimuth encoder rate [radians/step]
#define MODEL_AZM_ZERO     3    // Azimuth encoder zero point [radians]
#define MODEL_TILT1        4    // Tilt #1 [radians]
#define MODEL_TILT2        5    // Tilt #2 [radians]
#define MODEL_MISALIGN     6    // Non-perpendicularity [radians]
#define MODEL_FLEXURE      7    // Flexure (cos alt) [radians]
#define MODEL_AZM_COS      8    // cosine of azimuth
#define MODEL_AZM_SIN      9    // sine of azimuth
#define MODEL_ALT_SIN      10   // sine of altitude
#define MODEL_ALT_COS2     11   // cosine of 2 * altitude
#define MODEL_ALT_SIN2     12   // sine of 2 * altitude
#define MODEL_AZM_COS2     13   // cosine of 2 * azimuth
#define MODEL_AZM_SIN2     14   // sine of 2 * azimuth
#define MODEL_WARPED_TBL_1 15   // Warped table 1
#define MODEL_WARPED_TBL_2 16   // Warped table 2
#define MODEL_N_PARAMS     17   // Maximum number of model parameters
#define MODEL_N_STARS      100   // Maximum number of alignment stars

// Represents a model of a telescope mount's alignment with the celestial coordinate system.
// Determined by best fit to a selection of reference stars.
// Contains methods for transforming celestial coordinates to mount axis encoder positions
// and vice-versa.

class SSMountModel
{
protected:
    
    double _m[MODEL_N_PARAMS];              // array of model parameters, meanings as #defined above.
    char _adjustable[MODEL_N_PARAMS];       // array of booleans indicating which parameters are adjustable (true) or fixed (false).
    double _xres;                           // mount X (azimuth) axis encoder steps per revolution
    double _yres;                           // mount Y (altitude) axis encoder steps per revolution
    int _n_stars;                           // number of reference stars used for alignment

public:
    vector<double> _x_stars;                // mount X (azimuth) axis encoder position of alignment stars
    vector<double> _y_stars;                // mount Y (altitude) axis encoder position of alignment stars
    vector<double> _azm_stars;              // azimuth angles of alignment stars [radians]
    vector<double> _alt_stars;              // altitude angles of alignment stars [radians]

public:
    
    SSMountModel ( double xres, double yres );
    
    bool adjustable ( int param );
    void adjustable ( int param, bool adj );
    
    double getParameter ( int param ) { return param >= 0 && param < MODEL_N_PARAMS ? _m[param] : 0.0; }
    void setParameter ( int param, double value ) { if ( param >= 0 && param < MODEL_N_PARAMS ) _m[param] = value; }

    void encodersToCelestial ( double x, double y, SSAngle &azm, SSAngle &alt );
    void celestialToEncoders ( SSAngle azm, SSAngle alt, double &x, double &y );
    
    int numStars ( void ) { return _n_stars; }
    bool addStar ( double x, double y, SSAngle azm, SSAngle alt );
    bool getStar ( int i, double &x, double &y, SSAngle &azm, SSAngle &alt );
    bool setStar ( int i, double x, double y, SSAngle azm, SSAngle alt );
    bool delStar ( int i );
    void reset ( void );
    
    double align ( void );
    bool getResiduals ( int i, double &azm_resid, double &alt_resid );
    
    int nearestStar ( SSSpherical point, SSAngle &nearestDistance, int startRef = 0, int numRefs = MODEL_N_STARS );
    int farthestStar ( SSSpherical point, SSAngle &farthestDistance, int startRef = 0, int numRefs = MODEL_N_STARS );
};

#endif /* SSMountModel_hpp */
