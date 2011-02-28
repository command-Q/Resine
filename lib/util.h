/*
 * Resine - Fourier-based image resampling library.
 * Copyright 2010-2011 command-Q.org. All rights reserved.
 * This library is distributed under the terms of the GNU Lesser General Public License, Version 2.
 * 
 * util.h - libresine utility and helper functions. 
 *	Includes a high precision timer for profiling and wrapper functions to insure the proper memory allocation regardless of the transform type.
 */

#ifndef UTIL_H
#define UTIL_H

#include <string.h>
#include <sys/time.h>
#include "common.h"

typedef struct {
	struct timeval* timers;
	int stops;
}* stopwatch;

stopwatch stopwatch_create();
void	watch_add_stop(stopwatch);
double	elapsed(stopwatch,int);
void	destroy_watch(stopwatch);

void* rsn_malloc(rsn_config,size_t,int);
void* rsn_malloc_array(rsn_config,size_t,int,int);
void* rsn_realloc(rsn_config,void*,size_t,int);
void* rsn_realloc_array(rsn_config,void**,size_t,int,int);
void  rsn_free(int,void**);
void  rsn_free_array(int,int,void***);

void print_spectrum(int,int,int,int,rsn_spectrum,const char*);
void print_image(int L, int M, int N, rsn_image img, const char* file);

#endif