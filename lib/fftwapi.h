/*
 * Resine - Fourier-based image resampling library.
 * Copyright 2010-2012 command-Q.org. All rights reserved.
 * This library is distributed under the terms of the GNU Lesser General Public License, Version 2.
 * 
 * fftwapi.h - FFTW API wrapper abstracting the precision namespace.
 */

#ifndef FFTWAPI_H
#define FFTWAPI_H

#if HAS_FFTW
#	include "resine.h"
#	include <fftw3.h>
#	if RSN_PRECISION == SINGLE
#		define rsn_fftw_malloc             fftwf_malloc
#		define rsn_fftw_free               fftwf_free
#		define rsn_fftw_plan               fftwf_plan
#		define rsn_fftw_plan_r2r_3d        fftwf_plan_r2r_3d
#		define rsn_fftw_plan_many_r2r      fftwf_plan_many_r2r
#		define rsn_fftw_destroy_plan       fftwf_destroy_plan
#		define rsn_fftw_execute            fftwf_execute
#		define rsn_fftw_cleanup            fftwf_cleanup
#		define rsn_fftw_init_threads       fftwf_init_threads
#		define rsn_fftw_plan_with_nthreads fftwf_plan_with_nthreads
#		define rsn_fftw_cleanup_threads    fftwf_cleanup_threads
#	elif RSN_PRECISION == LONG
#		define rsn_fftw_malloc             fftwl_malloc
#		define rsn_fftw_free               fftwl_free
#		define rsn_fftw_plan               fftwl_plan
#		define rsn_fftw_plan_r2r_3d        fftwl_plan_r2r_3d
#		define rsn_fftw_plan_many_r2r      fftwl_plan_many_r2r
#		define rsn_fftw_destroy_plan       fftwl_destroy_plan
#		define rsn_fftw_execute            fftwl_execute
#		define rsn_fftw_cleanup            fftwl_cleanup
#		define rsn_fftw_init_threads       fftwl_init_threads
#		define rsn_fftw_plan_with_nthreads fftwl_plan_with_nthreads
#		define rsn_fftw_cleanup_threads    fftwl_cleanup_threads
#	else
#		define rsn_fftw_malloc             fftw_malloc
#		define rsn_fftw_free               fftw_free
#		define rsn_fftw_plan               fftw_plan
#		define rsn_fftw_plan_r2r_3d        fftw_plan_r2r_3d
#		define rsn_fftw_plan_many_r2r      fftw_plan_many_r2r
#		define rsn_fftw_destroy_plan       fftw_destroy_plan
#		define rsn_fftw_execute            fftw_execute
#		define rsn_fftw_cleanup            fftw_cleanup
#		define rsn_fftw_init_threads       fftw_init_threads
#		define rsn_fftw_plan_with_nthreads fftw_plan_with_nthreads
#		define rsn_fftw_cleanup_threads    fftw_cleanup_threads
#	endif
#endif

#endif
