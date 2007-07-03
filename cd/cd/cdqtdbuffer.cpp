#include "cdqt.h"
#include "cddbuf.h"


static void cdQtkillcnvDBuffer (void* cnv)
{
  //FIXME implement
}

static void cdQtdeactivate(void)
{
  //FIXME implement
}

static void cdQtclear(void)
{
  //FIXME implement
}

static void cdQtFlushDBuffer( void )
{
  //FIXME implement
}

static void cdQtCreateDBuffer(cdPrivateCanvas* cnv, cdCanvas* cnv_dbuffer)
{
  //FIXME implement
}

static int cdQtUpdateDBuffer(void)
{
  //FIXME implement
}

static int cdQtActivateDBuffer(void *cnv)
{
  //FIXME implement
  return CD_OK;
}

static void cdQtCreateCanvasDBuffer(cdPrivateCanvas* cnv, void *data)
{

  // Data is actually an already created canvas, according to
  // The CD docs.

  // A DBuffer is an offline drawing context, so we just do it on a QPixmap.

  printf ("called cdQtCreateCanvasDBuffer \n");

}

static void cdinittable(cdPrivateCanvas* cnv)
{
  cdQtinittable(cnv);

  cnv->Activate = cdQtActivateDBuffer;
  cnv->Update = cdQtUpdateDBuffer;
  cnv->Flush = cdQtFlushDBuffer;
  cnv->KillCanvas = cdQtkillcnvDBuffer;
  cnv->Deactivate = cdQtdeactivate;
  cnv->Clear = cdQtclear;
}

static cdPrivateContext cdDBufferContextTable =
{
  CD_CAP_ALL & ~(CD_CAP_PLAY | CD_CAP_YAXIS | 
                 CD_CAP_MARK | 
                 CD_CAP_WD | CD_CAP_VECTORTEXT),
  cdQtCreateCanvasDBuffer,  
  cdinittable,
  NULL, 
  NULL, 
};

static cdContext cdDBuffer =
{
  &cdDBufferContextTable
};

cdContext* cdContextDBuffer(void)
{
  return &cdDBuffer;
}





