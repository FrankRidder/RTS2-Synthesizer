#include <mqueue.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <time.h>
#include <string.h>

#include "oscillatorThread.h"
#include "shared.h"

TASK oscillatorThread()
{
    mqd_t queue_handle;
    struct mq_attr msgq_attr;
    

    mqWave message;
    //char message[QUEUE_MSGSIZE];

    mq_unlink(QUEUE_NAME);

	/* Create the message queue. The queue reader is NONBLOCK. */
    struct mq_attr attr = QUEUE_ATTR_INITIALIZER;
	mqd_t mq = mq_open(QUEUE_NAME, O_CREAT | O_RDONLY | O_NONBLOCK, QUEUE_PERMS, &attr);
	if(mq < 0) {
		fprintf(stderr, "[CONSUMER]: Error, cannot open the queue: %s.\n", strerror(errno));
		return 0;
	}
    printf("[CONSUMER]: Queue opened, queue descriptor: %d.\n", mq);
	
    unsigned int prio;
    ssize_t bytes_read;
    while (!end_tasks)
    {
        bytes_read = mq_receive(mq, (char *)&message, QUEUE_MSGSIZE, &prio);
        if (bytes_read >= 0) 
        {
            printf("Received pitch: %d\n", message.pitch);
        }
        sleep(1);
    }
    mq_close(mq);
}

/*
 *  Generate sin wave from -32760 to 32760
 */
void generateSin(int freq, short *samples, int buf_size) 
{
    for (int i = 0; i < buf_size; ++i) {
        samples[i] = 32760 * sin( (2.f * M_PI * freq) / (float)SAMPLE_RATE * i );
    }
}

/*
 *  Generate saw wave from -32760 to 32760
 */
void generateSaw(int freq, short *samples, int buf_size) 
{
    for (int i = 0; i < buf_size; ++i) {
        samples[i] = 32760 * ( 2 * (freq / (float)SAMPLE_RATE * i + floor(freq / (float)SAMPLE_RATE * i + 0.5) ) );
    }
}

/*
 *  Generate square wave from -32760 to 32760
 */
void generateSquare(int freq, short *samples, int buf_size) 
{
    int N = SAMPLE_RATE / freq / 2;
    int sign = -1;
    for (int i = 0; i < buf_size; ++i) {
        if (i % N == 0) sign = -sign;
        samples[i] = 32760 * sign;
    }
}