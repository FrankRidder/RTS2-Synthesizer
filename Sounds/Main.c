#include "KeyboardMonitor.h"

#include <stdio.h>
#include <stdlib.h>    // gives malloc
#include <math.h>
#include <unistd.h>    // gives sleep
#include <pthread.h>

#ifdef __APPLE__
#include <OpenAL/al.h>
#include <OpenAL/alc.h>
#elif __linux
#include <AL/al.h>
#include <AL/alc.h>
#endif

/*
	 gcc -o output Main.c KeyboardMonitor.c -lm -lopenal
	 ./ouput

Uses OpenAL:
https://github.com/scottstensland/render-audio-openal 
 */

ALCdevice  * openal_output_device;
ALCcontext * openal_output_context;

ALuint internal_buffer;
ALuint internal_buffer2;
ALuint streaming_source[2];

int al_check_error(const char * given_label) {

    ALenum al_error;
    al_error = alGetError();

    if(AL_NO_ERROR != al_error) {

        printf("ERROR - %s  (%s)\r\n", alGetString(al_error), given_label);
        return al_error;
    }
    return 0;
}

void MM_init_al() {

    const char * defname = alcGetString(NULL, ALC_DEFAULT_DEVICE_SPECIFIER);

    openal_output_device  = alcOpenDevice(defname);
    openal_output_context = alcCreateContext(openal_output_device, NULL);
    alcMakeContextCurrent(openal_output_context);

    // setup buffer and source

    alGenBuffers(1, & internal_buffer);
    alGenBuffers(1, & internal_buffer2);
    al_check_error("failed call to alGenBuffers");
}

void MM_exit_al() {

    ALenum errorCode = 0;

    // Stop the sources
    alSourceStopv(1, & streaming_source[0]);        //      streaming_source
    alSourceStopv(1, & streaming_source[1]);
    int ii;
    for (ii = 0; ii < 2; ++ii) {
        alSourcei(streaming_source[ii], AL_BUFFER, 0);
    }
    // Clean-up
    alDeleteSources(1, &streaming_source[0]);
    alDeleteBuffers(16, &streaming_source[0]);
	alDeleteSources(1, &streaming_source[1]);
    alDeleteBuffers(16, &streaming_source[1]);
    
    errorCode = alGetError();
    alcMakeContextCurrent(NULL);
    errorCode = alGetError();
    alcDestroyContext(openal_output_context);
    alcCloseDevice(openal_output_device);
}

void MM_render_one_buffer() {

    /* Fill buffer with Sine-Wave */
    // float freq = 440.f;
    float freq = 440.f;
    float incr_freq = 0.1f;

    int seconds = 4;
    // unsigned sample_rate = 22050;
    unsigned sample_rate = 44100;
    double my_pi = 3.14159;
    size_t buf_size = seconds * sample_rate;

    // allocate PCM audio buffer        
    short * samples = malloc(sizeof(short) * buf_size);

   printf("\nhere is freq %f\r\n", freq);
    int i=0;
    for(; i<buf_size; ++i) {
        samples[i] = 32760 * sin( (2.f * my_pi * freq)/sample_rate * i );

        //freq += incr_freq;
        // incr_freq += incr_freq;
        // freq *= factor_freq;

        if (100.0 > freq || freq > 5000.0) {

            incr_freq *= -1.0f;
        }
    }

    /* upload buffer to OpenAL */
    alBufferData( internal_buffer, AL_FORMAT_MONO16, samples, buf_size, sample_rate);
    al_check_error("populating alBufferData");

    free(samples);

    /* Set-up sound source and play buffer */
    // ALuint src = 0;
    // alGenSources(1, &src);
    // alSourcei(src, AL_BUFFER, internal_buffer);
    alGenSources(1, & streaming_source[0]);
    alSourcei(streaming_source[0], AL_BUFFER, internal_buffer);
    // alSourcePlay(src);
    alSourcePlay(streaming_source[0]);

    // ---------------------

    ALenum current_playing_state;
    alGetSourcei(streaming_source[0], AL_SOURCE_STATE, & current_playing_state);
    al_check_error("alGetSourcei AL_SOURCE_STATE");

    while (AL_PLAYING == current_playing_state) {

       // printf("still playing ... so sleep\r\n");

        sleep(1);   // should use a thread sleep NOT sleep() for a more responsive finish

        alGetSourcei(streaming_source[0], AL_SOURCE_STATE, & current_playing_state);
        al_check_error("alGetSourcei AL_SOURCE_STATE");
    }

    printf("end of playing\r\n");

    /* Dealloc OpenAL */
    

}   //  MM_render_one_buffer
void MM_render_one_buffer2() {

    /* Fill buffer with Sine-Wave */
    // float freq = 440.f;
    float freq = 1000.f;
    float incr_freq = 0.1f;

    int seconds = 4;
    // unsigned sample_rate = 22050;
    unsigned sample_rate = 44100;
    double my_pi = 3.14159;
    size_t buf_size = seconds * sample_rate;

    // allocate PCM audio buffer        
    short * samples = malloc(sizeof(short) * buf_size);

   printf("\nhere is freq %f\r\n", freq);
    int i=0;
    for(; i<buf_size; ++i) {
        samples[i] = 32760 * sin( (2.f * my_pi * freq)/sample_rate * i );

        //freq += incr_freq;
        // incr_freq += incr_freq;
        // freq *= factor_freq;

        if (100.0 > freq || freq > 5000.0) {

            incr_freq *= -1.0f;
        }
    }

    /* upload buffer to OpenAL */
    alBufferData( internal_buffer2, AL_FORMAT_MONO16, samples, buf_size, sample_rate);
    al_check_error("populating alBufferData2");

    free(samples);

    /* Set-up sound source and play buffer */
    // ALuint src = 0;
    // alGenSources(1, &src);
    // alSourcei(src, AL_BUFFER, internal_buffer);
    alGenSources(1, & streaming_source[1]);
    alSourcei(streaming_source[1], AL_BUFFER, internal_buffer2);
    // alSourcePlay(src);
    alSourcePlay(streaming_source[1]);

    // ---------------------

    ALenum current_playing_state;
    alGetSourcei(streaming_source[1], AL_SOURCE_STATE, & current_playing_state);
    al_check_error("alGetSourcei AL_SOURCE_STATE");

    while (AL_PLAYING == current_playing_state) {

        //printf("still playing ... so sleep\r\n");

        sleep(1);   // should use a thread sleep NOT sleep() for a more responsive finish

        alGetSourcei(streaming_source[0], AL_SOURCE_STATE, & current_playing_state);
        al_check_error("alGetSourcei AL_SOURCE_STATE");
    }

    printf("end of playing\r\n");

    /* Dealloc OpenAL */

}   //  MM_render_one_buffer

int main(int argc, char *argv[])
{
   int c, done;

   set_getch_mode();
	
	
   printf("Press ctrl-C to finish\r\n");

   done = 0;
   
   pthread_t thread1;
    pthread_t thread2;

   while (!done)
   {
      c = getch();

      switch (c)
      {
         case 3: 
            done = 1;
            MM_exit_al();
            break;

         case 'w':
         case 'W':
            printf("w pressed\r\n");
            MM_init_al();
            //MM_render_one_buffer();
			pthread_create(&thread1, NULL, (void *) MM_render_one_buffer, NULL);
			usleep(500000);
			//MM_render_one_buffer2();
			pthread_create(&thread2, NULL, (void *) MM_render_one_buffer2, NULL);
			pthread_join(thread1, NULL);
			pthread_join(thread2, NULL);
			MM_exit_al();
            break;

         case 'a':
         case 'A':
            printf("a pressed\r\n");
            break;

         case 'd':
         case 'D':
            printf("d pressed\r\n");
            break;

         case 'x':
         case 'X':
            printf("x pressed\r\n");
            break;
      }
		
      usleep(1000); 
  }
}

