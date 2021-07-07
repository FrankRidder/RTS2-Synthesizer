#ifndef SOUND_MANAGER
#define SOUND_MANAGER

// Call in setup
extern void al_init();
extern void al_exit();
extern void playInLoop(int source, int frequency);
extern void stopPlaying(int source);
TASK audioThread(void* arg);
#endif