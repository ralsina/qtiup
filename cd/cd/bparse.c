#include <stdio.h>      /* FILE, ftell, fseek, fputc, fopen, fclose, fputs, fprintf */
#include <string.h>     /* strlen, strncpy */
#include <stdlib.h>     /* malloc, free */
#include "cd.h"
#include "cdcgm.h"
#include "list.h"
#include "types.h"
#include "bparse.h"
#include "intcgm.h"
#include "intcgm2.h"
#include "intcgm4.h"
#include "intcgm6.h"

/*********************
* Delimiter elements *
*********************/

int cgmb_noop ( void )
{
 return 0;
}

int cgmb_begmtf ( void )
{
 char *header = NULL;

 if ( cgm.len )
  {
   if ( cgmb_s ( &header ) ) return 1;
  }

 if (cdcgmbegmtfcb)
  {
   int err;
   err = cdcgmbegmtfcb ( CD_CGM, &view_xmin, &view_ymin, &view_xmax, &view_ymax );
   view_clip = 0;
   if ( err==CD_ABORT ) return -1; 
  }

 return 0;
} 

int cgmb_endmtf ( void )
{
 return 0;
}

int cgmb_begpic ( void )
{
 char *s = NULL;
 
 if ( cgm.first )
   cgm.first = 0;
 else
   cdFlush();
                 
 if ( color_table==NULL )
  {
   if ( cgm.max_cix < 1 ) cgm.max_cix = 1;

   color_table = (trgb *) malloc ( sizeof(trgb)*(cgm.max_cix+1) );

   color_table[0].red   = 255;
   color_table[0].green = 255;
   color_table[0].blue  = 255;
   color_table[1].red   = 0;
   color_table[1].green = 0;
   color_table[1].blue  = 0;
  }

 cdClip(CD_CLIPAREA);
   
 if ( cgmb_s ( &s ) ) return 1;

 if (cdcgmbegpictcb)
  {
   int err;
   err = cdcgmbegpictcb ( CD_CGM, s );
   if ( err==CD_ABORT ) return -1; 
  }

 return 0;
}

int cgmb_begpib ( void )
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

 return 0;
}

int cgmb_endpic ( void )
{
 return 0;
}

/*******************************
* Metafile Descriptor Elements *
*******************************/

int cgmb_mtfver ( void )
{
 long version;

 if ( cgmb_i ( &version ) ) return 1;

 return 0;
}

int cgmb_mtfdsc ( void )
{
 char *s = NULL;

 if ( cgmb_s ( &s ) ) return 1;

 return 0;
}

int cgmb_vdctyp ( void )
{
 if ( cgmb_e   ( &(cgm.vdc_type) ) ) return 1;

 return 0;
}

int cgmb_intpre ( void )
{
 long prec;

 if ( cgmb_i ( &prec ) ) return 1;

 if ( prec==8 ) cgm.int_prec.b_prec = 0;
 else if ( prec==16 )  cgm.int_prec.b_prec = 1; 
 else if ( prec==24 )  cgm.int_prec.b_prec = 2; 
 else if ( prec==32 )  cgm.int_prec.b_prec = 3; 

 return 0;
}

int cgmb_realpr ( void )
{
 short mode, i1;
 long i2, i3;

 if ( cgmb_e ( &i1 ) ) return 1;
 if ( cgmb_i ( &i2 ) ) return 1;
 if ( cgmb_i ( &i3 ) ) return 1;

 if ( i1 == 0 && i2 == 9 && i3 == 23 ) mode = 0;
 else if ( i1 == 0 && i2 == 12 && i3 == 52 ) mode = 1;
 else if ( i1 == 1 && i2 == 16 && i3 == 16 ) mode = 2;
 else if ( i1 == 1 && i2 == 32 && i3 == 32 ) mode = 3;

 cgm.real_prec.b_prec = mode;

 return 0;
}

int cgmb_indpre ( void )
{
 long prec;

 if ( cgmb_i ( &prec ) ) return 1;

 if ( prec==8 ) cgm.ix_prec.b_prec = 0;
 else if ( prec==16 ) cgm.ix_prec.b_prec = 1;
 else if ( prec==24 ) cgm.ix_prec.b_prec = 2;
 else if ( prec==32 ) cgm.ix_prec.b_prec = 3;

 return 0;
}

int cgmb_colpre ( void )
{
 long prec;

 if ( cgmb_i ( &prec ) ) return 1;

 if ( prec==8 ) cgm.cd_prec = 0;
 else if ( prec==16 ) cgm.cd_prec = 1;
 else if ( prec==24 ) cgm.cd_prec = 2;
 else if ( prec==32 ) cgm.cd_prec = 3;

 return 0;
}

int cgmb_colipr ( void )
{
 long prec;

 if ( cgmb_i ( &prec ) ) return 1;

 if ( prec==8 ) cgm.cix_prec = 0;
 else if ( prec==16 ) cgm.cix_prec = 1;
 else if ( prec==24 ) cgm.cix_prec = 2;
 else if ( prec==32 ) cgm.cix_prec = 3;

 return 0;
}

int cgmb_maxcoi ( void )
{
 if ( cgmb_ci ( (unsigned long *)&(cgm.max_cix) ) ) return 1;

 color_table = (trgb *) realloc ( color_table, sizeof(trgb)*(cgm.max_cix+1) );

 color_table[0].red   = 255;
 color_table[0].green = 255;
 color_table[0].blue  = 255;
 color_table[1].red   = 0;
 color_table[1].green = 0;
 color_table[1].blue  = 0;

 return 0;
}

int cgmb_covaex ( void )
{
 if ( cgmb_rgb   ( &(cgm.color_ext.black.red), &(cgm.color_ext.black.green), &(cgm.color_ext.black.blue) ) ) return 1;
 if ( cgmb_rgb   ( &(cgm.color_ext.white.red), &(cgm.color_ext.white.green), &(cgm.color_ext.white.blue) ) ) return 1;

 return 0;
}

int cgmb_mtfell ( void )
{
 long group, element;
 long n, i;

 if ( cgmb_i ( &n ) ) return 1;

 for ( i=0; i<n; i++ )
   {
    if ( cgmb_ix ( &group ) ) return 1;
    if ( cgmb_ix ( &element ) ) return 1;
   }

 return 0;
}

int cgmb_bmtfdf ( void )
{
 int c, id, len, cont, totbc; 
 unsigned short b;
 int count=0;
 int old_cgmlen;
 char *buff;

 buff = (char *) malloc ( sizeof(char)*cgm.len );

 memcpy ( buff, cgm.buff.dados, cgm.len );

 old_cgmlen = cgm.len;

 totbc = 0;

 while ( count<old_cgmlen )
  {
   cgm.bc = 0;

   b = ((unsigned char)buff[count] << 8) | (unsigned char)buff[count+1];
   count += 2;

   cgm.bl += 2;

   len = b & 0x001F;

   id = ( b & 0x0FE0 ) >> 5;

   c = ( b & 0xF000 ) >> 12;

   cont = 0;

   if ( len > 30 )
    {
     b = ((unsigned char)buff[count] << 8) | (unsigned char)buff[count+1];
     count += 2;

     cgm.bl += 2;

     len = b & 0x7FFF;
     cont = ( b & 0x8000 );
    }

   cgm.len = len;

   if ( cgm.len )
    {
     if ( cgm.len>cgm.buff.size )
       cgm.buff.dados = (char *) realloc ( (char *)cgm.buff.dados, sizeof(char) * cgm.len );

     memcpy ( cgm.buff.dados, &buff[count], cgm.len );
	   count += cgm.len;

     cgm.bl += cgm.len;

     if ( len & 1 )
      {
       count++;
       cgm.bl += 1;
      }
    
     while ( cont )
      {
       unsigned short b;
       int old_len = cgm.len;

       cgm.bl += 2;

       b = ((unsigned char)buff[count] << 8) | (unsigned char)buff[count+1];
       count += 2;

       cont = ( b & 0x8000 );

       len = b & 0x7fff;

       cgm.len += len;

       if ( cgm.len>cgm.buff.size )
        cgm.buff.dados = (char *) realloc ( (char *)cgm.buff.dados, sizeof(char) * cgm.len );

       memcpy ( &cgm.buff.dados[old_len], &buff[count], len );
       count += len;

       if ( len & 1 )
        {
         count++;
       
         cgm.bl += 1;
        }
      }
    }

   if ( cgmb_exec_comand ( c, id ) ) return 1;
   totbc += count;
   /*count=0;*/
  }

 return 0;
}

int cgmb_fntlst ( void )
{
 char *fl = NULL;

 if ( text_att.font_list==NULL ) text_att.font_list = NewList();

 while ( cgm.bc < cgm.len )
  {
   if ( cgmb_s ( &fl ) ) return 1;

   AppendList ( text_att.font_list, fl );
  }

 return 0;
}

int cgmb_chslst ( void )
{
 short mode;
 char *s = NULL;

 while ( cgm.bc < cgm.len )
   {
    if ( cgmb_e ( &mode ) ) return 1;
    if ( cgmb_s ( &s ) ) return 1;
   }

 return 0;
}

int cgmb_chcdac ( void )
{
 short mode;

 if ( cgmb_e ( &mode ) ) return 1;

 return 0;
}
 

/******************************
* Picture Descriptor Elements *
******************************/

int cgmb_sclmde ( void )
{
 if ( cgmb_e ( &(cgm.scaling_mode.mode) ) ) return 1;
 if ( cgm.real_prec.b_prec==1 )
  {
   if ( cgmb_getfl64 ( &(cgm.scaling_mode.scale_factor) )  ) return 1;
  }
 else
  {
   float f;
   if ( cgmb_getfl32 ( (float *) &f )  ) return 1;
   if ( f<1e-20 ) f = 1;
   cgm.scaling_mode.scale_factor = f;
  }

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

 return 0;
}

int cgmb_clslmd ( void )
{
 if ( cgmb_e ( &(cgm.clrsm) ) ) return 1;

 return 0;
}

int cgmb_lnwdmd ( void )
{
 if ( cgmb_e ( &(cgm.lnwsm) ) ) return 1;

 return 0;
}

int cgmb_mkszmd ( void )
{
 if ( cgmb_e ( &(cgm.mkssm) ) ) return 1;

 return 0;
}

int cgmb_edwdmd ( void )
{
 if ( cgmb_e ( &(cgm.edwsm) ) ) return 1;

 return 0;
}

int cgmb_vdcext ( void )
{
 if ( cgmb_p ( &(cgm.vdc_ext.first.x), &(cgm.vdc_ext.first.y) ) ) return 1;
 if ( cgmb_p ( &(cgm.vdc_ext.second.x), &(cgm.vdc_ext.second.y) ) ) return 1;

 if (cdcgmvdcextcb)
  {
   int err;
   err = cdcgmvdcextcb( CD_CGM, cgm.vdc_type, &(cgm.vdc_ext.first.x), &(cgm.vdc_ext.first.y),
                                              &(cgm.vdc_ext.second.x), &(cgm.vdc_ext.second.y) );
   if (err==CD_ABORT) return -1;
  }

 do_vdcext ( cgm.vdc_ext.first, cgm.vdc_ext.second );

 return 0;
}

int cgmb_bckcol ( void )
{
 if ( cgmb_rgb ( &(cgm.back_color.red), &(cgm.back_color.green), &(cgm.back_color.blue) ) ) return 1;

 do_bckcol ( cgm.back_color );

 return 0;
}

/*******************
* Control Elements *
*******************/

int cgmb_vdcipr ( void )
{
 long prec;

 if ( cgmb_i ( &prec ) ) return 1;

 if ( prec==8 ) cgm.vdc_int.b_prec = 0;
 else if ( prec==16 )  cgm.vdc_int.b_prec = 1; 
 else if ( prec==24 )  cgm.vdc_int.b_prec = 2; 
 else if ( prec==32 )  cgm.vdc_int.b_prec = 3; 

 return 0;
}

int cgmb_vdcrpr ( void )
{
 short i1;
 long mode, i2, i3;

 if ( cgmb_e ( &i1 ) ) return 1;
 if ( cgmb_i ( &i2 ) ) return 1;
 if ( cgmb_i ( &i3 ) ) return 1 ;

 if ( i1 == 0 && i2 == 9 && i3 == 23 ) mode = 0;
 else if ( i1 == 0 && i2 == 12 && i3 == 52 ) mode = 1;
 else if ( i1 == 1 && i2 == 16 && i3 == 16 ) mode = 2;
 else if ( i1 == 1 && i2 == 32 && i3 == 32 ) mode = 3;

 cgm.vdc_real.b_prec = mode;

 return 0;
}

int cgmb_auxcol ( void )
{
 if ( cgmb_co ( &(cgm.aux_color) ) ) return 1;

 return 0;
}

int cgmb_transp ( void )
{
 if ( cgmb_e ( &(cgm.transparency) ) ) return 1;

 do_transp ( cgm.transparency );

 return 0;
}

int cgmb_clprec ( void )
{
 if ( cgmb_p   ( &(cgm.clip_rect.first.x),  &(cgm.clip_rect.first.y) ) ) return 1;
 if ( cgmb_p   ( &(cgm.clip_rect.second.x),  &(cgm.clip_rect.second.y) ) ) return 1;

 do_clprec ( cgm.clip_rect.first, cgm.clip_rect.second );

 return 0;
}

int cgmb_clpind ( void )
{
 if ( cgmb_e   ( &(cgm.clip_ind) ) ) return 1;

 do_clpind ( cgm.clip_ind );

 return 0;
}

/*******************************
* Graphical Primitive Elements *
*******************************/

static tpoint *_cgm_point_list ( int *np )
{
 *np=0;

 while ( cgm.bc < cgm.len )
   {
    if ( cgmb_p ( &(point_list[*np].x), &(point_list[*np].y) ) ) return NULL;
    ++(*np);
    if ( *np==npoints)
      {
       npoints *= 2;
       point_list = (tpoint *) realloc ( point_list, npoints*sizeof(tpoint) );
      }
   }

 return point_list;
}

int cgmb_polyln ( void )
{
 tpoint *pts;
 int np;

 pts = _cgm_point_list ( &np );
 if ( pts==NULL ) return 1;

 do_polyln ( np, pts );

 return 0;
}

int cgmb_djtply ( void )
{
 tpoint *pts;
 int np;

 pts = _cgm_point_list ( &np );
 if ( pts==NULL ) return 1;

 do_djtply ( np, pts );

 return 0;
}

int cgmb_polymk ( void )
{
 tpoint *pts;
 int np;

 pts = _cgm_point_list ( &np );
 if ( pts==NULL ) return 1;

 do_polymk ( np, pts );

 return 0;
}

int cgmb_text ( void )
{
 tpoint pos;
 char *s = NULL;
 short t;

 if ( cgmb_p ( &pos.x, &pos.y ) ) return 1;
 if ( cgmb_e ( &t ) ) return 1;
 if ( cgmb_s ( &s ) ) return 1;

 do_text ( NORM_TEXT, s, pos );

 free ( s );

 return 0;
}

int cgmb_rsttxt ( void )
{
 double height, width;
 tpoint pos;
 char *s = NULL;
 short t;

 if ( cgmb_vdc ( &width ) ) return 1;
 if ( cgmb_vdc ( &height ) ) return 1;
 if ( cgmb_p ( &pos.x, &pos.y ) ) return 1;
 if ( cgmb_e ( &t ) ) return 1;
 if ( cgmb_s ( &s ) ) return 1;

 text_att.height = height;

 do_text ( RESTRICTED_TEXT, s, pos );

 if ( s!= NULL ) free ( s );

 return 0;
}

int cgmb_apdtxt ( void )
{
 char *s = NULL;
 short t;

 if ( cgmb_e ( &t ) ) return 1;
 if ( cgmb_s ( &s ) ) return 1;

 if ( s==NULL ) free ( s );

 return 0;
}

int cgmb_polygn ( void )
{
 tpoint *pts;
 int np;
 static int porra=0;

 porra++;

 pts = _cgm_point_list ( &np );
 if ( pts==NULL ) return 1;

 do_polygn ( np, pts );

 return 0;
}

static int _cgm_vertex_list ( tpoint **pts, short **flags, int *np )
{
 int block=500;

 *np=0;
 *pts = (tpoint *) malloc ( block*sizeof(tpoint) );
 *flags = (short *) malloc ( block*sizeof(short) );

 while ( cgm.bc < cgm.len )
   {
    if ( cgmb_p ( &((*pts)[*np].x), &((*pts)[*np].y) ) ) return 1;
    if ( cgmb_e ( &((*flags)[*np]) ) ) return 1;

    ++(*np);
    if ( *np==block)
      {
       block *= 2;
       *pts = (tpoint *) realloc ( *pts, block*sizeof(tpoint) );
       *flags = (short *) realloc ( *flags, block*sizeof(short) );
      }
   }

 return 0;
}

int cgmb_plgset ( void )
{
 tpoint *pts;
 short *flags;
 int np;

 if ( _cgm_vertex_list (  &pts, &flags, &np ) ) return 1;

 do_plgset( NO, np, pts, flags );

 free ( pts );
 free ( flags );

 return 0;
}

int cgmb_cellar ( void )
{
 register int i, j, k;
 long prec;
 long sx, sy;
 short mode;
 int b;
 unsigned char dummy;
 tpoint corner1, corner2, corner3;
 tcolor *cell;

 if ( cgmb_p ( &(corner1.x), &(corner1.y) ) ) return 1;
 if ( cgmb_p ( &(corner2.x), &(corner2.y) ) ) return 1;
 if ( cgmb_p ( &(corner3.x), &(corner3.y) ) ) return 1;

 if ( cgmb_i ( &sx ) ) return 1;
 if ( cgmb_i ( &sy ) ) return 1;

 if ( cgmb_i ( &prec ) ) return 1;

 if ( cgmb_e ( &mode ) ) return 1;

 cell = (tcolor *) malloc ( sx*sy*sizeof(tcolor) );

 if ( mode )
  for ( k=0; k<sy; k++ )
   {
    b=cgm.bc;
	  cgm.pc=0;
    for ( i=0; i<sx; i++ )
     {
      if ( cgmb_getpixel ( &(cell[k*sx+i]), prec ) ) return 1;
     }
    if ( k<(sy-1) && (cgm.bc-b)%2 ) cgmb_getc ( &dummy );
    GetFilePos();
   }
 else
  for ( k=0; k<sy; k++ )
   {
    b=cgm.bc;
	  cgm.pc=0;
    for ( i=0; i<sx; )
     {
      long l;
      tcolor cor;
      if ( cgmb_i ( &l ) ) return 1;
      if ( cgmb_getpixel ( &cor, prec ) ) return 1;
      for ( j=0; j<l; j++ )
       {  
        cell[k*sx+i] = cor;
        i++;
       }
     }
    if ( k<(sy-1) && (cgm.bc-b)%2 ) cgmb_getc ( &dummy );
    GetFilePos();
   }

 if ( cgm.clrsm == 0 ) /* indexed */
  {
   for ( i=0; i<sx*sy; i++ )
    {
     int ind = cell[i].ind;
     cell[i].rgb.red = color_table[ind].red;
     cell[i].rgb.green = color_table[ind].green;
     cell[i].rgb.blue = color_table[ind].blue;
	 }
  }

 do_cellar ( corner1, corner2, corner3, sx, sy, prec, cell );

 free ( cell );

 return 0;
}

static long sample_type, n_samples;

static int BuildString ( char *sin, char **sout, int *slen, int *block )
{
 *slen = strlen ( sin ) + strlen(*sout) + 1 + 1; /* + espaco em branco no final +\0 */
 if (*slen > *block)
  {
   *block *= 2;
   *sout = (char *) realloc(*sout,sizeof(char)*(*block));
   if ( *sout==NULL ) return 1;
  }

 strcat(*sout,sin);
 strcat(*sout," ");

 return 0;
}

int cgm_generalized_drawing_primitive_4 ( void )
{
 long j, i;
 tpoint pt[4];
 unsigned char c;
 double r;
 char *s=NULL, tmp[80];
 int block = 500, slen = 0;
 int id = -4;

 s = (char *) malloc ( block*sizeof(char) );

 strcpy ( s, "" );

 for ( j=0; j<4; j++ )
  {
   if ( cgmb_p ( &(pt[j].x), &(pt[j].y) ) ) return 1;
  }

 if ( cgmb_getc(&c) ) return 1;
 if ( cgmb_getc(&c) ) return 1;

 for ( j=0; j<6; j++ )
   {
    if ( cgmb_r(&r) ) return 1;
    sprintf(tmp,"%g",r);
	if ( BuildString ( tmp, &s, &slen, &block ) ) return 1;
   }

 if ( cgmb_i(&i) ) return 1;
 sprintf(tmp,"%ld",i);
 if ( BuildString ( tmp, &s, &slen, &block ) ) return 1;

 if ( cgmb_i(&sample_type) ) return 1;
 sprintf(tmp,"%ld",sample_type);
 if ( BuildString ( tmp, &s, &slen, &block ) ) return 1;

 if ( cgmb_i(&n_samples) ) return 1;
 sprintf(tmp,"%ld",n_samples);
 if ( BuildString ( tmp, &s, &slen, &block ) ) return 1;

 for ( j=0; j<2; j++ )
   {
    if ( cgmb_r(&r) ) return 1;
    sprintf(tmp,"%g",r);
    if ( BuildString ( tmp, &s, &slen, &block ) ) return 1;
   }

 for ( j=0; j<4; j++ )
   {
    if ( cgmb_i(&i) ) return 1;
    sprintf(tmp,"%ld",i);
    if ( BuildString ( tmp, &s, &slen, &block ) ) return 1;
   }

 do_gdp ( id, pt, s );

 free(s);

 return 0;
}

typedef int (*_getdata) (void *);

int cgm_generalized_drawing_primitive_5 ( void )
{
 int  (*getdata) (void *);
 void  *data;
 char  format[10];
 int   i;
 char  *s, tmp[80];
 int   block = 500, slen = 0;
 long  id=-5;

 s = (char *) malloc ( sizeof(char)*block );

 strcpy ( s, "" );

 switch ( sample_type )
   {
    case 0:
      getdata = (_getdata) cgmb_geti16;
      data = (short *) malloc ( sizeof(short) );
	  if ( data==NULL ) return 1;
      strcpy ( format, "%d\0" );
      break;
    case 1:
      getdata = (_getdata) cgmb_geti32;
      data = (long *) malloc ( sizeof(long) );
	  if ( data==NULL ) return 1;
      strcpy ( format, "%d\0" );
      break;
    case 2:
      getdata = (_getdata) cgmb_getfl32;
      data = (float *) malloc ( sizeof(float) );
	  if ( data==NULL ) return 1;
      strcpy ( format, "%g\0" );
      break;
    case 3:
      getdata = (_getdata) cgmb_geti8;
      data = (signed char *) malloc ( sizeof(signed char) );
	  if ( data==NULL ) return 1;
      strcpy ( format, "%d\0" );
      break;
    case 4:
      getdata = (_getdata) cgmb_geti16;
      data = (short *) malloc ( sizeof(short) );
	  if ( data==NULL ) return 1;
      strcpy ( format, "%d\0" );
      break;
    case 5:
      getdata = (_getdata) cgmb_geti8;
      data = (signed char *) malloc ( sizeof(signed char) );
	  if ( data==NULL ) return 1;
      strcpy ( format, "%d\0" );
      break;
   }

 for ( i=0; i<n_samples; i++ )
   {

    getdata(data);

    if (sample_type==0)
      sprintf(tmp,format,*(short *)data);
    else if (sample_type==1)
      sprintf(tmp,format,*(long *)data);
    else if (sample_type==2)
      sprintf(tmp,format,*(float *)data);
    else if (sample_type==3)
      sprintf(tmp,format,*(signed char *)data);
    else if (sample_type==4)
      sprintf(tmp,format,*(short *)data);
    else if (sample_type==5)
      sprintf(tmp,format,*(signed char *)data);

    if ( BuildString ( tmp, &s, &slen, &block ) ) return 1;

    if (sample_type==4 || sample_type==5)
      {
       unsigned long ci;
       char endstr='\0';

       if ( cgmb_ci  ( &ci ) ) return 1;
       sprintf(tmp,"%ld%c",ci,endstr);
       if ( BuildString ( tmp, &s, &slen, &block ) ) return 1;
      }
  }

 if ( cgm.bc < cgm.len )
  {
   int i;
   unsigned char c;

   for ( i=0; i<cgm.len-cgm.bc; i++ )
    {
     if ( cgmb_getc(&c) ) return 1;
     if ( cgmb_getc(&c) ) return 1;
    }
  }

 do_gdp ( id, NULL, s );

 free(s);

 return 0;
}

int cgmb_gdp ( void )
{
 long id, n, i;
 double x, y;
 char *s = NULL;

 cgmb_i ( &id );

 if ( id==-4 ) 
  {
   if ( cgm_generalized_drawing_primitive_4 ( ) ) return 1;
  }
 else if ( id==-5 ) 
  {
   if ( cgm_generalized_drawing_primitive_5 ( ) ) return 1;
  }
 else
  {
   if ( cgmb_i ( &n ) ) return 1;
   for ( i=0; i<n; i++ )
    {
	   if ( cgmb_p ( &x, &y ) ) return 1;
	  }
   if ( cgmb_s ( &s ) ) return 1;
  }

 return 0;
}

int cgmb_rect ( void )
{
 tpoint point1;
 tpoint point2;

 if ( cgmb_p ( &(point1.x), &(point1.y) ) ) return 1;
 if ( cgmb_p ( &(point2.x), &(point2.y) ) ) return 1;

 do_rect ( point1, point2 );

 return 0;
}

int cgmb_circle ( void )
{
 tpoint center;
 double radius;

 if ( cgmb_p ( &(center.x), &(center.y) ) ) return 1;

 if ( cgmb_vdc ( &radius ) ) return 1;

 do_circle ( center, radius );

 return 0;
}

int cgmb_circ3p ( void )
{
 tpoint starting;
 tpoint intermediate;
 tpoint ending;

 if ( cgmb_p ( &(starting.x), &(starting.y) ) ) return 1;
 if ( cgmb_p ( &(intermediate.x), &(intermediate.y) ) ) return 1;
 if ( cgmb_p ( &(ending.x), &(ending.y) ) ) return 1;

 do_circ3p ( starting, intermediate, ending );

 return 0;
}

int cgmb_cir3pc ( void )
{
 tpoint starting;
 tpoint intermediate;
 tpoint ending;
 short close_type;

 if ( cgmb_p ( &(starting.x), &(starting.y) ) ) return 1;
 if ( cgmb_p ( &(intermediate.x), &(intermediate.y) ) ) return 1;
 if ( cgmb_p ( &(ending.x), &(ending.y) ) ) return 1;

 if ( cgmb_e ( &close_type ) ) return 1;

 do_circ3pc ( starting, intermediate, ending, close_type );

 return 0;
}

int cgmb_circnt ( void )
{
 tpoint center;
 tpoint start;
 tpoint end;
 double  radius;

 if ( cgmb_p ( &(center.x), &(center.y) ) ) return 1;

 if ( cgmb_vdc ( &(start.x) ) ) return 1;
 if ( cgmb_vdc ( &(start.y) ) ) return 1;

 if ( cgmb_vdc ( &(end.x) ) ) return 1;
 if ( cgmb_vdc ( &(end.y) ) ) return 1;

 if ( cgmb_vdc ( &radius ) ) return 1;

 do_circcnt ( center, start, end, radius );

 return 0;
}

int cgmb_ccntcl ( void )
{
 tpoint center;
 tpoint start;
 tpoint end;
 double radius;
 short close_type;

 if ( cgmb_p ( &(center.x), &(center.y) ) ) return 1;

 if ( cgmb_vdc ( &(start.x) ) ) return 1;
 if ( cgmb_vdc ( &(start.y) ) ) return 1;

 if ( cgmb_vdc ( &(end.x) ) ) return 1;
 if ( cgmb_vdc ( &(end.y) ) ) return 1;

 if ( cgmb_vdc ( &radius ) ) return 1;

 if ( cgmb_e ( &close_type ) ) return 1;

 do_ccntcl ( center, start, end, radius, close_type );

 return 0;
}

int cgmb_ellips ( void )
{
 tpoint center;
 tpoint first_CDP;
 tpoint second_CDP;

 if ( cgmb_p ( &(center.x), &(center.y) ) ) return 1;

 if ( cgmb_p ( &(first_CDP.x), &(first_CDP.y) ) ) return 1;
 if ( cgmb_p ( &(second_CDP.x), &(second_CDP.y) ) ) return 1;

 do_ellips ( center, first_CDP, second_CDP );

 return 0;
}

int cgmb_ellarc ( void )
{
 tpoint center;
 tpoint first_CDP;
 tpoint second_CDP;
 tpoint start, end;

 if ( cgmb_p ( &(center.x), &(center.y) ) ) return 1;

 if ( cgmb_p ( &(first_CDP.x), &(first_CDP.y) ) ) return 1;
 if ( cgmb_p ( &(second_CDP.x), &(second_CDP.y) ) ) return 1;

 if ( cgmb_vdc ( &(start.x) ) ) return 1;
 if ( cgmb_vdc ( &(start.y) ) ) return 1;

 if ( cgmb_vdc ( &(end.x) ) ) return 1;
 if ( cgmb_vdc ( &(end.y) ) ) return 1;

 do_ellarc ( center, first_CDP, second_CDP, start, end );

 return 0;
}

int cgmb_ellacl ( void )
{

 tpoint center;
 tpoint first_CDP;
 tpoint second_CDP;
 tpoint start, end;
 short close_type;

 if ( cgmb_p ( &(center.x), &(center.y) ) ) return 1;

 if ( cgmb_p ( &(first_CDP.x), &(first_CDP.y) ) ) return 1;
 if ( cgmb_p ( &(second_CDP.x), &(second_CDP.y) ) ) return 1;

 if ( cgmb_vdc ( &(start.x) ) ) return 1;
 if ( cgmb_vdc ( &(start.y) ) ) return 1;

 if ( cgmb_vdc ( &(end.x) ) ) return 1;
 if ( cgmb_vdc ( &(end.y) ) ) return 1;

 if ( cgmb_e ( &close_type ) ) return 1;

 do_ellacl ( center, first_CDP, second_CDP, start, end, close_type );

 return 0;
}

/*********************
* Attribute Elements *
*********************/

int cgmb_lnbdin( void )
{
 if ( cgmb_ix ( &(line_att.index) ) ) return 1;

 return 0;
}

int cgmb_lntype ( void )
{
 if ( cgmb_ix ( &(line_att.type) ) ) return 1;

 return 0;
}

int cgmb_lnwidt ( void )
{
 if ( cgm.lnwsm==0 )
  {
   if ( cgmb_vdc ( &(line_att.width) ) ) return 1;
  }
 else
  {
   if ( cgmb_r ( &(line_att.width) ) ) return 1;
  }

 return 0;
}

int cgmb_lncolr( void )
{
 if ( cgmb_co ( &(line_att.color) ) ) return 1;

 return 0;
}

int cgmb_mkbdin( void )
{
 if ( cgmb_ix ( &(marker_att.index) ) ) return 1;

 return 0;
}

int cgmb_mktype( void )
{
 if ( cgmb_ix ( &(marker_att.type) ) ) return 1;

 return 0;
}

int cgmb_mksize( void )
{
 if ( cgm.mkssm == 0 )
  {
   if ( cgmb_vdc ( &(marker_att.size) ) ) return 1;
  }
 else
  {
   if ( cgmb_r ( &(marker_att.size) ) ) return 1;
  }

 return 0;
}

int cgmb_mkcolr( void )
{
 if ( cgmb_co ( &(marker_att.color) ) ) return 1;

 return 0;
}

int cgmb_txbdin( void )
{
 if ( cgmb_ix ( &(text_att.index) ) ) return 1;

 return 0;
}

int cgmb_txftin ( void )
{
 char *font;
 char *font_array[] = {"SYSTEM", "COURIER", "TIMES", "HELVETICA", NULL};
 int cdfont[] = {CD_SYSTEM, CD_COURIER, CD_TIMES_ROMAN, CD_HELVETICA};
 char *style_array[] = {"BOLDITALIC", "ITALIC", "BOLD", "PLAIN", NULL};
 int cdstyle[] = {CD_BOLD_ITALIC, CD_ITALIC, CD_BOLD, CD_PLAIN};
 int i;

 if ( cgmb_ix ( &(text_att.font_index) ) ) return 1;

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

 return 0;
}

int cgmb_txtprc( void )
{
 if ( cgmb_e ( &(text_att.prec) ) ) return 1;

 return 0;
}

int cgmb_chrexp ( void )
{
 if ( cgmb_r ( &(text_att.exp_fact) ) ) return 1;

 return 0;
}

int cgmb_chrspc ( void )
{
 if ( cgmb_r ( &(text_att.char_spacing) ) ) return 1;

 return 0;
}

int cgmb_txtclr( void )
{
 if ( cgmb_co ( &(text_att.color) ) ) return 1;

 return 0;
}

int cgmb_chrhgt ( void )
{
 if ( cgmb_vdc ( &(text_att.height) ) ) return 1;

 do_text_height ( text_att.height );

 return 0;
}

int cgmb_chrori ( void )
{
 if ( cgmb_vdc ( &(text_att.char_up.x) ) ) return 1;
 if ( cgmb_vdc ( &(text_att.char_up.y) ) ) return 1;
 if ( cgmb_vdc ( &(text_att.char_base.x) ) ) return 1;
 if ( cgmb_vdc ( &(text_att.char_base.y) ) ) return 1;

 return 0;
}

int cgmb_txtpat ( void )
{
 if ( cgmb_e ( &(text_att.path) ) ) return 1;

 return 0;
}

int cgmb_txtali ( void )
{
 if ( cgmb_e ( &(text_att.alignment.hor) ) ) return 1;
 if ( cgmb_e ( &(text_att.alignment.ver) ) ) return 1;

 if ( cgmb_r ( &(text_att.alignment.cont_hor) ) ) return 1;
 if ( cgmb_r ( &(text_att.alignment.cont_ver) ) ) return 1;

 do_txtalign ( text_att.alignment.hor, text_att.alignment.ver );

 return 0;
}

int cgmb_chseti( void )
{
 long set;

 if ( cgmb_ix ( &set ) ) return 1;

 return 0;
}

int cgmb_achsti( void )
{
 long set;

 if ( cgmb_i ( &set ) ) return 1;

 return 0;
}

int cgmb_fillin( void )
{
 if ( cgmb_ix ( &(fill_att.index) ) ) return 1;

 return 0;
}

int cgmb_intsty( void )
{
 if ( cgmb_e ( &(fill_att.int_style) ) ) return 1;

 return 0;
}

int cgmb_fillco( void )
{
 if ( cgmb_co ( &(fill_att.color) ) ) return 1;

 return 0;
}

int cgmb_hatind( void )
{
 if ( cgmb_ix ( &(fill_att.hatch_index) ) ) return 1;
 if ( fill_att.hatch_index==3 ) fill_att.hatch_index = 4;
 else if ( fill_att.hatch_index==4 ) fill_att.hatch_index = 3;

 return 0;
}

int cgmb_patind( void )
{
 if ( cgmb_ix ( &(fill_att.pat_index) ) ) return 1;

 return 0;
}

int cgmb_edgind( void )
{
 if ( cgmb_ix ( &(edge_att.index) ) ) return 1;

 return 0;
}

int cgmb_edgtyp( void )
{
 if ( cgmb_ix ( &(edge_att.type) ) ) return 1;

 return 0;
}

int cgmb_edgwid ( void )
{
 if ( cgm.edwsm==0 )
  {
   if ( cgmb_vdc ( &(edge_att.width) ) ) return 1;
  }
 else
  {
   if ( cgmb_r ( &(edge_att.width) ) ) return 1;
  }

 return 0;
}

int cgmb_edgcol ( void )
{
 if ( cgmb_co ( &(edge_att.color) ) ) return 1;

 return 0;
}
 

int cgmb_edgvis ( void )
{
 if ( cgmb_e ( &(edge_att.visibility) ) ) return 1;

 return 0;
}

int cgmb_fillrf ( void )
{
 if ( cgmb_p ( &(fill_att.ref_pt.x),  &(fill_att.ref_pt.y) ) ) return 1;

 return 0;
}

int cgmb_pattab ( void )
{
 long localp;
 int i;
 pat_table *pat;

 pat = (pat_table *) malloc ( sizeof(pat_table) );

 if ( fill_att.pat_list==NULL ) fill_att.pat_list = NewList();

 if ( cgmb_i ( &(pat->index) ) ) return 1;

 if ( cgmb_i ( &(pat->nx) ) ) return 1;
 if ( cgmb_i ( &(pat->ny) ) ) return 1;

 if ( cgmb_i ( &(localp) ) ) return 1;

 pat->pattern = (tcolor *) malloc ( pat->nx*pat->ny*sizeof(tcolor) );

 for ( i=0; i<(pat->nx*pat->ny); i++ )
  {
   if ( cgmb_getpixel ( &(pat->pattern[i]), localp ) ) return 1;
  }

 AppendList ( fill_att.pat_list, pat );

 return 0;
}

int cgmb_patsiz ( void )
{
 if ( cgmb_vdc ( &(fill_att.pat_size.height.x) ) ) return 1;
 if ( cgmb_vdc ( &(fill_att.pat_size.height.y) ) ) return 1;
 if ( cgmb_vdc ( &(fill_att.pat_size.width.x) ) ) return 1;
 if ( cgmb_vdc ( &(fill_att.pat_size.width.y) ) ) return 1;

 return 0;
}

int cgmb_coltab ( void )
{
 unsigned long starting_index, i;
 int p[] = {8, 16, 24, 32};
 int n = (cgm.len-cgm.cix_prec)/(3*(p[cgm.cd_prec]/8));

 if ( cgmb_ci ( &(starting_index) ) ) return 1;

 for ( i=starting_index; i<starting_index+n; i++ )
  {
   if ( cgmb_rgb ( &(color_table[i].red), &(color_table[i].green), &(color_table[i].blue) ) ) return 1;
  }

 if ( cgm.bc==(cgm.len-1) ) cgm.bc++;

 return 0;
}

int cgmb_asf ( void )
{
 tasf *pair;

 if ( asf_list==NULL ) asf_list = NewList();

 while( cgm.bc < cgm.len )
  {
   pair = (tasf *) malloc ( sizeof (tasf) );

   if ( cgmb_e ( &(pair->type) ) ) return 1;
   if ( cgmb_e ( &(pair->value) ) ) return 1;

   AppendList ( asf_list, pair );
  }

 return 0;
}

/*****************
* Escape Element *
*****************/

/********************
* External elements *
********************/

int cgmb_escape ( void )   /* escape */
{
#if 1

 {
  int i;
  unsigned char c;
  for ( i=0; i<cgm.len; i++ ) cgmb_getc(&c);
 }

#else

 {
 long identifier;
 char *data_rec;

 if ( cgmb_i ( &(identifier) ) ) return 1;

 if ( cgmb_s ( &data_rec ) ) return 1;

 free(data_rec);
 }

#endif
                  
 return 0;
}

int cgmb_messag ( void )
{
 char *text;
 short flag;

 if ( cgmb_e ( &flag ) ) return 1;

 if ( cgmb_s ( &text ) ) return 1;

 free(text);

 return 0;
}

int cgmb_appdta ( void )
{
 long identifier;
 char *data_rec;

 if ( cgmb_i ( &identifier ) ) return 1;

 if ( cgmb_s ( &data_rec ) ) return 1;

 free(data_rec);

 return 0;
}
