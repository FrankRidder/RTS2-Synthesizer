#include "oscillatorThread.h"

#include <pthread.h>
#include "shared.h"

TASK oscillatorThread(void *arg) {
    arguments_t *buffer = (arguments_t *) arg;

    while (!end_tasks) {
        /*
         * ================ Consume ====================
         */
        static int kickstart[NUM_OSCS];
        if (kickstart[buffer->thread_id] != 0) {
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
            pthread_cond_wait(&buffer->output->can_produce, &buffer->output->mutex);
        }

        if (buffer->osc->pitch == 0)
            for (int i = 0; i < SAMPLES_PER_BUFFER; i++) buffer->output->buf[i] = 0;
        else if (buffer->osc->waveform == SIN) 
            generateSin(buffer->thread_id, buffer->osc->pitch, buffer->output->buf, SAMPLES_PER_BUFFER);
        else if (buffer->osc->waveform == SQUARE)
            generateSquare(buffer->thread_id, buffer->osc->pitch, buffer->output->buf, SAMPLES_PER_BUFFER);
        else if (buffer->osc->waveform == SAW) 
            generateSaw(buffer->thread_id, buffer->osc->pitch, buffer->output->buf, SAMPLES_PER_BUFFER);
        else if (buffer->osc->waveform == TRIANGLE)
            generateTriangle(buffer->thread_id, buffer->osc->pitch, buffer->output->buf, SAMPLES_PER_BUFFER);

        buffer->output->len = 1;

        // signal the fact that new items may be consumed
        pthread_cond_signal(&buffer->output->can_consume);
        pthread_mutex_unlock(&buffer->output->mutex);

    }
    return 0;
}

/*
 *  Generate sin wave from -32760 to 32760
 */
void generateSin(int tid, unsigned int freq, short *samples, int buf_size) {
    float pitch  = (float) freq;
    float radians_per_second = (float) (pitch * 2.0f * M_PI);
    float seconds_per_frame = 1.0f / (float) SAMPLE_RATE;

    static float t[NUM_OSCS];
    for (int frame = 0; frame < buf_size; frame += 1) {
        samples[frame] = (short) (32760 * sin(radians_per_second * t[tid]));
        t[tid] = fmodf(t[tid] + seconds_per_frame, 1.0f);
    }
}
/*
 *  Generate saw wave from -32760 to 32760
 */
void generateSaw(int tid, unsigned int freq, short *samples, int buf_size) {
    float seconds_per_frame = 1.0f / (float) SAMPLE_RATE;
    float pitch  = (float) freq;
    float period = 1.0f / pitch;

    static float t[NUM_OSCS];
    for (int i = 0; i < buf_size; i++)
    {
        samples[i] = (short) (32760 * 2 * ((t[tid]/period) - floorf(0.5f + t[tid]/period)) );
        t[tid] += seconds_per_frame;
    }
    t[tid] = fmodf(t[tid], 1.0f);
}

/*
 *  Generate square wave from -32760 to 32760
 */
void generateSquare(int tid, unsigned int freq, short *samples, int buf_size) {
    float seconds_per_frame = 1.0f / (float) SAMPLE_RATE;
    float pitch  = (float) freq;
    
    static float t[NUM_OSCS];
    for (int i = 0; i < buf_size; i++)
    {
        samples[i] = (short) ( 32760 * 2 * (2*floorf(pitch*t[tid]) - floorf(2*pitch*t[tid])) + 32760 );
        t[tid] += seconds_per_frame;
    }
    t[tid] = fmodf(t[tid], 1.0f);
}

/*
 *  Generate saw wave from -32760 to 32760
 */
void generateTriangle(int tid, unsigned int freq, short *samples, int buf_size) {
    float seconds_per_frame = 1.0f / (float) SAMPLE_RATE;
    float pitch  = (float) freq;
    float period = 1.0f / pitch;

    static float t[NUM_OSCS];
    for (int i = 0; i < buf_size; i++)
    {
        samples[i] = (short) (2 * fabsf(32760 * 2 * ((t[tid]/period) - floorf(0.5f + t[tid]/period))) - 32760);
        t[tid] += seconds_per_frame;
    }
    t[tid] = fmodf(t[tid], 1.0f);
}
