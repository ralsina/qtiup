/*
  IDLE Redraw in IupCanvas - Example in C 

  This example shows a better way to redraw in a canvas. Using the IDLE callback the 
  application saves unnecessary redraws. 
  
  In the example the redraw is made to be slow and a gauge is updated according to 
  the redraw status. The color of a button is also changed to indicate the end of the 
  canvas redraw. 

  Mark Stroetzel Glasberg <mark@tecgraf.puc-rio.br> - Fev, 2002

*/

#include <stdio.h>
#include <iup.h>
#include <cd.h>
#include <cdiup.h>
#include "iupcontrols.h"

Ihandle *dlg     = NULL;
Ihandle *bt      = NULL;
Ihandle *gauge   = NULL;
Ihandle *tabs    = NULL;
Ihandle *cv      = NULL;
cdCanvas*cdcanvas= NULL;

int need_redraw;

int fake_redraw(self)
{
  need_redraw = 1;
  return IUP_DEFAULT;
}

int redraw(self)
{
  if(need_redraw == 1)
  {
    int i;
    cdCanvas *oldCanvas;

    need_redraw = 0;
    IupSetAttribute(bt, IUP_FGCOLOR, "255 0 0");

    oldCanvas = cdActiveCanvas() ;
    cdActivate( cdcanvas );
    cdForeground( CD_BLUE );

    /* IupFlush should be called after cdActivate to allow the button to be
       updated after the canvas activation, otherwise Motif will not draw it */
    IupFlush();

    for(i = 0; i < 300000; i++)
    {
      char v[40];
      cdBox(0, 300, 0, i/1000);
      sprintf(v, "%f", (float)i/300000);
      IupSetAttribute(gauge, IUP_VALUE, v);

      /* Uncomment the next line if you do not wish the application to be blocked 
         during redraw (redraw will be slower) */
      /*IupLoopStep();*/
    }

    IupSetAttribute(bt, IUP_FGCOLOR, "255 255 255");
    cdActivate( oldCanvas );
  }
  return IUP_DEFAULT;
}

void init(void) 
{
  gauge = IupGauge();
  cv    = IupCanvas("fake_redraw");
  bt    = IupButton("BT!!!!", "nada");
  IupSetAttribute(bt,    IUP_SIZE, "50x50");
  IupSetAttribute(bt,    IUP_FGCOLOR, "255 255 255");
  IupSetAttribute(gauge, IUP_SIZE, "200x15");
  IupSetAttribute(cv,    IUP_SIZE, "200x200");
  dlg   = IupDialog(IupVbox(cv, IupHbox(gauge, bt, NULL), NULL));
  IupSetAttribute(dlg, IUP_TITLE, "Redraw test");
}

int main(void) 
{
  IupOpen();
  IupControlsOpen();
  
  init();

  IupSetFunction(IUP_IDLE_ACTION, (Icallback) redraw);

  IupMap(dlg);
  cdcanvas = cdCreateCanvas(CD_IUP, cv) ;
  IupSetCallback(cv, "ACTION", (Icallback) fake_redraw);

  IupShowXY(dlg, IUP_CENTER, IUP_CENTER);
  IupMainLoop();
  IupDestroy(dlg);
  IupControlsClose();
  IupClose();

  return 0;
}
