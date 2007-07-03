/*
%M Canvas Draw - CD_CGM Compter Graphics Metafile
%a Camilo Freire
%d 31 1 96
%p Sistemas Graficos
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <limits.h>

#include "cd.h"
#include "cdprivat.h"
#include "cdcgm.h"
#include "cgm.h"

#ifndef PI
#define PI 3.141593
#endif
#define WIDTH SHRT_MAX
#define HEIGHT SHRT_MAX

#define get_red(_)   (((float)cdRed(_))/255)
#define get_green(_) (((float)cdGreen(_))/255)
#define get_blue(_)  (((float)cdBlue(_))/255)


typedef struct cdCanvasCGM 
{
  cdPrivateCanvas* cnv;
  
  char cgmFileName[256];   /* Arquivo CGM */
  int  codificacao;        /* Codificacao */
  CGM  *cgm;
  int  vdc_int_prec;
  int first;               /* Primeira primitiva a ser desenhada */
  long point;
  int hassize;
  int patindex;
  
  struct 
  {
    struct 
    {
      float xmin, xmax;
      float ymin, ymax;
    } bbox;
    
    int first;
  } clip;
  
  struct 
  {
    float xmin, xmax;
    float ymin, ymax;
  } b_box;
} cdCanvasCGM;

static cdCanvasCGM *active_cgm;

static double cve_black[] = { 0.0, 0.0, 0.0 };
static double cve_white[] = { 1.0, 1.0, 1.0 };
 
int cgmplay ( char*, int, int, int, int);

/*
%F Atualiza os valores do bounding box
*/
static void setbbox ( float x, float y )
{
  if ( active_cgm->first )
  {
    active_cgm->b_box.xmin = x;
    active_cgm->b_box.xmax = x;
    active_cgm->b_box.ymin = y;
    active_cgm->b_box.ymax = y;
    active_cgm->first = 0;
  }
  
  if ( x<active_cgm->b_box.xmin ) active_cgm->b_box.xmin = x;
  if ( x>active_cgm->b_box.xmax ) active_cgm->b_box.xmax = x;
  if ( y<active_cgm->b_box.ymin ) active_cgm->b_box.ymin = y;
  if ( y>active_cgm->b_box.ymax ) active_cgm->b_box.ymax = y;
}


/*
%F metafile descriptor elements
*/
static void metafile_descriptor (cdCanvasCGM *p)
{
  const char *font_list[] = { "SYSTEM", "COURIER", "TIMES_ROMAN", "HELVETICA", 
    "SYSTEM_BOLD", "COURIER_BOLD", "TIMES_ROMAN_BOLD", "HELVETICA_BOLD",
    "SYSTEM_ITALIC", "COURIER_ITALIC", "TIMES_ROMAN_ITALIC",
    "HELVETICA_ITALIC", "SYSTEM_BOLDITALIC", "COURIER_BOLDITALIC",
    "TIMES_ROMAN_BOLDITALIC", "HELVETICA_BOLDITALIC", NULL };
  
  cgm_metafile_version        ( p->cgm, 1);
  cgm_metafile_description    ( p->cgm, "CD generated" );
  cgm_vdc_type                ( p->cgm, 0 /* integer */ );
  cgm_integer_precision       ( p->cgm, 16 );
  cgm_real_precision          ( p->cgm, 3 /* fixed 32 */ );
  cgm_index_precision         ( p->cgm, 16 );
  cgm_colour_precision        ( p->cgm, 8 );
  cgm_colour_index_precision  ( p->cgm, 8 );
  cgm_maximum_colour_index    ( p->cgm, 255ul );
  cgm_colour_value_extent     ( p->cgm, cve_black, cve_white );
  
  {
    static int classes[] = { -1 /* drawing set */ };
    static int ids    [] = {  1 /* plus control set */ };
    cgm_metafile_element_list   ( p->cgm, 1, classes, ids );
  }
  
  cgm_begin_metafile_defaults ( p->cgm );
  
  cgm_vdc_integer_precision ( p->cgm, p->vdc_int_prec );
  cgm_interior_style  ( p->cgm, 1 );  /* SOLID */
  cgm_edge_visibility  ( p->cgm, 0 );  /* OFF */
  
  cgm_end_metafile_defaults   ( p->cgm );
  
  cgm_font_list ( p->cgm, font_list );
}

/*
%F Pictire descriptor elements
*/
static void picture_descriptor (cdCanvasCGM *p)
{
  cgm_scaling_mode             ( p->cgm, 1 /* metric */, (float) 1 );
  cgm_colour_selection_mode    ( p->cgm, 1 /* direct */ );
  cgm_line_width_specify_mode  ( p->cgm, 0 /* absolute=0, scaled=1 */ );
  cgm_marker_size_specify_mode ( p->cgm, 0 /* absolute=0, scaled=1 */ );
  
  p->point = ftell ( p->cgm->fp );
  if ( p->codificacao == CD_CLEAR_TEXT )
  {
    fprintf ( p->cgm->fp, "%80s\n", "" );
    fprintf ( p->cgm->fp, "%80s\n", "" );
  }
  else
  {
    cgm_vdc_extent     ( p->cgm, 0, 0, (double)p->cnv->w, (double)p->cnv->h);
  }
}

/*
%F Control descriptor elements
*/
static void control_elements (cdCanvasCGM *p)
{
  double c[3] = {1.0,1.0,1.0};
  
  cgm_vdc_integer_precision ( p->cgm, p->vdc_int_prec );
  cgm_vdc_real_precision    ( p->cgm, 2 /* fixed 32 */ );
  cgm_auxiliary_colour      ( p->cgm, c );
}

static void cdkillcanvas(void *cnv)
{
  long pt;
  cdCanvasCGM *p = (cdCanvasCGM *)cnv;
  
  pt = ftell ( p->cgm->fp );
  fseek ( p->cgm->fp, p->point, SEEK_SET );

  if (p->hassize)
   cgm_vdc_extent     ( p->cgm, 0, 0, (double)p->cnv->w, (double)p->cnv->h);
  else
  {
   if ( p->clip.first )
    cgm_vdc_extent ( p->cgm, p->b_box.xmin, p->b_box.ymin, p->b_box.xmax, p->b_box.ymax );
   else
    cgm_vdc_extent ( p->cgm, p->clip.bbox.xmin, p->clip.bbox.ymin, p->clip.bbox.xmax, p->clip.bbox.ymax );
  }

  fseek ( p->cgm->fp, pt, SEEK_SET );
  
  cgm_end_picture ( p->cgm );
  cgm_end_metafile ( p->cgm );
  
  free(p);
}

static int cdactivate(void *data)
{
  active_cgm = (cdCanvasCGM *)data;
  return CD_OK;
}

static void cddeactivate(void)
{
  fflush(active_cgm->cgm->fp);
  active_cgm = NULL;
}

/*
%F Comeca uma nova pagina.
*/
static void cdflush(void)
{
  long pt;
  
  pt = ftell ( active_cgm->cgm->fp );
  fseek ( active_cgm->cgm->fp, active_cgm->point, SEEK_SET );
  
  if (active_cgm->hassize)
   cgm_vdc_extent     ( active_cgm->cgm, 0, 0, (double)active_cgm->cnv->w, (double)active_cgm->cnv->h);
  else
  {
   if ( active_cgm->clip.first )
    cgm_vdc_extent ( active_cgm->cgm, active_cgm->b_box.xmin, active_cgm->b_box.ymin,
                     active_cgm->b_box.xmax, active_cgm->b_box.ymax );
   else
     cgm_vdc_extent ( active_cgm->cgm, active_cgm->clip.bbox.xmin, active_cgm->clip.bbox.ymin,
                      active_cgm->clip.bbox.xmax, active_cgm->clip.bbox.ymax );
  }
 
  fseek ( active_cgm->cgm->fp, pt, SEEK_SET );
  
  cgm_end_picture        ( active_cgm->cgm );

  cgm_begin_picture      ( active_cgm->cgm, "Picture x" );
  picture_descriptor ( active_cgm);
  cgm_begin_picture_body ( active_cgm->cgm );
}


typedef int(*_cdsizecb)(cdContext *driver, int w, int h, double w_mm, double h_mm);
int(*cdsizecb)(cdContext *driver, int w, int h, double w_mm, double h_mm) = NULL;

typedef int(*_cdcgmbegmtfcb)(cdContext *driver, int *xmin, int *ymin, int *xmax, int *ymax);
int(*cdcgmbegmtfcb)(cdContext *driver, int *xmin, int *ymin, int *xmax, int *ymax) = NULL;

typedef int(*_cdcgmcountercb)(cdContext *driver, double size);
int(*cdcgmcountercb)(cdContext *driver, double size) = NULL;

typedef int(*_cdcgmsclmdecb)(cdContext *driver, short scl_mde, short *drw_mode, double *factor);
int(*cdcgmsclmdecb)(cdContext *driver, short scl_mode, short *drw_mode, double *factor) = NULL;

typedef int(*_cdcgmvdcextcb)(cdContext *driver, short type, double *xmn, double *ymn, double *xmx, double *ymx);
int(*cdcgmvdcextcb)(cdContext *driver, short type, double *xmn, double *ymn, double *xmx, double *ymx) = NULL;

typedef int(*_cdcgmbegpictcb)(cdContext *driver , char *pict );
int(*cdcgmbegpictcb)(cdContext *driver, char *pict ) = NULL;

typedef int(*_cdcgmbegpictbcb)(cdContext *driver, double scale_x, double scale_y, 
                               double vdc_x2pix, double vdc_y2pix,
                               double vdc_x2mm, double vdc_y2mm, int drw_mode, 
                               double xmin, double ymin, double xmax, double ymax);
int(*cdcgmbegpictbcb)(cdContext *driver, double scale_x, double scale_y,
                      double vdc_x2pix, double vdc_y2pix, double vdc_x2mm,
                      double vdc_y2mm, int drw_mode,
                      double xmin, double ymin, double xmax, double ymax ) = NULL;

static int cdregistercallback(int cb, int(*func)(cdContext *driver, ...))
{
  switch (cb)
  {
  case CD_SIZECB:
    cdsizecb = (_cdsizecb)func;
    return CD_OK;
  case CD_CGMBEGMTFCB:
    cdcgmbegmtfcb = (_cdcgmbegmtfcb)func;
    return CD_OK;
  case CD_CGMCOUNTERCB:
    cdcgmcountercb = (_cdcgmcountercb)func;
    return CD_OK;
  case CD_CGMSCLMDECB:
    cdcgmsclmdecb = (_cdcgmsclmdecb)func;
    return CD_OK;
  case CD_CGMVDCEXTCB:
    cdcgmvdcextcb = (_cdcgmvdcextcb)func;
    return CD_OK;
  case CD_CGMBEGPICTCB:
    cdcgmbegpictcb = (_cdcgmbegpictcb)func;
    return CD_OK;
  case CD_CGMBEGPICTBCB:
    cdcgmbegpictbcb = (_cdcgmbegpictbcb)func;
    return CD_OK;
  }
  
  return CD_ERROR;
}

static int cdplay(int xmin, int xmax, int ymin, int ymax, void *data)
{
  return cgmplay((char*)data, xmin, xmax, ymin, ymax);
}

/******************************************************/
/* coordinate transformation                          */
/******************************************************/

static int cdclip(int mode)
{
  if (mode == CD_CLIPPOLYGON)
    return active_cgm->cnv->clip_mode;

  cgm_clip_indicator ( active_cgm->cgm, mode );

  if (mode == CD_CLIPAREA)
    cgm_clip_rectangle ( active_cgm->cgm, (double) active_cgm->cnv->clip_xmin, (double) active_cgm->cnv->clip_ymin,
                                          (double) active_cgm->cnv->clip_xmax, (double) active_cgm->cnv->clip_ymax );

  return mode;
}

static void clip_bbox (int xmin, int ymin, int xmax, int ymax)
{
  if ( active_cgm->clip.first )
  {
    active_cgm->clip.bbox.xmin = (float)xmin;
    active_cgm->clip.bbox.xmax = (float)xmax;
    active_cgm->clip.bbox.ymin = (float)ymin;
    active_cgm->clip.bbox.ymax = (float)ymax;
    active_cgm->clip.first = 0;
  }
  
  if ( xmin < active_cgm->clip.bbox.xmin ) active_cgm->clip.bbox.xmin = (float)xmin;
  if ( ymin < active_cgm->clip.bbox.ymin ) active_cgm->clip.bbox.ymin = (float)ymin;
  if ( xmax > active_cgm->clip.bbox.xmax ) active_cgm->clip.bbox.xmax = (float)xmax;
  if ( ymax > active_cgm->clip.bbox.ymax ) active_cgm->clip.bbox.ymax = (float)ymax;
}

static void cdcliparea(int *xmin, int *xmax, int *ymin, int *ymax)
{
  if (active_cgm->cnv->clip_mode == CD_CLIPAREA)
    cgm_clip_rectangle ( active_cgm->cgm, (double) *xmin, (double) *ymin,
                                          (double) *xmax, (double) *ymax );
  
  clip_bbox ( *xmin, *ymin, *xmax, *ymax );
}


/******************************************************/
/* primitives                                         */
/******************************************************/

static void cdline(int px1, int py1, int px2, int py2)
{
  float points[4];

  points[0] = (float)px1;
  points[1] = (float)py1;
  points[2] = (float)px2;
  points[3] = (float)py2;
  
  cgm_polyline( active_cgm->cgm, 2, points);
  
  setbbox ( points[0], points[1] );
  setbbox ( points[2], points[3] );
}

static int cdinteriorstyle (int style);

static void cdrect(int xmin, int xmax, int ymin, int ymax)
{
  float points[4];
  
  points[0] = (float)xmin;
  points[1] = (float)ymin;
  points[2] = (float)xmax;
  points[3] = (float)ymax;
  
  cgm_interior_style ( active_cgm->cgm, HOLLOW);
  cgm_rectangle( active_cgm->cgm, points);
  cdinteriorstyle(active_cgm->cnv->interior_style);
  
  setbbox ( points[0], points[1] );
  setbbox ( points[2], points[1] );
  setbbox ( points[2], points[3] );
  setbbox ( points[0], points[3] );
}

static void cdbox(int xmin, int xmax, int ymin, int ymax)
{
  float points[4];
  
  points[0] = (float)xmin;
  points[1] = (float)ymin;
  points[2] = (float)xmax;
  points[3] = (float)ymax;
  
  cgm_rectangle( active_cgm->cgm, points);
  
  setbbox ( points[0], points[1] );
  setbbox ( points[2], points[1] );
  setbbox ( points[2], points[3] );
  setbbox ( points[0], points[3] );
}

static void arc ( int xc, int yc, int w, int h, double a1, double a2,
                 float *center, float *first_end_point,
                 float *second_end_point, double *dx_start, double *dy_start,
                 double *dx_end, double *dy_end )
{
  float width, height;
  
  center[0] = (float)xc;
  center[1] = (float)yc;
  
  width = (float)w/2;
  height = (float)h/2;
  
  first_end_point[0] = center[0] + width;
  first_end_point[1] = center[1];
  
  second_end_point[0] = center[0];
  second_end_point[1] = center[1] + height;
  
  *dx_start = width*cos(a1*(PI/180.));
  *dy_start = height*sin(a1*(PI/180.));
  
  *dx_end = width*cos(a2*(PI/180.));
  *dy_end = height*sin(a2*(PI/180.));
  
  setbbox ( center[0]-width, center[1]-height );
  setbbox ( center[0]+width, center[1]-height );
  setbbox ( center[0]+width, center[1]+height );
  setbbox ( center[0]-width, center[1]+height );
}

static void cdarc(int xc, int yc, int w, int h, double a1, double a2)
{
  float center[2], first_end_point[2], second_end_point[2];
  double dx_start, dy_start, dx_end, dy_end;
  
  arc ( xc, yc, w, h, a1, a2, center, first_end_point, second_end_point,
       &dx_start, &dy_start, &dx_end, &dy_end );
  
  cgm_elliptical_arc ( active_cgm->cgm, center, first_end_point, second_end_point, dx_start, dy_start, dx_end, dy_end );
}

static void cdsector(int xc, int yc, int w, int h, double a1, double a2)
{
  float center[2], first_end_point[2], second_end_point[2];
  double dx_start, dy_start, dx_end, dy_end;
  
  arc ( xc, yc, w, h, a1, a2, center, first_end_point, second_end_point,
       &dx_start, &dy_start, &dx_end, &dy_end );
  
  
  cgm_elliptical_arc_close ( active_cgm->cgm, center, first_end_point, second_end_point,
                             dx_start, dy_start, dx_end, dy_end, 0 );
  
  setbbox ( (float) xc-w/2, (float) yc-h/2 );
  setbbox ( (float) xc+w/2, (float) yc-h/2 );
  setbbox ( (float) xc+w/2, (float) yc+h/2 );
  setbbox ( (float) xc-w/2, (float) yc+h/2 );
}

static void settextbbox ( float x, float y, int width, int height )
{
  switch ( active_cgm->cnv->text_alignment )
  {
  case CD_NORTH:
    setbbox ( x-(width/2.), y-height );
    setbbox ( x+(width/2.), y );
    break;
  case CD_SOUTH:
    setbbox ( x-(width/2.), y+height );
    setbbox ( x+(width/2.), y );
    break;
  case CD_EAST:
    setbbox ( x-width, y-(height/2.) );
    setbbox ( x, y+(height/2.) );
    break;
  case CD_WEST:
    setbbox ( x, y-(height/2.) );
    setbbox ( x+width, y+(height/2.) );
    break;
  case CD_NORTH_EAST:
    setbbox ( x-width, y-height );
    setbbox ( x, y );
    break;
  case CD_NORTH_WEST:
    setbbox ( x, y-height );
    setbbox ( x+width, y );
    break;
  case CD_SOUTH_EAST:
    setbbox ( x-width, y );
    setbbox ( x, y+height );
    break;
  case CD_SOUTH_WEST:
    setbbox ( x, y );
    setbbox ( x+width, y+height );
    break;
  case CD_CENTER:
    setbbox ( x-(width/2.), y-(height/2.) );
    setbbox ( x+(width/2.), y+(height/2.) );
    break;
  case CD_BASE_LEFT:
    setbbox ( x, y );
    setbbox ( x+width, y+height );
    break;
  case CD_BASE_CENTER:
    setbbox ( x-(width/2.), y );
    setbbox ( x+(width/2.), y+height );
    break;
  case CD_BASE_RIGHT:
    setbbox ( x-width, y );
    setbbox ( x, y+height );
    break;
  }
}

static void cdtext(int x, int y, char *s)
{
  int width, height;
  
  cgm_text( active_cgm->cgm, 1 /* final */ , (double)x, (double)y, s );
  
  cdTextSize(s, &width, &height);
  
  settextbbox ( (float) x, (float) y, width, height );
}

static void cdpoly(int mode, cdPoint* poly, int n)
{
  int i;
  float *fpoly;
  
  fpoly = (float *)malloc(2 * (n+1) * sizeof(float));
  
  for (i = 0; i < n; i++)
  {
    fpoly[2*i] = (float) poly[i].x;
    fpoly[2*i+1] = (float) poly[i].y;

    setbbox (fpoly[2*i] , fpoly[2*i+1] );
  }

  switch ( mode )
  {
  case CD_OPEN_LINES:
    cgm_polyline( active_cgm->cgm, n, fpoly );
    break;
  case CD_CLOSED_LINES:
    fpoly[2*n] = fpoly[0];
    fpoly[2*n+1] = fpoly[1];
    n++;
    cgm_polyline( active_cgm->cgm, n, fpoly );
    break;
  case CD_FILL:
    cgm_polygon( active_cgm->cgm, n, fpoly);
    break;
  }

  free(fpoly);
}


/******************************************************/
/* attributes                                         */
/******************************************************/

static int cdlinestyle(int style)
{
  cgm_line_type( active_cgm->cgm, (long)(style + 1));
  return style;
}

static int cdlinewidth(int width)
{
  cgm_line_width( active_cgm->cgm, (double)width );
  return width;
}

static int cdinteriorstyle ( int style )
{
  switch ( style )
  {
  case CD_SOLID:
    style = 1;
    break;
  case CD_STIPPLE:
  case CD_PATTERN:
    style = 2;
    break;
  case CD_HATCH:
    style = 3;
    break;
  }
  
  cgm_interior_style ( active_cgm->cgm, style );
  
  return style;
}

static int cdhatch(int style)
{
  int cgm_style = style;

  if ( cgm_style==2 ) 
    cgm_style = 3;
  else if ( cgm_style==3 ) 
    cgm_style = 2;

  cgm_hatch_index ( active_cgm->cgm, (long)cgm_style+1 );

  cgm_interior_style ( active_cgm->cgm, 3 );

  return style;
}

static void cdstipple(int n, int m, unsigned char *stipple)
{
  double *pattab;
  int i, j=0;

  pattab = (double *) malloc ( n*m*3*sizeof(double));
  
  for ( i=0; i<n*m; i++ )
  {
    pattab[j+0] = ( stipple[i] ) ? get_red(active_cgm->cnv->foreground) : get_red(active_cgm->cnv->background);
    pattab[j+1] = ( stipple[i] ) ? get_green(active_cgm->cnv->foreground) : get_green(active_cgm->cnv->background);
    pattab[j+2] = ( stipple[i] ) ? get_blue(active_cgm->cnv->foreground) : get_blue(active_cgm->cnv->background);
    j+=3;
  }
  
  cgm_pattern_table ( active_cgm->cgm, (long) active_cgm->patindex, (long) n, (long) m, (int) 8, pattab );
  cgm_pattern_index ( active_cgm->cgm, (long) active_cgm->patindex++ );
  free(pattab);
  
  cgm_interior_style  ( active_cgm->cgm, 2 );  /* PATTERN */
}

static void cdpattern(int n, int m, long int *pattern)
{
  double *pattab;
  int i, j=0;

  pattab = (double *) malloc ( n*m*3*sizeof(double) );
  
  for ( i=0; i<n*m; i++ )
  {
    pattab[j+0] = get_red(pattern[i]);
    pattab[j+1] = get_green(pattern[i]);
    pattab[j+2] = get_blue(pattern[i]);
    j+=3;
  }
  
  cgm_pattern_table ( active_cgm->cgm, (long) active_cgm->patindex, (long) n, (long) m, (int) 8, pattab );
  cgm_pattern_index ( active_cgm->cgm, (long) active_cgm->patindex++ );
  free(pattab);
  
  cgm_interior_style  ( active_cgm->cgm, 2 );  /* PATTERN */
}

static void cdfont(int *type_face, int *style, int *size)
{
  long index;
  
  switch (*type_face)
  {
  case CD_SYSTEM:
    switch (*style)
    {
    case CD_PLAIN:
      index = 1;
      break;
    case CD_BOLD:
      index = 5;
      break;
    case CD_ITALIC:
      index = 9;
      break;
    case CD_BOLD_ITALIC:
      index = 13;
      break;
    }
    break;
  case CD_COURIER:
    switch (*style)
    {
    case CD_PLAIN:
      index = 2;
      break;
    case CD_BOLD:
      index = 6;
      break;
    case CD_ITALIC:
      index = 10;
      break;
    case CD_BOLD_ITALIC:
      index = 14;
      break;
    }
    break;
  case CD_TIMES_ROMAN:
    switch (*style)
    {
    case CD_PLAIN:
      index = 3;
      break;
    case CD_BOLD:
      index = 7;
      break;
    case CD_ITALIC:
      index = 11;
      break;
    case CD_BOLD_ITALIC:
      index = 15;
      break;
    }
    break;
  case CD_HELVETICA:
    switch (*style)
    {
    case CD_PLAIN:
      index = 4;
      break;
    case CD_BOLD:
      index = 8;
      break;
    case CD_ITALIC:
      index = 12;
      break;
    case CD_BOLD_ITALIC:
      index = 16;
      break;
    }
    break;
  }
  
  if (*type_face==CD_SYSTEM)
    *type_face = CD_COURIER;

  if (*size < 0)
    cgm_char_height ( active_cgm->cgm, -(*size));
  else
    cgm_char_height ( active_cgm->cgm, floor ( ((double)*size)/CD_MM2PT+0.5 ) * active_cgm->cnv->xres );

  cgm_text_font_index( active_cgm->cgm, index );
}

static int cdtextalignment(int alignment)
{
  int hor, ver;
  enum { NORMHORIZ, LEFT, CTR, RIGHT };
  enum { NORMVERT, TOP, CAP, HALF, BASE, BOTTOM };
  
  switch ( alignment )
  {
  case CD_NORTH:
    hor = CTR;
    ver = TOP;
    break;
  case CD_SOUTH:
    hor = CTR;
    ver = BOTTOM;
    break;
  case CD_EAST:
    hor = RIGHT;
    ver = HALF;
    break;
  case CD_WEST:
    hor = LEFT;
    ver = HALF;
    break;
  case CD_NORTH_EAST:
    hor = RIGHT;
    ver = TOP;
    break;
  case CD_NORTH_WEST:
    hor = LEFT;
    ver = TOP;
    break;
  case CD_SOUTH_EAST:
    hor = RIGHT;
    ver = BOTTOM;
    break;
  case CD_SOUTH_WEST:
    hor = LEFT;
    ver = BOTTOM;
    break;
  case CD_CENTER:
    hor = CTR;
    ver = HALF;
    break;
  case CD_BASE_LEFT:
    hor = LEFT;
    ver = BASE;
    break;
  case CD_BASE_CENTER:
    hor = CTR;
    ver = BASE;
    break;
  case CD_BASE_RIGHT:
    hor = RIGHT;
    ver = BASE;
    break;
  }
  
  cgm_text_alignment ( active_cgm->cgm, hor, ver , (double)0.0, (double)0.0 );
  
  return alignment;
}

static int cdmarktype(int type)
{
  int cgm_type = 0;

  switch (type)
  {
  case CD_DIAMOND:
  case CD_HOLLOW_DIAMOND:
  case CD_HOLLOW_BOX:
  case CD_BOX:
    cgm_type = MARKER_DOT;
    break;
  case CD_PLUS:
    cgm_type = MARKER_PLUS;
    break;
  case CD_STAR:
    cgm_type = MARKER_ASTERISK;
    break;
  case CD_HOLLOW_CIRCLE:
  case CD_CIRCLE:
    cgm_type = MARKER_CIRCLE;
    break;
  case CD_X:
    cgm_type = MARKER_CROSS;
    break;
  }
  
  cgm_marker_type( active_cgm->cgm, cgm_type);

  return type;
}

static int cdmarksize(int size)
{
  cgm_marker_size( active_cgm->cgm, (double)size);
  return size;
}


/******************************************************/
/* color                                              */
/******************************************************/

static long int cdforeground(long int color)
{
  double cor[3];
  
  cor[0] = get_red(color);
  cor[1] = get_green(color);
  cor[2] = get_blue(color);
  
  cgm_marker_colour( active_cgm->cgm, cor);
  cgm_text_colour( active_cgm->cgm, cor );
  cgm_fill_colour( active_cgm->cgm, cor );
  cgm_line_colour( active_cgm->cgm, cor );
  
  return color;
}

static long int cdbackground(long int color)
{
  double bc[3];
  
  bc[0] = get_red(color);
  bc[1] = get_green(color);
  bc[2] = get_blue(color);
  
  active_cgm->cnv->background = color;
  cgm_backgound_colour ( active_cgm->cgm, bc );
  
  return color;
}

static int cdbackopacity(int opaque)
{
  if (opaque == CD_TRANSPARENT)
    cgm_transparency(active_cgm->cgm, 1);
  else
    cgm_transparency(active_cgm->cgm, 0);
  return opaque;
}

/******************************************************/
/* client images                                      */
/******************************************************/

static void cdputimagerectrgb(int iw, int ih, unsigned char *r, unsigned char *g, unsigned char *b, int x, int y, int w, int h, int xmin, int xmax, int ymin, int ymax)
{
  float p[6];
  double  *color_array;
  int i,j,index,c;
  int rw, rh;

  rw = xmax-xmin+1;
  rh = ymax-ymin+1;
  
  color_array = (double *) malloc ( rw*rh*3*sizeof(double) );
  if (!color_array)
    return;
  
  p[0] = (float) x;      p[1] = (float) (y+h);
  p[2] = (float) (x+w);  p[3] = (float) y;
  p[4] = (float) (x+w);  p[5] = (float) (y+h);
  
  for ( i=0; i<rh; i++ )
  {
    for ( j=0; j<rw; j++ )
    {
      index = (ih-i-1-ymin)*iw+j+xmin;
      c = i*rw*3+j*3;
      color_array[c]   = (double) r[index]/255.;
      color_array[c+1] = (double) g[index]/255.;
      color_array[c+2] = (double) b[index]/255.;
    }
  }
    
  cgm_cell_array ( active_cgm->cgm, p, (long)rw, (long)rh, 8, color_array );
  
  free(color_array);
  
  setbbox ( (float) p[0], (float) p[1] );
  setbbox ( (float) p[2], (float) p[3] );
}

static void cdputimagerectmap(int iw, int ih, unsigned char *index, long int *colors, int x, int y, int w, int h, int xmin, int xmax, int ymin, int ymax)
{
  float p[6];
  double *color_array;
  int i,j,c;
  unsigned char r, g, b;
  int rw, rh;

  rw = xmax-xmin+1;
  rh = ymax-ymin+1;
  
  color_array = (double *) malloc ( rw*rh*3*sizeof(double) );
  if (!color_array)
    return;
  
  p[0] = (float) x;      p[1] = (float) y;
  p[2] = (float) (x+w);  p[3] = (float) (y+h);
  p[4] = (float) (x+w);  p[5] = (float) y;
  
  for ( i=0; i<rh; i++ )
  {
    for ( j=0; j<rw; j++ )
    {
      c = i*rw*3+j*3;
      cdDecodeColor(colors[index[(ih-i-1-ymin)*iw+j+xmin]], &r,&b,&g);
      color_array[c]   = ((double)r)/255.;
      color_array[c+1] = ((double)g)/255.;
      color_array[c+2] = ((double)b)/255.;
    }
  }
    
  cgm_cell_array ( active_cgm->cgm, p, (long)rw, (long)rh, 8, color_array );
  
  free(color_array);
  
  setbbox ( (float) p[0], (float) p[1] );
  setbbox ( (float) p[2], (float) p[3] );
}

/******************************************************/
/* server images                                      */
/******************************************************/

static void cdpixel(int x, int y, long int color)
{
  float pts[2];
 
  pts[0] = (float) x;
  pts[1] = (float) y;

  if (active_cgm->cnv->mark_type != CD_DIAMOND &&
      active_cgm->cnv->mark_type != CD_HOLLOW_DIAMOND &&
      active_cgm->cnv->mark_type != CD_HOLLOW_BOX &&
      active_cgm->cnv->mark_type != CD_BOX)
    cgm_marker_type( active_cgm->cgm, MARKER_DOT);

  if (active_cgm->cnv->mark_size != 1)
    cgm_marker_size( active_cgm->cgm, 1.0);

  cgm_polymarker ( active_cgm->cgm, 1, pts );

  if (active_cgm->cnv->mark_size != 1)
    cgm_marker_size( active_cgm->cgm, (double)active_cgm->cnv->mark_size);

  if (active_cgm->cnv->mark_type != CD_DIAMOND &&
      active_cgm->cnv->mark_type != CD_HOLLOW_DIAMOND &&
      active_cgm->cnv->mark_type != CD_HOLLOW_BOX &&
      active_cgm->cnv->mark_type != CD_BOX)
    cdmarktype(active_cgm->cnv->mark_type);
}

static void cdmark(int x, int y)
{
  float pts[2];
 
  pts[0] = (float) x;
  pts[1] = (float) y;
  
  cgm_polymarker ( active_cgm->cgm, 1, pts );
}

/*
%F Cria um canvas CGM.
Parametros passados em data:
[nome]   nome do arquivo de saida <= 255 caracteres
[size]   tamanho do papel
-t   codificacao clear text se nao binaria
*/
static void cdcreatecanvas(cdPrivateCanvas* cnv, void *data)
{
  cdCanvasCGM *new_cgm;
  char *line = (char *)data;
  char c;
  char words[4][256];
  char filename[10240] = "";
  double w=0, h=0, r=0;
  int p=0;
  int i, n;

  line += cdGetFileName(line, filename);
  if (filename[0] == 0)
    return;
  
  n = sscanf(line, "%s %s %s %s", words[0], words[1], words[2], words[3]);
  
  new_cgm = (cdCanvasCGM *)malloc(sizeof(cdCanvasCGM));

  cnv->context_canvas = new_cgm;
  new_cgm->cnv = cnv;

  strcpy(new_cgm->cgmFileName, filename);
  
  cnv->w_mm = (double) WIDTH;
  cnv->h_mm = (double) HEIGHT;
  cnv->xres = 1.;
  cnv->yres = 1.;
  cnv->bpp = 24;

  new_cgm->vdc_int_prec = 16;
  new_cgm->codificacao = CD_BIN;
  new_cgm->first = 1;
  new_cgm->clip.first = 1;
  new_cgm->patindex = 1;
  
  new_cgm->hassize = 0;		/* Indica se foi passado um tamanho para o canvas */
  
  for ( i=0; i<n-1; i++ )
  {
    if ( sscanf ( words[i], "%lgx%lg",  &w, &h )==2 )
    {
      cnv->w_mm = w;
      cnv->h_mm = h;
      new_cgm->hassize = 1;
    }
    else if ( sscanf ( words[i], "%lg", &r )==1 )
      cnv->yres = cnv->xres = r;
    else if  ( sscanf ( words[i], "-%c%d", &c, &p )>0 )
    {
      if ( c=='t' ) 
        new_cgm->codificacao = CD_CLEAR_TEXT;
      else if ( c=='p' ) 
        new_cgm->vdc_int_prec = p;
    }
  }
  
  if ( new_cgm->vdc_int_prec != 16 && w == 0.0 && h == 0.0 )
  {
    cnv->w_mm = (double) (pow(2,p)/2)-1;
    cnv->h_mm = (double) (pow(2,p)/2)-1;
  }
  
  /* update canvas context */
  cnv->w = (int)(cnv->w_mm * cnv->xres);
  cnv->h = (int)(cnv->h_mm * cnv->yres);
  
  new_cgm->cgm = cgm_begin_metafile ( new_cgm->cgmFileName, new_cgm->codificacao, "CD - CanvasDraw, Tecgraf/PUC-RIO" );
  
  metafile_descriptor(new_cgm);
  
  cgm_begin_picture ( new_cgm->cgm, "Picture x" );

  picture_descriptor (new_cgm);

  cgm_clip_rectangle ( new_cgm->cgm, 0, 0, (double)cnv->w, (double)cnv->h);
  cgm_clip_indicator (new_cgm->cgm, 0);

  cgm_begin_picture_body ( new_cgm->cgm );
  
  control_elements (new_cgm);
}

static void cdinittable(cdPrivateCanvas* cnv)
{
  /* initialize function table*/
  cnv->Flush = cdflush;
  cnv->Pixel = cdpixel;
  cnv->Mark = cdmark;
  cnv->Line = cdline;
  cnv->Poly = cdpoly;
  cnv->Rect = cdrect;
  cnv->Box = cdbox;
  cnv->Arc = cdarc;
  cnv->Sector = cdsector;
  cnv->Text = cdtext;
  cnv->PutImageRectRGB = cdputimagerectrgb;
  cnv->PutImageRectMap = cdputimagerectmap;

  cnv->Clip = cdclip;
  cnv->ClipArea = cdcliparea;
  cnv->LineStyle = cdlinestyle;
  cnv->LineWidth = cdlinewidth;
  cnv->InteriorStyle = cdinteriorstyle;
  cnv->Hatch = cdhatch;
  cnv->Stipple = cdstipple;
  cnv->Pattern = cdpattern;
  cnv->Font = cdfont;
  cnv->TextAlignment = cdtextalignment;
  cnv->Background = cdbackground;
  cnv->Foreground = cdforeground;
  cnv->BackOpacity = cdbackopacity;
  cnv->MarkType = cdmarktype;
  cnv->MarkSize = cdmarksize;

  cnv->KillCanvas = cdkillcanvas;
  cnv->Activate = cdactivate;
  cnv->Deactivate = cddeactivate;
}

/******************************************************/

static cdPrivateContext cdCGMContextTable =
{
  CD_CAP_ALL & ~(CD_CAP_CLEAR | CD_CAP_PALETTE | 
                 CD_CAP_CLIPPOLY | CD_CAP_WRITEMODE | 
                 CD_CAP_IMAGESRV | 
                 CD_CAP_LINECAP | CD_CAP_LINEJOIN | CD_CAP_REGION | CD_CAP_CHORD |
                 CD_CAP_FONTDIM | CD_CAP_TEXTSIZE | 
                 CD_CAP_IMAGERGBA | CD_CAP_GETIMAGERGB | 
                 CD_CAP_WD | CD_CAP_VECTORTEXT | CD_CAP_TEXTORIENTATION),
  cdcreatecanvas,
  cdinittable,
  cdplay,
  cdregistercallback
};


static cdContext cdCGM =
{
  &cdCGMContextTable
};


cdContext* cdContextCGM(void)
{
  return &cdCGM;
}

