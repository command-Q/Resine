/*
 * Resine - Fourier-based image resampling library.
 * Copyright 2010-2011 command-Q.org. All rights reserved.
 * This program is distributed under the terms of the Do What The Fuck You Want To Public License, Version 2. 
 *
 * image.c - Image I/O functions for the example app.
 */

#include "image.h"

#include <math.h>
#include <stdarg.h>
#include <stdlib.h>

#include <png.h>
#include <jpeglib.h>

void abort_(const char* s, ...) {
	va_list args;
	va_start(args, s);
	vfprintf(stderr, s, args);
	fprintf(stderr, "\n");
	va_end(args);
	abort();
}

rsn_image read_png_file(rsn_infop info, const char* filename) {
	FILE* f = fopen(filename,"rb");
	if(!f)	abort_("[read_png_file] File %s could not be opened for reading",filename);
	unsigned char header[8];
	fread(header,1,8,f);
	if(png_sig_cmp(header,0,8)) abort_("[read_png_file] File %s is not recognized as a PNG file",filename);

	png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if(!png_ptr) abort_("[read_png_file] png_create_read_struct failed");

	png_infop info_ptr = png_create_info_struct(png_ptr);
	if(!info_ptr) {
		abort_("[read_png_file] png_create_info_struct failed");
		abort(); // This is never touched, it's just to satisfy CSA
	}
	if(setjmp(png_jmpbuf(png_ptr))) abort_("[read_png_file] Error during init_io");

	png_init_io(png_ptr,f);
	png_set_sig_bytes(png_ptr,8);
	png_read_info(png_ptr,info_ptr);
	info->width = png_get_image_width(png_ptr,info_ptr);
	info->height = png_get_image_height(png_ptr,info_ptr);
    info->channels = png_get_channels(png_ptr, info_ptr);
	
	if(setjmp(png_jmpbuf(png_ptr))) abort_("[read_png_file] Error during read_image");

	rsn_image image = malloc(sizeof(rsn_line)*info->height);
	png_size_t rowbytes = png_get_rowbytes(png_ptr,info_ptr);
	for(int y = 0; y < info->height; y++)
		image[y] = malloc(rowbytes);

	png_read_image(png_ptr,image);
	
	png_infop end_ptr = png_create_info_struct(png_ptr);
	png_read_end(png_ptr, end_ptr);

	fclose(f);
	png_destroy_read_struct(&png_ptr,&info_ptr,&end_ptr);
	png_destroy_info_struct(png_ptr,&info_ptr);

	return image;
}

rsn_image read_jpeg_file(rsn_infop info,const char* filename) {
	FILE* f = fopen(filename,"rb");
	if(!f) abort_("Error opening jpeg file %s\n!",filename);
	
	struct jpeg_decompress_struct cinfo;
	struct jpeg_error_mgr jerr;
	cinfo.err = jpeg_std_error(&jerr);
	jpeg_create_decompress(&cinfo);
	jpeg_stdio_src(&cinfo,f);
	jpeg_read_header(&cinfo,TRUE);
	

	jpeg_start_decompress(&cinfo);

	info->width = cinfo.output_width;
	info->height = cinfo.output_height;
	info->channels = cinfo.num_components;

	rsn_image image = malloc(sizeof(rsn_line)*info->height);

	for(int y = 0; cinfo.output_scanline < info->height; y++) {
		image[y] = malloc(sizeof(rsn_pel)*info->width*info->channels);	
		jpeg_read_scanlines(&cinfo,&image[y],1);
	}

	jpeg_finish_decompress(&cinfo);
	jpeg_destroy_decompress(&cinfo);
	fclose(f);

	return image;
}

void write_png_file(rsn_info info,const char* filename, rsn_image image) {
	FILE *f = fopen(filename, "wb");
	if(!f) abort_("[write_png_file] File %s could not be opened for writing", filename);
		
	png_structp png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if(!png_ptr) abort_("[write_png_file] png_create_write_struct failed");

	png_infop info_ptr = png_create_info_struct(png_ptr);
	if(!info_ptr) abort_("[write_png_file] png_create_info_struct failed");

	if(setjmp(png_jmpbuf(png_ptr)))	abort_("[write_png_file] Error during init_io");

	png_init_io(png_ptr,f);

	if(setjmp(png_jmpbuf(png_ptr)))	abort_("[write_png_file] Error during writing header");
	int ocsp;
	if(!(info.channels % 2)) ocsp = (info.channels - 2) | PNG_COLOR_MASK_ALPHA;
	else ocsp = info.channels - 1;
	png_set_IHDR(png_ptr,info_ptr,info.width_s,info.height_s,8,ocsp,PNG_INTERLACE_NONE,PNG_COMPRESSION_TYPE_DEFAULT,PNG_FILTER_TYPE_BASE);
	
	png_write_info(png_ptr,info_ptr);

	if(setjmp(png_jmpbuf(png_ptr)))	abort_("[write_png_file] Error during writing bytes");

	png_write_image(png_ptr,image);

	if(setjmp(png_jmpbuf(png_ptr)))	abort_("[write_png_file] Error during end of write");
	png_write_end(png_ptr,NULL);
	fclose(f);
	png_destroy_write_struct(&png_ptr,&info_ptr);
	png_destroy_info_struct(png_ptr,&info_ptr);
}

void write_jpeg_file(rsn_info info, const char* filename, rsn_image image, int quality) {	
	FILE* f = fopen(filename,"wb");	
	if(!f) abort_("Error opening output jpeg file %s\n!",filename);

	struct jpeg_compress_struct cinfo;
	struct jpeg_error_mgr jerr;

	cinfo.err = jpeg_std_error(&jerr);
	jpeg_create_compress(&cinfo);
	jpeg_stdio_dest(&cinfo,f);

	cinfo.image_width = info.width_s;	
	cinfo.image_height = info.height_s;
	cinfo.input_components = info.channels;
	cinfo.in_color_space = (int)ceil(info.channels/2.f);

	jpeg_set_defaults(&cinfo);
	jpeg_set_quality(&cinfo,quality,TRUE);

	jpeg_start_compress(&cinfo,TRUE);

	for(int y = 0; cinfo.next_scanline < info.height_s; y++)
		jpeg_write_scanlines(&cinfo,&image[y],1);		

	jpeg_finish_compress(&cinfo);
	jpeg_destroy_compress(&cinfo);
	fclose(f);
}
