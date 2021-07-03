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

#include "shared.h"

#define NTHREADS 4

// pthread variables
static pthread_t threads[NTHREADS];

// Terminal variables
static struct termios term;

// Signal Handler for SIGTSTP, we will get huge memory leaks if not closing properly.
void sighandler(int sig_num)
{
    // Reset handler to catch SIGTSTP next time
    signal(SIGTSTP, sighandler);
    printf("Cannot execute Ctrl+Z, use ESC instead\n");
}

void initialiseTerminal()
{
    tcgetattr(fileno(stdin), &term);
    term.c_lflag &= ~ECHO;
    tcsetattr(fileno(stdin), 0, &term);
}

void createThreads()
{
    int status = 0;
    struct sched_param param;
    pthread_attr_t tattr;

    pthread_attr_init(&tattr);                      //tattr init met defaultwaarden
    pthread_attr_setschedpolicy(&tattr, SCHED_RR);  //sched policy aanpassen

    /* setting the new scheduling param */
    pthread_attr_setschedparam(&tattr, &param);

    printf("Creating threads .. \r\n");

    status = pthread_create(&threads[0], &tattr, KeyboardMonitor, NULL);    //Create threads
    if (status != 0) {
        printf("While creating thread 1, pthread_create returned error code %d\r\n", status);
        exit(-1);
    }

    status = pthread_create(&threads[1], &tattr, oscillatorThread, NULL);    //Create threads
    if (status != 0) {
        printf("While creating thread 1, pthread_create returned error code %d\r\n", status);
        exit(-1);
    }

    end_tasks = 0;
}

void terminate()
{
    /* Wait for threads to end */
    for (int i = 0; i < NTHREADS; i++)
    {
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

int main(int argc, char *argv[]) 
{
    if (2 != argc) {
        printf("Usage: %s <input device path>\n", argv[0]);
        return 1;
    }
    
    
    signal(SIGTSTP, sighandler);    // Disable CTRL+Z
    initialiseTerminal();           // Disable echo
    al_init();                      // Initialise sound module
    KeyboardSetup((void*)argv[1]);  // Initialise the keyboard
    createThreads();                // Create threads
    
    terminate();                    // Wait for threads to end and terminate program
    return 0;
}