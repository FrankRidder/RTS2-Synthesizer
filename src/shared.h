#ifndef SHARED_H
#define SHARED_H

#include <pthread.h>

#define SAMPLES_PER_BUFFER			8
#define SAMPLE_RATE                 44100

#define BUFFER_INITIALIZER ((struct buffer_type){.pitch = 440, .volume = 0.0,.waveform = SIN,.len = 0,.mutex = PTHREAD_MUTEX_INITIALIZER,.can_produce = PTHREAD_COND_INITIALIZER,.can_consume = PTHREAD_COND_INITIALIZER})

typedef void* TASK;

enum wavesforms{SIN, SQUARE, SAW, TRIANGLE};
int end_tasks;
unsigned int filter_freq;
unsigned int global_volume;

typedef struct buffer_type{
    int buf[SAMPLES_PER_BUFFER]; // the buffer
    size_t len; // number of items in the buffer

    unsigned int pitch;
    float volume;
    enum wavesforms waveform; 

    pthread_mutex_t mutex; // needed to add/remove data from the buffer
    pthread_cond_t can_produce; // signaled when items are removed
    pthread_cond_t can_consume; // signaled when items are added
} buffer_t;

typedef struct {
    buffer_t *input;
    buffer_t *output;
} arguments_t; 



#endif
