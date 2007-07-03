/*
%M Canvas Draw no X11 - CD/Xlib - IUP
%a Renato Borges & Carlos Cassino
%d 03 Fev 95
%p Sistemas Graficos
*/

#include <stdlib.h>
#include <memory.h>
#include <iup.h>

#include "cdqt.h"
#include "cdiup.h"
#include <stdio.h>

#include <QWidget>
#include <QRect>
#include <QPainter>
#include <QX11Info>

#include <../../iup/qt/objects.h>

#include <iglobal.h>


static void cdkillcanvas(void *cnv)
{
  //FIXME implement
}

static int cdupdate(void)
{
  int x, y;
  unsigned int w,h;
  w=_cdcnvQt->widget->width();
  h=_cdcnvQt->widget->height();
  
  _cdcnvQt->cnv->w = w;
  _cdcnvQt->cnv->h = h;
  
  
  _cdcnvQt->cnv->w_mm = w / _cdcnvQt->cnv->xres;
  _cdcnvQt->cnv->h_mm = h / _cdcnvQt->cnv->yres;

  return CD_OK;
}

static int cdactivate(void *data)
{
  _cdcnvQt = (cdCanvasQt *)data;
  cdupdate();
  return CD_OK;
}

static void cddeactivate(void)
{
  cdFlush();
  _cdcnvQt = NULL;
}

static void cdclear(void)
{
  QPaintDevice *w=_cdcnvQt->widget;
  QPainter p(w);
  p.eraseRect(0,0,w->width(),w->height());
}

static void cdcreatecanvas(cdPrivateCanvas* cnv, void *data)
{

  // There is really no need to do much since
  // any Qt widget is capable enough to be a CD canvas
  
  Ihandle *h = (Ihandle *)data;
  cdCanvasQt *new_cnv =new cdCanvasQt;
  QWidget *w=(static_cast < IObject *>(h->handle))->widget;
  new_cnv->widget=w;
  new_cnv->cnv=cnv;
  cnv->context_canvas=new_cnv;
  
  //FIXME probably wrong as all hell
  cnv->bpp = 24;
  
  //Now this is mostly guesswork ;-)

  //cnv->xres = ((double)DisplayWidth(dpy, scr) / (double)DisplayWidthMM(dpy, scr));
  cnv->xres = ((double)QX11Info::appDpiX())/25.6;
  //cnv->yres = ((double)DisplayHeight(dpy, scr) / (double)DisplayHeightMM(dpy, scr));
  cnv->yres = ((double)QX11Info::appDpiY())/25.6;
  cnv->w_mm = ((double)cnv->w) / cnv->xres;
  cnv->h_mm = ((double)cnv->h) / cnv->yres;
  
  IupSetAttribute(h, "_CD_CANVAS", (char*)cnv->canvas);
}

static void cdinittable(cdPrivateCanvas* cnv)
{
  cdQtinittable(cnv);

  cnv->KillCanvas = cdkillcanvas;
  cnv->Activate = cdactivate;
  cnv->Update = cdupdate;
  cnv->Deactivate = cddeactivate;
  cnv->Clear = cdclear;

}

/******************************************************/

static cdPrivateContext cdIUPContextTable =
{
  CD_CAP_ALL,
  cdcreatecanvas,
  cdinittable,
  NULL,
  NULL
};

static cdContext cdIUP =
{
  &cdIUPContextTable
};


cdContext* cdContextIup(void)
{
  return &cdIUP;
}

