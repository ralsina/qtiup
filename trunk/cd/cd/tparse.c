#define _INTCGM2_C_

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <cd.h>
#include <cdcgm.h>

#include "list.h"
#include "types.h"
#include "bparse.h"
#include "intcgm.h"
#include "intcgm2.h"
#include "intcgm4.h"
#include "intcgm6.h"

int cgmt_begmtf ( void ) /* begin metafile */
{
 char *str=NULL;

 if ( cgmt_s ( &str ) ) return 1;

 if (cdcgmbegmtfcb)
  {
   int err;
   err = cdcgmbegmtfcb ( CD_CGM, &view_xmin, &view_ymin, &view_xmax, &view_ymax );
   view_clip = 0;
   if ( err==CD_ABORT ) return -1; 
  }

 free(str);

 return cgmt_ter();
}

int cgmt_endmtf ( void ) /* end metafile */
{
 cgmt_ter();
 return 1;
}

int cgmt_begpic ( void ) /* begin picture */
{
 char *string=NULL;
 
 if ( cgm.first ) 
  cgm.first = 0;
 else
  cdFlush();
  
 cdClip(CD_CLIPAREA);
 
 if ( cgmt_s ( &string ) ) return 1;

  if (cdcgmbegpictcb)
  {
   int err;
   err = cdcgmbegpictcb ( CD_CGM, string );
   if ( err==CD_ABORT ) return -1; 
  }

 free(string);

 return cgmt_ter();
}

int cgmt_begpib ( void ) /* begin picture body */
{
 if (cdcgmbegpictbcb)
  {
   int err;
   err = cdcgmbegpictbcb ( CD_CGM, 1., 1., scale_factor_x, scale_factor_y,
	                       scale_factor_mm_x*cgm.scaling_mode.scale_factor,
	                       scale_factor_mm_y*cgm.scaling_mode.scale_factor,
						   cgm.drawing_mode,
						   vdc_ext.xmin, vdc_ext.ymin, vdc_ext.xmax, vdc_ext.ymax );
   if ( err==CD_ABORT ) return -1; 
  }


 if (cdsizecb)
  {
   int err, w, h;
   double w_mm=0., h_mm=0.;

   w = xmax-xmin;
   h = ymax-ymin;

   if ( cgm.vdc_type==INTEGER )
    {
	 w = (int) (cgm.vdc_ext.second.x - cgm.vdc_ext.first.x);
	 h = (int) (cgm.vdc_ext.second.y - cgm.vdc_ext.first.y);
	 if ( cgm.scaling_mode.mode==METRIC )
	  {
	   w_mm = w * cgm.scaling_mode.scale_factor;
	   h_mm = h * cgm.scaling_mode.scale_factor;
	  }
	}
   else
    {
	 if ( cgm.scaling_mode.mode==METRIC )
	  {
	   w_mm = (cgm.vdc_ext.second.x - cgm.vdc_ext.first.x) * cgm.scaling_mode.scale_factor;
	   h_mm = (cgm.vdc_ext.second.y - cgm.vdc_ext.first.y) * cgm.scaling_mode.scale_factor;
	  }
	}

   err = cdsizecb ( CD_CGM, w, h, w_mm, h_mm );
   if ( err==CD_ABORT ) return -1; 
  }

 return cgmt_ter();
}

int cgmt_endpic ( void ) /* end picture */
{
 return cgmt_ter();
}

int cgmt_mtfver ( void )   /* metafile version */
{
 long version;

 if ( cgmt_i ( &version ) ) return 1;

 return cgmt_ter();
}

int cgmt_mtfdsc ( void )   /* metafile description */
{
 char *string=NULL;

 if ( cgmt_s ( &string ) ) return 1;

 free(string);

 return cgmt_ter();
}

int cgmt_vdctyp ( void )  /* vdc type */
{
 const char *options[] = { "integer", "real", NULL };

 if ( cgmt_e ( &(cgm.vdc_type), options ) ) return 1;

 return cgmt_ter();
}

int cgmt_intpre ( void )   /* integer precision */
{
 if ( cgmt_i ( &(cgm.int_prec.t_prec.minint) ) ) return 1;
 if ( cgmt_i ( &(cgm.int_prec.t_prec.maxint) ) ) return 1;

 return cgmt_ter();
}

int cgmt_realpr ( void )   /* real precision */
{
 if ( cgmt_r ( &(cgm.real_prec.t_prec.minreal) ) ) return 1;
 if ( cgmt_r ( &(cgm.real_prec.t_prec.maxreal) ) ) return 1;
 if ( cgmt_i ( &(cgm.real_prec.t_prec.digits) ) ) return 1;

 return cgmt_ter();
}

int cgmt_indpre ( void )   /* index precision */
{
 if ( cgmt_i ( &(cgm.ix_prec.t_prec.minint) ) ) return 1;
 if ( cgmt_i ( &(cgm.ix_prec.t_prec.maxint) ) ) return 1;

 return cgmt_ter();
}

int cgmt_colpre ( void )   /* colour precision */
{
 if ( cgmt_i ( &(cgm.cd_prec) ) ) return 1;

 return cgmt_ter();
}

int cgmt_colipr ( void )   /* colour index precision */
{
 if ( cgmt_i ( &(cgm.cix_prec) ) ) return 1;

 return cgmt_ter();
}

int cgmt_maxcoi ( void )   /* maximum colour index */
{
 if ( cgmt_i ( &(cgm.max_cix) ) ) return 1;

 color_table = (trgb *) realloc ( color_table, sizeof(trgb)*(cgm.max_cix+1) );

 color_table[0].red   = 255;
 color_table[0].green = 255;
 color_table[0].blue  = 255;
 color_table[1].red   = 0;
 color_table[1].green = 0;
 color_table[1].blue  = 0;

 return cgmt_ter();
}

int cgmt_covaex ( void )   /* colour value extent */
{
 if ( cgmt_rgb ( &(cgm.color_ext.black.red), &(cgm.color_ext.black.green), &(cgm.color_ext.black.blue) ) ) return 1;

 if ( cgmt_rgb ( &(cgm.color_ext.white.red), &(cgm.color_ext.white.green), &(cgm.color_ext.white.blue) ) ) return 1;

 return cgmt_ter();
}

int cgmt_mtfell ( void )   /* metafile element list */
{
 char *elist=NULL;

 if ( cgmt_s ( &elist ) ) return 1;

 free(elist);

 return cgmt_ter();
}

int cgmt_bmtfdf (void )   /* begin metafile defaults */
{
 return cgmt_ter();
}

int cgmt_emtfdf ( void )   /* end metafile defaults */
{
 return cgmt_ter();
}

int cgmt_fntlst ( void )   /* font list */
{
 char *font=NULL;

 if ( text_att.font_list==NULL ) text_att.font_list = NewList();

 while ( cgmt_ter() ) 
  {
   if ( cgmt_s ( &font ) ) return 1;
   AppendList ( text_att.font_list, font );
  }

 return 0;
}

int cgmt_chslst ( void )  /* character set list */
{
 const char *options[] = { "std94", "std96", "std94multibyte", "std96multibyte",
                           "completecode", NULL };
 char *tail;
 short code;

 do
  {
   if ( cgmt_e ( &(code), options ) ) return 1;

   if ( cgmt_s ( &tail ) ) return 1;

   free ( tail );

  } while ( cgmt_ter() );

 return 0;
}

int cgmt_chcdac ( void )   /* character coding announcer */
{
 const char *options[] = { "basic7bit", "basic8bit", "extd7bit", "extd8bit", NULL };
 short code;

 if ( cgmt_e ( &(code), options ) ) return 1;

 return cgmt_ter ();
}

int cgmt_sclmde ( void )   /* scaling mode */
{
 const char *options[] = { "abstract", "metric", NULL };

 if ( cgmt_e ( &(cgm.scaling_mode.mode), options ) ) return 1;

 if ( cgmt_r ( &(cgm.scaling_mode.scale_factor) ) ) return 1;

 if ( cgm.scaling_mode.mode==ABSTRACT ) cgm.scaling_mode.scale_factor=1.;

 cgm.drawing_mode = ABSTRACT;
  
 if (cdcgmsclmdecb) 
  {
   int err;
   err = cdcgmsclmdecb ( CD_CGM, cgm.scaling_mode.mode, &cgm.drawing_mode, &cgm.scaling_mode.scale_factor );
   if ( err==CD_ABORT ) return -1;
  }

 if ( cgm.drawing_mode==ABSTRACT )
  {
   clip_xmin = canvas2vdcx ( view_xmin );
   clip_xmax = canvas2vdcx ( view_xmax );
   clip_ymin = canvas2vdcy ( view_ymin );
   clip_ymax = canvas2vdcy ( view_ymax );
  }
 else
  {
   clip_xmin = vdc_ext.xmin*cgm.scaling_mode.scale_factor;
   clip_xmax = vdc_ext.xmax*cgm.scaling_mode.scale_factor;
   clip_ymin = vdc_ext.ymin*cgm.scaling_mode.scale_factor;
   clip_ymax = vdc_ext.ymax*cgm.scaling_mode.scale_factor;
  }
  
 return cgmt_ter();
}

int cgmt_clslmd ( void )   /* colour selection mode */
{
 const char *options[] = { "indexed", "direct", NULL };

 if ( cgmt_e ( &(cgm.clrsm), options ) ) return 1;

 return cgmt_ter();
}

int cgmt_lnwdmd ( void )   /* line width specification mode */
{
 const char *options[] = { "abstract", "scaled", NULL };

 if ( cgmt_e ( &(cgm.lnwsm), options ) ) return 1;

 return cgmt_ter();
}

int cgmt_mkszmd ( void )   /* marker size specification mode */
{
 const char *options[] = { "abstract", "scaled", NULL };

 if ( cgmt_e ( &(cgm.mkssm), options ) ) return 1;

 return cgmt_ter();
}

int cgmt_edwdmd ( void )   /* edge width specification mode */
{
 const char *options[] = { "abstract", "scaled", NULL };

 if ( cgmt_e ( &(cgm.edwsm), options ) ) return 1;

 return cgmt_ter();
}

int cgmt_vdcext ( void )   /* vdc extent */
{
 if ( cgmt_p ( &(cgm.vdc_ext.first.x), &(cgm.vdc_ext.first.y) ) ) return 1;

 if ( cgmt_p ( &(cgm.vdc_ext.second.x), &(cgm.vdc_ext.second.y) ) ) return 1;

 if (cdcgmvdcextcb)
  {
   int err;
   err = cdcgmvdcextcb( CD_CGM, cgm.vdc_type, &(cgm.vdc_ext.first.x), &(cgm.vdc_ext.first.y),
                                              &(cgm.vdc_ext.second.x), &(cgm.vdc_ext.second.y) );
   if (err==CD_ABORT) return -1;
  }

 do_vdcext ( cgm.vdc_ext.first, cgm.vdc_ext.second );

 return cgmt_ter();
}

int cgmt_bckcol ( void )   /* backgound colour */
{
 if ( cgmt_rgb ( &(cgm.back_color.red), &(cgm.back_color.green), &(cgm.back_color.blue) ) ) return 1;

 do_bckcol ( cgm.back_color );

 return cgmt_ter();
}

int cgmt_vdcipr ( void )   /* vdc integer precision */
{
 if ( cgmt_i ( &(cgm.vdc_int.t_prec.minint) ) ) return 1;
 if ( cgmt_i ( &(cgm.vdc_int.t_prec.maxint) ) ) return 1;

 return cgmt_ter();
}

int cgmt_vdcrpr ( void )   /* vdc real precision */
{
 if ( cgmt_r ( &(cgm.vdc_real.t_prec.minreal) ) ) return 1;
 if ( cgmt_r ( &(cgm.vdc_real.t_prec.maxreal) ) ) return 1;
 if ( cgmt_i ( &(cgm.vdc_real.t_prec.digits) ) ) return 1;

 return cgmt_ter();
}

int cgmt_auxcol ( void )   /* auxiliary colour */
{
 if ( cgmt_co ( &(cgm.aux_color) ) ) return 1;

 return cgmt_ter();
}

int cgmt_transp ( void )   /* transparency */
{
 const char *options[] = { "on", "off", NULL };

 if ( cgmt_e ( &(cgm.transparency), options ) ) return 1;

 do_transp ( cgm.transparency );

 return cgmt_ter();
}

int cgmt_clprec ( void )   /* clip rectangle */
{
 if ( cgmt_p ( &(cgm.clip_rect.first.x),  &(cgm.clip_rect.first.y) ) ) return 1;

 if ( cgmt_p ( &(cgm.clip_rect.second.x),  &(cgm.clip_rect.second.y) ) ) return 1;

 do_clprec ( cgm.clip_rect.first, cgm.clip_rect.second );

 return cgmt_ter();
}

int cgmt_clpind ( void )   /* clip indicator */
{
 const char *options[] = { "on", "off", NULL };

 if ( cgmt_e ( &(cgm.clip_ind), options ) ) return 1;

 do_clpind ( cgm.clip_ind );

 return cgmt_ter();
}

static tpoint *get_points ( int *np )
{
 *np=0;

 do
  {
   if ( cgmt_p ( &(point_list[*np].x), &(point_list[*np].y) ) ) return NULL;
   ++(*np);
   if ( *np==npoints)
     {
      npoints *= 2;
      point_list = (tpoint *) realloc ( point_list, npoints*sizeof(tpoint) );
     }
  } while ( cgmt_ter() );

 return point_list;
}

int cgmt_polyln ( void )   /* polyline */
{
 tpoint *pt;
 int np;

 pt = get_points( &np );
 if ( pt == NULL ) return 1;

 do_polyln ( np, pt );

 return 0;
}

int cgmt_incply ( void )   /* incremental polyline */
{
 tpoint *pt;
 int np;

 pt = get_points( &np );
 if ( pt == NULL ) return 1;

 do_incply ( np, pt );

 return 0;
}

int cgmt_djtply ( void )   /* disjoint polyline */
{
 tpoint *pt;
 int np;

 pt = get_points( &np );
 if ( pt == NULL ) return 1;

 do_djtply ( np, pt );

 return 0;
}

int cgmt_indjpl ( void )   /* incremental disjoint polyline */
{
 tpoint *pt;
 int np;

 pt = get_points( &np );
 if ( pt == NULL ) return 1;

 do_indpl ( np, pt );

 return 0;
}

int cgmt_polymk ( void )   /* polymarker */
{
 tpoint *pt;
 int np;

 pt = get_points( &np );
 if ( pt == NULL ) return 1;

 do_polymk ( np, pt );

 return 0;
}

int cgmt_incplm ( void )   /* incremental polymarker */
{
 tpoint *pt;
 int np;

 pt = get_points( &np );
 if ( pt == NULL ) return 1;

 do_incplm ( np, pt );

 return 0;
}

int cgmt_text ( void )   /* text */
{
 tpoint pos;
 const char *options[] = { "final", "notfinal", NULL };
 short flag;
 char *string;

 if ( cgmt_p ( &(pos.x), &(pos.y) ) ) return 1;

 if ( cgmt_e ( &flag, options ) )  return 1;

 if ( cgmt_s ( &string ) ) return 1;

 do_text ( NORM_TEXT, string, pos );

 free ( string );

 return cgmt_ter();
}

int cgmt_rsttxt ( void )   /* restricted text */
{
 double width, height;
 tpoint pos;
 const char *options[] = { "final", "notfinal", NULL };
 short flag;
 char *string;

 if ( cgmt_vdc ( &width ) ) return 1;

 if ( cgmt_vdc ( &height ) ) return 1;

 if ( cgmt_p ( &(pos.x), &(pos.y) ) ) return 1;

 if ( cgmt_e ( &flag, options ) ) return 1;

 if ( cgmt_s ( &string ) ) return 1;

 text_att.height = height;

 do_text ( RESTRICTED_TEXT, string, pos );

 if ( string!= NULL ) free ( string );

 return cgmt_ter();
}

int cgmt_apdtxt ( void )   /* append text */
{
 const char *options[] = { "final", "notfinal", NULL };
 short flag;
 char *string;

 if ( cgmt_e ( &flag, options ) ) return 1;

 if ( cgmt_s ( &string ) ) return 1;

 free ( string );

 return cgmt_ter();
}

int cgmt_polygn ( void )   /* polygon */
{
 tpoint *pt;
 int np;

 pt = get_points( &np );
 if ( pt == NULL ) return 1;

 do_polygn ( np, pt );

 return 0;
}

int cgmt_incplg ( void )   /* incremental polygon */
{
 tpoint *pt;
 int np;

 pt = get_points( &np );
 if ( pt == NULL ) return 1;

 do_incplg ( np, pt );

 return 0;
}

static int get_point_set ( tpoint **pts, short **flags, int *np )
{
 int block=500;
 const char *options[] = { "invis", "vis", "closeinvis", "closevis", NULL };

 *np=0;
 *pts = (tpoint *) malloc ( block*sizeof(tpoint) );
 *flags = (short *) malloc ( block*sizeof(short) );

 do
  {
   if ( cgmt_p ( &((*pts)[*np].x), &((*pts)[*np].y) ) ) return 1;

   if ( cgmt_e ( &((*flags)[*np]), options ) ) return 1;

   ++(*np);
   if ( *np==block)
    {
     block *= 2;
     *pts = (tpoint *) realloc ( *pts, block*sizeof(tpoint) );
     *flags = (short *) realloc ( *flags, block*sizeof(short) );
    }
  } while ( cgmt_ter() );

 return 0;
}

int cgmt_plgset ( void )   /* polygon set */
{
 tpoint *pt;
 short *flag;
 int np;

 if ( get_point_set ( &pt, &flag, &np ) ) return 1;

 do_plgset( NO, np, pt, flag );

 free ( pt );
 free ( flag );

 return 0;
}

int cgmt_inpgst ( void )   /* incremental polygon set */
{
 tpoint *pt;
 short *flag;
 int np;

 if ( get_point_set ( &pt, &flag, &np ) ) return 1;

 do_plgset( YES, np, pt, flag );

 free ( pt );
 free ( flag );

 return 0;
}

int cgmt_cellar ( void )   /* cell array */
{
 tpoint corner1;
 tpoint corner2;
 tpoint corner3;
 long nx, ny;
 long local_color_prec;
 tcolor *cell;
 int i,k;

 if ( cgmt_p ( &(corner1.x), &(corner1.y) ) ) return 1;
 if ( cgmt_p ( &(corner2.x), &(corner2.y) ) ) return 1;
 if ( cgmt_p ( &(corner3.x), &(corner3.y) ) ) return 1;

 if ( cgmt_i ( &nx ) ) return 1;
 if ( cgmt_i ( &ny ) ) return 1;

 if ( cgmt_i ( &(local_color_prec) ) ) return 1;

 cell = (tcolor *) malloc ( nx*ny*sizeof(tcolor) );
 
 cgmt_getparentheses();

  for ( k=0; k<ny; k++ )
   {
    for ( i=0; i<nx; i++ )
     {
      if ( cgmt_co ( &(cell[k*nx+i]) ) ) return 1;
	   }
    GetFilePos();
   }

 cgmt_getparentheses();

 do_cellar ( corner1, corner2, corner3, nx, ny, local_color_prec, cell );

 free(cell);

 return cgmt_ter();
}

int cgmt_gdp ( void )   /* generalized drawing picture */
{
 long identifier;
 tpoint *pt = NULL;
 int block = 500;
 int np = 0;
 char *data_rec;

 pt = (tpoint *) malloc ( block*sizeof(tpoint) );

 if ( cgmt_i ( &identifier ) ) return 1;

 while ( strstr(cgmt_getsep(),",")==NULL )
  {
   if ( cgmt_p ( &(pt[np].x), &(pt[np].y) ) )
    {
	 if ( pt!=NULL ) free(pt);
     return 1;
	}
   ++np;

   if ( np==block )
    {
     block *= 2;
     pt = (tpoint *) realloc ( pt, block*sizeof(tpoint) );
    }
  }

 if ( cgmt_s ( &data_rec ) ) return 1;

 do_gdp ( identifier, pt, data_rec );

 free ( data_rec );
 free ( pt );

 return cgmt_ter();
}

int cgmt_rect ( void )   /* rectangle */
{
 tpoint point1;
 tpoint point2;

 if ( cgmt_p ( &(point1.x), &(point1.y) ) ) return 1;
 if ( cgmt_p ( &(point2.x), &(point2.y) ) ) return 1;

 do_rect ( point1, point2 );

 return cgmt_ter();
}

int cgmt_circle ( void )   /* circle */
{
 tpoint center;
 double radius;

 if ( cgmt_p ( &(center.x), &(center.y) ) ) return 1;

 if ( cgmt_vdc ( &radius ) ) return 1;

 do_circle ( center, radius );

 return cgmt_ter();
}

int cgmt_circ3p ( void )   /* circular arc 3 point */
{
 tpoint starting;
 tpoint intermediate;
 tpoint ending;

 if ( cgmt_p ( &(starting.x), &(starting.y) ) ) return 1;
 if ( cgmt_p ( &(intermediate.x), &(intermediate.y) ) ) return 1;
 if ( cgmt_p ( &(ending.x), &(ending.y) ) ) return 1;

 do_circ3p ( starting, intermediate, ending );

 return cgmt_ter();
}

int cgmt_cir3pc ( void )  /* circular arc 3 point close */
{
 tpoint starting;
 tpoint intermediate;
 tpoint ending;
 const char *options[] = { "pie", "chord", NULL };
 short close_type;

 if ( cgmt_p ( &(starting.x), &(starting.y) ) ) return 1;
 if ( cgmt_p ( &(intermediate.x), &(intermediate.y) ) ) return 1;
 if ( cgmt_p ( &(ending.x), &(ending.y) ) ) return 1;

 if ( cgmt_e ( &close_type, options ) ) return 1;

 do_circ3pc ( starting, intermediate, ending, close_type );

 return cgmt_ter();
}

int cgmt_circnt ( void )   /* circular arc centre */
{
 tpoint center;
 tpoint start;
 tpoint end;
 double  radius;

 if ( cgmt_p ( &(center.x), &(center.y) ) ) return 1;

 if ( cgmt_vdc ( &(start.x) ) ) return 1;
 if ( cgmt_vdc ( &(start.y) ) ) return 1;

 if ( cgmt_vdc ( &(end.x) ) ) return 1;
 if ( cgmt_vdc ( &(end.y) ) ) return 1;

 if ( cgmt_vdc ( &radius ) ) return 1;

 do_circcnt ( center, start, end, radius );

 return cgmt_ter();
}

int cgmt_ccntcl ( void )   /* circular arc centre close */
{
 tpoint center;
 tpoint start;
 tpoint end;
 double radius;
 const char *options[] = { "pie", "chord", NULL };
 short close_type;

 if ( cgmt_p ( &(center.x), &(center.y) ) ) return 1;

 if ( cgmt_vdc ( &(start.x) ) ) return 1;
 if ( cgmt_vdc ( &(start.y) ) ) return 1;

 if ( cgmt_vdc ( &(end.x) ) ) return 1;
 if ( cgmt_vdc ( &(end.y) ) ) return 1;

 if ( cgmt_vdc ( &radius ) ) return 1;

 if ( cgmt_e ( &close_type, options ) ) return 1;

 do_ccntcl ( center, start, end, radius, close_type );

 return cgmt_ter();
}

int cgmt_ellips ( void )   /* ellipse */
{
 tpoint center;
 tpoint first_CDP;
 tpoint second_CDP;

 if ( cgmt_p ( &(center.x), &(center.y) ) ) return 1;

 if ( cgmt_p ( &(first_CDP.x), &(first_CDP.y) ) ) return 1;
 if ( cgmt_p ( &(second_CDP.x), &(second_CDP.y) ) ) return 1;

 do_ellips ( center, first_CDP, second_CDP );

 return cgmt_ter();
}

int cgmt_ellarc ( void )   /* elliptical arc */
{
 tpoint center;
 tpoint first_CDP;
 tpoint second_CDP;
 tpoint start, end;

 if ( cgmt_p ( &(center.x), &(center.y) ) ) return 1;

 if ( cgmt_p ( &(first_CDP.x), &(first_CDP.y) ) ) return 1;
 if ( cgmt_p ( &(second_CDP.x), &(second_CDP.y) ) ) return 1;

 if ( cgmt_vdc ( &(start.x) ) ) return 1;
 if ( cgmt_vdc ( &(start.y) ) ) return 1;

 if ( cgmt_vdc ( &(end.x) ) ) return 1;
 if ( cgmt_vdc ( &(end.y) ) ) return 1;

 do_ellarc ( center, first_CDP, second_CDP, start, end );

 return cgmt_ter();
}

int cgmt_ellacl ( void )   /* elliptical arc close */
{
 tpoint center;
 tpoint first_CDP;
 tpoint second_CDP;
 tpoint start, end;
 const char *options[] = { "pie", "chord", NULL };
 short close_type;

 if ( cgmt_p ( &(center.x), &(center.y) ) ) return 1;

 if ( cgmt_p ( &(first_CDP.x), &(first_CDP.y) ) ) return 1;
 if ( cgmt_p ( &(second_CDP.x), &(second_CDP.y) ) ) return 1;

 if ( cgmt_vdc ( &(start.x) ) ) return 1;
 if ( cgmt_vdc ( &(start.y) ) ) return 1;

 if ( cgmt_vdc ( &(end.x) ) ) return 1;
 if ( cgmt_vdc ( &(end.y) ) ) return 1;

 if ( cgmt_e ( &close_type, options ) ) return 1;

 do_ellacl ( center, first_CDP, second_CDP, start, end, close_type );

 return cgmt_ter();
}

int cgmt_lnbdin ( void )   /* line bundle index */
{
 if ( cgmt_i ( &(line_att.index) ) ) return 1;

 return cgmt_ter();
}

int cgmt_lntype ( void )   /* line type */
{
 if ( cgmt_i ( &(line_att.type) ) ) return 1;

 return cgmt_ter();
}

int cgmt_lnwidt ( void )  /* line width */
{
 if ( cgm.lnwsm==0 )
  {
   if ( cgmt_vdc ( &(line_att.width) ) ) return 1;
  }
 else
  {

   if ( cgmt_r ( &(line_att.width) ) ) return 1;
  }

 return cgmt_ter();
}

int cgmt_lncolr ( void )   /* line colour */
{
 if ( cgmt_co ( &(line_att.color) ) ) return 1;

 return cgmt_ter();
}

int cgmt_mkbdin ( void )   /* marker bundle index */
{
 if ( cgmt_i ( &(marker_att.index) ) ) return 1;

 return cgmt_ter();
}

int cgmt_mktype ( void )   /* marker type */
{
 if ( cgmt_i ( &(marker_att.type) ) ) return 1;

 return cgmt_ter();
}

int cgmt_mksize ( void )   /* marker size */
{
 if ( cgm.mkssm == 0 )
  {
   if ( cgmt_vdc ( &(marker_att.size) ) ) return 1;
  }
 else
  {
   if ( cgmt_r ( &(marker_att.size) ) ) return 1;
  }

 return cgmt_ter();
}

int cgmt_mkcolr ( void )   /* marker colour */
{
 if ( cgmt_co ( &(marker_att.color) ) ) return 1;

 return cgmt_ter();
}

int cgmt_txbdin ( void )   /* text bundle index */
{
 if ( cgmt_i ( &(text_att.index) ) ) return 1;

 return cgmt_ter();
}

int cgmt_txftin ( void )   /* text font index */
{
 char *font;
 char *font_array[] = {"SYSTEM", "COURIER", "TIMES", "HELVETICA", NULL};
 int cdfont[] = {CD_SYSTEM, CD_COURIER, CD_TIMES_ROMAN, CD_HELVETICA};
 char *style_array[] = {"BOLDITALIC", "ITALIC", "BOLD", "PLAIN", NULL};
 int cdstyle[] = {CD_BOLD_ITALIC, CD_ITALIC, CD_BOLD, CD_PLAIN};
 int i;

 if ( cgmt_i ( &(text_att.font_index) ) ) return 1;

 font = (char *) GetList ( text_att.font_list, text_att.font_index );

 if ( font==NULL ) font = (char*)strdup ( "SYSTEM" );

 text_att.font = 0;
 for ( i=0; font_array[i]!=NULL; i++ )
  {
   if ( strstr( font, font_array[i] ) )
    {
     text_att.font = cdfont[i];
     break;
    }
  }

 text_att.style = 0;
 for ( i=0; style_array[i]!=NULL; i++ )
  {
   if ( strstr( font, style_array[i] ) )
    {
     text_att.style = cdstyle[i];
     break;
    }
  }

 setfont ( text_att.font, text_att.style, text_att.height );

 return cgmt_ter();
}

int cgmt_txtprc ( void )   /* text precision */
{
 const char *options[] = { "string", "char", "stroke", NULL };

 if ( cgmt_e ( &(text_att.prec), options ) ) return 1;

 return cgmt_ter();
}

int cgmt_chrexp ( void )   /* char expansion factor */
{
 if ( cgmt_r ( &(text_att.exp_fact) ) ) return 1;

 return cgmt_ter();
}

int cgmt_chrspc ( void )   /* char spacing */
{
 if ( cgmt_r ( &(text_att.char_spacing) ) ) return 1;

 return cgmt_ter();
}

int cgmt_txtclr ( void )   /* text colour */
{
 if ( cgmt_co ( &(text_att.color) ) ) return 1;

 return cgmt_ter();
}

int cgmt_chrhgt ( void )   /* char height */
{
 if ( cgmt_vdc ( &(text_att.height) ) ) return 1;

 do_text_height ( text_att.height );

 return cgmt_ter();
}

int cgmt_chrori ( void )   /* char orientation */
{
 if ( cgmt_vdc ( &(text_att.char_up.x) ) ) return 1;
 if ( cgmt_vdc ( &(text_att.char_up.y) ) ) return 1;
 if ( cgmt_vdc ( &(text_att.char_base.x) ) ) return 1;
 if ( cgmt_vdc ( &(text_att.char_base.y) ) ) return 1;

 return cgmt_ter();
}

int cgmt_txtpat ( void )   /* text path */
{
 const char *options[] = { "right", "left", "up", "down", NULL };

 if ( cgmt_e ( &(text_att.path), options ) ) return 1;

 return cgmt_ter();
}

int cgmt_txtali ( void )   /* text alignment */
{
 const char *hor[] = { "normhoriz", "left", "ctr", "right", "conthoriz", NULL };
 const char *ver[] = { "normvert", "top", "cap", "half", "base", "bottom",
                       "contvert", NULL };

 if ( cgmt_e ( &(text_att.alignment.hor), hor ) ) return 1;
 if ( cgmt_e ( &(text_att.alignment.ver), ver ) ) return 1;

 if ( cgmt_r ( &(text_att.alignment.cont_hor) ) ) return 1;
 if ( cgmt_r ( &(text_att.alignment.cont_ver) ) ) return 1;

 do_txtalign ( text_att.alignment.hor, text_att.alignment.ver );

 return cgmt_ter();
}

int cgmt_chseti ( void )   /* character set index */
{
 long set;

 if ( cgmt_i ( &set ) ) return 1;

 return cgmt_ter();
}

int cgmt_achsti ( void )   /* alternate character set index */
{
 long set;

 if ( cgmt_i ( &set ) ) return 1;

 return cgmt_ter();
}

int cgmt_fillin ( void )   /* fill bundle index */
{
 if ( cgmt_i ( &(fill_att.index) ) ) return 1;

 return cgmt_ter();
}

int cgmt_intsty ( void )   /* interior style */
{
 const char *options[] = { "hollow", "solid", "pat", "hatch", "empty", NULL };

 if ( cgmt_e ( &(fill_att.int_style), options ) ) return 1;

 return cgmt_ter();
}

int cgmt_fillco ( void )   /* fill colour */
{
 if ( cgmt_co ( &(fill_att.color) ) ) return 1;

 return cgmt_ter();
}

int cgmt_hatind ( void )   /* hatch index */
{
 if ( cgmt_i ( &(fill_att.hatch_index) ) ) return 1;
 if ( fill_att.hatch_index==3 ) fill_att.hatch_index = 4;
 else if ( fill_att.hatch_index==4 ) fill_att.hatch_index = 3;

 return cgmt_ter();
}

int cgmt_patind ( void )   /* pattern index */
{
 if ( cgmt_i ( &(fill_att.pat_index) ) ) return 1;

 return cgmt_ter();
}

int cgmt_edgind ( void )   /* edge bundle index */
{
 if ( cgmt_i ( &(edge_att.index) ) ) return 1;

 return cgmt_ter();
}

int cgmt_edgtyp( void )   /* edge type */
{
 if ( cgmt_i ( &(edge_att.type) ) ) return 1;

 return cgmt_ter();
}

int cgmt_edgwid ( void )   /* edge width */
{
 if ( cgm.edwsm==0 )
  {
   if ( cgmt_vdc ( &(edge_att.width) ) ) return 1;
  }
 else
  {
   if ( cgmt_r ( &(edge_att.width) ) ) return 1;
  }

 return cgmt_ter();
}

int cgmt_edgcol ( void )   /* edge colour */
{
 if ( cgmt_co ( &(edge_att.color) ) ) return 1;

 return cgmt_ter();
}

int cgmt_edgvis ( void )   /* edge visibility */
{
 const char *options[] = { "off", "on", NULL };

 if ( cgmt_e ( &(edge_att.visibility), options ) ) return 1;

 return cgmt_ter();
}

int cgmt_fillrf ( void )   /* fill reference point */
{
 if ( cgmt_p ( &(fill_att.ref_pt.x),  &(fill_att.ref_pt.y) ) ) return 1;

 return cgmt_ter();
}

int cgmt_pattab ( void )   /* pattern table */
{
 long local_color_prec;
 int i;
 pat_table *pat, *p;

 pat = (pat_table *) malloc ( sizeof(pat_table) );

 if ( fill_att.pat_list==NULL ) fill_att.pat_list = NewList();

 if ( cgmt_i ( &(pat->index) ) ) return 1;

 if ( cgmt_i ( &(pat->nx) ) ) return 1;
 if ( cgmt_i ( &(pat->ny) ) ) return 1;

 if ( cgmt_i ( &(local_color_prec) ) ) return 1;

 pat->pattern = (tcolor *) malloc ( pat->nx*pat->ny*sizeof(tcolor) );

 cgmt_getparentheses();

 for ( i=0; i<(pat->nx*pat->ny); i++ )
   if ( cgmt_co ( &(pat->pattern[i]) ) ) return 1;

 cgmt_getparentheses();

 for ( i=0; (p=(pat_table *)GetList(fill_att.pat_list,i))!=NULL; i++ )
  {
   if ( p->index==pat->index )
    {
     free(p->pattern);
     DelList(fill_att.pat_list,i);
     break;
    }
  }

 AppendList ( fill_att.pat_list, pat );

 return cgmt_ter();
}

int cgmt_patsiz ( void )   /* pattern size */
{
 if ( cgmt_vdc ( &(fill_att.pat_size.height.x) ) ) return 1;
 if ( cgmt_vdc ( &(fill_att.pat_size.height.y) ) ) return 1;
 if ( cgmt_vdc ( &(fill_att.pat_size.width.x) ) ) return 1;
 if ( cgmt_vdc ( &(fill_att.pat_size.width.y) ) ) return 1;

 return cgmt_ter();
}

int cgmt_coltab ( void )   /* colour table */
{
 long starting_index;

 if ( cgmt_i ( &(starting_index) ) ) return 1;

 while ( cgmt_ter() )
  {
   if ( cgmt_rgb ( &(color_table[starting_index].red), &(color_table[starting_index].green),
                   &(color_table[starting_index].blue) ) ) return 1;
   starting_index++;
  }

 return 0;
}

int cgmt_asf ( void )   /* asfs */
{
 const char *asf_value[] = { "bundled", "indiv", NULL };
 const char *asf_type[] = { "linetype"     , "linewidth" , "linecolr"  ,
                            "markertype"   , "markersize", "markercolr",
                            "textfontindex", "textprec"  , "charexp"   ,
                            "charspace"    , "textcolr"  , "intstyle"  ,
                            "fillcolr"     , "hatchindex", "patindex"  ,
                            "edgetype"     , "edgewidth" , "edgecolr"  ,
                            "all"          , "allline"   , "allmarker" ,
                            "alltext"      , "allfill"   , "alledge", NULL };
 tasf *pair;

 if ( asf_list==NULL ) asf_list = NewList();

 while( cgmt_ter() )
  {
   pair = (tasf *) malloc ( sizeof (tasf) );

   if ( cgmt_e ( &(pair->type), asf_type ) ) return 1;
   if ( cgmt_e ( &(pair->value), asf_value ) ) return 1;

   AppendList ( asf_list, pair );
  }

 return 0;
}

int cgmt_escape ( void )   /* escape */
{
 long identifier;
 char *data_rec;

 if ( cgmt_i ( &(identifier) ) ) return 1;

 if ( cgmt_s ( &data_rec ) ) return 1;

 free(data_rec);
                  
 return cgmt_ter();
}

int cgmt_messag ( void )   /* message */
{
 const char *options[] = { "noaction", "action", NULL };
 char *text;
 short flag;

 if ( cgmt_e ( &flag, options ) ) return 1;

 if ( cgmt_s ( &text ) ) return 1;

 free(text);

 return cgmt_ter();
}

int cgmt_appdta ( void )   /* application data */
{
 long identifier;
 char *data_rec;

 if ( cgmt_i ( &identifier ) ) return 1;

 if ( cgmt_s ( &data_rec ) ) return 1;

 free(data_rec);

 return cgmt_ter();
}
