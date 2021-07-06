#include "filterThread.h"
#include "filter.h"
#include <stdio.h>
#include <stdlib.h>    // gives malloc

TASK filterThread(void* arg)
{
    arguments_t *buffer = (arguments_t*)arg;
        
    short * samples = malloc(sizeof(short) * SAMPLES_PER_BUFFER);
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
        for(int i = 0; i < SAMPLES_PER_BUFFER; i++) {
            samples[i] = buffer->input->buf[i];
        }
        // signal the fact that new items may be produced
        pthread_cond_signal(&buffer->input->can_produce);
        pthread_mutex_unlock(&buffer->input->mutex);

        /*
         * ================ Process ====================
         */
        // Recreating filter because there is no option to change frequency (yet)
        BWLowPass* filter_bw = create_bw_low_pass_filter(4, SAMPLE_RATE, filter_freq);
        for(int i = 1; i < SAMPLES_PER_BUFFER; i++) {
            samples[i] = bw_low_pass(filter_bw, samples[i] * 10);
        }
        free_bw_low_pass(filter_bw);


        /*
         * ================ Produce ====================
         */
        pthread_mutex_lock(&buffer->output->mutex);
        if (buffer->output->len == 1) { // full
            // wait until some elements are consumed
            int status = pthread_cond_wait(&buffer->output->can_produce, &buffer->output->mutex);
            //printf("Status filter: %d\n", status);
        }
        for (int i = 0; i < SAMPLES_PER_BUFFER; i++)
        {
            buffer->output->buf[i] = samples[i];
        }
        buffer->output->len = 1;

        // signal the fact that new items may be consumed
        pthread_cond_signal(&buffer->output->can_consume);
        pthread_mutex_unlock(&buffer->output->mutex);
        //printf("filter thread ran\n");



        // for(int i = 1; i < buf_size; i++) {
        //     filtered_samples[i] = bw_low_pass(filter_bw, samples[i] * 10);
        // }
    }


    return NULL;
}