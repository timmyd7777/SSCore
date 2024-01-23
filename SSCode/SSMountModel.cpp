// SSMountModel.cpp
// SSCore
//
// Created by Tim DeBenedictis on 3/21/23.
// Copyright © 2023 Southern Stars. All rights reserved.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "SSMountModel.hpp"

/* lsquare.cpp: least-squares computations

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

#define LSQUARE struct lsquare

LSQUARE
   {
   int n_params, n_obs;
   double *wtw, *uw;
   };

void *lsquare_init( const int n_params)
{
   LSQUARE *rval = (LSQUARE *)calloc( 1,
          sizeof( LSQUARE) + n_params * (n_params + 1) * sizeof( double));

   if( !rval)
      return( (void *)rval);
   rval->n_params = n_params;
   rval->n_obs = 0;
   rval->uw = (double *)( rval + 1);
   rval->wtw = rval->uw + n_params;
   return(( void *)rval);
}

int lsquare_add_observation( void *lsquare, const double residual,
                                  const double weight, const double *obs)
{
   LSQUARE *lsq = (LSQUARE *)lsquare;
   int i, j;
   const int n_params = lsq->n_params;

   for( i = 0; i < n_params; i++)
      {
      const double w2_obs_i = weight * weight * obs[i];

      lsq->uw[i] += residual * w2_obs_i;
      for( j = 0; j < n_params; j++)
         lsq->wtw[i + j * n_params] += w2_obs_i * obs[j];
      }
   lsq->n_obs++;
   return( lsq->n_obs);
}

double lsquare_determinant;

   /* A simple Gauss-Jordan matrix inverter,  with partial pivoting.  It
      first extends the size x size square matrix into a size-high by
      (2*size) wide one,  with the expanded space on the right side filled
      with an identity matrix.  It then zeroes out the lower left triangle
      of the original size x size matrix.  Some row-swapping is done in
      this procedure (that's the "partial pivoting" aspect;  see _Numerical
      Recipes_,  chap. 2.1 for details.)   */

static double *calc_inverse( const double *src, const int size)
{
   double *rval;
   double *temp = (double *)calloc( 2 * size * size, sizeof( double)), *tptr;
   double *tptr1, *tptr2, tval;
   int i, j, k;
   const int dsize = 2 * size;

   lsquare_determinant = 1.;
   if( !temp)
      return( NULL);
   for( i = 0; i < size; i++)
      {
      tptr = temp + i * dsize;
      memcpy( tptr, src + i * size, size * sizeof( double));
      tptr += size;
      for( j = 0; j < size; j++)
         *tptr++ = ((i == j) ? 1. : 0.);
      }

   tptr1 = temp;
   for( i = 0; i < size; i++, tptr1 += dsize)
      {
      int pivot = -1;
      double best_val = 0.;

      tptr = tptr1;
      for( j = i; j < size; j++, tptr += dsize)
         if( fabs( tptr[i]) > best_val)
            {
            best_val = fabs( tptr[i]);
            pivot = j;
            }

      if( pivot == -1)     /* un-invertable matrix:  return NULL */
         {
         free( temp);
         return( NULL);
         }

      if( pivot != i)                  /* swap rows */
         {
         tptr2 = temp + dsize * pivot;
         for( j = i; j < dsize; j++)
            {
            tval = tptr1[j];
            tptr1[j] = tptr2[j];
            tptr2[j] = tval;
            }
         }

      for( j = i + 1; j < size; j++)
         {
         tptr2 = temp + dsize * j;
         tval = tptr2[i] / tptr1[i];
         for( k = i; k < dsize; k++)
            tptr2[k] -= tptr1[k] * tval;
         }
      }
                  /* the lower left triangle is now cleared;  time to */
                  /* zero out the upper right triangle: */

   for( i = size - 1; i >= 0; i--)
      {
      tptr1 = temp + i * dsize;
      for( j = size; j < dsize; j++)
         {
         lsquare_determinant /= tptr1[i];
         tptr1[j] /= tptr1[i];
         }
      tptr2 = temp;
      for( k = 0; k < i; k++, tptr2 += dsize)
         for( j = size; j < dsize; j++)
            tptr2[j] -= tptr2[i] * tptr1[j];
      }

   rval = (double *)calloc( size * size, sizeof( double));
   if( rval)
      for( i = 0; i < size; i++)
         memcpy( rval + i * size, temp + (i * 2 + 1) * size,
                                       size * sizeof( double));
   free( temp);
   return( rval);
}

static void mult_matrices( double *prod, const double *a, const int awidth,
                  const int aheight, const double *b, const int bwidth)
{
   int i, j;

   for( j = 0; j < aheight; j++)
      for( i = 0; i < bwidth; i++, prod++)
         {
         int k;
         const double *aptr = a + j * awidth, *bptr = b + i;

         *prod = 0.;
         for( k = awidth; k; k--, bptr += bwidth)
            *prod += *aptr++ * (*bptr);
         }
}

#ifdef LSQUARE_ERROR
static void dump_matrix( FILE *ofile, const double *matrix, const int size)
{
   int i;
   double largest_element = 0.;

   for( i = 0; i < size * size; i++)
      {
      if( largest_element < fabs( *matrix))
         largest_element = fabs( *matrix);
      fprintf( ofile, "%11.2e%s", *matrix++, !((i + 1) % size) ? "\n" : "");
      }
   fprintf( ofile, "Largest element: %11.2e\n", largest_element);
}
#endif

/* calc_inverse_improved() computes a matrix inverse using the simpler
   'calc_inverse()' function,  a plain ol' Gauss-Jordan inverter (see above).
   It then uses a rather simple trick from _Numerical Recipes_,  chap. 2.5,
   "Iterative Improvement of a Solution to Linear Equations",  to "polish"
   the result.  I added this step when I had some concerns that my least
   squares solutions weren't what they ought to be.  The problem lay elsewhere.
   But this _should_ ensure that matrix inversion is more accurate than it
   otherwise would be,  at almost no computational cost.  */

static double *calc_inverse_improved( const double *src, const int size)
{
   double *inverse = calc_inverse( src, size);

   if( inverse)
      {
      double *err_mat = (double *)calloc( 2 * size * size, sizeof( double));
      double *b_times_delta = err_mat + size * size;
      int i;
#ifdef LSQUARE_ERROR
      FILE *ofile = fopen( "lsquare.dat", "ab");
#endif

      mult_matrices( err_mat, src, size, size, inverse, size);
      for( i = 0; i < size; i++)
         err_mat[i * (size + 1)] -= 1.;
#ifdef LSQUARE_ERROR
      fprintf( ofile, "%d-square matrix delta (= I - AB):\n", size);
      dump_matrix( ofile, err_mat, size);
      fclose( ofile);
#endif
      mult_matrices( b_times_delta, inverse, size, size, err_mat, size);
      for( i = 0; i < size * size; i++)
         inverse[i] -= b_times_delta[i];
      free( err_mat);
      }
   return( inverse);
}

int lsquare_solve( const void *lsquare, double *result)
{
   const LSQUARE *lsq = (const LSQUARE *)lsquare;
   int i, j, n_params = lsq->n_params;
   double *inverse;

   if( n_params > lsq->n_obs)       /* not enough observations yet */
      return( -1);

// inverse = calc_inverse( lsq->wtw, n_params);
   inverse = calc_inverse_improved( lsq->wtw, n_params);
   if( !inverse)
      return( -2);            /* couldn't invert matrix */

   for( i = 0; i < n_params; i++)
      result[i] = 0.;

   for( i = 0; i < n_params; i++)
      for( j = 0; j < n_params; j++)
         result[i] += inverse[i + j * n_params] * lsq->uw[j];

   free( inverse);
   return( 0);
}

double *lsquare_covariance_matrix( const void *lsquare)
{
   const LSQUARE *lsq = (const LSQUARE *)lsquare;
   double *rval = NULL;

   if( lsq->n_params <= lsq->n_obs)       /* got enough observations */
      rval = calc_inverse_improved( lsq->wtw, lsq->n_params);
   return( rval);
}

double *lsquare_wtw_matrix( const void *lsquare)
{
   const LSQUARE *lsq = (const LSQUARE *)lsquare;

   return( lsq->wtw);
}

void lsquare_free( void *lsquare)
{
   free( lsquare);
}

#define PI 3.1415926535897932384626433
#define SWAP( A, B, TEMP)   { TEMP = A;  A = B;  B = TEMP;  }

static void make_vect( double *vect, const double az, const double alt)
{
   double cos_alt = cos( alt);

   *vect++ = cos( az) * cos_alt;
   *vect++ = sin( az) * cos_alt;
   *vect++ = sin( alt);
}

int encoder_to_alt_az( const double *m, const double x, const double y, double *alt, double *az)
{
   double pseudo_az  = m[MODEL_AZM_ZERO]  + m[MODEL_AZM_RATE] * x;
   double pseudo_alt = m[MODEL_ALT_ZERO] + m[MODEL_ALT_RATE] * y;
   double vect[3], real_alt, delta_az, real_az;
   const double cos_misalign = cos( m[MODEL_MISALIGN]);
// double tan_misalign = sin( m[MODEL_MISALIGN]) / cos_misalign;
   int i;

   pseudo_alt += m[MODEL_FLEXURE] * cos( pseudo_alt) +
                 m[MODEL_WARPED_TBL_1] * cos( pseudo_az * 2.) +
                 m[MODEL_WARPED_TBL_2] * sin( pseudo_az * 2.) +
                 m[MODEL_ALT_SIN] * sin( pseudo_alt) +
                 m[MODEL_ALT_SIN2] * sin( pseudo_alt * 2.) +
                 m[MODEL_ALT_COS2] * cos( pseudo_alt * 2.);
   pseudo_az += m[MODEL_AZM_COS] * cos( pseudo_az) +
                m[MODEL_AZM_SIN] * sin( pseudo_az) +
                m[MODEL_AZM_SIN2] * sin( pseudo_az * 2.) +
                m[MODEL_AZM_COS2] * cos( pseudo_az * 2.);
               /* now correct for misalign,  the non-perpendicularity of the */
               /* alt and az axes: */

   if( !m[MODEL_MISALIGN])
      {
      real_alt = pseudo_alt;
      delta_az = 0.;
      }
   else
      {
      real_alt = asin( sin( pseudo_alt) * cos_misalign);
      delta_az = acos( cos( pseudo_alt) / cos( real_alt));
      if( m[MODEL_MISALIGN] < 0.)
         delta_az = -delta_az;

                  /* 2 Jun 1999:  the following line attempts to handle */
                  /* some 'over the pole' weirdness.  If pseudo_alt winds */
                  /* up on the opposite side of the pole,  we have to     */
                  /* adjust the azimuth accordingly. */

//    if( cos( pseudo_alt) < 0.)
//       delta_az = pi + delta_az;
      }

   real_az = pseudo_az + delta_az;

               /* and now go to vector form,  so we can tilt it twice: */
   make_vect( vect, real_az, real_alt);

   for( i = 0; i < 2; i++)
      {
      double tilt = m[MODEL_TILT1 + i];

      double temp = sin( tilt) * vect[i] + cos( tilt) * vect[2];
      vect[i] = cos( tilt) * vect[i] - sin( tilt) * vect[2];
      vect[2] = temp;
      }
   *az = atan2( vect[1], vect[0]);
   *alt = asin( vect[2]);
   return( 0);
}

int alt_az_to_encoder( const double *m, const double alt, const double az, double *x, double *y)
{
   double pseudo_az, pseudo_alt;
   double vect[3];
   double cos_misalign = cos( m[MODEL_MISALIGN]);
   double tan_misalign = sin( m[MODEL_MISALIGN]) / cos_misalign;
   int i;

   make_vect( vect, az, alt);

   for( i = 1; i >= 0; i--)
      {
      double tilt = m[MODEL_TILT1 + i];

      double temp = -sin( tilt) * vect[i] + cos( tilt) * vect[2];
      vect[i] = cos( tilt) * vect[i] + sin( tilt) * vect[2];
      vect[2] = temp;
      }

   pseudo_az = atan2( vect[1], vect[0]);
   pseudo_alt = asin( vect[2]);
                                    /* now correct for non-perpen. axes: */
   pseudo_az -= vect[2] * tan_misalign / cos( pseudo_alt);
   pseudo_alt = asin( sin( pseudo_alt) / cos_misalign);

   pseudo_alt -= m[MODEL_FLEXURE] * cos( pseudo_alt) +
                 m[MODEL_WARPED_TBL_1] * cos( pseudo_az * 2.) +
                 m[MODEL_WARPED_TBL_2] * sin( pseudo_az * 2.) +
                 m[MODEL_ALT_SIN] * sin( pseudo_alt) +
                 m[MODEL_ALT_SIN2] * sin( pseudo_alt * 2.) +
                 m[MODEL_ALT_COS2] * cos( pseudo_alt * 2.);
   pseudo_az -= m[MODEL_AZM_COS] * cos( pseudo_az) +
                m[MODEL_AZM_SIN] * sin( pseudo_az) +
                m[MODEL_AZM_SIN2] * sin( pseudo_az * 2.) +
                m[MODEL_AZM_COS2] * cos( pseudo_az * 2.);

   *x = (pseudo_az - m[MODEL_AZM_ZERO]) / m[MODEL_AZM_RATE];
   *y = (pseudo_alt - m[MODEL_ALT_ZERO]) / m[MODEL_ALT_RATE];
   return( 0);
}

int improve_model( double *m, const int n_stars, const double *x,
            const double *y, const double *alt, const double *az,
            const char *adjustable)
{
   void *lsquare;
   int i, j, pass, n_params, idx;
   double increments[MODEL_N_PARAMS];

   for( i = n_params = 0; i < MODEL_N_PARAMS; i++)
      if( adjustable[i])
         n_params++;
   lsquare = lsquare_init( n_params);
   if( !lsquare)
      return( -1);

   for( i = 0; i < n_stars; i++)
      {
      double xslopes[MODEL_N_PARAMS], yslopes[MODEL_N_PARAMS];
      double alt0, az0, xresid, yresid, cos_alt0;

      encoder_to_alt_az( m, *x, *y, &alt0, &az0);
      cos_alt0 = cos( alt0);

      xresid = sin( az0 - *az) * cos_alt0;
      yresid = sin( alt0 - *alt);

      for( j = idx = 0; j < MODEL_N_PARAMS; j++)
         if( adjustable[j])
            {
            double delta = .001;
            double output_alt[2], output_az[2];
            double m1[MODEL_N_PARAMS];

            if( !j || j == 2)     /* cut down on delta for rates */
               delta = .000001;
            memcpy( m1, m, MODEL_N_PARAMS * sizeof( double));
            m1[j] -= delta;
            for( pass = 0; pass < 2; pass++)
               {
               if( pass)
                  m1[j] += delta + delta;
               encoder_to_alt_az( m1, *x, *y,
                                     output_alt + pass, output_az + pass);
               }

            xslopes[idx] = sin( output_az[1] - output_az[0]) * cos_alt0;
            xslopes[idx] /= 2. * delta;
            yslopes[idx] = sin( output_alt[1] - output_alt[0]);
            yslopes[idx] /= 2. * delta;
            idx++;
            }

      lsquare_add_observation( lsquare, -xresid, 1., xslopes);
      lsquare_add_observation( lsquare, -yresid, 1., yslopes);
      x++;
      y++;
      alt++;
      az++;
      }

   if( !lsquare_solve( lsquare, increments))
      for( i = n_params = 0; i < MODEL_N_PARAMS; i++)
         if( adjustable[i])
            m[i] += increments[n_params++];

   lsquare_free( lsquare);
   return( 0);
}

double centralize_ang( double ival, double center)
{
   while( ival > center + PI)
      ival -= PI + PI;
   while( ival < center - PI)
      ival += PI + PI;
   return( ival);
}

double correct_for_encoders( int n_stars, double *enc, double *ang, double res)
{
   double d_ang, d_encoder;
   double scale;
   int i;

   d_ang = centralize_ang( ang[1] - ang[0], 0);
   d_encoder = enc[1] - enc[0] + res;
   while( d_encoder > res / 2.)
      d_encoder -= res;
   scale = (double)res / (PI * 2.);
   if( d_ang * d_encoder < 0.)
      scale = -scale;

   for( i = 1; i < n_stars; i++)
      {
      double target_enc;

      target_enc = enc[0] + (ang[i] - ang[0]) * scale;
      while( enc[i] < target_enc - res / 2.)
         enc[i] += res;
      while( enc[i] > target_enc + res / 2.)
         enc[i] -= res;
      }
   return( 1. / scale);
}

// SSMountModel constructor.
// If known, the telescope's mount azimuth and altitude axis encoder
// steps per revolution should be passed in (xres,yres). If unknown,
// set them to zero; they will be estimated during align().
// Initially, the model will have 4 adjustable parameters:
// MODEL_ALT_RATE, MODEL_ALT_ZERO, MODEL_AZM_RATE, MODEL_AZM_ZERO.

SSMountModel::SSMountModel ( double xres, double yres )
{
    _xres = xres;
    _yres = yres;
    _n_stars = 0;
    
    memset ( _m, 0, sizeof ( _m ) );
    memset ( _adjustable, false, MODEL_N_PARAMS );
    memset ( _x_stars, 0, sizeof ( _x_stars ) );
    memset ( _y_stars, 0, sizeof ( _y_stars ) );
    memset ( _azm_stars, 0, sizeof ( _azm_stars ) );
    memset ( _alt_stars, 0, sizeof ( _alt_stars ) );
    
    // Note MODEL_AZM_RATE and MODEL_ALT_RATE will be made
    // non-adjustable in align() if xres and yres are nonzero.
    
    for ( int j = 0; j < 4; j++ )
       _adjustable[j] = true;
}

// Set adjustability flag for model parameter (param) indexed from 0 ... MODEL_N_PARAMS - 1

bool SSMountModel::adjustable ( int param )
{
    return param >= 0 && param < MODEL_N_PARAMS ? _adjustable[param] : false;
}

// Gets adjustability flag for model parameter (param) indexed from 0 ... MODEL_N_PARAMS - 1

void SSMountModel::adjustable ( int param, bool adj )
{
    if ( param >= 0 && param < MODEL_N_PARAMS )
        _adjustable[param] = adj;
}

// Convert mount axis encoder positions (x,y = azimuth,altitude) to celestial coordinates (azm,alt).
// Mount model must have been aligned by calling align() after adding alignment stars.

void SSMountModel::encodersToCelestial ( double x, double y, SSAngle &azm, SSAngle &alt )
{
    double al = 0.0, az = 0.0;
    encoder_to_alt_az ( _m, x, y, &al, &az );
    azm = SSAngle ( mod2pi ( az ) );
    alt = SSAngle ( al );
}

// Convert celestial coordinates (azm,alt) to mount axis encoder positions (x,y = azimuth,altitude).
// Mount model must have been aligned by calling align() after adding alignment stars.

void SSMountModel::celestialToEncoders ( SSAngle azm, SSAngle alt, double &x, double &y )
{
    double ex = 0.0, ey = 0.0;
    alt_az_to_encoder ( _m, alt, azm, &ex, &ey );
    x = ex;
    y = ey;
}

// Adds an alignment star to the end of the mount model's alignment star array.
// Star's encoder position (x,y) corresponds to celestial coordinates (azm,alt).
// Returns true if successful or false if alignment star array is full.
// Call align() after adding alignment stars.

bool SSMountModel::addStar ( double x, double y, SSAngle azm, SSAngle alt )
{
    if ( _n_stars >= MODEL_N_STARS )
        return false;
    
    _x_stars[ _n_stars ] = x;
    _y_stars[ _n_stars ] = y;
    _azm_stars[ _n_stars ] = azm;
    _alt_stars[ _n_stars ] = alt;
    _n_stars++;
    
    return true;
}

// Deletes alignment star at index i in the mount model's alignment star array.
// The index must be 0 ... numStars() - 1.
// Returns true if successful or false if the index is out of range.
// Call align() after deleting alignment stars.

bool SSMountModel::delStar ( int i )
{
    if ( i < 0 || i >= _n_stars )
        return false;
    
    _n_stars--;
    memcpy ( &_x_stars[i], &_x_stars[i+1], _n_stars * sizeof ( double ) );
    memcpy ( &_y_stars[i], &_y_stars[i+1], _n_stars * sizeof ( double ) );
    memcpy ( &_azm_stars[i], &_azm_stars[i+1], _n_stars * sizeof ( double ) );
    memcpy ( &_alt_stars[i], &_alt_stars[i+1], _n_stars * sizeof ( double ) );

    _x_stars[_n_stars] = _x_stars[_n_stars] = 0.0;
    _azm_stars[_n_stars] = _alt_stars[_n_stars] = 0.0;
    return true;
}

// Removes all alignment star data and resets all mount model parameters to zero.

void SSMountModel::reset ( void )
{
    memset ( _x_stars, 0, sizeof ( _x_stars ) );
    memset ( _y_stars, 0, sizeof ( _y_stars ) );
    memset ( _azm_stars, 0, sizeof ( _azm_stars ) );
    memset ( _alt_stars, 0, sizeof ( _alt_stars ) );
    memset ( _m, 0, sizeof ( _m ) );
    _n_stars = 0;
}

// Calculates all adjustable mount model parameters. Call this after addStars().
// Returns the RMS error for all alignment stars in radians, or zero on failure.

double SSMountModel::align ( void )
{
    memset ( _m, 0, sizeof ( _m ) );

    if ( _xres && _yres )
    {
        if ( _n_stars < 2 )
        {
            _m[MODEL_AZM_RATE] = 2.0 * PI / _xres;
            _m[MODEL_ALT_RATE] = 2.0 * PI / _yres;
        }
        else
        {
            _m[MODEL_AZM_RATE] = correct_for_encoders ( _n_stars, _x_stars, _azm_stars, fabs ( _xres ) );
            _m[MODEL_ALT_RATE] = correct_for_encoders ( _n_stars, _y_stars, _alt_stars, fabs ( _yres ) );
        }
        
        _adjustable[MODEL_AZM_RATE] = false;
        _adjustable[MODEL_ALT_RATE] = false;
    }
    
    _m[MODEL_ALT_ZERO] = 0.0;
    _m[MODEL_AZM_ZERO] = 0.0;
        
    for ( int i = 0; i < _n_stars; i++ )
    {
        double d_alt = _alt_stars[i] - _y_stars[i] * _m[MODEL_ALT_RATE];
        double d_azm = _azm_stars[i] - _x_stars[i] * _m[MODEL_AZM_RATE];
        
        while ( d_alt > _m[MODEL_ALT_ZERO] + PI )
            d_alt -= PI + PI;
        while ( d_alt < _m[MODEL_ALT_ZERO] - PI )
            d_alt += PI + PI;
        while ( d_azm > _m[MODEL_AZM_ZERO] + PI )
            d_azm -= PI + PI;
        while ( d_azm < _m[MODEL_AZM_ZERO] - PI )
            d_azm += PI + PI;
        
        _m[MODEL_ALT_ZERO] = ( d_alt + i * _m[MODEL_ALT_ZERO] ) / ( i + 1 );
        _m[MODEL_AZM_ZERO] = ( d_azm + i * _m[MODEL_AZM_ZERO] ) / ( i + 1 );
    }

    // If this is a trivial one-star alignment, we're done.
    
    if ( _n_stars < 2 )
        return 0;
    
    // Initial model made
    
    for( int iter = 4; iter < MODEL_N_PARAMS; iter++)
    {
        char adjust[MODEL_N_PARAMS];
        memcpy ( adjust, _adjustable, iter );
        memset ( adjust + iter, 0, MODEL_N_PARAMS - iter );
        improve_model ( _m, _n_stars, _x_stars, _y_stars, _alt_stars, _azm_stars, adjust );
        improve_model ( _m, _n_stars, _x_stars, _y_stars, _alt_stars, _azm_stars, adjust );
    }

    // Now do an iterative nonlinear least-squares best fit to find the
    // remaining model parameters (and improve the initial ones).
    
    for ( int iter = 0; iter < 10; iter++ )
        improve_model ( _m, _n_stars, _x_stars, _y_stars, _alt_stars, _azm_stars, _adjustable );
    
    // compute residuals in radians (not degrees) of individual alignment stars
    
    double rms_x = 0.0, rms_y = 0.0, rms_total = 0.0;
    for ( int i = 0; i < _n_stars; i++ )
    {
        double xresid = 0.0, yresid = 0.0;
        getResiduals ( i, xresid, yresid );
        rms_x += xresid * xresid;
        rms_y += yresid * yresid;
    }
    
    // compute RMS residual in alt and az; return total RMS residual in radians
    
    rms_total = sqrt ( ( rms_x + rms_y ) / _n_stars );
    rms_x = sqrt ( rms_x / _n_stars );
    rms_y = sqrt ( rms_y / _n_stars );

    return rms_total;
}

// Calculates residuals, i.e. differences between actual and predicted position
// for an alignment star in azimuth (azm_resid) and altitude (alt_resid), in radians.
// Alignment star index (i) must range from 0 ... number of alignment stars - 1.
// Only valid after alignment has been performed.

bool SSMountModel::getResiduals ( int i, double &azm_resid, double &alt_resid )
{
    if ( i < 0 || i >= _n_stars )
        return false;
    
    double alt1 = 0.0, azm1 = 0.0;
    encoder_to_alt_az ( _m, _x_stars[i], _y_stars[i], &alt1, &azm1 );
    while ( azm1 - _azm_stars[i] >  PI ) azm1 -= PI + PI;
    while ( azm1 - _azm_stars[i] < -PI ) azm1 += PI + PI;
    azm_resid = ( azm1 - _azm_stars[i] ) * cos ( alt1 );
    alt_resid = ( alt1 - _alt_stars[i] );

    return true;
}

