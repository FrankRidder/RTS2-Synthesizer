#include "KeyboardMonitor.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <termios.h>


struct termios orig_termios;

#define NO_KEY_PRESSED -1

void cancel_getch_mode()
{
   tcsetattr(0, TCSANOW, &orig_termios);
}

void set_getch_mode()
{
   struct termios new_termios;

   /* take two copies - one for now, one for later */

   tcgetattr(0, &orig_termios);
   memcpy(&new_termios, &orig_termios, sizeof(new_termios));

   /* register cleanup handler, and set the new terminal mode */

   atexit(cancel_getch_mode);
   cfmakeraw(&new_termios);
   tcsetattr(0, TCSANOW, &new_termios);
}

int getch()
{
   int r;
   unsigned char c;

   if ((r = read(0, &c, sizeof(c))) < 0)
      return NO_KEY_PRESSED;
   else
      return c;
}
