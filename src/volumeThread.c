#include "volumeThread.h"
#include "ADSR.h"
#include "shared.h"
#include <stdlib.h>    // gives malloc
#include <stdio.h>

TASK volumeThread(void *arg) {
    arguments_t *buffer = (arguments_t *) arg;

    ADSR *adsr = createADSR();

    // initialize settings
    setAttackRate(adsr,    .1 * SAMPLE_RATE );  // .1 second
    setDecayRate(adsr,     .3 * SAMPLE_RATE );
    setReleaseRate(adsr,    3 * SAMPLE_RATE );
    setSustainLevel(adsr,  .8 );
    
    // gate(&adsr, true);
    short *samples = malloc(sizeof(short) * SAMPLES_PER_BUFFER);
    float adsr_volume;

    while (!end_tasks) {
        /*
         * ================ Consume ====================
         */
        pthread_mutex_lock(&buffer->input->mutex);
        while (buffer->input->len == 0 && !end_tasks) { // empty
            // wait for new items to be appended to the buffer
            pthread_cond_wait(&buffer->input->can_consume, &buffer->input->mutex);
        }
        buffer->input->len = 0;
        for (int i = 0; i < SAMPLES_PER_BUFFER; i++) {
            samples[i] = buffer->input->buf[i];
        }
        // signal the fact that new items may be produced
        pthread_cond_signal(&buffer->input->can_produce);
        pthread_mutex_unlock(&buffer->input->mutex);

        /*
         * ================ Process ====================
         */

        gate(adsr, buffer->osc->turnon);
        //gate(adsr, true);
        for (int i = 1; i < SAMPLES_PER_BUFFER; i++) {
            adsr_volume = process(adsr);
            samples[i] *= global_volume * adsr_volume;
        }
        if (adsr_volume == 0) buffer->osc->pitch = 0;


        /*
         * ================ Produce ====================
         */
        pthread_mutex_lock(&buffer->output->mutex);
        if (buffer->output->len == 1) { // full
            // wait until some elements are consumed
            int status = pthread_cond_wait(&buffer->output->can_produce, &buffer->output->mutex);
            //printf("Status volume: %d\n", status);
        }
        for (int i = 0; i < SAMPLES_PER_BUFFER; i++) {
            buffer->output->buf[i] = samples[i];
        }
        buffer->output->len = 1;

        // signal the fact that new items may be consumed
        pthread_cond_signal(&buffer->output->can_consume);
        pthread_mutex_unlock(&buffer->output->mutex);
        //printf("volume thread ran\n");


        // for(int i = 1; i < buf_size; i++) {
        //     sample[i] *= global_volume * process(adsr);
        // }
    }
    free(adsr);
    return NULL;
}