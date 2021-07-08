#include "audioThread.h"

#include <stdio.h>
#include <stdlib.h>    // gives malloc
#include <unistd.h>    // gives sleep
#include <pthread.h>

#include <AL/al.h>
#include <AL/alc.h>
#include <string.h>


/* For testing */
#include "oscillatorThread.h"

#define  NBUFFERS 8

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
            ALint sState = 0;
            alGetSourcei(streaming_source[i], AL_SOURCE_STATE, &sState);
            if (sState != AL_PLAYING)
            {
                alSourcePlay(streaming_source[i]);
            }
        }

    }
}

void al_exit() {
    // Stop the sources
    alSourceStopv(NUM_OSCS, streaming_source);        //      streaming_source

    // Clean-up
    alDeleteSources(NUM_OSCS, streaming_source);
    for (int i = 0; i < NUM_OSCS; i++)
    {
        alDeleteBuffers(NBUFFERS, internal_buffer[i]);
    }
    
    alGetError();
    alcMakeContextCurrent(NULL);
    alGetError();
    alcDestroyContext(openal_output_context);
    alcCloseDevice(openal_output_device);
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

            memcpy(sample_buffers[i], buffer->input[i]->buf, SAMPLES_PER_BUFFER);


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
                usleep(50);
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
                pthread_cond_wait(&buffer->output[i]->can_produce, &buffer->output[i]->mutex);
                //printf("Status volume: %d\n", status);
            }
            buffer->output[i]->len = 1;
            // signal the fact that new items may be consumed
            pthread_cond_signal(&buffer->output[i]->can_consume);
            pthread_mutex_unlock(&buffer->output[i]->mutex);
        }

    }
    free(samples);
    return NULL;
}

