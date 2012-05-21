/*
 * Resine - Fourier-based image resampling library.
 * Copyright 2010-2012 command-Q.org. All rights reserved.
 * This library is distributed under the terms of the GNU Lesser General Public License, Version 2.
 * 
 * dsp.h - Native DSP utilities.
 */

#ifndef DSP_H
#define DSP_H

#include "resine.h"

#include <math.h>

#if RSN_PRECISION == SINGLE
#	define	rsn_cos			cosf
#	define	rsn_sin			sinf
#	define	rsn_sqrt		sqrtf
#	define	rsn_fabs		fabsf
#	define	rsn_log			logf
#	define	rsn_pow			powf
#	define	rsn_copysign	copysignf
#elif RSN_PRECISION == LONG
#	define	rsn_cos			cosl
#	define	rsn_sin			sinl
#	define	rsn_sqrt		sqrtl
#	define	rsn_fabs		fabsl
#	define	rsn_log			logl
#	define	rsn_pow			powl
#	define	rsn_copysign	copysignl
#else
#	define	rsn_cos			cos
#	define	rsn_sin			sin
#	define	rsn_sqrt		sqrt
#	define	rsn_fabs		fabs
#	define	rsn_log			log
#	define	rsn_pow			pow
#	define	rsn_copysign	copysign
#endif

/* Canonical implementation of the i/DCT with (very) minor optimizations */
rsn_frequency CC(int,int);

/* Direct implementation */
void rsn_dct(int,int,int,rsn_image,rsn_spectrum);
void rsn_idct(int,int,int,rsn_spectrum,rsn_image);
void rsn_dct_direct(int,int,int,rsn_image,rsn_spectrum);
/* Row Column method */
void rsn_dct_rowcol(int,int,int,rsn_image,rsn_spectrum);
void rsn_idct_rowcol(int,int,int,rsn_spectrum,rsn_image);

#endif