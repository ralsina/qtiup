/*Advanced IupCanvas Example in C 
This example shows how several canvas callbacks are used and how the scrollbar works. */

#include <stdio.h>
#include "iup.h"
#include "cd.h"
#include "cdiup.h"

unsigned char image[640][400];

int repaint_cb(Ihandle* self, float sx, float sy)
{
  int x, y;
  sx = IupGetFloat(self, IUP_POSX);
  sy = -IupGetFloat(self, IUP_POSY);

  if(cdActiveCanvas())
    cdClear();
	
  for(x=0;x<320;x++)
    for(y=0;y<200;y++)
    {
      if(image[(int)(320.0*sx)+x][(int)(200.0*sy)+y])cdMark(x,y);
    }

  return IUP_DEFAULT; 
}

int button_cb(Ihandle* self, int botao, int estado, int x, int y)
{
  float sx = IupGetFloat(self, IUP_POSX);
  float sy = -IupGetFloat(self, IUP_POSY);
	
  cdUpdateYAxis(&y);
  if(botao == IUP_BUTTON1 && estado)
  {
    cdMark(x,y);
    image[(int)(320.0*sx)+x][(int)(200.0*sy)+y] = 1 ;
  }
	
  return IUP_DEFAULT;	
}

int motion_cb(Ihandle* self, int x, int y)
{
  Ihandle* label;
  static char buffer[40];	
  float sx = IupGetFloat(self, IUP_POSX);

  float sy = -IupGetFloat(self, IUP_POSY);

  sprintf(buffer,"(%d, %d)",(int)(320.0*sx)+x,(int)(200.0*sy)+y) ;

  label = IupGetHandle("label");
  IupStoreAttribute(label,IUP_TITLE,buffer);  

  return IUP_DEFAULT;
}

int scroll_cb(Ihandle* self)
{
  repaint_cb(self,0.0,0.0);
	
  return IUP_DEFAULT;	
}

int enter_cb(Ihandle* self)
{
  cdBackground(CD_WHITE);	
  repaint_cb(self,0.0,0.0);
	
  return IUP_DEFAULT;
}

int leave_cb(Ihandle* self)
{
  Ihandle *label;	
  cdBackground(CD_GRAY);	
  repaint_cb(self,0.0,0.0);
  label = IupGetHandle("label");
  IupSetAttribute(label,IUP_TITLE,"IupCanvas");  
	
  return IUP_DEFAULT;
}

int main()
{
  Ihandle* dlg, *iupcanvas, *label;	
  cdCanvas *cdcanvas;
  int x,y;

  for(x=0;x<640;x++)
    for(y=0;y<400;y++)
      image[x][y]=0;	    

  IupOpen();

  label = IupLabel("IupCanvas");
  IupSetHandle("label", label);
  
  iupcanvas = IupCanvas(NULL);

  IupSetAttributes(iupcanvas, "CURSOR=CROSS, RASTERSIZE=320x200, EXPAND=NO, SCROLLBAR=YES, DX=0.5, DY=0.5");
  
  IupSetCallback(iupcanvas, "ACTION",(Icallback)repaint_cb);
  IupSetCallback(iupcanvas, "BUTTON_CB",(Icallback)button_cb);
  IupSetCallback(iupcanvas, "SCROLL_CB",(Icallback)scroll_cb);
  IupSetCallback(iupcanvas, "MOTION_cb",(Icallback)motion_cb);
  IupSetCallback(iupcanvas, "ENTERWINDOW_CB",(Icallback)enter_cb);
  IupSetCallback(iupcanvas, "LEAVEWINDOW_CB",(Icallback)leave_cb);
  
  dlg = IupDialog(IupVbox(iupcanvas,IupHbox(IupFill(), label, IupFill(), NULL), NULL));
  IupSetAttributes(dlg, "TITLE=IupCanvas, RESIZE=NO, MAXBOX=NO");

  IupShowXY(dlg,IUP_CENTER,IUP_CENTER);

  cdcanvas = cdCreateCanvas(CD_IUP, iupcanvas);
  if(!cdcanvas)
  {
    IupMessage("IupCanvas","Error creating canvas");
    IupDestroy(dlg);
    IupClose();
    return 0 ;
  }
  
  if(cdActivate(cdcanvas) == CD_ERROR)
  {
    IupMessage("IupCanvas","Error creating canvas");
    IupDestroy(dlg);
    IupClose();
    return 0 ;
  }
  
  cdClear();
  IupMainLoop();
  IupDestroy(dlg);
  IupClose();
  return 0;
}