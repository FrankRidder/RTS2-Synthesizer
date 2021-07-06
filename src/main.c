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

#define NTHREADS 10

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
    struct sched_param param;
    pthread_attr_t tattr;

    pthread_attr_init(&tattr);                      //tattr init met defaultwaarden
    pthread_attr_setschedpolicy(&tattr, SCHED_RR);  //sched policy aanpassen

    /* setting the new scheduling param */
    pthread_attr_setschedparam(&tattr, &param);

    printf("Creating threads .. \r\n");

    //static buffer_t buffer_from_keyboard_to_osc = BUFFER_INITIALIZER;
    static buffer_t buf_osc_to_filter1 = BUFFER_INITIALIZER;
    static buffer_t buf_filter_to_volume1 = BUFFER_INITIALIZER;
    static buffer_t buf_volume_to_audio1 = BUFFER_INITIALIZER;

    static buffer_t buf_audio_to_osc1 = BUFFER_INITIALIZER;

    for (int i = 0; i < NUM_OSCS; i++)
    {
        oscillators[i] = OSCILLATOR_INITIALIZER;
    }
    static arguments_t arg_oscillator1 = {
        .input = &buf_audio_to_osc1,
        .output = &buf_osc_to_filter1,
        .osc = &oscillators[0]
    };

    static arguments_t arg_filter1 = {
            .input = &buf_osc_to_filter1,
            .output = &buf_filter_to_volume1,
            .osc = &oscillators[0]
    };

    static arguments_t arg_volume1 = {
            .input = &buf_filter_to_volume1,
            .output = &buf_volume_to_audio1,
            .osc = &oscillators[0]
    };

    static arguments_t arg_arg_audio1 = {
        .input = &buf_volume_to_audio1,
        .output = &buf_audio_to_osc1,
        .osc = &oscillators[0]
    };

    // Create threads
    pthread_create(&threads[0], &tattr, KeyboardMonitor, (void *) oscillators);
    pthread_create(&threads[1], &tattr, oscillatorThread, (void *) &arg_oscillator1);
    pthread_create(&threads[2], &tattr, filterThread, (void *) &arg_filter1);
    pthread_create(&threads[3], &tattr, volumeThread, (void *) &arg_volume1);
    pthread_create(&threads[4], &tattr, audioThread, (void *) &arg_arg_audio1);

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