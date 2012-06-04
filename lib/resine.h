/*
 * Resine - Fourier-based image resampling library.
 * Copyright 2010-2012 command-Q.org. All rights reserved.
 * This library is distributed under the terms of the GNU Lesser General Public License, Version 2.
 *
 * resine.h - libresine public API.
 */

#ifndef RESINE_H
#define RESINE_H

#ifndef SKIP_CONFIG
#	include "resine_config.h"
#endif

#include <stddef.h>

#define RSN_VER_MAJOR 0
#define RSN_VER_MINOR 9
#define RSN_VER_MICRO 4

#define RSN_TOSTRING(s)  #s
#define RSN_STRINGIFY(s) RSN_TOSTRING(s)
#define RSN_VERSION      RSN_STRINGIFY(RSN_VER_MAJOR.RSN_VER_MINOR.RSN_VER_MICRO)

typedef unsigned char rsn_pel;
typedef rsn_pel*      rsn_line;
typedef rsn_line*     rsn_image;

#define SINGLE 1
#define DOUBLE 2
#define LONG   3
#define QUAD   4

#if RSN_PRECISION == SINGLE
#	define RSN_PRECISION_STR    "Single"
#	define RSN_PRECISION_FORMAT "f"
#	define RSN_SUFFIX_PRECISION(prefix,suffix) prefix ## f ## suffix
typedef float rsn_frequency;
#elif RSN_PRECISION == LONG
#	define RSN_PRECISION_STR    "Long"
#	define RSN_PRECISION_FORMAT "Lf"
#	define RSN_SUFFIX_PRECISION(prefix,suffix) prefix ## l ## suffix
typedef long double rsn_frequency;
#elif RSN_PRECISION == QUAD
#	define RSN_PRECISION_STR    "Quad"
#	define RSN_PRECISION_FORMAT "Qf"
#	define RSN_SUFFIX_PRECISION(prefix,suffix) prefix ## q ## suffix
typedef __float128 rsn_frequency;
#else
#	define RSN_PRECISION_STR    "Double"
#	define RSN_PRECISION_FORMAT "f"
#	define RSN_SUFFIX_PRECISION(prefix,suffix) prefix ## suffix
typedef double rsn_frequency;
#endif
typedef rsn_frequency* rsn_spectrum;
typedef rsn_spectrum*  rsn_spectra;
/* Premultiplied coefficient data */
typedef rsn_spectrum** rsn_wisdom;

#define RSN_TRANSFORM_NONE   (-1)
#define RSN_TRANSFORM_NATIVE   0
#define RSN_TRANSFORM_FFTW     1
#define RSN_TRANSFORM_KISS     2

#if HAS_FFTW
#	define RSN_TRANSFORM_DEFAULT RSN_TRANSFORM_FFTW
#elif HAS_KISS
#	define RSN_TRANSFORM_DEFAULT RSN_TRANSFORM_KISS
#else
#	define RSN_TRANSFORM_DEFAULT RSN_TRANSFORM_NATIVE
#endif

#define RSN_SCALING_STANDARD 0

#define RSN_GREED_LEAN            0
#define RSN_GREED_PREALLOC        1
#define RSN_GREED_RETAIN          2
#define RSN_GREED_PREALLOC_RETAIN 3

typedef struct {
	int transform, scaling, verbosity, threads, greed;
} rsn_config;

typedef struct {
	rsn_config config;
	int channels, width, height, width_s, height_s;
} rsn_info;
typedef rsn_info* rsn_infop;

typedef struct {
	rsn_image    image,      image_s;
	rsn_spectrum freq_image, freq_image_s;
} rsn_data;
typedef rsn_data* rsn_datap;

/* Returns the default configuration, suitable for most cases */
rsn_config rsn_defaults();

/* Returns the input image scaled to the dimensions given in the info struct. */
rsn_image resine(rsn_info,rsn_image);

/* Constructs a data container according to the configuration provided in rsn_info.
 * Image data is referenced, not copied. */
rsn_datap rsn_init(rsn_info,rsn_image);

/* High-level wrapper for forward transform, scale, inverse transform */
void resine_data(rsn_info,rsn_datap);

/* Mid-level transform wrappers */
void rsn_decompose(rsn_info,rsn_datap);
void rsn_scale(rsn_info,rsn_datap);
void rsn_recompose(rsn_info,rsn_datap);

/* Cleans out Resine data, leaving only the output image.
 * If the returned image will not be freed by the caller, it will leak. For this behavior, use rsn_destroy instead. */
rsn_image rsn_cleanup(rsn_info,rsn_datap);

/* Destroys all Resine-created data. It or rsn_cleanup should match each rsn_init.
 * Like rsn_cleanup, the data pointer is invalid after this call. */
void rsn_destroy(rsn_info,rsn_datap);


/* Utility functions */

typedef struct stopwatch* stopwatch;
stopwatch stopwatch_create();
void watch_add_stop(stopwatch);
double elapsed(stopwatch,unsigned int stop);
void destroy_watch(stopwatch);

/* Convenience, not required unless manipulating data members directly. */
void* rsn_malloc(rsn_config, size_t base, int multiplier);
void* rsn_malloc_array(rsn_config, size_t, int length, int multiplier);
void* rsn_realloc(rsn_config, void*, size_t base, int multiplier);
void* rsn_realloc_array(rsn_config, void**, size_t base, int length, int multiplier);
void  rsn_free(int transform, void**);
void  rsn_free_array(int transform, int length, void***);

void print_spectrum(int channels, int height, int width, int precision, rsn_spectrum, const char* filename);
void print_image(int channels, int height, int width, rsn_image, const char* filename);


/* Extra DSP functions */

/* Logarithmic spectrogram images */
/* Absolute-value spectrogram (canonical) */
rsn_image spectrogram(int channels, int height, int width, rsn_spectrum);

/* Invertible center-anchored spectrogram */
rsn_image spectrogram_anchored(int channels, int height, int width, rsn_spectrum);
rsn_spectrum spectrogram_decompress(int channels, int height, int width, rsn_image);

/* Composites hidden image over spectrogram.
 * The image to be composited is required to be the same size as the input, with alpha.
 * Needs more testing. */
void composite_spectrum(int channels, int height, int width, rsn_spectrum,rsn_image);

#endif
