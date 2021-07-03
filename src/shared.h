#ifndef SHARED_H
#define SHARED_H

#define SAMPLES_PER_BUFFER			1024
#define SAMPLE_RATE                 44100

typedef void* TASK;

enum wavesforms{SIN, SQUARE, SAW, TRIANGLE};

unsigned int filter_freq;
unsigned int volume;

typedef struct mqWaveforms
{
    int pitch;
    enum wavesforms waveform; 
    float volume;
    // short frame_buffer[SAMPLES_PER_BUFFER];
} mqWaveforms;


#endif
