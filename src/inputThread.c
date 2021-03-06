#include "inputThread.h"

#include <linux/input.h>

#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <math.h>

//test
#include "audioThread.h"

#define BITS_PER_LONG (sizeof(long) * 8)
#define NBITS(x) ((((x)-1)/BITS_PER_LONG)+1)

// Key notes
#define FREQUENCY(n) ( pow( pow(2.,1./12.), (n)-49. ) * 440. + .5)

// Terminal colors
#define KNRM  "\x1B[0m"
#define KRED  "\x1B[31m"
#define KGRN  "\x1B[32m"
#define KCYN  "\x1B[36m"

#define BOLDON  "\e[1m"
#define BOLDOFF "\e[0m"


static int FileDevice;
static int ReadDevice;

int KeyboardSetup(TASK pathname) {
    int version;
    unsigned short id[4];
    unsigned long bit[EV_MAX][NBITS(KEY_MAX)];

    //----- OPEN THE INPUT DEVICE -----
    if ((FileDevice = open((char *) pathname, O_RDONLY)) < 0)        //<<<<SET THE INPUT DEVICE PATH HERE
    {
        perror("KeyboardMonitor can't open input device\r\n");
        close(FileDevice);
        return 0;
    }

    //----- GET DEVICE VERSION -----
    if (ioctl(FileDevice, EVIOCGVERSION, &version)) {
        perror("KeyboardMonitor can't get version\r\n");
        close(FileDevice);
        return 0;
    }

    //----- GET DEVICE INFO -----
    ioctl(FileDevice, EVIOCGID, id);

    memset(bit, 0, sizeof(bit));
    ioctl(FileDevice, EVIOCGBIT(0, EV_MAX), bit[0]);
    return 1;
}

void printInformation(oscillators_t *oscs)  
{
    static const char *waves[4] = {"sin", "square", "saw", "triangle"};

    printf("\e[1;1H\e[2J"); // Clear terminal
    printf(KCYN"=============================================================\n"KNRM);
    printf(KCYN"======                  "BOLDON KGRN"RTS Synthesizer" BOLDOFF KCYN "                 =====\n"KNRM);
    printf(KCYN"======                      " KNRM "Made by" KCYN "                     =====\n"KNRM);
    printf(KCYN"======  "KNRM"Frank Ridder | Florian Hagens | Steven Wijnja"KCYN"   =====\n"KNRM);
    printf(KCYN"=============================================================\n"KNRM);
    printf("\n\n");

    printf("\t  Volume: %d%%\n\n", (int)(global_volume * 100 + 0.5));

    printf("\t  Filter is %s\n" , filter_activated ? KGRN "activated!" KNRM : KRED "deactivated!" KNRM);
    printf("\t  Filter cut-off frequency: %d \n\n", filter_freq);

    printf("\t  Octave: %d\n\n", octave);
    
    for (int i = 0; i < NUM_OSCS; i++)
    {
        printf("\t  Pitch: %d \t", oscs[i].pitch);
        printf("\t  Waveform: %s wave\n", waves[oscs[i].waveform]);
    }

    printf("\e[?25l"); // Hide cursor
}

TASK KeyboardMonitor(void *arg) {
    oscillators_t *oscs = (oscillators_t *) arg;
    struct input_event InputEvent[64];
    int Index;

    //----- READ KEYBOARD EVENTS -----
    while (!end_tasks) {
        ReadDevice = (int) read(FileDevice, InputEvent, sizeof(struct input_event) * 64);
        if (ReadDevice < (int) sizeof(struct input_event)) {
            //This should never happen
            printf("haha fail\r\n");
            perror("KeyboardMonitor error reading - keyboard lost?");
            close(FileDevice);
            return 0;
        } else {
            // Array to keep track of which keys have been pressed, so its associated tone can be stopped
            static int keyTracker[4] = {0, 0, 0, 0};
            for (Index = 0; Index < ReadDevice / sizeof(struct input_event); Index++) {
                //We have:
                //	InputEvent[Index].time		timeval: 16 bytes (8 bytes for seconds, 8 bytes for microseconds)
                //	InputEvent[Index].type		See input-event-codes.h
                //	InputEvent[Index].code		See input-event-codes.h
                //	InputEvent[Index].value		01 for keypress, 00 for release, 02 for autorepeat

                if (InputEvent[Index].type == EV_KEY) {
                    if (InputEvent[Index].value == 2) {
                        //This is an auto repeat of a held down key
                    } else if (InputEvent[Index].value == 1) {

                        int keycode = InputEvent[Index].code;

                        switch (keycode)
                        {
                            case KEY_ESC:
                                printf("Closing...\n");
                                end_tasks = 1;
                                sleep(1);
                                printf("\e[1;1H\e[2J"); // Clear terminal
                                al_exit();
                                return 0;

                            case KEY_KPMINUS:
                                if(filter_freq > 0) filter_freq -= 100;
                                break;

                            case KEY_KPPLUS:
                                if(filter_freq < 5000) filter_freq += 100;
                                break;

                            case KEY_MINUS:
                                if (global_volume > 0.0) global_volume -= 0.05f;
                                break;

                            case KEY_EQUAL:
                                if (global_volume < 1.0) global_volume += 0.05f;
                                break;

                            case KEY_SPACE:
                                filter_activated = !filter_activated;
                                break;

                            case KEY_KP1:
                                octave = 1;
                                break;
                            case KEY_KP2:
                                octave = 2;
                                break;
                            case KEY_KP3:
                                octave = 3;
                                break;
                            case KEY_KP4:
                                octave = 4;
                                break;
                            case KEY_KP5:
                                octave = 5;
                                break;
                            case KEY_KP6:
                                octave = 6;
                                break;
                            case KEY_KP7:
                                octave = 7;
                                break;
                            case KEY_KP8:
                                octave = 8;
                                break;
                            case KEY_KP9:
                                octave = 9;
                                break;


                            default: break;
                        }
                        
                        int n = (keycode + 1) % 14 + (12 * octave);
                        //----- KEY DOWN -----

                        if ((keycode >= 2 && keycode <= 8) ||
                                (keycode >= 16 && keycode <= 22) ||
                                (keycode >= 30 && keycode <= 36) ||
                                (keycode >= 44 && keycode <= 50)
                        ) {
                            for (int i = 0; i < NUM_OSCS; i++) {
                                if (keyTracker[i] == 0) {
                                    keyTracker[i] = InputEvent[Index].code;
                                    oscs[i].pitch = (int) FREQUENCY(n + 1);
                                    oscs[i].turnon = true;
                                    oscs[i].waveform = (InputEvent[Index].code + 1) / 14;
                                    break;
                                }
                            }
                        }
                    } else if (InputEvent[Index].value == 0) {
                        //----- KEY UP -----
                        for (int i = 0; i < NUM_OSCS; i++) {
                            if (keyTracker[i] == InputEvent[Index].code) {
                                keyTracker[i] = 0;
                                oscs[i].turnon = false;
                                break;
                            }
                        }
                    }
                }
            }
        
            printInformation(oscs);
        }
        
    }
    return NULL;
}
