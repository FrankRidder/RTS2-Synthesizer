#include "filterThread.h"
#include "filter.h"
#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>    // gives malloc
#include <string.h>

TASK filterThread(void* arg)
{
    arguments_t *buffer = (arguments_t*)arg;
        
    short * samples = malloc(sizeof(short) * SAMPLES_PER_BUFFER);
    BWLowPass* filter_bw = create_bw_low_pass_filter(4, SAMPLE_RATE, (float) filter_freq);

    while (!end_tasks) 
    {
        /*
         * ================ Consume ====================
         */
        pthread_mutex_lock(&buffer->input->mutex);
        while (buffer->input->len == 0 && !end_tasks) { // empty
            // wait for new items to be appended to the buffer
            pthread_cond_wait(&buffer->input->can_consume, &buffer->input->mutex);
        }
        buffer->input->len = 0;

        memcpy(samples, buffer->input->buf, SAMPLES_PER_BUFFER);
        // signal the fact that new items may be produced
        pthread_cond_signal(&buffer->input->can_produce);
        pthread_mutex_unlock(&buffer->input->mutex);

        /*
         * ================ Process ====================
         */
        // Recreating filter because there is no option to change frequency (yet)
        if (filter_activated) {
            change_bw_low_pass(filter_bw, SAMPLE_RATE, (float) filter_freq);
            for(int i = 1; i < SAMPLES_PER_BUFFER; i++) {
                samples[i] = (short) bw_low_pass(filter_bw, (float) samples[i]);
            }
        }

        /*
         * ================ Produce ====================
         */
        pthread_mutex_lock(&buffer->output->mutex);
        if (buffer->output->len == 1) { // full
            // wait until some elements are consumed
            pthread_cond_wait(&buffer->output->can_produce, &buffer->output->mutex);
            //printf("Status filter: %d\n", status);
        }

        memcpy(buffer->output->buf, samples, SAMPLES_PER_BUFFER);
        buffer->output->len = 1;

        // signal the fact that new items may be consumed
        pthread_cond_signal(&buffer->output->can_consume);
        pthread_mutex_unlock(&buffer->output->mutex);
        //printf("filter thread ran\n");

    }
    free_bw_low_pass(filter_bw);

    return NULL;
}
