/*
 * filter.h
 *
 * Copyright (c) 2018 Disi A
 * 
 * Author: Disi A
 * Email: adis@live.cn
 *  https://www.mathworks.com/matlabcentral/profile/authors/3734620-disi-a
 */
#ifndef filter_h
#define filter_h

#if __cplusplus
extern "C"{
#endif

#define DOUBLE_PRECISION 0



#if DOUBLE_PRECISION
#define FTR_PRECISION double
#if defined(_WIN32) || defined(__ZEPHYR__)
#define M_PI 3.141592653589793238462643383279502884197163993751
#endif
#else
#define FTR_PRECISION float
#if defined(_WIN32) || defined(__ZEPHYR__)
#define M_PI 3.1415927
#endif
#endif

typedef struct {
    int n;
	FTR_PRECISION *A;
    FTR_PRECISION *d1;
    FTR_PRECISION *d2;
    FTR_PRECISION *w0;
    FTR_PRECISION *w1;
    FTR_PRECISION *w2;
} BWLowPass;
// BWHighPass uses exactly the same struct
typedef BWLowPass BWHighPass;

typedef struct {
    int m;
    FTR_PRECISION ep;
	FTR_PRECISION *A;
    FTR_PRECISION *d1;
    FTR_PRECISION *d2;
    FTR_PRECISION *w0;
    FTR_PRECISION *w1;
    FTR_PRECISION *w2;
} CHELowPass;
typedef CHELowPass CHEHighPass;

BWLowPass* create_bw_low_pass_filter(int order, FTR_PRECISION sampling_frequency, FTR_PRECISION half_power_frequency);

void change_bw_low_pass(BWLowPass* filter, FTR_PRECISION s, FTR_PRECISION f);

void free_bw_low_pass(BWLowPass* filter);

FTR_PRECISION bw_low_pass(BWLowPass* filter, FTR_PRECISION input);

#if __cplusplus
}
#endif

#endif /* filter_h */
