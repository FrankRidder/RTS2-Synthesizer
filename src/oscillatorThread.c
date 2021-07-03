#include "oscillatorThread.h"
#include "shared.h"

TASK oscillatorThread()
{
    
}

/*
 *  Generate sin wave from -32760 to 32760
 */
void generateSin(int freq, short *samples, int buf_size) 
{
    for (int i = 0; i < buf_size; ++i) {
        samples[i] = 32760 * sin( (2.f * M_PI * freq) / (float)SAMPLE_RATE * i );
    }
}

/*
 *  Generate saw wave from -32760 to 32760
 */
void generateSaw(int freq, short *samples, int buf_size) 
{
    for (int i = 0; i < buf_size; ++i) {
        samples[i] = 32760 * ( 2 * (freq / (float)SAMPLE_RATE * i + floor(freq / (float)SAMPLE_RATE * i + 0.5) ) );
    }
}

/*
 *  Generate square wave from -32760 to 32760
 */
void generateSquare(int freq, short *samples, int buf_size) 
{
    int N = SAMPLE_RATE / freq / 2;
    int sign = -1;
    for (int i = 0; i < buf_size; ++i) {
        if (i % N == 0) sign = -sign;
        samples[i] = 32760 * sign;
    }
}