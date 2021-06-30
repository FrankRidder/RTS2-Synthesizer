#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include <termios.h>
#include <signal.h>

//Functions used by threads
#include "inputThread.h"
#include "SoundManager.h"

#include "shared.h"


#define NTHREADS 4

// Signal Handler for SIGTSTP, we will get huge memory leaks if not closing properly.
void sighandler(int sig_num)
{
    // Reset handler to catch SIGTSTP next time
    signal(SIGTSTP, sighandler);
    printf("Cannot execute Ctrl+Z, use ESC instead\n");
}

int main(int argc, char *argv[]) {
    int status = 0;
    // Set the SIGTSTP (Ctrl-Z) signal handler
    // to sigHandler
    signal(SIGTSTP, sighandler);

    struct sched_param param;

    al_init();

    // Turn echo off
    struct termios term;
    tcgetattr(fileno(stdin), &term);
    term.c_lflag &= ~ECHO;
    tcsetattr(fileno(stdin), 0, &term);



    pthread_t threads[NTHREADS];
    pthread_attr_t tattr;
    pthread_attr_init(&tattr);//tattr init met defaultwaarden
    pthread_attr_setschedpolicy(&tattr, SCHED_RR); //sched policy aanpassen

    int fd; // File descriptor.
    if (2 != argc) {
        printf("Usage: %s <input device path>\n", argv[0]);
        return 1;
    }

    /* setting the new scheduling param */
    pthread_attr_setschedparam(&tattr, &param);

    printf("Creating threads .. \r\n");

    status = pthread_create(&threads[0], &tattr, KeyboardMonitor, (void *) argv[1]);    //Create threads
    if (status != 0) {
        printf("While creating thread 1, pthread_create returned error code %d\r\n", status);
        exit(-1);
    }
    pthread_join(threads[0], NULL);


    int stdin_copy = dup(STDIN_FILENO);
    /* remove garbage from stdin */
    tcdrain(stdin_copy);
    tcflush(stdin_copy, TCIFLUSH);
    close(stdin_copy);
    term.c_lflag |= ECHO;  /* turn on ECHO */
    tcsetattr(fileno(stdin), 0, &term);
    return 0;
}