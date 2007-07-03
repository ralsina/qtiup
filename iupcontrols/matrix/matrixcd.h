/** \file
 * \brief iupmatrix. CD help macros.
 *
 * See Copyright Notice in iup.h
 * $Id: matrixcd.h,v 1.8 2005/07/11 15:51:40 uid20021 Exp $
 */
 
#ifndef __MATRIXCD_H 
#define __MATRIXCD_H

#include <cd.h>
#include <cdiup.h>

#ifdef __cplusplus
extern "C" {
#endif

#define INVY(y) (YmaxCanvas(mat) - (y))

#define cdcv(mat) (mat)->cdh

#define IsCanvasSet(mat,err)  do{if((mat)->cdh) \
                                  err = CD_OK; \
                                else                            \
                                  err = CD_ERROR;               \
                               }while(0)

#define CdLine(x1,y1,x2,y2)         cdLine(x1,INVY(y1),x2,INVY(y2))
#define CdVertex(x,y)               cdVertex(x,INVY(y))
#define CdBox(xmin,xmax,ymin,ymax) \
	cdBox(xmin < xmax ? xmin : xmax, \
      	      xmin < xmax ? xmax : xmin,\
      	      INVY(ymin) < INVY(ymax) ? INVY(ymin) : INVY(ymax),\
      	      INVY(ymin) < INVY(ymax) ? INVY(ymax) : INVY(ymin))
#define CdRect(xmin,xmax,ymin,ymax) \
	cdRect(xmin < xmax ? xmin : xmax, \
      	      xmin < xmax ? xmax : xmin,\
      	      INVY(ymin) < INVY(ymax) ? INVY(ymin) : INVY(ymax),\
      	      INVY(ymin) < INVY(ymax) ? INVY(ymax) : INVY(ymin))
#define CdClipArea(x1,x2,y1,y2)     cdClipArea(x1,x2,INVY(y2),INVY(y1));

#define CdPutText(x,y,s,m)        do{cdTextAlignment(m);       \
                                     cdText(x,INVY(y),s);      \
                                    }while(0)

#define CdRestoreBgColor()  cdForeground(oldbgc);

void iupmatSetCdFrameColor(Ihandle *h);

#ifdef __cplusplus
}
#endif

#endif
