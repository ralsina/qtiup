char *RCScdhdcpy="$Id: wdhdcpy.c,v 1.1 2005/04/02 21:44:08 scuri Exp $";
/*
**   wdHdcpy.c - WD Hardcopy Client functions.
**
**   Description:
**   ---------------------------------------------------------------
**
**   This file contains functions to manage hardcopy redisplays of
**   CD canvas primitives define in world coordinates (WD).
**
**   ---------------------------------------------------------------
**
**   entry pts:
**   ---------------------------------------------------------------
**
**   void wdHardcopy(cdContext* ctx, cdCanvas *cnv, void *data, void(*draw_func)(void))
**
**      ctx         - target drivers (context)                    (in)
**      cnv         - target visualization surface (canvas)       (in)
**      data        - PS canvas information                       (in)
**      draw_func   - pointer to draw function                    (in)
**
**   ---------------------------------------------------------------
**
**   Version:      0-002
**
**   Created:    08-May-96    Luiz Fernando Martha
**       Stolen from g3dhdcpy.c and modified.
**
**   Modified:   04-Dec-98    Antonio Scuri
**       Use only one function to do the job.
**
*/

/*
**   ---------------------------------------------------------------
**   Global variables and symbols
*/
#include <stdlib.h>
#include <stdio.h>

#include "cd.h"
#include "wd.h"

/*
**   ---------------------------------------------------------------
**   Local variables and symbols:
*/

/*
**   ---------------------------------------------------------------
**   Private functions:
*/
static void _wdHdcpyDoit(cdCanvas *cnv, cdCanvas *hcp, void(*draw_func)(void));


/*========================  _wdHdcpyDoit  =======================*/

static void _wdHdcpyDoit(cdCanvas *cnv, cdCanvas *hcp, void(*draw_func)(void))
{
  double  left, right, bottom, top;   /* canvas visualization window       */
  int     canvas_hsize, canvas_vsize; /* canvas sizes in pixels            */
  int     hdcpy_hsize, hdcpy_vsize;   /* paper sizes in points             */
  double  canvas_vpr;                 /* canvas viewport distortion ratio  */
  double  hdcpy_vpr;                  /* paper viewport distortion ratio   */
  int     xc, yc;                     /* paper center in pixels            */
  int     xmin, xmax, ymin, ymax;     /* paper viewport                    */

  /* Activate canvas visualization surface. */
  if (cdActivate(cnv) != CD_OK) return;

  /* Get current canvas window parameters and sizes. */
  wdGetWindow( &left, &right, &bottom, &top );
  cdGetCanvasSize( &canvas_hsize, &canvas_vsize, 0L, 0L );

  /* Activate hardcopy visualization surface. */
  if (cdActivate(hcp) != CD_OK) return;

  /* Set window parameters on hardcopy surface. */
  wdWindow( left, right, bottom, top );

  /* Adjust paper viewport, centralized, matching canvas viewport. */
  canvas_vpr = (double)canvas_vsize / (double)canvas_hsize;
  cdGetCanvasSize( &hdcpy_hsize, &hdcpy_vsize, 0L, 0L );
  hdcpy_vpr = (double)hdcpy_vsize / (double)hdcpy_hsize;
  xc = (int)((double)hdcpy_hsize/2.0);
  yc = (int)((double)hdcpy_vsize/2.0);

  if (canvas_vpr < hdcpy_vpr)
  {
    xmin = 0;
    xmax = hdcpy_hsize;
    ymin = yc - (int)((double)hdcpy_hsize*(double)canvas_vpr/2.0);
    ymax = yc + (int)((double)hdcpy_hsize*(double)canvas_vpr/2.0);
  }
  else
  {
    xmin = xc - (int)((double)hdcpy_vsize/(double)canvas_vpr/2.0);
    xmax = xc + (int)((double)hdcpy_vsize/(double)canvas_vpr/2.0);
    ymin = 0;
    ymax = hdcpy_vsize;
  }

  cdClipArea(xmin, xmax, ymin, ymax);
  cdClip(CD_CLIPAREA);
  wdViewport(xmin, xmax, ymin, ymax);

  /* Draw on hardcopy surface.  */
  draw_func();

  /* Reactivate given canvas. */
  cdActivate(cnv);
}

/*
**   ---------------------------------------------------------------
**   Entry points begin here:
*/

/*===========================  wdHdcpyPS  =======================*/

void wdHardcopy(cdContext* ctx, void *data, cdCanvas *cnv, void(*draw_func)(void))
{
 cdCanvas *canvas;

/* Create a visualization surface.
 */
 canvas = cdCreateCanvas(ctx, data);
 if (!canvas) return;

/* Do hardcopy.
 */
 _wdHdcpyDoit(cnv, canvas, draw_func);

/* Destroy visualization surface.
 */
 cdKillCanvas(canvas);
}

