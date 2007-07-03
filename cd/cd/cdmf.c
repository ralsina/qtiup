/*
 Canvas Draw - CD_MF Driver
*/

#include <stdio.h> 
#include <stdlib.h> 
#include <string.h> 
#include <limits.h> 

#include "cd.h"
#include "wd.h"
#include "cdprivat.h"
#include "cdmf.h"
#include "cdmfpriv.h"


/* codes for the primitives and attributes in the metafile */
enum 
{
  CDMF_FLUSH,                    /*  0 */
  CDMF_CLEAR,                    /*  1 */
  CDMF_CLIP,                     /*  2 */
  CDMF_CLIPAREA,                 /*  3 */
  CDMF_LINE,                     /*  4 */
  CDMF_BOX,                      /*  5 */
  CDMF_ARC,                      /*  6 */
  CDMF_SECTOR,                   /*  7 */
  CDMF_TEXT,                     /*  8 */
  CDMF_BEGIN,                    /*  9 */
  CDMF_VERTEX,                   /* 10 */
  CDMF_END,                      /* 11 */
  CDMF_MARK,                     /* 12 */
  CDMF_BACKOPACITY,              /* 13 */
  CDMF_WRITEMODE,                /* 14 */
  CDMF_LINESTYLE,                /* 15 */
  CDMF_LINEWIDTH,                /* 16 */
  CDMF_INTERIORSTYLE,            /* 17 */
  CDMF_HATCH,                    /* 18 */
  CDMF_STIPPLE,                  /* 19 */
  CDMF_PATTERN,                  /* 20 */
  CDMF_FONT,                     /* 21 */
  CDMF_NATIVEFONT,               /* 22 */
  CDMF_TEXTALIGNMENT,            /* 23 */
  CDMF_MARKTYPE,                 /* 24 */
  CDMF_MARKSIZE,                 /* 25 */
  CDMF_PALETTE,                  /* 26 */
  CDMF_BACKGROUND,               /* 27 */
  CDMF_FOREGROUND,               /* 28 */
  CDMF_PUTIMAGERGB,              /* 29 */
  CDMF_PUTIMAGEMAP,              /* 30 */
  CDMF_PIXEL,                    /* 31 */
  CDMF_SCROLLAREA,               /* 32 */
  CDMF_TEXTORIENTATION,          /* 33 */
  CDMF_RECT,                     /* 34 */
  CDMF_PUTIMAGERGBA,             /* 35 */
  CDMF_WLINE,                    /* 36 */
  CDMF_WRECT,                    /* 37 */
  CDMF_WBOX,                     /* 38 */
  CDMF_WARC,                     /* 39 */
  CDMF_WSECTOR,                  /* 40 */
  CDMF_WTEXT,                    /* 41 */
  CDMF_WVERTEX,                  /* 42 */
  CDMF_WMARK,                    /* 43 */
  CDMF_VECTORTEXT,               /* 44 */
  CDMF_MULTILINEVECTORTEXT,      /* 45 */
  CDMF_WVECTORTEXT,              /* 46 */
  CDMF_WMULTILINEVECTORTEXT,     /* 47 */
  CDMF_WINDOW,                   /* 48 */
  CDMF_WCLIPAREA,                /* 49 */
  CDMF_VECTORFONT,               /* 50 */
  CDMF_VECTORTEXTDIRECTION,      /* 51 */
  CDMF_VECTORTEXTTRANSFORM,      /* 52 */
  CDMF_VECTORTEXTSIZE,           /* 53 */
  CDMF_VECTORCHARSIZE,           /* 54 */
  CDMF_WVECTORTEXTDIRECTION,     /* 55 */
  CDMF_WVECTORTEXTSIZE,          /* 56 */
  CDMF_WVECTORCHARSIZE,          /* 57 */
  CDMF_FILLMODE,                 /* 58 */
  CDMF_LINESTYLEDASHES,          /* 59 */
  CDMF_LINECAP,                  /* 60 */
  CDMF_LINEJOIN,                 /* 61 */
  CDMF_CHORD,                    /* 62 */
  CDMF_WCHORD                    /* 63 */
};                                  
                                    

static cdCanvasMF *mfActiveCanvas = NULL;

void cdMFKillCanvas(void *canvas)
{
  cdCanvasMF *cnv = (cdCanvasMF *)canvas;
  fclose(cnv->Stream);
  free(cnv);
}

static int cdactivate(void *canvas)
{
  mfActiveCanvas = (cdCanvasMF *)canvas;
  return CD_OK;
}

static void cddeactivate(void)
{
  fflush(mfActiveCanvas->Stream);
  mfActiveCanvas = NULL;
}

static void cdflush(void)
{
  fprintf(mfActiveCanvas->Stream, "%d\n", CDMF_FLUSH);
}

static void cdclear(void)
{
  fprintf(mfActiveCanvas->Stream, "%d\n", CDMF_CLEAR);
}

static int cdclip(int mode)
{
  fprintf(mfActiveCanvas->Stream, "%d %d\n", CDMF_CLIP, mode);
  return mode;
}

static void cdcliparea(int* xmin, int* xmax, int* ymin, int* ymax)
{
  fprintf(mfActiveCanvas->Stream, "%d %d %d %d %d\n", CDMF_CLIPAREA, *xmin, *xmax, *ymin, *ymax);
}

static void wdcliparea(double *xmin, double *xmax, double *ymin, double *ymax)
{
  fprintf(mfActiveCanvas->Stream, "%d %g %g %g %g\n", CDMF_WCLIPAREA, *xmin, *xmax, *ymin, *ymax);
}

static void cdline(int x1, int y1, int x2, int y2)
{
  fprintf(mfActiveCanvas->Stream, "%d %d %d %d %d\n", CDMF_LINE, x1, y1, x2, y2);
}

static void wdline(double x1, double y1, double x2, double y2)
{
  fprintf(mfActiveCanvas->Stream, "%d %g %g %g %g\n", CDMF_WLINE, x1, y1, x2, y2);
}

static void cdrect(int xmin, int xmax, int ymin, int ymax)
{
  fprintf(mfActiveCanvas->Stream, "%d %d %d %d %d\n", CDMF_RECT, xmin, xmax, ymin, ymax);
}

static void wdrect(double xmin, double xmax, double ymin, double ymax)
{
  fprintf(mfActiveCanvas->Stream, "%d %g %g %g %g\n", CDMF_WRECT, xmin, xmax, ymin, ymax);
}

static void cdbox(int xmin, int xmax, int ymin, int ymax)
{
  fprintf(mfActiveCanvas->Stream, "%d %d %d %d %d\n", CDMF_BOX, xmin, xmax, ymin, ymax);
}

static void wdbox(double xmin, double xmax, double ymin, double ymax)
{
  fprintf(mfActiveCanvas->Stream, "%d %g %g %g %g\n", CDMF_WBOX, xmin, xmax, ymin, ymax);
}

static void cdarc(int xc, int yc, int w, int h, double a1, double a2)
{
  fprintf(mfActiveCanvas->Stream, "%d %d %d %d %d %g %g\n", CDMF_ARC, xc, yc, w, h, a1, a2);
}

static void wdarc(double xc, double yc, double w, double h, double a1, double a2)
{
  fprintf(mfActiveCanvas->Stream, "%d %g %g %g %g %g %g\n", CDMF_WARC, xc, yc, w, h, a1, a2);
}

static void cdsector(int xc, int yc, int w, int h, double a1, double a2)
{
  fprintf(mfActiveCanvas->Stream, "%d %d %d %d %d %g %g\n", CDMF_SECTOR, xc, yc, w, h, a1, a2);
}

static void wdsector(double xc, double yc, double w, double h, double a1, double a2)
{
  fprintf(mfActiveCanvas->Stream, "%d %g %g %g %g %g %g\n", CDMF_WSECTOR, xc, yc, w, h, a1, a2);
}

static void cdchord(int xc, int yc, int w, int h, double a1, double a2)
{
  fprintf(mfActiveCanvas->Stream, "%d %d %d %d %d %g %g\n", CDMF_CHORD, xc, yc, w, h, a1, a2);
}

static void wdchord(double xc, double yc, double w, double h, double a1, double a2)
{
  fprintf(mfActiveCanvas->Stream, "%d %g %g %g %g %g %g\n", CDMF_WCHORD, xc, yc, w, h, a1, a2);
}

static void cdtext(int x, int y, char *text)
{
  fprintf(mfActiveCanvas->Stream, "%d %d %d %s\n", CDMF_TEXT, x, y, text);
}

static void wdtext(double x, double y, char *text)
{
  fprintf(mfActiveCanvas->Stream, "%d %g %g %s\n", CDMF_WTEXT, x, y, text);
}

static void cdpoly(int mode, cdPoint* poly, int n)
{
  int i;

  if (mode == CD_FILL && mfActiveCanvas->cnv->fill_mode != mfActiveCanvas->last_fill_mode)
  {
    fprintf(mfActiveCanvas->Stream, "%d %d\n", CDMF_FILLMODE, mfActiveCanvas->cnv->fill_mode);
    mfActiveCanvas->last_fill_mode = mfActiveCanvas->cnv->fill_mode;
  }

  fprintf(mfActiveCanvas->Stream, "%d %d\n", CDMF_BEGIN, mode);

  for(i = 0; i<n; i++)
    fprintf(mfActiveCanvas->Stream, "%d %d %d\n", CDMF_VERTEX, poly[i].x, poly[i].y);

  fprintf(mfActiveCanvas->Stream, "%d\n", CDMF_END);
}

static void wdpoly(int mode, wdPoint* poly, int n)
{
  int i;

  if (mode == CD_FILL && mfActiveCanvas->cnv->fill_mode != mfActiveCanvas->last_fill_mode)
  {
    fprintf(mfActiveCanvas->Stream, "%d %d\n", CDMF_FILLMODE, mfActiveCanvas->cnv->fill_mode);
    mfActiveCanvas->last_fill_mode = mfActiveCanvas->cnv->fill_mode;
  }

  fprintf(mfActiveCanvas->Stream, "%d %d\n", CDMF_BEGIN, mode);

  for(i = 0; i<n; i++)
    fprintf(mfActiveCanvas->Stream, "%d %g %g\n", CDMF_WVERTEX, poly[i].x, poly[i].y);

  fprintf(mfActiveCanvas->Stream, "%d\n", CDMF_END);
}

static int cdbackopacity(int opacity)
{
  fprintf(mfActiveCanvas->Stream, "%d %d\n", CDMF_BACKOPACITY, opacity);
  return opacity;
}

static int cdwritemode(int mode)
{
  fprintf(mfActiveCanvas->Stream, "%d %d\n", CDMF_WRITEMODE, mode);
  return mode;
}

static int cdlinestyle(int style)
{
  if (style == CD_CUSTOM && mfActiveCanvas->cnv->line_style != mfActiveCanvas->last_line_style)
  {
    int i;
    fprintf(mfActiveCanvas->Stream, "%d %d", CDMF_LINESTYLEDASHES, mfActiveCanvas->cnv->line_dashes_count);
    for (i = 0; i < mfActiveCanvas->cnv->line_dashes_count; i++)
      fprintf(mfActiveCanvas->Stream, " %d", mfActiveCanvas->cnv->line_dashes[i]);
    fprintf(mfActiveCanvas->Stream, "\n");
    mfActiveCanvas->last_line_style = mfActiveCanvas->cnv->line_style;
  }

  fprintf(mfActiveCanvas->Stream, "%d %d\n", CDMF_LINESTYLE, style);
  return style;
}

static int cdlinewidth(int width)
{
  fprintf(mfActiveCanvas->Stream, "%d %d\n", CDMF_LINEWIDTH, width);
  return width;
}

static int cdlinecap(int cap)
{
  fprintf(mfActiveCanvas->Stream, "%d %d\n", CDMF_LINECAP, cap);
  return cap;
}

static int cdlinejoin(int join)
{
  fprintf(mfActiveCanvas->Stream, "%d %d\n", CDMF_LINEJOIN, join);
  return join;
}

static int cdmarktype(int type)
{
  fprintf(mfActiveCanvas->Stream, "%d %d\n", CDMF_MARKTYPE, type);
  return type;
}

static int cdmarksize(int size)
{
  fprintf(mfActiveCanvas->Stream, "%d %d\n", CDMF_MARKSIZE, size);
  return size;
}

static int cdinteriorstyle (int style)
{
  fprintf(mfActiveCanvas->Stream, "%d %d\n", CDMF_INTERIORSTYLE, style);
  return style;
}

static int cdhatch(int style)
{
  fprintf(mfActiveCanvas->Stream, "%d %d\n", CDMF_HATCH, style);
  return style;
}

static void cdstipple(int w, int h, unsigned char *stipple)
{
  int c, t;

  fprintf(mfActiveCanvas->Stream, "%d %d %d\n", CDMF_STIPPLE, w, h);

  t = w * h;

  for (c = 0; c < t; c++)
  {
    fprintf(mfActiveCanvas->Stream, "%d ", (int)*stipple++);
    if ((c + 1) % w == 0)
      fprintf(mfActiveCanvas->Stream, "\n");
  }
}

static void cdpattern(int w, int h, long int *pattern)
{
  int c, t;
  unsigned char r, g, b;

  fprintf(mfActiveCanvas->Stream, "%d %d %d\n", CDMF_PATTERN, w, h);

  t = w * h;

  /* stores the pattern with separeted RGB values */
  for (c = 0; c < t; c++)
  {
    cdDecodeColor(*pattern++, &r, &g, &b);
    fprintf(mfActiveCanvas->Stream, "%d %d %d ", (int)r, (int)g, (int)b);
    if (c % w == 0)
      fprintf(mfActiveCanvas->Stream, "\n");
  }
}

static void cdfont(int* type_face, int* style, int* size)
{
  fprintf(mfActiveCanvas->Stream, "%d %d %d %d\n", CDMF_FONT, *type_face, *style, *size);
}

static void cdnativefont(char* font)
{
  fprintf(mfActiveCanvas->Stream, "%d %s\n", CDMF_NATIVEFONT, font);
}

static int cdtextalignment(int alignment)
{
  fprintf(mfActiveCanvas->Stream, "%d %d\n", CDMF_TEXTALIGNMENT, alignment);
  return alignment;
}

static double cdtextorientation(double angle)
{
  fprintf(mfActiveCanvas->Stream, "%d %g\n", CDMF_TEXTORIENTATION, angle);
  return angle;
}

static void cdmark(int x, int y)
{
  fprintf(mfActiveCanvas->Stream, "%d %d %d\n", CDMF_MARK, x, y);
}

static void wdmark(double x, double y)
{
  fprintf(mfActiveCanvas->Stream, "%d %g %g\n", CDMF_WMARK, x, y);
}

static void cdpalette(int n, long int *palette, int mode)
{
  int c;
  unsigned char r, g, b;

  fprintf(mfActiveCanvas->Stream, "%d %d %d\n", CDMF_PALETTE, n, mode);

  for (c = 0; c < n; c++)
  {
    cdDecodeColor(*palette++, &r, &g, &b);
    fprintf(mfActiveCanvas->Stream, "%d %d %d\n", (int)r, (int)g, (int)b);
  }
}

static long cdbackground(long int color)
{
  unsigned char r, g, b;
  cdDecodeColor(color, &r, &g, &b);
  fprintf(mfActiveCanvas->Stream, "%d %d %d %d\n", CDMF_BACKGROUND, (int)r, (int)g, (int)b);
  return color;
}

static long cdforeground(long int color)
{
  unsigned char r, g, b;
  cdDecodeColor(color, &r, &g, &b);
	fprintf(mfActiveCanvas->Stream, "%d %d %d %d\n", CDMF_FOREGROUND, (int)r, (int)g, (int)b);
  return color;
}

static void cdputimagerectrgb(int iw, int ih, unsigned char *r, unsigned char *g, unsigned char *b, int x, int y, int w, int h, int xmin, int xmax, int ymin, int ymax)
{
  int c, l, offset;

  fprintf(mfActiveCanvas->Stream, "%d %d %d %d %d %d %d\n", CDMF_PUTIMAGERGB, iw, ih, x, y, w, h);

  r += xmin;
  g += xmin;
  b += xmin;

  offset = iw - (xmax-xmin+1);

  for (l = ymin; l <= ymax; l++)
  {
    for (c = xmin; c <= xmax; c++)
    {
      fprintf(mfActiveCanvas->Stream, "%d %d %d ", (int)*r++, (int)*g++, (int)*b++);
    }

    r += offset;
    g += offset;
    b += offset;

    fprintf(mfActiveCanvas->Stream, "\n");
  }
}

static void cdputimagerectrgba(int iw, int ih, unsigned char *r, unsigned char *g, unsigned char *b, unsigned char *a, int x, int y, int w, int h, int xmin, int xmax, int ymin, int ymax)
{
  int c, l, offset;

  fprintf(mfActiveCanvas->Stream, "%d %d %d %d %d %d %d\n", CDMF_PUTIMAGERGBA, iw, ih, x, y, w, h);

  r += xmin;
  g += xmin;
  b += xmin;
  a += xmin;

  offset = iw - (xmax-xmin+1);

  for (l = ymin; l <= ymax; l++)
  {
    for (c = xmin; c <= xmax; c++)
    {
      fprintf(mfActiveCanvas->Stream, "%d %d %d %d ", (int)*r++, (int)*g++, (int)*b++, (int)*a++);
    }

    r += offset;
    g += offset;
    b += offset;
    a += offset;

    fprintf(mfActiveCanvas->Stream, "\n");
  }
}

static void cdputimagerectmap(int iw, int ih, unsigned char *index, long int *colors, int x, int y, int w, int h, int xmin, int xmax, int ymin, int ymax)
{
  int c, l, n = 0, offset;
  unsigned char r, g, b;

  fprintf(mfActiveCanvas->Stream, "%d %d %d %d %d %d %d\n", CDMF_PUTIMAGEMAP, iw, ih, x, y, w, h);

  index += xmin;
  offset = iw - (xmax-xmin+1);

  for (l = ymin; l <= ymax; l++)
  {
    for (c = xmin; c <= xmax; c++)
    {
      if (*index > n)
        n = *index;

      fprintf(mfActiveCanvas->Stream, "%d ", (int)*index++);
    }

    index += offset;

    fprintf(mfActiveCanvas->Stream, "\n");
  }

  n++;

  for (c = 0; c < n; c++)
  {
    cdDecodeColor(*colors++, &r, &g, &b);
    fprintf(mfActiveCanvas->Stream, "%d %d %d\n", (int)r, (int)g, (int)b);
  }
}

static void cdpixel(int x, int y, long int color)
{
  unsigned char r, g, b;
  cdDecodeColor(color, &r, &g, &b);
  fprintf(mfActiveCanvas->Stream, "%d %d %d %d %d %d\n", CDMF_PIXEL, x, y, (int)r, (int)g, (int)b);
}

static void cdscrollarea(int xmin,int xmax, int ymin,int ymax, int dx,int dy)
{
  fprintf(mfActiveCanvas->Stream, "%d %d %d %d %d %d %d\n", CDMF_SCROLLAREA, xmin, xmax, ymin, ymax, dx, dy);
}

static void wdmultilinevectortext( double px, double py, char* s)
{
  fprintf(mfActiveCanvas->Stream, "%d %g %g %s\n", CDMF_WMULTILINEVECTORTEXT, px, py, s);
}

static void wdvectortext(double px, double py, char* s)
{
  fprintf(mfActiveCanvas->Stream, "%d %g %g %s\n", CDMF_WVECTORTEXT, px, py, s);
}

static void cdmultilinevectortext( int px, int py, char* s)
{
  fprintf(mfActiveCanvas->Stream, "%d %d %d %s\n", CDMF_MULTILINEVECTORTEXT, px, py, s);
}

static void cdvectortext(int px, int py, char* s)
{
  fprintf(mfActiveCanvas->Stream, "%d %d %d %s\n", CDMF_VECTORTEXT, px, py, s);
}

static void wdwindow(double xmin, double xmax, double  ymin, double ymax)
{
  fprintf(mfActiveCanvas->Stream, "%d %g %g %g %g\n", CDMF_WINDOW, xmin, xmax, ymin, ymax);
}

static void wdvectortextdirection(double x1, double y1, double x2, double y2)
{
  fprintf(mfActiveCanvas->Stream, "%d %g %g %g %g\n", CDMF_WVECTORTEXTDIRECTION, x1, y1, x2, y2);
}

static void wdvectortextsize(double size_x, double size_y, char *s)
{
  fprintf(mfActiveCanvas->Stream, "%d %g %g %s\n", CDMF_WVECTORTEXTSIZE, size_x, size_y, s);
}

static void wdvectorcharsize(double size)
{
  fprintf(mfActiveCanvas->Stream, "%d %g\n", CDMF_WVECTORCHARSIZE, size);
}

static void cdvectortextdirection(int x1, int y1, int x2, int y2)
{
  fprintf(mfActiveCanvas->Stream, "%d %d %d %d %d\n", CDMF_VECTORTEXTDIRECTION, x1, y1, x2, y2);
}

static void cdvectortextsize(int size_x, int size_y, char *s)
{
  fprintf(mfActiveCanvas->Stream, "%d %d %d %s\n", CDMF_VECTORTEXTSIZE, size_x, size_y, s);
}

static void cdvectorcharsize(int size)
{
  fprintf(mfActiveCanvas->Stream, "%d %d\n", CDMF_VECTORCHARSIZE, size);
}

static void wdvectortexttransform(double* matrix)
{
  fprintf(mfActiveCanvas->Stream, "%d %g %g %g %g %g %g %g %g %g\n", CDMF_VECTORTEXTTRANSFORM, 
    matrix[0], matrix[1], matrix[2], matrix[3], matrix[4], matrix[5], matrix[6], matrix[7], matrix[8]);
}

static char* wdvectorfont(char *file)
{
  fprintf(mfActiveCanvas->Stream, "%d %s\n", CDMF_VECTORFONT, file);
  return "Unknown";
}


/**********/
/* cdPlay */
/**********/


static double factorX = 1;
static double factorY = 1;
static int offsetX = 0;
static int offsetY = 0;
static double factorS = 1;

static int sScaleX(int x)
{
  return (int)(x * factorX + offsetX + 0.5);
}

static int sScaleY(int y)
{
  return (int)(y * factorY + offsetY + 0.5);
}

static int sScaleW(int w)
{
  return (int)(w * factorX + 0.5);
}

static int sScaleH(int h)
{
  return (int)(h * factorY + 0.5);
}

static int sScaleS(int s)
{
  s = (int)(s * factorS + 0.5);
  return s == 0? 1: s;
}

typedef int(*_cdsizecb)(cdContext *driver, int w, int h, double w_mm, double h_mm);
static _cdsizecb cdsizecb = NULL;

static int cdregistercallback(int cb, int(*func)(cdContext *driver, ...))
{
  switch (cb)
  {
  case CD_SIZECB:
    cdsizecb = (_cdsizecb)func;
    return CD_OK;
  }

  return CD_ERROR;
}

static int cdplay(int xmin, int xmax, int ymin, int ymax, void *cnv)
{
  char* filename = (char*)cnv;
  FILE* Stream;
  char TextBuffer[512];
  int iparam1, iparam2, iparam3, iparam4, iparam5, iparam6, iparam7, iparam8, iparam9, iparam10;
  int c, t, n, w, h, func;
  double dparam1, dparam2, dparam3, dparam4, dparam5, dparam6;
  unsigned char* stipple, * _stipple, *red, *green, *blue, *_red, *_green, *_blue, *index, *_index, *_alpha, *alpha;
  long int *pattern, *palette, *_pattern, *_palette, *colors, *_colors;
  int* dashes;
  double matrix[9];
  
  Stream = fopen(filename, "r");
  if (!Stream)
    return CD_ERROR;

  func = -1;
  w = 0;
  h = 0;

  factorX = 1;
  factorY = 1;
  offsetX = xmin;
  offsetY = ymin;
  factorS = 1;

  fscanf(Stream, "%s %d %d", TextBuffer, &w, &h);

  if (strcmp(TextBuffer, "CDMF") != 0)
  {
    fclose(Stream);
    return CD_ERROR;
  }

  if (w != 0 && h != 0 && xmax != 0 && ymax != 0)
  {
    factorX = ((double)(xmax - xmin)) / w;
    factorY = ((double)(ymax - ymin)) / h;

    if (factorX < factorY)
      factorS = factorX;
    else
      factorS = factorY;
  }

  if (cdsizecb)
  {
    int err;
    err = cdsizecb(CD_METAFILE, w, h, w, h);
    if (err)
      return CD_ERROR;
  }

  while (1)
  {
    fscanf(Stream, "%d", &func);

    if (feof(Stream))
      break;

    switch (func)
    {
    case CDMF_FLUSH:
      cdFlush();
      break;
    case CDMF_CLEAR:
      cdClear();
      break;
    case CDMF_CLIP:
      fscanf(Stream, "%d", &iparam1);
      cdClip(iparam1);
      break;
    case CDMF_CLIPAREA:
      fscanf(Stream, "%d %d %d %d", &iparam1, &iparam2, &iparam3, &iparam4);
      cdClipArea(sScaleX(iparam1), sScaleX(iparam2), sScaleY(iparam3), sScaleY(iparam4));
      break;
    case CDMF_WCLIPAREA:
      fscanf(Stream, "%lg %lg %lg %lg", &dparam1, &dparam2, &dparam3, &dparam4);
      wdClipArea(dparam1, dparam2, dparam3, dparam4);
      break;
    case CDMF_LINE:
      fscanf(Stream, "%d %d %d %d", &iparam1, &iparam2, &iparam3, &iparam4);
      cdLine(sScaleX(iparam1), sScaleY(iparam2), sScaleX(iparam3), sScaleY(iparam4));
      break;
    case CDMF_WLINE:
      fscanf(Stream, "%lg %lg %lg %lg", &dparam1, &dparam2, &dparam3, &dparam4);
      wdLine(dparam1, dparam2, dparam3, dparam4);
      break;
    case CDMF_RECT:
      fscanf(Stream, "%d %d %d %d", &iparam1, &iparam2, &iparam3, &iparam4);
      cdRect(sScaleX(iparam1), sScaleX(iparam2), sScaleY(iparam3), sScaleY(iparam4));
      break;
    case CDMF_WRECT:
      fscanf(Stream, "%lg %lg %lg %lg", &dparam1, &dparam2, &dparam3, &dparam4);
      wdRect(dparam1, dparam2, dparam3, dparam4);
      break;
    case CDMF_BOX:
      fscanf(Stream, "%d %d %d %d", &iparam1, &iparam2, &iparam3, &iparam4);
      cdBox(sScaleX(iparam1), sScaleX(iparam2), sScaleY(iparam3), sScaleY(iparam4));
      break;
    case CDMF_WBOX:
      fscanf(Stream, "%lg %lg %lg %lg", &dparam1, &dparam2, &dparam3, &dparam4);
      wdBox(dparam1, dparam2, dparam3, dparam4);
      break;
    case CDMF_ARC:
      fscanf(Stream, "%d %d %d %d %lg %lg", &iparam1, &iparam2, &iparam3, &iparam4, &dparam1, &dparam2);
      cdArc(sScaleX(iparam1), sScaleY(iparam2), sScaleW(iparam3), sScaleH(iparam4), dparam1, dparam2);
      break;
    case CDMF_WARC:
      fscanf(Stream, "%lg %lg %lg %lg %lg %lg", &dparam1, &dparam2, &dparam3, &dparam4, &dparam5, &dparam6);
      wdArc(dparam1, dparam2, dparam3, dparam4, dparam5, dparam6);
      break;
    case CDMF_SECTOR:
      fscanf(Stream, "%d %d %d %d %lg %lg", &iparam1, &iparam2, &iparam3, &iparam4, &dparam1, &dparam2);
      cdSector(sScaleX(iparam1), sScaleY(iparam2), sScaleW(iparam3), sScaleH(iparam4), dparam1, dparam2);
      break;
    case CDMF_WSECTOR:
      fscanf(Stream, "%lg %lg %lg %lg %lg %lg", &dparam1, &dparam2, &dparam3, &dparam4, &dparam5, &dparam6);
      wdSector(dparam1, dparam2, dparam3, dparam4, dparam5, dparam6);
      break;
    case CDMF_CHORD:
      fscanf(Stream, "%d %d %d %d %lg %lg", &iparam1, &iparam2, &iparam3, &iparam4, &dparam1, &dparam2);
      cdChord(sScaleX(iparam1), sScaleY(iparam2), sScaleW(iparam3), sScaleH(iparam4), dparam1, dparam2);
      break;
    case CDMF_WCHORD:
      fscanf(Stream, "%lg %lg %lg %lg %lg %lg", &dparam1, &dparam2, &dparam3, &dparam4, &dparam5, &dparam6);
      wdChord(dparam1, dparam2, dparam3, dparam4, dparam5, dparam6);
      break;
    case CDMF_TEXT:
      fscanf(Stream, "%d %d %[^\n]", &iparam1, &iparam2, TextBuffer);
      cdText(sScaleX(iparam1), sScaleY(iparam2), TextBuffer);
      break;
    case CDMF_WTEXT:
      fscanf(Stream, "%lg %lg %[^\n]", &dparam1, &dparam2, TextBuffer);
      wdText(dparam1, dparam2, TextBuffer);
      break;
    case CDMF_BEGIN:
      fscanf(Stream, "%d", &iparam1);
      cdBegin(iparam1);
      break;
    case CDMF_VERTEX:
      fscanf(Stream, "%d %d", &iparam1, &iparam2);
      cdVertex(sScaleX(iparam1), sScaleY(iparam2));
      break;
    case CDMF_WVERTEX:
      fscanf(Stream, "%lg %lg", &dparam1, &dparam2);
      wdVertex(dparam1, dparam2);
      break;
    case CDMF_END:
      cdEnd();
      break;
    case CDMF_MARK:
      fscanf(Stream, "%d %d", &iparam1, &iparam2);
      cdMark(sScaleX(iparam1), sScaleY(iparam2));
      break;
    case CDMF_WMARK:
      fscanf(Stream, "%lg %lg", &dparam1, &dparam2);
      wdMark(dparam1, dparam2);
      break;
    case CDMF_BACKOPACITY:
      fscanf(Stream, "%d", &iparam1);
      cdBackOpacity(iparam1);
      break;
    case CDMF_WRITEMODE:
      fscanf(Stream, "%d", &iparam1);
      cdWriteMode(iparam1);
      break;
    case CDMF_LINESTYLE:
      fscanf(Stream, "%d", &iparam1);
      cdLineStyle(iparam1);
      break;
    case CDMF_LINEWIDTH:
      fscanf(Stream, "%d", &iparam1);
      cdLineWidth(sScaleS(iparam1));
      break;
    case CDMF_LINECAP:
      fscanf(Stream, "%d", &iparam1);
      cdLineCap(iparam1);
      break;
    case CDMF_LINEJOIN:
      fscanf(Stream, "%d", &iparam1);
      cdLineJoin(iparam1);
      break;
    case CDMF_LINESTYLEDASHES:
      fscanf(Stream, "%d", &iparam1);
      dashes = (int*)malloc(iparam1*sizeof(int));
      for (c = 0; c < iparam1; c++)
        fscanf(Stream, "%d", &dashes[c]);
      cdLineStyleDashes(dashes, iparam1);
      free(dashes);
      break;
    case CDMF_FILLMODE:
      fscanf(Stream, "%d", &iparam1);
      cdFillMode(iparam1);
      break;
    case CDMF_INTERIORSTYLE:
      fscanf(Stream, "%d", &iparam1);
      cdInteriorStyle(iparam1);
      break;
    case CDMF_HATCH:
      fscanf(Stream, "%d", &iparam1);
      cdHatch(iparam1);
      break;
    case CDMF_STIPPLE:
      fscanf(Stream, "%d %d", &iparam1, &iparam2);
      t = iparam1 * iparam2;
      stipple = (unsigned char*)malloc(t);
      _stipple = stipple;
      for (c = 0; c < t; c++)
      {
        fscanf(Stream, "%d", &iparam3);
        *_stipple++ = (unsigned char)iparam3;
      }
      cdStipple(iparam1, iparam2, stipple);
      free(stipple);
      break;
    case CDMF_PATTERN:
      fscanf(Stream, "%d %d", &iparam1, &iparam2);
      t = iparam1 * iparam2;
      pattern = (long int*)malloc(t * sizeof(long));
      _pattern = pattern;
      for (c = 0; c < t; c++)
      {
        fscanf(Stream, "%d %d %d", &iparam3, &iparam4, &iparam5);
        *_pattern++ = cdEncodeColor((unsigned char)iparam3, (unsigned char)iparam4, (unsigned char)iparam5);
      }
      cdPattern(iparam1, iparam2, pattern);
      free(pattern);
      break;
    case CDMF_FONT:
      fscanf(Stream, "%d %d %d", &iparam1, &iparam2, &iparam3);
      cdFont(iparam1, iparam2, iparam3);
      break;
    case CDMF_NATIVEFONT:
      fscanf(Stream, "%s", TextBuffer);
      cdNativeFont(TextBuffer);
      break;
    case CDMF_TEXTALIGNMENT:
      fscanf(Stream, "%d", &iparam1);
      cdTextAlignment(iparam1);
      break;
    case CDMF_TEXTORIENTATION:
      fscanf(Stream, "%lg", &dparam1);
      cdTextOrientation(dparam1);
      break;
    case CDMF_MARKTYPE:
      fscanf(Stream, "%d", &iparam1);
      cdMarkType(iparam1);
      break;
    case CDMF_MARKSIZE:
      fscanf(Stream, "%d", &iparam1);
      cdMarkSize(sScaleS(iparam1));
      break;
    case CDMF_PALETTE:
      fscanf(Stream, "%d %d", &iparam1, &iparam2);
      _palette = palette = (long int*)malloc(iparam1);
      for (c = 0; c < iparam1; c++)
      {
        fscanf(Stream, "%d %d %d", &iparam3, &iparam4, &iparam5);
        *_palette++ = cdEncodeColor((unsigned char)iparam3, (unsigned char)iparam4, (unsigned char)iparam5);
      }
      cdPalette(iparam1, palette, iparam2);
      free(palette);
      break;
    case CDMF_BACKGROUND:
      fscanf(Stream, "%d %d %d", &iparam1, &iparam2, &iparam3);
      cdBackground(cdEncodeColor((unsigned char)iparam1, (unsigned char)iparam2, (unsigned char)iparam3));
      break;
    case CDMF_FOREGROUND:
      fscanf(Stream, "%d %d %d", &iparam1, &iparam2, &iparam3);
      cdForeground(cdEncodeColor((unsigned char)iparam1, (unsigned char)iparam2, (unsigned char)iparam3));
      break;
    case CDMF_PUTIMAGERGB:
      fscanf(Stream, "%d %d %d %d %d %d", &iparam1, &iparam2, &iparam3, &iparam4, &iparam5, &iparam6);
      t = iparam1 * iparam2;
      _red = red = (unsigned char*) malloc(t);
      _green = green = (unsigned char*) malloc(t);
      _blue = blue = (unsigned char*) malloc(t);
      for (c = 0; c < t; c++)
      {
        fscanf(Stream, "%d %d %d", &iparam7, &iparam8, &iparam9);
        *_red++ = (unsigned char)iparam7;
        *_green++ = (unsigned char)iparam8;
        *_blue++ = (unsigned char)iparam9;
      }
      cdPutImageRGB(iparam1, iparam2, red, green, blue, sScaleX(iparam3), sScaleY(iparam4), sScaleX(iparam5), sScaleY(iparam6));
      free(red);
      free(green);
      free(blue);
      break;
    case CDMF_PUTIMAGERGBA:
      fscanf(Stream, "%d %d %d %d %d %d", &iparam1, &iparam2, &iparam3, &iparam4, &iparam5, &iparam6);
      t = iparam1 * iparam2;
      _red = red = (unsigned char*) malloc(t);
      _green = green = (unsigned char*) malloc(t);
      _blue = blue = (unsigned char*) malloc(t);
      _alpha = alpha = (unsigned char*) malloc(t);
      for (c = 0; c < t; c++)
      {
        fscanf(Stream, "%d %d %d %d", &iparam7, &iparam8, &iparam9, &iparam10);
        *_red++ = (unsigned char)iparam7;
        *_green++ = (unsigned char)iparam8;
        *_blue++ = (unsigned char)iparam9;
        *_alpha++ = (unsigned char)iparam10;
      }
      cdPutImageRGBA(iparam1, iparam2, red, green, blue, alpha, sScaleX(iparam3), sScaleY(iparam4), sScaleX(iparam5), sScaleY(iparam6));
      free(red);
      free(green);
      free(blue);
      free(alpha);
      break;
    case CDMF_PUTIMAGEMAP:
      fscanf(Stream, "%d %d %d %d %d %d", &iparam1, &iparam2, &iparam3, &iparam4, &iparam5, &iparam6);
      t = iparam1 * iparam2;
      n = 0;
      _index = index = (unsigned char*) malloc(t);
      for (c = 0; c < t; c++)
      {
        fscanf(Stream, "%d", &iparam7);
        *_index++ = (unsigned char)iparam7;
        if (iparam7 > n)
          n = iparam7;
      }
      _colors = colors = (long int*)malloc(n);
      for (c = 0; c < n; c++)
      {
        fscanf(Stream, "%d %d %d", &iparam7, &iparam8, &iparam9);
        *_colors++ = cdEncodeColor((unsigned char)iparam7, (unsigned char)iparam8, (unsigned char)iparam9);
      }
      cdPutImageMap(iparam1, iparam2, index, colors, sScaleX(iparam3), sScaleY(iparam4), sScaleX(iparam5), sScaleY(iparam6));
      free(index);
      free(colors);
      break;
    case CDMF_PIXEL:
      fscanf(Stream, "%d %d %d %d %d", &iparam1, &iparam2, &iparam3, &iparam4, &iparam5);
      cdPixel(sScaleX(iparam1), sScaleY(iparam2), cdEncodeColor((unsigned char)iparam3, (unsigned char)iparam4, (unsigned char)iparam5));
      break;
    case CDMF_SCROLLAREA:
      fscanf(Stream, "%d %d %d %d %d %d", &iparam1, &iparam2, &iparam3, &iparam4, &iparam5, &iparam6);
      cdScrollArea(sScaleX(iparam1), sScaleX(iparam2), sScaleY(iparam3), sScaleY(iparam4), sScaleX(iparam5), sScaleY(iparam6));
      break;
    case CDMF_WVECTORTEXT:
      fscanf(Stream, "%lg %lg %[^\n]", &dparam1, &dparam2, TextBuffer);
      wdVectorText(dparam1, dparam2, TextBuffer);
      break;
    case CDMF_WMULTILINEVECTORTEXT:
      fscanf(Stream, "%lg %lg %[^\n]", &dparam1, &dparam2, TextBuffer);
      wdVectorText(dparam1, dparam2, TextBuffer);
      break;
    case CDMF_VECTORTEXT:
      fscanf(Stream, "%d %d %[^\n]", &iparam1, &iparam2, TextBuffer);
      cdVectorText(iparam1, iparam2, TextBuffer);
      break;
    case CDMF_MULTILINEVECTORTEXT:
      fscanf(Stream, "%d %d %[^\n]", &iparam1, &iparam2, TextBuffer);
      cdVectorText(iparam1, iparam2, TextBuffer);
      break;
    case CDMF_WVECTORCHARSIZE:
      fscanf(Stream, "%lg", &dparam1);
      wdVectorCharSize(dparam1);
      break;
    case CDMF_WVECTORTEXTSIZE:
      fscanf(Stream, "%lg %lg %[^\n]", &dparam1, &dparam2, TextBuffer);
      wdVectorTextSize(dparam1, dparam2, TextBuffer);
      break;
    case CDMF_WVECTORTEXTDIRECTION:
      fscanf(Stream, "%lg %lg %lg %lg", &dparam1, &dparam2, &dparam3, &dparam4);
      wdVectorTextDirection(dparam1, dparam2, dparam3, dparam4);
      break;
    case CDMF_VECTORCHARSIZE:
      fscanf(Stream, "%d", &iparam1);
      cdVectorCharSize(iparam1);
      break;
    case CDMF_VECTORTEXTSIZE:
      fscanf(Stream, "%d %d %[^\n]", &iparam1, &iparam2, TextBuffer);
      cdVectorTextSize(iparam1, iparam2, TextBuffer);
      break;
    case CDMF_VECTORTEXTDIRECTION:
      fscanf(Stream, "%d %d %d %d", &iparam1, &iparam2, &iparam3, &iparam4);
      cdVectorTextDirection(iparam1, iparam2, iparam3, iparam4);
      break;
    case CDMF_VECTORFONT:
      fscanf(Stream, "%[^\n]", TextBuffer);
      cdVectorFont(TextBuffer);
      break;
    case CDMF_VECTORTEXTTRANSFORM:
      fscanf(Stream, "%lg %lg %lg %lg %lg %lg %lg %lg %lg", &matrix[0], &matrix[1], &matrix[2], &matrix[3], &matrix[4], &matrix[5], &matrix[6], &matrix[7], &matrix[8]);
      wdVectorTextTransform(matrix);
      break;
    case CDMF_WINDOW:
      fscanf(Stream, "%lg %lg %lg %lg", &dparam1, &dparam2, &dparam3, &dparam4);
      wdWindow(dparam1, dparam2, dparam3, dparam4);
      break;
    default:
      fclose(Stream);
      return CD_ERROR;
    }
  }

  fclose(Stream);

  return CD_OK;
}


/*******************/
/* Canvas Creation */
/*******************/

void cdMFCreateCanvas(cdPrivateCanvas *cnv, void *data)
{
  char filename[10240] = "";
  char* strdata = (char*)data;
  double w_mm = INT_MAX, h_mm = INT_MAX, res = 3.78;
  char* text_data = (char*)data;
  cdCanvasMF* new_mf;

  strdata += cdGetFileName(strdata, filename);
  if (filename[0] == 0)
    return;

  sscanf(text_data, "%lgx%lg %lg", &w_mm, &h_mm, &res);

  new_mf = (cdCanvasMF *)malloc(sizeof(cdCanvasMF));

  new_mf->Stream = fopen(filename, "w");
  if (!new_mf->Stream)
  {
    free(new_mf);
    return;
  }

  strcpy(new_mf->filename, filename);

  /* store the base canvas */
  new_mf->cnv = cnv;

  /* update canvas context */
  cnv->w = (int)(w_mm * res);
  cnv->h = (int)(h_mm * res);
  cnv->w_mm = w_mm;
  cnv->h_mm = h_mm;
  cnv->bpp = 24;
  cnv->xres = res;
  cnv->yres = res;
  cnv->context_canvas = new_mf;

  fprintf(new_mf->Stream, "CDMF %d %d\n", cnv->w, cnv->h);
}

void cdMFInitTable(cdPrivateCanvas* cnv)
{
  /* initialize function table*/
  cnv->Flush = cdflush;
  cnv->Clear = cdclear;
  cnv->Pixel = cdpixel;
  cnv->Mark = cdmark;
  cnv->Line = cdline;
  cnv->Poly = cdpoly;
  cnv->Rect = cdrect;
  cnv->Box = cdbox;
  cnv->Arc = cdarc;
  cnv->Sector = cdsector;
  cnv->Chord = cdchord;
  cnv->Text = cdtext;
  cnv->VectorText = cdvectortext;
  cnv->MultiLineVectorText = cdmultilinevectortext;
/*  cnv->FontDim = cdfontdim; (use font size estimator) */
/*  cnv->TextSize = cdtextsize; (use font size estimator) */
  cnv->PutImageRectRGB = cdputimagerectrgb;
  cnv->PutImageRectRGBA = cdputimagerectrgba;
  cnv->PutImageRectMap = cdputimagerectmap;
  cnv->ScrollArea = cdscrollarea;
  cnv->wMark = wdmark;
  cnv->wLine = wdline;
  cnv->wPoly = wdpoly;
  cnv->wRect = wdrect;
  cnv->wBox = wdbox;
  cnv->wArc = wdarc;
  cnv->wSector = wdsector;
  cnv->wChord = wdchord;
  cnv->wText = wdtext;
  cnv->wVectorText = wdvectortext;
  cnv->wMultiLineVectorText = wdmultilinevectortext;

  cnv->Clip = cdclip;
  cnv->ClipArea = cdcliparea;
  cnv->BackOpacity = cdbackopacity;
  cnv->WriteMode = cdwritemode;
  cnv->MarkType = cdmarktype;
  cnv->MarkSize = cdmarksize;
  cnv->LineStyle = cdlinestyle;
  cnv->LineWidth = cdlinewidth;
  cnv->LineCap = cdlinecap;
  cnv->LineJoin = cdlinejoin;
  cnv->InteriorStyle = cdinteriorstyle;
  cnv->Hatch = cdhatch;
  cnv->Stipple = cdstipple;
  cnv->Pattern = cdpattern;
  cnv->Font = cdfont;
  cnv->NativeFont = cdnativefont;
  cnv->TextAlignment = cdtextalignment;
  cnv->TextOrientation = cdtextorientation;
  cnv->Palette = cdpalette;
  cnv->Background = cdbackground;
  cnv->Foreground = cdforeground;
/*  cnv->GetImageRGB = cdgetimagergb; (can not be implemented here) */
/*  cnv->CreateImage = cdcreateimage; (can not be implemented here) */
/*  cnv->GetImage = cdgetimage; (can not be implemented here) */
/*  cnv->PutImageRect = cdputimagerect; (can not be implemented here) */
/*  cnv->KillImage = cdkillimage; (can not be implemented here) */
  cnv->Window = wdwindow;
/*  cnv->Viewport = wdViewport; (can not be implemented here) */
  cnv->wClipArea = wdcliparea;
  cnv->VectorFont = wdvectorfont;
  cnv->VectorTextTransform = wdvectortexttransform;
  cnv->wVectorTextDirection = wdvectortextdirection;
  cnv->wVectorTextSize = wdvectortextsize;
  cnv->wVectorCharSize = wdvectorcharsize;
  cnv->VectorTextDirection = cdvectortextdirection;
  cnv->VectorTextSize = cdvectortextsize;
  cnv->VectorCharSize = cdvectorcharsize;

  cnv->KillCanvas = cdMFKillCanvas;
  cnv->Activate = cdactivate;
/*  cnv->Update = cdupdate; (not necessary) */
  cnv->Deactivate = cddeactivate;
/*  cnv->wdCall = wdcall; (not necessary) */
}

static cdPrivateContext cdMetafileContextTable =
{
  CD_CAP_ALL & ~(CD_CAP_GETIMAGERGB | CD_CAP_IMAGESRV | 
                 CD_CAP_REGION | CD_CAP_FONTDIM | CD_CAP_TEXTSIZE),
  cdMFCreateCanvas,
  cdMFInitTable,
  cdplay,
  cdregistercallback
};
 

static cdContext cdMetafile =
{
  &cdMetafileContextTable
};


cdContext* cdContextMetafile(void)
{
  return &cdMetafile;
}
