#include <linux/input.h>

#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>

#define BITS_PER_LONG (sizeof(long) * 8)
#define NBITS(x) ((((x)-1)/BITS_PER_LONG)+1)
/*
	gcc -o output Main.c
	./output
 */

void KeyboardMonitor (void)
{
	int FileDevice;
	int ReadDevice;
	int Index;
	struct input_event InputEvent[64];
	int version;
	unsigned short id[4];
	unsigned long bit[EV_MAX][NBITS(KEY_MAX)];

	//----- OPEN THE INPUT DEVICE -----
	if ((FileDevice = open("/dev/input/event4", O_RDONLY)) < 0)		//<<<<SET THE INPUT DEVICE PATH HERE
	{
		perror("KeyboardMonitor can't open input device\r\n");
		close(FileDevice);
		return;
	}

	//----- GET DEVICE VERSION -----
	if (ioctl(FileDevice, EVIOCGVERSION, &version))
	{
		perror("KeyboardMonitor can't get version\r\n");
		close(FileDevice);
		return;
	}
	//printf("Input driver version is %d.%d.%d\n", version >> 16, (version >> 8) & 0xff, version & 0xff);

	//----- GET DEVICE INFO -----
	ioctl(FileDevice, EVIOCGID, id);
	//printf("Input device ID: bus 0x%x vendor 0x%x product 0x%x version 0x%x\n", id[ID_BUS], id[ID_VENDOR], id[ID_PRODUCT], id[ID_VERSION]);
	
	memset(bit, 0, sizeof(bit));
	ioctl(FileDevice, EVIOCGBIT(0, EV_MAX), bit[0]);

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
			return;
		}
		else
		{
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
						//----- KEY DOWN -----
						printf("key down\r\n");
					}
					else if (InputEvent[Index].value == 0)
					{
						//----- KEY UP -----
						printf("key up\r\n");
					}
				}
			}
		}

	}
}

int main(int argc, char *argv[])
{
 KeyboardMonitor();
}
