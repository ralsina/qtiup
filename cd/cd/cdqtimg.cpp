#include <stdlib.h>

#include <QPixmap>
#include <QPainter>

#include "cdqt.h"
#include "cdimage.h"


static void cdkillcanvas(void *cnv)
{
  //FIXME implement
}

static int cdactivate(void *data)
{
  _cdcnvQt = (cdCanvasQt *)data;
  //cdupdate();
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
  QPixmap *cdi = ((cdxImage *)((cdServerImage*)data)->img)->img;
  cdCanvasQt *new_cnv =new cdCanvasQt;
  new_cnv->widget=cdi;
  new_cnv->cnv=cnv;
  cnv->context_canvas=new_cnv;
}

static void cdinittable(cdPrivateCanvas* cnv)
{
  cdQtinittable(cnv);

  cnv->KillCanvas = cdkillcanvas;
  cnv->Activate = cdactivate;
  cnv->Deactivate = cddeactivate;
  cnv->Clear = cdclear;
}

/******************************************************/

static cdPrivateContext cdImageContextTable =
{
  CD_CAP_ALL & ~(CD_CAP_PLAY | CD_CAP_YAXIS | 
                 CD_CAP_MARK | 
                 CD_CAP_WD | CD_CAP_VECTORTEXT),
  cdcreatecanvas,
  cdinittable,
  NULL,
  NULL
};
 
static cdContext cdImageCtx =
{
  &cdImageContextTable
};


cdContext* cdContextImage(void)
{
  return &cdImageCtx;
}


