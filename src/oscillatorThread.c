#include <mqueue.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <time.h>
#include <string.h>
#include <pthread.h>

#include "oscillatorThread.h"
#include "shared.h"

TASK oscillatorThread(void* arg)
{
    buffer_t *buffer = (buffer_t*)arg;

    while (!end_tasks)
    {
        pthread_mutex_lock(&buffer->mutex);
        if (buffer->len == 0) { // empty
            // wait for new items to be appended to the buffer
            pthread_cond_wait(&buffer->can_consume, &buffer->mutex);
        }
        --buffer->len;
        printf("Consumed: %d length: %ld\n", buffer->buf[buffer->len], buffer->len);

        // signal the fact that new items may be produced
        pthread_cond_signal(&buffer->can_produce);
        pthread_mutex_unlock(&buffer->mutex);
    }    
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