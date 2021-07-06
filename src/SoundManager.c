#include <stdio.h>
#include <stdlib.h>    // gives malloc
#include <math.h>
#include <unistd.h>    // gives sleep
#include <pthread.h>


#include <AL/al.h>
#include <AL/alc.h>

#include <assert.h>

/* For testing */
#include "oscillatorThread.h"
#include "filter.h"


#define  NBUFFERS 4

ALCdevice *openal_output_device;
ALCcontext *openal_output_context;

static ALuint internal_buffer[NUM_OSCS][NBUFFERS];
static ALuint streaming_source[NUM_OSCS];


int al_check_error(const char *given_label) {

    ALenum al_error;
    al_error = alGetError();

    if (AL_NO_ERROR != al_error) {
        printf("ERROR - %s  (%s)\r\n", alGetString(al_error), given_label);
        return al_error;
    }
    return 0;
}

void al_init() {
    const char *defname = alcGetString(NULL, ALC_DEFAULT_DEVICE_SPECIFIER);

    openal_output_device = alcOpenDevice(defname);
    openal_output_context = alcCreateContext(openal_output_device, NULL);

    alcMakeContextCurrent(openal_output_context);
    al_check_error("alcMakeContextCurrent");

    alGenSources(NUM_OSCS, streaming_source);
    al_check_error("alGenSources");

    for (int i = 0; i < NUM_OSCS; i++)
    {
        alGenBuffers(NBUFFERS, internal_buffer[i]);
        al_check_error("alGenBuffers");
    }

    short * samples = malloc(sizeof(short) * SAMPLES_PER_BUFFER);

    for (int i = 0; i < NUM_OSCS; i++)
    {   
        for (int j = 0; j < NBUFFERS; j++)
        {
            alBufferData( internal_buffer[i][j], AL_FORMAT_MONO16, samples, SAMPLES_PER_BUFFER, SAMPLE_RATE);
            al_check_error("alBufferData");
            // Queue the buffer
            alSourceQueueBuffers(streaming_source[i], 1, &internal_buffer[i][j]);
            al_check_error("alQueueData");               
        }
        ALint sState = 0;
        alGetSourcei(streaming_source[i], AL_SOURCE_STATE, &sState);   
        if (sState != AL_PLAYING) 
        {
            alSourcePlay(streaming_source[i]);
        }
    }
}

void al_exit() {
    // This function is not used (yet)
    ALenum errorCode = 0;

    // Stop the sources
    alSourceStopv(NUM_OSCS, streaming_source);        //      streaming_source

    // Clean-up
    alDeleteSources(NUM_OSCS, streaming_source);
    for (int i = 0; i < NUM_OSCS; i++)
    {
        alDeleteBuffers(NBUFFERS, internal_buffer[i]);
    }
    
    errorCode = alGetError();
    alcMakeContextCurrent(NULL);
    errorCode = alGetError();
    alcDestroyContext(openal_output_context);
    alcCloseDevice(openal_output_device);
}


float filter(float cutofFreq) {
    float RC = 1.0f / (cutofFreq * 2.0f * (float)M_PI);
    float dt = 1.0f / SAMPLE_RATE;
    float alpha = dt / (RC + dt);

    return alpha;
}

void band_pass_example() {
    BWBandPass *filter = create_bw_band_pass_filter(4, 250.0f, 2.0f, 45.0f);

    for (int i = 0; i < 100; i++) {
        printf("Output[%d]:%f\n", i, bw_band_pass(filter, (float) i * 100));
    }

    free_bw_band_pass(filter);
}

TASK audioThread(void* arg)
{
    audio_arguments_t *buffer = (audio_arguments_t*)arg;
    static short * samples;
    samples = malloc(sizeof(short) * SAMPLES_PER_BUFFER);



    short sample_buffers[NUM_OSCS][SAMPLES_PER_BUFFER];
    while (!end_tasks)
    {
        /*
         * ================ Consume ====================
         */
        for (int i = 0; i < NUM_OSCS; i++)
        {
            pthread_mutex_lock(&buffer->input[i]->mutex);
            while (buffer->input[i]->len == 0 && !end_tasks) { // empty
                // wait for new items to be appended to the buffer
                pthread_cond_wait(&buffer->input[i]->can_consume, &buffer->input[i]->mutex);
            }
            --buffer->input[i]->len;
            for(int j = 0; j < SAMPLES_PER_BUFFER; j++) {
                sample_buffers[i][j] = buffer->input[i]->buf[j];
            }

            // signal the fact that new items may be produced
            pthread_cond_signal(&buffer->input[i]->can_produce);
            pthread_mutex_unlock(&buffer->input[i]->mutex);
            //printf("Pipeline %d done\n", buffer->thread_id);
        }



        static ALint availBuffers=0;
        ALuint  uiBuffer;
        for (int i = 0; i < NUM_OSCS; i++)
        {
            do {
                alGetSourcei(streaming_source[i],AL_BUFFERS_PROCESSED,&availBuffers);
                al_check_error("alStream");
                usleep(10);
            } while (availBuffers == 0 && !end_tasks);
            //printf("avail buffers %d\n", availBuffers);

            if (availBuffers >= 1 && !end_tasks)
            {
                // Remove the buffer from the queue (uiBuffer contains the buffer ID for the dequeued buffer)
                uiBuffer = 0;
                alSourceUnqueueBuffers(streaming_source[i], 1, &uiBuffer);
                //printf("Buffers left: %d\n", availBuffers);
                alBufferData( uiBuffer, AL_FORMAT_MONO16, sample_buffers[i], SAMPLES_PER_BUFFER, SAMPLE_RATE);
                al_check_error("alBufferData");

                // Queue the buffer
                alSourceQueueBuffers(streaming_source[i], 1, &uiBuffer);
                al_check_error("alQueueData");

                availBuffers--;
                //usleep(23*1000);
            }
            
            al_check_error("alGetQueuStatus");
            ALint sState = 0;
            alGetSourcei(streaming_source[i], AL_SOURCE_STATE, &sState);
            if (sState != AL_PLAYING) 
            {
                alSourcePlay(streaming_source[i]);
            }      
        }

        /*
         * ================ Produce ====================
         */
        for (int i = 0; i < NUM_OSCS; i++)
        {
            pthread_mutex_lock(&buffer->output[i]->mutex);
            if (buffer->output[i]->len == 1) { // full
                // wait until some elements are consumed
                int status = pthread_cond_wait(&buffer->output[i]->can_produce, &buffer->output[i]->mutex);
                //printf("Status volume: %d\n", status);
            }
            buffer->output[i]->len = 1;
            // signal the fact that new items may be consumed
            pthread_cond_signal(&buffer->output[i]->can_consume);
            pthread_mutex_unlock(&buffer->output[i]->mutex);
        }
        static int counter = 0;
        counter++;

        clock_gettime(CLOCK_MONOTONIC, &finish);
        double elapsed = 0.0;
        
        elapsed = (finish.tv_sec - start.tv_sec);
        elapsed += (finish.tv_nsec - start.tv_nsec) / 1000000000.0; //ns
        elapsed *= 1000;
        //printf("elapsed: %lf\n", elapsed);
        
        // static int prot = 0;
        // if (prot == 0) {
        //     clock_gettime(CLOCK_MONOTONIC, &finish);
        //     double elapsed;
        //     elapsed = (finish.tv_sec - start.tv_sec);
        //     elapsed += (finish.tv_nsec - start.tv_nsec) / 1000000000.0;
        //     prot++;
        //     printf("elapsed: %lf\n", elapsed);
        // }    

        //printf("audio thread ran\n");
        
        // clock_gettime(CLOCK_MONOTONIC, &finish);
        // double elapsed;
        // elapsed = (finish.tv_sec - start.tv_sec);
        // elapsed += (finish.tv_nsec - start.tv_nsec) / 1000000000.0; //ns

        // elapsed *= 1000; // From seconds to milliseconds
        // printf("Executed at %lf\n", elapsed);
        //clock_gettime(CLOCK_MONOTONIC, &start);
    }
    free(samples);
}

void playInLoop(int source, int frequency)
{
    // assert(source < NBUFFERS);

    // //alGenSources(1, &streaming_source[source]);
    // unsigned sample_rate = 44100;
    // double my_pi = 3.14159;
    // size_t buf_size = 1 * sample_rate;

    // // allocate PCM audio buffer        
    // short * samples = malloc(sizeof(short) * buf_size);
    // short * filtered_samples = malloc(sizeof(short) * buf_size);
    
    // // for (int i = 0; i < buf_size; ++i) {
    // //     samples[i] = 32760 * sin( (2.f * my_pi * frequency) / sample_rate * i );
    // // }
    // generateSaw(frequency, samples, buf_size);

    // BWLowPass* filter_bw = create_bw_low_pass_filter(4, 44100, 2000);
    // for(int i = 1; i < buf_size; i++) {
    //     filtered_samples[i] = bw_low_pass(filter_bw, samples[i] * 10);
    // }
    // free_bw_low_pass(filter_bw);
    // /* Simple low pass filter
    //     float a = filter(50);
    //     filtered_samples[0] = a * samples[0];
    //     for(int i = 1; i < buf_size; i++) {
    //         filtered_samples[i] = filtered_samples[i - 1] + (a*(samples[i] - filtered_samples[i - 1]));
    //     }
    // */

    // alBufferData( internal_buffer[source], AL_FORMAT_MONO16, filtered_samples, buf_size, sample_rate);
    // al_check_error("alBufferData");


    // // Queue the buffer
    // alSourceQueueBuffers(streaming_source[source],1,&internal_buffer[source]);

    // free(samples);
    // free(filtered_samples);

    // // Restart the source if needed
    // // (if we take too long and the queue dries up,
    // //  the source stops playing).
    // ALint sState=0;
    // alGetSourcei(streaming_source[source],AL_SOURCE_STATE,&sState);
    // if (sState!=AL_PLAYING) {
    //     alSourcePlay(streaming_source[source]);
    // }

    // // // Turn on looping and attach buffer
    // // alSourcei(streaming_source[source], AL_LOOPING, 1);
    // // alSourcei(streaming_source[source], AL_BUFFER, internal_buffer[source]);

    // // alSourcePlay(streaming_source[source]);
}

void stopPlaying(int source) {
    ALenum errorCode = 0;

    // Stop the source
    alSourceStop(streaming_source[source]);
    al_check_error("alSourceStop");

    // Stop looping and detach buffer
    alSourcei(streaming_source[source], AL_LOOPING, 0);
    alSourcei(streaming_source[source], AL_BUFFER, 0),
            al_check_error("alSourcei");

    //printf("Deleting sources from %d\r\n", source);
}
