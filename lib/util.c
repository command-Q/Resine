/*
 * Resine - Fourier-based image resampling library.
 * Copyright 2010-2012 command-Q.org. All rights reserved.
 * This library is distributed under the terms of the GNU Lesser General Public License, Version 2.
 *
 * util.c - libresine utility and helper functions. 
 *	Includes a high precision timer for profiling and wrapper functions to ensure the proper memory allocation regardless of the transform type.
 */


#include "resine.h"

#include "fftwapi.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#if RSN_PRECISION == QUAD
#	include <quadmath.h>
#endif

struct stopwatch {
	clock_t* timers;
	unsigned int stops;
};

stopwatch stopwatch_create() {
	stopwatch watch = malloc(sizeof(struct stopwatch));
	watch->timers = malloc(sizeof(clock_t));
	watch->stops = 1;
	watch->timers[0] = clock();
	return watch;
}

void watch_add_stop(stopwatch watch) {
	watch->timers = realloc(watch->timers,sizeof(clock_t)*++watch->stops);
	watch->timers[watch->stops-1] = clock();
}

double elapsed(stopwatch watch,unsigned int stop) {
	return (clock() - watch->timers[stop]) / (double)CLOCKS_PER_SEC;
}

void destroy_watch(stopwatch watch) {
	free(watch->timers);
	free(watch);
}

void rsn_free(int type, void** data) {
	if(!*data) return;
	switch(type) {
#if HAS_FFTW
		case RSN_TRANSFORM_FFTW:rsn_fftw_free(*data);	break;
#endif
		default:                free(*data);            break;
	}
	*data = NULL;
}

void rsn_free_array(int type, int length, void*** data) {
	if(!*data) return;
	for(int i = 0; i < length; i++) rsn_free(type,&((*data)[i]));
	free(*data);
	*data = NULL;
}

void* rsn_malloc(rsn_config config, size_t base, int multiplier) {
	switch(base == sizeof(rsn_pel) ? 0 : config.transform) {
#if HAS_FFTW
		case RSN_TRANSFORM_FFTW:return memset(rsn_fftw_malloc(base*multiplier),0,base*multiplier);
#endif
		default:                return calloc(multiplier,base);
	}
}

void* rsn_malloc_array(rsn_config config, size_t base, int y, int x) {
	void** type = malloc(sizeof(void*)*y);
	for(int j = 0; j < y; j++) type[j] = rsn_malloc(config,base,x);
	return type;
}

void* rsn_realloc(rsn_config config, void* orig, size_t base, int multiplier) {
	switch(base == sizeof(rsn_pel) ? 0 : config.transform) {
#if HAS_FFTW
		case RSN_TRANSFORM_FFTW:
				rsn_free(config.transform,&orig);
				return memset(rsn_fftw_malloc(base*multiplier),0,base*multiplier);
#endif
		default:return realloc(orig,base*multiplier);
	}
}

void* rsn_realloc_array(rsn_config config, void** orig, size_t base, int y, int x) {
	orig = realloc(orig,sizeof(void*)*y);
	for(int j = 0; j < y; j++) orig[j] = rsn_realloc(config,orig[j],base,x);
	return orig;
}

void print_spectrum(int L, int M, int N, int precision, rsn_spectrum spectrum, const char* file) {
	int z,y,x,i,l,padding = 0;
	double max,min;
	max = min = spectrum[0];
	for(i = 0; i < L*M*N; i++) {
		if((l = log10f(spectrum[i])) > padding) padding = l;
		if(spectrum[i] < min) min = spectrum[i];
		else if(spectrum[i] > max) max = spectrum[i];
	}
	printf("Minimum frequency: %f\nMaximum frequency: %f\n",(double)min,(double)max);
	padding += precision + 2;
	char p[padding+1];
	FILE* f = fopen(file,"w");
	for(z = 0; z < L; z++) {
		fprintf(f,"Plane %d\n",z);
		for(y = 0; y < M; y++) {
			for(x = 0; x < N; x++) {
#if RSN_PRECISION == QUAD
				quadmath_snprintf(p,padding+1,"%*.*"RSN_PRECISION_FORMAT,padding,precision,spectrum[z*M*N+y*N+x]);
#else
				snprintf(p,padding+1,"%*.*"RSN_PRECISION_FORMAT,padding,precision,spectrum[z*M*N+y*N+x]);
#endif
				fputs(p,f);
			}
			fputc('\n',f);
		}
		fputc('\n',f);
	}
	fclose(f);
}

void print_image(int L, int M, int N, rsn_image img, const char* file) {
	int z,y,x;
	FILE* f = fopen(file,"w");
	for(z = 0; z < L; z++) {
		fprintf(f,"Plane %d\n",z);
		for(y = 0; y < M; y++) {
			for(x = 0; x < N; x++)
				fprintf(f,"%4d",img[y][x*L+z]);
			fputc('\n',f);
		}
		fputc('\n',f);
	}
	fclose(f);
}
