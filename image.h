/*
 * Resine - Fourier-based image resampling library.
 * This example code is distributed under no claim of copyright. 
 *
 * image.h - Image I/O functions for the example app.
 */

#ifndef IMAGE_H
#define IMAGE_H

#include <resine.h>

#define PNG_DEBUG	3

#define RSN_IMGTYPE_NONE		-1
#define RSN_IMGTYPE_PNG			 0
#define RSN_IMGTYPE_JPEG		 1

/* Utility functions */
void abort_(const char*,...);

/* Image I/O */
rsn_image read_png_file(rsn_infop,const char*);
rsn_image read_jpeg_file(rsn_infop,const char*);
void write_png_file(rsn_info,const char*,rsn_image);
void write_jpeg_file(rsn_info,const char*,rsn_image,int);

#endif