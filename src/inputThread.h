#ifndef INPUT_THREAD
#define INPUT_THREAD

#include "shared.h"

int KeyboardSetup(TASK pathname);
TASK KeyboardMonitor(void* arg);

#endif //INPUT_THREAD
