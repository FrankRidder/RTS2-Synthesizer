#ifndef WAVE_GEN
#define WAVE_GEN

#include <math.h>
#include "shared.h"

TASK oscillatorThread(void *arg);

/*
 *  Generate sin wave from -32760 to 32760
 */
void generateSin(int thread_id, unsigned int freq, short *samples, int buf_size);

/*
 *  Generate saw wave from -32760 to 32760
 */
void generateSaw(int thread_id, unsigned int freq, short *samples, int buf_size);

/*
 *  Generate square wave from -32760 to 32760
 */
void generateSquare(int thread_id, unsigned int freq, short *samples, int buf_size);

/*
 *  Generate triangular wave from -32760 to 32760
 */
void generateTriangle(int thread_id, unsigned int freq, short *samples, int buf_size);



#endif //WAVE_GEN
