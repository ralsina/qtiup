#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "iup.h"
#include "cd.h"
#include "iupcontrols.h"


static int nlines_cb(Ihandle* h) {
  return 7;
}

static int ncols_cb(Ihandle* h) {
  return 7;
}

static int height_cb(Ihandle* h, int i) {
  return (int)(30+i*1.5);
}

static int width_cb(Ihandle* h, int j) {
  return (int)(50+j*1.5);
}

static int mouseclick_cb(Ihandle* h, int b, char* m, int i, int j, 
int x, int y, char* r) {
  printf("CLICK: %d: (%02d, %02d)\n", b, i, j);
  return IUP_DEFAULT;
}

static int scrolling_cb(Ihandle* h, int i, int j) {
  printf("SCROLL: (%02d, %02d)\n", i, j);
  return IUP_DEFAULT;
}

static int vspan_cb(Ihandle* h, int i, int j) {
  if (i == 1 && j == 1) return 2;
  if (i == 5 && j == 5) return 2;
  return 1;
}

static int hspan_cb(Ihandle* h, int i, int j) {
  if (i == 1 && j == 1) return 2;
  if (i == 5 && j == 5) return 2;
  return 1;
}

static int draw_cb(Ihandle* h, int i, int j, int xmin, int xmax,
int ymin, int ymax) {
  int xm = (xmax + xmin) / 2;
  int ym = (ymax + ymin) / 2;
  static char buffer[64];

  if (i == 1 && j == 2) return IUP_DEFAULT;
  if (i == 2 && j == 1) return IUP_DEFAULT;
  if (i == 2 && j == 2) return IUP_DEFAULT;
  if (i == 5 && j == 6) return IUP_DEFAULT;
  if (i == 6 && j == 5) return IUP_DEFAULT;
  if (i == 6 && j == 6) return IUP_DEFAULT;

  if (i == 1 && j == 1) cdForeground(CD_WHITE);
  else {
    cdForeground(cdEncodeColor(
          (unsigned char)(i*20), 
          (unsigned char)(j*100), 
          (unsigned char)(i+100)
    ));
  }
  cdBox(xmin, xmax, ymin, ymax);
  cdTextAlignment(CD_CENTER);
  cdForeground(CD_BLACK);
  sprintf(buffer, "(%02d, %02d)", i, j);
  cdText(xm, ym, buffer);

  return IUP_DEFAULT;
}

static Ihandle* create(void) {
  Ihandle* cells = IupCells(); 
  IupSetCallback(cells, IUP_MOUSECLICK_CB, (Icallback)mouseclick_cb);
  IupSetCallback(cells, IUP_DRAW_CB, (Icallback)draw_cb);
  IupSetCallback(cells, IUP_WIDTH_CB, (Icallback)width_cb);
  IupSetCallback(cells, IUP_HEIGHT_CB, (Icallback)height_cb);
  IupSetCallback(cells, IUP_NLINES_CB, (Icallback)nlines_cb);
  IupSetCallback(cells, IUP_NCOLS_CB, (Icallback)ncols_cb);
  IupSetCallback(cells, IUP_HSPAN_CB, (Icallback)hspan_cb);
  IupSetCallback(cells, IUP_VSPAN_CB, (Icallback)vspan_cb);
  IupSetCallback(cells, IUP_SCROLLING_CB, (Icallback)scrolling_cb);
  IupSetAttribute(cells, IUP_BOXED, IUP_NO);
  IupSetAttribute(cells, IUP_RASTERSIZE,"395x255");

  return cells;
}

/* Main program */
int main (void) {
  Ihandle* dlg, *cells, *box;

  IupOpen();       
  IupControlsOpen();

  cells = create();
  box = IupVbox(cells, NULL);
  IupSetAttribute(box,"MARGIN","10x10");

  dlg = IupDialog(box) ;

  IupSetAttribute(dlg,IUP_TITLE,"IupCells");
  IupShowXY(dlg,IUP_CENTER,IUP_CENTER) ;
  IupMainLoop() ;
  IupDestroy(dlg);
  IupControlsClose();
  IupClose() ;  
  return 0 ;
}



