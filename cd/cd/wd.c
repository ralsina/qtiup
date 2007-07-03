char *wd_c = "$Id: wd.c,v 1.8 2005/11/09 00:10:47 scuri Exp $";
/*
%M WD para Canvas Draw no X11 - CD/Xlib
%a Renato Borges & Carlos Cassino, Antonio Scuri
%d 03 Fev 95
%p Sistemas Graficos
*/

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <memory.h>

#include "cd.h"
#include "cdprivat.h"
#include "wd.h"


/*******************************************************************/
/*
%S             Fun'c~oes em coordenadas do mundo
*/
/*******************************************************************/

#define SWAPINT(a,b) {int c=a;a=b;b=c;}
#define _cdInvertYAxis(_) (active_private_canvas->h - (_) - 1)


static cdPrivateCanvas *active_private_canvas = NULL;


void wdCall(int usage)
{
  if (active_private_canvas->using_wd != usage)
  {
    active_private_canvas->using_wd = usage;
    active_private_canvas->wdCall(usage);
  }
}
    
static void wdUpdateTransformation(cdPrivateCanvas * cnv)
{
  cnv->sx = (cnv->view_xmax - cnv->view_xmin)/(cnv->win_xmax - cnv->win_xmin);
  cnv->tx =  cnv->view_xmin - cnv->win_xmin*cnv->sx;

  cnv->sy = (cnv->view_ymax - cnv->view_ymin)/(cnv->win_ymax - cnv->win_ymin);
  cnv->ty =  cnv->view_ymin - cnv->win_ymin*cnv->sy;

  cnv->s = sqrt(cnv->sx * cnv->sx + cnv->sy * cnv->sy);
}

void wdUpdateActive(cdPrivateCanvas* cnv)
{
  active_private_canvas = cnv;
}

void wdSetDefaults(cdPrivateCanvas* cnv)
{
  cnv->win_xmin = 0;
  cnv->win_xmax = cnv->w_mm;
  cnv->win_ymin = 0;
  cnv->win_ymax = cnv->h_mm;

  cnv->view_xmin = 0;
  cnv->view_xmax = cnv->w-1;
  cnv->view_ymin = 0;
  cnv->view_ymax = cnv->h-1;

  wdUpdateTransformation(cnv);
}

/******************************************************/
/* coordinate transformation                          */
/******************************************************/

/*
%F Define nova janela no mundo real para converter primitivas 
   do tipo wdLine atraves do par "window-viewport".
*/
void wdWindow(double xmin, double xmax, double  ymin, double ymax)
{
  active_private_canvas->win_xmin = xmin;
  active_private_canvas->win_xmax = xmax;
  active_private_canvas->win_ymin = ymin;
  active_private_canvas->win_ymax = ymax;

  wdUpdateTransformation(active_private_canvas);

  if (active_private_canvas->Window) 
    active_private_canvas->Window(xmin, xmax, ymin, ymax);
  else
  {
    if (active_private_canvas->using_wd) 
      active_private_canvas->wdCall(1);
  }
}

/*
%F Obt'em a janela corrente.
*/
void wdGetWindow (double *xmin, double  *xmax,  double  *ymin, double *ymax)
{
  if (xmin) *xmin = active_private_canvas->win_xmin;
  if (xmax) *xmax = active_private_canvas->win_xmax;
  if (ymin) *ymin = active_private_canvas->win_ymin;
  if (ymax) *ymax = active_private_canvas->win_ymax;
}

/*
%F Define novo viewport no canvas para converter primitivas 
   do tipo wdLine atraves do par "window-viewport".
*/
void wdViewport(int xmin, int xmax, int ymin, int ymax)
{
  active_private_canvas->view_xmin = xmin;
  active_private_canvas->view_xmax = xmax;
  active_private_canvas->view_ymin = ymin;
  active_private_canvas->view_ymax = ymax;

  wdUpdateTransformation(active_private_canvas);

  if (active_private_canvas->Viewport) 
    active_private_canvas->Viewport(xmin, xmax, ymin, ymax);
  else
  {
    if (active_private_canvas->using_wd) 
      active_private_canvas->wdCall(1);
  }
}

/*
%F Obt'em o viewport corrente.
*/
void wdGetViewport (int *xmin, int *xmax, int *ymin, int *ymax)
{
  if (xmin) *xmin = active_private_canvas->view_xmin;
  if (xmax) *xmax = active_private_canvas->view_xmax;
  if (ymin) *ymin = active_private_canvas->view_ymin;
  if (ymax) *ymax = active_private_canvas->view_ymax;
}

/*
% F Converte de coordenadas do mundo (reais) para coordenadas 
    em pixel (inteiras) no canvas.
*/
#define _wdWorld2Canvas(_xw, _yw, _xv, _yv)                   \
{                                                             \
  _xv = (int)(active_private_canvas->sx*(_xw) + active_private_canvas->tx); \
  _yv = (int)(active_private_canvas->sy*(_yw) + active_private_canvas->ty); \
}

void wdWorld2Canvas(double xw, double yw, int *xv, int *yv)
{
  if (xv) *xv = (int)(active_private_canvas->sx*xw + active_private_canvas->tx);
  if (yv) *yv = (int)(active_private_canvas->sy*yw + active_private_canvas->ty);
}

#define _wdWorld2CanvasSize(_Ww, _Hw, _Wv, _Hv)                   \
{                                                             \
  _Wv = (int)(active_private_canvas->sx*(_Ww)); \
  _Hv = (int)(active_private_canvas->sy*(_Hw)); \
}

void wdWorld2CanvasSize(double hw, double vw, int *hv, int *vv)
{
  if (hv) *hv = (int)(active_private_canvas->sx*hw);
  if (vv) *vv = (int)(active_private_canvas->sy*vw);
}

/*
%F Converte de coordenadas em pixel (inteiras) do canvas para coordenadas 
   do mundo (reais).
*/
#define _wdCanvas2World(_xv, _yv, _xw, _yw)                      \
{                                                                \
  _xw = ((double)(_xv) - active_private_canvas->tx)/active_private_canvas->sx; \
  _yw = ((double)(_yv) - active_private_canvas->ty)/active_private_canvas->sy; \
}

void wdCanvas2World (int xv, int yv, double *xw, double *yw)
{
  if (xw) *xw = ((double)xv - active_private_canvas->tx)/active_private_canvas->sx;
  if (yw) *yw = ((double)yv - active_private_canvas->ty)/active_private_canvas->sy;
}

/*
%F Define a area de clip em coordenadas do mundo.
*/
void wdClipArea(double xmin, double xmax, double ymin, double ymax)
{
  int xminr, xmaxr, yminr, ymaxr;

  if (active_private_canvas->wClipArea)
  {
    wdCall(1);
    active_private_canvas->wClipArea(&xmin, &xmax, &ymin, &ymax);

    _wdWorld2Canvas(xmin, ymin, xminr, yminr);
    _wdWorld2Canvas(xmax, ymax, xmaxr, ymaxr);

    if (active_private_canvas->origin)
    {
      xminr += active_private_canvas->origin_x;
      xmaxr += active_private_canvas->origin_x;
      yminr += active_private_canvas->origin_y;
      ymaxr += active_private_canvas->origin_y;
    }

    if (active_private_canvas->invert_yaxis)
    {
      yminr = _cdInvertYAxis(yminr);
      ymaxr = _cdInvertYAxis(ymaxr);
      SWAPINT(yminr, ymaxr);
    }

    active_private_canvas->clip_xmin = xminr;
    active_private_canvas->clip_xmax = xmaxr;
    active_private_canvas->clip_ymin = yminr;
    active_private_canvas->clip_ymax = ymaxr;
  }
  else if (active_private_canvas->ClipArea) 
  {
    _wdWorld2Canvas(xmin, ymin, xminr, yminr);
    _wdWorld2Canvas(xmax, ymax, xmaxr, ymaxr);
    cdClipArea(xminr, xmaxr, yminr, ymaxr);
  }
}

int wdPointInRegion(double x, double y)
{
  int xr, yr;
  _wdWorld2Canvas(x, y, xr, yr);
  return cdPointInRegion(xr, yr);
}

void wdOffsetRegion(double x, double y)
{
  int xr, yr;
  _wdWorld2Canvas(x, y, xr, yr);
  cdOffsetRegion(xr, yr);
}

void wdRegionBox(double *xmin, double *xmax, double *ymin, double *ymax)
{
  int xminr, xmaxr, yminr, ymaxr;
  cdRegionBox(&xminr, &xmaxr, &yminr, &ymaxr);
  _wdCanvas2World(xminr, yminr, *xmin, *ymin);
  _wdCanvas2World(xmaxr, ymaxr, *xmax, *ymax);
}

int wdGetClipArea(double *xmin, double *xmax, double *ymin, double *ymax)
{
  int xminr, xmaxr, yminr, ymaxr;
  int clip = cdGetClipArea(&xminr, &xmaxr, &yminr, &ymaxr);
  _wdCanvas2World(xminr, yminr, *xmin, *ymin);
  _wdCanvas2World(xmaxr, ymaxr, *xmax, *ymax);
  return clip;
}

double * wdGetClipPoly(int *n)
{
  static double wpoly[2*_CD_POLY_BLOCK];
  if (n) *n = active_private_canvas->clip_poly_n;
  if (active_private_canvas->clip_wpoly && active_private_canvas->clip_wpoly_n < 2*_CD_POLY_BLOCK)
  {
    memcpy(wpoly, active_private_canvas->clip_wpoly, active_private_canvas->clip_wpoly_n*sizeof(wdPoint));
    return wpoly;
  }
  else
    return NULL;
}

/******************************************************/
/* primitives                                         */
/******************************************************/

void wdLine (double x1, double y1, double x2, double y2)
{
  if (active_private_canvas->wLine)
  {
    if (active_private_canvas->sim_mode & CD_SIM_CLIP && 
        active_private_canvas->clip_mode != CD_CLIPOFF)
    {
      int xr1, xr2, yr1, yr2;
      _wdWorld2Canvas(x1, y1, xr1, yr1);
      _wdWorld2Canvas(x2, y2, xr2, yr2);
      cdLine(xr1, yr1, xr2, yr2);
      return;
    }

    wdCall(1);
    active_private_canvas->wLine(x1,y1,x2,y2);
  }
  else
  {
    int xr1, xr2, yr1, yr2;
    _wdWorld2Canvas(x1, y1, xr1, yr1);
    _wdWorld2Canvas(x2, y2, xr2, yr2);
    cdLine(xr1, yr1, xr2, yr2);
  }
}

void wdBox (double xmin, double xmax, double ymin, double ymax)
{
  if (active_private_canvas->interior_style == CD_HOLLOW)
  {
    wdRect(xmin, xmax, ymin, ymax);
    return;
  }

  if (active_private_canvas->wBox)
  {
    if (active_private_canvas->sim_mode & CD_SIM_CLIP && 
        active_private_canvas->clip_mode != CD_CLIPOFF)
    {
      int xminr, xmaxr, yminr, ymaxr;
      _wdWorld2Canvas(xmin, ymin, xminr, yminr);
      _wdWorld2Canvas(xmax, ymax, xmaxr, ymaxr);
      cdBox(xminr, xmaxr, yminr, ymaxr);
      return;
    }

    wdCall(1);
    active_private_canvas->wBox(xmin, xmax, ymin, ymax);
  }
  else
  {
    int xminr, xmaxr, yminr, ymaxr;
    _wdWorld2Canvas(xmin, ymin, xminr, yminr);
    _wdWorld2Canvas(xmax, ymax, xmaxr, ymaxr);
    cdBox(xminr, xmaxr, yminr, ymaxr);
  }
}

void wdRect(double xmin, double xmax, double ymin, double ymax)
{
  if (active_private_canvas->wRect)
  {
    if (active_private_canvas->sim_mode & CD_SIM_CLIP && 
        active_private_canvas->clip_mode != CD_CLIPOFF)
    {
      int xminr, xmaxr, yminr, ymaxr;
      _wdWorld2Canvas(xmin, ymin, xminr, yminr);
      _wdWorld2Canvas(xmax, ymax, xmaxr, ymaxr);
      cdRect(xminr, xmaxr, yminr, ymaxr);
      return;
    }

    wdCall(1);
    active_private_canvas->wRect(xmin, xmax, ymin, ymax);
  }
  else
  {
    int xminr, xmaxr, yminr, ymaxr;
    _wdWorld2Canvas(xmin, ymin, xminr, yminr);
    _wdWorld2Canvas(xmax, ymax, xmaxr, ymaxr);
    cdRect(xminr, xmaxr, yminr, ymaxr);
  }
}

void wdArc(double xc, double yc, double w, double h, double angle1, double angle2)
{
  if (active_private_canvas->wArc)
  {
    if (active_private_canvas->sim_mode & CD_SIM_CLIP && 
        active_private_canvas->clip_mode != CD_CLIPOFF)
    {
      int xcr, ycr, wr, hr;
      _wdWorld2Canvas(xc, yc, xcr, ycr);
      _wdWorld2CanvasSize(w, h, wr, hr);
      if (wr && hr) cdArc(xcr, ycr, wr, hr, angle1, angle2);
      return;
    }

    wdCall(1);
    active_private_canvas->wArc(xc, yc, w, h, angle1, angle2);
  }
  else
  {
    int xcr, ycr, wr, hr;
    _wdWorld2Canvas(xc, yc, xcr, ycr);
    _wdWorld2CanvasSize(w, h, wr, hr);
    if (wr && hr) cdArc(xcr, ycr, wr, hr, angle1, angle2);
  }
}

void wdSector(double xc, double yc, double w, double h, double angle1, double angle2)
{
  if (active_private_canvas->interior_style == CD_HOLLOW)
  {
    double xi,yi,xf,yf;
    
    xi = xc + w*cos(CD_DEG2RAD*angle1)/2.0;
    yi = yc + h*sin(CD_DEG2RAD*angle1)/2.0;
    
    xf = xc + w*cos(CD_DEG2RAD*angle2)/2.0;
    yf = yc + h*sin(CD_DEG2RAD*angle2)/2.0;

    wdArc(xc, yc, w, h, angle1, angle2);
    wdLine(xi, yi, xc, yc);
    wdLine(xc, yc, xf, yf);
    return;
  }

  if (active_private_canvas->wSector)
  {
    if (active_private_canvas->sim_mode & CD_SIM_CLIP && 
        active_private_canvas->clip_mode != CD_CLIPOFF)
    {
      int xcr, ycr, wr, hr;
      _wdWorld2Canvas(xc, yc, xcr, ycr);
      _wdWorld2CanvasSize(w, h, wr, hr);
      if (wr && hr) cdSector(xcr, ycr, wr, hr, angle1, angle2);
      return;
    }

    wdCall(1);
    active_private_canvas->wSector(xc, yc, w, h, angle1, angle2);
  }
  else
  {
    int xcr, ycr, wr, hr;
    _wdWorld2Canvas(xc, yc, xcr, ycr);
    _wdWorld2CanvasSize(w, h, wr, hr);
    cdSector(xcr, ycr, wr, hr, angle1, angle2);
  }
}

void wdChord(double xc, double yc, double w, double h, double angle1, double angle2)
{
  if (active_private_canvas->interior_style == CD_HOLLOW)
  {
    double xi,yi,xf,yf;
    
    xi = xc + w*cos(CD_DEG2RAD*angle1)/2.0;
    yi = yc + h*sin(CD_DEG2RAD*angle1)/2.0;
    
    xf = xc + w*cos(CD_DEG2RAD*angle2)/2.0;
    yf = yc + h*sin(CD_DEG2RAD*angle2)/2.0;

    wdArc(xc, yc, w, h, angle1, angle2);
    wdLine(xi, yi, xf, yf);
    return;
  }

  if (active_private_canvas->wChord)
  {
    if (active_private_canvas->sim_mode & CD_SIM_CLIP && 
        active_private_canvas->clip_mode != CD_CLIPOFF)
    {
      int xcr, ycr, wr, hr;
      _wdWorld2Canvas(xc, yc, xcr, ycr);
      _wdWorld2CanvasSize(w, h, wr, hr);
      if (wr && hr) cdChord(xcr, ycr, wr, hr, angle1, angle2);
      return;
    }

    wdCall(1);
    active_private_canvas->wChord(xc, yc, w, h, angle1, angle2);
  }
  else
  {
    int xcr, ycr, wr, hr;
    _wdWorld2Canvas(xc, yc, xcr, ycr);
    _wdWorld2CanvasSize(w, h, wr, hr);
    cdChord(xcr, ycr, wr, hr, angle1, angle2);
  }
}

void wdText(double x, double y, char *s)
{
  if (active_private_canvas->wText)
  {
    if (active_private_canvas->sim_mode & CD_SIM_CLIPAREA && 
        active_private_canvas->clip_mode == CD_CLIPAREA)
    {
      int xr, yr;
      _wdWorld2Canvas(x, y, xr, yr);
      cdText(xr, yr, s);
      return;
    }

    wdCall(1);
    active_private_canvas->wText(x, y, s);
  }
  else
  {
    int xr, yr;
    _wdWorld2Canvas(x, y, xr, yr);
    cdText(xr, yr, s);
  }
}

void wdVertex(double x, double y)
{
  if (active_private_canvas->wPoly)
  {
    active_private_canvas->wd_poly = 1;

    if (active_private_canvas->poly_n == active_private_canvas->wpoly_size)
    {
      active_private_canvas->wpoly_size += _CD_POLY_BLOCK;
      active_private_canvas->wpoly = (wdPoint *)realloc(active_private_canvas->wpoly, sizeof(wdPoint) * active_private_canvas->wpoly_size);
    }

    active_private_canvas->wpoly[active_private_canvas->poly_n].x = x;
    active_private_canvas->wpoly[active_private_canvas->poly_n].y = y;
    active_private_canvas->poly_n++;
  }
  else
  {
    int xr, yr;
    _wdWorld2Canvas(x, y, xr, yr);
    cdVertex(xr, yr);
  }
}

void wdMark(double x, double y)
{
  if (active_private_canvas->wMark)
  {
    if (active_private_canvas->sim_mode & CD_SIM_CLIP)
    {
      int xr, yr;
      _wdWorld2Canvas(x, y, xr, yr);

      if (active_private_canvas->invert_yaxis)
        yr = _cdInvertYAxis(yr);

	    if(active_private_canvas->sim_mode & CD_SIM_CLIPAREA && 
         active_private_canvas->clip_mode == CD_CLIPAREA && 
         cdSimClipPointInBox(xr,yr))
        return;
	    else if(active_private_canvas->sim_mode & CD_SIM_CLIPPOLY && 
              active_private_canvas->clip_mode == CD_CLIPPOLYGON && 
              cdSimClipPointInPoly(xr,yr) == 0) 
        return;
    }

    wdCall(1);
    active_private_canvas->wMark(x, y);
  }
  else
  {
    int xr, yr;
    _wdWorld2Canvas(x, y, xr, yr);
    cdMark(xr, yr);
  }
}

void wdPixel(double x, double y, long int color)
{
  int xr, yr;
  _wdWorld2Canvas(x, y, xr, yr);
  cdPixel(xr, yr, color);
}

void wdPutImageRect(void* image, double x, double y, int xmin, int xmax, int ymin, int ymax)
{
  int xr, yr;
  _wdWorld2Canvas(x, y, xr, yr);
  cdPutImageRect(image, xr, yr, xmin, xmax, ymin, ymax);
}

void wdPutImageRectRGB(int iw, int ih, unsigned char *r, unsigned char *g, unsigned char *b, double x, double y, double w, double h, int xmin, int xmax, int ymin, int ymax)
{
  int xr, yr, wr, hr;
  _wdWorld2Canvas(x, y, xr, yr);
  _wdWorld2CanvasSize(w, h, wr, hr);
  cdPutImageRectRGB(iw, ih, r, g, b, xr, yr, wr, hr, xmin, xmax, ymin, ymax);
}

void wdPutImageRectRGBA(int iw, int ih, unsigned char *r, unsigned char *g, unsigned char *b, unsigned char *a, double x, double y, double w, double h, int xmin, int xmax, int ymin, int ymax)
{
  int xr, yr, wr, hr;
  _wdWorld2Canvas(x, y, xr, yr);
  _wdWorld2CanvasSize(w, h, wr, hr);
  cdPutImageRectRGBA(iw, ih, r, g, b, a, xr, yr, wr, hr, xmin, xmax, ymin, ymax);
}

void wdPutImageRectMap(int iw, int ih, unsigned char *index, long int *colors, double x, double y, double w, double h, int xmin, int xmax, int ymin, int ymax)
{
  int xr, yr, wr, hr;
  _wdWorld2Canvas(x, y, xr, yr);
  _wdWorld2CanvasSize(w, h, wr, hr);
  cdPutImageRectMap(iw, ih, index, colors, xr, yr, wr, hr, xmin, xmax, ymin, ymax);
}

void wdPutBitmap(cdBitmap* image, double x, double y, double w, double h)
{
  int xr, yr, wr, hr;
  _wdWorld2Canvas(x, y, xr, yr);
  _wdWorld2CanvasSize(w, h, wr, hr);
  cdPutBitmap(image, xr, yr, wr, hr);
}


/******************************************************/
/* attributes                                         */
/******************************************************/

/*
%F Define a largura da linha em milimetros.
*/
double wdLineWidth(double width_mm)
{
  int width;
  double line_width_mm = active_private_canvas->line_width/active_private_canvas->xres;

  if (width_mm == CD_QUERY)
    return line_width_mm;

  width = (int)(width_mm*active_private_canvas->xres);
  if (width < 1) width = 1;

  if (active_private_canvas->LineWidth) 
    active_private_canvas->line_width = active_private_canvas->LineWidth(width);

  return line_width_mm;
}

/*
%F Define o tamanho da fonte em milimetros.
*/
void wdFont(int type_face, int style, double size_mm)
{
  cdFont(type_face, style, (int)(size_mm*CD_MM2PT));
}

void wdGetFont(int *type_face, int *style, double *size)
{
  int point_size;
  cdGetFont(type_face, style, &point_size);
  if (point_size<0)
  {
    if (size) cdPixel2MM(-point_size, 0, size, NULL);
  }
  else
  {
    if (size) *size = ((double)point_size) / CD_MM2PT;
  }
}

/*
%F Define o tamanho da marca em milimetros.
*/
double wdMarkSize(double size_mm)
{
  int size;
  double mark_size_mm = active_private_canvas->mark_size/active_private_canvas->xres;

  if (size_mm == CD_QUERY)
    return mark_size_mm;

  size = (int)(size_mm*active_private_canvas->xres);
  if (size < 1) size = 1;

  active_private_canvas->mark_size = size;

  return mark_size_mm;
}

/*
%F Obt'em as maiores dimens~oes poss'iveis para um caracter no fonte corrente.
*/
void wdFontDim(double *max_width, double *height, double *ascent, double *descent)
{
  double origin_x, origin_y;
  double distance_x, distance_y;
  int font_max_width, font_height, font_ascent, font_descent;
  cdFontDim(&font_max_width, &font_height, &font_ascent, &font_descent);
  _wdCanvas2World(0, 0, origin_x, origin_y);
  _wdCanvas2World(font_max_width, font_height, distance_x, distance_y);
  if (max_width) *max_width = fabs(distance_x - origin_x);
  if (height) *height = fabs(distance_y - origin_y);
  wdCanvas2World(0, font_ascent, 0, &distance_y);
  if (ascent) *ascent = fabs(distance_y - origin_y);
  wdCanvas2World(0, font_descent, 0, &distance_y);
  if (descent) *descent = fabs(distance_y - origin_y);
}

/*
%F Obt'em a caixa envolvente de um string.
*/
void wdTextSize(char *s, double *width, double *height)
{
  int text_width, text_height;
  double origin_x, origin_y;
  double text_x, text_y;
  _wdCanvas2World(0, 0, origin_x, origin_y);
  cdTextSize(s, &text_width, &text_height);
  _wdCanvas2World(text_width, text_height, text_x, text_y);
  if (width) *width = fabs(text_x - origin_x);
  if (height) *height = fabs(text_y - origin_y);
}

void wdTextBox(double x, double y, char *s, double *xmin, double *xmax, double *ymin, double *ymax)
{
  int rx, ry, rxmin, rxmax, rymin, rymax;

  _wdWorld2Canvas(x, y, rx, ry);
  cdTextBox(rx, ry, s, &rxmin, &rxmax, &rymin, &rymax);

  wdCanvas2World (rxmin, rymin, xmin, ymin);
  wdCanvas2World (rxmax, rymax, xmax, ymax);
}

void wdTextBounds(double x, double y, char *s, double *rect)
{
  int rx, ry, rrect[8];

  _wdWorld2Canvas(x, y, rx, ry);
  cdTextBounds(rx, ry, s, rrect);

  wdCanvas2World (rrect[0], rrect[1], &rect[0], &rect[1]);
  wdCanvas2World (rrect[2], rrect[3], &rect[2], &rect[3]);
  wdCanvas2World (rrect[4], rrect[5], &rect[4], &rect[5]);
  wdCanvas2World (rrect[6], rrect[7], &rect[6], &rect[7]);
}

void wdPattern(int w, int h, long int *color, double w_mm, double h_mm)
{
  long int *pattern = 0;
  int wpxl, hpxl, x, y;
  int wratio, hratio;

  cdMM2Pixel(w_mm, h_mm, &wpxl, &hpxl);

  wratio = (int)(((double)(wpxl) / (double)(w)) + 0.5);
  hratio = (int)(((double)(hpxl) / (double)(h)) + 0.5);

  wratio = (wratio <= 0)? 1: wratio;
  hratio = (hratio <= 0)? 1: hratio;

  wpxl = wratio * w;
  hpxl = hratio * h;

  pattern = (long int*)malloc(wpxl*hpxl*sizeof(long int));

  for (y=0; y<hpxl; y++)
  {
    for (x=0; x<wpxl; x++)
    {
      pattern[x + y*wpxl] = color[x/wratio + (y/hratio)*w];
    }
  }

  cdPattern(wpxl, hpxl, pattern);

  free(pattern);
}

void wdStipple(int w, int h, unsigned char *fgbg, double w_mm, double h_mm)
{
  unsigned char *stipple = 0;
  int wpxl, hpxl, x, y;
  int wratio, hratio;

  cdMM2Pixel(w_mm, h_mm, &wpxl, &hpxl);

  wratio = (int)(((double)(wpxl) / (double)(w)) + 0.5);
  hratio = (int)(((double)(hpxl) / (double)(h)) + 0.5);

  wratio = (wratio <= 0)? 1: wratio;
  hratio = (hratio <= 0)? 1: hratio;

  wpxl = wratio * w;
  hpxl = hratio * h;

  stipple = (unsigned char*)malloc(wpxl*hpxl); 

  for (y=0; y<hpxl; y++)
  {
    for (x=0; x<wpxl; x++)
    {
      stipple[x + y*wpxl] = fgbg[x/wratio + (y/hratio)*w];
    }
  }

  cdStipple(wpxl, hpxl, stipple);

  free(stipple);
}

