

#include <stdio.h>
#include <stdlib.h>    // gives malloc
#include <math.h>
#include <unistd.h>    // gives sleep
#include <pthread.h>


#include <AL/al.h>
#include <AL/alc.h>

#include <assert.h>

/* For testing */
#include "waveGenerator.h"
#include "filter.h"


#define  NBUFFERS 4

ALCdevice  * openal_output_device;
ALCcontext * openal_output_context;

ALuint internal_buffer[NBUFFERS];
ALuint streaming_source[NBUFFERS];



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


void playInLoop(int source, int frequency)
{
    assert(source < NBUFFERS);

    //alGenSources(1, &streaming_source[source]);
    unsigned sample_rate = 44100;
    double my_pi = 3.14159;
    size_t buf_size = 10 * sample_rate;

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

    alGenBuffers(1, &internal_buffer[source]);
    al_check_error("alGenBuffers");

    alBufferData( internal_buffer[source], AL_FORMAT_MONO16, filtered_samples, buf_size, sample_rate);
    al_check_error("alBufferData");
    
    free(samples);
    free(filtered_samples);

    // Turn on looping and attach buffer
    alSourcei(streaming_source[source], AL_LOOPING, 1);
    alSourcei(streaming_source[source], AL_BUFFER, internal_buffer[source]);

    alSourcePlay(streaming_source[source]);
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

    // Delete buffer
    alDeleteBuffers(1, &internal_buffer[source]);
    al_check_error("alDeleteBuffers");

    //printf("Deleting sources from %d\r\n", source);
}
