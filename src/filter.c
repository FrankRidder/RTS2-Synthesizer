/*
 * filter.c
 *
 * Copyright (c) 2018 Disi A
 * 
 * Author: Disi A
 * Email: adis@live.cn
 *  https://www.mathworks.com/matlabcentral/profile/authors/3734620-disi-a
 */
#include <stdlib.h>
#include <math.h>
#include "filter.h"

#if DOUBLE_PRECISION
#define COS cos
#define SIN sin
#define TAN tan
#define COSH cosh
#define SINH sinh
#define SQRT sqrt
#define LOG log
#else
#define COS cosf
#define SIN sinf
#define TAN tanf
#define COSH coshf
#define SINH sinhf
#define SQRT sqrtf
#define LOG logf
#endif

BWLowPass* create_bw_low_pass_filter(int order, FTR_PRECISION s, FTR_PRECISION f) {
    BWLowPass* filter = (BWLowPass *) malloc(sizeof(BWLowPass));
    filter -> n = order/2;
    filter -> A = (FTR_PRECISION *)malloc(filter -> n*sizeof(FTR_PRECISION));
    filter -> d1 = (FTR_PRECISION *)malloc(filter -> n*sizeof(FTR_PRECISION));
    filter -> d2 = (FTR_PRECISION *)malloc(filter -> n*sizeof(FTR_PRECISION));
    filter -> w0 = (FTR_PRECISION *)calloc(filter -> n, sizeof(FTR_PRECISION));
    filter -> w1 = (FTR_PRECISION *)calloc(filter -> n, sizeof(FTR_PRECISION));
    filter -> w2 = (FTR_PRECISION *)calloc(filter -> n, sizeof(FTR_PRECISION));

    FTR_PRECISION a = TAN(M_PI * f/s);
    FTR_PRECISION a2 = a * a;
    FTR_PRECISION r;
    
    int i;
    
    for(i=0; i < filter -> n; ++i){
        r = SIN(M_PI*(2.0*i+1.0)/(4.0*filter -> n));
        s = a2 + 2.0f*a*r + 1.0f;
        filter -> A[i] = a2/s;
        filter -> d1[i] = 2.0f*(1-a2)/s;
        filter -> d2[i] = -(a2 - 2.0f*a*r + 1.0f)/s;
    }
    return filter;
}

void change_bw_low_pass(BWLowPass* filter, FTR_PRECISION s, FTR_PRECISION f)
{
    FTR_PRECISION a = TAN(M_PI * f/s);
    FTR_PRECISION a2 = a * a;
    FTR_PRECISION r;
    
    int i;
    
    for(i=0; i < filter -> n; ++i){
        r = SIN(M_PI*(2.0*i+1.0)/(4.0*filter -> n));
        s = a2 + 2.0f*a*r + 1.0f;
        filter -> A[i] = a2/s;
        filter -> d1[i] = 2.0f*(1-a2)/s;
        filter -> d2[i] = -(a2 - 2.0f*a*r + 1.0f)/s;
    }
}

void free_bw_low_pass(BWLowPass* filter){
    free(filter -> A);
    free(filter -> d1);
    free(filter -> d2);
    free(filter -> w0);
    free(filter -> w1);
    free(filter -> w2);
    free(filter);
}

FTR_PRECISION bw_low_pass(BWLowPass* filter, FTR_PRECISION x){
    int i;
    for(i=0; i<filter->n; ++i){
        filter->w0[i] = filter->d1[i]*filter->w1[i] + filter->d2[i]*filter->w2[i] + x;
        x = filter->A[i]*(filter->w0[i] + 2.0f*filter->w1[i] + filter->w2[i]);
        filter->w2[i] = filter->w1[i];
        filter->w1[i] = filter->w0[i];
    }
    return x;
}
