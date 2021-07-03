#ifndef SOUND_MANAGER
#define SOUND_MANAGER

// Call in setup
int sound_init();
void sound_close();
void playInLoop(int frequency);
void stopPlaying();
#endif