#ifndef INPUT_THREAD
#define INPUT_THREAD

#include <linux/input.h>

#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>

void *KeyboardMonitor (void *pathname);

#endif