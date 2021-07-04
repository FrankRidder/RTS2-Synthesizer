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

ALCdevice  * openal_output_device;
ALCcontext * openal_output_context;

static ALuint internal_buffer[NBUFFERS];
static ALuint streaming_source[NBUFFERS];



int al_check_error(const char * given_label) {

    ALenum al_error;
    al_error = alGetError();

    if(AL_NO_ERROR != al_error) {
        printf("ERROR - %s  (%s)\r\n", alGetString(al_error), given_label);
        return al_error;
    }
    return 0;
}

void al_init() {
    const char * defname = alcGetString(NULL, ALC_DEFAULT_DEVICE_SPECIFIER);

    openal_output_device  = alcOpenDevice(defname);
    openal_output_context = alcCreateContext(openal_output_device, NULL);

    alcMakeContextCurrent(openal_output_context);
    al_check_error("alcMakeContextCurrent");

    alGenSources(NBUFFERS, streaming_source);
    al_check_error("alGenSources");

    alGenBuffers(NBUFFERS, internal_buffer);
    al_check_error("alGenBuffers");
}

void al_exit() { 
    // This function is not used (yet)


    ALenum errorCode = 0;

    // Stop the sources
    alSourceStopv(1, & streaming_source[0]);        //      streaming_source
    alSourceStopv(1, & streaming_source[1]);
    int ii;
    for (ii = 0; ii < 2; ++ii) {
        alSourcei(streaming_source[ii], AL_BUFFER, 0);
    }
    // Clean-up
    alDeleteSources(NBUFFERS, streaming_source);
    alDeleteBuffers(NBUFFERS, internal_buffer);
    
    errorCode = alGetError();
    alcMakeContextCurrent(NULL);
    errorCode = alGetError();
    alcDestroyContext(openal_output_context);
    alcCloseDevice(openal_output_device);
}


float filter(float cutofFreq){
    float RC = 1.0/(cutofFreq * 2 * M_PI);
    float dt = 1.0/SAMPLE_RATE;
    float alpha = dt/(RC+dt);

    return alpha;
}

void band_pass_example()
{
    BWBandPass* filter = create_bw_band_pass_filter(4, 250, 2, 45);
    
    for(int i = 0; i < 100; i++){
        printf("Output[%d]:%f\n", i, bw_band_pass(filter, i* 100));
    }

    free_bw_band_pass(filter);
}

TASK audioThread(void* arg)
{
    arguments_t *buffer = (arguments_t*)arg;
        
    

    while (!end_tasks)
    {
        short * samples = malloc(sizeof(short) * SAMPLES_PER_BUFFER);

        ALint availBuffers=0;
        alGetSourcei(streaming_source[0],AL_BUFFERS_PROCESSED,&availBuffers);
        while (availBuffers > 1 || !end_tasks)
        {
            usleep(100);
            alGetSourcei(streaming_source[0],AL_BUFFERS_PROCESSED,&availBuffers);
        }
        al_check_error("alGetQueuStatus");
        
        /*
         * ================ Consume ====================
         */
        pthread_mutex_lock(&buffer->input->mutex);
        while (buffer->input->len == 0 && !end_tasks) { // empty
            // wait for new items to be appended to the buffer
            pthread_cond_wait(&buffer->input->can_consume, &buffer->input->mutex);
        }
        --buffer->input->len;
        for(int i = 0; i < SAMPLES_PER_BUFFER; i++) {
           samples[i] = buffer->input->buf[i];
        }

        // signal the fact that new items may be produced
        pthread_cond_signal(&buffer->input->can_produce);
        pthread_mutex_unlock(&buffer->input->mutex);

        /*
         * ================ Process ====================
         */

        alBufferData( internal_buffer[0], AL_FORMAT_MONO16, buffer->input->buf, SAMPLES_PER_BUFFER, SAMPLE_RATE);
        al_check_error("alBufferData");

        // Queue the buffer
        alSourceQueueBuffers(streaming_source[0], 1, &internal_buffer[0]);
        al_check_error("alQueueData");

        // Restart the source if needed
        // (if we take too long and the queue dries up,
        //  the source stops playing).
        ALint sState = 0;
        alGetSourcei(streaming_source[0], AL_SOURCE_STATE, &sState);
        if (sState != AL_PLAYING) 
        {
            alSourcePlay(streaming_source[0]);
        }        

        printf("audio thread ran\n");
        free(samples);
    }
    
}

void playInLoop(int source, int frequency)
{
    assert(source < NBUFFERS);

    //alGenSources(1, &streaming_source[source]);
    unsigned sample_rate = 44100;
    double my_pi = 3.14159;
    size_t buf_size = 1 * sample_rate;

    // allocate PCM audio buffer        
    short * samples = malloc(sizeof(short) * buf_size);
    short * filtered_samples = malloc(sizeof(short) * buf_size);
    
    // for (int i = 0; i < buf_size; ++i) {
    //     samples[i] = 32760 * sin( (2.f * my_pi * frequency) / sample_rate * i );
    // }
    generateSaw(frequency, samples, buf_size);

    BWLowPass* filter_bw = create_bw_low_pass_filter(4, 44100, 2000);
    for(int i = 1; i < buf_size; i++) {
        filtered_samples[i] = bw_low_pass(filter_bw, samples[i] * 10);
    }
    free_bw_low_pass(filter_bw);
    /* Simple low pass filter
        float a = filter(50);
        filtered_samples[0] = a * samples[0];
        for(int i = 1; i < buf_size; i++) {
            filtered_samples[i] = filtered_samples[i - 1] + (a*(samples[i] - filtered_samples[i - 1]));
        }
    */

    alBufferData( internal_buffer[source], AL_FORMAT_MONO16, filtered_samples, buf_size, sample_rate);
    al_check_error("alBufferData");


    // Queue the buffer
    alSourceQueueBuffers(streaming_source[source],1,&internal_buffer[source]);

    free(samples);
    free(filtered_samples);

    // Restart the source if needed
    // (if we take too long and the queue dries up,
    //  the source stops playing).
    ALint sState=0;
    alGetSourcei(streaming_source[source],AL_SOURCE_STATE,&sState);
    if (sState!=AL_PLAYING) {
        alSourcePlay(streaming_source[source]);
    }

    // // Turn on looping and attach buffer
    // alSourcei(streaming_source[source], AL_LOOPING, 1);
    // alSourcei(streaming_source[source], AL_BUFFER, internal_buffer[source]);

    // alSourcePlay(streaming_source[source]);
}

void stopPlaying(int source)
{
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
