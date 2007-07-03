#define _INTCGM1_C_

#include <stdio.h>      /* FILE, ftell, fseek, fputc, fopen, fclose, fputs, fprintf */
#include <stdlib.h>     /* malloc, free */
#include <string.h>     /* strlen */
#include <math.h>       /* floor */

#ifdef SunOS
#include <unistd.h>       /* SEEK_SET, SEEK_END */
#endif

#include <float.h>      /* FLT_MIN, FLT_MAX */
#include <limits.h>     /* INT_MIN, INT_MAX */

#include "cd.h"
#include "list.h"
#include "types.h"
#include "intcgm2.h"
#include "intcgm.h"

static int isitbin ( char *fname )
{
 unsigned char ch[2];
 int erro, c, id;
 unsigned short b;
 FILE *f = fopen ( fname, "rb" );
 if (!f)
   return 0;

 erro = fread ( ch, 1, 2, f );
  
 b = (ch[0] << 8) + ch[1];

 id = ( b & 0x0FE0 ) >> 5;

 c = ( b & 0xF000 ) >> 12;
 
 fclose(f);
 
 if ( c==0 && id==1 )
  return 1;
 else
  return 0; 
}

int cgmplay ( char* filename, int xmn, int xmx, int ymn, int ymx )
{

 xmin = xmn;
 xmax = xmx;
 ymin = ymn;
 ymax = ymx;

 view_xmin = xmn;
 view_xmax = xmx;
 view_ymin = ymn;
 view_ymax = ymx;

 scale_factor_x = 1;
 scale_factor_y = 1;
 scale_factor   = 1;

 block = 500;
 npoints = 500;
 cgm.buff.size = 1024;

 point_list = (tpoint *) malloc ( sizeof(tpoint)*npoints);
 cgm.buff.dados = (char *) malloc ( sizeof(char) * cgm.buff.size );

 if ( isitbin(filename) )
 {
   cgm.fp = fopen ( filename, "rb" );
   if (!cgm.fp)
   {
     free(point_list);
     free(cgm.buff.dados);
     return CD_ERROR;
   }

   fseek ( cgm.fp, 0, SEEK_END );
   cgm.file_size = ftell ( cgm.fp );
   fseek ( cgm.fp, 0, SEEK_SET );

   cgm.mode = 1;
   cgm.cgmf = funcs[cgm.mode];

   cgm.int_prec.b_prec  = 1;
   cgm.real_prec.b_prec = 2;
   cgm.ix_prec.b_prec   = 1;
   cgm.cd_prec   = 0;
   cgm.cix_prec  = 0;
   cgm.vdc_int.b_prec  = 1;
   cgm.vdc_real.b_prec = 2;
 }
 else
 {
   cgm.fp = fopen ( filename, "r" );
   if (!cgm.fp)
   {
     free(point_list);
     free(cgm.buff.dados);
     return CD_ERROR;
   }

   fseek ( cgm.fp, 0, SEEK_END );
   cgm.file_size = ftell ( cgm.fp );
   fseek ( cgm.fp, 0, SEEK_SET );

   cgm.mode = 2;
   cgm.cgmf = funcs[cgm.mode];

   cgm.int_prec.t_prec.minint  = -32767;
   cgm.int_prec.t_prec.maxint  = 32767;
   cgm.real_prec.t_prec.minreal = -32767;
   cgm.real_prec.t_prec.maxreal = 32767;
   cgm.real_prec.t_prec.digits = 4;
   cgm.ix_prec.t_prec.minint   = 0;
   cgm.ix_prec.t_prec.maxint   = 127;
   cgm.cd_prec   = 127;
   cgm.vdc_int.t_prec.minint  = -32767;
   cgm.vdc_int.t_prec.maxint  = 32767;
   cgm.vdc_real.t_prec.minreal = 0;
   cgm.vdc_real.t_prec.maxreal = 1;
   cgm.vdc_real.t_prec.digits = 4;
 }

 cgm.first = 1;
 cgm.len = 0;
 cgm.vdc_type = INTEGER;
 cgm.max_cix = 63;
 cgm.scaling_mode.mode = ABSTRACT;
 cgm.scaling_mode.scale_factor = 1.;
 cgm.drawing_mode = ABSTRACT;
 cgm.clrsm = INDEXED;
 cgm.lnwsm = SCALED;
 cgm.mkssm = SCALED;
 cgm.edwsm = SCALED;
 cgm.vdc_ext.first.x = 0;
 cgm.vdc_ext.first.y = 0;
 cgm.vdc_ext.second.x = 32767;
 cgm.vdc_ext.second.y = 32767;
 cgm.back_color.red = 0;
 cgm.back_color.green = 0;
 cgm.back_color.blue = 0;
 cgm.aux_color.rgb.red = 0;
 cgm.aux_color.rgb.green = 0;
 cgm.aux_color.rgb.blue = 0;
 cgm.color_ext.black.red = 0;
 cgm.color_ext.black.green = 0;
 cgm.color_ext.black.blue = 0;
 cgm.color_ext.white.red = 255;
 cgm.color_ext.white.green = 255;
 cgm.color_ext.white.blue = 255;
 cgm.transparency = ON;
 cgm.clip_rect.first.x = 0;
 cgm.clip_rect.first.y = 0;
 cgm.clip_rect.second.x = 32767;
 cgm.clip_rect.second.y = 32767;
 cgm.clip_ind = ON;
 cgm.bc = 0;
 cgm.bl= 0;
 cgm.cl = 0;

 line_att.index = 1;
 line_att.type = LINE_SOLID;
 line_att.width = 1;
 line_att.color.ind = 1;

 marker_att.index = 1;
 marker_att.type = 1;
 marker_att.size = 1;
 marker_att.color.ind = 1;

 text_att.index = 1;
 text_att.font_index = 1;
 text_att.font_list = NULL;
 text_att.font = CD_SYSTEM;
 text_att.style = CD_PLAIN;
 text_att.size = 8;
 text_att.prec = STRING;
 text_att.exp_fact = 1;
 text_att.char_spacing = 0;
 text_att.color.ind = 1;
 text_att.height = 1;
 text_att.char_up.x = 0;
 text_att.char_up.y = 1;
 text_att.char_base.x = 1;
 text_att.char_base.y = 0;
 text_att.path = PATH_RIGHT;
 text_att.alignment.hor = NORMHORIZ;
 text_att.alignment.ver  = NORMVERT;
 text_att.alignment.cont_hor = 0;
 text_att.alignment.cont_ver = 0;

 fill_att.index = 1;
 fill_att.int_style = HOLLOW;
 fill_att.color.ind = 1;
 fill_att.hatch_index = 1;
 fill_att.pat_index = 1;
 fill_att.ref_pt.x = 0;
 fill_att.ref_pt.y = 0;
 fill_att.pat_list = NULL;
 fill_att.pat_size.height.x = 0;
 fill_att.pat_size.height.y = 0;
 fill_att.pat_size.height.x = 0;
 fill_att.pat_size.width.y = 0;

 edge_att.index = 1;
 edge_att.type  = EDGE_SOLID;
 edge_att.width = 1;
 edge_att.color.ind = 1;
 edge_att.visibility = OFF;

 cdLineWidth(1);

 color_table = (trgb *) malloc ( sizeof(trgb)*cgm.max_cix);
 color_table[0].red   = 255;
 color_table[0].green = 255;
 color_table[0].blue  = 255;
 color_table[1].red   = 0;
 color_table[1].green = 0;
 color_table[1].blue  = 0;

 while ( !(*cgm.cgmf)() ){};

 if ( point_list!=NULL )
 {
   free(point_list);
   point_list = NULL;
 }

 if ( cgm.buff.dados!=NULL )
 {
   free(cgm.buff.dados);
   cgm.buff.dados = NULL;
 }

 if ( color_table!=NULL )
 {
   free(color_table);
   color_table = NULL;
 }

 fclose(cgm.fp);

 return CD_OK;
}
