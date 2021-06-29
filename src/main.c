#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include <termios.h>

//Functions used by threads
#include "inputThread.h"
#include "SoundManager.h"

#include "shared.h"


#define NTHREADS 4


int main(int argc, char *argv[]) {
    int status = 0;

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

    pthread_exit(NULL);
}