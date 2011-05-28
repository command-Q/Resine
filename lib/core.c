/*
 * Resine - Fourier-based image resampling library.
 * Copyright 2010-2011 command-Q.org. All rights reserved.
 * This library is distributed under the terms of the GNU Lesser General Public License, Version 2.
 * 
 * core.c - libresine core.
 */

#include "resine.h"

#include "fftwapi.h"
#include "dsp.h"

#include <stdlib.h>

/* Methods here should be either public or fully local, so no separate private header */
void rsn_decompose_native(rsn_info,rsn_datap);
void rsn_recompose_native(rsn_info,rsn_datap);
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
.transform	=	RSN_TRANSFORM_DEFAULT,\
.scaling	=	RSN_SCALING_STANDARD,\
.verbosity	=	0,\
.threads	=	1,\
.greed		=	RSN_GREED_RETAIN\
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
		data->image_s	   = rsn_malloc_array(info.config,sizeof(rsn_pel),info.height_s,info.width_s*info.channels);
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
		case RSN_TRANSFORM_FFTW:rsn_decompose_fftw_2d(info,data);	break;
#endif
		default:				rsn_decompose_native(info,data);	break;
	}
	
/* Bad idea, let the client handle this one
 *	if(!(info.config.greed & RSN_GREED_RETAIN)) rsn_free_array(RSN_TRANSFORM_NONE,info.height,(void***)&data->image);
 */
}

void rsn_recompose(rsn_info info, rsn_datap data) {
	if(!data->image_s)
		data->image_s = rsn_malloc_array(info.config,sizeof(rsn_pel),info.height_s,info.width_s*info.channels);
	
	switch (info.config.transform) {
#if HAS_FFTW
		case RSN_TRANSFORM_FFTW:rsn_recompose_fftw_2d(info,data);	break;
#endif
		default:				rsn_recompose_native(info,data);	break;
	}
	
	if(!(info.config.greed & RSN_GREED_RETAIN)) rsn_free(info.config.transform,(void**)&data->freq_image_s);
}

/* Native transform functions (SLOW) */
void rsn_decompose_native(rsn_info info, rsn_datap data) {
	rsn_dct_rowcol(info.channels,info.height,info.width,data->image,data->freq_image);
}

void rsn_recompose_native(rsn_info info, rsn_datap data) {
	rsn_idct(info.channels,info.height_s,info.width_s,data->freq_image_s,data->image_s);
}

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
				data->image_s[y][x*info.channels+z] =	output[z*info.height_s*info.width_s+y*info.width_s+x] > 255 ? 255 :
														output[z*info.height_s*info.width_s+y*info.width_s+x] < 0 ? 0 :
														round(output[z*info.height_s*info.width_s+y*info.width_s+x]);
	rsn_fftw_free(output);
}



void rsn_decompose_fftw_2d(rsn_info info, rsn_datap data) {
	int z,y,x;
	rsn_spectra planes = rsn_malloc_array(info.config,sizeof(rsn_frequency),info.channels,info.height*info.width);

	for(z = 0; z < info.channels; z++)
		for(y = 0; y < info.height; y++) 
			for(x = 0; x < info.width; x++)
				planes[z][y*info.width+x] = data->image[y][x*info.channels+z];

#if RSN_IS_THREADED 
	rsn_fftw_plan_with_nthreads(info.config.threads);
#endif

	rsn_fftw_plan p = rsn_fftw_plan_r2r_2d(info.height,info.width,planes[0],planes[0],FFTW_REDFT10,FFTW_REDFT10,FFTW_ESTIMATE); //FFTW_MEASURE
	rsn_fftw_execute(p);	
	for(z = 1; z < info.channels; z++)
		rsn_fftw_execute_r2r(p,planes[z],planes[z]);
	rsn_fftw_destroy_plan(p);
	
	for(z = 0; z < info.channels; z++)
		for(y = 0; y < info.height; y++) 
			for(x = 0; x < info.width; x++)
				data->freq_image[z*info.height*info.width+y*info.width+x] = planes[z][y*info.width+x];

	rsn_free_array(info.config.transform,info.channels,(void***)&planes);
}

void rsn_recompose_fftw_2d(rsn_info info, rsn_datap data) {
	int z,y,x;
	rsn_spectra planes = rsn_malloc_array(info.config,sizeof(rsn_frequency),info.channels,info.height_s*info.width_s);
	for(z = 0; z < info.channels; z++)
		for(y = 0; y < info.height_s; y++)
			for(x = 0; x < info.width_s; x++)
				planes[z][y*info.width_s+x] = data->freq_image_s[z*info.height_s*info.width_s+y*info.width_s+x];
	
#if RSN_IS_THREADED 
	rsn_fftw_plan_with_nthreads(info.config.threads);
#endif
	rsn_fftw_plan ip = rsn_fftw_plan_r2r_2d(info.height_s,info.width_s,planes[0],planes[0],FFTW_REDFT01,FFTW_REDFT01,FFTW_ESTIMATE); //FFTW_MEASURE
	rsn_fftw_execute(ip);	
	for(z = 1; z < info.channels; z++)
		rsn_fftw_execute_r2r(ip,planes[z],planes[z]);
	rsn_fftw_destroy_plan(ip);

	for(z = 0; z < info.channels; z++)
		for(y = 0; y < info.height_s; y++)
			for(x = 0; x < info.width_s; x++) {
				planes[z][y*info.width_s+x] *= 0.5/rsn_sqrt(info.width*info.height);
				data->image_s[y][x*info.channels+z] =	planes[z][y*info.width_s+x] > 255 ? 255 : planes[z][y*info.width_s+x] < 0 ? 0 :
														round(planes[z][y*info.width_s+x]);
			}

	rsn_free_array(info.config.transform,info.channels,(void***)&planes);
}
#endif

/* Scaling */
void rsn_scale(rsn_info info, rsn_datap data) {
	if(!data->freq_image_s)
		data->freq_image_s = rsn_malloc(info.config,sizeof(rsn_frequency),info.channels*info.height_s*info.width_s);

	switch (info.config.scaling) {
		case RSN_SCALING_SMOOTH:rsn_upscale_smooth(info,data);	break;
		default:				rsn_scale_standard(info,data);	break;
	}

	if(!(info.config.greed & RSN_GREED_RETAIN)) rsn_free(info.config.transform,(void**)&data->freq_image);
}

void rsn_scale_standard(rsn_info info, rsn_datap data) {
	int z,y,x;
	rsn_frequency scale;
#ifdef HAS_FFTW // This is so stupid
	if(info.config.transform == RSN_TRANSFORM_FFTW)
		scale = 0.5/rsn_sqrt(info.width*info.height);
	else
#endif
		scale = rsn_sqrt(info.width_s*info.height_s)/rsn_sqrt(info.width*info.height);
	
	int ylim = info.height < info.height_s ? info.height : info.height_s;
	int xlim = info.width < info.width_s ? info.width : info.width_s;
	for(z = 0; z < info.channels; z++)
		for(y = 0; y < ylim; y++)
			for(x = 0; x < xlim; x++)
				data->freq_image_s[z*info.height_s*info.width_s+y*info.width_s+x] = 
					data->freq_image[z*info.height*info.width+y*info.width+x] * scale;
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

// Bad idea, let the client handle this one
//	rsn_free_array(RSN_TRANSFORM_NONE,info.height,(void***)&data->image);		

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