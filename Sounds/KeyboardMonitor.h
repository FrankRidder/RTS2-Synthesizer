#ifndef KEYBOARD_MONITOR
#define KEYBOARD_MONITOR

// Call in setup
extern void set_getch_mode();

extern void cancel_getch_mode();

// Non blocking getchar() method
extern int getch();
#endif