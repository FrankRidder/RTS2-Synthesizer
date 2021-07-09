#ifndef SHARED_H
#define SHARED_H

#include <pthread.h>
#include <stdbool.h>

#define NUM_OSCS 3

#define SAMPLES_PER_BUFFER			1024
#define SAMPLE_RATE                 44100

#define BUFFER_INITIALIZER ((struct buffer_type) {  \
    .len = 0,                                       \
    .mutex = PTHREAD_MUTEX_INITIALIZER,             \
    .can_produce = PTHREAD_COND_INITIALIZER,        \
    .can_consume = PTHREAD_COND_INITIALIZER         \
})                                                  

#define OSCILLATOR_INITIALIZER ((struct osc_type) {  \
    .pitch = 0,                                       \
    .waveform = SIN,                                    \
    .turnon = false,                                    \
    .free = true,                                       \
})   


/*
 * Global variables
 */
typedef void* TASK;
int end_tasks;

unsigned int filter_freq;
float global_volume;
int filter_activated;
int octave;

/*
 * Oscillator structs
 */
enum wavesforms{SIN, SQUARE, SAW, TRIANGLE};
typedef struct osc_type {
    unsigned int pitch;
    enum wavesforms waveform; 
    bool turnon;
    bool free;
} oscillators_t;

oscillators_t oscillators[NUM_OSCS];

/*
 * Buffer structs
 */
typedef struct buffer_type{
    short buf[SAMPLES_PER_BUFFER];      // the buffer
    size_t len;                         // number of items in the buffer

    pthread_mutex_t mutex;              // needed to add/remove data from the buffer
    pthread_cond_t can_produce;         // signaled when items are removed
    pthread_cond_t can_consume;         // signaled when items are added
} buffer_t;

typedef struct {
    buffer_t *input;
    buffer_t *output;
    oscillators_t *osc;
    int thread_id;
} arguments_t; 

typedef struct {
    buffer_t *input[4];
    buffer_t *output[4];
    oscillators_t *osc;
    int thread_id;
} audio_arguments_t;

#endif //SHARED_H
