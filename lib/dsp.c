/*
 * Resine - Fourier-based image resampling library.
 * Copyright 2010-2012 command-Q.org. All rights reserved.
 * This library is distributed under the terms of the GNU Lesser General Public License, Version 2.
 * 
 * dsp.c - Native DSP utilities.
 */

#include "dsp.h"

#include "fftwapi.h"

#include <stdlib.h>

/* Canonical implementation of the i/DCT with (very) minor optimizations */
rsn_frequency CC(int a, int b) { return a ? (b ? 1 : M_SQRT1_2) : (b ? M_SQRT1_2 : 0.5); }

/* Direct nonnormalized DCT, pretty much just useful for testing */
void rsn_dct_direct(int L, int M, int N, rsn_image f, rsn_spectrum F) {
	int z, v, u, j, i;
	for(z = 0; z < L; z++)
		for(v = 0; v < M; v++)
			for(u = 0; u < N; u++)
				for(j = 0; j < M; j++)
					for(i = 0; i < N; i++)
						F[z*N*M+v*N+u] += f[j][i*L+z] * rsn_cos((j+0.5)*v*M_PI/M) * rsn_cos((i+0.5)*u*M_PI/N);
}

void rsn_dct(int L, int M, int N, rsn_image f, rsn_spectrum F) {
	rsn_frequency NORM  = 2/rsn_sqrt(N*M);
	rsn_frequency HNORM = 1/rsn_sqrt(N*M);
	rsn_frequency NORM2 = 2/rsn_sqrt(2*N*M);
	rsn_frequency PI_M  = M_PI / M;
	rsn_frequency PI_N	= M_PI / N;
	rsn_frequency HPI_M = M_PI / (2*M);
	rsn_frequency HPI_N = M_PI / (2*N);
	rsn_frequency wisdom[M][N];
	rsn_frequency PI_Mv, PI_Nu, HPI_Mv, HPI_Nu;
	int z, v, u, j, i;
	for(v = 0, PI_Mv = 0, HPI_Mv = 0; v < M; v++, PI_Mv += PI_M, HPI_Mv += HPI_M) 
		for(u = 0, PI_Nu = 0, HPI_Nu = 0; u < N; u++, PI_Nu += PI_N, HPI_Nu += HPI_N) {
			for(j = 0; j < M; j++)
				for(i = 0; i < N; i++) {
					if(!(u+v))  wisdom[j][i] = HNORM;
					else if(!u) wisdom[j][i] = NORM2 * rsn_cos(j*PI_Mv+HPI_Mv);
					else if(!v) wisdom[j][i] = NORM2 * rsn_cos(i*PI_Nu+HPI_Nu);
					else        wisdom[j][i] = NORM  * rsn_cos(j*PI_Mv+HPI_Mv) * rsn_cos(i*PI_Nu+HPI_Nu);
					F[v*N+u] += wisdom[j][i] * f[j][i*L];
				}
			for(z = 1; z < L; z++)
				for(j = 0; j < M; j++)
					for(i = 0; i < N; i++)
						F[z*N*M+v*N+u] += wisdom[j][i] * f[j][i*L+z];
		}
}

void rsn_idct(int L, int M, int N, rsn_spectrum F, rsn_image f) {
	rsn_frequency NORM = 2/rsn_sqrt(N*M);
	rsn_frequency HNORM = 1/rsn_sqrt(N*M);
	rsn_frequency NORM2 = 2/rsn_sqrt(2*N*M);
	rsn_frequency PI_M = M_PI / M;
	rsn_frequency PI_N = M_PI / N;
	rsn_frequency HPI_M = M_PI / (2*M);
	rsn_frequency HPI_N = M_PI / (2*N);
	rsn_frequency wisdom[M][N];
	rsn_frequency s;
	rsn_frequency PI_Mv, PI_Nu, HPI_Mv, HPI_Nu;
	int z, v, u, j, i;
	for(j = 0; j < M; j++)
		for(i = 0; i < N; i++) {
			for(v = 0, s = 0, PI_Mv = 0, HPI_Mv = 0; v < M; v++, PI_Mv += PI_M, HPI_Mv += HPI_M)
				for(u = 0, PI_Nu = 0, HPI_Nu = 0; u < N; u++, PI_Nu += PI_N, HPI_Nu += HPI_N) {
					if(!(u+v))  wisdom[0][0] = HNORM;
					else if(!u) wisdom[v][0] = NORM2 * rsn_cos(j*PI_Mv+HPI_Mv);
					else if(!v) wisdom[0][u] = NORM2 * rsn_cos(i*PI_Nu+HPI_Nu);	
					else        wisdom[v][u] = NORM  * rsn_cos(j*PI_Mv+HPI_Mv) * rsn_cos(i*PI_Nu+HPI_Nu);
					s += wisdom[v][u] * F[v*N+u];
				}
			f[j][i*L] = s > 255 ? 255 : s < 0 ? 0 : round(s);

			for(z = 1; z < L; z++) {
				for(v = 0, s = 0; v < M; v++)
					for(u = 0; u < N; u++)
						s += wisdom[v][u] * F[z*N*M+v*N+u];
				f[j][i*L+z] = s > 255 ? 255 : s < 0 ? 0 : round(s);
			}
		}
}

void rsn_dct_rowcol(int L, int M, int N, rsn_image f, rsn_spectrum F) {
	rsn_spectrum tmp = malloc(sizeof(rsn_frequency)*M*N);
	int t_len = N > M ? N : M;
	rsn_spectrum twiddles = malloc(sizeof(rsn_frequency)*t_len*t_len);
	for(int t1 = 0; t1 < t_len; t1++)
		for(int t2 = 0; t2 < t_len; t2++)
			twiddles[t1*t_len+t2] = rsn_cos(M_PI/t_len * (t2+0.5) * t1) * 2;

	for(int z = 0; z < L; z++) {
		for(int row = 0; row < M; row++)
			for(int u = 0; u < N; u++) {
				tmp[row*N+u] = 0.0;
				for(int i = 0; i < N; i++)
					tmp[row*N+u] += f[row][i*L+z] * twiddles[u*t_len+i];
			}
		for(int col = 0; col < N; col++)
			for(int v = 0; v < M; v++)
				for(int j = 0; j < M; j++)
					F[z*M*N+v*N+col] += tmp[j*N+col] * twiddles[v*t_len+j];
	}
	free(tmp);
	free(twiddles);
}

void rsn_idct_rowcol(int L, int M, int N, rsn_spectrum F, rsn_image f) {
	rsn_spectrum tmp = malloc(sizeof(rsn_frequency)*M*N);
	rsn_frequency s;
	int t_len = N > M ? N : M;
	rsn_spectrum twiddles = malloc(sizeof(rsn_frequency)*t_len*(t_len-1));
	for(int t1 = 0; t1 < t_len; t1++)
		for(int t2 = 1; t2 < t_len; t2++)
			twiddles[t1*(t_len-1)+t2-1] = rsn_cos(M_PI/t_len * (t1+0.5) * t2);

	for(int z = 0; z < L; z++) {
		for(int row = 0; row < M; row++)
			for(int i = 0; i < N; i++) {
				tmp[row*N+i] = F[z*M*N+row*N+0]/2;
				for(int u = 1; u < N; u++)
					tmp[row*N+i] += F[z*M*N+row*N+u] * twiddles[i*(t_len-1)+u-1];
			}
		for(int col = 0; col < N; col++)
			for(int j = 0; j < M; j++) {
				s = tmp[0*N+col]/2;
				for(int v = 1; v < M; v++)
					s += tmp[v*N+col] * twiddles[j*(t_len-1)+v-1];
				s /= N*M;
				f[j][col*L+z] = s > 255 ? 255 : s < 0 ? 0 : round(s);
			}
	}
	free(tmp);
	free(twiddles);
}

rsn_image spectrogram(int L, int M, int N, rsn_spectrum F) {
	int z,y,x,i;
	rsn_frequency c,max = rsn_fabs(F[0]);
	rsn_image f = malloc(sizeof(rsn_line)*M);
	for(y = 0; y < M; y++) f[y] = malloc(sizeof(rsn_pel)*N*L);
	rsn_frequency gain = 0.5/rsn_sqrt(N*M);

	for(i = 1; i < L*M*N; i++)
		if(rsn_fabs(F[i]) > max) max = rsn_fabs(F[i]);
	c = 255/rsn_log(max*gain+1);

	for(z = 0; z < L; z++)
		for(y = 0; y < M; y++)
			for(x = 0; x < N; x++)
				f[y][x*L+z] = round(c * rsn_log(rsn_fabs(F[z*M*N+y*N+x])*gain + 1));

	return f;
}

rsn_image spectrogram_anchored(int L, int M, int N, rsn_spectrum F) {
	int z,y,x;
	rsn_image f = malloc(sizeof(rsn_line)*M);
	for(y = 0; y < M; y++) f[y] = malloc(sizeof(rsn_pel)*N*L);

	rsn_frequency c = 127.5/rsn_log(M*N*510/rsn_sqrt(N*M)+1);

	for(z = 0; z < L; z++)
		for(y = 0; y < M; y++)
			for(x = 0; x < N; x++)
				f[y][x*L+z] = round(c * rsn_copysign(rsn_log(rsn_fabs(F[z*M*N+y*N+x]) + 1),F[z*M*N+y*N+x]) + 127.5);

	return f;
}

rsn_spectrum spectrogram_decompress(int L, int M, int N, rsn_image f) {
	int z,y,x;
	rsn_spectrum F = malloc(sizeof(rsn_frequency)*L*M*N);

	rsn_frequency c = 127.5/rsn_log(M*N*510/rsn_sqrt(N*M)+1);

	for(z = 0; z < L; z++)
		for(y = 0; y < M; y++)
			for(x = 0; x < N; x++)
				F[z*M*N+y*N+x] = rsn_copysign((rsn_pow(M_E,rsn_fabs((f[y][x*L+z]-127.5)/c)) - 1),f[y][x*L+z]-127.5);

	return F;
}

/* The image to be composited is required to be the same size as the input, with alpha */
void composite_spectrum(int L, int M, int N, rsn_spectrum F, rsn_image m) {
	int z,y,x,i,a=L-1,A=L;
	if(L % 2) {
		a = L;
		A = L+1;
	}

	rsn_frequency c,max = rsn_fabs(F[0]);
	for(i = 1; i < L*M*N; i++)
		if(rsn_fabs(F[i]) > max) max = rsn_fabs(F[i]);
	c = 255/rsn_log(max+1);
	
	for(z = 0; z < L; z++)
		for(y = 0; y < M; y++)
			for(x = 0; x < N; x++)
				F[z*M*N+y*N+x] = (rsn_pow(M_E,m[y][x*A+z]/c)-1) * 0.5/rsn_sqrt(M*N) * m[y][x*A+a]/255.0 + F[z*M*N+y*N+x] * (255-m[y][x*A+a])/255.0;
}
