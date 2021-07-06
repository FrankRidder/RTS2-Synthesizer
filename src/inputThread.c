#include "inputThread.h"
#include <mqueue.h>
#include <time.h>
#include <string.h>
#include <pthread.h>
#include <math.h>

//test
#include "SoundManager.h"
#include "filterThread.h"
#include <errno.h>

#define BITS_PER_LONG (sizeof(long) * 8)
#define NBITS(x) ((((x)-1)/BITS_PER_LONG)+1)

#define NUM_NOTES 88
#define OCTAVE(n) (((n)+9)/12)
#define NOTENAME(n) (name[((n)+9)%12])
#define FREQUENCY(n) ( pow( pow(2.,1./12.), (n)-49. ) * 440. + .5)

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
    //printf("Input driver version is %d.%d.%d\n", version >> 16, (version >> 8) & 0xff, version & 0xff);

    //----- GET DEVICE INFO -----
    ioctl(FileDevice, EVIOCGID, id);
    //printf("Input device ID: bus 0x%x vendor 0x%x product 0x%x version 0x%x\n", id[ID_BUS], id[ID_VENDOR], id[ID_PRODUCT], id[ID_VERSION]);

    memset(bit, 0, sizeof(bit));
    ioctl(FileDevice, EVIOCGBIT(0, EV_MAX), bit[0]);
    printf("Waiting for input...\r\n");
    return 1;
}

TASK KeyboardMonitor(void *arg) {
    oscillators_t *oscs = (oscillators_t *) arg;
    struct input_event InputEvent[64];
    int Index;

    struct mq_attr msgq_attr;

    //----- READ KEYBOARD EVENTS -----
    while (!end_tasks) {
        ReadDevice = (int) read(FileDevice, InputEvent, sizeof(struct input_event) * 64);
        //printf("number of events: %d \r\n", ReadDevice / sizeof(struct input_event));
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
                        //cout << (int)(InputEvent[Index].code) << " Auto Repeat";
                        //cout << endl;
                    } else if (InputEvent[Index].value == 1) {

                        //playInLoop(0, 440);
                        if (InputEvent[Index].code == KEY_ESC) {
                            printf("Closing\n");
                            end_tasks = 1;
                            al_exit();
                            return 0;
                        }
                        if(InputEvent[Index].code == KEY_MINUS || InputEvent[Index].code == KEY_KPMINUS){
                            if(filter_freq > 0){
                                filter_freq -= 100;
                            }
                        }

                        if(InputEvent[Index].code == KEY_EQUAL || InputEvent[Index].code == KEY_KPPLUS ){
                            if(filter_freq < 5000){
                                filter_freq += 100;
                            }
                        }

                        /* Key codes:
                         * 1..7 => 2..8
                         * Q..U => 16..22
                         * A..J => 30..36
                         * Z..M => 44..50
                         */

                        char *name[12] = {"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"};
                        // C note is a, than some calculations to get up in octaives
                        const int octave = 4;
                        int keycode = InputEvent[Index].code;
                        int n = (keycode + 1) % 14 + (12 * octave);
                        printf("%-2s%d  %d\n", NOTENAME(n), OCTAVE(n), (int) FREQUENCY(n + 1));
                        //----- KEY DOWN -----
                        printf("Key down - key code: %d\n", InputEvent[Index].code);

                        if (keycode >= 2 && keycode <= 8 ||
                            keycode >= 16 && keycode <= 22 ||
                            keycode >= 30 && keycode <= 36 ||
                            keycode >= 44 && keycode <= 50
                                ) {
                            for (int i = 0; i < 3; i++) {
                                if (keyTracker[i] == 0) {
                                    keyTracker[i] = InputEvent[Index].code;
                                    oscs[i].pitch = (int) FREQUENCY(n + 1);
                                    oscs[i].turnon = true;
                                    oscs[i].waveform = (InputEvent[Index].code + 1) / 14; // TODO: Verify
                                    break;
                                }
                            }
                        }
                    } else if (InputEvent[Index].value == 0) {
                        //----- KEY UP -----
                        printf("Key up - key code: %d\n", InputEvent[Index].code);
                        for (int i = 0; i < 3; i++) {
                            if (keyTracker[i] == InputEvent[Index].code) {
                                keyTracker[i] = 0;
                                oscs[i].pitch = 440;
                                oscs[i].turnon = false;
                                break;
                            }
                        }
                        // stopPlaying(0);
                    }
                }
            }
        }
    }
    return NULL;
}
