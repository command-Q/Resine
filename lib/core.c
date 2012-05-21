/*
 * Resine - Fourier-based image resampling library.
 * Copyright 2010-2012 command-Q.org. All rights reserved.
 * This library is distributed under the terms of the GNU Lesser General Public License, Version 2.
 * 
 * core.c - libresine core.
 */

#include "resine.h"

#include "fftwapi.h"
#include "dsp.h"

#include <stdbool.h>
#include <stdlib.h>

#if HAS_KISS
#	if RSN_PRECISION == LONG
#		pragma message("KISS FFT does not support long precision, KISS operations will use double instead.")
#		define kiss_fft_scalar double
#	else
#		define kiss_fft_scalar rsn_frequency
#	endif
#include <kiss_fftndr.h>
#endif

/* Methods here should be either public or fully local, so no separate private header */
void rsn_decompose_native(rsn_info,rsn_datap);
void rsn_recompose_native(rsn_info,rsn_datap);
#if HAS_KISS
void rsn_decompose_kiss(rsn_info,rsn_datap);
void rsn_recompose_kiss(rsn_info,rsn_datap);
#endif
#if HAS_FFTW
void rsn_decompose_fftw(rsn_info,rsn_datap);
void rsn_recompose_fftw(rsn_info,rsn_datap);
void rsn_decompose_fftw_2d(rsn_info,rsn_datap);
void rsn_recompose_fftw_2d(rsn_info,rsn_datap);
#endif
void rsn_scale_standard(rsn_info,rsn_datap);
void rsn_upscale_smooth(rsn_info,rsn_datap);

// Will replace the function call in a future rev
#define RSN_DEFAULTS (rsn_config) {\
.transform = RSN_TRANSFORM_DEFAULT,\
.scaling   = RSN_SCALING_STANDARD,\
.verbosity = 0,\
.threads   = 1,\
.greed     = RSN_GREED_RETAIN\
}
rsn_config rsn_defaults() {
	return RSN_DEFAULTS;
}

/* Initialize structs, set up threads */
rsn_datap rsn_init(rsn_info info, rsn_image img) {
	rsn_datap data = malloc(sizeof(rsn_data));
	data->image = img;
	data->freq_image = NULL;
	data->freq_image_s = NULL;
	data->image_s = NULL;

	if(info.config.greed & RSN_GREED_PREALLOC) {
		data->freq_image   = rsn_malloc(info.config,sizeof(rsn_frequency),info.channels*info.height*info.width);
		data->freq_image_s = rsn_malloc(info.config,sizeof(rsn_frequency),info.channels*info.height_s*info.width_s);
		data->image_s      = rsn_malloc_array(info.config,sizeof(rsn_pel),info.height_s,info.width_s*info.channels);
	}
#if RSN_IS_THREADED && HAS_FFTW
	fftw_init_threads();
#endif
	return data;
}

/* Transform function wrappers */
void rsn_decompose(rsn_info info, rsn_datap data) {
	if(!data->freq_image) 
		data->freq_image = rsn_malloc(info.config,sizeof(rsn_frequency),info.channels*info.height*info.width);

	switch (info.config.transform) {
#if HAS_FFTW
		case RSN_TRANSFORM_FFTW:rsn_decompose_fftw_2d(info,data); break;
#endif
#if HAS_KISS
		case RSN_TRANSFORM_KISS:rsn_decompose_kiss(info,data);    break;
#endif
		default:                rsn_decompose_native(info,data);  break;
	}
}

void rsn_recompose(rsn_info info, rsn_datap data) {
	if(!data->image_s)
		data->image_s = rsn_malloc_array(info.config,sizeof(rsn_pel),info.height_s,info.width_s*info.channels);

	switch (info.config.transform) {
#if HAS_FFTW
		case RSN_TRANSFORM_FFTW:rsn_recompose_fftw_2d(info,data); break;
#endif
#if HAS_KISS
		case RSN_TRANSFORM_KISS:rsn_recompose_kiss(info,data);    break;
#endif
		default:                rsn_recompose_native(info,data);  break;
	}
	
	if(!(info.config.greed & RSN_GREED_RETAIN)) rsn_free(info.config.transform,(void**)&data->freq_image_s);
}

/* Native transform functions (SLOW) */
void rsn_decompose_native(rsn_info info, rsn_datap data) {
	rsn_dct_rowcol(info.channels,info.height,info.width,data->image,data->freq_image);
}

void rsn_recompose_native(rsn_info info, rsn_datap data) {
	rsn_idct_rowcol(info.channels,info.height_s,info.width_s,data->freq_image_s,data->image_s);
}

/* KissFFT transform functions */
#if HAS_KISS
void rsn_decompose_kiss(rsn_info info, rsn_datap data) {
	kiss_fftndr_cfg cfg = kiss_fftndr_alloc((int[]){info.height*2,info.width*2},2,false,NULL,NULL);
	kiss_fft_scalar* mirrored = malloc(sizeof(kiss_fft_scalar)*info.height*2*info.width*2);
	kiss_fft_cpx* cpxF = malloc(sizeof(kiss_fft_cpx)*(info.width+1)*info.height*2);
	kiss_fft_cpx* shift_matrix = malloc(sizeof(kiss_fft_cpx)*(info.width+1)*info.height*2);

	/* Shift is a simple factorization of
		e^(-I*PI*n / 2N) * e^(-I*PI*m / 2M)
	 */
	kiss_fft_scalar EXP = -M_PI/(2.0*info.width*info.height);
	for(int y = 0; y < info.height; y++)
		for(int x = 0; x < info.width; x++)
			shift_matrix[y*(info.width+1)+x] = (kiss_fft_cpx) {
				rsn_cos(EXP*(x*info.width+y*info.height)), 
				rsn_sin(EXP*(x*info.width+y*info.height))
			};

	for(int z = 0; z < info.channels; z++) {
		for(int y = 0; y < info.height; y++) {
			for(int x = 0; x < info.width; x++) {
				mirrored[y*info.width*2+x] = data->image[y][x*info.channels+z];
				mirrored[y*info.width*2+x+info.width] = data->image[y][(info.width-1-x)*info.channels+z];
			}
			memcpy(mirrored + (info.height*2-1-y)*info.width*2,mirrored + y*info.width*2,sizeof(kiss_fft_scalar)*info.width*2);
		}

		kiss_fftndr(cfg,mirrored,cpxF);

		for(int y = 0; y < info.height; y++)
			for(int x = 0; x < info.width; x++)
				data->freq_image[z*info.height*info.width+y*info.width+x] = 
				cpxF[y*(info.width+1)+x].r * shift_matrix[y*(info.width+1)+x].r - 
				cpxF[y*(info.width+1)+x].i * shift_matrix[y*(info.width+1)+x].i;
//In terms of C99 complex
//				creal((cpxF[y*(info.width+1)+x].r + I*cpxF[y*(info.width+1)+x].i) * cexp(I*(EXP*(x*info.width+y*info.height))));

	}
	free(cpxF);
	free(mirrored);
	free(cfg);
}

void rsn_recompose_kiss(rsn_info info, rsn_datap data) {
	kiss_fftndr_cfg cfg = kiss_fftndr_alloc((int[]){info.height_s*2,info.width_s*2},2,true,NULL,NULL);
	kiss_fft_cpx* cpxF = malloc(sizeof(kiss_fft_cpx)*(info.width_s+1)*info.height_s*2);
	kiss_fft_cpx* shift_matrix = malloc(sizeof(kiss_fft_cpx)*(info.width_s+1)*info.height_s*2);
	kiss_fft_scalar* mirrored = malloc(sizeof(kiss_fft_scalar)*info.height_s*2*info.width_s*2);

	// pre-zero nyquist
	for(int y = 0; y < info.height_s*2; y++)
		cpxF[y*(info.width_s+1)+info.width_s] = (kiss_fft_cpx){0};
	for(int x = 0; x < info.width_s; x++)
		cpxF[info.height_s*(info.width_s+1)+x] = (kiss_fft_cpx){0};

	/* Shift:
		e^(I*PI*n / 2N) * e^(I*PI*m / 2M)
	 */
	kiss_fft_scalar EXP = M_PI/(2.0*info.width_s*info.height_s);
	rsn_frequency* coeff = data->freq_image_s; // iterate for infinitesimal speedup

	for(int x = 0; x < info.width_s; x++)
		shift_matrix[x] = (kiss_fft_cpx) {
			rsn_cos(EXP*(x*info.width_s)),
			rsn_sin(EXP*(x*info.width_s))
		};
	for(int y = 1; y < info.height_s; y++)
		for(int x = 0; x < info.width_s; x++) {
			shift_matrix[y*(info.width_s+1)+x] = (kiss_fft_cpx) {
				rsn_cos(EXP*(x*info.width_s+y*info.height_s)),
				rsn_sin(EXP*(x*info.width_s+y*info.height_s))
			};
			shift_matrix[(info.height_s*2-y)*(info.width_s+1)+x] = (kiss_fft_cpx) {
				-rsn_cos(EXP*(x*info.width_s+(info.height_s*2-y)*info.height_s)),
				-rsn_sin(EXP*(x*info.width_s+(info.height_s*2-y)*info.height_s))
			};
		}

	for(int z = 0; z < info.channels; z++) {
		for(int x = 0; x < info.width_s; x++,coeff++)
			cpxF[x] = (kiss_fft_cpx) {
				*coeff * shift_matrix[x].r,
				*coeff * shift_matrix[x].i
			};
		for(int y = 1; y < info.height_s; y++)
			for(int x = 0; x < info.width_s; x++,coeff++) {
				// Un-shift the upper-left half of the spectrum (DCT portion)
				kiss_fft_cpx shift = shift_matrix[y*(info.width_s+1)+x];
				cpxF[y*(info.width_s+1)+x] = (kiss_fft_cpx) {
					*coeff * shift.r,
					*coeff * shift.i
				};
				// Re-create the lower-left half. The entire right side is reconstructed by KISS for real transforms.
				shift =  shift_matrix[(info.height_s*2-y)*(info.width_s+1)+x];
				cpxF[(info.height_s*2-y)*(info.width_s+1)+x] = (kiss_fft_cpx) {
					*coeff * shift.r,
					*coeff * shift.i
				};
			}

		kiss_fftndri(cfg,cpxF,mirrored);

		for(int y = 0; y < info.height_s; y++)
			for(int x = 0; x < info.width_s; x++) {
				mirrored[y*info.width_s*2+x] /= 4*info.width_s*info.height_s;
				data->image_s[y][x*info.channels+z] = mirrored[y*info.width_s*2+x] > 255 ? 255 : mirrored[y*info.width_s*2+x] < 0 ? 0 : round(mirrored[y*info.width_s*2+x]);
			}
	}
	free(mirrored);
	free(cpxF);
	free(cfg);
}
#endif

/* FFTW transform functions */
#if HAS_FFTW
void rsn_decompose_fftw(rsn_info info, rsn_datap data) {
	int z,y,x;
	for(z = 0; z < info.channels; z++)
		for(y = 0; y < info.height; y++) 
			for(x = 0; x < info.width; x++)
				data->freq_image[z*info.height*info.width+y*info.width+x] = data->image[y][x*info.channels+z];

#if RSN_IS_THREADED 
	rsn_fftw_plan_with_nthreads(info.config.threads);
#endif
	rsn_fftw_plan p = rsn_fftw_plan_r2r_3d(info.channels,info.height,info.width,data->freq_image,data->freq_image,FFTW_REDFT10,FFTW_REDFT10,FFTW_REDFT10,FFTW_ESTIMATE); //FFTW_MEASURE
	rsn_fftw_execute(p);
	rsn_fftw_destroy_plan(p);
}

void rsn_recompose_fftw(rsn_info info, rsn_datap data) {
	int z,y,x;
	rsn_spectrum output = rsn_fftw_malloc(sizeof(rsn_frequency)*info.channels*info.width_s*info.height_s);
#if RSN_IS_THREADED 
	rsn_fftw_plan_with_nthreads(info.config.threads);
#endif
	rsn_fftw_plan ip = rsn_fftw_plan_r2r_3d(info.channels,info.height_s,info.width_s,data->freq_image_s,output,FFTW_REDFT01,FFTW_REDFT01,FFTW_REDFT01,FFTW_ESTIMATE); //FFTW_MEASURE
	rsn_fftw_execute(ip);
	rsn_fftw_destroy_plan(ip);

	for(z = 0; z < info.channels; z++)
		for(y = 0; y < info.height_s; y++)
			for(x = 0; x < info.width_s; x++)
				data->image_s[y][x*info.channels+z] = output[z*info.height_s*info.width_s+y*info.width_s+x] > 255 ? 255 :
				                                      output[z*info.height_s*info.width_s+y*info.width_s+x] < 0 ? 0 :
				                                      round(output[z*info.height_s*info.width_s+y*info.width_s+x]);
	rsn_fftw_free(output);
}

void rsn_decompose_fftw_2d(rsn_info info, rsn_datap data) {
	const int dims[2] = {info.height,info.width};
	fftw_r2r_kind kind[info.channels];
	for(int z = 0; z < info.channels; z++) kind[z] = FFTW_REDFT10;
#if RSN_IS_THREADED 
	rsn_fftw_plan_with_nthreads(info.config.threads);
#endif

	rsn_fftw_plan p = fftw_plan_many_r2r(2,dims,info.channels,
	                                     data->freq_image,dims,1,info.width*info.height,
	                                     data->freq_image,dims,1,info.width*info.height,
	                                     kind,FFTW_ESTIMATE);

	rsn_spectrum fptr = data->freq_image;
	for(int z = 0; z < info.channels; z++)
		for(int y = 0; y < info.height; y++)
			for(int x = 0; x < info.width; x++,fptr++)
				*fptr = data->image[y][x*info.channels+z];

	rsn_fftw_execute(p);
	rsn_fftw_destroy_plan(p);
}

void rsn_recompose_fftw_2d(rsn_info info, rsn_datap data) {
	const int dims[2] = {info.height_s,info.width_s};
	fftw_r2r_kind kind[info.channels];
	for(int z = 0; z < info.channels; z++) kind[z] = FFTW_REDFT01;
#if RSN_IS_THREADED 
	rsn_fftw_plan_with_nthreads(info.config.threads);
#endif
	rsn_spectrum f = rsn_fftw_malloc(sizeof(rsn_frequency)*info.channels*info.height_s*info.width_s);

	rsn_fftw_plan p = fftw_plan_many_r2r(2,dims,info.channels,
	                                     data->freq_image_s,dims,1,info.width_s*info.height_s,
	                                     f,dims,1,info.width_s*info.height_s,
	                                     kind,FFTW_ESTIMATE);

	rsn_fftw_execute(p);
	rsn_fftw_destroy_plan(p);

	rsn_spectrum fptr = f;
	for(int z = 0; z < info.channels; z++)
		for(int y = 0; y < info.height_s; y++)
			for(int x = 0; x < info.width_s; x++, fptr++) {
				*fptr /= 4*info.width_s*info.height_s;
				data->image_s[y][x*info.channels+z] = *fptr > 255 ? 255 : *fptr < 0 ? 0 : round(*fptr);
			}
	rsn_fftw_free(f);
}
#endif

/* Scaling */
void rsn_scale(rsn_info info, rsn_datap data) {
	if(!data->freq_image_s)
		data->freq_image_s = rsn_malloc(info.config,sizeof(rsn_frequency),info.channels*info.height_s*info.width_s);

	switch (info.config.scaling) {
		case RSN_SCALING_SMOOTH:rsn_upscale_smooth(info,data); break;
		default:                rsn_scale_standard(info,data); break;
	}

	if(!(info.config.greed & RSN_GREED_RETAIN)) rsn_free(info.config.transform,(void**)&data->freq_image);
}

void rsn_scale_standard(rsn_info info, rsn_datap data) {
	int z,y,x;
	rsn_frequency scale = (info.width_s*info.height_s)/(rsn_frequency)(info.width*info.height);

	int ylim = info.height < info.height_s ? info.height : info.height_s;
	int xlim = info.width < info.width_s ? info.width : info.width_s;
	for(z = 0; z < info.channels; z++)
		for(y = 0; y < ylim; y++)
			for(x = 0; x < xlim; x++)
				data->freq_image_s[z*info.height_s*info.width_s+y*info.width_s+x] = data->freq_image[z*info.height*info.width+y*info.width+x] * scale;
}

/* 
 This method was initially coded to use the coefficients from another upsampling method to complement the empty coefficients created by standard upsampling.
   However, changes to the library since have made it incompatible. It remains a stub for future reimplementation.
 */
void rsn_upscale_smooth(rsn_info info, rsn_datap data) {
	fprintf(stderr,"WARNING: Smooth upscaling is currently broken. Performing standard scaling instead.");
	rsn_scale_standard(info,data);
}

rsn_image resine(rsn_info info, rsn_image image) {
	rsn_datap data = rsn_init(info,image);
	resine_data(info,data);
	return rsn_cleanup(info,data);
}

void resine_data(rsn_info info, rsn_datap data) {
	stopwatch watch = NULL; // shut up clang
	if(info.config.verbosity) watch = stopwatch_create();
	
	rsn_decompose(info,data);

	if(info.config.verbosity) {
		printf("Forward transform completed in %f seconds\n",elapsed(watch,0));
		watch_add_stop(watch);
	}

	rsn_scale(info,data);

	if(info.config.verbosity) {
		printf("Scaling completed in %f seconds\n",elapsed(watch,1));
		watch_add_stop(watch);
	}

	rsn_recompose(info,data);

	if(info.config.verbosity) {
		printf("Inverse transform completed in %f seconds\n",elapsed(watch,2));
		printf("Total processing took %f seconds\n",elapsed(watch,0));
		destroy_watch(watch);
	}
}

rsn_image rsn_cleanup(rsn_info info, rsn_datap data) {
#if HAS_FFTW
#	if RSN_IS_THREADED
	fftw_cleanup_threads();
#	else
	rsn_fftw_cleanup();
#	endif	
#endif	

	rsn_free(info.config.transform,(void**)&data->freq_image);
	rsn_free(info.config.transform,(void**)&data->freq_image_s);
	rsn_image out = data->image_s;
	free(data);
	data = NULL;
	return out;
}

void rsn_destroy(rsn_info info, rsn_datap data) {
	rsn_free_array(RSN_TRANSFORM_NONE,info.height_s,(void***)&data->image_s);
	rsn_cleanup(info,data);
}
