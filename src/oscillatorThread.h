#ifndef WAVE_GEN
#define WAVE_GEN

#include <math.h>
#include "shared.h"

/*
 *  Generate sin wave from -32760 to 32760
 */
void generateSin(int freq, short *samples, int buf_size);

/*
 *  Generate saw wave from -32760 to 32760
 */
void generateSaw(int freq, short *samples, int buf_size);

/*
 *  Generate square wave from -32760 to 32760
 */
void generateSquare(int freq, short *samples, int buf_size);



#endif