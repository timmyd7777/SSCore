// SSVPEphemeris.cpp
// SSTest
//
// Created by Tim DeBenedictis on 4/16/20.
// Copyright © 2020 Southern Stars. All rights reserved.
//
// This is a C++ wrapper around Paul Shchlyter's C implementation of
// Van Flandern a& Pulkinnen's "Low-Precision Formulae for Planetary Positions" paper,
// found here: https://groups.google.com/forum/#!topic/sci.astro.amateur/ejhyizPyclE

#include <stddef.h>
#include <math.h>

#include "SSVPEphemeris.hpp"

/*

TRIG.H

Trigonometric and angle funtions, sign & int funct:s for Turbo C
Paul Schlyter, 1987-06-13
Macros DM and DMS added, 1989-05-08
Macros M and S changed to MIN and SEC

*/



#define PI        3.1415926535897932384
#define TWOPI     ( 2.0 * PI )
#define HALFPI    ( PI / 2.0 )
#define RADEG     ( 180.0 / PI )
#define DEGRAD    ( PI / 180.0 )
#define INV360    ( 1.0 / 360.0 )
#define ONEHALF   0.5


#define DM(d,m)    ( (d) + (m)/60.0 )
#define DMS(d,m,s) ( (d) + (m)/60.0 + (s)/3600.0 )
#define MIN(m)     ( (m)/60.0 )
#define SEC(s)     ( (s)/3600.0 )
#define S(s)       ( (s)/3600.0 )

/* Sign function  +  a better Int function */

#define sign(sgn,magn)  ( sgn==0 ? 0 : sgn>0 ? magn : -magn )


/* Trig macros for degree values */


#define sind(x)  sin((x)*DEGRAD)
#define cosd(x)  cos((x)*DEGRAD)
#define tand(x)  tan((x)*DEGRAD)

#define atand(x)    (RADEG*atan(x))
#define asind(x)    (RADEG*asin(x))
#define acosd(x)    (RADEG*acos(x))
#define atan2d(y,x) (RADEG*atan2(y,x))

/* VARV.C, normalize angle within one revoultion */


double varv180(double angle);
double varv360(double angle);


/*

VARV.C
Reduce angles within one revolution
Paul Schlyter, 1987-06-13

*/





#define INV360    ( 1.0 / 360.0 )
#define ONEHALF   0.5


double varv180 ( double x )
/*************************/
/* Reduce angle to within -180..+180 degrees */
{
return  x - 360.0 * floor( x * INV360 + ONEHALF );
}


double varv360 ( double x )
/*************************/
/* Reduce angle to within 0..360 degrees */
{
return  x - 360.0 * floor( x * INV360 );
}


/*

SOLPOSVF.C

Suns position
Referens:  van Flandern & Pulkkinen
Astrophys. J. Suppl., 41:391-411, nov 1979

Paul Schlyter, 1987-06-14

*/





double frac( double x )
/****************************/
{
return( x - floor(x) );
}  /* frac */


void solpos_vf( double JD, double *slon, double *slat, double *sr )
/*****************************************************************/
{

double dnr, t, ls, gs, lm, dm, g2, g4, g5,
       sgs, cgs, sg2, cg2, sg5, cg5, s5, c5, s2, c2, s2gs, c2gs;

dnr = JD - 2451545.0;
t = dnr/36525.0 + 1.0;

ls = 360.0 * frac( 0.779072  +  0.00273790931  * dnr );
gs = 360.0 * frac( 0.993126  +  0.00273777850  * dnr );

/* Moon */
lm = 360.0 * frac( 0.606434  +  0.03660110129 * dnr );
dm = lm - ls;

/* Venus, Mars, Jupiter: */
g2 = 360.0 * frac( 0.140023  +  0.00445036173 * dnr );
g4 = 360.0 * frac( 0.053856  +  0.00145561327 * dnr );
g5 = 360.0 * frac( 0.056531  +  0.00023080893 * dnr );

/* Suns position: */
sgs = sind(gs);
cgs = cosd(gs);
sg2 = sind(g2);
cg2 = cosd(g2);
sg5 = sind(g5);
cg5 = cosd(g5);
s5 = sgs*cg5-cgs*sg5;
c5 = cgs*cg5+sgs*sg5;
s2 = sgs*cg2-cgs*sg2;
c2 = cgs*cg2+sgs*sg2;

s2gs = 2*sgs*cgs;
c2gs = 2*cgs*cgs-1.0;

*slon = ls + (
      (6910.-17.*t) * sgs
              + 72. * s2gs
              -  7. * c5
              +  6. * sind(dm)
              + 6.4 * sind(4*gs-8*g4+3*g5+38.9)
              -  5. * (2*c2*c2-1.0)
              -  4. * s2
              +  3. * 2*s2*c2
              -  3. * sg5
              -  3. * 2*s5*c5
             ) / 3600.0;

*sr =  1.00014  -  0.01675 * cgs  -  0.00014 * c2gs;

if ( slat != NULL )
    *slat = 0.0;

}  /* solpos_vf */



/*

MONPOSVF.C

Moons position
Referens:  van Flandern & Pulkkinen
Astrophys. J. Suppl., 41:391-411, nov 1979

Paul Schlyter, 1987-06-14

*/







void monpos_vf( double JD, double *lon, double *lat, double *r )
/****************************************************************/
{

double dnr, t, ls, gs, lm, gm, fm, nm, dm, venarg, /* r, */
       sgs, cgs, sgm, cgm, sfm, cfm, sdm, cdm, snm, cnm, s2gs, c2gs,
       s2gm, c2gm, s3gm, c3gm, s4gm, /* c4gm, */
       s2fm, c2fm, s3fm, c3fm,
       s2dm, c2dm, s3dm, c3dm, s4dm, c4dm,
       sgmpgs, cgmpgs, sgmmgs, cgmmgs, sgmp2d, cgmp2d, sgmm2g,
       cgmm2g, s2mmgs, c2mmgs, s2fm2d, c2fm2d, sgmm2d, cgmm2d,
       s2gm2d, c2gm2d, sfmp2d, cfmp2d, sfmm2d, cfmm2d,
       s2gpgs, c2gpgs, s2gp2d, c2gp2d,
       sfmp4d, cfmp4d, sfmm4d, cfmm4d;

dnr = JD - 2451545.0;
t = dnr / 36525.0 + 1.0;

/* Sun: */
ls = 360.0 * frac( 0.779072  +  0.00273790931 * dnr );
gs = 360.0 * frac( 0.993126  +  0.00273777850 * dnr );

/* Moon: */
lm = 360.0 * frac( 0.606434  +  0.03660110129  * dnr );
gm = 360.0 * frac( 0.374897  +  0.03629164709  * dnr );
fm = 360.0 * frac( 0.259091  +  0.03674819520  * dnr );
nm = lm - fm;
dm = lm - ls;

/* Planetary perturbation,
   argument:  gm + 16*ls - 18*l2, l2 = Venus mean long. */
venarg = 360.0 * frac( 0.741085 - 1.024001E-05 * dnr );

sgs = sind(gs);
cgs = cosd(gs);

sgm = sind(gm);
cgm = cosd(gm);
sfm = sind(fm);
cfm = cosd(fm);
sdm = sind(dm);
cdm = cosd(dm);
snm = sind(nm);
cnm = cosd(nm);

s2gs = 2*sgs*cgs;
c2gs = 2*cgs*cgs-1.0;

s2gm = 2*sgm*cgm;
c2gm = 2*cgm*cgm-1.0;
s3gm = s2gm*cgm+c2gm*sgm;
c3gm = c2gm*cgm-s2gm*sgm;
s4gm = 2*s2gm*c2gm;
/* c4gm = 2*c2gm*c2gm-1.0; */

s2fm = 2*sfm*cfm;
c2fm = 2*cfm*cfm-1.0;
s3fm = s2fm*cfm+c2fm*sfm;
c3fm = c2fm*cfm-s2fm*sfm;

s2dm = 2*sdm*cdm;
c2dm = 2*cdm*cdm-1.0;
s3dm = s2dm*cdm+c2dm*sdm;
c3dm = c2dm*cdm-s2dm*sdm;
s4dm = 2*s2dm*c2dm;
c4dm = 2*c2dm*c2dm-1.0;

sgmpgs = sgm*cgs+cgm*sgs;
cgmpgs = cgm*cgs-sgm*sgs;
sgmmgs = sgm*cgs-cgm*sgs;
cgmmgs = cgm*cgs+sgm*sgs;
sgmp2d = sgm*c2dm+cgm*s2dm;
cgmp2d = cgm*c2dm-sgm*s2dm;
sgmm2g = sgm*c2gs-cgm*s2gs;
cgmm2g = cgm*c2gs+sgm*s2gs;
s2mmgs = s2gm*cgs-c2gm*sgs;
c2mmgs = c2gm*cgs+s2gm*sgs;
s2fm2d = s2fm*c2dm-c2fm*s2dm;
c2fm2d = c2fm*c2dm+s2fm*s2dm;
sgmm2d = sgm*c2dm-cgm*s2dm;
cgmm2d = cgm*c2dm+sgm*s2dm;
s2gm2d = s2gm*c2dm-c2gm*s2dm;
c2gm2d = c2gm*c2dm+s2gm*s2dm;
sfmp2d = sfm*c2dm+cfm*s2dm;
cfmp2d = cfm*c2dm-sfm*s2dm;
sfmm2d = sfm*c2dm-cfm*s2dm;
cfmm2d = cfm*c2dm+sfm*s2dm;

s2gpgs = s2gm*cgs+c2gm*sgs;
c2gpgs = c2gm*cgs-s2gm*sgs;
s2gp2d = s2gm*c2dm+c2gm*s2dm;
c2gp2d = c2gm*c2dm-s2gm*s2dm;

sfmp4d = sfm*c4dm+cfm*s4dm;
cfmp4d = cfm*c4dm-sfm*s4dm;
sfmm4d = sfm*c4dm-cfm*s4dm;
cfmm4d = cfm*c4dm+sfm*s4dm;

*lon = lm + (
         22640. * sgm
        - 4586. * sgmm2d
        + 2370. * s2dm
        +  769. * s2gm
        -  668. * sgs
        -  412. * s2fm
        -  212. * s2gm2d
        -  206. * (sgmpgs*c2dm-cgmpgs*s2dm)
        +  192. * sgmp2d
        +  165. * (s2dm*cgs-c2dm*sgs)
        +  148. * sgmmgs
        -  125. * sdm
        -  110. * sgmpgs
        -   55. * s2fm2d
        -   45. * (sgm*c2fm+cgm*s2fm)
        +   40. * (sgm*c2fm-cgm*s2fm)
        -   38. * (sgm*c4dm-cgm*s4dm)
        +   36. * s3gm
        -   31. * (s2gm*c4dm-c2gm*s4dm)
        +   28. * (sgmmgs*c2dm-cgmmgs*s2dm)
        -   24. * (s2dm*cgs+c2dm*sgs)
        +   19. * (sgm*cdm-cgm*sdm)
        +   18. * (sdm*cgs+cdm*sgs)
        +   15. * (sgmmgs*c2dm+cgmmgs*s2dm)
        +   14. * (s2gm*c2dm+c2gm*s2dm)
        +   14. * s4dm
        -   13. * (s3gm*c2dm-c3gm*s2dm)
+ (14.2+0.55*t) * sind(venarg+140.72-22.43*t)
        +   10. * (s2gm*cgs-c2gm*sgs)
        +    9. * (sgmm2d*c2fm-cgmm2d*s2fm)
        -    9. * (s2gm2d*cgs+c2gm2d*sgs)
        -    8. * (sgm*cdm+cgm*sdm)
        +    8. * (s2dm*c2gs-c2dm*s2gs)
        -    8. * (s2gm*cgs+c2gm*sgs)
        -    7. * s2gs
        -    7. * (sgmm2d*c2gs+cgmm2d*s2gs)
        +    7. * snm
        -    6. * (sgmp2d*c2fm-cgmp2d*s2fm)
        -    6. * (s2fm*c2dm+c2fm*s2dm)
        -    4. * (sgmpgs*c4dm-cgmpgs*s4dm)
        -    4. * (s2gm*c2fm+c2gm*s2fm)
        +    3. * (sgm*c3dm-cgm*s3dm)
        -    3. * (sgmpgs*c2dm+cgmpgs*s2dm)
        -    3. * (s2gpgs*c4dm-c2gpgs*s4dm)
        +    3. * sgmm2g
        +    3. * (sgmm2g*c2dm-cgmm2g*s2dm)
        -    2. * (s2mmgs*c2dm-c2mmgs*s2dm)
        -    2. * (s2fm2d*cgs+c2fm2d*sgs)
        +    2. * (sgm*c4dm+cgm*s4dm)
        +    2. * s4gm
        +    2. * (s4dm*cgs-c4dm*sgs)
        +    2. * (s2gm*cdm-c2gm*sdm)
           ) / 3600.0;

*lat =  (
         18461. * sfm
        + 1010. * (sgm*cfm+cgm*sfm)
        + 1000. * (sgm*cfm-cgm*sfm)
        -  624. * (sfm*c2dm-cfm*s2dm)
        -  199. * (sgmm2d*cfm-cgmm2d*sfm)
        -  167. * (sgmm2d*cfm+cgmm2d*sfm)
        +  117. * (sfm*c2dm+cfm*s2dm)
        +   62. * (s2gm*cfm+c2gm*sfm)
        +   33. * (sgmp2d*cfm-cgmp2d*sfm)
        +   32. * (s2gm*cfm-c2gm*sfm)
        -   30. * (sfmm2d*cgs+cfmm2d*sgs)
        -   16. * (s2gm*cfmm2d+c2gm*sfmm2d)
        +   15. * (sgm*cfmp2d+cgm*sfmp2d)
        +   12. * (sfmm2d*cgs-cfmm2d*sgs)
        -    9. * (sgmpgs*cfmp2d-cgmpgs*sfmp2d)
        -    8. * (sfm*cnm+cfm*snm)
        +    8. * (sfmp2d*cgs-cfmp2d*sgs)
        -    7. * (sgmpgs*cfmm2d+cgmpgs*sfmm2d)
        +    7. * (sgmmgs*cfm+cgmmgs*sfm)
        -    7. * (sgm*cfmm4d+cgm*sfmm4d)
        -    6. * (sfm*cgs+cfm*sgs)
        -    6. * s3fm
        +    6. * (sgmmgs*cfm-cgmmgs*sfm)
        -    5. * (sfm*cdm+cfm*sdm)
        -    5. * (sgmpgs*cfm+cgmpgs*sfm)
        -    5. * (sgmpgs*cfm-cgmpgs*sfm)
        +    5. * (sfm*cgs-cfm*sgs)
        +    5. * (sfm*cdm-cfm*sdm)
        +    4. * (s3gm*cfm+c3gm*sfm)
        -    4. * sfmm4d
        -    3. * (sgm*cfmp4d-cgm*sfmp4d)
        +    3. * (sgm*c3fm-cgm*s3fm)
        -    2. * (s2gm*cfmp4d-c2gm*sfmp4d)
        -    2. * (s3fm*c2dm-c3fm*s2dm)
        +    2. * (s2gp2d*cfm-c2gp2d*sfm)
        +    2. * (sgmmgs*cfmm2d-cgmmgs*sfmm2d)
        +    2. * (s2gm*cfmp2d-c2gm*sfmp2d)
        +    2. * (s3gm*cfm-c3gm*sfm)
           ) / 3600.0;

*r =  60.36298
       - 3.27746 * cgm
       - 0.57994 * cgmm2d
       - 0.46357 * c2dm
       - 0.08904 * c2gm
       + 0.03865 * c2gm2d
       - 0.03237 * (c2dm*cgs+s2dm*sgs)
       - 0.02688 * (cgm*c2dm-sgm*s2dm)
       - 0.02358 * (cgmm2d*cgs-sgmm2d*sgs)
       - 0.02030 * cgmmgs
       + 0.01719 * cdm
       + 0.01671 * cgmpgs
       + 0.01247 * (cgm*c2fm+sgm*s2fm)
       + 0.00704 * cgs
       + 0.00529 * (c2dm*cgs-s2dm*sgs)
       - 0.00524 * (cgm*c4dm+sgm*s4dm)
       + 0.00398 * (cgmmgs*c2dm+sgmmgs*s2dm)
       - 0.00366 * c3gm
       - 0.00295 * (c2gm*c4dm+s2gm*s4dm)
       - 0.00263 * (cdm*cgs-sdm*sgs)
       + 0.00249 * (c3gm*c2dm+s3gm*s2dm)
       - 0.00221 * (cgmmgs*c2dm-sgmmgs*s2dm)
       + 0.00185 * (c2fm*c2dm+s2fm*s2dm)
       - 0.00161 * (c2dm*c2gs+s2dm*s2gs)
       + 0.00147 * (cgmm2d*c2fm-sgmm2d*s2fm)
       - 0.00142 * c4dm
       + 0.00139 * (c2gm2d*cgs-s2gm2d*sgs)
       - 0.00118 * (cgmpgs*c4dm+sgmpgs*s4dm)
       - 0.00116 * (c2gm*c2dm-s2gm*s2dm)
       - 0.00110 * (c2gm*cgs+s2gm*sgs)   ;

/* *par = asind( 1.0 / r ); */

}  /* monpos_vf */



/*

VF_MER.C - Mercurys position

Referens:  van Flandern & Pulkkinen
Astrophys. J. Suppl., 41:391-411, nov 1979

Paul Schlyter, 1987-06-14

*/




void merpos_vf( double JD, double *lon, double *lat, double *r )
/**************************************************************/
{
double dnr, t,
       l1, g1, f1, sg1, cg1, sf1, cf1, s2f1, c2f1, s3f1, c3f1,
       s2g1, c2g1, s3g1, c3g1, s4g1, c4g1, s5g1, c5g1,
       g2;


dnr = JD - 2451545.0;
t = dnr / 36525.0 + 1.0;


/* Venus mean anomaly: */
g2 = 360.0 * frac( 0.140023  +  0.00445036173 * dnr );

/* Mercurys heliocentric position: */

l1 = 360.0 * frac( 0.700695  +  0.01136771400 * dnr );
g1 = 360.0 * frac( 0.485541  +  0.01136759566 * dnr );
f1 = 360.0 * frac( 0.566441  +  0.01136762384 * dnr );

sg1 = sind(g1);
cg1 = cosd(g1);
sf1 = sind(f1);
cf1 = cosd(f1);

s2g1 = 2*sg1*cg1;
c2g1 = 2*cg1*cg1-1.0;
s3g1 = s2g1*cg1+c2g1*sg1;
c3g1 = c2g1*cg1-s2g1*sg1;
s4g1 = 2*s2g1*c2g1;
c4g1 = 2*c2g1*c2g1-1.0;
s5g1 = s4g1*cg1+c4g1*sg1;
c5g1 = c4g1*cg1-s4g1*sg1;

s2f1 = 2*sf1*cf1;
c2f1 = 2*cf1*cf1-1.0;
s3f1 = s2f1*cf1+c2f1*sf1;
c3f1 = c2f1*cf1-s2f1*sf1;

*lon = l1 + (
   (84378.+8.*t) * sg1
         +10733. * s2g1
         + 1892. * s3g1
         -  646. * s2f1
         +  381. * s4g1
         -  306. * (sg1*c2f1-cg1*s2f1)
         -  274. * (sg1*c2f1+cg1*s2f1)
         -   92. * (s2g1*c2f1+c2g1*s2f1)
         +   83. * s5g1
         -   28. * (s3g1*c2f1+c3g1*s2f1)
         +   25. * (s2g1*c2f1-c2g1*s2f1)
         +   19. * (2*s3g1*c3g1)
         -    9. * (s4g1*c2f1+c4g1*s2f1)
         +    7. * cosd(2*g1-5*g2)
            ) / 3600.0;

*lat = (   24134. * sf1
         +  5180. * (sg1*cf1-cg1*sf1)
         +  4910. * (sg1*cf1+cg1*sf1)
         +  1124. * (s2g1*cf1+c2g1*sf1)
         +   271. * (s3g1*cf1+c3g1*sf1)
         +   132. * (s2g1*cf1-c2g1*sf1)
         +    67. * (s4g1*cf1+c4g1*sf1)
         +    18. * (s3g1*cf1-c3g1*sf1)
         +    17. * (s5g1*cf1+c5g1*sf1)
         -    10. * s3f1
         -     9. * (sg1*c3f1-cg1*s3f1)
       ) / 3600.0;

*r = 0.39528
         - 0.07834 * cg1
         - 0.00795 * c2g1
         - 0.00121 * c3g1
         - 0.00022 * c4g1;

}  /* merpos_vf */




/*

VF_VEN.C - Venus position

Referens:  van Flandern & Pulkkinen
Astrophys. J. Suppl., 41:391-411, nov 1979

Paul Schlyter, 1987-06-14

*/




void venpos_vf( double JD, double *lon, double *lat, double *r )
/**************************************************************/
{
double dnr, t, gs, sgs, cgs,
       l2, g2, f2, sg2, cg2, sf2, cf2, s2g2, c2g2, s3g2, c3g2,
       s2gs, c2gs, s3gs, c3gs;


dnr = JD - 2451545.0;
t = dnr / 36525.0 + 1.0;


/* Fundamental arguments Sun: */
gs = 360.0 * frac( 0.993126  +  0.00273777850 * dnr );
sgs = sind(gs);
cgs = cosd(gs);

/* Venus mean anomaly: */
g2 = 360.0 * frac( 0.140023  +  0.00445036173 * dnr );

/* Venus heliocentric position: */

l2 = 360.0 * frac( 0.505498  +  0.00445046867 * dnr );
g2 = 360.0 * frac( 0.140023  +  0.00445036173 * dnr );
f2 = 360.0 * frac( 0.292498  +  0.00445040017 * dnr );

sg2 = sind(g2);
cg2 = cosd(g2);
sf2 = sind(f2);
cf2 = cosd(f2);

s2g2 = 2*sg2*cg2;
c2g2 = 2*cg2*cg2-1.0;
s3g2 = s2g2*cg2+c2g2*sg2;
c3g2 = c2g2*cg2-s2g2*sg2;

s2gs = 2*sgs*cgs;
c2gs = 2*cgs*cgs-1.0;
s3gs = s2gs*cgs+c2gs*sgs;
c3gs = c2gs*cgs-s2gs*sgs;

*lon = l2 + (
   (2814.-20.*t) * sg2
          - 181. * (2*sf2*cf2)
          +  12. * s2g2
          -  10. * (c2gs*c2g2+s2gs*s2g2)
          +   7. * (c3gs*c3g2+s3gs*s3g2)
            ) / 3600.0;

*lat = (  12215. * sf2
           + 83. * (sg2*cf2+cg2*sf2)
           + 83. * (sg2*cf2-cg2*sf2)
       ) / 3600.0;

*r = 0.72335  -  0.00493 * cg2;

}  /* venpos_vf */




/*

VF_MAR.C - Mars position

Referens:  van Flandern & Pulkkinen
Astrophys. J. Suppl., 41:391-411, nov 1979

Paul Schlyter, 1987-06-14

*/




void marpos_vf( double JD, double *lon, double *lat, double *r )
/**************************************************************/
{
double dnr, t, gs, sgs, cgs, g5,
       g2, sg2, cg2,
       s2gs, c2gs,
       l4, g4, f4, sg4, cg4, sf4, cf4, sg5, cg5,
       s2g4, c2g4, s3g4, c3g4, s4g4, c4g4, s2f4, c2f4, s2g5, c2g5;


dnr = JD - 2451545.0;
t = dnr / 36525.0 + 1.0;


/* Fundamental arguments Sun: */
gs = 360.0 * frac( 0.993126  +  0.00273777850 * dnr );
sgs = sind(gs);
cgs = cosd(gs);

/* Frequently used perturbation arguments: */
/* Venus: */
g2 = 360.0 * frac( 0.140023  +  0.00445036173 * dnr );
/* Mars: */
g4 = 360.0 * frac( 0.053856  +  0.00145561327 * dnr );
/* Jupiter: */
g5 = 360.0 * frac( 0.056531  +  0.00023080893 * dnr );

/* Mars heliocentric position: */

l4 = 360.0 * frac( 0.987353  +  0.00145575328 * dnr );
g4 = 360.0 * frac( 0.053856  +  0.00145561327 * dnr );
f4 = 360.0 * frac( 0.849694  +  0.00145569465 * dnr );

sg4 = sind(g4);
cg4 = cosd(g4);
sf4 = sind(f4);
cf4 = cosd(f4);
sg2 = sind(g2);
cg2 = cosd(g2);
sg5 = sind(g5);
cg5 = cosd(g5);

s2gs = 2*sgs*cgs;
c2gs = 2*cgs*cgs-1.0;

s2g4 = 2*sg4*cg4;
c2g4 = 2*cg4*cg4-1.0;
s3g4 = s2g4*cg4+c2g4*sg4;
c3g4 = c2g4*cg4-s2g4*sg4;
s4g4 = 2*s2g4*c2g4;
c4g4 = 2*c2g4*c2g4-1.0;
s2f4 = 2*sf4*cf4;
c2f4 = 2*cf4*cf4-1.0;
s2g5 = 2*sg5*cg5;
c2g5 = 2*cg5*cg5-1.0;

*lon = l4 + (
  (38451.+37.*t) * sg4
  + (2238.+4.*t) * s2g4
          + 181. * s3g4
          -  52. * s2f4
          -  22. * (cg4*c2g5+sg4*s2g5)
          -  19. * (sg4*cg5-cg4*sg5)
          +  17. * (cg4*cg5+sg4*sg5)
          +  17. * s4g4
          -  16. * (c2g4*c2g5+s2g4*s2g5)
          +  13. * (cgs*c2g4+sgs*s2g4)
          -  10. * (sg4*c2f4-cg4*s2f4)
          -  10. * (sg4*c2f4+cg4*s2f4)
          +   7. * (cgs*cg4+sgs*sg4)
          -   7. * (c2gs*c3g4+s2gs*s3g4)
          -   5. * (sg2*c3g4-cg2*s3g4)
          -   5. * (sgs*cg4-cgs*sg4)
          -   5. * (sgs*c2g4-cgs*s2g4)
          -   4. * (c2gs*c4g4+s2gs*s4g4)
          +   4. * cg5
          +   3. * (cg2*c3g4+sg2*s3g4)
          +   3. * (s2g4*c2g5-c2g4*s2g5)
            ) / 3600.0;

*lat = (  6603. * sf4
         + 622. * (sg4*cf4-cg4*sf4)
         + 615. * (sg4*cf4+cg4*sf4)
         +  64. * (s2g4*cf4+c2g4*sf4)
       ) / 3600.0;

*r =    1.53031
     -  0.14170 * cg4
     -  0.00660 * c2g4
     -  0.00047 * c3g4;

}  /* marpos_vf */




/*

VF_JUP.C - Jupiters position

Referens: van Flandern & Pulkkinen
Astrophys. J. Suppl., 41:391-411, nov 1979

Paul Schlyter, 1987-06-14

*/







void juppos_vf( double JD, double *lon, double *lat, double *r )
/**************************************************************/
{

double dnr, t, g5, g6, g7,
       sg5, cg5, sg6, cg6, sg7, cg7,
       s2g5, c2g5, s3g5, c3g5, s4g5, c4g5, s5g5, c5g5,
       s2g6, c2g6, s3g6, c3g6, s4g6, c4g6, s5g6, c5g6,
       s6g6, c6g6, s10g6, c10g6, /* s7g6, c7g6, s9g6, c9g6, s11g6, c11g6, */
       s2g7, c2g7, s3g7, c3g7,
       /*s2gm4g,*/ c2gm4g, s2gm5g, c2gm5g, s2gm6g, c2gm6g,
       l5, sl5, cl5, s2l5, c2l5;


dnr = JD - 2451545.0;
t = dnr/36525.+1.0;

g5 = 360.0 * frac( 0.056531 + 0.00023080893 * dnr );
g6 = 360.0 * frac( 0.882987 + 0.00009294371 * dnr );
g7 = 360.0 * frac( 0.400589 + 0.00003269438 * dnr );

sg5 = sind(g5);
cg5 = cosd(g5);
sg6 = sind(g6);
cg6 = cosd(g6);
sg7 = sind(g7);
cg7 = cosd(g7);

s2g5 = 2*sg5*cg5;
c2g5 = 2*cg5*cg5-1.0;
s3g5 = s2g5*cg5+c2g5*sg5;
c3g5 = c2g5*cg5-s2g5*sg5;
s4g5 = 2*s2g5*c2g5;
c4g5 = 2*c2g5*c2g5-1.0;
s5g5 = s4g5*cg5+c4g5*sg5;
c5g5 = c4g5*cg5-s4g5*sg5;

s2g6 = 2*sg6*cg6;
c2g6 = 2*cg6*cg6-1.0;
s3g6 = s2g6*cg6+c2g6*sg6;
c3g6 = c2g6*cg6-s2g6*sg6;
s4g6 = 2*s2g6*c2g6;
c4g6 = 2*c2g6*c2g6-1.0;
s5g6 = s4g6*cg6+c4g6*sg6;
c5g6 = c4g6*cg6-s4g6*sg6;
s6g6 = 2*s3g6*c3g6;
c6g6 = 2*c3g6*c3g6-1.0;
/* s7g6 = s6g6*cg6+c6g6*sg6; */
/* c7g6 = c6g6*cg6-s6g6*sg6; */
/* s9g6 = s7g6*c2g6+c7g6*s2g6; */
/* c9g6 = c7g6*c2g6-s7g6*s2g6; */
s10g6 = 2*s5g6*c5g6;
c10g6 = 2*c5g6*c5g6-1.0;
/* s11g6 = s10g6*cg6+c10g6*sg6; */
/* c11g6 = c10g6*cg6-s10g6*sg6; */

s2g7 = 2*sg7*cg7;
c2g7 = 2*cg7*cg7-1.0;
s3g7 = s2g7*cg7+c2g7*sg7;
c3g7 = c2g7*cg7-s2g7*sg7;
/* s2gm4g = s2g5*c4g6-c2g5*s4g6; */
c2gm4g = c2g5*c4g6+s2g5*s4g6;
s2gm5g = s2g5*c5g6-c2g5*s5g6;
c2gm5g = c2g5*c5g6+s2g5*s5g6;
s2gm6g = s2g5*c6g6-c2g5*s6g6;
c2gm6g = c2g5*c6g6+s2g5*s6g6;


l5 =  varv180(g5 + 11.9077);

sl5 = sind(l5);
cl5 = cosd(l5);

s2l5 = 2*sl5*cl5;
c2l5 = 2*cl5*cl5-1.0;

*lon = l5 + (
    (19934.+68.*t) * sg5
   +         5023. * t
   +         2511.
   + (1093.-19.*t) * c2gm5g
   +   (601.+3.*t) * s2g5
   -  (479.+43.*t) * s2gm5g
   -          185. * (s2g5*c2g6-c2g5*s2g6)
   +   (137.-2.*t) * (s3g5*c5g6-c3g5*s5g6)
   -          131. * (sg5*c2g6-cg5*s2g6)
   +           79. * (cg5*cg6+sg5*sg6)
   -           76. * (c2g5*c2g6+s2g5*s2g6)
   -   (37.+74.*t) * cg5
   +           66. * (c2g5*c3g6+s2g5*s3g6)
   +           63. * (c3g5*c5g6+s3g5*s5g6)
   +           53. * (cg5*c5g6+sg5*s5g6)
   +           49. * (s2g5*c3g6-c2g5*s3g6)
   +           25. * s2l5
   +           25. * s3g5
   -    (23.+2.*t) * (sg5*c5g6-cg5*s5g6)
   +           17. * c2gm4g
   +           17. * (c3g5*c3g6+s3g5*s3g6)
   -           14. * (sg5*cg6-cg5*sg6)
   -           13. * (s3g5*c4g6-c3g5*s4g6)
   -            9. * c2l5
   +            9. * cg6
   -            9. * sg6
   -            9. * (s3g5*c2g6-c3g5*s2g6)
   +            9. * (s4g5*c5g6-c4g5*s5g6)
   +            9. * (s2gm6g*c3g7+c2gm6g*s3g7)
   -            8. * (c4g5*c10g6+s4g5*s10g6)
   +            7. * (c3g5*c4g6+s3g5*s4g6)
   -            7. * (cg5*c3g6+sg5*s3g6)
   -            7. * (s4g5*c10g6-c4g5*s10g6)
   -            7. * (sg5*c3g6-cg5*s3g6)
   +            6. * (c4g5*c5g6+s4g5*s5g6)
   -            6. * (s3g5*c3g6-c3g5*s3g6)
   +            5. * c2g6
   -            4. * (s4g5*c4g6-c4g5*s4g6)
   -            4. * c3g6
   +            4. * (c2g5*cg6+s2g5*sg6)
   -            4. * (c3g5*c2g6+s3g5*s2g6)
   -          4.*t * c2g5
   +            3. * c5g6
   +            3. * (c5g5*c10g6+s5g5*s10g6)
   +            3. * s2g6
   -            2. * (s2l5*cg5-c2l5*sg5)
   +            2. * (s2l5*cg5+c2l5*sg5)
                   ) / 3600.0;

*lat = (
    (-4692.+21.*t) * cg5
   +  (259.+30.*t) * sg5
   +          227.
   -          227. * c2g5
   +           16. * (s3g5*c5g6-c3g5*s5g6)
   -           13. * (sg5*c5g6-cg5*s5g6)
   -           12. * c3g5
   +           12. * s2g5
   +            7. * (c3g5*c5g6+s3g5*s5g6)
   -            5. * (cg5*c5g6+sg5*s5g6)
      ) / 3600.0;

*r =             5.20883
   - (0.25122+0.00084*t) * cg5
   -             0.00604 * c2g5
   +             0.00260 * (c2g5*c2g6+s2g5*s2g6)
   -             0.00170 * (c3g5*c5g6+s3g5*s5g6)
   -             0.00106 * (s2g5*c2g6-c2g5*s2g6)
   - (0.00046+0.00091*t) * sg5
   +             0.00069 * (s2g5*c3g6-c2g5*s3g6)
   -             0.00067 * (sg5*c5g6-cg5*s5g6)
   +             0.00066 * (s3g5*c5g6-c3g5*s5g6)
   +             0.00063 * (sg5*cg6-cg5*sg6)
   -             0.00051 * (c2g5*c3g6+s2g5*s3g6)
   -             0.00029 * (cg5*c5g6+sg5*s5g6)
   +             0.00027 * (cg5*c2g6+sg5*s2g6)
   -             0.00022 * c3g5
   -             0.00021 * s2gm5g;


}  /* juppos_vf */










void satpos_vf( double JD, double *lon, double *lat, double *r )
/**************************************************************/
{

double dnr, t, g5, g6, g7,
       sg5, cg5, sg6, cg6, sg7, cg7,
       s2g5, c2g5, s3g5, c3g5, s4g5, c4g5, /* s5g5, c5g5, */
       s2g6, c2g6, s3g6, c3g6, s4g6, c4g6, s5g6, c5g6,
       s6g6, c6g6, s7g6, c7g6, s9g6, c9g6, s10g6, c10g6, s11g6, c11g6,
       s2g7, c2g7, s3g7, c3g7,
       s2gm4g, c2gm4g, s2gm5g, c2gm5g, s2gm6g, c2gm6g,
       l6, sl6, cl6, s2l6, c2l6, s2gm7g, c2gm7g,
       lona, lonb;


dnr = JD - 2451545.0;
t = dnr/36525.+1.0;

g5 = 360.0 * frac( 0.056531 + 0.00023080893 * dnr );
g6 = 360.0 * frac( 0.882987 + 0.00009294371 * dnr );
g7 = 360.0 * frac( 0.400589 + 0.00003269438 * dnr );

sg5 = sind(g5);
cg5 = cosd(g5);
sg6 = sind(g6);
cg6 = cosd(g6);
sg7 = sind(g7);
cg7 = cosd(g7);

s2g5 = 2*sg5*cg5;
c2g5 = 2*cg5*cg5-1.0;
s3g5 = s2g5*cg5+c2g5*sg5;
c3g5 = c2g5*cg5-s2g5*sg5;
s4g5 = 2*s2g5*c2g5;
c4g5 = 2*c2g5*c2g5-1.0;
/* s5g5 = s4g5*cg5+c4g5*sg5; */
/* c5g5 = c4g5*cg5-s4g5*sg5; */

s2g6 = 2*sg6*cg6;
c2g6 = 2*cg6*cg6-1.0;
s3g6 = s2g6*cg6+c2g6*sg6;
c3g6 = c2g6*cg6-s2g6*sg6;
s4g6 = 2*s2g6*c2g6;
c4g6 = 2*c2g6*c2g6-1.0;
s5g6 = s4g6*cg6+c4g6*sg6;
c5g6 = c4g6*cg6-s4g6*sg6;
s6g6 = 2*s3g6*c3g6;
c6g6 = 2*c3g6*c3g6-1.0;
s7g6 = s6g6*cg6+c6g6*sg6;
c7g6 = c6g6*cg6-s6g6*sg6;
s9g6 = s7g6*c2g6+c7g6*s2g6;
c9g6 = c7g6*c2g6-s7g6*s2g6;
s10g6 = 2*s5g6*c5g6;
c10g6 = 2*c5g6*c5g6-1.0;
s11g6 = s10g6*cg6+c10g6*sg6;
c11g6 = c10g6*cg6-s10g6*sg6;

s2g7 = 2*sg7*cg7;
c2g7 = 2*cg7*cg7-1.0;
s3g7 = s2g7*cg7+c2g7*sg7;
c3g7 = c2g7*cg7-s2g7*sg7;
s2gm4g = s2g5*c4g6-c2g5*s4g6;
c2gm4g = c2g5*c4g6+s2g5*s4g6;
s2gm5g = s2g5*c5g6-c2g5*s5g6;
c2gm5g = c2g5*c5g6+s2g5*s5g6;
s2gm6g = s2g5*c6g6-c2g5*s6g6;
c2gm6g = c2g5*c6g6+s2g5*s6g6;

l6 = varv180(g6 + 90.1109);

sl6 = sind(l6);
cl6 = cosd(l6);

s2l6 = 2*sl6*cl6;
c2l6 = 2*cl6*cl6-1.0;

s2gm7g = s2g5*c7g6-c2g5*s7g6;
c2gm7g = c2g5*c7g6+s2g5*s7g6;


lona =
    (23045.-142.*t) * sg6
   +        5014.*t
   -  (2689.-60.*t) * c2gm5g
   +          2507.
   + (1177.+101.*t) * s2gm5g
   -    (826.-3.*t) * c2gm4g
   +   (802.-11.*t) * s2g6
   +    (425.+2.*t) * (sg5*c2g6-cg5*s2g6)
   -  (114.+229.*t) * cg6
   -    (153.-3.*t) * (c2g5*c6g6+s2g5*s6g6)
   -     (70.+3.*t) * c2l6
   +     (67.-3.*t) * s2l6
   +     (66.+6.*t) * s2gm6g
   +            41. * (sg5*c3g6-cg5*s3g6)
   +            39. * s3g6
   +            31. * (sg5*cg6-cg5*sg6)
   +            31. * (s2g5*c2g6-c2g5*s2g6)
   -            29. * (c2g5*c3g6+s2g5*s3g6)
   -            28. * (s2gm6g*c3g7+c2gm6g*s3g7)
   +            28. * (cg5*c3g6+sg5*s3g6)
   -    (12.-22.*t) * s2gm4g
   -            22. * (sg6*c3g7-cg6*s3g7)
   +            20. * (s2g5*c3g6-c2g5*s3g6)
   +     (20.+6.*t) * (c4g5*c10g6+s4g5*s10g6)
   +            19. * (c2g6*c3g7+s2g6*s3g7)
   +     (19.-6.*t) * (s4g5*c10g6-c4g5*s10g6)
   -     (7.+17.*t) * c2g6
   -            16. * (cg6*c3g7+sg6*s3g7);

lonb =
   +            12. * cg5
   -            12. * (s2g6*c2g7-c2g6*s2g7)
   -            11. * c2gm7g
   +            10. * (s2g6*c3g7-c2g6*s3g7)
   +            10. * (c2g5*c2g6+s2g5*s2g6)
   +             9. * (s4g5*c9g6-c4g5*s9g6)
   -             8. * (sg6*c2g7-cg6*s2g7)
   -             8. * (c2l6*cg6-s2l6*sg6)
   +             8. * (c2l6*cg6+s2l6*sg6)
   +             8. * (cg6*cg7+sg6*sg7)
   -             8. * (s2l6*cg6-c2l6*sg6)
   +             7. * (s2l6*cg6+c2l6*sg6)
   -      (7.-4.*t) * (cg5*c2g6+sg5*s2g6)
   -             5. * (s3g5*c7g6-c3g5*s7g6)
   -             5. * (c3g5*c3g6+s3g5*s3g6)
   -             5. * (c2g6*c2g7+s2g6*s2g7)
   +             5. * (s3g5*c4g6-c3g5*s4g6)
   +             5. * s2gm7g
   +             4. * (s3g5*c3g6-c3g5*s3g6)
   +             4. * (s3g5*c5g6-c3g5*s5g6)
   +             3. * (c2gm6g*c3g7-s2gm6g*s3g7)
   +             3. * (c3g5*c7g6+s3g5*s7g6)
   +             3. * (c4g5*c9g6+s4g5*s9g6)
   +             3. * (s3g5*c6g6-c3g5*s6g6)
   +             3. * (s2g5*cg6-c2g5*sg6)
   +             3. * (sg5*c4g6-cg5*s4g6)
   +             2. * (c3g6*c3g7+s3g6*s3g7)
   +             2. * s4g6
   -             2. * (c3g5*c4g6+s3g5*s4g6)
   -             2. * (c2g5*cg6+s2g5*sg6)
   -             2. * (s2gm7g*c3g7+c2gm7g*s3g7)
   +             2. * (cg5*c4g6+sg5*s4g6)
   +             2. * (c4g5*c11g6+s4g5*s11g6)
   -             2. * (sg6*cg7-cg6*sg7);

*lon = l6 + ( lona + lonb ) / 3600.0;

*lat = (
      (8297.+18.*t) * sg6
   -  (3346.-79.*t) * cg6
   +    (462.-4.*t) * s2g6
   -           189. * c2g6
   +     185.-10.*t
   -            71. * c2gm4g
   +           3.*t * s2gm4g
   +            46. * s2gm6g
   -            45. * c2gm6g
   +            29. * s3g6
   -            20. * (c2g5*c3g6+s2g5*s3g6)
   -            14. * c2gm5g
   -            11. * c3g6
   +             9. * (sg5*c3g6-cg5*s3g6)
   +             8. * (sg5*cg6-cg5*sg6)
   -             6. * (s2g5*c3g6-c2g5*s3g6)
   +             5. * s2gm7g
   -             5. * c2gm7g
   +             4. * s2gm5g
   -             3. * (cg5*cg6+sg5*sg6)
   +             3. * (cg5*c3g6+sg5*s3g6)
   +             3. * (sg5*c2g6-cg5*s2g6)
   +             2. * s4g6
   -             2. * (c2g5*c2g6+s2g5*s2g6)
      ) / 3600.0;

*r =
     (9.55774-0.00028*t)
   - (0.53252-0.00328*t) * cg6
   -             0.01878 * s2gm4g
   -             0.01482 * c2g6
   +             0.00817 * (sg5*cg6-cg5*sg6)
   -             0.00539 * (cg5*c2g6+sg5*s2g6)
   - (0.00225+0.00524*t) * sg6
   +             0.00349 * s2gm5g
   +             0.00347 * s2gm6g
   +             0.00149 * (c2g5*c6g6+s2g5*s6g6)
   -             0.00126 * (c2g5*c2g6+s2g5*s2g6)
   +             0.00104 * (cg5*cg6+sg5*sg6)
   +             0.00101 * c2gm5g
   +             0.00098 * (cg5*c3g6+sg5*s3g6)
   -             0.00073 * (c2g5*c3g6+s2g5*s3g6)
   -             0.00062 * c3g6
   +             0.00042 * (s2g6*c3g7-c2g6*s3g7)
   +             0.00041 * (s2g5*c2g6-c2g5*s2g6)
   -             0.00040 * (sg5*c3g6-cg5*s3g6)
   +             0.00040 * (c2g5*c4g6+s2g5*s4g6)
   -             0.00023 * sg5
   +             0.00020 * (s2g5*c7g6-c2g5*s7g6);

}  /* satpos_vf */



/*

VF_URA.C - Uranus position

Referens: van Flandern & Pulkkinen
Astrophys. J. Suppl., 41:391-411, nov 1979

Paul Schlyter, 1987-06-14

*/






void urapos_vf( double jd, double *lon, double *lat, double *r )
/**************************************************************/
{
double dnr, t,
       g5, g6, g7, g8,
       sg5, cg5, sg6, cg6, sg7, cg7, sg8, cg8,
       s2g5, c2g5,
       s2g6, c2g6, s3g6, c3g6, s6g6, c6g6,
       s2g7, c2g7, s3g7, c3g7,
       s2g8, c2g8, s3g8, c3g8,
       s2gm6g, c2gm6g,
       l7, f7, sf7, cf7, s4g7, c4g7, s2f7, s4g8, c4g8;


dnr = jd - 2451545.0;
t = dnr/36525.0 + 1.0;


g5 = 360.0 * frac( 0.056531 + 0.00023080893 * dnr );
g6 = 360.0 * frac( 0.882987 + 0.00009294371 * dnr );
g7 = 360.0 * frac( 0.400589 + 0.00003269438 * dnr );
g8 = 360.0 * frac( 0.725368 + 0.00001672092 * dnr );

sg5 = sind(g5);
cg5 = cosd(g5);
sg6 = sind(g6);
cg6 = cosd(g6);
sg7 = sind(g7);
cg7 = cosd(g7);
sg8 = sind(g8);
cg8 = cosd(g8);

s2g5 = 2*sg5*cg5;
c2g5 = 2*cg5*cg5-1.0;

s2g6 = 2*sg6*cg6;
c2g6 = 2*cg6*cg6-1.0;
s3g6 = s2g6*cg6+c2g6*sg6;
c3g6 = c2g6*cg6-s2g6*sg6;
s6g6 = 2*s3g6*c3g6;
c6g6 = 2*c3g6*c3g6-1.0;

s2g7 = 2*sg7*cg7;
c2g7 = 2*cg7*cg7-1.0;
s3g7 = s2g7*cg7+c2g7*sg7;
c3g7 = c2g7*cg7-s2g7*sg7;

s2g8 = 2*sg8*cg8;
c2g8 = 2*cg8*cg8-1.0;
s3g8 = s2g8*cg8+c2g8*sg8;
c3g8 = c2g8*cg8-s2g8*sg8;

s2gm6g = s2g5*c6g6-c2g5*s6g6;
c2gm6g = c2g5*c6g6+s2g5*s6g6;


/* Uranus position: */

l7 =  varv180(g7 + 169.0488);
f7 =  360.0 * frac( 0.664614 + 0.00003265562 * dnr );

sf7 = sind(f7);
cf7 = cosd(f7);

s4g7 = 2*s2g7*c2g7;
c4g7 = 2*c2g7*c2g7-1.0;

s2f7 = 2*sf7*cf7;

s4g8 = 2*s2g8*c2g8;
c4g8 = 2*c2g8*c2g8-1.0;


*lon = l7 + (
   (19397.+110.*t-9.*t*t) * sg7
   +          (570.+7.*t) * s2g7
   - (12.+536.*t+12.*t*t) * cg7
   +                 143. * (sg6*c2g7-cg6*s2g7)
   +          (102.-7.*t) * (sg6*c3g7-cg6*s3g7)
   +           (76.+7.*t) * (cg6*c3g7+sg6*s3g7)
   -                  49. * (sg5*cg7-cg5*sg7)
   +              32.*t*t
   -                30.*t * c2g7
   +                  29. * (s2gm6g*c3g7+c2gm6g*s3g7)
   +                  29. * (c2g7*c2g8+s2g7*s2g8)
   -                  28. * (cg7*cg8+sg7*sg8)
   +                  23. * s3g7
   -                  21. * (cg5*cg7+sg5*sg7)
   +                  20. * (sg7*cg8-cg7*sg8)
   +           (20.+8.*t) * (cg6*c2g7+sg6*s2g7)
   -                  19. * (cg6*cg7+sg6*sg7)
   +                  17. * (s2g7*c3g8-c2g7*s3g8)
   +                  14. * (s3g7*c3g8-c3g7*s3g8)
   +                  13. * (sg6*cg7-cg6*sg7)
   +                  10. * (s2g7*c2g8-c2g7*s2g8)
   -                   9. * s2f7
   +                   9. * (c2g7*c3g8+s2g7*s3g8)
   +                   6. * (s2gm6g*c2g7+c2gm6g*s2g7)
   +                   6. * (c2gm6g*c2g7-s2gm6g*s2g7)
   +                   5. * (sg6*c4g7-cg6*s4g7)
   -                   4. * (s3g7*c4g8-c3g7*s4g8)
   +                   4. * (c3g7*c3g8+s3g7*s3g8)
   -                   3. * cg8
   -                   2. * sg8
            ) / 3600.0;

*lat =  (           2775. * sf7
                 +   131. * (sg7*cf7-cg7*sf7)
                 +   130. * (sg7*cf7+cg7*sf7)
        ) / 3600.0;

*r  = 19.21216
    - (0.90154+0.00508*t) * cg7
    -           0.02488*t * sg7
    -             0.02121 * c2g7
    -             0.00585 * (cg6*c2g7+sg6*s2g7)
    -             0.00451 * (cg5*cg7+sg5*sg7)
    +             0.00336 * (sg6*cg7-cg6*sg7)
    +             0.00198 * (sg5*cg7-cg5*sg7)
    +             0.00118 * (cg6*c3g7+sg6*s3g7)
    +             0.00107 * (sg6*c2g7-cg6*s2g7)
    -           0.00103*t * s2g7
    -             0.00081 * (c3g7*c3g8+s3g7*s3g8);

}  /* urapos_vf */



/*

VF_NEP.C - Neptunus position

Referens: van Flandern & Pulkkinen
Astrophys. J. Suppl., 41:391-411, nov 1979

Paul Schlyter, 1987-06-14

*/



void neppos_vf( double jd, double *lon, double *lat, double *r )
/**************************************************************/
{
double dnr, t,
       g5, g6, g7, g8,
       sg5, cg5, sg6, cg6, sg7, cg7, sg8, cg8,
       s2g7, c2g7,
       s2g8, c2g8, s3g8, c3g8,
       l7,
       l8, f8, sf8, cf8;


dnr = jd - 2451545.0;
t = dnr/36525.0 + 1.0;


g5 = 360.0 * frac( 0.056531 + 0.00023080893 * dnr );
g6 = 360.0 * frac( 0.882987 + 0.00009294371 * dnr );
g7 = 360.0 * frac( 0.400589 + 0.00003269438 * dnr );
g8 = 360.0 * frac( 0.725368 + 0.00001672092 * dnr );

sg5 = sind(g5);
cg5 = cosd(g5);
sg6 = sind(g6);
cg6 = cosd(g6);
sg7 = sind(g7);
cg7 = cosd(g7);
sg8 = sind(g8);
cg8 = cosd(g8);

s2g7 = 2*sg7*cg7;
c2g7 = 2*cg7*cg7-1.0;

s2g8 = 2*sg8*cg8;
c2g8 = 2*cg8*cg8-1.0;
s3g8 = s2g8*cg8+c2g8*sg8;
c3g8 = c2g8*cg8-s2g8*sg8;


/* Neptunus position: */

l7 =  varv180(g7 + 169.0488);
l8 =  varv180(g8 +  43.7558);
f8 =  360.0 * frac( 0.480856 + 0.00001663715 * dnr );

sf8 = sind(f8);
cf8 = cosd(f8);


*lon = l8 + (
      (3523.-4.*t+4.*t*t) * sg8
   -                  50. * 2*sf8*cf8
   -                43.*t * cg8
   +                  29. * (sg5*cg8-cg5*sg8)
   +                  19. * s2g8
   -                  18. * (cg5*cg8+sg5*sg8)
   +                  13. * (cg6*cg8+sg6*sg8)
   +                  13. * (sg6*cg8-cg6*sg8)
   -                   9. * (s2g7*c3g8-c2g7*s3g8)
   +                   9. * (c2g7*c2g8+s2g7*s2g8)
   -                   5. * (c2g7*c3g8+s2g7*s3g8)
   +                   4. * (cg7*c2g8+sg7*s2g8)
            ) / 3600.0;

*lat = (    (6404.-33.*t) * sf8
                   +  55. * (sg8*cf8+cg8*sf8)
                   +  55. * (sg8*cf8-cg8*sf8)
       ) / 3600.0;

*r  =            30.07175
            -     0.25701 * cg8
            -     0.00787 * cosd(2*l7-g7-2*l8)
            +     0.00409 * (cg5*cg8+sg5*sg8)
            -   0.00314*t * sg8
            +     0.00250 * (sg5*cg8-cg5*sg8)
            -     0.00194 * (sg6*cg8-cg6*sg8)
            +     0.00185 * (cg6*cg8+sg6*sg8);

}  /* neppos_vf */



/*

PLTPOSVF.C

Plutos position
Referens: van Flandern & Pulkkinen
Astrophys. J. Suppl., 41:391-411, nov 1979

Paul Schlyter, 1987-06-14

*/




void plutpos_vf( double jd, double *lon, double *lat, double *r )
/***************************************************************/
{

double dnr, t,
       l9, g9, f9,
       sg9, cg9, sf9, cf9,
       s2g9, c2g9, s3g9, c3g9, s4g9, c4g9,
       s2f9, c2f9, s3f9, c3f9;


dnr = jd - 2451545.0;
t = dnr/36525.0 + 1.0;

l9 =  360.0 * frac( 0.663854 + 0.00001115482 * dnr );
g9 =  360.0 * frac( 0.041020 + 0.00001104864 * dnr );
f9 =  varv180(g9 + 113.8806);

sg9 = sind(g9);
cg9 = cosd(g9);
sf9 = sind(f9);
cf9 = cosd(f9);

s2g9 = 2*sg9*cg9;
c2g9 = 2*cg9*cg9-1.0;
s3g9 = s2g9*cg9+c2g9*sg9;
c3g9 = c2g9*cg9-s2g9*sg9;
s4g9 = 2*s2g9*c2g9;
c4g9 = 2*c2g9*c2g9-1.0;

s2f9 = 2*sf9*cf9;
c2f9 = 2*cf9*cf9-1.0;
s3f9 = s2f9*cf9+c2f9*sf9;
c3f9 = c2f9*cf9-s2f9*sf9;

*lon = l9 + (
(101577.+200.*t+227.*t*t) * sg9
               +   15517. * s2g9
               -    3593. * s2f9
               +    3414. * s3g9
               -    2201. * (sg9*c2f9-cg9*s2f9)
               -    1871. * (sg9*c2f9+cg9*s2f9)
               +     839. * s4g9
               -     757. * (s2g9*c2f9+c2g9*s2f9)
               -     285. * (s3g9*c2f9+c3g9*s2f9)
               +     218. * (s2g9*c2f9-c2g9*s2f9)
           ) / 3600.0;

*lat = (
                   57726. * sf9
               +   15257. * (sg9*cf9-cg9*sf9)
               +   14102. * (sg9*cf9+cg9*sf9)
               +    3870. * (s2g9*cf9+c2g9*sf9)
               +    1138. * (s3g9*cf9+c3g9*sf9)
               +     472. * (s2g9*cf9-c2g9*sf9)
               +     353. * (s4g9*cf9+c4g9*sf9)
               -     144. * (sg9*c3f9-cg9*s3f9)
               -     119. * s3f9
               -     111. * (sg9*c3f9+cg9*s3f9)
       ) / 3600.0;

*r  =            40.74638
               -  9.58235 * cg9
               -  1.16703 * c2g9
               -  0.22649 * c3g9
               -  0.04996 * c4g9;

}  /* plutpos_vf */

// Returns planet's ecliptic coordinates at a particular Julian Ephemeris Date (jed)
// Planet identifier (planet) is 0 = Sun, 1 = Mercury, 2 = Venus ... 10 or 301 = Moon.
// Returned coordinates are heliocentric for planets, and geocentric for Moon.
// Radial distance from center is in AU for planets, and Earth-radii for Moon.
// Ecliptic longitude and latitude are in radians for all objects.

SSSpherical SSVPEphemeris::eclipticCoordinates ( int planet, double jed )
{
    double lon = 0.0, lat = 0.0, r = 0.0;
    
    if ( planet == 1 )
        merpos_vf ( jed, &lon, &lat, &r );
    else if ( planet == 2 )
        venpos_vf ( jed, &lon, &lat, &r );
    else if ( planet == 3 )
    {
        solpos_vf ( jed, &lon, &lat, &r );
        lon += 180.0;
    }
    else if ( planet == 4 )
        marpos_vf ( jed, &lon, &lat, &r );
    else if ( planet == 5 )
        juppos_vf ( jed, &lon, &lat, &r );
    else if ( planet == 6 )
        satpos_vf ( jed, &lon, &lat, &r );
    else if ( planet == 7 )
        urapos_vf ( jed, &lon, &lat, &r );
    else if ( planet == 8 )
        neppos_vf ( jed, &lon, &lat, &r );
    else if ( planet == 9 )
        plutpos_vf ( jed, &lon, &lat, &r );
    else if ( planet == 10 || planet == 301 )
        monpos_vf ( jed, &lon, &lat, &r );

    return SSSpherical ( DEGRAD * lon, DEGRAD * lat, r );
}

// Returns planet's equatorial rectangular position vector at a particular Julian Ephemeris Date (jed)
// referred to a particular precession epoch (epoch), expressed as a year, like 2000.0.
// For current equatorial coordinates (i.e. epoch equals jed), pass 0 for epoch.
// Planet identifier (planet) is 0 = Sun, 1 = Mercury, 2 = Venus ... 10 or 301 = Moon.
// Returned vector is heliocentric in AU for planets, and geocentric in Earth-radii for Moon.
// Nutation is neglected, so this function returns mean equatorial position vectors!

SSVector SSVPEphemeris::equatorialPosition ( int planet, double jed, double epoch )
{
    SSSpherical ecl = eclipticCoordinates ( planet, jed );

    double d = jed - 2451545.0;
    double t = d / 36525.0 + 1.0;
    double eps = DEGRAD * ( 84428.0 + 47.0 * t ) / 3600.0;
    double lon_corr = epoch ? DEGRAD * ( 3.82394E-5 * ( ( epoch - 2000.0 ) - ( d + 1.5 ) ) ) : 0.0;    // precession correction
    double x = ecl.rad * ( cos ( ecl.lat ) * cos ( ecl.lon + lon_corr ) );
    double y = ecl.rad * ( cos ( ecl.lat ) * sin ( ecl.lon + lon_corr ) * cos ( eps ) - sin ( ecl.lat ) * sin ( eps ) );
    double z = ecl.rad * ( cos ( ecl.lat ) * sin ( ecl.lon + lon_corr ) * sin ( eps ) + sin ( ecl.lat ) * cos ( eps ) );
    
    return SSVector ( x, y, z );
}

// Computes planet's equatorial rectangular position and velocity vectors in fundamental
// (J2000 mean equatorial) frame at a particular Julian Ephemeris Date (jed)
// Planet identifier (planet) is 0 = Sun, 1 = Mercury, 2 = Venus ... 10 or 301 = Moon.
// Returned vector is heliocentric in AU for planets, and geocentric in Earth-radii for Moon.
// See comments regarding velocity computation approximation below.

void SSVPEphemeris::fundamentalPositionVelocity ( int planet, double jed, SSVector &pos, SSVector &vel )
{
    double dt = 1.0;
    
    if ( planet == 1 )
        dt = 0.01136771400;
    else if ( planet == 2 )
        dt = 0.00445046867;
    else if ( planet == 3 )
        dt = 0.00273790931;
    else if ( planet == 4 )
        dt = 0.00145575328;
    else if ( planet == 5 )
        dt = 0.00023080893;
    else if ( planet == 6 )
        dt = 0.00009294371;
    else if ( planet == 7 )
        dt = 0.00003269438;
    else if ( planet == 8 )
        dt = 0.00001672092;
    else if ( planet == 9 )
        dt = 0.00001115482;
    else if ( planet == 10 || planet == 301 )
        dt = 0.03660110129;
    
    // Compute delta time needed for planet's mean longitude to increase by 0.1 degrees.
    // Compute planet's J2000 position at current date, then at current date minus delta time.
    // Compute planet's velocity as difference in position divided by delta time:
    // a very accurate approximation since its motion will be nearly linear over that time.
    // The computational load is identical to the mathematically correct approach,
    // because the time derivative of a trig series is another trig series.
    // And these are low-precision planetary position formulae, anyhow!
    
    dt = ( 1.0 / 3600.0 ) / dt;
    pos = equatorialPosition ( planet, jed, 2000.0 );
    vel = equatorialPosition ( planet, jed - dt, 2000.0 );
    vel = ( pos - vel ) / dt;
}
