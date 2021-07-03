#ifndef SHARED_H
#define SHARED_H

#define SAMPLES_PER_BUFFER			1024
#define SAMPLE_RATE                 44100

#define MQ_FROM_KEYBOARD_TO_OSC1     "/fromKeyboardToOsc1"
#define MQ_FROM_KEYBOARD_TO_OSC2     "fromKeyboardToOsc2"
#define MQ_FROM_KEYBOARD_TO_OSC3     "fromKeyboardToOsc3"

#define MQ_FROM_OSC_TO_FILTER1       "fromOscToFilter1"
#define MQ_FROM_OSC_TO_FILTER2       "fromOscToFilter2"
#define MQ_FROM_OSC_TO_FILTER3       "fromOscToFilter3"

#define MQ_FROM_FILTER_TO_VCA1       "fromFilterToVolume1"
#define MQ_FROM_FILTER_TO_VCA2       "fromFilterToVolume2"
#define MQ_FROM_FILTER_TO_VCA3       "fromFilterToVolume3"

#define MQ_FROM_VCA_TO_AUDIO1        "fromVolumeToAudio1"
#define MQ_FROM_VCA_TO_AUDIO2        "fromVolumeToAudio2"
#define MQ_FROM_VCA_TO_AUDIO3        "fromVolumeToAudio3"

#define QUEUE_NAME  "/fromKeyboardToOsc1" /* Queue name. */
#define QUEUE_PERMS ((int)(0644))
#define QUEUE_MAXMSG  16 /* Maximum number of messages. */
#define QUEUE_MSGSIZE 1024 /* Length of message. */
#define QUEUE_ATTR_INITIALIZER ((struct mq_attr){0, QUEUE_MAXMSG, QUEUE_MSGSIZE, 0, {0}})


typedef void* TASK;

enum wavesforms{SIN, SQUARE, SAW, TRIANGLE};
int end_tasks;
unsigned int filter_freq;
unsigned int global_volume;

typedef struct mqWave
{
    int pitch;
    enum wavesforms waveform; 
    float volume;
    // short frame_buffer[SAMPLES_PER_BUFFER];
} mqWave;



#endif
