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
    arguments_t *buffer = (arguments_t*)arg;


    while (!end_tasks)
    {
        pthread_mutex_lock(&buffer->input->mutex);
        while (buffer->input->len == 0 && !end_tasks) { // empty
            // wait for new items to be appended to the buffer
            pthread_cond_wait(&buffer->input->can_consume, &buffer->input->mutex);
        }
        --buffer->input->len;
        printf("Consumed: %d length: %ld\n", buffer->input->buf[buffer->input->len], buffer->input->len);

        // signal the fact that new items may be produced
        pthread_cond_signal(&buffer->input->can_produce);
        pthread_mutex_unlock(&buffer->input->mutex);
    }    
}

/*
 *  Generate sin wave from -32760 to 32760
 */
void generateSin(int freq, short *samples, int frame_count) 
{
    static float seconds_offset = 0.0f;
    float pitch = (float)freq;
    float radians_per_second = pitch * 2.0f * M_PI;
    float seconds_per_frame = 1.0f / (float)SAMPLE_RATE;
    for (int frame = 0; frame < frame_count; frame += 1) {
        samples[frame] = 32760 * sin((seconds_offset + frame * seconds_per_frame) * radians_per_second);
    }
    seconds_offset = fmod(seconds_offset + seconds_per_frame * frame_count, 1.0);
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