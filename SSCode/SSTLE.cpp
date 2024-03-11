// SSTLE.cpp
// SSCore
//
// Created by Tim DeBenedictis on 4/4/20.
// Copyright © 2020 Southern Stars. All rights reserved.
//
// This C++ source code derives mostly from NORADS's spacetrack report #3,
// FORTRAN routines and Dr. T. Kelso's Pascal ports of same. This file
// contains SGP, SGP4, and SDP4 satellite ephemeris routines for the prediction
// of a satellites position and velocity from its three-line element (TLE) set
// of orbital parameters.

#include <string.h>

#include "SSUtilities.hpp"
#include "SSTime.hpp"
#include "SSTLE.hpp"

// Static data used by SGP orbit model

struct sgp_args
{
    double ao,qo,xlo,d1o,d2o,d3o,d4o,omgdt,xnodot,c5,c6;
};

// Static data used by SGP4 orbit model

struct sgp4_args
{
    int isimp;
    double aodp,aycof,c1,c4,c5,cosio,d2,d3,d4,delmo,omgcof,
           eta,omgdot,sinio,xnodp,sinmo,t2cof,t3cof,t4cof,t5cof,
           x1mth2,x3thm1,x7thm1,xmcof,xmdot,xnodcf,xnodot,xlcof;
};

// Common arguments and static data used by deep-space functions

struct deep_args
{
    int iresfl, isynfl;
    
    // Used by dpinit part of Deep()
    double eosq,sinio,cosio,betao,aodp,theta2,sing,cosg,
           betao2,xmdot,omgdot,xnodot,xnodp;

    // Used by dpsec and dpper parts of Deep()
    double xll,omgadf,xnode,em,xinc,xn,t;

    // Used by Thetag() and Deep()
    double ds50;
  
    // Static variables stored between calls to Deep()
    double thgr,xnq,xqncl,omegaq,zmol,zmos,savtsn,ee2,e3,xi2,
           xl2,xl3,xl4,xgh2,xgh3,xgh4,xh2,xh3,sse,ssi,ssg,xi3,
           se2,si2,sl2,sgh2,sh2,se3,si3,sl3,sgh3,sh3,sl4,sgh4,
           ssl,ssh,d3210,d3222,d4410,d4422,d5220,d5232,d5421,
           d5433,del1,del2,del3,fasx2,fasx4,fasx6,xlamo,xfact,
           xni,atime,stepp,stepn,step2,preep,pl,sghs,xli,
           d2201,d2211,sghl,sh1,pinc,pe,shs,zsingl,zcosgl,
           zsinhl,zcoshl,zsinil,zcosil;
};

// Static data used by SDP4 orbit model

struct sdp4_args
{
    double x3thm1,c1,x1mth2,c4,xnodcf,t2cof,xlcof,aycof,x7thm1;

    struct deep_args deep;
};

// Entry points for dodeep() method.

enum deep_entry
{
    dpinit = 1, // Deep-space initialization code
    dpsec = 2,  // Deep-space secular code
    dpper = 3   // Deep-space periodic code
};

static double pio2 = 1.57079633;
static double x3pio2 = 4.71238898;
static double twopi = 6.2831853;
static double xpi = 3.14159265;
static double e6a = 1.0e-6;
static double tothrd = 6.6666667e-1;
static double xj2 = 1.082616e-3;
static double xj3 = -2.53881e-6;
static double xj4 = -1.65597e-6;
static double xke = 7.43669161e-2;
static double xkmper = 6.378135e3;
static double xmnpda = 1.44e3;
static double xae = 1.0;
static double ck2 = 0.5 * xj2 * xae * xae;
static double ck4 = -0.375 * xj4 * xae * xae * xae * xae;
static double q0 = 120.0;
static double s0 = 78.0;
static double qoms2t = ( ( q0 - s0 ) * xae / xkmper )
                     * ( ( q0 - s0 ) * xae / xkmper )
                     * ( ( q0 - s0 ) * xae / xkmper )
                     * ( ( q0 - s0 ) * xae / xkmper );
static double s = xae * ( 1.0 + s0 / xkmper );
static double a3ovk2 = -xj3 / ck2;

static double zns = 1.19459E-5;
static double c1ss = 2.9864797E-6;
static double zes = 0.01675;
static double znl = 1.5835218E-4;
static double c1l = 4.7968065E-7;
static double zel = 0.05490;
static double zcosis = 0.91744867;
static double zsinis = 0.39785416;
static double zsings = -0.98088458;
static double zcosgs = 0.1945905;
static double q22 = 1.7891679E-6;
static double q31 = 2.1460748E-6;
static double q33 = 2.2123015E-7;
static double g22 = 5.7686396;
static double g32 = 0.95240898;
static double g44 = 1.8014998;
static double g52 = 1.0508330;
static double g54 = 4.4108898;
static double root22 = 1.7891679E-6;
static double root32 = 3.7393792E-7;
static double root44 = 7.3636953E-9;
static double root52 = 1.1428639E-7;
static double root54 = 2.1765803E-9;
static double thdt = 4.3752691E-3;

// Local functions

double actan ( double sinx, double cosx )
{
    if( cosx == 0.0 )
    {
        if ( sinx > 0 )
            return pio2;
        else
            return x3pio2;
    }
    else
    {
        if ( cosx > 0.0 )
        {
            if ( sinx > 0.0 )
                return atan ( sinx / cosx );
            else
                return twopi + atan ( sinx / cosx );
        }
        else
            return xpi + atan ( sinx / cosx );
    }
}

double fmod2p ( double x )
{
    int i;
    double fmod2p = x;
    
    i = fmod2p / twopi;
    fmod2p -= i * twopi;
    
    if ( fmod2p < 0 )
        fmod2p += twopi;

    return ( fmod2p );
}

double thetag ( double ep, deep_args *deep_arg )
{
    double yr, d, ds50, theta, temp, thetag;
    int jy, n, i;
    
    // Modification -- determine whether epoch conforms to TLE format
    // and if not, assume it's a Julian date.
    
    if ( ep < 99366.0 )
    {
        yr = ( ep + 2.0e-7 ) * 1.0e-3;
        jy = yr;
        yr = jy;
        d = ep - yr * 1.0e3;
        // Modification to support Y2K
        // Valid 1957 through 2056
        if ( jy < 57 )
            jy = jy + 100;
        n = ( jy - 69 ) / 4;
        if ( jy < 70 )
            n = ( jy - 72 ) / 4;
        ds50 = 7305.0 + 365.0 * ( jy - 70 ) + n + d;
    }
    else
    {
        ds50 = ep - 2433281.5;
    }
    
    theta = 1.72944494 + 6.3003880987 * ds50;
    temp = theta / twopi;
    i = temp;
    temp = i;
    thetag = theta - temp * twopi;
    if ( thetag < 0.0 )
        thetag = thetag + twopi;

    deep_arg->ds50 = ds50;
    return ( thetag );
}

// math/vector functions

double acose ( double x )
{
    double rval;

    if ( x >= 1.0 )
        rval = 0.;
    else if ( x <= -1.0 )
        rval = xpi;
    else
        rval = acos ( x );
    
    return ( rval );
}

// v1 . v2

double dot(double* v1, double* v2)
{
    double sum = 0;
    
    for ( int i = 0; i < 3; i++ )
        sum += v1[i] * v2[i];
    
    return sum;
}

// |v|

double norm ( double *v )
{
    return sqrt ( dot ( v, v ) );
}

// a * v = av

void smult(double a, double* v, double* av)
{
    for ( int i = 0; i < 3; i++ )
        av[i] = a * v[i];
}

// v1 + v2 = s

void vadd ( double *v1, double *v2, double *s )
{
    for ( int i = 0; i < 3; i++ )
        s[i] = v1[i] + v2[i];
}

// v1 x v2 = b

void cross ( double v1[3], double v2[3], double b[3] )
{
    b[0] = v1[1] * v2[2] - v1[2] * v2[1];
    b[1] = v1[2] * v2[0] - v1[0] * v2[2];
    b[2] = v1[0] * v2[1] - v1[1] * v2[0];
}

// u = v / ||v||

void unitv( double v[], double u[] )
{
    double no = norm( v );
    for ( int i = 0; i < 3; i++ )
        u[i] = v[i] / no;
}

// default constructor zero initializes

SSTLE::SSTLE ( void )
{
    name = desig = "";
    norad = 0;
    jdepoch = xndt2o = xndd6o = bstar = xincl = xnodeo = eo = omegao = xmo = xno = 0.0;
    deep = false;
    argp.sgp = nullptr;
}

// copy constructor nullifies argp pointer; any of the sgp() methods will re-create it.
// This ensures that deleting this SSTLE won't delete the original SSTLE's argp.

SSTLE::SSTLE ( const SSTLE &other )
{
    *this = other;
    argp.sgp = nullptr;
}

// Returns WGS72 Earth equatorial radius in kilometers.
// Use this to convert output of sgp(), sgp4(), sdp4() to km!

double SSTLE::kmper ( void )
{
    return xkmper;
}

// Determines whether to use a deep-space (true) or near-Earth (false) ephemeris.
// For satellites with orbit period > 225 minutesm use deep space ephemeris.

bool SSTLE::isdeep ( void )
{
    double ao, xnodp, dd1, dd2, delo, temp, a1, del1, r1;
  
    dd1 = xke / xno;
    dd2 = tothrd;
    a1 = pow ( dd1, dd2 );
    r1 = cos ( xincl );
    dd1 = ( 1.0 - eo * eo );
    temp = ck2 * 1.5f * ( r1 * r1 * 3.0 - 1.0) / pow ( dd1, 1.5 );
    del1 = temp / ( a1 * a1 );
    ao = a1 * ( 1.0 - del1 * ( tothrd * 0.5 + del1 * ( del1 * 1.654320987654321 + 1.0 ) ) );
    delo = temp / ( ao * ao );
    xnodp = xno / ( delo + 1.0 );

    if ( twopi / xnodp / xmnpda >= 0.15625 )
        return true;
    else
        return false;
}

// Used internally by SDP4 deep space ephemeris; should not be called directly.
// The entry point (ientry) is one of (dpinit,dpper,dpsec) defined above.
// Values are input and output in the deep_args struct (arg)

void SSTLE::dodeep ( int ientry, deep_args *arg )
{
    double thgr = 0,xnq = 0,xqncl = 0,omegaq = 0,zmol = 0,zmos = 0,savtsn = 0,ee2 = 0,e3 = 0,xi2 = 0,
           xl2 = 0,xl3 = 0,xl4 = 0,xgh2 = 0,xgh3 = 0,xgh4 = 0,xh2 = 0,xh3 = 0,sse = 0,ssi = 0,ssg = 0,xi3 = 0,
           se2 = 0,si2 = 0,sl2 = 0,sgh2 = 0,sh2 = 0,se3 = 0,si3 = 0,sl3 = 0,sgh3 = 0,sh3 = 0,sl4 = 0,sgh4 = 0,
           ssl = 0,ssh = 0,d3210 = 0,d3222 = 0,d4410 = 0,d4422 = 0,d5220 = 0,d5232 = 0,d5421 = 0,
           d5433 = 0,del1 = 0,del2 = 0,del3 = 0,fasx2 = 0,fasx4 = 0,fasx6 = 0,xlamo = 0,xfact = 0,
           xni = 0,atime = 0,stepp = 0,stepn = 0,step2 = 0,preep = 0,pl = 0,sghs = 0,xli = 0,
           d2201 = 0,d2211 = 0,sghl = 0,sh1 = 0,pinc = 0,pe = 0,shs = 0,zsingl = 0,zcosgl = 0,
           zsinhl = 0,zcoshl = 0,zsinil = 0,zcosil = 0;

    double a1 = 0,a2 = 0,a3 = 0,a4 = 0,a5 = 0,a6 = 0,a7 = 0,a8 = 0,a9 = 0,a10 = 0,ainv2 = 0,alfdp = 0,aqnv = 0,
           sgh = 0,sini2 = 0,sinis = 0,sinok = 0,sh = 0,si = 0,sil = 0,day = 0,betdp = 0,dalf = 0,
           bfact = 0,c = 0,cc = 0,cosis = 0,cosok = 0,cosq = 0,ctem = 0,f322 = 0,zx = 0,zy = 0,
           dbet = 0,dls = 0,eoc = 0,eq = 0,f2 = 0,f220 = 0,f221 = 0,f3 = 0,f311 = 0,f321 = 0,xnoh = 0,
           f330 = 0,f441 = 0,f442 = 0,f522 = 0,f523 = 0,f542 = 0,f543 = 0,g200 = 0,g201 = 0,
           g211 = 0,pgh = 0,ph = 0,s1 = 0,s2 = 0,s3 = 0,s4 = 0,s5 = 0,s6 = 0,s7 = 0,se = 0,sel = 0,ses = 0,xls = 0,
           g300 = 0,g310 = 0,g322 = 0,g410 = 0,g422 = 0,g520 = 0,g521 = 0,g532 = 0,g533 = 0,gam = 0,
           sinq = 0,sinzf = 0,sis = 0,sl = 0,sll = 0,sls = 0,stem = 0,temp = 0,temp1 = 0,x1 = 0,x2 = 0,
           x2li = 0,x2omi = 0,x3 = 0,x4 = 0,x5 = 0,x6 = 0,x7 = 0,x8 = 0,xl = 0,xldot = 0,xmao = 0,xnddt = 0,
           xndot = 0,xno2 = 0,xnodce = 0,xnoi = 0,xomi = 0,xpidot = 0,z1 = 0,z11 = 0,z12 = 0,z13 = 0,
           z2 = 0,z21 = 0,z22 = 0,z23 = 0,z3 = 0,z31 = 0,z32 = 0,z33 = 0,ze = 0,zf = 0,zm = 0,zmo = 0,zn = 0,
           zsing = 0,zsinh = 0,zsini = 0,zcosg = 0,zcosh = 0,zcosi = 0,delt=0,ft=0;

    int ls = 0, dl = 0, er = 0;
  
    // Recover saved values
    
    thgr = arg->thgr;
    xnq = arg->xnq;
    xqncl = arg->xqncl;
    omegaq = arg->omegaq;
    zmol = arg->zmol;
    zmos = arg->zmos;
    savtsn = arg->savtsn;
    ee2 = arg->ee2;
    e3 = arg->e3;
    xi2 = arg->xi2;
    xl2 = arg->xl2;
    xl3 = arg->xl3;
    xl4 = arg->xl4;
    xgh2 = arg->xgh2;
    xgh3 = arg->xgh3;
    xgh4 = arg->xgh4;
    xh2 = arg->xh2;
    xh3 = arg->xh3;
    sse = arg->sse;
    ssi = arg->ssi;
    ssg = arg->ssg;
    xi3 = arg->xi3;
    se2 = arg->se2;
    si2 = arg->si2;
    sl2 = arg->sl2;
    sgh2 = arg->sgh2;
    sh2 = arg->sh2;
    se3 = arg->se3;
    si3 = arg->si3;
    sl3 = arg->sl3;
    sgh3 = arg->sgh3;
    sh3 = arg->sh3;
    sl4 = arg->sl4;
    sgh4 = arg->sgh4;
    ssl = arg->ssl;
    ssh = arg->ssh;
    d3210 = arg->d3210;
    d3222 = arg->d3222;
    d4410 = arg->d4410;
    d4422 = arg->d4422;
    d5220 = arg->d5220;
    d5232 = arg->d5232;
    d5421 = arg->d5421;
    d5433 = arg->d5433;
    del1 = arg->del1;
    del2 = arg->del2;
    del3 = arg->del3;
    fasx2 = arg->fasx2;
    fasx4 = arg->fasx4;
    fasx6 = arg->fasx6;
    xlamo = arg->xlamo;
    xfact = arg->xfact;
    xni = arg->xni;
    atime = arg->atime;
    stepp = arg->stepp;
    stepn = arg->stepn;
    step2 = arg->step2;
    preep = arg->preep;
    pl = arg->pl;
    sghs = arg->sghs;
    xli = arg->xli;
    d2201 = arg->d2201;
    d2211 = arg->d2211;
    sghl = arg->sghl;
    sh1 = arg->sh1;
    pinc = arg->pinc;
    pe = arg->pe;
    shs = arg->shs;
    zsingl = arg->zsingl;
    zcosgl = arg->zcosgl;
    zsinhl = arg->zsinhl;
    zcoshl = arg->zcoshl;
    zsinil = arg->zsinil;
    zcosil = arg->zcosil;
      
    switch ( ientry )
    {
        case dpinit: // Entrance for deep space initialization
            
            thgr = thetag ( jdepoch, arg );
            eq = eo;
            xnq = arg->xnodp;
            aqnv = 1/arg->aodp;
            xqncl = xincl;
            xmao = xmo;
            xpidot = arg->omgdot+arg->xnodot;
            sinq = sin(xnodeo);
            cosq = cos(xnodeo);
            omegaq = omegao;

            // Initialize lunar solar terms
            
            day = arg->ds50+18261.5;  // Days since 1900 Jan 0.5
            if (day != preep)
            {
                preep = day;
                xnodce = 4.5236020-9.2422029E-4*day;
                stem = sin(xnodce);
                ctem = cos(xnodce);
                zcosil = 0.91375164-0.03568096*ctem;
                zsinil = sqrt(1-zcosil*zcosil);
                zsinhl = 0.089683511*stem/zsinil;
                zcoshl = sqrt(1-zsinhl*zsinhl);
                c = 4.7199672+0.22997150*day;
                gam = 5.8351514+0.0019443680*day;
                zmol = fmod2p ( c - gam );
                zx = 0.39785416*stem/zsinil;
                zy = zcoshl*ctem+0.91744867*zsinhl*stem;
                zx = actan(zx,zy);
                zx = gam+zx-xnodce;
                zcosgl = cos(zx);
                zsingl = sin(zx);
                zmos = 6.2565837+0.017201977*day;
                zmos = fmod2p(zmos);
            }

            // Do solar terms
            
            savtsn = 1E20;
            zcosg = zcosgs;
            zsing = zsings;
            zcosi = zcosis;
            zsini = zsinis;
            zcosh = cosq;
            zsinh = sinq;
            cc = c1ss;
            zn = zns;
            ze = zes;
            zmo = zmos;
            xnoi = 1/xnq;

            // Loop breaks when Solar terms are done a second
            // time, after Lunar terms are initialized

            ls = 0;
            for(;;)
            {
                 // Solar terms done again after Lunar terms are done
                
                 a1 = zcosg*zcosh+zsing*zcosi*zsinh;
                 a3 = -zsing*zcosh+zcosg*zcosi*zsinh;
                 a7 = -zcosg*zsinh+zsing*zcosi*zcosh;
                 a8 = zsing*zsini;
                 a9 = zsing*zsinh+zcosg*zcosi*zcosh;
                 a10 = zcosg*zsini;
                 a2 = arg->cosio*a7+ arg->sinio*a8;
                 a4 = arg->cosio*a9+ arg->sinio*a10;
                 a5 = -arg->sinio*a7+ arg->cosio*a8;
                 a6 = -arg->sinio*a9+ arg->cosio*a10;
                 x1 = a1*arg->cosg+a2*arg->sing;
                 x2 = a3*arg->cosg+a4*arg->sing;
                 x3 = -a1*arg->sing+a2*arg->cosg;
                 x4 = -a3*arg->sing+a4*arg->cosg;
                 x5 = a5*arg->sing;
                 x6 = a6*arg->sing;
                 x7 = a5*arg->cosg;
                 x8 = a6*arg->cosg;
                 z31 = 12*x1*x1-3*x3*x3;
                 z32 = 24*x1*x2-6*x3*x4;
                 z33 = 12*x2*x2-3*x4*x4;
                 z1 = 3*(a1*a1+a2*a2)+z31*arg->eosq;
                 z2 = 6*(a1*a3+a2*a4)+z32*arg->eosq;
                 z3 = 3*(a3*a3+a4*a4)+z33*arg->eosq;
                 z11 = -6*a1*a5+arg->eosq*(-24*x1*x7-6*x3*x5);
                 z12 = -6*(a1*a6+a3*a5)+ arg->eosq*(-24*(x2*x7+x1*x8)-6*(x3*x6+x4*x5));
                 z13 = -6*a3*a6+arg->eosq*(-24*x2*x8-6*x4*x6);
                 z21 = 6*a2*a5+arg->eosq*(24*x1*x5-6*x3*x7);
                 z22 = 6*(a4*a5+a2*a6)+ arg->eosq*(24*(x2*x5+x1*x6)-6*(x4*x7+x3*x8));
                 z23 = 6*a4*a6+arg->eosq*(24*x2*x6-6*x4*x8);
                 z1 = z1+z1+arg->betao2*z31;
                 z2 = z2+z2+arg->betao2*z32;
                 z3 = z3+z3+arg->betao2*z33;
                 s3 = cc*xnoi;
                 s2 = -0.5*s3/arg->betao;
                 s4 = s3*arg->betao;
                 s1 = -15*eq*s4;
                 s5 = x1*x3+x2*x4;
                 s6 = x2*x3+x1*x4;
                 s7 = x2*x4-x1*x3;
                 se = s1*zn*s5;
                 si = s2*zn*(z11+z13);
                 sl = -zn*s3*(z1+z3-14-6*arg->eosq);
                 sgh = s4*zn*(z31+z33-6);
                 sh = -zn*s2*(z21+z23);
                 if (xqncl < 5.2359877E-2)
                    sh = 0;
                 ee2 = 2*s1*s6;
                 e3 = 2*s1*s7;
                 xi2 = 2*s2*z12;
                 xi3 = 2*s2*(z13-z11);
                 xl2 = -2*s3*z2;
                 xl3 = -2*s3*(z3-z1);
                 xl4 = -2*s3*(-21-9*arg->eosq)*ze;
                 xgh2 = 2*s4*z32;
                 xgh3 = 2*s4*(z33-z31);
                 xgh4 = -18*s4*ze;
                 xh2 = -2*s2*z22;
                 xh3 = -2*s2*(z23-z21);

                 if(ls == 1)
                     break;

                 // Do lunar terms
                
                 sse = se;
                 ssi = si;
                 ssl = sl;
                 ssh = sh/arg->sinio;
                 ssg = sgh-arg->cosio*ssh;
                 se2 = ee2;
                 si2 = xi2;
                 sl2 = xl2;
                 sgh2 = xgh2;
                 sh2 = xh2;
                 se3 = e3;
                 si3 = xi3;
                 sl3 = xl3;
                 sgh3 = xgh3;
                 sh3 = xh3;
                 sl4 = xl4;
                 sgh4 = xgh4;
                 zcosg = zcosgl;
                 zsing = zsingl;
                 zcosi = zcosil;
                 zsini = zsinil;
                 zcosh = zcoshl*cosq+zsinhl*sinq;
                 zsinh = sinq*zcoshl-cosq*zsinhl;
                 zn = znl;
                 cc = c1l;
                 ze = zel;
                 zmo = zmol;
                 ls = 1;
            } // End of for(;;)

            sse = sse+se;
            ssi = ssi+si;
            ssl = ssl+sl;
            ssg = ssg+sgh-arg->cosio/arg->sinio*sh;
            ssh = ssh+sh/arg->sinio;

            // Geopotential resonance initialization for 12 hour orbits
            
            arg->iresfl = 0;
            arg->isynfl = 0;

            if( !((xnq < 0.0052359877) && (xnq > 0.0034906585)) )
            {
                if( (xnq < 0.00826) || (xnq > 0.00924) )
                    break;
                if (eq < 0.5)
                    break;
                arg->iresfl = 1;
                eoc = eq*arg->eosq;
                g201 = -0.306-(eq-0.64)*0.440;
                
                if (eq <= 0.65)
                {
                    g211 = 3.616-13.247*eq+16.290*arg->eosq;
                    g310 = -19.302+117.390*eq-228.419*arg->eosq+156.591*eoc;
                    g322 = -18.9068+109.7927*eq-214.6334*arg->eosq+146.5816*eoc;
                    g410 = -41.122+242.694*eq-471.094*arg->eosq+313.953*eoc;
                    g422 = -146.407+841.880*eq-1629.014*arg->eosq+1083.435*eoc;
                    g520 = -532.114+3017.977*eq-5740*arg->eosq+3708.276*eoc;
                }
                else
                {
                    g211 = -72.099+331.819*eq-508.738*arg->eosq+266.724*eoc;
                    g310 = -346.844+1582.851*eq-2415.925*arg->eosq+1246.113*eoc;
                    g322 = -342.585+1554.908*eq-2366.899*arg->eosq+1215.972*eoc;
                    g410 = -1052.797+4758.686*eq-7193.992*arg->eosq+3651.957*eoc;
                    g422 = -3581.69+16178.11*eq-24462.77*arg->eosq+ 12422.52*eoc;
                    if (eq <= 0.715)
                        g520 = 1464.74-4664.75*eq+3763.64*arg->eosq;
                    else
                        g520 = -5149.66+29936.92*eq-54087.36*arg->eosq+31324.56*eoc;
                } // End if (eq <= 0.65)

                if (eq < 0.7)
                {
                    g533 = -919.2277+4988.61*eq-9064.77*arg->eosq+5542.21*eoc;
                    g521 = -822.71072+4568.6173*eq-8491.4146*arg->eosq+5337.524*eoc;
                    g532 = -853.666+4690.25*eq-8624.77*arg->eosq+ 5341.4*eoc;
                }
                else
                {
                    g533 = -37995.78+161616.52*eq-229838.2*arg->eosq+109377.94*eoc;
                    g521 = -51752.104+218913.95*eq-309468.16*arg->eosq+146349.42*eoc;
                    g532 = -40023.88+170470.89*eq-242699.48*arg->eosq+115605.82*eoc;
                } // End if (eq <= 0.7)

                sini2 = arg->sinio*arg->sinio;
                f220 = 0.75*(1+2*arg->cosio+arg->theta2);
                f221 = 1.5*sini2;
                f321 = 1.875*arg->sinio*(1-2*arg->cosio-3*arg->theta2);
                f322 = -1.875*arg->sinio*(1+2*arg->cosio-3*arg->theta2);
                f441 = 35*sini2*f220;
                f442 = 39.3750*sini2*sini2;
                f522 = 9.84375*arg->sinio*(sini2*(1-2*arg->cosio-5*
                       arg->theta2)+0.33333333*(-2+4*arg->cosio+
                       6*arg->theta2));
                f523 = arg->sinio*(4.92187512*sini2*(-2-4*
                       arg->cosio+10*arg->theta2)+6.56250012
                       *(1+2*arg->cosio-3*arg->theta2));
                f542 = 29.53125*arg->sinio*(2-8*
                       arg->cosio+arg->theta2*
                       (-12+8*arg->cosio+10*arg->theta2));
                f543 = 29.53125*arg->sinio*(-2-8*arg->cosio+
                       arg->theta2*(12+8*arg->cosio-10*arg->theta2));
                xno2 = xnq*xnq;
                ainv2 = aqnv*aqnv;
                temp1 = 3*xno2*ainv2;
                temp = temp1*root22;
                d2201 = temp*f220*g201;
                d2211 = temp*f221*g211;
                temp1 = temp1*aqnv;
                temp = temp1*root32;
                d3210 = temp*f321*g310;
                d3222 = temp*f322*g322;
                temp1 = temp1*aqnv;
                temp = 2*temp1*root44;
                d4410 = temp*f441*g410;
                d4422 = temp*f442*g422;
                temp1 = temp1*aqnv;
                temp = temp1*root52;
                d5220 = temp*f522*g520;
                d5232 = temp*f523*g532;
                temp = 2*temp1*root54;
                d5421 = temp*f542*g521;
                d5433 = temp*f543*g533;
                xlamo = xmao+xnodeo+xnodeo-thgr-thgr;
                bfact = arg->xmdot+arg->xnodot+arg->xnodot-thdt-thdt;
                bfact = bfact+ssl+ssh+ssh;
            } // if( !(xnq < 0.0052359877) && (xnq > 0.0034906585) )
            else
            {
                arg->iresfl = 1;
                arg->isynfl = 1;
                // Synchronous resonance terms initialization
                g200 = 1+arg->eosq*(-2.5+0.8125*arg->eosq);
                g310 = 1+2*arg->eosq;
                g300 = 1+arg->eosq*(-6+6.60937*arg->eosq);
                f220 = 0.75*(1+arg->cosio)*(1+arg->cosio);
                f311 = 0.9375*arg->sinio*arg->sinio*(1+3*arg->cosio)-0.75*(1+arg->cosio);
                f330 = 1+arg->cosio;
                f330 = 1.875*f330*f330*f330;
                del1 = 3*xnq*xnq*aqnv*aqnv;
                del2 = 2*del1*f220*g200*q22;
                del3 = 3*del1*f330*g300*q33*aqnv;
                del1 = del1*f311*g310*q31*aqnv;
                fasx2 = 0.13130908;
                fasx4 = 2.8843198;
                fasx6 = 0.37448087;
                xlamo = xmao+xnodeo+omegao-thgr;
                bfact = arg->xmdot+xpidot-thdt;
                bfact = bfact+ssl+ssg+ssh;
            } // End if( !(xnq < 0.0052359877) && (xnq > 0.0034906585) )

            xfact = bfact-xnq;

            // Initialize integrator
            xli = xlamo;
            xni = xnq;
            atime = 0;
            stepp = 720;
            stepn = -720;
            step2 = 259200;
            break; // End case dpinit:

        case dpsec: // Entrance for deep space secular effects
            
            arg->xll = arg->xll+ssl*arg->t;
            arg->omgadf = arg->omgadf+ssg*arg->t;
            arg->xnode = arg->xnode+ssh*arg->t;
            arg->em = eo+sse*arg->t;
            arg->xinc = xincl+ssi*arg->t;
            if (arg->xinc < 0)
            {
                arg->xinc = -arg->xinc;
                arg->xnode = arg->xnode + xpi;
                arg->omgadf = arg->omgadf-xpi;
            }
            
            if( arg->iresfl == 0 )
                break;

            do
            {
                if((atime == 0) || ((arg->t >= 0) && (atime < 0)) || ((arg->t < 0) && (atime >= 0)) )
                {
                    // Epoch restart
                    if( arg->t >= 0 )
                        delt = stepp;
                    else
                        delt = stepn;

                    atime = 0;
                    xni = xnq;
                    xli = xlamo;
                }
                else
                {
                    if( fabs(arg->t) >= fabs(atime) )
                    {
                        if ( arg->t > 0 )
                            delt = stepp;
                        else
                            delt = stepn;
                    }
                }

                do
                {
                    if ( fabs(arg->t-atime) >= stepp )
                    {
                        dl = 1;
                        er = 0;
                    }
                    else
                    {
                        ft = arg->t-atime;
                        dl = 0;
                        er = 0;
                    }

                    if( fabs(arg->t) < fabs(atime) )
                    {
                        if (arg->t >= 0)
                            delt = stepn;
                        else
                            delt = stepp;
                        dl = 1;
                        er = 1;
                    }

                    // Dot terms calculated
                    if( arg->isynfl )
                    {
                        xndot = del1*sin(xli-fasx2)+del2*sin(2*(xli-fasx4))+del3*sin(3*(xli-fasx6));
                        xnddt = del1*cos(xli-fasx2)+2*del2*cos(2*(xli-fasx4))+3*del3*cos(3*(xli-fasx6));
                    }
                    else
                    {
                        xomi = omegaq+arg->omgdot*atime;
                        x2omi = xomi+xomi;
                        x2li = xli+xli;
                        xndot = d2201*sin(x2omi+xli-g22)
                               +d2211*sin(xli-g22)
                               +d3210*sin(xomi+xli-g32)
                               +d3222*sin(-xomi+xli-g32)
                               +d4410*sin(x2omi+x2li-g44)
                               +d4422*sin(x2li-g44)
                               +d5220*sin(xomi+xli-g52)
                               +d5232*sin(-xomi+xli-g52)
                               +d5421*sin(xomi+x2li-g54)
                               +d5433*sin(-xomi+x2li-g54);
                        xnddt = d2201*cos(x2omi+xli-g22)
                               +d2211*cos(xli-g22)
                               +d3210*cos(xomi+xli-g32)
                               +d3222*cos(-xomi+xli-g32)
                               +d5220*cos(xomi+xli-g52)
                               +d5232*cos(-xomi+xli-g52)
                               +2*(d4410*cos(x2omi+x2li-g44)
                               +d4422*cos(x2li-g44)
                               +d5421*cos(xomi+x2li-g54)
                               +d5433*cos(-xomi+x2li-g54));
                    } // End of if (arg->isynfl)

                    xldot = xni+xfact;
                    xnddt = xnddt*xldot;

                    if(dl == 1)
                    {
                        xli = xli+xldot*delt+xndot*step2;
                        xni = xni+xndot*delt+xnddt*step2;
                        atime = atime+delt;
                    }
                }
                while(dl == 1 && er == 0);
            }
            while(dl == 1 && er == 1);

            arg->xn = xni+xndot*ft+xnddt*ft*ft*0.5;
            xl = xli+xldot*ft+xndot*ft*ft*0.5;
            temp = -arg->xnode+thgr+arg->t*thdt;

            if (arg->isynfl == 0)
                arg->xll = xl+temp+temp;
            else
                arg->xll = xl-arg->omgadf+temp;

            break; // End case dpsec

        case dpper: // Entrance for lunar-solar periodics
            
            sinis = sin(arg->xinc);
            cosis = cos(arg->xinc);
            if (fabs(savtsn-arg->t) >= 30)
            {
                savtsn = arg->t;
                zm = zmos+zns*arg->t;
                zf = zm+2*zes*sin(zm);
                sinzf = sin(zf);
                f2 = 0.5*sinzf*sinzf-0.25;
                f3 = -0.5*sinzf*cos(zf);
                ses = se2*f2+se3*f3;
                sis = si2*f2+si3*f3;
                sls = sl2*f2+sl3*f3+sl4*sinzf;
                sghs = sgh2*f2+sgh3*f3+sgh4*sinzf;
                shs = sh2*f2+sh3*f3;
                zm = zmol+znl*arg->t;
                zf = zm+2*zel*sin(zm);
                sinzf = sin(zf);
                f2 = 0.5*sinzf*sinzf-0.25;
                f3 = -0.5*sinzf*cos(zf);
                sel = ee2*f2+e3*f3;
                sil = xi2*f2+xi3*f3;
                sll = xl2*f2+xl3*f3+xl4*sinzf;
                sghl = xgh2*f2+xgh3*f3+xgh4*sinzf;
                sh1 = xh2*f2+xh3*f3;
                pe = ses+sel;
                pinc = sis+sil;
                pl = sls+sll;
            }

            pgh = sghs+sghl;
            ph = shs+sh1;
            arg->xinc = arg->xinc+pinc;
            arg->em = arg->em+pe;

            if (xqncl >= 0.2)
            {
                // Apply periodics directly
                
                ph = ph/arg->sinio;
                pgh = pgh-arg->cosio*ph;
                arg->omgadf = arg->omgadf+pgh;
                arg->xnode = arg->xnode+ph;
                arg->xll = arg->xll+pl;
            }
            else
            {
                // Apply periodics with Lyddane modification
                
                sinok = sin(arg->xnode);
                cosok = cos(arg->xnode);
                alfdp = sinis*sinok;
                betdp = sinis*cosok;
                dalf = ph*cosok+pinc*cosis*sinok;
                dbet = -ph*sinok+pinc*cosis*cosok;
                alfdp = alfdp+dalf;
                betdp = betdp+dbet;
                arg->xnode = fmod2p ( arg->xnode );
                xls = arg->xll+arg->omgadf+cosis*arg->xnode;
                dls = pl+pgh-pinc*arg->xnode*sinis;
                xls = xls+dls;
                xnoh = arg->xnode;
                arg->xnode = actan (alfdp,betdp);

                // This is a patch to Lyddane modification
                // suggested by Rob Matson.
                
                if(fabs(xnoh-arg->xnode) > xpi)
                {
                  if(arg->xnode < xnoh)
                    arg->xnode +=twopi;
                  else
                    arg->xnode -=twopi;
                }

                arg->xll = arg->xll+pl;
                arg->omgadf = xls-arg->xll-cos(arg->xinc)*arg->xnode;
            }

            break; // End case dpper
    } // End switch(ientry)

    // Store variables for future use
    
    arg->thgr = thgr;
    arg->xnq = xnq;
    arg->xqncl = xqncl;
    arg->omegaq = omegaq;
    arg->zmol = zmol;
    arg->zmos = zmos;
    arg->savtsn = savtsn;
    arg->ee2 = ee2;
    arg->e3 = e3;
    arg->xi2 = xi2;
    arg->xl2 = xl2;
    arg->xl3 = xl3;
    arg->xl4 = xl4;
    arg->xgh2 = xgh2;
    arg->xgh3 = xgh3;
    arg->xgh4 = xgh4;
    arg->xh2 = xh2;
    arg->xh3 = xh3;
    arg->sse = sse;
    arg->ssi = ssi;
    arg->ssg = ssg;
    arg->xi3 = xi3;
    arg->se2 = se2;
    arg->si2 = si2;
    arg->sl2 = sl2;
    arg->sgh2 = sgh2;
    arg->sh2 = sh2;
    arg->se3 = se3;
    arg->si3 = si3;
    arg->sl3 = sl3;
    arg->sgh3 = sgh3;
    arg->sh3 = sh3;
    arg->sl4 = sl4;
    arg->sgh4 = sgh4;
    arg->ssl = ssl;
    arg->ssh = ssh;
    arg->d3210 = d3210;
    arg->d3222 = d3222;
    arg->d4410 = d4410;
    arg->d4422 = d4422;
    arg->d5220 = d5220;
    arg->d5232 = d5232;
    arg->d5421 = d5421;
    arg->d5433 = d5433;
    arg->del1 = del1;
    arg->del2 = del2;
    arg->del3 = del3;
    arg->fasx2 = fasx2;
    arg->fasx4 = fasx4;
    arg->fasx6 = fasx6;
    arg->xlamo = xlamo;
    arg->xfact = xfact;
    arg->xni = xni;
    arg->atime = atime;
    arg->stepp = stepp;
    arg->stepn = stepn;
    arg->step2 = step2;
    arg->preep = preep;
    arg->pl = pl;
    arg->sghs = sghs;
    arg->xli = xli;
    arg->d2201 = d2201;
    arg->d2211 = d2211;
    arg->sghl = sghl;
    arg->sh1 = sh1;
    arg->pinc = pinc;
    arg->pe = pe;
    arg->shs = shs;
    arg->zsingl = zsingl;
    arg->zcosgl = zcosgl;
    arg->zsinhl = zsinhl;
    arg->zcoshl = zcoshl;
    arg->zsinil = zsinil;
    arg->zcosil = zcosil;
}

// SGP orbit model. Computes satellite position and velocity
// in the Earth-centered, inertial equatorial reference frame,
// in units of Earth-radii and Earth-radii per minute.
// Elapsed time since orbital element epoch (tsince) is in minutes.
// A simpler and faster (but less accurate) orbit model than SGP4/SDP4.

void SSTLE::sgp ( double tsince, SSVector &pos, SSVector &vel )
{
    double ao,qo,xlo,d1o,d2o,d3o,d4o,omgdt,xnodot,c5,c6;

    double temp, rdot, cosu, sinu, cos2u, po2no, sin2u, a, e,
           p, rr, u, ecose, esine, omgas, cosik, cosio, xinck,
           sinik, sinio, a1, c1, c2, c3, c4, d1, axnsl, aynsl,
           sinuk, rvdot, cosuk,dd1, dd2, coseo1, sineo1, pl,
           rk, po, uk, xl, su, ux, uy, uz, vx, vy, vz, pl2,
           xnodek, cosnok, xnodes, el2, eo1, r1, sinnok,
           xls, xmx, xmy, tem2, tem5;

    int i;

    if ( argp.sgp == nullptr )
    {
        // Initialization

        sgp_args *arg = argp.sgp = new sgp_args;

        c1 = ck2 * 1.5;
        c2 = ck2 / 4.0;
        c3 = ck2 / 2.0;
        r1 = xae;
        c4 = xj3 * ( r1 * ( r1 * r1 ) ) / ( ck2 * 4.0 );
        cosio = cos ( xincl );
        sinio = sin ( xincl );
        dd1 = xke / xno;
        dd2 = tothrd;
        a1 = pow ( dd1, dd2 );
        dd1 = ( 1.0 - eo * eo );
        d1 = c1 / a1 / a1 * ( cosio * 3.0 * cosio - 1.0 ) / pow ( dd1, 1.5 );
        ao = a1 * ( 1.0 - d1 * 0.33333333333333331 - d1 * d1 - d1 * 1.654320987654321 * d1 * d1 );
        po = ao * ( 1.0 - eo * eo );
        qo = ao * ( 1.0 - eo );
        xlo = xmo + omegao + xnodeo;
        d1o = c3 * sinio * sinio;
        d2o = c2 * ( cosio * 7.0 * cosio - 1.0 );
        d3o = c1 * cosio;
        d4o = d3o * sinio;
        po2no = xno / ( po * po );
        omgdt = c1 * po2no * ( cosio * 5.0 * cosio -1.0 );
        xnodot = d3o * -2.0 * po2no;
        c5 = c4 * 0.5 * sinio * ( cosio * 5.0 + 3.0 ) / ( cosio + 1.0 );
        c6 = c4 * sinio;

        // End of SGP initialization, save variables for further use

        arg->ao = ao;
        arg->qo = qo;
        arg->xlo = xlo;
        arg->d1o = d1o;
        arg->d2o = d2o;
        arg->d3o = d3o;
        arg->d4o = d4o;
        arg->omgdt = omgdt;
        arg->xnodot = xnodot;
        arg->c5 = c5;
        arg->c6 = c6;
    }
    else
    {
        // Recover saved variables
        
        sgp_args *arg = argp.sgp;

        ao = arg->ao;
        qo = arg->qo;
        xlo = arg->xlo;
        d1o = arg->d1o;
        d2o = arg->d2o;
        d3o = arg->d3o;
        d4o = arg->d4o;
        omgdt = arg->omgdt;
        xnodot = arg->xnodot;
        c5 = arg->c5;
        c6 = arg->c6;
    }
    
    // Update for secular gravity and atmospheric drag
    
    a = xno + ( xndt2o * 2.0 + xndd6o * 3.0 * tsince ) * tsince;
    dd1 = xno / a;
    dd2 = tothrd;
    a = ao * pow ( dd1, dd2 );
    e = e6a;
    if ( a > qo )
        e = 1.0 - qo / a;
    p = a * ( 1.0 - e * e );
    xnodes = xnodeo + xnodot * tsince;
    omgas = omegao + omgdt * tsince;
    r1 = xlo + ( xno + omgdt + xnodot + ( xndt2o + xndd6o * tsince ) * tsince ) * tsince;
    xls = fmod2p ( r1 );

    // Long period periodics
    
    axnsl = e * cos( omgas);
    aynsl = e * sin ( omgas ) - c6 / p;
    r1 = xls - c5 / p * axnsl;
    xl = fmod2p ( r1 );

    // Solve Kepler's equation
    
    r1 = xl - xnodes;
    u = fmod2p ( r1 );
    eo1 = u;
    tem5 = 1.0;

    i = 0;
    do
    {
        sineo1 = sin ( eo1 );
        coseo1 = cos ( eo1 );
        if ( fabs ( tem5 ) < e6a )
          break;
        
        tem5 = 1.0 - coseo1 * axnsl - sineo1 * aynsl;
        tem5 = ( u - aynsl * coseo1 + axnsl * sineo1 - eo1 ) / tem5;
        tem2 = fabs ( tem5 );
        if ( tem2 > 1.0 )
            tem5 = tem2/tem5;
        eo1 += tem5;
    }
    while ( i++ < 10 );

    // Short period preliminary quantities
    
    ecose = axnsl * coseo1 + aynsl * sineo1;
    esine = axnsl * sineo1 - aynsl * coseo1;
    el2 = axnsl * axnsl + aynsl * aynsl;
    pl = a * ( 1.0 - el2 );
    pl2 = pl * pl;
    rr = a * ( 1.0 - ecose );
    rdot = xke * sqrt ( a ) / rr * esine;
    rvdot = xke * sqrt ( pl ) / rr;
    temp = esine / ( sqrt ( 1.0 - el2 ) + 1.0 );
    sinu = a / rr * ( sineo1 - aynsl - axnsl * temp );
    cosu = a / rr * ( coseo1 - axnsl + aynsl * temp );
    su = actan ( sinu, cosu );

    // Update for short periodics
    
    sin2u = ( cosu + cosu ) * sinu;
    cos2u = 1.0 - 2.0 * sinu * sinu;
    rk = rr + d1o / pl * cos2u;
    uk = su - d2o / pl2 * sin2u;
    xnodek = xnodes + d3o * sin2u / pl2;
    xinck = xincl + d4o / pl2 * cos2u;

    // Orientation vectors
    
    sinuk = sin ( uk );
    cosuk = cos ( uk );
    sinnok = sin ( xnodek );
    cosnok = cos ( xnodek );
    sinik = sin ( xinck );
    cosik = cos ( xinck );
    xmx = -sinnok * cosik;
    xmy = cosnok * cosik;
    ux = xmx * sinuk + cosnok * cosuk;
    uy = xmy * sinuk + sinnok * cosuk;
    uz = sinik * sinuk;
    vx = xmx * cosuk - cosnok * sinuk;
    vy = xmy * cosuk - sinnok * sinuk;
    vz = sinik * cosuk;

    // Position and velocity

    pos.x = rk * ux;
    pos.y = rk * uy;
    pos.z = rk * uz;
    
    vel.x = rdot * ux;
    vel.y = rdot * uy;
    vel.z = rdot * uz;
    
    vel.x = rvdot * vx + vel.x;
    vel.y = rvdot * vy + vel.y;
    vel.z = rvdot * vz + vel.z;
}

// SGP4 orbit model. Computes satellite position and velocity
// in Earth-centered, inertial equatorial reference frame,
// in units of Earth-radii and Earth-radii per minute.
// Elapsed time since orbital element epoch (tsince) is in minutes.
// Use this for near-Earth satellites with orbit periods < 225 minutes.

void SSTLE::sgp4 ( double tsince, SSVector &pos, SSVector &vel )
{
    double aodp,aycof,c1,c4,c5,cosio,d2,d3,d4,delmo,omgcof,
           eta,omgdot,sinio,xnodp,sinmo,t2cof,t3cof,t4cof,t5cof,
           x1mth2,x3thm1,x7thm1,xmcof,xmdot,xnodcf,xnodot,xlcof;
 
    double cosuk,sinuk,rfdotk,vx,vy,vz,ux,uy,uz,xmy,xmx,
           cosnok,sinnok,cosik,sinik,rdotk,xinck,xnodek,uk,
           rk,cos2u,sin2u,u,sinu,cosu,betal,rfdot,rdot,r,pl,
           elsq,esine,ecose,epw,cosepw,x1m5th,xhdot1,tfour,
           sinepw,capu,ayn,xlt,aynl,xll,axn,xn,beta,xl,e,a,
           tcube,delm,delomg,templ,tempe,tempa,xnode,tsq,xmp,
           omega,xnoddf,omgadf,xmdf,a1,a3ovk2,ao,betao,betao2,
           c1sq,c2,c3,coef,coef1,del1,delo,eeta,eosq,etasq,
           perige,pinvsq,psisq,qoms24,s4,temp,temp1,temp2,
           temp3,temp4,temp5,temp6,theta2,theta4,tsi;

    int i;

    sgp4_args *arg;
    
    if ( argp.sgp4 == nullptr )
    {
        // Initialization

        arg = argp.sgp4 = new sgp4_args;

        // Recover original mean motion (xnodp) and
        // semimajor axis (aodp) from input elements.
        
        a1 = pow(xke/xno,tothrd);
        cosio = cos(xincl);
        theta2 = cosio*cosio;
        x3thm1 = 3*theta2-1.0;
        eosq = eo*eo;
        betao2 = 1-eosq;
        betao = sqrt(betao2);
        del1 = 1.5*ck2*x3thm1/(a1*a1*betao*betao2);
        ao = a1*(1-del1*(0.5*tothrd+del1*(1+134/81*del1)));
        delo = 1.5*ck2*x3thm1/(ao*ao*betao*betao2);
        xnodp = xno/(1+delo);
        aodp = ao/(1-delo);

        // For perigee less than 220 kilometers, the "simple" flag is set
        // and the equations are truncated to linear variation in sqrt a
        // and quadratic variation in mean anomaly.  Also, the c3 term,
        // the delta omega term, and the delta m term are dropped.
        
        if((aodp*(1-eo)/xae) < (220/xkmper+xae))
            arg->isimp = 1;
        else
            arg->isimp = 0;

        // For perigee below 156 km, the
        // values of s and qoms2t are altered.
        
        s4 = s;
        qoms24 = qoms2t;
        perige = (aodp*(1-eo)-xae)*xkmper;
        if(perige < 156)
        {
            if(perige <= 98)
                s4 = 20;
            else
                s4 = perige-78;
            qoms24 = pow((120-s4)*xae/xkmper,4);
            s4 = s4/xkmper+xae;
        }

        pinvsq = 1/(aodp*aodp*betao2*betao2);
        tsi = 1/(aodp-s4);
        eta = aodp*eo*tsi;
        etasq = eta*eta;
        eeta = eo*eta;
        psisq = fabs(1-etasq);
        coef = qoms24*pow(tsi,4);
        coef1 = coef/pow(psisq,3.5);
        c2 = coef1*xnodp*(aodp*(1+1.5*etasq+eeta*(4+etasq))+
        0.75*ck2*tsi/psisq*x3thm1*(8+3*etasq*(8+etasq)));
        c1 = bstar*c2;
        sinio = sin(xincl);
        a3ovk2 = -xj3/ck2*pow(xae,3);
        c3 = coef*tsi*a3ovk2*xnodp*xae*sinio/eo;
        x1mth2 = 1-theta2;
        c4 = 2*xnodp*coef1*aodp*betao2*(eta*(2+0.5*etasq)+
        eo*(0.5+2*etasq)-2*ck2*tsi/(aodp*psisq)*
        (-3*x3thm1*(1-2*eeta+etasq*(1.5-0.5*eeta))+0.75*
        x1mth2*(2*etasq-eeta*(1+etasq))*cos(2*omegao)));
        c5 = 2*coef1*aodp*betao2*(1+2.75*(etasq+eeta)+eeta*etasq);
        theta4 = theta2*theta2;
        temp1 = 3*ck2*pinvsq*xnodp;
        temp2 = temp1*ck2*pinvsq;
        temp3 = 1.25*ck4*pinvsq*pinvsq*xnodp;
        xmdot = xnodp+0.5*temp1*betao*x3thm1+0.0625*temp2*betao*(13-78*theta2+137*theta4);
        x1m5th = 1-5*theta2;
        omgdot = -0.5*temp1*x1m5th+0.0625*temp2*(7-114*theta2+395*theta4)+temp3*(3-36*theta2+49*theta4);
        xhdot1 = -temp1*cosio;
        xnodot = xhdot1+(0.5*temp2*(4-19*theta2)+2*temp3*(3-7*theta2))*cosio;
        omgcof = bstar*c3*cos(omegao);
        xmcof = -tothrd*coef*bstar*xae/eeta;
        xnodcf = 3.5*betao2*xhdot1*c1;
        t2cof = 1.5*c1;
        xlcof = 0.125*a3ovk2*sinio*(3+5*cosio)/(1+cosio);
        aycof = 0.25*a3ovk2*sinio;
        delmo = pow(1+eta*cos(xmo),3);
        sinmo = sin(xmo);
        x7thm1 = 7*theta2-1;
        
        if (arg->isimp == 0)
        {
            c1sq = c1*c1;
            d2 = 4*aodp*tsi*c1sq;
            temp = d2*tsi*c1/3;
            d3 = (17*aodp+s4)*temp;
            d4 = 0.5*temp*aodp*tsi*(221*aodp+31*s4)*c1;
            t3cof = d2+2*c1sq;
            t4cof = 0.25*(3*d3+c1*(12*d2+10*c1sq));
            t5cof = 0.2*(3*d4+12*c1*d3+6*d2*d2+15*c1sq*(2*d2+c1sq));
        }
        else
        {
            d2 = d3 = d4 = t3cof = t4cof = t5cof = 0.0;
        }

        // End of SGP4 initialization, save variables for further use
        
        arg->aodp = aodp;
        arg->aycof = aycof;
        arg->c1 = c1;
        arg->c4 = c4;
        arg->c5 = c5;
        arg->cosio = cosio;
        arg->d2 = d2;
        arg->d3 = d3;
        arg->d4 = d4;
        arg->delmo = delmo;
        arg->omgcof = omgcof;
        arg->eta = eta;
        arg->omgdot = omgdot;
        arg->sinio = sinio;
        arg->xnodp = xnodp;
        arg->sinmo = sinmo;
        arg->t2cof = t2cof;
        arg->t3cof = t3cof;
        arg->t4cof = t4cof;
        arg->t5cof = t5cof;
        arg->x1mth2 = x1mth2;
        arg->x3thm1 = x3thm1;
        arg->x7thm1 = x7thm1;
        arg->xmcof = xmcof;
        arg->xmdot = xmdot;
        arg->xnodcf = xnodcf;
        arg->xnodot = xnodot;
        arg->xlcof = xlcof;
    }
    else
    {
        // Recover saved variables
        
        arg = argp.sgp4;
        
        aodp = arg->aodp;
        aycof = arg->aycof;
        c1 = arg->c1;
        c4 = arg->c4;
        c5 = arg->c5;
        cosio = arg->cosio;
        d2 = arg->d2;
        d3 = arg->d3;
        d4 = arg->d4;
        delmo = arg->delmo;
        omgcof = arg->omgcof;
        eta = arg->eta;
        omgdot = arg->omgdot;
        sinio = arg->sinio;
        xnodp = arg->xnodp;
        sinmo = arg->sinmo;
        t2cof = arg->t2cof;
        t3cof = arg->t3cof;
        t4cof = arg->t4cof;
        t5cof = arg->t5cof;
        x1mth2 = arg->x1mth2;
        x3thm1 = arg->x3thm1;
        x7thm1 = arg->x7thm1;
        xmcof = arg->xmcof;
        xmdot = arg->xmdot;
        xnodcf = arg->xnodcf;
        xnodot = arg->xnodot;
        xlcof = arg->xlcof;
    }
    
    // Update for secular gravity and atmospheric drag.
    
    xmdf = xmo+xmdot*tsince;
    omgadf = omegao+omgdot*tsince;
    xnoddf = xnodeo+xnodot*tsince;
    omega = omgadf;
    xmp = xmdf;
    tsq = tsince*tsince;
    xnode = xnoddf+xnodcf*tsq;
    tempa = 1-c1*tsince;
    tempe = bstar*c4*tsince;
    templ = t2cof*tsq;

    if (arg->isimp == 0)
    {
        delomg = omgcof*tsince;
        delm = xmcof*(pow(1+eta*cos(xmdf),3)-delmo);
        temp = delomg+delm;
        xmp = xmdf+temp;
        omega = omgadf-temp;
        tcube = tsq*tsince;
        tfour = tsince*tcube;
        tempa = tempa-d2*tsq-d3*tcube-d4*tfour;
        tempe = tempe+bstar*c5*(sin(xmp)-sinmo);
        templ = templ+t3cof*tcube+tfour*(t4cof+tsince*t5cof);
    }

    a = aodp*pow(tempa,2);
    e = eo-tempe;
    xl = xmp+omega+xnode+xnodp*templ;
    beta = sqrt(1-e*e);
    xn = xke/pow(a,1.5);

    // Long period periodics
    
    axn = e*cos(omega);
    temp = 1/(a*beta*beta);
    xll = temp*xlcof*axn;
    aynl = temp*aycof;
    xlt = xl+xll;
    ayn = e*sin(omega)+aynl;

    // Solve Kepler's Equation
    
    capu = fmod2p(xlt-xnode);
    temp2 = capu;

    i = 0;
    do
    {
        sinepw = sin(temp2);
        cosepw = cos(temp2);
        temp3 = axn*sinepw;
        temp4 = ayn*cosepw;
        temp5 = axn*cosepw;
        temp6 = ayn*sinepw;
        epw = (capu-temp4+temp3-temp2)/(1-temp5-temp6)+temp2;
        if(fabs(epw-temp2) <= e6a)
            break;
        temp2 = epw;
    }
    while ( i++ < 10 );

    // Short period preliminary quantities

    ecose = temp5+temp6;
    esine = temp3-temp4;
    elsq = axn*axn+ayn*ayn;
    temp = 1-elsq;
    pl = a*temp;
    r = a*(1-ecose);
    temp1 = 1/r;
    rdot = xke*sqrt(a)*esine*temp1;
    rfdot = xke*sqrt(pl)*temp1;
    temp2 = a*temp1;
    betal = sqrt(temp);
    temp3 = 1/(1+betal);
    cosu = temp2*(cosepw-axn+ayn*esine*temp3);
    sinu = temp2*(sinepw-ayn-axn*esine*temp3);
    u = actan ( sinu, cosu );
    sin2u = 2*sinu*cosu;
    cos2u = 2*cosu*cosu-1;
    temp = 1/pl;
    temp1 = ck2*temp;
    temp2 = temp1*temp;

    // Update for short periodics
    
    rk = r*(1-1.5*temp2*betal*x3thm1)+0.5*temp1*x1mth2*cos2u;
    uk = u-0.25*temp2*x7thm1*sin2u;
    xnodek = xnode+1.5*temp2*cosio*sin2u;
    xinck = xincl+1.5*temp2*cosio*sinio*cos2u;
    rdotk = rdot-xn*temp1*x1mth2*sin2u;
    rfdotk = rfdot+xn*temp1*(x1mth2*cos2u+1.5*x3thm1);

    // Orientation vectors

    sinuk = sin(uk);
    cosuk = cos(uk);
    sinik = sin(xinck);
    cosik = cos(xinck);
    sinnok = sin(xnodek);
    cosnok = cos(xnodek);
    xmx = -sinnok*cosik;
    xmy = cosnok*cosik;
    ux = xmx*sinuk+cosnok*cosuk;
    uy = xmy*sinuk+sinnok*cosuk;
    uz = sinik*sinuk;
    vx = xmx*cosuk-cosnok*sinuk;
    vy = xmy*cosuk-sinnok*sinuk;
    vz = sinik*cosuk;

    // Position and velocity
    
    pos.x = rk*ux;
    pos.y = rk*uy;
    pos.z = rk*uz;
    
    vel.x = rdotk*ux+rfdotk*vx;
    vel.y = rdotk*uy+rfdotk*vy;
    vel.z = rdotk*uz+rfdotk*vz;
}

// SGP4 orbit model. Computes satellite position and velocity
// in Earth-centered, inertial equatorial reference frame,
// in units of Earth-radii and Earth-radii per minute.
// Elapsed time since orbital element epoch (tsince) is in minutes.
// Use this for deep-space satellites with orbit periods > 225 minutes.

void SSTLE::sdp4 ( double tsince, SSVector &pos, SSVector &vel )
{
    int i = 0;

    double x3thm1 = 0, c1 = 0, x1mth2 = 0, c4 = 0, xnodcf = 0, t2cof = 0, xlcof = 0, aycof = 0, x7thm1 = 0;

    double a = 0,axn = 0,ayn = 0,aynl = 0,beta = 0,betal = 0,capu = 0,cos2u = 0,cosepw = 0,cosik = 0,
           cosnok = 0,cosu = 0,cosuk = 0,ecose = 0,elsq = 0,epw = 0,esine = 0,pl = 0,theta4 = 0,
           rdot = 0,rdotk = 0,rfdot = 0,rfdotk = 0,rk = 0,sin2u = 0,sinepw = 0,sinik = 0,
           sinnok = 0,sinu = 0,sinuk = 0,tempe = 0,templ = 0,tsq = 0,u = 0,uk = 0,ux = 0,uy = 0,uz = 0,
           vx = 0,vy = 0,vz = 0,xinck = 0,xl = 0,xlt = 0,xmam = 0,xmdf = 0,xmx = 0,xmy = 0,xnoddf = 0,
           xnodek = 0,xll = 0,a1 = 0,a3ovk2 = 0,ao = 0,c2 = 0,coef = 0,coef1 = 0,x1m5th = 0,
           xhdot1 = 0,del1 = 0,r = 0,delo = 0,eeta = 0,eta = 0,etasq = 0,perige = 0,
           psisq = 0,tsi = 0,qoms24 = 0,s4 = 0,pinvsq = 0,temp = 0,tempa = 0,temp1 = 0,
           temp2 = 0,temp3 = 0,temp4 = 0,temp5 = 0,temp6 = 0;

//    deep_args deep_arg = { 0 };

    sdp4_args *arg;
    
    if ( argp.sdp4 == nullptr )
    {
        // Initialization
        
        arg = argp.sdp4 = new sdp4_args();

        // Recover original mean motion (xnodp) and
        // semimajor axis (aodp) from input elements.
        
        a1 = pow(xke/xno,tothrd);
        arg->deep.cosio = cos(xincl);
        arg->deep.theta2 = arg->deep.cosio*arg->deep.cosio;
        x3thm1 = 3*arg->deep.theta2-1;
        arg->deep.eosq = eo*eo;
        arg->deep.betao2 = 1-arg->deep.eosq;
        arg->deep.betao = sqrt(arg->deep.betao2);
        del1 = 1.5*ck2*x3thm1/(a1*a1*arg->deep.betao*arg->deep.betao2);
        ao = a1*(1-del1*(0.5*tothrd+del1*(1+134/81*del1)));
        delo = 1.5*ck2*x3thm1/(ao*ao*arg->deep.betao*arg->deep.betao2);
        arg->deep.xnodp = xno/(1+delo);
        arg->deep.aodp = ao/(1-delo);

        // For perigee below 156 km, the values
        // of s and qoms2t are altered.
        
        s4 = s;
        qoms24 = qoms2t;
        perige = (arg->deep.aodp*(1-eo)-xae)*xkmper;
        if(perige < 156)
        {
          if (perige <= 98 )
              s4 = 20;
          else
              s4 = perige-78;
          qoms24 = pow((120-s4)*xae/xkmper,4);
          s4 = s4/xkmper+xae;
        }
        
        pinvsq = 1/(arg->deep.aodp*arg->deep.aodp*arg->deep.betao2*arg->deep.betao2);
        arg->deep.sing = sin(omegao);
        arg->deep.cosg = cos(omegao);
        tsi = 1/(arg->deep.aodp-s4);
        eta = arg->deep.aodp*eo*tsi;
        etasq = eta*eta;
        eeta = eo*eta;
        psisq = fabs(1-etasq);
        coef = qoms24*pow(tsi,4);
        coef1 = coef/pow(psisq,3.5);
        c2 = coef1*arg->deep.xnodp*(arg->deep.aodp*(1+1.5*etasq+eeta*
           (4+etasq))+0.75*ck2*tsi/psisq*x3thm1*(8+3*etasq*(8+etasq)));
        c1 = bstar*c2;
        arg->deep.sinio = sin(xincl);
        a3ovk2 = -xj3/ck2*pow(xae,3);
        x1mth2 = 1-arg->deep.theta2;
        c4 = 2*arg->deep.xnodp*coef1*arg->deep.aodp*arg->deep.betao2*
               (eta*(2+0.5*etasq)+eo*(0.5+2*etasq)-2*ck2*tsi/
               (arg->deep.aodp*psisq)*(-3*x3thm1*(1-2*eeta+etasq*
               (1.5-0.5*eeta))+0.75*x1mth2*(2*etasq-eeta*(1+etasq))*
               cos(2*omegao)));
        theta4 = arg->deep.theta2*arg->deep.theta2;
        temp1 = 3*ck2*pinvsq*arg->deep.xnodp;
        temp2 = temp1*ck2*pinvsq;
        temp3 = 1.25*ck4*pinvsq*pinvsq*arg->deep.xnodp;
        arg->deep.xmdot = arg->deep.xnodp+0.5*temp1*arg->deep.betao*
                         x3thm1+0.0625*temp2*arg->deep.betao*
                         (13-78*arg->deep.theta2+137*theta4);
        x1m5th = 1-5*arg->deep.theta2;
        arg->deep.omgdot = -0.5*temp1*x1m5th+0.0625*temp2*
                          (7-114*arg->deep.theta2+395*theta4)+
                          temp3*(3-36*arg->deep.theta2+49*theta4);
        xhdot1 = -temp1*arg->deep.cosio;
        arg->deep.xnodot = xhdot1+(0.5*temp2*(4-19*arg->deep.theta2)+
                         2*temp3*(3-7*arg->deep.theta2))*arg->deep.cosio;
        xnodcf = 3.5*arg->deep.betao2*xhdot1*c1;
        t2cof = 1.5*c1;
        xlcof = 0.125*a3ovk2*arg->deep.sinio*(3+5*arg->deep.cosio)/
                (1+arg->deep.cosio);
        aycof = 0.25*a3ovk2*arg->deep.sinio;
        x7thm1 = 7*arg->deep.theta2-1;

        // initialize deep space perturbations
        
        dodeep ( dpinit, &arg->deep );
          
        // End of SDP4 initialization, save variables for further use.
          
        arg->x3thm1 = x3thm1;
        arg->c1 = c1;
        arg->x1mth2 = x1mth2;
        arg->c4 = c4;
        arg->xnodcf = xnodcf;
        arg->t2cof = t2cof;
        arg->xlcof = xlcof;
        arg->aycof = aycof;
        arg->x7thm1 = x7thm1;
    }
    else
    {
        // Recover saved variables
        
        arg = argp.sdp4;
        
        x3thm1 = arg->x3thm1;
        c1 = arg->c1;
        x1mth2 = arg->x1mth2;
        c4 = arg->c4;
        xnodcf = arg->xnodcf;
        t2cof = arg->t2cof;
        xlcof = arg->xlcof;
        aycof = arg->aycof;
        x7thm1 = arg->x7thm1;
    }

    // Update for secular gravity and atmospheric drag
    
    xmdf = xmo+arg->deep.xmdot*tsince;
    arg->deep.omgadf = omegao+arg->deep.omgdot*tsince;
    xnoddf = xnodeo+arg->deep.xnodot*tsince;
    tsq = tsince*tsince;
    arg->deep.xnode = xnoddf+xnodcf*tsq;
    tempa = 1-c1*tsince;
    tempe = bstar*c4*tsince;
    templ = t2cof*tsq;
    arg->deep.xn = arg->deep.xnodp;

    // Update for deep-space secular effects
    
    arg->deep.xll = xmdf;
    arg->deep.t = tsince;

    dodeep ( dpsec, &arg->deep );

    xmdf = arg->deep.xll;
    a = pow(xke/arg->deep.xn,tothrd)*tempa*tempa;
    arg->deep.em = arg->deep.em-tempe;
    xmam = xmdf+arg->deep.xnodp*templ;

    // Update for deep-space periodic effects
    
    arg->deep.xll = xmam;

    dodeep ( dpper, &arg->deep );

    xmam = arg->deep.xll;
    xl = xmam+arg->deep.omgadf+arg->deep.xnode;
    beta = sqrt(1-arg->deep.em*arg->deep.em);
    arg->deep.xn = xke/pow(a,1.5);

    // Long period periodics
    
    axn = arg->deep.em*cos(arg->deep.omgadf);
    temp = 1/(a*beta*beta);
    xll = temp*xlcof*axn;
    aynl = temp*aycof;
    xlt = xl+xll;
    ayn = arg->deep.em*sin(arg->deep.omgadf)+aynl;

    // Solve Kepler's Equation
    
    capu = fmod2p(xlt-arg->deep.xnode);
    temp2 = capu;

    i = 0;
    do
    {
        sinepw = sin(temp2);
        cosepw = cos(temp2);
        temp3 = axn*sinepw;
        temp4 = ayn*cosepw;
        temp5 = axn*cosepw;
        temp6 = ayn*sinepw;
        epw = (capu-temp4+temp3-temp2)/(1-temp5-temp6)+temp2;
        if(fabs(epw-temp2) <= e6a)
          break;
        temp2 = epw;
    }
    while ( i++ < 10 );

    // Short period preliminary quantities
    ecose = temp5+temp6;
    esine = temp3-temp4;
    elsq = axn*axn+ayn*ayn;
    temp = 1-elsq;
    pl = a*temp;
    r = a*(1-ecose);
    temp1 = 1/r;
    rdot = xke*sqrt(a)*esine*temp1;
    rfdot = xke*sqrt(pl)*temp1;
    temp2 = a*temp1;
    betal = sqrt(temp);
    temp3 = 1/(1+betal);
    cosu = temp2*(cosepw-axn+ayn*esine*temp3);
    sinu = temp2*(sinepw-ayn-axn*esine*temp3);
    u = actan ( sinu, cosu );
    sin2u = 2*sinu*cosu;
    cos2u = 2*cosu*cosu-1;
    temp = 1/pl;
    temp1 = ck2*temp;
    temp2 = temp1*temp;

    // Update for short periodics
    
    rk = r*(1-1.5*temp2*betal*x3thm1)+0.5*temp1*x1mth2*cos2u;
    uk = u-0.25*temp2*x7thm1*sin2u;
    xnodek = arg->deep.xnode+1.5*temp2*arg->deep.cosio*sin2u;
    xinck = arg->deep.xinc+1.5*temp2*arg->deep.cosio*arg->deep.sinio*cos2u;
    rdotk = rdot-arg->deep.xn*temp1*x1mth2*sin2u;
    rfdotk = rfdot+arg->deep.xn*temp1*(x1mth2*cos2u+1.5*x3thm1);

    // Orientation vectors
    
    sinuk = sin(uk);
    cosuk = cos(uk);
    sinik = sin(xinck);
    cosik = cos(xinck);
    sinnok = sin(xnodek);
    cosnok = cos(xnodek);
    xmx = -sinnok*cosik;
    xmy = cosnok*cosik;
    ux = xmx*sinuk+cosnok*cosuk;
    uy = xmy*sinuk+sinnok*cosuk;
    uz = sinik*sinuk;
    vx = xmx*cosuk-cosnok*sinuk;
    vy = xmy*cosuk-sinnok*sinuk;
    vz = sinik*cosuk;

    // Position and velocity
    
    pos.x = rk*ux;
    pos.y = rk*uy;
    pos.z = rk*uz;
    
    vel.x = rdotk*ux+rfdotk*vx;
    vel.y = rdotk*uy+rfdotk*vy;
    vel.z = rdotk*uz+rfdotk*vz;
}

// Calculates classical osculating orbit elements from position and velocity.
// based on http://sat.belastro.net/satelliteorbitdetermination.com/RV2EL.txt
// by Scott Campbell, campbel.7@hotmail.com.  Position and velocity vectors
// are in Earth-radii and Earth-raii per minute, and referred to Earth-fixed,
// intertial current equatorial frame (not ICRS!)

void SSTLE::rvel ( SSVector &pos, SSVector &vel )
{
    double rr2[3] = { pos.x, pos.y, pos.z };
    double vv2[3] = { vel.x, vel.y, vel.z };
    
    int i;

    double xinck, xnodek, ek, wk, xn, rk, uk, aodp, pl, rdotk, rfdotk, temp;
    double h[3], n[3], vec[3], vk[3];
    double vz[3] = {0, 0, 1};
    double vy[3];

    smult ( 1.0 / xke, vv2, vk );
    cross ( rr2, vk, h );
    pl = dot ( h, h );
    cross ( vz, h, n );
    if ( n[0] == 0.0 && n[1] == 0.0)
        n[0] = 1.0;
    unitv ( n, n );
    rk = norm ( rr2 );
    rdotk = dot ( rr2, vv2 ) / rk;
    rfdotk = norm ( h ) * xke / rk;
    temp = dot ( rr2, n ) / rk;
    uk = acose ( temp );
    if ( rr2[2] < 0.0)
        uk = twopi - uk;
    cross ( vk, h, vz );
    smult ( -1.0 / rk, rr2, vy );
    vadd ( vz, vy, vec );
    ek = norm ( vec );
    if( ek >= 1.)
        return;         // open orbit
    xnodek = atan2 ( n[1], n[0] );
    if ( xnodek < 0.0)
        xnodek += twopi;
    temp = sqrt ( h[0] * h[0] + h[1] * h[1] );
    xinck =  atan2 ( temp, h[2]);
    temp = dot ( vec, n ) / ek;
    wk = acose ( temp );
    if ( vec[2] < 0.0 )
        wk = fmod2p ( twopi - wk );
    aodp = pl / (1.0 - ek * ek );
    xn = xke * pow ( aodp, -1.5 );

    double cosio = 0, sinio = 0, sin2u = 0, cos2u = 0, temp1 = 0, temp2 = 0,
           rdot = 0, rfdot = 0, theta2 = 0, betal = 0, x3thm1 = 0, x1mth2 = 0, x7thm1 = 0,
           esine = 0, ecose = 0, elsq = 0, cosepw = 0, sinepw = 0, axn = 0, ayn = 0,
           cosu = 0, sinu = 0, capu = 0, xlcof = 0, aycof = 0, aynl = 0, xll = 0,
           xl = 0, a0 = 0, a1 = 0, a2 = 0, d0 = 0, d1 = 0, beta = 0, beta2 = 0, r = 0, u = 0;

    // In the first loop the osculating elements rk, uk, xnodek, xinck, rdotk,
    // and rfdotk are used as anchors to find the corresponding final SGP4
    // mean elements r, u, xnodeo, xincl, rdot, and rfdot.  Several other final
    // mean values based on these are also found: betal, cosio, sinio, theta2,
    // cos2u, sin2u, x3thm1, x7thm1, x1mth2.  In addition, the osculating values
    // initially held by aodp, pl, and xn are replaced by intermediate
    // (not osculating and not mean) values used by SGP4.  The loop converges
    // on the value of pl in about four iterations.

    // seed value for first loop
    xincl = xinck;
    u = uk;

    for ( i = 0; i < 99; i++ )
    {
        a2 = pl;
        betal = sqrt ( pl / aodp );
        temp1 = ck2  / pl;
        temp2 = temp1 / pl;
        cosio = cos ( xincl );
        sinio = sin ( xincl );
        sin2u = sin ( 2.0 * u );
        cos2u = cos ( 2.0 * u );
        theta2 = cosio * cosio;
        x3thm1 = 3.0 * theta2 - 1.0;
        x1mth2 = 1.0 - theta2;
        x7thm1 = 7.0 * theta2 - 1.0;
        r = ( rk - 0.5 * temp1 * x1mth2 * cos2u ) / ( 1.0 - 1.5 * temp2 * betal * x3thm1 );
        u = uk + 0.25 * temp2 * x7thm1 * sin2u;
        xnodeo = xnodek - 1.5 * temp2 * cosio * sin2u;
        xincl = xinck - 1.5 * temp2 * cosio * sinio * cos2u;
        rdot = rdotk + xn * temp1 * x1mth2 * sin2u;
        rfdot = rfdotk - xn * temp1 * ( x1mth2 * cos2u + 1.5 * x3thm1 );
        temp = r * rfdot / xke;
        pl = temp * temp;

        // vis-viva equation
        temp = 2.0 / r - ( rdot * rdot + rfdot * rfdot) / ( xke * xke );
        aodp = 1.0 / temp;

        xn = xke * pow ( aodp, -1.5 );
        if ( fabs ( a2 - pl ) < 1.e-13 )
            break;
    }

    // The next values are calculated from constants and a combination of mean
    // and intermediate quantities from the first loop.  These values all remain
    // fixed and are used in the second loop.

    // preliminary values for the second loop
    
    ecose = 1. - r / aodp;
    esine = r * rdot / ( xke * sqrt ( aodp ) );   // needed for Kepler's eqn.
    elsq = 1.0 - pl / aodp;               // intermediate eccentricity squared
    a3ovk2 = -xj3 / ck2;
    xlcof = 0.125 * a3ovk2 * sinio * ( 3.0 + 5.0 * cosio ) / ( 1.0 + cosio );
    aycof = 0.25 * a3ovk2 * sinio;
    temp1 = esine / ( 1.0 + sqrt ( 1.0 - elsq ) );
    cosu = cos ( u );
    sinu = sin ( u );

    // The second loop normally converges in about six iterations to the final
    // mean value for the eccentricity, eo.  The mean perigee, omegao, is also
    // determined.  Cosepw and sinepw are found to high accuracy and
    // are used to calculate an intermediate value for the eccentric anomaly,
    // temp2.  Temp2 is then used in Kepler's equation to find an intermediate
    // value for the true longitude, capu.

    // seed values for loop
    eo = sqrt ( elsq );
    omegao = wk;
    axn = eo * cos ( omegao );

    for ( i = 0; i < 99; i++ )
    {
        a2 = eo;
        beta = 1.0 - eo * eo;
        temp = 1.0 / ( aodp * beta );
        aynl = temp * aycof;
        ayn = eo * sin ( omegao ) + aynl;
        cosepw = r * cosu / aodp + axn - ayn * temp1;
        sinepw = r * sinu / aodp + ayn + axn * temp1;
        axn = cosepw * ecose + sinepw * esine;
        ayn = sinepw * ecose - cosepw * esine;
        omegao = fmod2p ( atan2 ( ayn - aynl, axn ) );
        // eo = axn / cos ( omegao );
        // use weighted average to tame instability at high eccentricities
        eo = 0.9 * eo + 0.1 * ( axn / cos ( omegao ) );
        if ( eo > 0.999 )
            eo = 0.999;
        if ( fabs ( a2 - eo ) < 1.e-13 )
            break;
    }

    temp2 = atan2 ( sinepw, cosepw );
    capu = temp2 - esine;             // Kepler's equation
    xll = temp * xlcof * axn;

    // xll adjusts the intermediate true longitude,
    // capu, to the mean true longitude, xl
    
    xl = capu - xll;
    xmo = fmod2p ( xl - omegao);        // mean anomaly

    // The third loop usually converges after three iterations to the
    // mean semi-major axis, a1, which is then used to find the mean motion, xno.

    a0 = aodp;
    a1 = a0;
    beta2 = sqrt ( beta );
    temp = 1.5 * ck2 * x3thm1 / ( beta * beta2 );
    for ( i = 0; i < 99; i++ )
    {
       a2 = a1;
       d0 = temp / ( a0 * a0 );
       a0 = aodp * ( 1.0 - d0 );
       d1 = temp / ( a1 * a1 );
       a1 = a0 / ( 1.0 - d1 / 3.0 - d1 * d1 - 134.0 * d1 * d1 * d1 / 81.0 );
       if ( fabs ( a2 - a1 ) < 1.e-13 )
           break;
    }
    
    xno = xke * pow ( a1 , -1.5 );
    
    delargs();
    deep = isdeep();
}

// Converts posiition and velocity vectors to SGP4 mean elements.
// See discussion at bottom of Scott Campbell's description page:
// http://sat.belastro.net/satelliteorbitdetermination.com/RV2EL.txt

void SSTLE::rv2el ( SSVector &pos, SSVector &vel )
{
    double rr[3] = { pos.x, pos.y, pos.z };
    double vv[3] = { vel.x, vel.y, vel.z };

    double ik, ok, ek, wk, mk, nk;
    double iz, oz, ez, wz, mz, nz;
    double rr1[3], vv1[3];

    rvel ( pos, vel );           // SGP4 x-elements from state vectors

    // These elements are pretty close.  Save the
    // x-elements as the k-element reference elements

    ik = xincl;
    ok = xnodeo;
    ek = eo;
    wk = omegao;
    mk = xmo;
    nk = xno;

    // save state vectors before they are changed by sgp4(0.0)

    rr1[0] = rr[0];
    rr1[1] = rr[1];
    rr1[2] = rr[2];
    vv1[0] = vv[0];
    vv1[1] = vv[1];
    vv1[2] = vv[2];

    sgp4 ( 0.0, pos, vel );        // SGP4 propagation of k-elements to rr', vv'
    rvel ( pos, vel );             // SGP4 of rr', vv' to x-elements'

    // first correction to k-elements is (k-elements - x-elements')

    xincl  = ik + ik - xincl;
    xnodeo = ok + ok - xnodeo;
    eo     = ek + ek - eo;
    omegao = wk + wk - omegao;
    xmo    = mk + mk - xmo;
    xno    = nk + nk - xno;

    // save z-elements. These elements are very close.

    iz = xincl;
    oz = xnodeo;
    ez = eo;
    wz = omegao;
    mz = xmo;
    nz = xno;

    sgp4 ( 0.0, pos, vel );      // SGP4 propagation of z-elements to rr", vv"
    rvel ( pos, vel);            // SGP4 of rr", vv" to x-elements"

    // second correction is small adjustment to z-elements
    // final elements are corrected by (k-elements - x-elements")

    xincl  = iz + ik - xincl;
    xnodeo = oz + ok - xnodeo;
    eo     = ez + ek - eo;
    omegao = wz + wk - omegao;
    xmo    = mz + mk - xmo;
    xno    = nz + nk - xno;

    // ensure 0 <= angle < twopi

    xincl  = fabs ( xincl );
    xnodeo = fmod2p ( xnodeo );
    omegao = fmod2p ( omegao );
    xmo    = fmod2p ( xmo );

    // restore state vectors
    
    pos.x = rr1[0];
    pos.y = rr1[1];
    pos.z = rr1[2];
    vel.x = vv1[0];
    vel.y = vv1[1];
    vel.z = vv1[2];
}

// Computes satellite position and velocity at a given Julian Date (jd)
// in Earth-centered, inertial current equatorial reference frame, in kilometers and kilometers per second.
// NOTE 1: Julian Date in civil time (UTC), not a Julian Ephemeris Datae in Dynamic Time (TDT)!
// NOTE 2: Position and Velocity are referenced to the Earth's equator at the orbital element epoch,
// not to the fundamental J2000 ICRF equator!

void SSTLE::toPositionVelocity ( double jd, SSVector &pos, SSVector &vel )
{
    double tsince = ( jd - jdepoch ) * xmnpda;
    
    if ( deep )
        sdp4 ( tsince, pos, vel );
    else
        sgp4 ( tsince, pos, vel );
    
    pos *= xkmper;
    vel *= xkmper / 60.0;
}

// Compute SGP4-compatible Keplerian elements in TLE format from satellite position
// and velocity (in Earth-radii and Earth-radii per minute) at a particular Julian
// Date (jd).  The satellite's BSTAR drag coefficient must be input in the TLE
// structure.  The first and second derivatives of mean motion (TLE fields xndt2o
// and xndd6o) are not modified.  The desired epoch of the computed orbital elements
// (as Julian Date) is input in (jd).

void SSTLE::fromPositionVelocity ( double jd, SSVector &pos, SSVector &vel )
{
    SSVector pos1 = pos / xkmper;
    SSVector vel1 = vel * 60.0 / xkmper;

    rv2el ( pos1, vel1 );
    jdepoch = jd;
}

// Reads a TLE record from three lines of an input stream (file).
// Returns 0 if successful or a negative number of failure.

int SSTLE::read ( FILE *file )
{
    string buf = "";
    int    year = 0, number = 0, iexp = 0, ibexp = 0;
    double xm0 = 0.0, xnode0 = 0.0, omega0 = 0.0;
    double e0 = 0.0, xn0 = 1.0, xndt20 = 0.0, xndd60 = 0.0;
    double day = 0.0, epoch = 0.0;
    double temp = M_2PI / xmnpda / xmnpda;

    // Read first line; trim trailing whitespace; copy satellite name
    
    if ( ! fgetline ( file, buf ) )
        return ( -1 );

    name = trim ( buf );

    // Read second line; must start with a '1'
    
    if ( ! fgetline ( file, buf ) )
        return ( -2 );
    
    if ( buf[0] != '1' )
        return ( -2 );
    
    number = strtoint ( buf.substr ( 2, 5 ) );
    desig = trim ( buf.substr ( 9, 6 ) );
    epoch = strtofloat64 ( buf.substr ( 18, 14 ) );
    xndt20 = strtofloat64 ( buf.substr ( 33, 10 ) );
    xndd60 = strtofloat64 ( buf.substr ( 44, 6 ) );
    iexp = strtoint ( buf.substr ( 50, 2 ) );
    bstar = strtofloat64 ( buf.substr ( 53, 6 ) );
    ibexp = strtoint ( buf.substr ( 59, 2 ) );
    elset = strtoint ( buf.substr ( 65, 3 ) );
    
    // Convert epoch to year and day of year

    year = epoch / 1000.0;
    day = epoch - year * 1000.0;
    if ( year > 56 )
        year += 1900;
    else
        year += 2000;

    // Convert other parameters

    norad = number;
    jdepoch = SSTime ( SSDate ( kGregorianJulian, 0.0, year, 1, day ) );
    xndt2o = xndt20 * temp;
    xndd6o = xndd60 * 1.0e-5 * pow ( 10.0, iexp ) * temp / xmnpda;
    bstar = bstar * 1.0e-5 * pow ( 10.0, ibexp );
             
    // Third line must start with a '2'
    
    if ( ! fgetline ( file, buf ) )
        return ( -3 );

    if ( buf[0] != '2' )
        return ( -3 );
    
    number = strtoint ( buf.substr ( 2, 5 ) );
    xincl = strtofloat64 ( buf.substr ( 8, 8 ) );
    xnode0 = strtofloat64 ( buf.substr ( 17, 8 ) );
    e0 = strtofloat64 ( buf.substr ( 26, 7 ) );
    omega0 = strtofloat64 ( buf.substr ( 34, 8 ) );
    xm0 = strtofloat64 ( buf.substr ( 43, 8 ) );
    xn0 = strtofloat64 ( buf.substr ( 52, 11 ) );
    
    // Convert other parameters
    
    xincl = degtorad ( xincl );
    xnodeo = degtorad ( xnode0 );
    eo = e0 * 1.0e-7;
    omegao = degtorad ( omega0 );
    xmo = degtorad ( xm0 );
    xno = xn0 * M_2PI / xmnpda;

    // Does this TLE already have SGP orbit model arguments?
    // If so delete them since they won't be valid after new are elements read.

    delargs();
    
    // Select a deep-space vs. near-earth ephemeris
    
    deep = isdeep();
    return ( 0 );
}

// Reads a TLE record in Celestrak CSV format from one line of an input stream (file).
// Returns 0 if successful or a negative number of failure.

int SSTLE::read_csv ( FILE *file )
{
    string buf = "";
    SSDate epoch;
    double temp = M_2PI / xmnpda / xmnpda;

    // Read CSV line; parse fields
    
    if ( ! fgetline ( file, buf ) )
        return ( -1 );

    vector<string> csv = split_csv ( buf );
    if ( csv.size() < 17 )
        return -2;
    
    name = trim ( csv[0] );
    desig = trim ( csv[1] );
    if ( sscanf ( trim ( csv[2] ).c_str(), "%d-%hd-%hdT%hd:%hd:%lf",
        &epoch.year, &epoch.month, &epoch.day, &epoch.hour, &epoch.min, &epoch.sec ) < 6 )
        return -3;
    
    jdepoch = SSTime ( epoch );
    xno = strtofloat64 ( csv[3] ) * M_2PI / xmnpda;
    eo = strtofloat64 ( csv[4] );
    xincl = degtorad ( strtofloat64 ( csv[5] ) );
    xnodeo = degtorad ( strtofloat64 ( csv[6] ) );
    omegao = degtorad ( strtofloat64 ( csv[7] ) );
    xmo = degtorad ( strtofloat64 ( csv[8] ) );
    norad = strtoint ( csv[11] );
    elset = strtoint ( csv[12] );
    bstar = strtofloat64 ( csv[14] );
    xndt2o = strtofloat64 ( csv[15] ) * temp;
    xndd6o = strtofloat64 ( csv[16] ) * temp / xmnpda;
    
    if ( norad < 1 || xno <= 0.0 )
        return -4;
    
    // Does this TLE already have SGP orbit model arguments?
    // If so delete them since they won't be valid after new are elements read.

    delargs();
    
    // Select a deep-space vs. near-earth ephemeris
    
    deep = isdeep();
    return ( 0 );
}

// Writes a TLE record to three lines of an output stream (file).
// Returns 0 if successful or a negative number of failure.

int SSTLE::write ( ostream &file )
{
    double temp = M_2PI / xmnpda / xmnpda;
    double xincl0 = radtodeg ( xincl );  // inclination
    double xnode0 = radtodeg ( xnodeo ); // right ascension of ascending node
    double e0 = eo * 1.0e7;              // eccentricity
    double omega0 = radtodeg ( omegao ); // argument of perigee in degrees
    double xm0 = radtodeg ( xmo );       // mean anomaly in degrees
    double xn0 = xmnpda * xno / M_2PI;   // mean motion in revolutions/day
    double xndt20 = xndt2o / temp;       // half of derivative of mean motion
    double xndd60 = fabs ( xndd6o ) * xmnpda / temp;    // one sixth of second derivative of mean motion
    double bstar0 = fabs ( bstar );      // BSTAR drag coefficient
    int iexp = 0, ibexp = 0;             // exponents
    
    if ( xndd60 > 0.0 )
    {
        iexp = floor ( log10 ( xndd60 ) ) + 1;
        xndd60 = xndd60 / pow ( 10.0, iexp - 5 );
        xndd60 = xndd6o > 0.0 ? xndd60 : -xndd60;
    }
    
    if ( bstar0 > 0.0 )
    {
        ibexp = floor ( log10 ( bstar0 ) ) + 1;
        bstar0 = bstar0 / pow ( 10.0, ibexp - 5 );
        bstar0 = bstar > 0.0 ? bstar0 : -bstar0;
    }
    
    // epoch in TLE format
    
    SSDate date = SSDate ( SSTime ( jdepoch ) );
    double day = jdepoch - SSTime ( SSDate ( kGregorianJulian, 0.0, date.year, 1, 0.0, 0, 0, 0.0 ) );
    double epoch = ( date.year % 100 ) * 1000.0 + day;
    
    // International designator could be formatted as "1998-067A", if imported from CSV format.
    // but classic TLE format requires designator formatted as "98067A", so abbreviate it.
    string tledesig = desig;
    if ( tledesig.length() > 6 )
    {
        tledesig = tledesig.substr ( 2, string::npos ); // elminate 2 leading characters
        replaceAll ( tledesig, "-", "" );               // eliminate hyphens
        tledesig = tledesig.substr ( 0, 6 );            // truncate to 6 characters no matter what
    }
    
    // format first line of orbital elements
    
    string line1 = format ( "1 %05dU %-6s   %13.8f %c.%08.0f %+06.0f-%1d %+06.0f%+1d 0  %03d0",
             norad, tledesig.c_str(), epoch,
             xndt20 > 0 ? '+' : '-', fabs ( xndt20 * 1.0e8 ),
             xndd60, -iexp, bstar0, ibexp, clamp ( elset, 0, 999 ) );
    line1[69] = checksum ( line1 );
    
    // format second line of orbital element
    
    string line2 = format ( "2 %05d %08.4lf %08.4lf %07.0f %08.4lf %08.4lf %11.8lf    00",
            norad, xincl0, xnode0, e0, omega0, xm0, xn0 );
    line2[69] = checksum ( line2 );

    // write to output stream
    
    file << name << endl;
    file << line1 << endl;
    file << line2 << endl;

    return 0;
}

char SSTLE::checksum ( string &line )
{
    int i, sum = 0;
    
    for ( i = 0; i < 68; i++ )
    {
        char c = line[i];
        
        if ( c > '0' && c <= '9')
            sum += c - '0';
        
        if ( c == '-' )
            sum += 1;
    }
    
    return ( sum % 10 + '0' );
}

void SSTLE::delargs ( void )
{
    if ( argp.sdp4 || argp.sgp4 )
    {
        if ( deep )
            delete argp.sdp4;
        else
            delete argp.sgp4;
        
        argp.sdp4 = nullptr;
        argp.sgp4 = nullptr;
    }
}

// Returns Keplerian orbital elements at a specific number of minutes since epoch (tsince).
// Based on analysis of SGP orbit model, and at least approximately correct, but needs further testing.
// In the returned orbit, q is Earth radii, mm is radians per minute, and i, w, n are referred to
// Earth's equatorial plane.

SSOrbit SSTLE::toOrbit ( double tsince )
{
    // Recover original semimajor axis and mean motion.
    
    double cosi = cos ( xincl );
    double a1 = pow ( xke / xno, ( 2.0 / 3.0 ) );
    double del1 = ( 0.75 * xj2 * xae * xae * ( 3.0 * cosi * cosi - 1.0 ) ) / ( a1 * a1 * pow ( 1.0 - eo * eo, 1.5 ) );
    double ao = a1 * ( 1.0 - del1 / 3.0 - del1 * del1 - 134.0 * del1 * del1 * del1 / 81.0 );
    double del0 = ( 0.75 * xj2 * xae * xae * ( 3.0 * cosi * cosi - 1.0 ) ) /  ( ao * ao * pow ( 1.0 - eo * eo, 1.5 ) );
    double xnodp = xno / ( 1.0 + del0 );
    double aodp = ao / ( 1.0 - del0 );

    // Compute secular rates of ascending node and perigee.
        
    double po = ao * ( 1.0 - eo * eo );
    double xnodot = -1.5 * xj2 * xae * xae * xno * cosi / ( po * po );
    double omegadot = 0.75 * xj2 * xae * xae * xno * ( 5.0 * cosi * cosi - 1.0 ) / ( po * po );
    
    // Update the angular elements for secular variations.  The perturbation
    // to the mean motion is already contained in the SGP-type mean motion.
    
    double omegat = fmod2p ( omegao + omegadot * tsince );
    double xnodet = fmod2p ( xnodeo + xnodot * tsince );
    double xmt = fmod2p ( xmo + xno * tsince );
    
    return SSOrbit ( jdepoch + tsince / xmnpda, aodp * ( 1.0 - eo ), eo, xincl, omegat, xnodet, xmt, xnodp );
}
