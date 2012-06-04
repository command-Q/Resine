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
#if RSN_PRECISION == QUAD
#	include <quadmath.h>
#	define RSN_SUFFIX_CONSTANT(token) token ## q
#else
#	define RSN_SUFFIX_CONSTANT(token) token
#endif

#define rsn_cos      RSN_SUFFIX_PRECISION(cos,)
#define rsn_sin      RSN_SUFFIX_PRECISION(sin,)
#define rsn_sqrt     RSN_SUFFIX_PRECISION(sqrt,)
#define rsn_fabs     RSN_SUFFIX_PRECISION(fabs,)
#define rsn_log      RSN_SUFFIX_PRECISION(log,)
#define rsn_pow      RSN_SUFFIX_PRECISION(pow,)
#define rsn_copysign RSN_SUFFIX_PRECISION(copysign,)
#define RSN_PI       RSN_SUFFIX_CONSTANT(M_PI)
#define RSN_SQRT1_2  RSN_SUFFIX_CONSTANT(M_SQRT1_2)
#define RSN_E        RSN_SUFFIX_CONSTANT(M_E)

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
