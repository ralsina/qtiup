#ifndef CDQT_H
#define CDQT_H

#include "cd.h"
#include "cdprivat.h"
#include <QColor>
#include <QPixmap>

/*
%V Contexto grafico do canvas
*/

class QPaintDevice;

#ifdef __cplusplus
extern "C" {
#endif


typedef struct cdCanvasQt {
  cdPrivateCanvas* cnv;
  QPaintDevice *widget;
  QColor fg;
  QColor bg;
} cdCanvasQt;

extern cdCanvasQt *_cdcnvQt;


typedef struct cdxImage {
  unsigned int w, h, depth;
  QPixmap *img;
} cdxImage;


cdCanvasQt *cdQtcreatecnv(cdPrivateCanvas* cnv, void *widget);
void cdQtinittable(cdPrivateCanvas* cnv);

#ifdef __cplusplus
}
#endif

#endif
