#ifndef SOUND_MANAGER
#define SOUND_MANAGER

#include "shared.h"

// Call in setup
extern void al_init();
extern void al_exit();
TASK audioThread(void* arg);

#endif //SOUND_MANAGER
