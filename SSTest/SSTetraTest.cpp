//  SSTetraTest.cpp
//
//  Created by Tim DeBenedictis on 11/24/23.
//  Copyright © 2023 Southern Stars. All rights reserved.

#include "SSMatrix.hpp"
#include "Tetra3.hpp"

int main ( int argc, const char *argv[] )
{
    Tetra3 t3 = Tetra3();
    if ( ! t3.loadDatabase ( argv[1] ) )
    {
        cout << "Can't load Tetra3 database from " << argv[1] << endl;
        return -1;
    }
    cout << "Loaded Tetra3 database with " << t3.numPatterns() << " patterns and " << t3.numStars() << " stars\n";

    // (x,y) coordinates sources extracted from test image IMG_2023-08-16-20-38-05.png
    
    vector<T3Source> sources =
    {
        { 422.2053, 1023.395 },
        { 281.8795, 717.26306 },
        { 16.997013, 397.85364 },
        { 301.96262, 257.07523 },
        { 257.723, 130.81393 },
        { 51.945026, 1246.7628 },
        { 553.97327, 1059.6846 },
        { 686.87354, 589.4739 },
        { 520.88165, 200.29431 },
        { 47.36175, 48.079197 },
        { 638.39435, 228.18639 },
        { 238.14902, 572.54694 },
        { 133.90385, 1207.5717 },
        { 601.55334, 665.07666 },
        { 391.38275, 362.40567 },
        { 3.9298568, 711.7092 },
        { 509.51547, 761.1291 },
        { 252.37495, 671.2923 },
        { 66.05293, 745.0243 },
        { 527.5116, 1050.9066 },
        { 414.95157, 680.3477 },
        { 180.20613, 1091.7495 },
        { 309.05966, 385.49396 },
        { 363.34433, 902.1853 },
        { 693.9221, 459.2104 },
        { 492.25372, 421.7162 },
        { 111.76887, 577.5644 },
        { 136.35097, 1166.6543 },
        { 231.51476, 630.458 },
        { 298.34338, 909.7004 },
        { 685.6176, 854.58813 },
        { 626.6023, 264.4852 },
        { 548.761, 589.7871 },
        { 494.89798, 317.44052 },
        { 407.7845, 749.5002 },
        { 175.77464, 1.3097101 },
        { 625.3755, 588.4812 },
        { 716.32, 778.4647 },
        { 512.4454, 687.7868 },
        { 498.4067, 531.715 },
        { 55.375015, 651.53796 },
        { 556.0732, 484.23492 },
        { 316.3822, 940.83386 },
        { 620.3483, 151.55928 },
        { 253.4568, 778.41846 },
        { 483.61166, 728.68823 },
        { 29.697231, 754.6336 },
        { 413.4434, 495.5556 },
        { 301.53394, 898.5347 },
        { 662.64374, 355.62747 },
        { 692.6906, 443.37155 },
        { 668.5518, 650.63855 },
        { 262.32025, 1267.565 },
        { 396.4324, 775.4081 },
        { 229.59746, 892.5005 },
        { 446.70026, 223.4975 },
        { 148.49733, 889.2969 },
        { 513.2085, 478.54318 },
        { 368.49814, 670.64166 },
        { 319.41254, 1032.5724 },
        { 679.5646, 1158.5579 },
        { 104.49749, 194.53372 },
        { 337.4633, 1038.56 },
        { 33.50009, 79.44353 },
        { 92.609924, 481.57355 },
        { 382.47665, 981.4335 }
    };
    
    // Initialize solver options
    
    T3Options opts;
    opts.fov_estimate = 24.0;
    opts.fov_max_error = 1.0;
    opts.match_radius = 0.01;
    opts.match_threshold = 1.0e-6;
    opts.num_threads = 0;
    opts.pattern_checking_stars = 20;
    opts.pattern_max_error = 0.0;
    
    // solve image
    
    T3Results results;
    if ( ! t3.solveFromSources ( sources, 720, 1280, opts, results ) )
    {
        cout << "Failed to solve " << sources.size() << " sources in " << results.t_solve << " ms!\n";
        return -2;
    }
    
    cout << "Solved " << sources.size() << " sources in " << results.t_solve << " ms.\n";
    cout << "R.A.: " << SSHourMinSec ( results.ra / 15.0 ).toString() << endl;
    cout << "Dec.: " << SSDegMinSec ( results.dec ).toString() << endl;
    cout << "FoV:  " << results.fov << " deg\n";
    cout << "Roll: " << results.roll << " deg\n";

    return 0;
}
