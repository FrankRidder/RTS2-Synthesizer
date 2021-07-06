#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include <termios.h>
#include <signal.h>

//Functions used by threads
#include "inputThread.h"
#include "SoundManager.h"
#include "oscillatorThread.h"
#include "filterThread.h"
#include "volumeThread.h"

#include "shared.h"

#define NTHREADS 20

// pthread variables
static pthread_t threads[NTHREADS];

// Terminal variables
static struct termios term;

// Signal Handler for SIGTSTP, we will get huge memory leaks if not closing properly.
void sighandler(int sig_num) {
    // Reset handler to catch SIGTSTP next time
    signal(SIGTSTP, sighandler);
    printf("Cannot execute Ctrl+Z, use ESC instead\n");

    for (int i = 0; i < NTHREADS; i++) {
        pthread_cancel(threads[i]);
    }
}

void initialiseTerminal() {
    tcgetattr(fileno(stdin), &term);
    term.c_lflag &= ~ECHO;
    tcsetattr(fileno(stdin), 0, &term);
}

void createThreads() {
    int status = 0;
    end_tasks = 0;
    filter_freq = 2000;
    global_volume = 1;
    filter_activated = 1;
    struct sched_param param;
    pthread_attr_t tattr;

    pthread_attr_init(&tattr);                      //tattr init met defaultwaarden
    pthread_attr_setschedpolicy(&tattr, SCHED_RR);  //sched policy aanpassen

    /* setting the new scheduling param */
    pthread_attr_setschedparam(&tattr, &param);
        
    printf("Creating threads .. \r\n");

    //static buffer_t buffer_from_keyboard_to_osc = BUFFER_INITIALIZER;
    static buffer_t buf_osc_to_filter[NUM_OSCS];
    static buffer_t buf_filter_to_volume[NUM_OSCS];
    static buffer_t buf_volume_to_audio[NUM_OSCS];
    static buffer_t buf_audio_to_osc[NUM_OSCS];

    for (int i = 0; i < NUM_OSCS; i++)
    {
        oscillators[i]          = OSCILLATOR_INITIALIZER;
        buf_osc_to_filter[i]    = BUFFER_INITIALIZER;
        buf_filter_to_volume[i] = BUFFER_INITIALIZER;
        buf_volume_to_audio[i]  = BUFFER_INITIALIZER;
        buf_audio_to_osc[i]     = BUFFER_INITIALIZER;
    }

    static arguments_t arg_oscillator[NUM_OSCS];
    static arguments_t arg_filter[NUM_OSCS];
    static arguments_t arg_volume[NUM_OSCS];
    static audio_arguments_t arg_audio;

    for (int i = 0; i < NUM_OSCS; i++)
    {
        arg_oscillator[i].input  = &buf_audio_to_osc[i];
        arg_oscillator[i].output = &buf_osc_to_filter[i];
        arg_oscillator[i].osc    = &oscillators[i];
        arg_oscillator[i].thread_id = i;

        arg_filter[i].input      = &buf_osc_to_filter[i];
        arg_filter[i].output     = &buf_filter_to_volume[i];
        arg_filter[i].osc        = &oscillators[i];
        arg_filter[i].thread_id = i;

        arg_volume[i].input      = &buf_filter_to_volume[i];
        arg_volume[i].output     = &buf_volume_to_audio[i];
        arg_volume[i].osc        = &oscillators[i];
        arg_volume[i].thread_id = i;

        arg_audio.input[i]       = &buf_volume_to_audio[i];
        arg_audio.output[i]      = &buf_audio_to_osc[i];
        arg_audio.thread_id      = i;
    }
    
    // Create threads
    int thread_id = 0;
    pthread_create(&threads[thread_id++], &tattr, KeyboardMonitor,  (void*)oscillators);  

    //static oscThreads[]
    for (int i = 0; i < NUM_OSCS; i++)
    {
        pthread_create(&threads[thread_id++], &tattr, oscillatorThread, (void*)&arg_oscillator[i]);
        pthread_create(&threads[thread_id++], &tattr, filterThread,     (void*)&arg_filter[i]);
        pthread_create(&threads[thread_id++], &tattr, volumeThread,     (void*)&arg_volume[i]);
    }
    pthread_create(&threads[thread_id++], &tattr, audioThread,      (void*)&arg_audio);
    
}

void terminate() {
    /* Wait for threads to end */
    for (int i = 0; i < NTHREADS; i++) {
        pthread_join(threads[i], NULL);
    }

    /* remove garbage from stdin */
    int stdin_copy = dup(STDIN_FILENO);
    tcdrain(stdin_copy);
    tcflush(stdin_copy, TCIFLUSH);
    close(stdin_copy);
    term.c_lflag |= ECHO;  /* turn on ECHO */
    tcsetattr(fileno(stdin), 0, &term);
}

int main(int argc, char *argv[]) {
    if (2 != argc) {
        printf("Usage: %s <input device path>\n", argv[0]);
        return 1;
    }

    signal(SIGTSTP, sighandler);    // Disable CTRL+Z
    initialiseTerminal();           // Disable echo
    al_init();                      // Initialise sound module
    KeyboardSetup((void *) argv[1]);  // Initialise the keyboard
    createThreads();                // Create threads

    terminate();                    // Wait for threads to end and terminate program
    return 0;
}