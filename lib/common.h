/*
 * Resine - Fourier-based image resampling library.
 * Copyright 2010-2011 command-Q.org. All rights reserved.
 * This library is distributed under the terms of the GNU Lesser General Public License, Version 2.
 * 
 * common.h - Definitions and includes shared between all files.
 */

#ifndef COMMON_H
#define COMMON_H

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <math.h>
#ifdef THREADED
#	include <pthread.h>
#endif

#if HAS_FFTW
#	include <fftw3.h>
#	define RSN_TRANSFORM_DEFAULT RSN_TRANSFORM_FFTW
#else
#	pragma message("NOTICE: Compiling without FFTW support; native transform functions will be used. These are VERY slow!")
#	define RSN_TRANSFORM_DEFAULT RSN_TRANSFORM_NATIVE
#endif

#define RSN_VER_MAJOR			  0
#define RSN_VER_MINOR			  9
#define RSN_VER_MICRO			  2

#define RSN_TOSTRING(s)			#s
#define RSN_STRINGIFY(s)		RSN_TOSTRING(s)
#define RSN_VERSION				RSN_STRINGIFY(RSN_VER_MAJOR.RSN_VER_MINOR.RSN_VER_MICRO)

#define RSN_TRANSFORM_NONE		 -1
#define RSN_TRANSFORM_NATIVE	  0
#define RSN_TRANSFORM_FFTW		  1
#define RSN_TRANSFORM_KISS		  2

#define RSN_SCALING_STANDARD	  0
#define RSN_SCALING_SMOOTH		  1

#define RSN_GREED_LEAN			  0
#define RSN_GREED_PREALLOC		  1
#define RSN_GREED_RETAIN		  2
#define RSN_GREED_PREALLOC_RETAIN 3

typedef unsigned char	rsn_pel;
typedef rsn_pel*		rsn_line;
typedef rsn_line*		rsn_image;

#define SINGLE		1
#define DOUBLE		2
#define QUAD		4

#if PRECISION == SINGLE
#	define RSN_PRECISION "Single"
#	define RSN_PRECISION_FORMAT	"f"
	typedef float		rsn_frequency;
#	if HAS_FFTW
#		define rsn_fftw_malloc				fftwf_malloc
#		define rsn_fftw_free				fftwf_free
#		define rsn_fftw_plan_r2r_3d			fftwf_plan_r2r_3d
#		define rsn_fftw_plan_r2r_2d			fftwf_plan_r2r_2d
#		define rsn_fftw_destroy_plan		fftwf_destroy_plan
#		define rsn_fftw_execute				fftwf_execute
#		define rsn_fftw_execute_r2r			fftwf_execute_r2r
#		define rsn_fftw_plan				fftwf_plan
#		define rsn_fftw_cleanup				fftwf_cleanup
#		define rsn_fftw_plan_with_nthreads	fftwf_plan_with_nthreads
#	endif
#elif PRECISION == QUAD
#	define RSN_PRECISION "Quadruple"
#	define RSN_PRECISION_FORMAT	"Lf"
	typedef long double rsn_frequency;
#	if HAS_FFTW
#		define rsn_fftw_malloc				fftwl_malloc
#		define rsn_fftw_free				fftwl_free
#		define rsn_fftw_plan_r2r_3d			fftwl_plan_r2r_3d
#		define rsn_fftw_plan_r2r_2d			fftwl_plan_r2r_2d
#		define rsn_fftw_destroy_plan		fftwl_destroy_plan
#		define rsn_fftw_execute				fftwl_execute
#		define rsn_fftw_execute_r2r			fftwl_execute_r2r
#		define rsn_fftw_plan				fftwl_plan
#		define rsn_fftw_cleanup				fftwl_cleanup
#		define rsn_fftw_plan_with_nthreads	fftwl_plan_with_nthreads
#	endif
#else
#	define RSN_PRECISION "Double"
#	define RSN_PRECISION_FORMAT	"f"
	typedef double		rsn_frequency;
#	if HAS_FFTW
#		define rsn_fftw_malloc				fftw_malloc
#		define rsn_fftw_free				fftw_free
#		define rsn_fftw_plan_r2r_3d			fftw_plan_r2r_3d
#		define rsn_fftw_plan_r2r_2d			fftw_plan_r2r_2d
#		define rsn_fftw_destroy_plan		fftw_destroy_plan
#		define rsn_fftw_execute				fftw_execute
#		define rsn_fftw_execute_r2r			fftw_execute_r2r
#		define rsn_fftw_plan				fftw_plan
#		define rsn_fftw_cleanup				fftw_cleanup
#		define rsn_fftw_plan_with_nthreads	fftw_plan_with_nthreads
#	endif
#endif
typedef rsn_frequency*	rsn_spectrum;
typedef rsn_spectrum*	rsn_spectra;
/* Premultiplied coefficient data */
typedef rsn_spectrum**	rsn_wisdom;

typedef struct {
	int transform,scaling,verbosity,threads,greed;
} rsn_config;

typedef struct {
	rsn_config config;
	int channels,width,height,width_s,height_s;
} rsn_info;
typedef rsn_info*	rsn_infop;

typedef struct {
	rsn_image		image,		image_s;
	rsn_spectrum	freq_image, freq_image_s;
} rsn_data;
typedef rsn_data*	rsn_datap;

#endif