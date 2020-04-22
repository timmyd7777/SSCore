// SSMoonEphemeris.cpp
// SSTest
//
// Created by Tim DeBenedictis on 4/22/20.
// Copyright © 2020 Southern Stars. All rights reserved.
//
// This code for computing outer planet moons is a C++ wrapper around
// an extended version of Bill Gray's Project Pluto code.  For Jupiter's
// Galilean moons, all of the periodic terms have been added here.

#include "SSCoordinates.hpp"
#include "SSMatrix.hpp"
#include "SSOrbit.hpp"
#include "SSMoonEphemeris.hpp"

#define PI 3.1415926535897932384626433832795028841971693993751058209749445923
#define TWO_PI (2. * PI)
#define DEG2RAD (PI / 180.)
#define COEFF2RAD (PI / 180.e+5)
#define PER (13.469942 * PI / 180.)
#define J1900 ( 2451545. - 36525.)
#define J2000   2451545.

      /* This algorithm has a lot of angles which are linear functions */
      /* of time,  with the slope and offset expressed in degrees.  To */
      /* get a result properly converted to radians,  the following    */
      /* macro is useful:                                              */
#define LINEAR_FUNC( A, B, t) ((A * DEG2RAD) + (B * DEG2RAD) * t)

      /* And there are a few cubic functions of time,  too: */
#define CUBIC_FUNC( A, B, C, D, t) (A * DEG2RAD + t * (B * DEG2RAD \
                        + t * (C * DEG2RAD + D * DEG2RAD * t)))

static void rotate_vector( const double angle, double *x, double *y)
{
   const double sin_angle = sin( angle);
   const double cos_angle = cos( angle);
   const double temp = cos_angle * *x - sin_angle * *y;

   *y = sin_angle * *x + cos_angle * *y;
   *x = temp;
}

/* 28 Sep 2002:  Kazumi Akiyama pointed out two slightly wrong
   coefficients (marked 'KA fix' below).  These change the position
   of Europa by as much as 300 km (worst case),  of Callisto by
   as much as 3 km.
 */

/* Formulae taken from Jean Meeus' _Astronomical Algorithms_.  WARNINg:
   the coordinates returned in the 'jsats' array are ecliptic Cartesian
   coordinates of _date_,  not J2000 or B1950!  Units are Jovian radii.
   Input time is in TD.                            */

int  calc_jsat_loc( const double jd, double *jsats,
                         const int sats_wanted, const long precision)
{
   const double t = jd - 2443000.5;          /* 1976 aug 10, 0:00 TD */
               /* calc precession since B1950 epoch */
   const double precess_time = (jd - 2433282.423) / 36525.;
   const double precession =
              LINEAR_FUNC( 1.3966626, .0003088, precess_time) * precess_time;
   const double dt = (jd - J2000) / 36525.;
                  /* mean longitudes of satellites, p 289: */
   const double l1 = LINEAR_FUNC( 106.07719, 203.488955790, t);
   const double l2 = LINEAR_FUNC( 175.73161, 101.374724735, t);
   const double l3 = LINEAR_FUNC( 120.55883,  50.317609209, t);
   const double l4 = LINEAR_FUNC(  84.44459,  21.571071177, t);

                  /* longitudes of perijoves: */
   const double pi1 = LINEAR_FUNC(  97.0881, 0.16138586, t);
   const double pi2 = LINEAR_FUNC( 154.8663, 0.04726307, t);
   const double pi3 = LINEAR_FUNC( 188.1840, 0.00712734, t);
   const double pi4 = LINEAR_FUNC( 335.2868, 0.00184000, t);

                  /* longitudes of ascending nodes */
                  /* on Jupiter's equatorial plane: */
   const double ome1 = LINEAR_FUNC( 312.3346, -0.13279386, t);
   const double ome2 = LINEAR_FUNC( 100.4411, -0.03263064, t);
   const double ome3 = LINEAR_FUNC( 119.1942, -0.00717703, t);
   const double ome4 = LINEAR_FUNC( 322.6168, -0.00175934, t);
         /* Longitude of Jupiter's ascending node;  p. 213 */
         /* (table 31A)                                    */
   const double asc_node = CUBIC_FUNC( 100.464407, 1.0209774, .00040315, 4.04e-7, dt);
         /* Inclination of Jupiter's orbit; same source */
   const double incl_orbit = CUBIC_FUNC( 1.303267, -.0054965, 4.66e-6, -2.e-9, dt);
         /* gam = gamma, principal inequality in the longitude of Jupiter */
   const double temp1 = LINEAR_FUNC( 163.679,  0.0010512, t);
   const double temp2 = LINEAR_FUNC(  34.486, -0.0161731, t);
   const double gam = 0.33033 * DEG2RAD * sin( temp1) + 0.03439 * DEG2RAD * sin( temp2);
         /* "There is a small libration, with a period of 2071 days,  in */
         /* the longitudes of the three inner satellites: when satellite */
         /* II decelerates,  I and III accelerate.  To take this into    */
         /* account,  we need the phase of free libration..."            */
   const double libration = LINEAR_FUNC( 199.6766, 0.17379190, t);
      /* Longitude of the node of the equator of Jupiter on the ecliptic: */
   const double psi = LINEAR_FUNC( 316.5182, -2.08e-6, t);
      /* Mean anomalies of Jupiter and Saturn: */
   const double g = LINEAR_FUNC( 30.23756, 0.0830925701, t) + gam;
   const double g_prime = LINEAR_FUNC( 31.97853, 0.0334597339, t);
   const double twice_per_plus_g = 2. * g + 2. * PER;
              /* Inclination of Jupiter's axis to its orbital plane: */
   const double incl = LINEAR_FUNC( 3.120262, .0006, (jd - J1900) / 36525.);
   double lon[5], tan_lat[5], rad[5];
   double loc[18];
   int i;

   for( i = 1; i < 5; i++)
      lon[i] = tan_lat[i] = rad[i] = 0.;

   if( sats_wanted & 1)       /* Io */
      {
      const double del1 =
               47259. * COEFF2RAD * sin( 2. * (l1 - l2))
               -3478. * COEFF2RAD * sin( pi3 - pi4)
               +1081. * COEFF2RAD * sin( l2 - 2. * l3 + pi3)
               + 738. * COEFF2RAD * sin( libration)
               + 713. * COEFF2RAD * sin( l2 - l3 - l3 + pi2)
               - 674. * COEFF2RAD * sin( pi1 + pi3 - twice_per_plus_g)
               + 666. * COEFF2RAD * sin( l2 - 2. * l3 + pi4)
               + 445. * COEFF2RAD * sin( l1 - pi3)
               - 354. * COEFF2RAD * sin( l1 - l2)
               - 317. * COEFF2RAD * sin( 2. * psi - 2. * PER)
               + 265. * COEFF2RAD * sin( l1 - pi4)
               - 186. * COEFF2RAD * sin( g)
               + 162. * COEFF2RAD * sin( pi2 - pi3)
               + 158. * COEFF2RAD * sin( 4. * (l1 - l2))
               - 155. * COEFF2RAD * sin( l1 - l3)
               - 138. * COEFF2RAD * sin( psi + ome3 - twice_per_plus_g)
               - 115. * COEFF2RAD * sin( 2. * ( l1 - 2. * l2 + ome2))
               +  89. * COEFF2RAD * sin( pi2 - pi4)
               +  85. * COEFF2RAD * sin( l1 + pi3 - twice_per_plus_g)
               +  83. * COEFF2RAD * sin( ome2 - ome3)
               +  53. * COEFF2RAD * sin( psi - ome2);

      lon[1] = l1 + del1;
      tan_lat[1] = 6393.e-7 * sin( lon[1] - ome1)
             + 1825.e-7 * sin( lon[1] - ome2)
             +  329.e-7 * sin( lon[1] - ome3)
             +  311.e-7 * sin( lon[1] - psi)
             +   93.e-7 * sin( lon[1] - ome4)
             +   75.e-7 * sin( 3. * lon[1] - 4. * l2 - 1.9927 * del1 + ome2)
             +   46.e-7 * sin(lon[1] + psi - twice_per_plus_g);
      rad[1] = -41339.e-7 * cos( 2. * (l1 - l2))
               -  387.e-7 * cos( l1 - pi1)
               -  214.e-7 * cos( l1 - pi4)
               +  170.e-7 * cos( l1 - l2)
               -  131.e-7 * cos( 4. * (l1 - l2))
               +  106.e-7 * cos( l1 - l3)
               -   66.e-7 * cos( l1 + pi3 - twice_per_plus_g);
      }

   if( sats_wanted & 2)       /* europa */
      {                       
      const double del2 =
               106476. * COEFF2RAD * sin( 2. * (l2 - l3))
                +4256. * COEFF2RAD * sin( l1 - l2 - l2 + pi3)
                +3581. * COEFF2RAD * sin( l2 - pi3)
                +2395. * COEFF2RAD * sin( l1 - l2 - l2 + pi4)
                +1984. * COEFF2RAD * sin( l2 - pi4)
                -1778. * COEFF2RAD * sin( libration)
                +1654. * COEFF2RAD * sin( l2 - pi2)
                +1334. * COEFF2RAD * sin( l2 - l3 - l3 + pi2)
                +1294. * COEFF2RAD * sin( pi3 - pi4)    /* KA fix */
                -1142. * COEFF2RAD * sin( l2 - l3)
                -1057. * COEFF2RAD * sin( g)
                - 775. * COEFF2RAD * sin( 2. * ( psi - PER))
                + 524. * COEFF2RAD * sin( 2. * (l1 - l2))
                - 460. * COEFF2RAD * sin( l1 - l3)
                + 316. * COEFF2RAD * sin( psi + ome3 - twice_per_plus_g)
                - 203. * COEFF2RAD * sin( pi1 + pi3 - twice_per_plus_g)
                + 146. * COEFF2RAD * sin( psi - ome3)
                - 145. * COEFF2RAD * sin( g + g)
                + 125. * COEFF2RAD * sin( psi - ome4)
                - 115. * COEFF2RAD * sin( l1 - 2. * l3 + pi3)
                -  94. * COEFF2RAD * sin( 2. * (l2 - ome2))
                +  86. * COEFF2RAD * sin( 2. * (l1 - 2. * l2 + ome2))
                -  86. * COEFF2RAD * sin( 5. * g_prime - 2. * g + DEG2RAD * 52.225)
                -  78. * COEFF2RAD * sin( l2 - l4)
                -  64. * COEFF2RAD * sin( 3. * l3 - 7. * l4 + 4 * pi4)
                +  64. * COEFF2RAD * sin( pi1 - pi4)
                -  63. * COEFF2RAD * sin( l1 - 2. * l3 + pi4)
                +  58. * COEFF2RAD * sin( ome3 - ome4)
                +  56. * COEFF2RAD * sin( 2. * (psi - PER - g))
                +  55. * COEFF2RAD * sin( 2. * (l1 - l3))
                +  52. * COEFF2RAD * sin( 3. * l3 - 7. * l4 + pi3 + 3. * pi4)
                -  43. * COEFF2RAD * sin( l1 - pi3)
                +  41. * COEFF2RAD * sin( 5. * (l2 - l3))
                +  41. * COEFF2RAD * sin( pi4 - PER)
                +  32. * COEFF2RAD * sin( ome2 - ome3)
                +  32. * COEFF2RAD * sin( 2. * (l3 - g - PER));

      lon[2] = l2 + del2;
      tan_lat[2] = 81004.e-7 * sin( lon[2] - ome2)
               +4512.e-7 * sin( lon[2] - ome3)
               -3284.e-7 * sin( lon[2] - psi)
               +1160.e-7 * sin( lon[2] - ome4)
               + 272.e-7 * sin( l1 - 2. * l3 + 1.0146 * del2 + ome2)
               - 144.e-7 * sin( lon[2] - ome1)
               + 143.e-7 * sin( lon[2] + psi - twice_per_plus_g)
               +  35.e-7 * sin( lon[2] - psi + g)
               -  28.e-7 * sin( l1 - 2. * l3 + 1.0146 * del2 + ome3);
      rad[2] = 93848.e-7 * cos( l1 - l2)
               -3116.e-7 * cos( l2 - pi3)
               -1744.e-7 * cos( l2 - pi4)
               -1442.e-7 * cos( l2 - pi2)
               + 553.e-7 * cos( l2 - l3)
               + 523.e-7 * cos( l1 - l3)
               - 290.e-7 * cos( 2. * (l1 - l2))
               + 164.e-7 * cos( 2. * (l2 - ome2))
               + 107.e-7 * cos( l1 - 2. * l3 + pi3)
               - 102.e-7 * cos( l2 - pi1)
               -  91.e-7 * cos( 2. * (l1 - l3));
      }

   if( sats_wanted & 4)       /* ganymede */
      {
      const double del3 =
               16490. * COEFF2RAD * sin( l3 - pi3)
               +9081. * COEFF2RAD * sin( l3 - pi4)
               -6907. * COEFF2RAD * sin( l2 - l3)
               +3784. * COEFF2RAD * sin( pi3 - pi4)
               +1846. * COEFF2RAD * sin( 2. * (l3 - l4))
               -1340. * COEFF2RAD * sin( g)
               -1014. * COEFF2RAD * sin( 2. * ( psi - PER))
               + 704. * COEFF2RAD * sin( l2 - l3 - l3 + pi3)
               - 620. * COEFF2RAD * sin( l2 - l3 - l3 + pi2)
               - 541. * COEFF2RAD * sin( l3 - l4)
               + 381. * COEFF2RAD * sin( l2 - l3 - l3 + pi4)
               + 235. * COEFF2RAD * sin( psi - ome3)
               + 198. * COEFF2RAD * sin( psi - ome4)
               + 176. * COEFF2RAD * sin( libration)
               + 130. * COEFF2RAD * sin( 3. * (l3 - l4))
               + 125. * COEFF2RAD * sin( l1 - l3)
               - 119. * COEFF2RAD * sin( 5. * g_prime - 2. * g + 52.225 * DEG2RAD)
               + 109. * COEFF2RAD * sin( l1 - l2)
               - 100. * COEFF2RAD * sin( 3. * l3 - 7. * l4 + 4. * pi4)
               +  91. * COEFF2RAD * sin( ome3 - ome4)
               +  80. * COEFF2RAD * sin( 3. * l3 - 7. * l4 + pi3 + 3. * pi4)
               -  75. * COEFF2RAD * sin( 2. * l2 - 3. * l3 + pi3)
               +  72. * COEFF2RAD * sin( pi1 + pi3 - twice_per_plus_g)
               +  69. * COEFF2RAD * sin( pi4 - PER)
               -  58. * COEFF2RAD * sin( 2. * l3 - 3. * l4 + pi4)
               -  57. * COEFF2RAD * sin( l3 - 2. * l4 + pi4)
               +  56. * COEFF2RAD * sin( l3 + pi3 - twice_per_plus_g)
               -  52. * COEFF2RAD * sin( l2 - 2. * l3 + pi1)
               -  50. * COEFF2RAD * sin( pi2 - pi3)
               +  48. * COEFF2RAD * sin( l3 - 2. * l4 + pi3)
               -  45. * COEFF2RAD * sin( 2. * l2 - 3. * l3 + pi4)
               -  41. * COEFF2RAD * sin( pi2 - pi4)
               -  38. * COEFF2RAD * sin( 2. * g)
               -  37. * COEFF2RAD * sin( pi3 - pi4 + ome3 - ome4)
               -  32. * COEFF2RAD * sin( 3. * l3 - 7. * l4 + 2. * pi3 + 2. * pi4)
               +  30. * COEFF2RAD * sin( 4. * (l3 - l4))
               +  29. * COEFF2RAD * sin( l3 + pi4 - twice_per_plus_g)
               -  28. * COEFF2RAD * sin( ome3 + psi - twice_per_plus_g);

      lon[3] = l3 + del3;
      tan_lat[3] = 32402.e-7 * sin( lon[3] - ome3)
              -16911.e-7 * sin( lon[3] - psi)
               +6847.e-7 * sin( lon[3] - ome4)
               -2797.e-7 * sin( lon[3] - ome2)
               + 321.e-7 * sin( lon[3] + psi - twice_per_plus_g)
               +  51.e-7 * sin( lon[3] - psi + g)
               -  45.e-7 * sin( lon[3] - psi - g)
               -  45.e-7 * sin( lon[3] - psi - 2. * PER)
               +  37.e-7 * sin( lon[3] + psi - twice_per_plus_g)
               +  30.e-7 * sin( 2. * l2 - 3. * lon[3] + 4.03 * del3 + ome2)
               -  21.e-7 * sin( 2. * l2 - 3. * lon[2] + 4.03 * del3 + ome3);
          
      rad[3] = -14388.e-7 * cos( l3 - pi3)
                -7919.e-7 * cos( l3 - pi4)
                +6342.e-7 * cos( l2 - l3)
                -1761.e-7 * cos( 2. * (l3 - l4))
                + 294.e-7 * cos( l3 - l4)
                - 156.e-7 * cos( 3. * (l3 - l4))
                + 156.e-7 * cos( l1 - l3)
                - 153.e-7 * cos( l1 - l2)
                -  70.e-7 * cos( 2. * l2 - 3. * l3 + pi3)
                -  51.e-7 * cos( l3 + pi3 - twice_per_plus_g);
      }

   if( sats_wanted & 8)       /* callisto */
      {
      const double del4 =
               84287. * COEFF2RAD * sin( l4 - pi4)
              + 3431. * COEFF2RAD * sin( pi4 - pi3)
              - 3305. * COEFF2RAD * sin( 2. * (psi - PER))
              - 3211. * COEFF2RAD * sin( g)
              - 1862. * COEFF2RAD * sin( l4 - pi3)
              + 1186. * COEFF2RAD * sin( psi - ome4)
              +  623. * COEFF2RAD * sin( l4 + pi4 - twice_per_plus_g)
              +  387. * COEFF2RAD * sin( 2. * (l4 - pi4))
              -  284. * COEFF2RAD * sin( 5. * g_prime - 2. * g + 52.225 * DEG2RAD)
              -  234. * COEFF2RAD * sin( 2. * (psi - pi4))
              -  223. * COEFF2RAD * sin( l3 - l4)         /* KA fix */
              -  208. * COEFF2RAD * sin( l4 - PER)
              +  178. * COEFF2RAD * sin( psi + ome4 - 2. * pi4)
              +  134. * COEFF2RAD * sin( pi4 - PER)
              +  125. * COEFF2RAD * sin( 2. * l4 - twice_per_plus_g)
              -  117. * COEFF2RAD * sin( 2. * g)
              -  112. * COEFF2RAD * sin( 2. * (l3 - l4))
              +  107. * COEFF2RAD * sin( 3. * l3 + 7. * l4 + 4. * pi4)
              +  102. * COEFF2RAD * sin( l4 - g- PER)
              +   96. * COEFF2RAD * sin( 2. * l4 - psi - ome4)
              +   87. * COEFF2RAD * sin( 2. * (psi - ome4))
              -   85. * COEFF2RAD * sin( 3. * l3 - 7. * l4 + pi3 + 3. * pi4)
              +   85. * COEFF2RAD * sin( l3 - 2. * l4 + pi4)
              -   81. * COEFF2RAD * sin( 2. * (l4 - psi))
              +   71. * COEFF2RAD * sin( l4 + pi4 - 2. * PER - 3. * g)
              +   61. * COEFF2RAD * sin( l1 - l4)
              -   56. * COEFF2RAD * sin( psi - ome3)
              -   54. * COEFF2RAD * sin( l3 - 2*l4 + pi3)
              +   51. * COEFF2RAD * sin( l2 - l4)
              +   42. * COEFF2RAD * sin( 2. * (psi - g - PER))
              +   39. * COEFF2RAD * sin( 2. * (pi4 - ome4))
              +   36. * COEFF2RAD * sin( psi + PER - pi4 - ome4)
              +   35. * COEFF2RAD * sin( 2. *g_prime - g + DEG2RAD * 188.37)
              -   35. * COEFF2RAD * sin( l4 - pi4 + 2. * PER - 2*psi)
              -   32. * COEFF2RAD * sin( l4 + pi4 - 2. * PER - g)
              +   30. * COEFF2RAD * sin( 2. * g_prime - 2*g + DEG2RAD * 149.15)
              +   29. * COEFF2RAD * sin( 3. * l3 - 7*l4 + 2*pi3 + 2*pi4)
              +   28. * COEFF2RAD * sin( l4 - pi4 + 2*psi - 2*PER)
              -   28. * COEFF2RAD * sin( 2. * (l4 - ome4))
              -   27. * COEFF2RAD * sin( pi3 - pi4 + ome3 - ome4)
              -   26. * COEFF2RAD * sin( 5. * g_prime - 3. * g + DEG2RAD * 188.37)
              +   25. * COEFF2RAD * sin( ome4 - ome3)
              -   25. * COEFF2RAD * sin( l2 - 3. *l3 + 2. *l4)
              -   23. * COEFF2RAD * sin( 3. * (l3 - l4))
              +   21. * COEFF2RAD * sin( 2. * l4 - 2. * PER - 3*g)
              -   21. * COEFF2RAD * sin( 2. * l3 - 3. * l4 + pi4)
              +   19. * COEFF2RAD * sin( l4 - pi4 - g)
              -   19. * COEFF2RAD * sin( 2. * l4 - pi3 - pi4)
              -   18. * COEFF2RAD * sin( l4 - pi4 + g)
              -   16. * COEFF2RAD * sin( l4 + pi3 - 2. * PER - 2*g);

      lon[4] = l4 + del4;
      tan_lat[4] = - 76579.e-7 * sin( lon[4] - psi)
               + 44134.e-7 * sin( lon[4] - ome4)
               -  5112.e-7 * sin( lon[4] - ome3)
               +   773.e-7 * sin( lon[4] + psi - twice_per_plus_g)
               +   104.e-7 * sin( lon[4] - psi + g)
               -   102.e-7 * sin( lon[4] - psi - g)
               +    88.e-7 * sin( lon[4] + psi - twice_per_plus_g - g)
               -    38.e-7 * sin( lon[4] + psi - twice_per_plus_g + g);
      rad[4] = -73546.e-7 * cos( l4 - pi4)
                +1621.e-7 * cos( l4 - pi3)
                + 974.e-7 * cos( l3 - l4)
                - 543.e-7 * cos( l4 + pi4 - twice_per_plus_g)
                - 271.e-7 * cos( 2. * (l4 - pi4))
                + 182.e-7 * cos( l4 - PER)
                + 177.e-7 * cos( 2. * (l3 - l4))
                - 167.e-7 * cos( 2. * l4 - psi - ome4)
                + 167.e-7 * cos( psi - ome4)
                - 155.e-7 * cos( 2. * l4 - twice_per_plus_g)
                + 142.e-7 * cos( 2. * (l4 - psi))
                + 105.e-7 * cos( l1 - l4)
                +  92.e-7 * cos( l2 - l4)
                -  89.e-7 * cos( l4 - PER - g)
                -  62.e-7 * cos( l4 + pi4 - twice_per_plus_g - g)
                +  48.e-7 * cos( 2. * (l4 - ome4));
      }

   for( i = 0; i < 15; i++)
      loc[i] = 0.;
   for( i = 1; i < 6; i++)
      if( sats_wanted & (1 << (i - 1)))
         {
         double *tptr = (double *)loc + (i - 1) * 3;

                                    /* calc coords by Jupiter's equator */
         if( i != 5)
            {
            static const double r0[4] = { 5.90569, 9.39657, 14.98832, 26.36273 };
            const double csc_lat = sqrt( 1. + tan_lat[i] * tan_lat[i]);
            const double r = r0[i - 1] * (1. + rad[i]);

            tptr[0] = r * cos( lon[i] - psi) / csc_lat;
            tptr[1] = r * sin( lon[i] - psi) / csc_lat;
            tptr[2] = r * tan_lat[i] / csc_lat;
            }
         else
            tptr[2] = 1.;     /* fictitious fifth satellite */

                            /* rotate to plane of Jup's orbit: */
         rotate_vector( incl, tptr + 1, tptr + 2);
                            /* rotate to Jup's ascending node: */
         rotate_vector( psi + precession - asc_node, tptr, tptr + 1);
                            /* rotate to the ecliptic */
         rotate_vector( incl_orbit, tptr + 1, tptr + 2);
                            /* rotate to vernal equinox.  This results */
                            /* in ecliptic coords of date.  In Meeus,  */
                            /* topo[0...2] will be A4, B4, C4.         */
         rotate_vector( asc_node, tptr, tptr + 1);
                            /* Meeus does further rotations to get into */
                            /* a system in which the z-axis points from */
                            /* earth to Jupiter and y points along the  */
                            /* rotation axis of Jupiter.  We don't need */
                            /* any of that here.                        */
         }
   memcpy( jsats, loc, 12 * sizeof( double));
   if( sats_wanted & 16)      /* imaginary sat wanted */
      memcpy( jsats + 12, loc + 12, 3 * sizeof( double));
   return( sats_wanted);
}

void SSMoonEphemeris::jupiterMoonPosition ( int id, double jed, SSVector &pos )
{
    double jsats[15] = { 0 };
    
    // compute Jupiter-centric position of requested moon, in Jupiter radii,
    // in ecliptic frame of date.
    
    if ( id == 501 )
    {
        calc_jsat_loc ( jed, jsats, 1, 0 );
        pos = SSVector ( jsats[0], jsats[1], jsats[2] );
    }
    else if ( id == 502 )
    {
        calc_jsat_loc ( jed, jsats, 2, 0 );
        pos = SSVector ( jsats[3], jsats[4], jsats[5] );
    }
    else if ( id == 503 )
    {
        calc_jsat_loc ( jed, jsats, 4, 0 );
        pos = SSVector ( jsats[6], jsats[7], jsats[8] );
    }
    else if ( id == 504 )
    {
        calc_jsat_loc ( jed, jsats, 8, 0 );
        pos = SSVector ( jsats[9], jsats[10], jsats[11] );
    }

    // convert from Jupiter radii to AU
    
    pos *= 71420.0 / SSCoordinates::kKmPerAU;
    
    // transform from ecliptic frame of date to J2000 equatorial frame.
    
    static double matrixJED = 0.0;
    static SSMatrix matrix;
    
    if ( jed != matrixJED )
    {
        SSMatrix eclMat = SSCoordinates::getEclipticMatrix ( SSCoordinates::getObliquity ( jed ) );
        SSMatrix preMat = SSCoordinates::getPrecessionMatrix ( jed ).transpose();
        matrix = preMat * eclMat;
        matrixJED = jed;
    }

    pos = matrix * pos;
}

/*
All references are from G. Dourneau unless otherwise noted.

The Phoebe orbital elements are from the _Explanatory Supplement to
the Astronomical Almanac_,  and should not be trusted very much;  they
are horribly outdated,  and don't match reality very well at all.
They are not actually used in any of my code.

There are a few places to look for alternative algorithms/code for the
satellites of Saturn.  Peter Duffett-Smith's book "Practical Astronomy
with your Calculator" provides a simpler theory,  with mostly circular
orbits,  and Dan Bruton has implemented this in BASIC code in his
SATSAT2 program.  At the other extreme,  the Bureau des Longitudes
(http://www.bdl.fr) provides Fortran code implementing the TASS 1.7
theory,  the successor to the Dourneau theory used in the following
code.  TASS probably supplies slightly better accuracy than the
Dourneau theory,  but you would have to be looking well below the
arcsecond level to see much difference.

None of these provides good data for Phoebe.  If you're really interested
in Phoebe,  let me know;  I can provide the source code used in Guide for
Phoebe (and the other irregular satellites of gas giants).  It uses
multipoint interpolation in precomputed ephemerides,  resulting in
wonderful accuracy.  (The precomputed ephemeris resulted from a
numerically integrated orbit.)

   'htc20.cpp' provides ephemerides for Helene,  Telesto,  and Calypso.
'rocks.cpp' provides ephemerides for many other faint inner satellites
of Saturn (and other planets).
*/

#define OBLIQUITY_1950 (23.445792 * PI / 180.)
            /* Constants defining the angle of a 'fixed' Saturnian equator */
            /* relative to the B1950.0 ecliptic.  The inner four moons are */
            /* all computed relative to the plane of Saturn's equator; you */
            /* then rotate by these two angles, and you're in B1950.0      */
            /* ecliptic coordinates.  (The outer four moons are all in that */
            /* system to begin with.)        */
#define INCL0 (28.0817 * PI / 180.)
#define ASC_NODE0 (168.8112 * PI / 180.)

#define JAPETUS_i0         (18.4602 * PI / 180.)
#define JAPETUS_i0_dot     (-.9518 * PI / 180.)

#define IGNORED_DOUBLE     0.

#define MIMAS           0
#define ENCELADUS       1
#define TETHYS          2
#define DIONE           3
#define RHEA            4
#define TITAN           5
#define HYPERION        6
#define JAPETUS         7
#define PHOEBE          8

#define SECONDS_TO_AU (9.538937 * (PI / 180.) / 3600.)

#define SAT_ELEMS struct sat_elems

SAT_ELEMS
   {
   double jd, semimaj, ecc, gamma, lambda;
   double omega, Omega, epoch;
   double loc[4];
   int sat_no;
   };

/* set_ssat_elems( ) is the core part of computing positions for the
satellites of Saturn,  and quite probably the only part of the code
you'll want to grab.  It is essentially just an implementation of
Gerard Dourneau's theory.  The only problem with this theory is that
each satellite has to be handled a little differently... thus the extensive
case statement in this function.  The result,  though,  is a set of
orbital elements for the object.  For the inner four moons,  this is
relative to the equator of Saturn,  and you have to do two rotations to
get a B1950.0 coordinate.  For the outer four moons,  you get B1950.0
elements right away. */

static int set_ssat_elems( SAT_ELEMS *elems, SSOrbit *orbit)
{
   static const long semimaj[9] = { 268180L, 344301, 426393, 545876,
               762277, 1766041, 2140790, 5148431, 18720552 };
   static const short epoch[8] = { 11093, 11093, 11093, 11093, 11093, 11368,
                15020, 9786 };
   static const short ecc0[8] = { 19050, 4850, 0, 2157, 265, 29092, -1,
                                       28298   /*, 163260 */ };
   static const short i_gamma0[8] = { 15630, 262, 10976, 139,
                 3469, 2960, 6435, -1 };
   static const long lam0[9] = {1276400, 2003170, 2853060, 2547120, 3592440,
                  2611582, 1770470, 763852, 2778720 };
   static const double n[9] = { 381.994497, 262.7319002, 190.69791226,
                  131.53493193, 79.6900472, 22.57697855,
                  16.91993829, 4.53795125, -.6541068 };
   static const long big_N0[9] = { 54500, 348000, 111330, 232000, 345000,
                            42000, 94900, 143198, 245998 };
   static const long big_N0_dot[9] = { -36507200, -15195000, -7224410,
                          -3027000, -1005700, -51180, -229200, -3919, -41353 };
   static const long big_P0[9] = { 106100, 309107, 0, 174800, 276590, 276590,
                          69898, 352910, 280165 };
   static const long big_P0_dot[9] = { 36554900, 12344121, 0, 3082000,
                          51180, 51180, -1867088, 11710, -19586 };
   const double sin_gamma0_tan_half_incl = .00151337;
   const double sin_gamma0 = .0060545;
   const double sin_incl1 = .470730;
   int sat = elems->sat_no;
   double t, t_d, t_centuries, t_centuries_squared;

   if( sat == PHOEBE)
      {
      elems->epoch = 2433282.5;
      elems->ecc = .16326;
      }
   else
      {
      elems->epoch = 2400000. + (double)epoch[sat];
      elems->ecc = (double)ecc0[sat] * 1.e-6;
      elems->gamma = (double)i_gamma0[sat] * (PI / 180.) / 10000.;
      }
   t_d = elems->jd - elems->epoch;
   t = t_d / 365.25;
   t_centuries = t / 100.;
   t_centuries_squared = t_centuries * t_centuries;
   if( sat == PHOEBE)
      elems->gamma = (173.949 - .020 * t) * (PI / 180.);

   elems->semimaj = (double)semimaj[sat] * SECONDS_TO_AU / 10000.;
   elems->lambda = (double)lam0[sat] / 10000. + n[sat] * t_d;
   elems->lambda *= PI / 180;          /* cvt to radians */
   elems->Omega = (double)big_N0[sat] / 1000. +
                               t * (double)big_N0_dot[sat] / 100000.;
   elems->Omega *= PI / 180;          /* cvt to radians */
   elems->omega = (double)big_P0[sat] / 1000. +
                               t * (double)big_P0_dot[sat] / 100000.;
   elems->omega *= PI / 180;          /* cvt to radians */

   switch( sat)
      {
      case MIMAS:
      case TETHYS:
         {
         const double libration_coeffs[3] = {-43.57 * PI / 180.,
                  -.7209 * PI / 180., -.0205 * PI / 180. };
         const double mu0 = 5.095 * PI / 180.;
         const double t0_prime = 1866.39;
         const double mimas_over_tethys = -21.12;
         double mu_delta_tau = mu0 *
                        ((elems->jd-J2000) / 365.25 + 2000. - t0_prime);
         int i;
         double delta_lon = 0.;

         for( i = 0; i < 3; i++)
            delta_lon += libration_coeffs[i] *
                                     sin( (double)(i+i+1) * mu_delta_tau);
         if( sat == TETHYS)
            delta_lon /= mimas_over_tethys;
         elems->lambda += delta_lon;
         }
         break;

      case ENCELADUS:
      case DIONE:
         {
         const double p2 = 15.4 * (PI / 180.) / 60.;
         const double q2 = 12.59 * (PI / 180.) / 60.;
         const double mu = 74.4 * (PI / 180.);
         const double nu = 32.39 * (PI / 180.);
         const double mu_prime = 134.3 * (PI / 180.);
         const double nu_prime = 92.62 * (PI / 180.);
         const double enceladus_over_dione = -12.;
         double delta_lon;

         delta_lon = p2 * sin( mu + nu * t) +
                     q2 * sin( mu_prime + nu_prime * t);
         if( sat == DIONE)
            delta_lon /= enceladus_over_dione;
         elems->lambda += delta_lon;
         }
         break;
      case RHEA:
         {
         const double ef = .001;
         const double chi = .0193 * PI / 180.;
         const double pi0 = 342.7 * PI / 180.;
         const double pi_dot = 10.057 * PI / 180.;
         const double big_Nt0 = 42.02 * PI / 180.;
         const double big_Nt_dot = -.5118 * PI / 180.;
         const double Omega1_plus_dOmega = ASC_NODE0 - .0078 * PI / 180.;
         const double Incl1_plus_dIncl = INCL0 - .0455 * PI / 180.;
         const double e0 = .000265;

         const double pi = pi0 + pi_dot * t;
         const double big_N = elems->Omega;
         const double big_Nt = big_Nt0 + big_Nt_dot * t;
         const double e_sin_omega = e0 * sin( pi) + ef * sin( elems->omega);
         const double e_cos_omega = e0 * cos( pi) + ef * cos( elems->omega);
         double perturb_Omega, perturb_incl;

         perturb_incl = sin_gamma0 * cos( big_N) + chi * cos( big_Nt);
         elems->gamma = Incl1_plus_dIncl + perturb_incl;
         perturb_Omega = sin_gamma0 * sin( big_N) + chi * sin( big_Nt);
         elems->Omega = Omega1_plus_dOmega + perturb_Omega / sin_incl1;
         elems->lambda += sin_gamma0_tan_half_incl * sin( big_N);
         elems->omega = atan2( e_sin_omega, e_cos_omega);
         elems->ecc = sqrt( e_sin_omega*e_sin_omega +e_cos_omega*e_cos_omega);
         }
         break;
      case TITAN:
         {
         const double Omega1_plus_dOmega = ASC_NODE0 - .1420 * PI / 180.;
         const double Incl1_plus_dIncl = INCL0 - .6303 * PI / 180.;
         const double g0 = 103.199 * PI / 180.;
         const double beta = .3752 * PI / 180.;

         double big_N = elems->Omega, g;
         double perturb_Omega, perturb_incl;

         elems->lambda += sin_gamma0_tan_half_incl * sin( big_N);
         perturb_Omega = sin_gamma0 * sin( big_N);
         elems->Omega = Omega1_plus_dOmega + perturb_Omega / sin_incl1;
         perturb_incl = sin_gamma0 * cos( big_N);
         elems->gamma = Incl1_plus_dIncl + perturb_incl;
         g = elems->omega - elems->Omega - 4.6 * PI / 180.;
         elems->ecc += beta * elems->ecc * (cos( g + g) - cos( g0 + g0));
         elems->omega += beta * elems->ecc * (sin( g + g) - sin( g0 + g0));
         }
         break;
      case HYPERION:
         {
         const double tau0 =                   92.39 * PI / 180.;
         const double tau_dot =                  .5621071 * PI / 180.;
         const double zeta0 =                 148.19 * PI / 180.;
         const double zeta_dot =              -19.18 * PI / 180.;
         const double phi0 =                  -34.7 * PI / 180.;
         const double phi_dot =               -61.7840 * PI / 180.;
         const double theta0 =                184.8 * PI / 180.;
         const double theta_dot =             -35.41 * PI / 180.;
         const double theta0_prime =          177.3 * PI / 180.;
         const double theta_dot_prime =       -35.41 * PI / 180.;
         const double C_e_zeta =                 .02303;
         const double C_e_2zeta =               -.00212;
         const double C_lam_tau =               9.142 * PI / 180.;
         const double C_lam_zeta =              -.260 * PI / 180.;
         const double C_omega_zeta =          -12.872 * PI / 180.;
         const double C_omega_2zeta =           1.668 * PI / 180.;
         const double C_a_tau =                 -.00003509;
         const double C_a_zeta_plus_tau =       -.00000067;
         const double C_a_zeta_minus_tau =       .00000071;
         const double C_e_tau =                 -.004099;
         const double C_e_3zeta =                .000151;
         const double C_e_zeta_plus_tau =       -.000167;
         const double C_e_zeta_minus_tau =       .000235;
         const double C_lam_2zeta =             -.0098 * PI / 180.;
         const double C_lam_zeta_plus_tau =      .2275 * PI / 180.;
         const double C_lam_zeta_minus_tau =     .2112 * PI / 180.;
         const double C_lam_phi =               -.0303 * PI / 180.;
         const double C_omega_tau =             -.4457 * PI / 180.;
         const double C_omega_3zeta =           -.2419 * PI / 180.;
         const double C_omega_zeta_plus_tau =   -.2657 * PI / 180.;
         const double C_omega_zeta_minus_tau =  -.3573 * PI / 180.;
         const double C_incl_theta =             .0180 * PI / 180.;
         const double C_Omega_theta_prime =      .0168 * PI / 180.;
         const double big_Nt0 =                42.02 * PI / 180.;
         const double big_Nt_dot =              -.5118 * PI / 180.;
         const double hy_gamma0 =                .6435 * PI / 180.;
         const double sin_hy_gamma0 =             .011231;

                                       /* from (45), p 59 */
         const double Omega1_plus_dOmega =    ASC_NODE0 - .747 * PI / 180.;
         const double Incl1_plus_dIncl =          INCL0 - .13 * PI / 180.;
/*       const double Omega1_plus_dOmega =    ASC_NODE0 - .0078 * PI / 180.; */
/*       const double Incl1_plus_dIncl =          INCL0 - .0455 * PI / 180.; */
         const double sin_Incl1_plus_dIncl =        0.468727;
         const double tan_half_Incl1_plus_dIncl =   0.248880;

                                       /* from (44), p 59 */
         const double big_T = (elems->jd - 2442000.5) / 365.25;
         const double t_T = (elems->jd - 2411368.0) / 365.25;
         const double big_N = elems->Omega;
         const double big_Nt = big_Nt0 + big_Nt_dot * t_T;
         const double tau = tau0 + tau_dot * t_d;
         const double zeta = zeta0 + zeta_dot * t;
         const double phi = phi0 + phi_dot * t;
         const double lambda_s = (176. + 12.22 * t) * PI / 180.;
         const double b_s = (8. + 24.44 * t) * PI / 180.;
         const double d_s = b_s + 5. * PI / 180.;

         const double theta = theta0 + theta_dot * big_T;
         const double theta_prime = theta0_prime + theta_dot_prime * big_T;
         double arg;

         elems->ecc = .103458;

         elems->gamma = sin_hy_gamma0 * cos( big_N)
                           + .315 * (PI / 180.) * cos( big_Nt)
                           - .018 * (PI / 180.) * cos( d_s)
                           + C_incl_theta * cos( theta);
         elems->gamma += Incl1_plus_dIncl;

         arg = sin( big_N);
         elems->Omega = sin_hy_gamma0 * arg
                           + .315 * (PI / 180.) * sin( big_Nt)
                           - .018 * (PI / 180.) * sin( d_s)
                           + C_Omega_theta_prime * sin( theta_prime);
         elems->Omega = Omega1_plus_dOmega
                                 + elems->Omega / sin_Incl1_plus_dIncl;
         elems->lambda += hy_gamma0 * tan_half_Incl1_plus_dIncl * arg;
         elems->omega += hy_gamma0 * tan_half_Incl1_plus_dIncl * arg;
         arg = sin( tau);
         elems->lambda += C_lam_tau * arg
                         + .007 * (PI / 180.) * sin( tau + tau)
                         - .014 * (PI / 180.) * sin( 3. * tau)
                         - .013 * (PI / 180.) * sin( lambda_s)
                         + .017 * (PI / 180.) * sin( b_s)
                         + C_lam_phi * sin( phi);
         elems->omega += C_omega_tau * arg
                      + C_omega_3zeta * sin( 3. * zeta);
         arg = sin( zeta + tau);
         elems->lambda += C_lam_zeta_plus_tau * arg;
         elems->omega += C_omega_zeta_plus_tau * arg;
         arg = sin( zeta - tau);
         elems->lambda += C_lam_zeta_minus_tau * arg;
         elems->omega += C_omega_zeta_minus_tau * arg;
         arg = sin( zeta);
         elems->lambda += C_lam_zeta * arg;
         elems->omega += C_omega_zeta * arg;
         arg = sin( zeta + zeta);
         elems->lambda += C_lam_2zeta * arg;
         elems->omega += C_omega_2zeta * arg;

         arg = cos( tau);
         elems->semimaj += C_a_tau * arg * SECONDS_TO_AU;
         elems->ecc += C_e_tau * arg;
         arg = cos( zeta + tau);
         elems->semimaj += C_a_zeta_plus_tau * arg * SECONDS_TO_AU;
         elems->ecc += C_e_zeta_plus_tau * arg;
         arg = cos( zeta - tau);
         elems->semimaj += C_a_zeta_minus_tau * arg * SECONDS_TO_AU;
         elems->ecc += C_e_zeta_minus_tau * arg
                      + C_e_zeta * cos( zeta)
                      + C_e_2zeta * cos( zeta + zeta)
                      + C_e_3zeta * cos( 3. * zeta)
                      + .00013 * cos( phi);
         }
         break;
      case JAPETUS:
         elems->gamma = JAPETUS_i0 + JAPETUS_i0_dot * t_centuries;
         elems->gamma += (-.072 + .0054 * t_centuries) * t_centuries_squared
                                 * PI / 180.;
         elems->Omega += (.116 + .008 * t_centuries) * t_centuries_squared
                                 * PI / 180.;
         elems->ecc += .001156 * t_centuries;

                              /* The following corrections are from p. 61, */
                              /* G. Dourneau,  group 50: */
         {
         const double big_T = (elems->jd - 2415020.) / 36525.;
         const double t_diff = elems->jd - 2411368.;
         const double lam_s =         (267.263 + 1222.114 * big_T) * (PI / 180.);
         const double omega_s_tilde = ( 91.796 +     .562 * big_T) * (PI / 180.);
         const double psi =           (  4.367 -     .195 * big_T) * (PI / 180.);
         const double theta =         (146.819 -    3.918 * big_T) * (PI / 180.);
         const double lam_t =         (261.319 + 22.576974 * t_diff) * (PI / 180.);
         const double omega_t_tilde = (277.102 +   .001389 * t_diff) * (PI / 180.);
         const double phi =           ( 60.470 +    1.521 * big_T) * (PI / 180.);
         const double Phi =           (205.055 -    2.091 * big_T) * (PI / 180.);

                              /* group 49: */
         const double l = elems->lambda - elems->omega;
         const double g  = elems->omega - elems->Omega - psi;
         const double g1 = elems->omega - elems->Omega - phi;
         const double ls = lam_s - omega_s_tilde;
         const double gs = omega_s_tilde - theta;
         const double lt = lam_t - omega_t_tilde;
         const double gt = omega_t_tilde - Phi;
         const double ls_plus_gs_2 = 2. * (ls + gs);
         const double ls_gs_minus_g_2 = ls_plus_gs_2 - 2. * g;
         const double lt_plus_gt = lt + gt;
         const double lt_gt_minus_g1 = lt_plus_gt - g1;


                              /* group 48: */
         const double d_a = elems->semimaj * (7.87 * cos( 2. * l - ls_gs_minus_g_2)
                                   + 98.79 * cos( l - lt_gt_minus_g1));
         const double d_e = -140.97 * cos( g1 - gt)
                             + 37.33 * cos( ls_gs_minus_g_2)
                             + 11.80 * cos( l - ls_gs_minus_g_2)
                             + 24.08 * cos( l)
                             + 28.49 * cos( l + l - lt_gt_minus_g1)
                             + 61.90 * cos( lt_gt_minus_g1);
         const double d_omega = .08077 * sin( g1 - gt)
                                + .02139 * sin( ls_gs_minus_g_2)
                                - .00676 * sin( l - ls_gs_minus_g_2)
                                + .01380 * sin( l)
                                + .01632 * sin( l + l - lt_gt_minus_g1)
                                + .03547 * sin( lt_gt_minus_g1);
         const double d_lambda = -.04299 * sin( l - lt_gt_minus_g1)
                                  -.00789 * sin( 2. * l - ls_gs_minus_g_2)
                                  -.06312 * sin( ls)
                                  -.00295 * sin( ls + ls)
                                  -.02231 * sin( ls_plus_gs_2)
                                  +.00650 * sin( ls_plus_gs_2 + phi);
         const double d_incl = .04204 * cos( ls_plus_gs_2 + phi)
                               +.00235 * cos( l + g1 + lt_plus_gt + phi)
                               +.00360 * cos( l - lt_gt_minus_g1 + phi);
         const double d_Omega = .04204 * sin( ls_plus_gs_2 + phi)
                               +.00235 * sin( l + g1 + lt_plus_gt + phi)
                               +.00358 * sin( l - lt_gt_minus_g1 + phi);

         elems->semimaj += d_a * 1.e-5;
         elems->omega += d_omega * (PI / 180.) / elems->ecc;
         elems->Omega += d_Omega * (PI / 180.) / sin( elems->gamma);
         elems->ecc += d_e * 1.e-5;
         elems->lambda += d_lambda * (PI / 180.);
         elems->gamma += d_incl * (PI / 180.);
         }
         break;
      case PHOEBE:
                              /* The elements given for Phoebe in the     */
                              /* _Explanatory Suppl_ run the 'wrong way'. */
                              /* Either the retrograde orbit confused them,  */
                              /* or they chose to swap conventions. */
         elems->lambda = 2. * elems->Omega - elems->lambda;
         elems->omega  = 2. * elems->Omega - elems->omega;
         break;
      default:
         break;
      }

   if( sat < RHEA)
      {
      elems->Omega -= ASC_NODE0;
      elems->omega -= ASC_NODE0;
      elems->lambda -= ASC_NODE0;
      }

   orbit->t = elems->jd;   // set orbit epoch equal to requested Julian Date
   orbit->mm = 0.0;        // mean motion is not actually zero but this allows position computation at requested JD.
   orbit->m = elems->lambda - elems->omega;
   orbit->m = fmod( orbit->m, TWO_PI);
   if( orbit->m > PI)
      orbit->m -= TWO_PI;
   if( orbit->m <-PI)
      orbit->m += TWO_PI;

   orbit->q = elems->semimaj * (1. - elems->ecc);
   orbit->e = elems->ecc;
   orbit->i = elems->gamma;
   orbit->w = elems->omega - elems->Omega;
   orbit->n = elems->Omega;
   return( 0);
}

void rotate_vector( double *v, const double angle, const int axis)
{
   const double sin_ang = sin( angle), cos_ang = cos( angle);
   const int a = (axis + 1) % 3, b = (axis + 2) % 3;
   const double temp = v[a] * cos_ang - v[b] * sin_ang;

   v[b] = v[b] * cos_ang + v[a] * sin_ang;
   v[a] = temp;
}

void SSMoonEphemeris::saturnMoonPosition ( int id, double jed, SSVector &pos )
{
    SAT_ELEMS elems = { 0 };
    SSOrbit orbit;
    
    if ( id == 901 )
        elems.sat_no = MIMAS;
    else if ( id == 902 )
        elems.sat_no = ENCELADUS;
    else if ( id == 903 )
        elems.sat_no = TETHYS;
    else if ( id == 904 )
        elems.sat_no = DIONE;
    else if ( id == 905 )
        elems.sat_no = RHEA;
    else if ( id == 906 )
        elems.sat_no = TITAN;
    else if ( id == 907 )
        elems.sat_no = HYPERION;
    else if ( id == 908 )
        elems.sat_no = JAPETUS;
    else if ( id == 909 )
        elems.sat_no = PHOEBE;
    
    elems.jd = jed;
    set_ssat_elems ( &elems, &orbit );
    
    SSVector vel;
    orbit.toPositionVelocity ( jed, pos, vel );
    
    elems.loc[0] = pos.x;
    elems.loc[1] = pos.y;
    elems.loc[2] = pos.z;
    
    if ( id <= 904 )    /* inner 4 satellites are returned in Saturnic */
      {                            /*  coords so gotta rotate to B1950.0 */
      rotate_vector( elems.loc, INCL0, 0);
      rotate_vector( elems.loc, ASC_NODE0, 2);
      }
                        /* After above,  elems.loc is ecliptic 1950 coords */
   rotate_vector( elems.loc, OBLIQUITY_1950, 0);
                        /* Now,  elems.loc is equatorial 1950 coords */

    pos.x = elems.loc[0];
    pos.y = elems.loc[1];
    pos.z = elems.loc[2];
    
    static SSMatrix matrix = SSCoordinates::getPrecessionMatrix ( SSTime::kB1950 ).transpose();
    pos = matrix * pos;
                            /* Now, pos is equatorial J2000... */
}
