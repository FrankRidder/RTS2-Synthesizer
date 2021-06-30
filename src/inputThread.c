#include "inputThread.h"
//test
#include "SoundManager.h"

#define BITS_PER_LONG (sizeof(long) * 8)
#define NBITS(x) ((((x)-1)/BITS_PER_LONG)+1)

void *KeyboardMonitor(void *pathname) {
    int FileDevice;
    int ReadDevice;
    int Index;
    struct input_event InputEvent[64];
    int version;
    unsigned short id[4];
    unsigned long bit[EV_MAX][NBITS(KEY_MAX)];

    //----- OPEN THE INPUT DEVICE -----
    if ((FileDevice = open((char *) pathname, O_RDONLY)) < 0)		//<<<<SET THE INPUT DEVICE PATH HERE
    {
        perror("KeyboardMonitor can't open input device\r\n");
        close(FileDevice);
        return 0;
    }

    //----- GET DEVICE VERSION -----
    if (ioctl(FileDevice, EVIOCGVERSION, &version))
    {
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
    //----- READ KEYBOARD EVENTS -----
    while (1)
    {
        ReadDevice = read(FileDevice, InputEvent, sizeof(struct input_event) * 64);
        //printf("number of events: %d \r\n", ReadDevice / sizeof(struct input_event));
        if (ReadDevice < (int) sizeof(struct input_event))
        {
            //This should never happen
            printf("haha fail\r\n");
            perror("KeyboardMonitor error reading - keyboard lost?");
            close(FileDevice);
            return 0;
        }
        else
        {
            // Array to keep track of which keys have been pressed, so its associated tone can be stopped
            static int keyTracker[4] = {0, 0, 0, 0};
            for (Index = 0; Index < ReadDevice / sizeof(struct input_event); Index++)
            {
                //We have:
                //	InputEvent[Index].time		timeval: 16 bytes (8 bytes for seconds, 8 bytes for microseconds)
                //	InputEvent[Index].type		See input-event-codes.h
                //	InputEvent[Index].code		See input-event-codes.h
                //	InputEvent[Index].value		01 for keypress, 00 for release, 02 for autorepeat

                if (InputEvent[Index].type == EV_KEY)
                {
                    if (InputEvent[Index].value == 2)
                    {
                        //This is an auto repeat of a held down key
                        //cout << (int)(InputEvent[Index].code) << " Auto Repeat";
                        //cout << endl;
                    }
                    else if (InputEvent[Index].value == 1)
                    {
                        if (InputEvent[Index].code == KEY_ESC)
                        {
                            printf("Closing\n");
                            al_exit();
                            return 0;
                        }

                        //----- KEY DOWN -----
                        printf("key down\r\n");
                        for (int i = 0; i < 4; i++) {
                            if (keyTracker[i] == 0) {
                                keyTracker[i] = InputEvent[Index].code;
                                
                                // Semi random frequency from key code between 0-20KHz
                                //int frequency = (InputEvent[Index].code * 100) % 20000; 
                                int frequency = 440;
                                playInLoop(i, frequency);
                                break;
                            }
                        }
                    }
                    else if (InputEvent[Index].value == 0)
                    {
                        //----- KEY UP -----
                        printf("key up\r\n");
                        for (int i = 0; i < 4; i++) {
                            if (keyTracker[i] == InputEvent[Index].code) {
                                keyTracker[i] = 0;
                                stopPlaying(i);
                            }
                        }
                    }
                }
            }
        }
    }

}
