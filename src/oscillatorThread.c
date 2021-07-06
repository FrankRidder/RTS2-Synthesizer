#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <time.h>
#include <string.h>
#include <pthread.h>

#include "oscillatorThread.h"
#include "shared.h"

#include <time.h>

TASK oscillatorThread(void* arg)
{
    arguments_t *buffer = (arguments_t*)arg;

    while (!end_tasks)
    {
        /*
         * ================ Consume ====================
         */
        static int kickstart[NUM_OSCS];
        if (kickstart[buffer->thread_id] != 0)
        {
            pthread_mutex_lock(&buffer->input->mutex);
            while (buffer->input->len == 0 && !end_tasks) { // empty
                // wait for new items to be appended to the buffer
                pthread_cond_wait(&buffer->input->can_consume, &buffer->input->mutex);
            }
            buffer->input->len = 0;
            // signal the fact that new items may be produced
            pthread_cond_signal(&buffer->input->can_produce);
            pthread_mutex_unlock(&buffer->input->mutex);            
        }
        kickstart[buffer->thread_id]++;

        pthread_mutex_lock(&buffer->output->mutex);
        if (buffer->output->len == 1) { // full
            // wait until some elements are consumed
            int status = pthread_cond_wait(&buffer->output->can_produce, &buffer->output->mutex);
            //printf("Status osc: %d\n", status);
        }

        if (buffer->osc->pitch == 0) for (int i = 0; i < SAMPLES_PER_BUFFER; i++) buffer->output->buf[i] = 0;
        else if (buffer->osc->waveform == SQUARE)
            generateSquare(buffer->osc->pitch, buffer->output->buf, SAMPLES_PER_BUFFER);
        else if (buffer->osc->waveform == SIN) generateSin(buffer->osc->pitch, buffer->output->buf, SAMPLES_PER_BUFFER);
        else if (buffer->osc->waveform == SAW) generateSaw(buffer->osc->pitch, buffer->output->buf, SAMPLES_PER_BUFFER);

        buffer->output->len = 1;

        // signal the fact that new items may be consumed
        pthread_cond_signal(&buffer->output->can_consume);
        pthread_mutex_unlock(&buffer->output->mutex);

                
        //printf("osc thread ran %d\n", buffer->osc->pitch);
        //printf("Osc thread %d started test\n", buffer->thread_id);

        // pthread_mutex_lock(&buffer->input->mutex);
        // while (buffer->input->len == 0 && !end_tasks) { // empty
        //     // wait for new items to be appended to the buffer
        //     pthread_cond_wait(&buffer->input->can_consume, &buffer->input->mutex);
        // }
        // --buffer->input->len;
        // int tid = pthread_self();
        // printf("Consumed [%d]: %d length: %ld\n", tid, buffer->input->buf[buffer->input->len], buffer->input->len);

        // // signal the fact that new items may be produced
        // pthread_cond_signal(&buffer->input->can_produce);
        // pthread_mutex_unlock(&buffer->input->mutex);
    }
    return 0;
}

/*
 *  Generate sin wave from -32760 to 32760
 */
void generateSin(unsigned int freq, short *samples, int frame_count) {
    static float seconds_offset = 0.0f;
    float pitch = (float) freq;
    float radians_per_second = pitch * 2.0f * M_PI;
    float seconds_per_frame = 1.0f / (float) SAMPLE_RATE;
    for (int frame = 0; frame < frame_count; frame += 1) {
        samples[frame] = (short) (32760 *
                                  sin((seconds_offset + (float) frame * seconds_per_frame) * radians_per_second));
    }
    seconds_offset = fmodf(seconds_offset + seconds_per_frame * (float) frame_count, 1.0f);
}

/*
 *  Generate saw wave from -32760 to 32760
 */
void generateSaw(unsigned int freq, short *samples, int buf_size) {
    for (int i = 0; i < buf_size; ++i) {
        samples[i] = (short) (327600 * (2 * ((float) freq / (float) SAMPLE_RATE * (float) i +
                                             floor((float) freq / (float) SAMPLE_RATE * (float) i + 0.5))));
    }

}

/*
 *  Generate square wave from -32760 to 32760
 */
void generateSquare(unsigned int freq, short *samples, int buf_size) {
    int N = SAMPLE_RATE / freq / 2;
    int sign = -1;
    for (int i = 0; i < buf_size; ++i) {
        if (i % N == 0) sign = -sign;
        samples[i] = (short) (32760 * sign);
    }
}