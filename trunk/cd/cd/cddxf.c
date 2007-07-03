char *cddxf_c = "$Id: cddxf.c,v 1.3 2005/06/23 20:23:20 scuri Exp $";
/*
   %M Canvas Draw - CD_DXF Drawing Exchange Format
   %a Milton e Vinicius
   %d 2 2 96
   %p Sistemas Graficos
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <limits.h>
#include "cd.h"
#include "cdprivat.h"
#include "cddxf.h"

#ifndef PI
#define PI 3.14159265358979323846
#endif

#ifndef round
#define round(x)  (int)floor(x + 0.5)
#endif
#ifndef ignore
#define ignore(x) (void)x
#endif
#ifndef max
#define max(x, y) ((x > y)? x : y)
#endif
#ifndef min
#define min(x, y) ((x < y)? x : y)
#endif
#ifndef MAX_INT
/* #define MAX_INT   (pow(256,sizeof(int))/2)-1 */
#define MAX_INT INT_MAX
#endif

typedef struct
{
  int x,y;
} point;

typedef struct
{                   /* for precision in ellipse calculations */
  double x,y;
} dpoint;

typedef struct
{
  cdPrivateCanvas* cnv;
  FILE *arq_ptr;                  /* pointer to file                        */
  int layer;                      /* layer                                  */

  int tf;                         /* text font                              */
  double th;                      /* text height (in points)                */
  int toa;                        /* text oblique angle (for italics)       */
  int tha, tva;                   /* text horizontal and vertical alignment */

  int fgcolor;                    /* foreground AutoCAD palette color       */

  int lt;                         /* line type                              */
  double lw;                      /* line width (in milimeters)             */
} Tdxf_gc;

static Tdxf_gc *active_dxf;               /* general context */


static void wnamline (int t, Tdxf_gc *cnv)   /* write name of a line */
{

  static char *line[] =
  {"CONTINUOUS",
   "DASHED",
   "HIDDEN",
   "CENTER",
   "PHANTOM",
   "DOT",
   "DASHDOT",
   "BORDER",
   "DIVIDE"};

/*
   AutoCAD line styles ( see acad.lin ):

   0 CONTINUOUS  ____________________________________________
   1 DASHED      __ __ __ __ __ __ __ __ __ __ __ __ __ __ __
   2 HIDDEN      _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _
   3 CENTER      ____ _ ____ _ ____ _ ____ _ ____ _ ____ _ __
   4 PHANTOM     _____ _ _ _____ _ _ _____ _ _ _____ _ _ ____
   5 DOT         ............................................
   6 DASHDOT     __ . __ . __ . __ . __ . __ . __ . __ . __ .
   7 BORDER      __ __ . __ __ . __ __ . __ __ . __ __ . __ _
   8 DIVIDE      __ . . __ . . __ . . __ . . __ . . __ . . __

*/

  fprintf (cnv->arq_ptr, "%s\n", line[t]);
}


static void wnamfont (int t, Tdxf_gc *cnv)   /* write name of a font */
{
  static char *font[] =
  {
    "STANDARD",
    "ROMAN",
    "ROMAN_BOLD",
    "ROMANTIC",
    "ROMANTIC_BOLD",
    "SANSSERIF",
    "SANSSERIF_BOLD",
  };
/*
             CD Fonts / Style                 AutoCAD Fonts
      -------------------------------------------------------------------
       CD_SYSTEM                           0 STANDARD
       CD_COURIER     / CD_PLAIN           1 ROMAN
       CD_COURIER     / CD_BOLD            2 ROMAN_BOLD
       CD_COURIER     / CD_ITALIC          1 ROMAN         (active_dxf->toa = 15)
       CD_COURIER     / CD_BOLD_ITALIC     2 ROMAN_BOLD    (active_dxf->toa = 15)
       CD_TIMES_ROMAN / CD_PLAIN           3 ROMANTIC
       CD_TIMES_ROMAN / CD_BOLD            4 ROMANTIC_BOLD
       CD_TIMES_ROMAN / CD_ITALIC          3 ROMANTIC      (active_dxf->toa = 15)
       CD_TIMES_ROMAN / CD_BOLD_ITALIC     4 ROMANTIC_BOLD (active_dxf->toa = 15)
       CD_HELVETICA   / CD_PLAIN           5 SANSSERIF
       CD_HELVETICA   / CD_BOLD            6 SANSSERIF_BOLD
       CD_HELVETICA   / CD_ITALIC          5 SANSSERIF     (active_dxf->toa = 15)
       CD_HELVETICA   / CD_BOLD_ITALIC     6 SANSSERIF_BOLD(active_dxf->toa = 15)
*/

  fprintf (cnv->arq_ptr, "%s\n", font[t]);
}


static void writepoly (Tdxf_gc *cnv, point *v, int nv) /* write polygon */
{
  int i;

  fprintf ( cnv->arq_ptr, "0\n" );
  fprintf ( cnv->arq_ptr, "POLYLINE\n" );
  fprintf ( cnv->arq_ptr, "8\n" );
  fprintf ( cnv->arq_ptr, "%d\n", cnv->layer); /* current layer */

  fprintf ( cnv->arq_ptr, "6\n" );
  wnamline( cnv->lt, cnv );                    /* line type */

  fprintf ( cnv->arq_ptr, "62\n" );
  fprintf ( cnv->arq_ptr, "%d\n", cnv->fgcolor );
  fprintf ( cnv->arq_ptr, "66\n" );
  fprintf ( cnv->arq_ptr, "1\n" );
  fprintf ( cnv->arq_ptr, "40\n" );
  fprintf ( cnv->arq_ptr, "%f\n", cnv->lw/cnv->cnv->xres );
  fprintf ( cnv->arq_ptr, "41\n" );           /* entire polygon line width */
  fprintf ( cnv->arq_ptr, "%f\n", cnv->lw/cnv->cnv->xres );
  for ( i=0; i<nv; i++ )
  {
    fprintf ( cnv->arq_ptr, "0\n" );
    fprintf ( cnv->arq_ptr, "VERTEX\n" );
    fprintf ( cnv->arq_ptr, "8\n" );
    fprintf ( cnv->arq_ptr, "%d\n", cnv->layer); /* current layer */
    fprintf ( cnv->arq_ptr, "10\n" );
    fprintf ( cnv->arq_ptr, "%f\n", v[i].x/cnv->cnv->xres );
    fprintf ( cnv->arq_ptr, "20\n" );
    fprintf ( cnv->arq_ptr, "%f\n", v[i].y/cnv->cnv->xres );
  }
  fprintf ( cnv->arq_ptr, "0\n" );
  fprintf ( cnv->arq_ptr, "SEQEND\n" );
}

static void deflines (Tdxf_gc *cnv)    /* define lines */
{
  int i, j;
  static char *line[] =
  {"Solid line",
   "Dashed line",
   "Hidden line",
   "Center line",
   "Phantom line",
   "Dot line",
   "Dashdot line",
   "Border line",
   "Divide Line"};

#define TABSIZE (sizeof(tab)/sizeof(tab[0]))

  static int tab[][8] =
  {
    { 0,  0, 0 ,  0,  0,  0, 0,  0 },
    { 2, 15, 10, -5,  0,  0, 0,  0 },
    { 2, 10, 5 , -5,  0,  0, 0,  0 },
    { 4, 35, 20, -5,  5, -5, 0,  0 },
    { 6, 50, 25, -5,  5, -5, 5, -5 },
    { 2,  5, 0 , -5,  0,  0, 0,  0 },
    { 4, 20, 10, -5,  0, -5, 0,  0 },
    { 6, 35, 10, -5, 10, -5, 0, -5 },
    { 6, 25, 10, -5,  0, -5, 0, -5 }
  };

  fprintf (cnv->arq_ptr, "0\n");
  fprintf (cnv->arq_ptr, "TABLE\n");
  fprintf (cnv->arq_ptr, "2\n");
  fprintf (cnv->arq_ptr, "LTYPE\n");
  fprintf (cnv->arq_ptr, "70\n");
  fprintf (cnv->arq_ptr, "5\n");
  for (j = 0; j < TABSIZE; j++)
  {
    fprintf (cnv->arq_ptr, "0\n");
    fprintf (cnv->arq_ptr, "LTYPE\n");
    fprintf (cnv->arq_ptr, "2\n");

    wnamline (j, cnv);                            /* line style */

    fprintf (cnv->arq_ptr, "70\n");
    fprintf (cnv->arq_ptr, "64\n");
    fprintf (cnv->arq_ptr, "3\n");
    fprintf (cnv->arq_ptr, "%s\n", line[j]);      /* line style */
    fprintf (cnv->arq_ptr, "72\n");
    fprintf (cnv->arq_ptr, "65\n");
    fprintf (cnv->arq_ptr, "73\n");
    fprintf (cnv->arq_ptr, "%d\n", tab[j][0]);    /* number of parameters */
    fprintf (cnv->arq_ptr, "40\n");
    fprintf (cnv->arq_ptr, "%d\n", tab[j][1]);
    for (i = 2; i < 2 + tab[j][0]; i++)
    {
      fprintf (cnv->arq_ptr, "49\n");
      fprintf (cnv->arq_ptr, "%d\n", tab[j][i]);  /* parameters */
    }
  }
  fprintf (cnv->arq_ptr, "0\n");
  fprintf (cnv->arq_ptr, "ENDTAB\n");
}


static void deffonts (Tdxf_gc *cnv)    /* define fonts */
{
  int i;
  static char *font[] =
  {
    "romanc.shx"  ,
    "romant.shx"  ,
    "rom_____.pfb",
    "romb____.pfb",
    "sas_____.pfb",
    "sasb____.pfb"
  };

  fprintf (cnv->arq_ptr, "0\n");
  fprintf (cnv->arq_ptr, "TABLE\n");
  fprintf (cnv->arq_ptr, "2\n");
  fprintf (cnv->arq_ptr, "STYLE\n");
  fprintf (cnv->arq_ptr, "70\n");
  fprintf (cnv->arq_ptr, "5\n");
  for (i = 1; i < 7; i++)
  {
    fprintf (cnv->arq_ptr, "0\n");
    fprintf (cnv->arq_ptr, "STYLE\n");
    fprintf (cnv->arq_ptr, "2\n");

    wnamfont (i, cnv);                            /* font style name */

    fprintf (cnv->arq_ptr, "3\n");
    fprintf (cnv->arq_ptr, "%s\n", font[i-1]);    /* font style file */
    fprintf (cnv->arq_ptr, "70\n");
    fprintf (cnv->arq_ptr, "64\n");
    fprintf (cnv->arq_ptr, "71\n");
    fprintf (cnv->arq_ptr, "0\n");
    fprintf (cnv->arq_ptr, "40\n");
    fprintf (cnv->arq_ptr, "0\n");
    fprintf (cnv->arq_ptr, "41\n");
    fprintf (cnv->arq_ptr, "1\n");
    fprintf (cnv->arq_ptr, "42\n");
    fprintf (cnv->arq_ptr, "0\n");
    fprintf (cnv->arq_ptr, "50\n");
    fprintf (cnv->arq_ptr, "0\n");
  }
  fprintf (cnv->arq_ptr, "0\n");
  fprintf (cnv->arq_ptr, "ENDTAB\n");
}

static int cdactivate(void *cnv)
{
  active_dxf = (Tdxf_gc *) cnv;
  return CD_OK;
}

static void cddeactivate (void)
{
  fflush (active_dxf->arq_ptr);               /* flush file */
  active_dxf = NULL;
}

static void cdkillcanvas(void *canvas)
{
  Tdxf_gc *cnv;
  cnv = (Tdxf_gc *)canvas;

  fprintf (cnv->arq_ptr, "0\n");
  fprintf (cnv->arq_ptr, "ENDSEC\n");
  fprintf (cnv->arq_ptr, "0\n");
  fprintf (cnv->arq_ptr, "EOF\n");     /* fputs eof */
  fprintf (cnv->arq_ptr, " \n");

  fflush (cnv->arq_ptr);               /* flush file */
  fclose (cnv->arq_ptr);

  free (cnv);
}

static void cdflush (void)
{
  active_dxf->layer++;
}

/*==========================================================================*/
/* Primitives                                                               */
/*==========================================================================*/
static void cdpoly(int mode, cdPoint* poly, int n)
{
  if (mode == CD_CLOSED_LINES || mode == CD_FILL)
  {
    poly[n].x = poly[0].x;
    poly[n].y = poly[0].y;
    n++;
  }

  writepoly (active_dxf, (point*)poly, n);                /* write polygon */
}

static void cdline (int x1, int y1, int x2, int y2)
{
  point v[2];                     /* uses new array of points to avoid      */

  v[0].x = x1;                    /* starting point */
  v[0].y = y1;
  v[1].x = x2;                    /* ending point   */
  v[1].y = y2;
  writepoly (active_dxf, v, 2);              /* draw line as a polygon */
}

static void cdboxrect(int xmin, int xmax, int ymin, int ymax)
{
  point v[5];                     /* uses new array of points to avoid      */

  v[0].x = xmin;
  v[0].y = ymin;
  v[1].x = xmin;
  v[1].y = ymax;
  v[2].x = xmax;                  /* box edges */
  v[2].y = ymax;
  v[3].x = xmax;
  v[3].y = ymin;
  v[4].x = xmin;
  v[4].y = ymin;
  writepoly (active_dxf, v, 5);              /* draw box as a polygon */
}

/*--------------------------------------------------------------------------*/
/* gives radius of the circle most resembling elliptic arc at angle t       */
/*--------------------------------------------------------------------------*/
static double calc_radius (double a, double b, double t)
{
  return (pow ((a*a*sin(t)*sin(t) + b*b*cos(t)*cos(t)), 1.5))/(a*b);
}

/*--------------------------------------------------------------------------*/
/* calculates bulge for a given circular arc segment (between points p1 and */
/* p2, with radius r). Bulge is the tangent of 1/4 the angle theta of the   */
/* arc segment(a bulge of 1 is a semicircle, which has an angle of 180 deg) */
/*--------------------------------------------------------------------------*/
static double calc_bulge (double a, double b, double t1, double t2)
{
  dpoint p1, p2;          /* initial and ending arc points                 */
  double r;               /* radius most resembling arc at angle (t1+t2)/2 */
  double theta;           /* angle of circular arc segment                 */
  double sin_theta;       /* sine of theta                                 */
  double dist_x;          /* distance between two points along the x axis  */
  double dist_y;          /* distance between two points along the y axis  */
  double halfdist;        /* half distance between two points              */

  p1.x = a*cos(t1);
  p1.y = b*sin(t1);
  p2.x = a*cos(t2);
  p2.y = b*sin(t2);
  r    = calc_radius (a, b, (t1+t2)/2);

  dist_x      = p2.x - p1.x;
  dist_y      = p2.y - p1.y;
  halfdist    = (sqrt (dist_x*dist_x + dist_y*dist_y))/2;
  sin_theta   = halfdist/r;
  if (sin_theta > 1)  sin_theta = 1;
  theta       = 2*asin(sin_theta);

  return tan(theta/4);
}

static void writevertex (int xc, int yc, double a, double b, double t,
                         double bulge)
{
  dpoint p;
  p.x = (xc + a*cos(t))/active_dxf->cnv->xres;
  p.y = (yc + b*sin(t))/active_dxf->cnv->xres;

  fprintf ( active_dxf->arq_ptr, "0\n" );
  fprintf ( active_dxf->arq_ptr, "VERTEX\n" );
  fprintf ( active_dxf->arq_ptr, "8\n" );
  fprintf ( active_dxf->arq_ptr, "%d\n", active_dxf->layer);      /* current layer */
  fprintf ( active_dxf->arq_ptr, "10\n" );
  fprintf ( active_dxf->arq_ptr, "%f\n", p.x );
  fprintf ( active_dxf->arq_ptr, "20\n" );               /* vertex coordinates     */
  fprintf ( active_dxf->arq_ptr, "%f\n", p.y );
  fprintf ( active_dxf->arq_ptr, "42\n" );               /* bulge from this vertex */
  fprintf ( active_dxf->arq_ptr, "%f\n", bulge );         /* to the next one        */
}

static void cdarc (int xc, int yc, int w, int h, double a1, double a2)
{
  double bulge;        /* bulge is the tangent of 1/4 the angle for a given */
                       /* circle arc segment (a bulge of 1 is a semicircle) */
  double t;            /* current arc angle being calculated    */
  double t1;           /* a1 in radians                         */
  double t2;           /* a2 in radians                         */
  double a;            /* half horizontal axis                  */
  double b;            /* half vertical axis                    */
  double seg_angle;    /* angle of every arc segment            */
  int nseg;            /* number of arc segments                */
  int i;

  a         = w/2;
  b         = h/2;
  t1        = a1*PI/180;                    /* a1 in radians */
  t2        = a2*PI/180;                    /* a2 in radians */
  nseg      = max(round(a2-a1)/(360/32),1); /* 32 segments in closed ellipse */
  seg_angle = (t2-t1)/nseg;

  fprintf ( active_dxf->arq_ptr, "0\n" );
  fprintf ( active_dxf->arq_ptr, "POLYLINE\n" );
  fprintf ( active_dxf->arq_ptr, "8\n" );
  fprintf ( active_dxf->arq_ptr, "%d\n", active_dxf->layer);  /* current layer */
  fprintf ( active_dxf->arq_ptr, "6\n" );
  wnamline( active_dxf->lt, active_dxf );                     /* line type */
  fprintf ( active_dxf->arq_ptr, "62\n" );
  fprintf ( active_dxf->arq_ptr, "%3d\n", active_dxf->fgcolor ); /* color */
  fprintf ( active_dxf->arq_ptr, "66\n" );
  fprintf ( active_dxf->arq_ptr, "1\n" );
  fprintf ( active_dxf->arq_ptr, "70\n" );
  fprintf ( active_dxf->arq_ptr, "128\n" );
  fprintf ( active_dxf->arq_ptr, "40\n" );
  fprintf ( active_dxf->arq_ptr, "%f\n", active_dxf->lw );
  fprintf ( active_dxf->arq_ptr, "41\n" );          /* entire arc line width */
  fprintf ( active_dxf->arq_ptr, "%f\n", active_dxf->lw );

  for (i=0, t=t1; i<nseg; i++, t+=seg_angle)
  {                                            /* calculate bulge between t */
    bulge = calc_bulge (a, b, t, t+seg_angle); /* and t+seg_angle and write */
    writevertex (xc, yc, a, b, t, bulge);      /* vertex at t               */
  }
  writevertex (xc, yc, a, b, t2, 0);     /* bulge of last vertex is useless */

  fprintf ( active_dxf->arq_ptr, "0\n" );
  fprintf ( active_dxf->arq_ptr, "SEQEND\n" );
}

static void cdsector (int xc, int yc, int w, int h, double a1, double a2)
{
  double bulge;        /* bulge is the tangent of 1/4 the angle for a given */
                       /* circle arc segment (a bulge of 1 is a semicircle) */
  double t;            /* current arc angle being calculated    */
  double t1;           /* a1 in radians                         */
  double t2;           /* a2 in radians                         */
  double a;            /* half horizontal axis                  */
  double b;            /* half vertical axis                    */
  double seg_angle;    /* angle of every arc segment            */
  int nseg;            /* number of arc segments                */
  int i;

  a         = w/2;
  b         = h/2;
  t1        = a1*PI/180;              /* a1 in radians */
  t2        = a2*PI/180;              /* a2 in radians */
  nseg      = round(a2-a1)/(360/32);  /* 32 segments in closed ellipse */
  seg_angle = (t2-t1)/nseg;

  fprintf ( active_dxf->arq_ptr, "0\n" );
  fprintf ( active_dxf->arq_ptr, "POLYLINE\n" );
  fprintf ( active_dxf->arq_ptr, "8\n" );
  fprintf ( active_dxf->arq_ptr, "%d\n", active_dxf->layer); /* current layer */
  fprintf ( active_dxf->arq_ptr, "6\n" );
  wnamline( active_dxf->lt, active_dxf );                     /* line type */
  fprintf ( active_dxf->arq_ptr, "62\n" );
  fprintf ( active_dxf->arq_ptr, "%3d\n", active_dxf->fgcolor ); /* color */
  fprintf ( active_dxf->arq_ptr, "66\n" );
  fprintf ( active_dxf->arq_ptr, "1\n" );
  fprintf ( active_dxf->arq_ptr, "70\n" );
  fprintf ( active_dxf->arq_ptr, "128\n" );
  fprintf ( active_dxf->arq_ptr, "40\n" );
  fprintf ( active_dxf->arq_ptr, "%f\n", active_dxf->lw );
  fprintf ( active_dxf->arq_ptr, "41\n" );          /* entire arc line width */
  fprintf ( active_dxf->arq_ptr, "%f\n", active_dxf->lw );

  if ((a2-a1) != 360)
    writevertex (xc, yc, 0, 0, 0, 0);    /* center */
  for (i=0, t=t1; i<nseg; i++, t+=seg_angle)
  {                                            /* calculate bulge between t */
    bulge = calc_bulge (a, b, t, t+seg_angle); /* and t+seg_angle and write */
    writevertex (xc, yc, a, b, t, bulge);      /* vertex at t               */
  }
  writevertex (xc, yc, a, b, t2, 0);     /* bulge of last vertex is useless */
  if ((a2-a1) != 360)
    writevertex (xc, yc, 0, 0, 0, 0);    /* center */

  fprintf ( active_dxf->arq_ptr, "  0\n" );
  fprintf ( active_dxf->arq_ptr, "SEQEND\n" );
}

static void cdtext (int x, int y, char *s)
{
  fprintf ( active_dxf->arq_ptr, "0\n" );
  fprintf ( active_dxf->arq_ptr, "TEXT\n" );
  fprintf ( active_dxf->arq_ptr, "8\n" );
  fprintf ( active_dxf->arq_ptr, "%d\n", active_dxf->layer);   /* current layer */
  fprintf ( active_dxf->arq_ptr, "7\n" );
  wnamfont( active_dxf->tf, active_dxf );                      /* current font  */
  fprintf ( active_dxf->arq_ptr, "62\n" );
  fprintf ( active_dxf->arq_ptr, "%d\n", active_dxf->fgcolor );         /* color            */
  fprintf ( active_dxf->arq_ptr, "10\n" );
  fprintf ( active_dxf->arq_ptr, "%f\n", x/active_dxf->cnv->xres );    /* current position */
  fprintf ( active_dxf->arq_ptr, "20\n" );
  fprintf ( active_dxf->arq_ptr, "%f\n", y/active_dxf->cnv->xres );
  fprintf ( active_dxf->arq_ptr, "11\n" );
  fprintf ( active_dxf->arq_ptr, "%f\n", x/active_dxf->cnv->xres );    /* alignment point  */
  fprintf ( active_dxf->arq_ptr, "21\n" );
  fprintf ( active_dxf->arq_ptr, "%f\n", y/active_dxf->cnv->xres );
  fprintf ( active_dxf->arq_ptr, "40\n" );
  fprintf ( active_dxf->arq_ptr, "%f\n",  active_dxf->th );    /* text height */
  fprintf ( active_dxf->arq_ptr, "50\n" );
  fprintf ( active_dxf->arq_ptr, "%f\n",  active_dxf->cnv->text_orientation );    /* text orientation angle    */
  fprintf ( active_dxf->arq_ptr, "51\n" );
  fprintf ( active_dxf->arq_ptr, "%3d\n", active_dxf->toa );   /* text oblique angle        */
  fprintf ( active_dxf->arq_ptr, "72\n" );
  fprintf ( active_dxf->arq_ptr, "%3d\n", active_dxf->tha );   /* text horizontal alignment */
  fprintf ( active_dxf->arq_ptr, "73\n" );
  fprintf ( active_dxf->arq_ptr, "%3d\n", active_dxf->tva );   /* text vertical alignment   */
  fprintf ( active_dxf->arq_ptr, "1\n" );
  fprintf ( active_dxf->arq_ptr, "%s\n", s );          /* text */
}


/*==========================================================================*/
/* Attributes                                                               */
/*==========================================================================*/

static int cdlinestyle (int style)
{
  switch (style)
  {
  case CD_CONTINUOUS:
    active_dxf->lt = 0;
    break;
  case CD_DASHED:
    active_dxf->lt = 1;
    break;
  case CD_DOTTED:
    active_dxf->lt = 5;
    break;
  case CD_DASH_DOT:
    active_dxf->lt = 6;
    break;
  case CD_DASH_DOT_DOT:
    active_dxf->lt = 8;
    break;
  }

  return style;
}

static int cdlinewidth (int width)
{
  active_dxf->lw = width/active_dxf->cnv->xres;
  return width;
}

static void cdfont (int *type_face, int *style, int *size)
{
  switch (*type_face)         /* obs: DXF's text height (active_dxf->th) corresponds */
  {                          /*      to Canvas Draw's ascent                */
    case CD_SYSTEM:
      active_dxf->tf = 0;
      active_dxf->toa = 0;
      active_dxf->th = *size * 0.75;
      break;

    case CD_COURIER:
      switch (*style)
      {
        case CD_PLAIN:
          active_dxf->tf = 1;
          active_dxf->toa = 0;
          break;

        case CD_BOLD:
          active_dxf->tf = 2;
          active_dxf->toa = 0;
          break;

        case CD_ITALIC:
          active_dxf->tf = 1;
          active_dxf->toa = 15;
          break;

        case CD_BOLD_ITALIC:
          active_dxf->tf = 2;
          active_dxf->toa = 15;
          break;
      }
      active_dxf->th = *size * 0.75;
      break;

    case CD_TIMES_ROMAN:
      switch (*style)
      {
        case CD_PLAIN:
          active_dxf->tf = 3;
          active_dxf->toa = 0;
          break;

        case CD_BOLD:
          active_dxf->tf = 4;
          active_dxf->toa = 0;
          break;

        case CD_ITALIC:
          active_dxf->tf = 3;
          active_dxf->toa = 15;
          break;

        case CD_BOLD_ITALIC:
          active_dxf->tf = 4;
          active_dxf->toa = 15;
          break;
      }
      active_dxf->th = *size * 1.125;
      break;

    case CD_HELVETICA:
      switch (*style)
      {
        case CD_PLAIN:
          active_dxf->tf = 5;
          active_dxf->toa = 0;
          break;

        case CD_BOLD:
          active_dxf->tf = 6;
          active_dxf->toa = 0;
          break;

        case CD_ITALIC:
          active_dxf->tf = 5;
          active_dxf->toa = 15;
          break;

        case CD_BOLD_ITALIC:
          active_dxf->tf = 6;
          active_dxf->toa = 15;
          break;
      }
      active_dxf->th = *size;
      break;
  }

  if (active_dxf->th < 0)
    active_dxf->th /= -active_dxf->cnv->xres*2.83465;  /*  pixels to points */
  else
    active_dxf->th /= 2.83465;                 /* mm to points */
}

static void cdfontdim (int *max_width, int *height, int *ascent, int *descent)
{
  double tangent_ta;
  double pixel_th;

  tangent_ta = tan(active_dxf->toa*PI/180);
  pixel_th = active_dxf->th*active_dxf->cnv->xres*2.83465;  /* points to pixels */
  switch (active_dxf->tf)
  {
    case 0:                                  /* STANDARD font (CD_SYSTEM) */
      if (height)    *height    = round(pixel_th*4/3);
      if (ascent)    *ascent    = round(pixel_th);
      if (descent)   *descent   = round(pixel_th/3);
      if (max_width) *max_width = round(pixel_th);
      break;

    case 1:                                  /* ROMAN fonts (CD_COURIER)  */
    case 2:
      if (height)    *height    = round(pixel_th*4/3);
      if (ascent)    *ascent    = round(pixel_th);
      if (descent)   *descent   = round(pixel_th/3);
      if (max_width) *max_width = round((pixel_th*21/20) + tangent_ta*(*ascent));
      break;

    case 3:                            /* ROMANTIC fonts (CD_TIMES_ROMAN) */
      if (height)    *height    = round(pixel_th*8/9);
      if (ascent)    *ascent    = round(pixel_th*2/3);
      if (descent)   *descent   = round(pixel_th*2/9);
      if (max_width) *max_width = round((pixel_th*14/15) + tangent_ta*(*ascent));
      break;

    case 4:
      if (height)    *height    = round(pixel_th*8/9);
      if (ascent)    *ascent    = round(pixel_th*2/3);
      if (descent)   *descent   = round(pixel_th*2/9);
      if (max_width) *max_width = round((pixel_th*29/30) + tangent_ta*(*ascent));
      break;

    case 5:                            /* SANSSERIF fonts (CD_HELVETICA)  */
    case 6:
      if (height)    *height    = round(pixel_th);
      if (ascent)    *ascent    = round(pixel_th*3/4);
      if (descent)   *descent   = round(pixel_th/4);
      if (max_width) *max_width = round((pixel_th*15/16) + tangent_ta*(*ascent));
      break;
  }
}

static void cdtextsize (char *s, int *width, int *height)
{
  int i;
  double tangent_ta;
  double pixel_th;

  i = (int)strlen(s);
  tangent_ta = tan(active_dxf->toa*PI/180);
  pixel_th = active_dxf->th*active_dxf->cnv->xres*2.83465;  /* points to pixels */

  switch (active_dxf->tf)  /* width return value based on maximum character width */
  {
    case 0:                                  /* STANDARD font (CD_SYSTEM) */
      if (height) *height = round(pixel_th*4/3);
      if (width)  *width  = round(pixel_th*i + (pixel_th/3)*(i-1));
      break;

    case 1:                                  /* ROMAN fonts (CD_COURIER)  */
    case 2:
      if (height) *height = round(pixel_th*4/3);
      if (width)  *width  = round((pixel_th*21/20)*i + (pixel_th/10)*(i-1) +
                                  tangent_ta*pixel_th);
      break;

    case 3:                            /* ROMANTIC fonts (CD_TIMES_ROMAN) */
      if (height) *height = round(pixel_th*2/3 + pixel_th*2/9);
      if (width)  *width  = round((pixel_th*14/15)*i + (pixel_th/45)*(i-1) +
                                   tangent_ta*pixel_th*2/3);
      break;

    case 4:
      if (height) *height = round(pixel_th*2/3 + pixel_th*2/9);
      if (width)  *width  = round((pixel_th*29/30)*i + (pixel_th*2/45)*(i-1) +
                                   tangent_ta*pixel_th*2/3);
      break;

    case 5:                            /* SANSSERIF fonts (CD_HELVETICA)  */
    case 6:
      if (height) *height = round(pixel_th);
      if (width)  *width  = round((pixel_th*15/16)*i + (pixel_th/45)*(i-1) +
                                  tangent_ta*pixel_th*3/4);
      break;
  }
}

static int cdtextalignment (int alignment)
{
  switch (alignment)          /* convert alignment to DXF format */
  {
    case CD_BASE_LEFT:
      active_dxf->tva = 0;
      active_dxf->tha = 0;
      break;

    case CD_BASE_CENTER:
      active_dxf->tva = 0;
      active_dxf->tha = 1;
      break;

    case CD_BASE_RIGHT:
      active_dxf->tva = 0;
      active_dxf->tha = 2;
      break;

    case CD_SOUTH_WEST:
      active_dxf->tva = 1;
      active_dxf->tha = 0;
      break;

    case CD_SOUTH:
      active_dxf->tva = 1;
      active_dxf->tha = 1;
      break;

    case CD_SOUTH_EAST:
      active_dxf->tva = 1;
      active_dxf->tha = 2;
      break;

    case CD_WEST:
      active_dxf->tva = 2;
      active_dxf->tha = 0;
      break;

    case CD_CENTER:
      active_dxf->tva = 2;
      active_dxf->tha = 1;
      break;

    case CD_EAST:
      active_dxf->tva = 2;
      active_dxf->tha = 2;
      break;

    case CD_NORTH_WEST:
      active_dxf->tva = 3;
      active_dxf->tha = 0;
      break;

    case CD_NORTH:
      active_dxf->tva = 3;
      active_dxf->tha = 1;
      break;

    case CD_NORTH_EAST:
      active_dxf->tva = 3;
      active_dxf->tha = 2;
      break;
  }

  return alignment;
}

/*==========================================================================*/
/* Color                                                                    */
/*==========================================================================*/

static void RGB_to_HSB (unsigned char r, unsigned char g, unsigned char b,
                        double *hue, double *sat, double *bright)
{
  double maximum;
  double minimum;
  double delta;
  double red   = r/255.;         /* red, green and blue range from 0 to 1 */
  double green = g/255.;
  double blue  = b/255.;

  maximum = max(max(red, green), blue);   /* stores higher index */
  minimum = min(min(red, green), blue);   /* stores lower index  */
  delta   = maximum - minimum;

  *bright = maximum*100;
  *sat    = 0;

  if (maximum != 0)     /* sat from 0 to 100 */
    *sat = (delta*100)/maximum;

  if (*sat != 0)        /* hue from 0 to 359 */
  {
    if (red   == maximum) *hue = (green - blue)/delta;
    if (green == maximum) *hue = 2 + (blue - red)/delta;
    if (blue  == maximum) *hue = 4 + (red - green)/delta;
    *hue *= 60;
    if (*hue < 0) *hue += 360;
  }
  else
    *hue = 0;           /* color is greyscale (hue is meaningless) */
}

static int HSB_to_AutoCAD_Palette (double hue, double sat, double bright)
{
  int index;
  int h, s, b;

  if (bright < 17)     /* 5 levels of brightness in AutoCAD palette, 6 with */
  {                    /* black. If bright < 17, index is black (7).        */
    index = 7;         /* 17 is 100/6 (rounded up)                          */
  }
  else if (sat < 10)              /* low saturation makes color tend to     */
  {                               /* grey/white. 6 levels of grey/white in  */
    b = (int)floor(bright/14.3)-1;/* palette WITHOUT black. 14.3 is 100/7   */
    index = 250 + b;              /* index is grey to white(255 in palette) */
  }
  else
  {
    h = round(hue/15.) + 1;
    if (h > 24) h -= 24;          /* 15 is 360/24                           */
    h *= 10;                      /* h ranges from 10 to 240 in palette     */
    s = (sat < 55) ? 1 : 0;       /* s is 'high'(0) or 'low'(1) in palette  */
    b = (int)floor(bright/16.7)-1;/* b is 0, 2, 4, 6 or 8 in palette        */
    b = 2*(4 - b);                /* (from brightest to dimmest)            */
    index = h + s + b;            /* index is simple sum of h, s and b      */
  }
  return index;
}

static int get_palette_index (long int color)      /* gives closest palette */
{                                                  /* index to RGB color    */
  unsigned char red, green, blue;
  double hue, sat, bright;

  cdDecodeColor (color, &red, &green, &blue);         /* AutoCAD palette is */
  RGB_to_HSB (red, green, blue, &hue, &sat, &bright); /* based on HSB model */

  return HSB_to_AutoCAD_Palette (hue, sat, bright);
}

static long int cdforeground (long int color)
{
  active_dxf->fgcolor = get_palette_index (color);
  return color;
}


/*==========================================================================*/
/* Server Images                                                            */
/*==========================================================================*/

static void cdpixel (int x, int y, long int color)
{
  int oldcolor = active_dxf->fgcolor;                    /* put 'color' as current */
  cdforeground (color);                          /* foreground color */
  fprintf ( active_dxf->arq_ptr, "0\n" );
  fprintf ( active_dxf->arq_ptr, "POINT\n" );
  fprintf ( active_dxf->arq_ptr, "8\n" );
  fprintf ( active_dxf->arq_ptr, "%d\n", active_dxf->layer);     /* current layer */
  fprintf ( active_dxf->arq_ptr, "62\n" );
  fprintf ( active_dxf->arq_ptr, "%d\n", active_dxf->fgcolor );  /* color */
  fprintf ( active_dxf->arq_ptr, "10\n" );
  fprintf ( active_dxf->arq_ptr, "%f\n", x/active_dxf->cnv->xres );         /* position */
  fprintf ( active_dxf->arq_ptr, " 20\n" );
  fprintf ( active_dxf->arq_ptr, "%f\n", y/active_dxf->cnv->xres );
  active_dxf->fgcolor = oldcolor;                        /* retrieve old fgcolor */
}

/******************************************************/

static void cdcreatecanvas(cdPrivateCanvas* cnv, void *data)
{
  char filename[10240] = "";
  char* strdata = (char*)data;
  Tdxf_gc *new_cnv;
  double param1, param2, param3;

  new_cnv = (Tdxf_gc *) malloc (sizeof (Tdxf_gc));

  param1 = 0;
  param2 = 0;
  param3 = 0;

  strdata += cdGetFileName(strdata, filename);
  if (filename[0] == 0)
    return;

  sscanf(strdata, "%lfx%lf %lf", &param1, &param2, &param3);

  new_cnv->arq_ptr = fopen (filename, "w");
  if (new_cnv->arq_ptr == NULL)
  {
    free(new_cnv);
    return;
  }

  if (!param1)
  {
    cnv->w_mm  = MAX_INT;
    cnv->h_mm = MAX_INT;
    cnv->xres = 3.78;
  }
  else if (!param2)
  {
    cnv->w_mm  = MAX_INT/param1;
    cnv->h_mm = MAX_INT/param1;
    cnv->xres = param1;
  }
  else if (!param3)
  {
    cnv->w_mm  = param1;
    cnv->h_mm = param2;
    cnv->xres = 3.78;
  }
  else
  {
    cnv->w_mm  = param1;
    cnv->h_mm = param2;
    cnv->xres = param3;
  }

  if ((max(cnv->w_mm, cnv->h_mm) * cnv->xres) > MAX_INT)
    return;                    /* returns error if number of points in */
                               /* drawing exceeds 'int' limits         */

  new_cnv->cnv = cnv;
  cnv->context_canvas = new_cnv;

  cnv->bpp = 8;

  cnv->yres = cnv->xres;

  cnv->w = (int)(cnv->w_mm * cnv->xres);
  cnv->h = (int)(cnv->h_mm * cnv->yres);

  new_cnv->layer = 0;            /* reset layer    */

  new_cnv->tf  = 0;              /* text font (0 is STANDARD)               */
  new_cnv->th  = 9;              /* text height                             */
  new_cnv->toa = 0;              /* text oblique angle                      */
  new_cnv->tva = 0;              /* text vertical alignment (0 is baseline) */
  new_cnv->tha = 0;              /* text horizontal alignment (0 is left)   */
  new_cnv->fgcolor   = 7;        /* foreground AutoCAD palette color        */

  fprintf (new_cnv->arq_ptr, "0\n");
  fprintf (new_cnv->arq_ptr, "SECTION\n");  /* header maker */
  fprintf (new_cnv->arq_ptr, "2\n");
  fprintf (new_cnv->arq_ptr, "HEADER\n");
  fprintf (new_cnv->arq_ptr, "9\n");
  fprintf (new_cnv->arq_ptr, "$LIMCHECK\n");
  fprintf (new_cnv->arq_ptr, "70\n");
  fprintf (new_cnv->arq_ptr, "1\n");
  fprintf (new_cnv->arq_ptr, "9\n");
  fprintf (new_cnv->arq_ptr, "$LIMMIN\n");
  fprintf (new_cnv->arq_ptr, "10\n");
  fprintf (new_cnv->arq_ptr, "0\n");
  fprintf (new_cnv->arq_ptr, "20\n");
  fprintf (new_cnv->arq_ptr, "0\n");
  fprintf (new_cnv->arq_ptr, "9\n");
  fprintf (new_cnv->arq_ptr, "$LIMMAX\n");
  fprintf (new_cnv->arq_ptr, "10\n");
  fprintf (new_cnv->arq_ptr, "%f\n", new_cnv->cnv->w_mm);
  fprintf (new_cnv->arq_ptr, "20\n");
  fprintf (new_cnv->arq_ptr, "%f\n", new_cnv->cnv->h_mm);
  fprintf (new_cnv->arq_ptr, "9\n");
  fprintf (new_cnv->arq_ptr, "$EXTMIN\n");
  fprintf (new_cnv->arq_ptr, "10\n");
  fprintf (new_cnv->arq_ptr, "0\n");
  fprintf (new_cnv->arq_ptr, "20\n");
  fprintf (new_cnv->arq_ptr, "0\n");
  fprintf (new_cnv->arq_ptr, "9\n");
  fprintf (new_cnv->arq_ptr, "$EXTMAX\n");
  fprintf (new_cnv->arq_ptr, "10\n");
  fprintf (new_cnv->arq_ptr, "%f\n", new_cnv->cnv->w_mm);
  fprintf (new_cnv->arq_ptr, "20\n");
  fprintf (new_cnv->arq_ptr, "%f\n", new_cnv->cnv->h_mm);
  fprintf (new_cnv->arq_ptr, "9\n");
  fprintf (new_cnv->arq_ptr, "$CLAYER\n");
  fprintf (new_cnv->arq_ptr, "8\n");
  fprintf (new_cnv->arq_ptr, "0\n");
  fprintf (new_cnv->arq_ptr, "9\n");
  fprintf (new_cnv->arq_ptr, "$LUNITS\n");
  fprintf (new_cnv->arq_ptr, "70\n");
  fprintf (new_cnv->arq_ptr, "2\n");
  fprintf (new_cnv->arq_ptr, "9\n");
  fprintf (new_cnv->arq_ptr, "$LUPREC\n");
  fprintf (new_cnv->arq_ptr, "70\n");    /* precision (resolution dependant) */
  fprintf (new_cnv->arq_ptr, "%d\n", (int)ceil(log10(new_cnv->cnv->xres)));
  fprintf (new_cnv->arq_ptr, "9\n");
  fprintf (new_cnv->arq_ptr, "$AUNITS\n");
  fprintf (new_cnv->arq_ptr, "70\n");
  fprintf (new_cnv->arq_ptr, "0\n");
  fprintf (new_cnv->arq_ptr, "9\n");
  fprintf (new_cnv->arq_ptr, "$AUPREC\n");
  fprintf (new_cnv->arq_ptr, "70\n");
  fprintf (new_cnv->arq_ptr, "2\n");
  fprintf (new_cnv->arq_ptr, "9\n");
  fprintf (new_cnv->arq_ptr, "$TEXTSTYLE\n");
  fprintf (new_cnv->arq_ptr, "7\n");
  fprintf (new_cnv->arq_ptr, "STANDARD\n");
  fprintf (new_cnv->arq_ptr, "0\n");
  fprintf (new_cnv->arq_ptr, "ENDSEC\n");
  fprintf (new_cnv->arq_ptr, "0\n");
  fprintf (new_cnv->arq_ptr, "SECTION\n");
  fprintf (new_cnv->arq_ptr, "2\n");
  fprintf (new_cnv->arq_ptr, "TABLES\n");

  deflines (new_cnv);      /* define lines */
  deffonts (new_cnv);      /* define fonts */

  fprintf (new_cnv->arq_ptr, "0\n");
  fprintf (new_cnv->arq_ptr, "ENDSEC\n");
  fprintf (new_cnv->arq_ptr, "0\n");
  fprintf (new_cnv->arq_ptr, "SECTION\n");
  fprintf (new_cnv->arq_ptr, "2\n");
  fprintf (new_cnv->arq_ptr, "ENTITIES\n");
}

static void cdinittable(cdPrivateCanvas* cnv)
{
  cnv->Flush = cdflush;
  cnv->Pixel = cdpixel;
  cnv->Line = cdline;
  cnv->Poly = cdpoly;
  cnv->Rect = cdboxrect;
  cnv->Box = cdboxrect;
  cnv->Arc = cdarc;
  cnv->Sector = cdsector;
  cnv->Text = cdtext;
  cnv->FontDim = cdfontdim;
  cnv->TextSize = cdtextsize;

  cnv->LineStyle = cdlinestyle;
  cnv->LineWidth = cdlinewidth;
  cnv->Font = cdfont;
  cnv->TextAlignment = cdtextalignment;
  cnv->Foreground = cdforeground;

  cnv->KillCanvas = cdkillcanvas;
  cnv->Activate = cdactivate;
  cnv->Deactivate = cddeactivate;
}

/******************************************************/

static cdPrivateContext cdDXFContextTable =
{
  CD_CAP_ALL & ~(CD_CAP_CLEAR | CD_CAP_PLAY | CD_CAP_PALETTE |
                 CD_CAP_CLIPAREA | CD_CAP_CLIPPOLY |
                 CD_CAP_MARK |
                 CD_CAP_LINECAP | CD_CAP_LINEJOIN | CD_CAP_REGION | CD_CAP_CHORD |
                 CD_CAP_IMAGERGB | CD_CAP_IMAGEMAP | CD_CAP_IMAGESRV |
                 CD_CAP_BACKGROUND | CD_CAP_BACKOPACITY | CD_CAP_WRITEMODE |
                 CD_CAP_HATCH | CD_CAP_STIPPLE | CD_CAP_PATTERN |
                 CD_CAP_IMAGERGBA | CD_CAP_GETIMAGERGB |
                 CD_CAP_WD | CD_CAP_VECTORTEXT),
  cdcreatecanvas,
  cdinittable,
  NULL,
  NULL,
};


static cdContext cdDXF =
{
  &cdDXFContextTable
};


cdContext* cdContextDXF(void)
{
  return &cdDXF;
}

