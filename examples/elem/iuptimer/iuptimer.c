/*IupTimer Example in C */
#include <stdio.h>
#include "iup.h"

Ihandle *timer1, *timer2;

int timer_cb(Ihandle *n)
{
  if(n == timer1)
    printf("timer 1 called\n");

  if(n == timer2)
  {
    printf("timer 2 called\n");
    return IUP_CLOSE;  
  }

  return IUP_DEFAULT;
}

int main()
{
  Ihandle *dialog, *text;

  IupOpen();

  text = IupLabel("Timer example");
  IupSetHandle("quit", text);

  /* Creating main dialog */
  dialog = IupDialog(IupVbox(text, NULL));
  IupSetAttribute(dialog, IUP_TITLE, "timer example");
  IupSetAttribute(dialog, IUP_SIZE, "200x200");
  IupShowXY(dialog, IUP_CENTER, IUP_CENTER);

  timer1 = IupTimer();
  timer2 = IupTimer();

  IupSetAttribute(timer1, "TIME", "100");
  IupSetAttribute(timer1, "RUN", "YES");
  IupSetCallback(timer1, "ACTION_CB", (Icallback)timer_cb);

  IupSetAttribute(timer2, "TIME", "400");
  IupSetAttribute(timer2, "RUN", "YES");
  IupSetCallback(timer2, "ACTION_CB", (Icallback)timer_cb);

  IupMainLoop();
  IupDestroy(timer1);
  IupDestroy(timer2);
  IupClose();

  return 0;
}
