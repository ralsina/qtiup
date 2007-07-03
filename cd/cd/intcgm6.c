#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include <cd.h>

#include "list.h"
#include "types.h"
#include "intcgm.h"
#include "intcgm2.h"
#include "intcgm6.h"

void setlinestyle ( int type )
{
  switch ( type )
  {
  case LINE_SOLID:
    cdLineStyle ( CD_CONTINUOUS );
    break;
  case LINE_DASH:
    cdLineStyle ( CD_DASHED );
    break;
  case LINE_DOT:
    cdLineStyle ( CD_DOTTED );
    break;
  case LINE_DASH_DOT:
    cdLineStyle ( CD_DASH_DOT );
    break;
  case LINE_DASH_DOT_DOT:
    cdLineStyle ( CD_DASH_DOT_DOT );
    break;
  }
}

void setlinewidth ( double width )
{
  int w;
  if ( cgm.lnwsm == ABSOLUTE )
    w = delta_vdc2canvas(width);
  else
    w = (int)floor ( width+0.5 );

  cdLineWidth ( w>0?w:1 );
}

void setmarktype ( int type )
{
  switch ( type )
  {
  case MARK_DOT:
    cdMarkType ( CD_STAR );
    break;
  case MARK_PLUS:
    cdMarkType ( CD_PLUS );
    break;
  case MARK_ASTERISK:
    cdMarkType ( CD_X );
    break;
  case MARK_CIRCLE:
    cdMarkType ( CD_CIRCLE );
    break;
  case MARK_CROSS:
    cdMarkType ( CD_PLUS );
    break;
  }
}

void setmarksize ( double size )
{
  if ( cgm.lnwsm == ABSOLUTE )
    cdMarkSize ( delta_vdc2canvas(size) );
  else
    cdMarkSize ( (int)floor ( size*0.5 ) );
}

long int *setpattern ( pat_table pat )
{
  int i;
  long int *cor = (long int *) malloc ( pat.nx*pat.ny*sizeof(long int) );
  
  for ( i=0; i<pat.nx*pat.ny ; i++ )
    cor[i] = getcolor ( pat.pattern[i] );
  
  return cor;
}

int setintstyle ( int style )
{
  if ( style==HOLLOW )
    return CD_CLOSED_LINES;
  else if ( style==SOLID )
  {
    cdInteriorStyle ( CD_SOLID );
    return CD_FILL;
  }
  else if ( style==PATTERN )
  {
    int i;
    pat_table *pat;
    long int *p;
    
    for ( i=1; (pat=(pat_table *)GetList( fill_att.pat_list,i ))!=NULL; i++ )
    {
      if ( pat->index==fill_att.pat_index ) break;
    }
    
    p = (long int *) malloc ( pat->nx*pat->ny*sizeof(long int) );
    
    for ( i=0; i<pat->nx*pat->ny; i++ )
    {
      if ( cgm.clrsm==DIRECT )
        p[i] = cdEncodeColor ((unsigned char)(pat->pattern[i].rgb.red*255./cgm.color_ext.white.red),
                              (unsigned char)(pat->pattern[i].rgb.green*255./cgm.color_ext.white.green),
                              (unsigned char)(pat->pattern[i].rgb.blue*255./cgm.color_ext.white.blue) );
      else
        p[i] = cdEncodeColor ((unsigned char)(color_table[pat->pattern[i].ind].red*255/cgm.color_ext.white.red),
                              (unsigned char)(color_table[pat->pattern[i].ind].green*255/cgm.color_ext.white.green),
                              (unsigned char)(color_table[pat->pattern[i].ind].blue*255/cgm.color_ext.white.blue) );
    }
    
    cdPattern( pat->nx, pat->ny, (long *) p );
    
    return CD_FILL;
  }
  else if ( style==HATCH )
  {
    cdHatch ( fill_att.hatch_index-1 );
    return CD_FILL;
  }
  else
    return CD_CLOSED_LINES;
}

long int getcolor ( tcolor cor )
{
  
  if ( cgm.clrsm==INDEXED )
    return cdEncodeColor ((unsigned char)(color_table[cor.ind].red*255/cgm.color_ext.white.red),
                          (unsigned char)(color_table[cor.ind].green*255/cgm.color_ext.white.green),
                          (unsigned char)(color_table[cor.ind].blue*255/cgm.color_ext.white.blue) );
  else
    return cdEncodeColor ((unsigned char)(cor.rgb.red*255/cgm.color_ext.white.red),
                          (unsigned char)(cor.rgb.green*255/cgm.color_ext.white.green),
                          (unsigned char)(cor.rgb.blue*255/cgm.color_ext.white.blue) );
}

int vdcx2canvas ( double vdc )
{
  if ( cgm.drawing_mode==ABSTRACT )
    return (int) floor ( scale_factor_x*(vdc-vdc_ext.xmin)+.5 ) + xmin;
  else
    return (int) floor ( scale_factor_mm_x*(vdc-vdc_ext.xmin)*cgm.scaling_mode.scale_factor +
                         vdc_ext.xmin*cgm.scaling_mode.scale_factor + .5 );
}

int vdcy2canvas ( double vdc )
{
  if ( cgm.drawing_mode==ABSTRACT )
    return (int) floor ( scale_factor_y*(vdc-vdc_ext.ymin)+.5 ) + ymin;
  else
    return (int) floor ( scale_factor_mm_y*(vdc-vdc_ext.ymin)*cgm.scaling_mode.scale_factor +
                         vdc_ext.ymin*cgm.scaling_mode.scale_factor + .5 );
}

int delta_vdc2canvas ( double vdc )
{
  int delta = (int) vdcx2canvas(vdc_ext.xmin+vdc) - (int) vdcx2canvas(vdc_ext.xmin);
  return delta;
}

double canvas2vdcx ( int x )
{
  if ( cgm.drawing_mode==ABSTRACT )
    return (double) (x-xmin)/scale_factor_x + vdc_ext.xmin;
  else
    return (double) (x-xmin)/(scale_factor_mm_x*cgm.scaling_mode.scale_factor) + vdc_ext.xmin;
}

double canvas2vdcy ( int y )
{
  if ( cgm.drawing_mode==ABSTRACT )
    return (double) (y-ymin)/scale_factor_y + vdc_ext.ymin;
  else
    return (double) (y-ymin)/(scale_factor_mm_y*cgm.scaling_mode.scale_factor) + vdc_ext.ymin;
}

void GetPolyBbox ( tpoint *pt, int n_points, double *bb_xmin, double *bb_ymin,
                  double *bb_xmax, double *bb_ymax )
{
  int i;
  
  *bb_xmin = *bb_xmax = pt[0].x;
  *bb_ymin = *bb_ymax = pt[0].y;
  
  for ( i=1; i<n_points; i++)
  {
    if ( pt[i].x < *bb_xmin ) *bb_xmin = pt[i].x;
    else if ( pt[i].x > *bb_xmax ) *bb_xmax = pt[i].x;
    if ( pt[i].y < *bb_ymin ) *bb_ymin = pt[i].y;
    else if ( pt[i].y > *bb_ymax ) *bb_ymax = pt[i].y;
  }
}

void GetIncPolyBbox ( tpoint *pt, int n_points, double *bb_xmin, double *bb_ymin,
                     double *bb_xmax, double *bb_ymax )
{
  int i;
  double px, py;
  
  px = *bb_xmin = *bb_xmax = pt[0].x;
  py = *bb_ymin = *bb_ymax = pt[0].y;
  
  for ( i=1; i<n_points; i++)
  {
    px += pt[i].x;   py += pt[i].y;
    if ( px < *bb_xmin ) *bb_xmin = px;
    else if ( px > *bb_xmax ) *bb_xmax = px;
    if ( py < *bb_ymin ) *bb_ymin = py;
    else if ( py > *bb_ymax ) *bb_ymax = py;
  }
}

unsigned char *my_realloc ( unsigned char *s, size_t size )
{
  
  if ( s == NULL )
    s = (unsigned char *) malloc ( size );
  else
    s = (unsigned char *) realloc ( s, size );
  
  return s;
}

int setfont ( int font, int style, double height )
{
  int size;
  int cy;
  
  cy = delta_vdc2canvas ( height );
  size = (int) floor (((cy/scale_factor_mm_y)/0.353)+0.5);
  
  cdFont( font, style, size );
  
  return 0;
}
