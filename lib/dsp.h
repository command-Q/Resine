/*
 * Resine - Fourier-based image resampling library.
 * Copyright 2010-2011 command-Q.org. All rights reserved.
 * This library is distributed under the terms of the GNU Lesser General Public License, Version 2.
 * 
 * dsp.h - Native DSP utilities.
 */

#ifndef DSP_H
#define DSP_H

#include "common.h"

#if PRECISION == SINGLE
#	define	rsn_cos			cosf
#	define	rsn_sqrt		sqrtf
#	define	rsn_fabs		fabsf
#	define	rsn_log			logf
#	define	rsn_pow			powf
#	define	rsn_copysign	copysignf
#elif PRECISION == QUADRUPLE
#	define	rsn_cos			cosl
#	define	rsn_sqrt		sqrtl
#	define	rsn_fabs		fabsl
#	define	rsn_log			logl
#	define	rsn_pow			powl
#	define	rsn_copysign	copysignl
#else
#	define	rsn_cos			cos
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
void rsn_idct_rowcol(int,int,int,rsn_spectrum,rsn_image); //FIXME

/* Logarithmic spectrogram image */
/* Absolute-value spectrogram (canonical) */
rsn_image spectrogram(int,int,int,rsn_spectrum);
/* Invertible center-anchored spectrogram */
rsn_image spectrogram_anchored(int,int,int,rsn_spectrum);
rsn_spectrum spectrogram_decompress(int,int,int,rsn_image);
/* Composites hidden image over spectrogram												*
 * The image to be composited is required to be the same size as the input, with alpha	*/
void composite_spectrum(int,int,int,rsn_spectrum,rsn_image);

#endif