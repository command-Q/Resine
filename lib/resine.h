/*
 * Resine - Fourier-based image resampling library.
 * Copyright 2010-2011 command-Q.org. All rights reserved.
 * This library is distributed under the terms of the GNU Lesser General Public License, Version 2.
 * 
 * resine.h - libresine core.
 */

#ifndef RESINE_H
#define RESINE_H

#include <string.h>

#include "common.h"
#include "util.h"
#include "dsp.h"

/* Initialize structs, set up threads */
rsn_config	rsn_defaults();
rsn_datap	rsn_init(rsn_info,rsn_image);

/* Transform function wrappers */
void rsn_decompose(rsn_info,rsn_datap);
void rsn_recompose(rsn_info,rsn_datap);

/* Native transform functions (SLOW) */
void rsn_decompose_native(rsn_info,rsn_datap);
void rsn_recompose_native(rsn_info,rsn_datap);

/* FFTW transform functions */
#if HAS_FFTW
void rsn_decompose_fftw(rsn_info,rsn_datap);
void rsn_recompose_fftw(rsn_info,rsn_datap);
void rsn_decompose_fftw_2d(rsn_info,rsn_datap);
void rsn_recompose_fftw_2d(rsn_info,rsn_datap);
#endif

/* Where the magic happens */
void rsn_scale(rsn_info,rsn_datap); // Wrapper
void rsn_scale_standard(rsn_info,rsn_datap);
void rsn_upscale_smooth(rsn_info,rsn_datap);

/* Core wrapper methods for transform_forward, scale, transform_inverse */
rsn_image resine(rsn_info,rsn_image);
rsn_image resine_data(rsn_info,rsn_datap);

/* Housekeeping. */
/* Cleans out Resine data, leaving only the output image */
rsn_image rsn_cleanup(rsn_info,rsn_datap);
/* Destroys all Resine data, must match each rsn_init */
void rsn_destroy(rsn_info,rsn_datap);

#endif