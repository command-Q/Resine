/*
 * Resine - Fourier-based image resampling library.
 * Copyright 2010-2012 command-Q.org. All rights reserved.
 * This library is distributed under the terms of the GNU Lesser General Public License, Version 2.
 * 
 * fftwapi.h - FFTW API wrapper abstracting the precision namespace.
 */

#ifndef FFTWAPI_H
#define FFTWAPI_H

#include "resine.h"

#if HAS_FFTW
#	include <fftw3.h>
#	define rsn_fftw_malloc             RSN_SUFFIX_PRECISION(fftw,_malloc)
#	define rsn_fftw_free               RSN_SUFFIX_PRECISION(fftw,_free)
#	define rsn_fftw_plan               RSN_SUFFIX_PRECISION(fftw,_plan)
#	define rsn_fftw_plan_r2r_3d        RSN_SUFFIX_PRECISION(fftw,_plan_r2r_3d)
#	define rsn_fftw_plan_many_r2r      RSN_SUFFIX_PRECISION(fftw,_plan_many_r2r)
#	define rsn_fftw_destroy_plan       RSN_SUFFIX_PRECISION(fftw,_destroy_plan)
#	define rsn_fftw_execute            RSN_SUFFIX_PRECISION(fftw,_execute)
#	define rsn_fftw_cleanup            RSN_SUFFIX_PRECISION(fftw,_cleanup)
#	define rsn_fftw_init_threads       RSN_SUFFIX_PRECISION(fftw,_init_threads)
#	define rsn_fftw_plan_with_nthreads RSN_SUFFIX_PRECISION(fftw,_plan_with_nthreads)
#	define rsn_fftw_cleanup_threads    RSN_SUFFIX_PRECISION(fftw,_cleanup_threads)
#endif

#endif
