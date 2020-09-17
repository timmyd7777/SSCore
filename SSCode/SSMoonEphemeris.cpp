// SSMoonEphemeris.cpp
// SSTest
//
// Created by Tim DeBenedictis on 4/22/20.
// Copyright © 2020 Southern Stars. All rights reserved.
//
// This code for computing outer planet moons is a C++ wrapper around
// an extended version of Bill Gray's Project Pluto code, used with permission.
// For Jupiter's Galilean moons, all of the periodic terms have been added here.

#include <string.h>

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
   orbit->mm = n[sat] - ( big_P0_dot[sat] / 100000. ) / 365.25;
   orbit->mm = degtorad ( fabs ( orbit->mm ) );
    
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

/* gust86.cpp: functions for Uranian satellite coords

Copyright (C) 2010, Project Pluto

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
02110-1301, USA.    */

/*
** gust86.cpp
** Implementation of the Lascar and Jacobson theory of the
** motion of the satellites of Uranus.  Written by Chris Marriott for
** SkyMap,  with some modifications by me (Bill J. Gray).
** Based on :
**
**  Laskar J., Jacobson, R.: 1987, GUST86 - An analytical ephemeris of the
**  Uranian satellites, Astron. Astrophys. 188, 212-224
** http://articles.adsabs.harvard.edu/cgi-bin/nph-journal_query?volume=188&plate_select=NO&page=212&plate=&cover=&journal=A%2BA..
**
** Created:   17-JUL-98
**
** $History: gust86.cpp $
**
   1 Feb 2012:  BJG:  Revised code to avoid errors on stricter compilers
such as OpenWATCOM.

   5 Nov 2008:  BJG:  Revised code to be more C-like and less C++-like,
did general cleanup & simplifying.  Now,  you can just get positions
and velocities with one relatively simple function.

** 10 Jan 2003: Bill J Gray:  changed the output from B1950 to J2000,
** by replacing the Uranicentric-to-B1950 matrix in "Position( )" with
** an Uranicentric-to-J2000 one.  (The original code is still available
** with #define ORIGINAL_B1950.)
**
**    The individual "per-satellite" functions contained a great deal
** of code identical except for coefficient values; I put that code into
** sum_uranian_series() with the coefficients passed in as arrays.
**    Most (though not all!) comments are now in English instead of French.
**
** *****************  Version 1  *****************
** User: Chris        Date: 2/10/98    Time: 6:54
** Created in $/SkyMap 4.0
** Moved from solar system DLL into main project.
**
** *****************  Version 2  *****************
** User: Chris        Date: 18/07/98   Time: 4:49p
** Updated in $/SkyMap 4.0/SolarSys
** Initial working version.
**
** *****************  Version 1  *****************
** User: Chris        Date: 17/07/98   Time: 11:59a
** Created in $/SkyMap 4.0/SolarSys
** Initial version.
*/

#define GUST86_ARIEL          0
#define GUST86_UMBRIEL        1
#define GUST86_TITANIA        2
#define GUST86_OBERON         3
#define GUST86_MIRANDA        4

#define DEGREES_TO_RADIANS (PI/180.)

static double an[5], ae[5], ai[5];         // satellite position data

//   OrbitalPosition
//   Compute basic orbital position data for the satellites.

static void gust86_mean_parameters( const double jde )
{
   static double curr_jde_set = -1.;

   if( jde != curr_jde_set)
      {
      const double t0 = 2444239.5;   // origin date for the theory: 1980 Jan 1
      const double days_since_1980 = jde - t0;             // time from origin
      const double days_per_year = 365.25;                 // days in a year
      const double years_since_1980 = days_since_1980 / days_per_year;
      int i;                           // loop counter

      static const double fqn[5] =    /* mean motion at epoch in radians/day */
         {
         4445190.550e-06, 2492952.519e-06, 1516148.111e-06,
          721718.509e-06, 466692.120e-06
         };

      static const double fqe[5] =     /* in degrees/year */
         {
         20.082, 6.217, 2.865, 2.078, 0.386
         };

      static const double fqi[5] =     /* in degrees/year */
         {
         -20.309, -6.288, -2.836, -1.843, -0.259
         };

      static const double phn[5] =     /* mean longitude at epoch in radians */
         {
         -238051.e-06, 3098046.e-06, 2285402.e-06,
           856359.e-06, -915592.e-06
         };

      static const double phe[5] =   /* in radians */
         {
         0.611392, 2.408974, 2.067774, 0.735131, 0.426767
         };

      static const double phi[5] =   /* in radians */
         {
         5.702313, 0.395757, 0.589326, 1.746237, 4.206896
         };

      // Compute the orbital data.

      for( i = 0; i < 5; i++)
         {
         an[i] = fqn[i] * days_since_1980 + phn[i];
         ae[i] = fqe[i] * DEGREES_TO_RADIANS * years_since_1980 + phe[i];
         ai[i] = fqi[i] * DEGREES_TO_RADIANS * years_since_1980 + phi[i];
         }
      }
}

static void sum_uranian_series( double *elems,
      const double *ae_series, const double *aet,
      const double *ai_series, const double *ait,
      const double *amplitudes, const double *phases, int n_extra_terms)
{
   int i;

   for( i = 2; i < 6; i++)
      elems[i] = 0;
   for( i = 5; i; i--)
      {
      elems[2] += *ae_series   * cos( *aet  );
      elems[3] += *ae_series++ * sin( *aet++);
      elems[4] += *ai_series   * cos( *ait  );
      elems[5] += *ai_series++ * sin( *ait++);
      }
   while( n_extra_terms--)
      {
      elems[2] += *amplitudes   * cos( *phases);
      elems[3] += *amplitudes++ * sin( *phases++);
      }
}

//   miranda_elems
//   Compute the orbital elements of Miranda.

static void miranda_elems( const double t, double *elems)
{
/* --- Z = K + IH  ---- */
   static const double ae_series[5] = { 1312.38e-6, 71.81e-6, 69.77e-6,
            6.75e-6, 6.27e-6 };
   static const double amplitudes[3] = {
               -123.31e-6, 39.52e-6, 194.10e-6 };
   double phases[3];
/* --- ZETA = Q + IP --- */
   static const double ai_series[5] = { 37871.71e-06, +27.01e-06, +30.76e-06,
                  +12.18e-06, +5.37e-06 };
/* --- RN => mean motion (radians/day) ---- */
   elems[0] = 4443522.67e-06
              -34.92e-06*cos(an[0]-3.e0*an[1]+2.e0*an[2])
               +8.47e-06*cos(2.*an[0]-6.*an[1]+4.*an[2])
               +1.31e-06*cos(3.*an[0]-9.*an[1]+6.*an[2])
              -52.28e-06*cos(an[0]-an[1])
             -136.65e-06*cos(2.*an[0]-2.*an[1]);
/* --- RL => mean longitude (radians) ---- */
   elems[1] =  -238051.58e-06
          +4445190.55e-06*t
            +25472.17e-06*sin(an[0]-3.*an[1]+2.*an[2])
             -3088.31e-06*sin(2.*an[0]-6.*an[1]+4.*an[2])
              -318.10e-06*sin(3.*an[0]-9.*an[1]+6.*an[2])
               -37.49e-06*sin(4.*an[0]-12.*an[1]+8.*an[2])
               -57.85e-06*sin(an[0]-an[1])
               -62.32e-06*sin(2.*an[0]-2.*an[1])
               -27.95e-06*sin(3.*an[0]-3.*an[1]);
   phases[0] = -an[0] + 2.*an[1];
   phases[1] = -2. * an[0] + 3.*an[1];
   phases[2] = an[0];

   sum_uranian_series( elems, ae_series, ae, ai_series, ai,
               amplitudes, phases, 3);
}


//   ariel_elems
//   Compute the orbital elements of Ariel.

static void ariel_elems( const double t, double *elems)
{
/* --- Z = K + IH --- */
   static const double ae_series[5] = { -3.35e-6, 1187.63e-6, 861.59e-6,
         71.50e-6, 55.59e-6 };
   static const double ai_series[5] = { -121.75e-6, 358.25e-06, 290.08e-06,
                   97.78e-06, 33.97e-06 };
   static const double amplitudes[4] = {
            -84.60e-06, +91.81e-06, +20.03e-06, +89.77e-06 };
   double phases[4];

/* --- RN => mean motion (radians/day) --- */
   elems[0] = 2492542.57e-06
                +2.55e-06*cos(an[0]-3.*an[1]+2.*an[2])
               -42.16e-06*cos(an[1]-an[2])
              -102.56e-06*cos(2.*an[1]-2.*an[2]);
/* --- RL => mean longitude (radians) --- */
   elems[1] =   3098046.41e-06
            +2492952.52e-06*t
               -1860.50e-06*sin(an[0]-3.*an[1]+2.*an[2])
                +219.99e-06*sin(2.*an[0]-6.*an[1]+4.*an[2])
                 +23.10e-06*sin(3.*an[0]-9.*an[1]+6.*an[2])
                  +4.30e-06*sin(4.*an[0]-12.*an[1]+8.*an[2])
                 -90.11e-06*sin(an[1]-an[2])
                 -91.07e-06*sin(2.*an[1]-2.*an[2])
                 -42.75e-06*sin(3.*an[1]-3.*an[2])
                 -16.49e-06*sin(2.*an[1]-2.*an[3]);
   phases[0] =  2. * an[2] - an[1];
   phases[1] =  3. * an[2] - 2. * an[1];
   phases[2] =  2. * an[3] - an[1];
   phases[3] =  an[1];

   sum_uranian_series( elems, ae_series, ae, ai_series, ai,
               amplitudes, phases, 4);
/*
*---- ZETA = Q + IP ----------------------------------------------------
*/

}


//   umbriel_elems
//   Compute the orbital elements of Umbriel.

static void umbriel_elems( const double t, double *elems)
{
/* --- Z = K + IH --- */
   static const double ae_series[5] = { -0.21e-6, -227.95e-6, 3904.69e-6,
          309.17e-6, 221.92e-6 };
   static const double ai_series[5] = { -10.86e-6, -81.51e-06, 1113.36e-06,
                  350.14e-06, 106.50e-06 };
   static const double amplitudes[11] = {
               29.34e-6, 26.20e-6, 51.19e-6, -103.86e-6, -27.16e-6,
               -16.22e-6, 549.23e-6, 34.70e-6, 12.81e-6, 21.81e-6,
               46.25e-6 };
   double phases[11];

/* --- RN => mean motion (radians/day) --- */
   elems[0] =  1515954.90e-06
                 +9.74e-06*cos(an[2]-2.*an[3]+ae[2])
               -106.00e-06*cos(an[1]-an[2])
                +54.16e-06*cos(2.*an[1]-2.*an[2])
                -23.59e-06*cos(an[2]-an[3])
                -70.70e-06*cos(2.*an[2]-2.*an[3])
                -36.28e-06*cos(3.*an[2]-3.*an[3]);
/* --- RL => mean longitude (radians) --- */
   elems[1] =  2285401.69e-06
            +1516148.11e-06*t
                +660.57e-06*sin(an[0]-3.*an[1]+2.*an[2])
                 -76.51e-06*sin(2.*an[0]-6.*an[1]+4.*an[2])
                  -8.96e-06*sin(3.*an[0]-9.*an[1]+6.*an[2])
                  -2.53e-06*sin(4.*an[0]-12.*an[1]+8.*an[2])
                 -52.91e-06*sin(an[2]-4.*an[3]+3.*an[4])
                  -7.34e-06*sin(an[2]-2.*an[3]+ae[4])
                  -1.83e-06*sin(an[2]-2.*an[3]+ae[3])
                +147.91e-06*sin(an[2]-2.*an[3]+ae[2]);

   elems[1] +=       -7.77e-06*sin(an[2]-2.*an[3]+ae[1])
                 +97.76e-06*sin(an[1]-an[2])
                 +73.13e-06*sin(2.*an[1]-2.*an[2])
                 +34.71e-06*sin(3.*an[1]-3.*an[2])
                 +18.89e-06*sin(4.*an[1]-4.*an[2])
                 -67.89e-06*sin(an[2]-an[3])
                 -82.86e-06*sin(2.*an[2]-2.*an[3]);

   elems[1] +=      -33.81e-06*sin(3.*an[2]-3.*an[3])
                 -15.79e-06*sin(4.*an[2]-4.*an[3])
                 -10.21e-06*sin(an[2]-an[4])
                 -17.08e-06*sin(2.*an[2]-2.*an[4]);

   phases[0]  = an[1];
   phases[1]  = an[2];
   phases[2]  = -an[1]+2.*an[2];
   phases[3]  = -2.*an[1]+3.*an[2];
   phases[4]  = -3.*an[1]+4.*an[2];
   phases[5]  =  an[3];
   phases[6]  = -an[2]+2.*an[3];
   phases[7]  = -2.*an[2]+3.*an[3];
   phases[8]  = -3.*an[2]+4.*an[3];
   phases[9]  = -an[2]+2.*an[4];
   phases[10] = an[2];

   sum_uranian_series( elems, ae_series, ae, ai_series, ai,
               amplitudes, phases, 11);

/*
*---- ZETA = Q + IP ----------------------------------------------------
*/
}


//   titania_elems
//   Compute the orbital elements of Titania.

static void titania_elems( const double t, double *elems)
{
   static const double ae_series[5] = { -0.02e-6, -1.29e-6, -324.51e-6,
                  932.81e-6, 1120.89e-6 };
   static const double ai_series[5] = { -1.43e-6, -1.06e-06, -140.13e-06,
                  685.72e-06, 378.32e-06 };
   static const double amplitudes[13] = {
                33.86e-6, 17.46e-6, 16.58e-6, 28.89e-6, -35.86e-6,
                -17.86e-6, -32.10e-6, -177.83e-6, 793.43e-6, 99.48e-6,
                44.83e-6, 25.13e-6, 15.43e-6 };
   double phases[13];

/* --- RN => mean motion (radians/day) --- */
   elems[0] = 721663.16e-06
                -2.64e-06*cos(an[2]-2.*an[3]+ae[2])
                -2.16e-06*cos(2.*an[3]-3.*an[4]+ae[4])
                +6.45e-06*cos(2.*an[3]-3.*an[4]+ae[3])
                -1.11e-06*cos(2.*an[3]-3.*an[4]+ae[2]);

   elems[0] +=   -62.23e-06*cos(an[1]-an[3])
               -56.13e-06*cos(an[2]-an[3])
               -39.94e-06*cos(an[3]-an[4])
               -91.85e-06*cos(2.*an[3]-2.*an[4])
               -58.31e-06*cos(3.*an[3]-3.*an[4])
               -38.60e-06*cos(4.*an[3]-4.*an[4])
               -26.18e-06*cos(5.*an[3]-5.*an[4])
               -18.06e-06*cos(6.*an[3]-6.*an[4]);
/* --- RL => mean longitude (radians) --- */
   elems[1] =  856358.79e-06
            +721718.51e-06*t
                +20.61e-06*sin(an[2]-4.*an[3]+3.*an[4])
                 -2.07e-06*sin(an[2]-2.*an[3]+ae[4])
                 -2.88e-06*sin(an[2]-2.*an[3]+ae[3])
                -40.79e-06*sin(an[2]-2.*an[3]+ae[2])
                 +2.11e-06*sin(an[2]-2.*an[3]+ae[1])
                -51.83e-06*sin(2.*an[3]-3.*an[4]+ae[4])
               +159.87e-06*sin(2.*an[3]-3.*an[4]+ae[3]);

   elems[1]  +=    -35.05e-06*sin(2.*an[3]-3.*an[4]+ae[2])
                 -1.56e-06*sin(3.*an[3]-4.*an[4]+ae[4])
                +40.54e-06*sin(an[1]-an[3])
                +46.17e-06*sin(an[2]-an[3])
               -317.76e-06*sin(an[3]-an[4])
               -305.59e-06*sin(2.*an[3]-2.*an[4])
               -148.36e-06*sin(3.*an[3]-3.*an[4])
                -82.92e-06*sin(4.*an[3]-4.*an[4]);

   elems[1]  +=    -49.98e-06*sin(5.*an[3]-5.*an[4])
                -31.56e-06*sin(6.*an[3]-6.*an[4])
                -20.56e-06*sin(7.*an[3]-7.*an[4])
                -13.69e-06*sin(8.*an[3]-8.*an[4]);
   phases[0] = an[1];
   phases[1] = an[3];
   phases[2] = -an[1]+2.*an[3];
   phases[3] = an[2];
   phases[4] = -an[2]+2.*an[3];
   phases[5] = an[3];
   phases[6] = an[4];
   phases[7] = -an[3]+2.*an[4];
   phases[8] = -2.*an[3]+3.*an[4];
   phases[9] = -3.*an[3]+4.*an[4];
   phases[10] = -4.*an[3]+5.*an[4];
   phases[11] = -5.*an[3]+6.*an[4];
   phases[12] = -6.*an[3]+7.*an[4];

   sum_uranian_series( elems, ae_series, ae, ai_series, ai,
               amplitudes, phases, 13);
/*
*---- ZETA= Q + IP ----------------------------------------------------
*/
}

//   oberon_elems
//   Compute the orbital elements of Oberon.

static void oberon_elems( const double t, double *elems)
{
   static const double ae_series[5] = { 0.00e-6, -0.35e-6, 74.53e-6,
           -758.68e-6, 1397.34e-6 };
   static const double ai_series[5] = { -0.44e-6, -0.31e-06, 36.89e-06,
                 -596.33e-06, 451.69e-06 };
   static const double amplitudes[12] = {
            39.00e-6, 17.66e-6, 32.42e-6, 79.75e-6, 75.66e-6, 134.04e-6,
            -987.26e-6, -126.09e-6, -57.42e-6, -32.41e-6, -19.99e-6, -12.94e-6 };
   double phases[12];

/* --- RN => mean motion (radians/day) --- */
   elems[0] = 466580.54e-06
                +2.08e-06*cos(2.*an[3]-3.*an[4]+ae[4])
                -6.22e-06*cos(2.*an[3]-3.*an[4]+ae[3])
                +1.07e-06*cos(2.*an[3]-3.*an[4]+ae[2])
               -43.10e-06*cos(an[1]-an[4]);

   elems[0] +=    -38.94e-06*cos(an[2]-an[4])
               -80.11e-06*cos(an[3]-an[4])
               +59.06e-06*cos(2.*an[3]-2.*an[4])
               +37.49e-06*cos(3.*an[3]-3.*an[4])
               +24.82e-06*cos(4.*an[3]-4.*an[4])
               +16.84e-06*cos(5.*an[3]-5.*an[4]);

   elems[1] =  -915591.80e-06
             +466692.12e-06*t
                  -7.82e-06*sin(an[2]-4.*an[3]+3.*an[4])
                 +51.29e-06*sin(2.*an[3]-3.*an[4]+ae[4])
                -158.24e-06*sin(2.*an[3]-3.*an[4]+ae[3])
                 +34.51e-06*sin(2.*an[3]-3.*an[4]+ae[2])
                 +47.51e-06*sin(an[1]-an[4])
                 +38.96e-06*sin(an[2]-an[4])
                +359.73e-06*sin(an[3]-an[4]);

   elems[1] +=      282.78e-06*sin(2.*an[3]-2.*an[4])
                +138.60e-06*sin(3.*an[3]-3.*an[4])
                 +78.03e-06*sin(4.*an[3]-4.*an[4])
                 +47.29e-06*sin(5.*an[3]-5.*an[4])
                 +30.00e-06*sin(6.*an[3]-6.*an[4])
                 +19.62e-06*sin(7.*an[3]-7.*an[4])
                 +13.11e-06*sin(8.*an[3]-8.*an[4]);

    phases[0] = an[1];
    phases[1] = -an[1]+2.*an[4];
    phases[2] = an[2];
    phases[3] = an[3];
    phases[4] = an[4];
    phases[5] = -an[3]+2.*an[4];
    phases[6] = -2.*an[3]+3.*an[4];
    phases[7] = -3.*an[3]+4.*an[4];
    phases[8] = -4.*an[3]+5.*an[4];
    phases[9] = -5.*an[3]+6.*an[4];
    phases[10] = -6.*an[3]+7.*an[4];
    phases[11] = -7.*an[3]+8.*an[4];

   sum_uranian_series( elems, ae_series, ae, ai_series, ai,
               amplitudes, phases, 12);

/*
*---- ZETA = Q + IP ---------------------------------------------------
*/
}

//   keplkh
//   Solve Kepler's equation.

static double keplkh( const double rl, const double rk, const double rh)
{
/*
      SUBROUTINE KEPLKH (RL,RK,RH,F,IT,IPRT)
*
*---- KEPLKH  1.0  12 DECEMBRE 1985 J. LASKAR --------------------------
*
*     RESOLUTION DE L'EQUATION DE KEPLER EN VARIABLES LONGITUDES, K, H
*
*-----------------------------------------------------------------------
*
*/
   const double eps = 1.0e-16;
   double f = 0;
   double f0, e0;
   const int itmax = 20;
   int it;

   if( rl==0.0)
      return( 0.);

   f0 = rl;
   e0 = fabs(rl);

   for( it=0; it<itmax; it++)
      {
      int k = 0;
      const double sf = sin(f0);
      const double cf = cos(f0);
      double e;
      const double ff0 = f0 - rk*sf + rh*cf - rl;
      const double fpf0 = 1.0 - rk*cf - rh*sf;
      double sdir = ff0/fpf0;
      double sdir_over_2_to_the_kth = sdir;

      do
         {
         f = f0 - sdir_over_2_to_the_kth;
         e = fabs(f - f0);
         if (e>e0)
            {
            k++;
            sdir_over_2_to_the_kth *= .5;
            }
         }
         while( e > e0);
      if (k==0 && e<=eps && ff0<=eps)
         it = itmax;       /* time to break out of loop */
      else
         {
         f0 = f;
         e0 = e;
         }
      }
   return( f);
}


//   ellipx
//   Compute rectangular coordinates from a set of orbital elements.

static void ellipx( const double ell[6], const double rmu, double xyz[6] )
{
/*
   SUBROUTINE ELLIPX (ELL,RMU,XYZ,DXYZ,IDER,IPRT)
*
*---- ELLIPX  1.1  18 March 1986  J. LASKAR -----------------------------
*
*     Calculate Cartesian coordinates (positions & velocities) and their
*     partial derivatives with respect to orbital elements,  given the
*     orbital elements as input.
*
*     ELL(6)     : Orbital elements     A: Semimajor axis
*                                       L: Mean longitude
*                                       K: ecc*COS(LONG asc node+ ARG PERI)
*                                       H: ecc*SIN(LONG asc node+ ARG PERI)
*                                       Q: SIN(Incl/2)*COS(asc node)
*                                       P: SIN(Incl/2)*SIN(asc node)
*     RMU        : Constant of gravitation for the two-body problem
*                  RMU = G*M1*(1+M2/M1) M1 Central mass
*                                       M2 Satellite mass
*     XYZ(6)     : State vector;  0..2 = position,  3..5 = velocity
*                  (The following three items were in the original code,
*                  but have been removed:)
*     DXYZ(6,7)  : Partial derivatives of the state vector with respect to
*                       the six orbital elements...
*                  DXYZ(I,J)=DRON(XYZ(I))/DRON(ELL(J))
*                       ...and with respect to the total GM:
*                  DXYZ(I,7)=DRON(XYZ(I))/DRON(RMU)
*     IDER       : 0 Get the state vector only,  or...
*                  1 Get the partial derivatives also
*     IPRT       : Print out results (not used)
*
*     Subroutine used: KEPLKH
*/
   double rot[2][3];
   double tx1[2], tx1t[2];
   double ra, rl, rk, rh, rp, rq;
   double rn, phi, psi, rki;
   int i, j;
   double f, sf, cf;
   double rlmf, umrsa, asr, rna2sr;

/*
*---- ELEMENTS UTILES --------------------------------------------------
*/
   ra=ell[0];
    rl=ell[1];
    rk=ell[2];
    rh=ell[3];
    rq=ell[4];
    rp=ell[5];
    rn=sqrt(rmu/(ra*ra*ra));
    phi=sqrt( 1.0 - rk*rk - rh*rh );
    rki = sqrt( 1.0 - rq*rq - rp*rp );
   psi = 1.0/(1.0 + phi);

/*
*---- Rotational matrix: ----------------------------------------------
*/
    rot[0][0] = 1.0 - 2*rp*rp;
   rot[1][0] = 2*rp*rq;
   rot[0][1] = 2*rp*rq;
   rot[1][1] = 1.0 - 2*rq*rq;
   rot[0][2] = -2*rp*rki;
   rot[1][2] = 2*rq*rki;

/*
*---- CALCUL DE LA LONGITUDE EXCENTRIQUE F -----------------------------
*---- F = ANOMALIE EXCENTRIQUE E + LONGITUDE DU PERIAPSE OMEGAPI -------
*/
      f = keplkh( rl, rk, rh);

      sf    =sin(f);
      cf    =cos(f);
      rlmf  =-rk*sf+rh*cf;
      umrsa =rk*cf+rh*sf;
      asr   =1.0/(1.0-umrsa);
      rna2sr=rn*ra*asr;
/*
*---- CALCUL DE TX1 ET TX1T --------------------------------------------
   tx1 = (x, y) location of satellite in the plane of its own orbit,
   tx1t = (vx, vy) in similar plane,  z = vz = 0.
*/
      tx1[0] =ra*(cf-psi*rh*rlmf-rk);
      tx1[1] =ra*(sf+psi*rk*rlmf-rh);
      tx1t[0]=rna2sr*(-sf+psi*rh*umrsa);
      tx1t[1]=rna2sr*( cf-psi*rk*umrsa);
/*
*---- CALCUL DE XYZ ----------------------------------------------------
   Now rotate from the plane of the orbit to the plane of Uranus' equator.
*/
   for (i=0; i<3; i++)
   {
         xyz[i]  =0.0;
         xyz[i+3]=0.0;

       for (j=0; j<2; j++)
       {
            xyz[i] += rot[j][i]*tx1[j];
            xyz[i+3] += rot[j][i]*tx1t[j];
       }
   }
}


//   Position
//   Compute position and velocity components for a single satellite
//   at a specified time.

void gust86_posn( const double jde, const int isat, double *r )

// Input arguments:
//   jde      Julian date, TDT
//   isat   Satellite index
//
//   Output arguments
//   r      Data array [0..2] position, [3..5] velocity components.
//         These are equatorial rectangular coordinates in km and
//         km/sec respectively, referred to epoch J2000.0.

{
   static const double gms[5] =
   {                               // GM of each of the five satellites
      4.4, 86.1, 84.0, 230.0, 200.0    // in km^3/s^2
   };
   const double AU_in_km = 149597870.0;         // 1AU in km
               // NOTE:  modern value is 149597870.7,  700 m larger
   const double t0 = 2444239.5;     // origin date for the theory = 1980 Jan 1
   const double gmsu = 5794554.5;   // Total GM of Uranus plus satellites,
                                    // in km^3/s^2
   const double gmu = gmsu - (gms[0]+gms[1]+gms[2]+gms[3]+gms[4]);
               /* Above is GM of Uranus alone,  without the satellites */
               /* Satellite GMs sum to 604.5;  gmu = 5794950.5         */
   const double rmu = gmu + gms[isat];
               /* Above is GM of Uranus plus the satellite we want */
   const double seconds_per_day = 24. * 60. * 60.;
   const double seconds_per_day_squared = seconds_per_day * seconds_per_day;
   const double days_since_1980 = jde - t0;
   double el[6], xu[6];
   int i, j;

/*---- INITIALISATIONS --------------------------------------------------*/

/*---- Test parameters: ----------------------------------------------*/

   gust86_mean_parameters( jde);
   // The function to call depends on the satellite.

   switch (isat)
   {
   case GUST86_ARIEL:
      ariel_elems( days_since_1980, el);
      break;

   case GUST86_UMBRIEL:
      umbriel_elems( days_since_1980, el);
      break;

   case GUST86_TITANIA:
      titania_elems( days_since_1980, el);
      break;

   case GUST86_OBERON:
      oberon_elems( days_since_1980, el);
      break;

   case GUST86_MIRANDA:
      miranda_elems( days_since_1980, el);
      break;

   default:       /* should never happen */
      return;
   }

         /* el[0] from the above actually gives the mean motion,  in radians
            per day.  Use Kepler's 3rd law to convert this to a semimajor
            axis in kilometers. */

   el[0] = pow( rmu*seconds_per_day_squared/(el[0] * el[0]), 1.0/3.0 );

/*---- Calculate Uranicentric XYZ coordinates (position & velocity) ----- */
   ellipx( el, rmu, xu );


   for( i=0; i<6; i++)
      r[i] = 0.0;
         /* Output is in the Uranicentric frame of reference.  Doing   */
         /* a matrix multiply by 'trans' converts to J2000.  See       */
         /* 'gust_ref.cpp' for details as to how this matrix was made. */
   for( i=0; i<3; i++)
      for( j=0; j<3; j++)
      {
       const double trans[3][3] = {
                 {  0.9753206898, -0.2207422915,  0.0047321138},
                 {  0.0619432123,  0.2529905682, -0.9654837185},
                 {  0.2119259083,  0.9419493686,  0.2604204221} };
         r[i] += trans[j][i]*xu[j];
         r[i+3] += trans[j][i]*xu[j+3];
      }

   for (i=0; i<6; i++)        /* scale output to be in AU & AU/s */
      r[i] /= AU_in_km;
}

// Returns Phobos's Mars-centric orbital elements at a given Julian Ephemeris Date (jed)
// referred to a fixed plane approximately equal to Mars's equator.
// From the Explanatory Supplement to the Astronomical Almanac, pp. 342-345.

SSOrbit phobosOrbit ( double jed )
{
    double d = jed - 2441266.5;
    double y = d / 365.25;
    double l = degtorad ( 232.41 + 1128.844556 * d + 0.00124 * y * y );
    double na = degtorad ( 47.39 - 0.0014 * y );
    double a = 6.26974e-5;
    double e = 0.0150;
    double gamma = degtorad ( 1.10 );
    double theta = mod2pi ( degtorad ( 327.90 - 0.43533 * d ) );
    double p = mod2pi ( degtorad ( 278.96 + 0.43526 * d ) - na );
    double m = mod2pi ( l - p - na );
    double mm = degtorad ( 1128.844556 - 0.43526 );
    
    return SSOrbit ( jed, a * ( 1.0 - e ), e, gamma, p - theta, theta, m, mm );
}

// Returns Deimos's Mars-cemtric orbital elements at a given Julian Ephemeris Date (jed)
// referred to a fixed plane approximately equal to Mars's equator.
// From the Explanatory Supplement to the Astronomical Almanac, pp. 342-345.

SSOrbit deimosOrbit ( double jed )
{
    double d = jed - 2441266.5;
    double y = d / 365.25;
    double h = mod2pi ( degtorad ( 196.55 - 0.01801 * d ) );
    double l = degtorad ( 28.96 + 285.161888 * d - 0.27 * sin ( h ) );
    double na = degtorad ( 46.37 - 0.0014 * y );
    double a = 1.56828e-4;
    double e = 0.0004;
    double gamma = degtorad ( 1.79 );
    double theta = mod2pi ( degtorad ( 240.38 - 0.01801 * d ) );
    double p = mod2pi ( degtorad ( 111.7 + 0.01798 * d ) - na );
    double m = mod2pi ( l - p - na );
    double mm = degtorad ( 285.161888 - 0.01798 );
    
    return SSOrbit ( jed, a * ( 1.0 - e ), e, gamma, p - theta, theta, m, mm );
}

// Returns matrix for transforming Phobos's XYZ vector to the Earth's J2000 equatorial frame.
// From the Explanatory Supplement to the Astronomical Almanac, pp. 342-345.

SSMatrix phobosMatrix ( double jed )
{
    double y = ( jed - 2441266.5 ) / 365.25;
    double na = degtorad ( 47.39 - 0.0014 * y );
    double ja = degtorad ( 37.27 + 0.0008 * y );

    return SSMatrix::rotation ( 2, 0, ja, 2, na );
}

// Returns matrix for transforming Deimos's XYZ vector to the Earth's J2000 equatorial frame.
// From the Explanatory Supplement to the Astronomical Almanac, pp. 342-345.

SSMatrix deimosMatrix ( double jed )
{
    double y = ( jed - 2441266.5 ) / 365.25;
    double na = degtorad ( 46.37 - 0.0014 * y );
    double ja = degtorad ( 36.62 + 0.0008 * y );

    return SSMatrix::rotation ( 2, 0, ja, 2, na );
}

// Returns Triton's Neptune-centric orbital elements on a given Julian Ephemeris Date (jed).
// The elements are referred to a fixed plane approximately equal to Neptune's equator.
// From the Explanatory Supplement to the Astronomical Almanac, pp. 373-375.

SSOrbit tritonOrbit ( double jed )
{
    double d = jed - 2433282.5;
    double a = 0.002368266;
    double e = 0.0;
    double gamma = degtorad ( 158.996 );
    double theta = mod2pi ( degtorad ( 151.401 + 0.57806 * d / 365.25 ) );
    double l = mod2pi ( degtorad ( 200.913 + 61.2588532 * d ) );
    double n = degtorad ( 61.2588532 - 0.57806 );
    
    return ( SSOrbit ( jed, a * ( 1.0 - e ), e, gamma, 0.0, theta, l, n ) );
}

// Returns Nereid's Neptune-centric orbital elements on a given Julian Ephemeris Date (jed).
// The elements are referred to Neptune's B1950 orbit plane.
// From the Explanatory Supplement to the Astronomical Almanac, pp. 375-377.

SSOrbit nereidOrbit ( double jed )
{
    double d = jed - 2433680.5;
    double t = d / 36525.0;
    double psi = degtorad ( fmod ( 289.2 + 2.68 * t, 360.0 ) );
    double a = 0.036868;
    double e = 0.74515;
    double gamma = degtorad ( 10.041 );
    double theta = mod2pi ( degtorad ( 329.3 - 2.4 * t + 19.7 * sin ( 2.0 * psi ) - 3.3 * sin ( 4.0 * psi ) ) );
    double p = mod2pi ( psi - degtorad ( 19.25 * sin ( 2.0 * psi ) + 3.23 * sin ( 4.0 * psi ) ) );
    double m = mod2pi ( degtorad ( 358.91 + 0.999552 * d ) );
    double mm = degtorad ( 0.999552 + 2.4 / 36525.0 );
    
    return SSOrbit ( jed, a * ( 1.0 - e ), e, gamma, p - theta, theta, m, mm );
}

// Returns matrix for transforming Triton's XYZ vector to the Earth's J2000 equatorial frame.
// From the Explanatory Supplement to the Astronomical Almanac, pp. 354-356.

SSMatrix tritonMatrix ( double jed )
{
    double t = ( jed - SSTime::kJ2000 ) / 36525.0;
    double n = degtorad ( fmod ( 359.28 + 54.308 * t, 360.0 ) );
    double ap = 298.72 + 2.58 * sin ( n ) - 0.04 * sin ( 2.0 * n );
    double dp =  42.63 - 1.90 * cos ( n ) + 0.01 * cos ( 2.0 * n );
    double je = degtorad ( 90.0 - dp );
    double ne = degtorad ( 90.0 + ap );
    
    return SSMatrix::rotation ( 2, 0, je, 2, ne );
}

// Returns matrix for transforming Nereid's XYZ vector to the Earth's J2000 equatorial frame.
// From the Explanatory Supplement to the Astronomical Almanac, pp. 354-356.

SSMatrix nereidMatrix ( void )
{
    static SSMatrix matrix = SSCoordinates::getPrecessionMatrix ( SSTime::kB1950 ).transpose()
                            * SSMatrix::rotation ( 2, 0, degtorad ( 22.313 ), 2, degtorad ( 3.522 ) );
    return matrix;
}

// Returns Charon's Pluto-centric orbital elements at a specific Julian Ephemeris Date (jed).
// The elements are referred to Pluto's equatorial plane.
// From JPL Planetary Satellite Orbital Parameters, https://ssd.jpl.nasa.gov/?sat_elem

SSOrbit charonOrbit ( double jed )
{
    double d = jed - 2451545.0;
    double y = d / 365.25;
    double a = 19591.0 / SSCoordinates::kKmPerAU;
    double e = 0.0002;
    double i = degtorad ( 0.080 );
    double w = mod2pi ( degtorad ( 146.106 - SSAngle::kTwoPi / ( 10178.040 * y ) ) );
    double n = mod2pi ( degtorad ( 26.928 + SSAngle::kTwoPi / ( 9020.398 * y ) ) );
    double m = mod2pi ( degtorad ( 131.070 + 56.3625210 * d ) );
    double mm = degtorad ( 56.3625210 );
    
    return SSOrbit ( jed, a * ( 1.0 - e ), e, i, w, n, m, mm );
}

// Returns matrix for transforming Charon's XYZ position vector to Earth's J2000 equatorial frame.
// From JPL Planetary Satellite Orbital Parameters, https://ssd.jpl.nasa.gov/?sat_elem

SSMatrix charonMatrix ( void )
{
    double a = 132.993;
    double d =  -6.613;
    double j = degtorad ( 90.0 - d );
    double n = degtorad ( 90.0 + a );
    
    return SSMatrix::rotation ( 2, 0, j, 2, n );
}

// Computes Phobos & Deimos's areo-centric position and velocity vectors, in units of AU and AU/day,
// in the fundamental J2000 mean equatorial frame, on a specified Julian Ephemeris Date (jed).
// The moon ID is 401 for Phobos, 402 for Deimons; for any other id, this method returns false.

bool SSMoonEphemeris::marsMoonPositionVelocity ( int id, double jed, SSVector &pos, SSVector &vel )
{
    SSOrbit  orbit;
    SSMatrix matrix;

    if ( id == 401 )
    {
        orbit = phobosOrbit ( jed );
        matrix = phobosMatrix ( jed );
    }
    else if ( id == 402 )
    {
        orbit = deimosOrbit ( jed );
        matrix = deimosMatrix ( jed );
    }
    else
        return false;
    
    orbit.toPositionVelocity ( jed, pos, vel );
    pos = matrix * pos;
    vel = matrix * vel;
    return true;
}

// Computes Jupiter's Galilean moons' Jupiter-centric position vector, in units of AU,
// in the fundamental J2000 mean equatorial frame, on a specified Julian Ephemeris Date (jed).
// The moon ID (id) is 501 = Io, 502 = Europa; 503 = Ganymede; 504 = Callisto;
// for any other moon ID, this method returns false.

bool jupiterMoonPosition ( int id, double jed, SSVector &pos )
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
    else
        return false;
    
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
    return true;
}

// Computes Jupiter's Galilean moons' Jupiter-centric position vector, in units of AU,
// in the fundamental J2000 mean equatorial frame, on a specified Julian Ephemeris Date (jed).
// The moon ID (id) is 501 = Io, 502 = Europa; 503 = Ganymede; 504 = Callisto; for any other moon ID,
// this method returns false. Velocity vector (vel) calculated by diffing position from one minute
// before JED to position at JED.

bool SSMoonEphemeris::jupiterMoonPositionVelocity ( int id, double jed, SSVector &pos, SSVector &vel )
{
    if ( ! jupiterMoonPosition ( id, jed, pos ) )
        return false;
    
    if ( ! jupiterMoonPosition ( id, jed - 1.0 / 1440.0, vel ) )
        return false;

    vel = ( pos - vel ) * 1440.0;
    return true;
}

// Computes Saturn's major moons' Saturn-centric position vector, in units of AU,
// in the fundamental J2000 mean equatorial frame, on a specified Julian Ephemeris Date (jed).
// The moon ID (id) is 601 = Mimas, 602 = Enceladus; 603 = Tethys; 604 = Dione, 605 = Rhea;
// 606 = Titan; 607 = Hyperion; 608 = Iapetus; 609 = Phoebe; for any other moon ID,
// this method returns false.

bool SSMoonEphemeris::saturnMoonPositionVelocity ( int id, double jed, SSVector &pos, SSVector &vel )
{
    SAT_ELEMS elems = { 0 };
    SSOrbit orbit;
    
    if ( id == 601 )
        elems.sat_no = MIMAS;
    else if ( id == 602 )
        elems.sat_no = ENCELADUS;
    else if ( id == 603 )
        elems.sat_no = TETHYS;
    else if ( id == 604 )
        elems.sat_no = DIONE;
    else if ( id == 605 )
        elems.sat_no = RHEA;
    else if ( id == 606 )
        elems.sat_no = TITAN;
    else if ( id == 607 )
        elems.sat_no = HYPERION;
    else if ( id == 608 )
        elems.sat_no = JAPETUS;
    else if ( id == 609 )
        elems.sat_no = PHOEBE;
    else
        return false;

    elems.jd = jed;
    set_ssat_elems ( &elems, &orbit );
    
    orbit.toPositionVelocity ( jed, pos, vel );
    
    double p[3] = { pos.x, pos.y, pos.z };
    double v[3] = { vel.x, vel.y, vel.z };
    
    // inner 4 satellites are returned in Saturnic
    // coords so gotta rotate to B1950.0
    
    if ( id <= 604 )
    {
        rotate_vector( p, INCL0, 0 );
        rotate_vector( p, ASC_NODE0, 2 );
        rotate_vector( v, INCL0, 0 );
        rotate_vector( v, ASC_NODE0, 2 );
    }
    
    // After above, p,v is ecliptic 1950 coords
    // Now, transform p,v to equatorial 1950 coords
    
    rotate_vector ( p, OBLIQUITY_1950, 0 );
    rotate_vector ( v, OBLIQUITY_1950, 0 );

    pos = SSVector ( p[0], p[1], p[2] );
    vel = SSVector ( v[0], v[1], v[2] );

    // Finally precess pos, vel to equatorial J2000.
    
    static SSMatrix matrix = SSCoordinates::getPrecessionMatrix ( SSTime::kB1950 ).transpose();
    pos = matrix * pos;
    vel = matrix * vel;
    
    return true;
}

// Computes Uranus's major moons' Uranocentric position and velocity vectors, in units of AU and AU/day,
// in the fundamental J2000 mean equatorial frame, on a specified Julian Ephemeris Date (jed).
// The moon ID (id) is 701 = Ariel, 702 = Umbriel; 703 = Titania; 704 = Oberon; 705 = Miranda.
// for any other moon ID, this method returns false.

bool SSMoonEphemeris::uranusMoonPositionVelocity ( int id, double jed, SSVector &pos, SSVector &vel )
{
    double rv[6] = { 0 };
    
    if ( id == 701 )
        id = GUST86_ARIEL;
    else if ( id == 702 )
        id = GUST86_UMBRIEL;
    else if ( id == 703 )
         id = GUST86_TITANIA;
    else if ( id == 704 )
        id = GUST86_OBERON;
    else if ( id == 705 )
        id = GUST86_MIRANDA;
    else
        return false;
    
    gust86_posn ( jed, id, rv );
    
    pos.x = rv[0];
    pos.y = rv[1];
    pos.z = rv[2];
    
    vel.x = rv[3] * SSTime::kSecondsPerDay;
    vel.y = rv[4] * SSTime::kSecondsPerDay;
    vel.z = rv[5] * SSTime::kSecondsPerDay;

    return true;
}

// Computes Triton & Nereid's Neptune-centric position and velocity vectors, in units of AU and AU/day,
// in the fundamental J2000 mean equatorial frame, on a specified Julian Ephemeris Date (jed).
// The moon ID is 801 for Triton, 802 for Nereid; for any other id, this method returns false.

bool SSMoonEphemeris::neptuneMoonPositionVelocity ( int id, double jed, SSVector &pos, SSVector &vel )
{
    SSOrbit  orbit;
    SSMatrix matrix;

    if ( id == 801 )
    {
        orbit = tritonOrbit ( jed );
        matrix = tritonMatrix ( jed );
    }
    else if ( id == 802 )
    {
        orbit = nereidOrbit ( jed );
        matrix = nereidMatrix();
    }
    else
        return false;
    
    orbit.toPositionVelocity ( jed, pos, vel );
    pos = matrix * pos;
    vel = matrix * vel;
    return true;
}

// Computes Charon's Pluto-centric position and velocity vectors, in units of AU and AU/day,
// in the fundamental J2000 mean equatorial frame, on a specified Julian Ephemeris Date (jed).
// The moon ID is 901; for any other id, this method returns false.

bool SSMoonEphemeris::plutoMoonPositionVelocity ( int id, double jed, SSVector &pos, SSVector &vel )
{
    SSOrbit orbit;
    static SSMatrix matrix = charonMatrix();

    if ( id == 901 )
        orbit = charonOrbit ( jed );
    else
        return false;
    
    orbit.toPositionVelocity ( jed, pos, vel );
    pos = matrix * pos;
    vel = matrix * vel;
    return true;
}
