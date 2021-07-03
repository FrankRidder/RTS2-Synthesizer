

#include <stdio.h>
#include <stdlib.h>    // gives malloc
#include <math.h>

#include "shared.h"
#include <string.h>
#include <stdint.h>

#include <soundio/soundio.h>

/* For testing */
#include "waveGenerator.h"
#include "filter.h"


struct SoundIoOutStream *outstream;
struct SoundIoDevice *device;
struct SoundIo *soundio;

static const size_t buf_size = 10 * SAMPLE_RATE;

static double seconds_offset = 0.0;
static volatile bool want_pause = false;

static volatile bool read = 1;

static short* buffer;

static void write_sample_s16ne(char *ptr, short sample) {
    int16_t *buf = (int16_t *)ptr;
    short range = (short)INT16_MAX - (short)INT16_MIN;
    short val = sample * range / 2.0;
    *buf = val;
}

static void write_callback(struct SoundIoOutStream *ioOutStream, int frame_count_min, int frame_count_max) {
    double float_sample_rate = ioOutStream->sample_rate;
    double seconds_per_frame = 1.0 / float_sample_rate;
    struct SoundIoChannelArea *areas;
    int err;

    int frames_left = frame_count_max;

    for (;;) {
        int frame_count = frames_left;
        if ((err = soundio_outstream_begin_write(ioOutStream, &areas, &frame_count))) {
            fprintf(stderr, "unrecoverable stream error: %s\n", soundio_strerror(err));
            exit(1);
        }
        if (!frame_count)
            break;

        const struct SoundIoChannelLayout *layout = &ioOutStream->layout;
        for (int frame = 0; frame < buf_size-1; frame++) {
            for (int channel = 0; channel < layout->channel_count; channel++) {
                write_sample_s16ne(areas[channel].ptr, buffer[frame]);
                areas[channel].ptr += areas[channel].step;
            }
        }

        read = 0;
        seconds_offset = fmod(seconds_offset + seconds_per_frame * frame_count, 1.0);

        if ((err = soundio_outstream_end_write(ioOutStream))) {
            if (err == SoundIoErrorUnderflow)
                return;
            fprintf(stderr, "unrecoverable stream error: %s\n", soundio_strerror(err));
            exit(1);
        }

        frames_left -= frame_count;
        if (frames_left <= 0)
            break;
    }

    soundio_outstream_pause(ioOutStream, want_pause);
}

u_int8_t sound_init() {

    buffer = calloc(buf_size, sizeof(short));

    soundio = soundio_create();
    if (!soundio) {
        fprintf(stderr, "out of memory\n");
        return 1;
    }

    int err = soundio_connect_backend(soundio, SoundIoBackendPulseAudio);

    if (err) {
        fprintf(stderr, "Unable to connect to backend: %s\n", soundio_strerror(err));
        return 1;
    }

    fprintf(stderr, "Backend: %s\n", soundio_backend_name(soundio->current_backend));

    // Needs te be done before being able to get the default output
    soundio_flush_events(soundio);

    int selected_device_index = soundio_default_output_device_index(soundio);

    if (selected_device_index < 0) {
        fprintf(stderr, "Output device not found\n");
        return 1;
    }

    device = soundio_get_output_device(soundio, selected_device_index);
    if (!device) {
        fprintf(stderr, "out of memory\n");
        return 1;
    }

    fprintf(stderr, "Output device: %s\n", device->name);

    if (device->probe_error) {
        fprintf(stderr, "Cannot probe device: %s\n", soundio_strerror(device->probe_error));
        return 1;
    }

    outstream = soundio_outstream_create(device);
    if (!outstream) {
        fprintf(stderr, "out of memory\n");
        return 1;
    }

    outstream->write_callback = write_callback;

    if ((err = soundio_outstream_open(outstream))) {
        fprintf(stderr, "unable to open device: %s", soundio_strerror(err));
        return 1;
    }

    fprintf(stderr, "Software latency: %f\n", outstream->software_latency);

    if (outstream->layout_error)
        fprintf(stderr, "unable to set channel layout: %s\n", soundio_strerror(outstream->layout_error));

    if ((err = soundio_outstream_start(outstream))) {
        fprintf(stderr, "unable to start device: %s\n", soundio_strerror(err));
        return 1;
    }
    return 0;
}

void sound_close() {
    // This function is not used (yet)
    soundio_outstream_destroy(outstream);
    soundio_device_unref(device);
    soundio_destroy(soundio);
}


void playInLoop(int frequency){
    double my_pi = 3.14159;

    // allocate PCM audio buffer        
    short * samples = malloc(sizeof(short) * buf_size);
    short * filtered_samples = malloc(sizeof(short) * buf_size);
    
    // for (int i = 0; i < buf_size; ++i) {
    //     samples[i] = 32760 * sin( (2.f * my_pi * frequency) / sample_rate * i );
    // }
    soundio_flush_events(soundio);
    soundio_outstream_pause(outstream, false);

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
    while(!read){
        
    }

    memcpy(buffer,filtered_samples,buf_size);
    free(samples);
    free(filtered_samples);
}

void stopPlaying(){
    soundio_flush_events(soundio);
    soundio_outstream_pause(outstream, true);
}
