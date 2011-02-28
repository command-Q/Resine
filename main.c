/*
 * Resine - Fourier-based image resampling library.
 * Copyright 2010-2011 command-Q.org. All rights reserved.
 * This program is distributed under the terms of the Do What The Fuck You Want To Public License, Version 2. 
 *
 * main.c - Example command-line application using libresine.
 *	Reads in an image file of type PNG or JPEG, scales it, and writes it back to the format of your choice.
 */

#include "image.h"

int main(int argc, char **argv) {
	if(argc < 2) {
		printf("Resine - Fourier-based image resampling library.\n"
			   "\t©2010-2011 command-Q.org\n"
			   "\tVersion %s\n"
			   "\t%s precision configuration (%ld byte float).\n"
#if HAS_FFTW
			   "\tCompiled with FFTW support.\n"
#endif
			   "\n"
			   "Usage: resine [options] infile outfile\n"
			   "\n"
			   "infile: PNG or JPEG, 8-32 bit color.\n"
			   "outfile: PNG or JPEG, same bitdepth as input. Outfile may be ommitted, but nothing will be written to disk.\n"
			   "\n"
			   "options:\n"
			   "\n"
			   "Scaling: Any of the following qualifiers.\n"
			   "\n"
			   "\t-s <float>\t Overall scale factor\n"
			   "\n"
			   "\t-x <float>\t Horizontal scaling factor\n"
			   "\t-y <float>\t Vertical scaling factor\n"
			   "\n"
			   "\t-w <int>\t New width\n"
			   "\t-h <int>\t New height\n"
			   "\n"
			   "Resine options:\n"
			   "\n"
			   "\t-T <int>\t Transform type [%d]\n"
			   "\t        \t\t- 0: Native (SLOW)\n"
#if HAS_FFTW
			   "\t        \t\t- 1: FFTW\n"
#endif
			   "\t-S <int>\t Scaling method [0]\n"
			   "\t        \t\t- 0: Standard\n"
			   "\t        \t\t- 1: Smooth (upscaling only)\n"
			   "\t-G <int>\t Greed - Memory consumption/speed trade-offs [2]\n"
			   "\t        \t\t- 0: Lean - Allocate and free memory on the fly\n"
			   "\t        \t\t- 1: Prealloc - Preallocate image data\n"
			   "\t        \t\t- 2: Retain - Don't free any memory until rsn_destroy is called\n"
			   "\t        \t\t- 3: Prealloc and retain\n"
#if THREADED
			   "\t-t <int>\t Number of threads to use.\n"		   
#endif
			   "\t-g <filename>\t Graph: Draw spectrogram to file <filename>.png (NOTE: Bumps Greed level to Retain if necessary).\n"		   
			   "\t-p <filename>\t Print: Dump transform data into file <filename>.\n"		   
			   "\t-v      \t Verbose: Print duration of transforms.\n"
			   "\n"
			   "Command-line options:\n"
			   "\n"
			   "\t-q <int>\t JPEG compression quality (0-100) [90]\n"
			   "\n",
			   RSN_VERSION,RSN_PRECISION,sizeof(rsn_frequency),RSN_TRANSFORM_DEFAULT);
		return 0;
	}

	int c, in_type=RSN_IMGTYPE_NONE, out_type=RSN_IMGTYPE_NONE, jpeg_q=90;
	float sx = 1.0,sy = 1.0;
	char *print = NULL, *graph = NULL;
	
	rsn_info info;
	info.config = rsn_defaults();
		
	while((c = getopt(argc,argv,"s:x:y:w:h:T:S:G:p:g:vq:")) != -1)
		switch (c) {
			case 's' : sx = sy = strtof(optarg,NULL);					break;
			case 'x' : sx = strtof(optarg,NULL);						break;
			case 'y' : sy = strtof(optarg,NULL);						break;
			case 'w' : info.width_s = strtol(optarg,NULL,10);			break;
			case 'h' : info.height_s = strtol(optarg,NULL,10);			break;
			case 'T' : info.config.transform = strtol(optarg,NULL,10);	break;
			case 'S' : info.config.scaling = strtol(optarg,NULL,10);	break;
			case 'G' : info.config.greed = strtol(optarg,NULL,10);		break;
			case 't' : info.config.threads = strtol(optarg,NULL,10);	break;
			case 'p' : print = optarg;									break;
			case 'g' : graph = optarg;									break;
			case 'v' : info.config.verbosity = 1;						break;
			case 'q' : jpeg_q = strtol(optarg,NULL,10);					break;
			default  :													break;
		}
	if(graph && !(info.config.greed & RSN_GREED_RETAIN)) info.config.greed = RSN_GREED_RETAIN;
	char* infile = argv[optind++];
	char* outfile = NULL;
	if(optind < argc) outfile = argv[optind];
	if(!strncasecmp(strrchr(infile,'.'),".jp",3)) in_type = RSN_IMGTYPE_JPEG;
	else if(!strncasecmp(strrchr(infile,'.'),".png",4)) in_type = RSN_IMGTYPE_PNG;
	if(outfile) {
		if(!strncasecmp(strrchr(outfile,'.'),".jp",3)) out_type = RSN_IMGTYPE_JPEG;
		else if(!strncasecmp(strrchr(outfile,'.'),".png",4)) out_type = RSN_IMGTYPE_PNG;		
	}

	rsn_image img = 0; // shut up clang
	switch(in_type) {
		case RSN_IMGTYPE_PNG  : img = read_png_file(&info,infile);	break;
		case RSN_IMGTYPE_JPEG : img = read_jpeg_file(&info,infile);	break;
		case RSN_IMGTYPE_NONE :
		default				  :	fprintf(stderr,"Image is not a supported type (PNG, JPEG).\n");	return 1; // Unsupported type
	}
	if(!info.height_s) info.height_s = round(info.height*sx);
	if(!info.width_s) info.width_s = round(info.width*sy);

	/* Flatten alpha channel when necessary */
	int z,y,x;
	bool flatten = !(info.channels % 2);
	if(out_type == RSN_IMGTYPE_JPEG && flatten) {
		info.channels--;
		for(y = 0; y < info.height; y++) {
			for(x = 0; x < info.width; x++)
				for(z = 0; z < info.channels; z++)
					img[y][x*info.channels+z] = img[y][x*(info.channels+1)+z] * img[y][x*(info.channels+1)+info.channels] / 255.0;			
			realloc(img[y],sizeof(rsn_pel)*info.width*info.channels);
		}
		flatten = false;
	}
	for(y = 0; flatten && y < info.height; y++)
		for(x = info.channels-1; flatten && x < info.width*info.channels; x+= info.channels)
			if(img[y][x] != 255) flatten = false;
	if(flatten) {
		info.channels--;
		for(y = 0; y < info.height; y++) {
			for(x = 0; x < info.width; x++)
				for(z = 0; z < info.channels; z++)
					img[y][x*info.channels+z] = img[y][x*(info.channels+1)+z];
			realloc(img[y],sizeof(rsn_pel)*info.width*info.channels);
		}
	}
	
	rsn_datap data = rsn_init(info,img);
	rsn_image oimg = resine_data(info,data);
	if(print) print_spectrum(info.channels,info.height_s,info.width_s,2,data->freq_image_s,print);
	if(graph) {
		rsn_image specta = spectrogram(info.channels,info.height_s,info.width_s,data->freq_image_s);
		write_png_file(info,graph,specta);
		rsn_free_array(RSN_TRANSFORM_NONE,info.height_s,(void***)&specta);
	}
	
	switch(out_type) {
		case  RSN_IMGTYPE_PNG : write_png_file(info,outfile,oimg);			break;
		case RSN_IMGTYPE_JPEG : write_jpeg_file(info,outfile,oimg,jpeg_q);	break;
		default				  :												break;
	}
	
	rsn_destroy(info,data);
	rsn_free_array(RSN_TRANSFORM_NONE,info.height,(void***)&img);

	return 0;
}