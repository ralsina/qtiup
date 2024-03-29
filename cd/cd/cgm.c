/*
* drivers/cgm.c
* driver source code
* Computer Graphics Metafile functions
* Microsoft C 5.0 - Turbo C 2.0 - VAX C 3.0
* Grupo de Tecnologia em Computacao Grafica - PUC/RIO
* 23 Ago 91
*/

#include <stdio.h>      /* FILE, ftell, fseek, fputc, fopen, fclose, fputs, fprintf */
#include <stdlib.h>     /* malloc, free */
#include <string.h>     /* strlen */
#include <math.h>       /* floor */

#include <float.h>      /* FLT_MIN, FLT_MAX */
#include <limits.h>     /* INT_MIN, INT_MAX */

#include "cgm.h"

/**************************************************
***************************************************
**                                               **
**                  FUNCOES CGM                  **
**                                               **
***************************************************
**************************************************/


typedef struct {
        const char *ct;
        const char *c;
        } comando;

/************************************************
*                                               *
*            Dados para nao-binario             *
*                                               *
************************************************/

/* delimiter elements */

static const comando _cgmX_NULL             = { ""                   , "" };
static const comando _cgmX_BEGMF            = { "BEGMF"              , "\x30\x20" };
static const comando _cgmX_ENDMF            = { "ENDMF"              , "\x30\x21" };
static const comando _cgmX_BEG_PIC          = { "BEG_PIC"            , "\x30\x22" };
static const comando _cgmX_BEG_PIC_BODY     = { "BEG_PIC_BODY"       , "\x30\x23" };
static const comando _cgmX_END_PIC          = { "END_PIC"            , "\x30\x24" };

/* metafile descriptor elements */

static const comando _cgmX_MF_VERSION       = { "MF_VERSION"         , "\x31\x20" };
static const comando _cgmX_MF_DESC          = { "MF_DESC"            , "\x31\x21" };
static const comando _cgmX_VDC_TYPE         = { "VDC_TYPE"           , "\x31\x22" };
static const comando _cgmX_INTEGER_PREC     = { "INTEGER_PREC"       , "\x31\x23" };
static const comando _cgmX_REAL_PREC        = { "REAL_PREC"          , "\x31\x24" };
static const comando _cgmX_INDEX_PREC       = { "INDEX_PREC"         , "\x31\x25" };
static const comando _cgmX_COLR_PREC        = { "COLR_PREC"          , "\x31\x26" };
static const comando _cgmX_COLR_INDEX_PREC  = { "COLR_INDEX_PREC"    , "\x31\x27" };
static const comando _cgmX_MAX_COLR_INDEX   = { "MAX_COLR_INDEX"     , "\x31\x28" };
static const comando _cgmX_COLR_VALUE_EXT   = { "COLR_VALUE_EXT"     , "\x31\x29" };
static const comando _cgmX_MF_ELEM_LIST     = { "MF_ELEM_LIST"       , "\x31\x2a" };
static const comando _cgmX_BEG_MF_DEFAULTS  = { "BEG_MF_DEFAULTS"    , "\x31\x2b" };
static const comando _cgmX_END_MF_DEFAULTS  = { "END_MF_DEFAULTS"    , "\x31\x2c" };
static const comando _cgmX_FONT_LIST        = { "FONT_LIST"          , "\x31\x2d" };
static const comando _cgmX_CHAR_SET_LIST    = { "CHAR_SET_LIST"      , "\x31\x2e" };
static const comando _cgmX_CHAR_CODING      = { "CHAR_CODING"        , "\x31\x2f" };

/* picture descriptor elements */

static const comando _cgmX_SCALE_MODE       = { "SCALE_MODE"         , "\x30\x20" };
static const comando _cgmX_COLR_MODE        = { "COLR_MODE"          , "\x30\x21" };
static const comando _cgmX_LINE_WIDTH_MODE  = { "LINE_WIDTH_MODE"    , "\x30\x22" };
static const comando _cgmX_MARKER_SIZE_MODE = { "MARKER_SIZE_MODE"   , "\x30\x23" };
static const comando _cgmX_EDGE_WIDTH_MODE  = { "EDGE_WIDTH_MODE"    , "\x30\x24" };
static const comando _cgmX_VDC_EXTENT       = { "VDC_EXT"            , "\x30\x25" };
static const comando _cgmX_BACK_COLR        = { "BACK_COLR"          , "\x30\x26" };

/* control elements */

static const comando _cgmX_VDC_INTEGER_PREC = { "VDC_INTEGER_PREC"   , "\x30\x20" };
static const comando _cgmX_VDC_REAL_PREC    = { "VDC_REAL_PREC"      , "\x30\x21" };
static const comando _cgmX_AUX_COLR         = { "AUX_COLR"           , "\x30\x22" };
static const comando _cgmX_TRANSPARENCY     = { "TRANSPARENCY"       , "\x30\x23" };
static const comando _cgmX_CLIP_RECT        = { "CLIP_RECT"          , "\x30\x24" };
static const comando _cgmX_CLIP             = { "CLIP"               , "\x30\x25" };

/* primitive elements */

static const comando _cgmX_LINE             = { "LINE"               , "\x20" };
static const comando _cgmX_DISJT_LINE       = { "DISJT_LINE"         , "\x21" };
static const comando _cgmX_MARKER           = { "MARKER"             , "\x22" };
static const comando _cgmX_TEXT             = { "TEXT"               , "\x23" };
static const comando _cgmX_RESTR_TEXT       = { "RESTR_TEXT"         , "\x24" };
static const comando _cgmX_APND_TEXT        = { "APND_TEXT"          , "\x25" };
static const comando _cgmX_POLYGON          = { "POLYGON"            , "\x26" };
static const comando _cgmX_POLYGON_SET      = { "POLYGON_SET"        , "\x27" };
static const comando _cgmX_CELL_ARRAY       = { "CELL_ARRAY"         , "\x28" };
static const comando _cgmX_GDP              = { "GDP"                , "\x29" };
static const comando _cgmX_RECT             = { "RECT"               , "\x2a" };
static const comando _cgmX_CIRCLE           = { "CIRCLE"             , "\x34\x20" };
static const comando _cgmX_ARC_3_PT         = { "ARC_3_PT"           , "\x34\x21" };
static const comando _cgmX_ARC_3_PT_CLOSE   = { "ARC_3_PT_CLOSE"     , "\x34\x22" };
static const comando _cgmX_ARC_CTR          = { "ARC_CTR"            , "\x34\x23" };
static const comando _cgmX_ARC_CTR_CLOSE    = { "ARC_CTR_CLOSE"      , "\x34\x24" };
static const comando _cgmX_ELLIPSE          = { "ELLIPSE"            , "\x34\x25" };
static const comando _cgmX_ELLIP_ARC        = { "ELLIP_ARC"          , "\x34\x26" };
static const comando _cgmX_ELLIP_ARC_CLOSE  = { "ELLIP_ARC_CLOSE"    , "\x34\x27" };

/* attribute elements */

static const comando _cgmX_LINE_INDEX       = { "LINE_INDEX"         , "\x35\x20" };
static const comando _cgmX_LINE_TYPE        = { "LINE_TYPE"          , "\x35\x21" };
static const comando _cgmX_LINE_WIDTH       = { "LINE_WIDTH"         , "\x35\x22" };
static const comando _cgmX_LINE_COLR        = { "LINE_COLR"          , "\x35\x23" };
static const comando _cgmX_MARKER_INDEX     = { "MARKER_INDEX"       , "\x35\x24" };
static const comando _cgmX_MARKER_TYPE      = { "MARKER_TYPE"        , "\x35\x25" };
static const comando _cgmX_MARKER_WIDTH     = { "MARKER_SIZE"        , "\x35\x26" };
static const comando _cgmX_MARKER_COLR      = { "MARKER_COLR"        , "\x35\x27" };
static const comando _cgmX_TEXT_INDEX       = { "TEXT_INDEX"         , "\x35\x30" };
static const comando _cgmX_TEXT_FONT_INDEX  = { "TEXT_FONT_INDEX"    , "\x35\x31" };
static const comando _cgmX_TEXT_PREC        = { "TEXT_PREC"          , "\x35\x32" };
static const comando _cgmX_CHAR_EXPAN       = { "CHAR_EXPAN"         , "\x35\x33" };
static const comando _cgmX_CHAR_SPACE       = { "CHAR_SPACE"         , "\x35\x34" };
static const comando _cgmX_TEXT_COLR        = { "TEXT_COLR"          , "\x35\x35" };
static const comando _cgmX_CHAR_HEIGHT      = { "CHAR_HEIGHT"        , "\x35\x36" };
static const comando _cgmX_CHAR_ORI         = { "CHAR_ORI"           , "\x35\x37" };
static const comando _cgmX_TEXT_PATH        = { "TEXT_PATH"          , "\x35\x38" };
static const comando _cgmX_TEXT_ALIGN       = { "TEXT_ALIGN"         , "\x35\x39" };
static const comando _cgmX_CHAR_SET_INDEX   = { "CHAR_SET_INDEX"     , "\x35\x3a" };
static const comando _cgmX_ALT_CHAR_SET     = { "ALT_CHAR_SET_INDEX" , "\x35\x3b" };
static const comando _cgmX_FILL_INDEX       = { "FILL_INDEX"         , "\x36\x20" };
static const comando _cgmX_INT_STYLE        = { "INT_STYLE"          , "\x36\x21" };
static const comando _cgmX_FILL_COLR        = { "FILL_COLR"          , "\x36\x22" };
static const comando _cgmX_HATCH_INDEX      = { "HATCH_INDEX"        , "\x36\x23" };
static const comando _cgmX_PAT_INDEX        = { "PAT_INDEX"          , "\x36\x24" };
static const comando _cgmX_EDGE_INDEX       = { "EDGE_INDEX"         , "\x36\x25" };
static const comando _cgmX_EDGE_TYPE        = { "EDGE_TYPE"          , "\x36\x26" };
static const comando _cgmX_EDGE_WIDTH       = { "EDGE_WIDTH"         , "\x36\x27" };
static const comando _cgmX_EDGE_COLR        = { "EDGE_COLR"          , "\x36\x28" };
static const comando _cgmX_EDGE_VIS         = { "EDGE_VIS"           , "\x36\x29" };
static const comando _cgmX_FILL_REF_PT      = { "FILL_REF_PT"        , "\x36\x2a" };
static const comando _cgmX_PAT_TABLE        = { "PAT_TABLE"          , "\x36\x2b" };
static const comando _cgmX_PAT_SIZE         = { "PAT_SIZE"           , "\x36\x2c" };
static const comando _cgmX_COLR_TABLE       = { "COLR_TABLE"         , "\x36\x30" };
static const comando _cgmX_ASF              = { "ASF"                , "\x36\x31" };

/* escape elements */

static const comando _cgmX_ESCAPE           = { "ESCAPE"             , "\x37\x20" };
static const comando _cgmX_DOMAIN_RING      = { "DOMAIN_RING"        , "\x37\x30" };

/* external elements */

static const comando _cgmX_MESSAGE          = { "MESSAGE"            , "\x37\x21" };
static const comando _cgmX_APPL_DATA        = { "APPL_DATA"          , "\x37\x22" };

/* drawing sets */
static const comando _cgmX_DRAWING_SET      = { "drawing_set"        , "\x40"     };
static const comando _cgmX_DRAWING_PLUS     = { "drawing_plus"       , "\x41"     };

static const comando *_elements_list_sets[] = {
         & _cgmX_DRAWING_SET,
         & _cgmX_DRAWING_PLUS,
         NULL };


static const comando *delimiter[] = {
         & _cgmX_NULL,
         & _cgmX_BEGMF,
         & _cgmX_ENDMF,
         & _cgmX_BEG_PIC,
         & _cgmX_BEG_PIC_BODY,
         & _cgmX_END_PIC,
         NULL };

static const comando *metafile[] = {
         & _cgmX_END_MF_DEFAULTS,
         & _cgmX_MF_VERSION,
         & _cgmX_MF_DESC,
         & _cgmX_VDC_TYPE,
         & _cgmX_INTEGER_PREC,
         & _cgmX_REAL_PREC,
         & _cgmX_INDEX_PREC,
         & _cgmX_COLR_PREC,
         & _cgmX_COLR_INDEX_PREC,
         & _cgmX_MAX_COLR_INDEX,
         & _cgmX_COLR_VALUE_EXT,
         & _cgmX_MF_ELEM_LIST,
         & _cgmX_BEG_MF_DEFAULTS,
         & _cgmX_FONT_LIST,
         & _cgmX_CHAR_SET_LIST,
         & _cgmX_CHAR_CODING,
         NULL };

static const comando *picture[] = {
         & _cgmX_NULL,
         & _cgmX_SCALE_MODE,
         & _cgmX_COLR_MODE,
         & _cgmX_LINE_WIDTH_MODE,
         & _cgmX_MARKER_SIZE_MODE,
         & _cgmX_EDGE_WIDTH_MODE,
         & _cgmX_VDC_EXTENT,
         & _cgmX_BACK_COLR,
         NULL };

static const comando *control[] = {
         & _cgmX_NULL,
         & _cgmX_VDC_INTEGER_PREC,
         & _cgmX_VDC_REAL_PREC,
         & _cgmX_AUX_COLR,
         & _cgmX_TRANSPARENCY,
         & _cgmX_CLIP_RECT,
         & _cgmX_CLIP,
         NULL };

static const comando *primitive[] = {
         & _cgmX_NULL,
         & _cgmX_LINE,
         & _cgmX_DISJT_LINE,
         & _cgmX_MARKER,
         & _cgmX_TEXT,
         & _cgmX_RESTR_TEXT,
         & _cgmX_APND_TEXT,
         & _cgmX_POLYGON,
         & _cgmX_POLYGON_SET,
         & _cgmX_CELL_ARRAY,
         & _cgmX_GDP,
         & _cgmX_RECT,
         & _cgmX_CIRCLE,
         & _cgmX_ARC_3_PT,
         & _cgmX_ARC_3_PT_CLOSE,
         & _cgmX_ARC_CTR,
         & _cgmX_ARC_CTR_CLOSE,
         & _cgmX_ELLIPSE,
         & _cgmX_ELLIP_ARC,
         & _cgmX_ELLIP_ARC_CLOSE,
         NULL };

static const comando *attributes[] = {
         & _cgmX_NULL,
         & _cgmX_LINE_INDEX,
         & _cgmX_LINE_TYPE,
         & _cgmX_LINE_WIDTH,
         & _cgmX_LINE_COLR,
         & _cgmX_MARKER_INDEX,
         & _cgmX_MARKER_TYPE,
         & _cgmX_MARKER_WIDTH,
         & _cgmX_MARKER_COLR,
         & _cgmX_TEXT_INDEX,
         & _cgmX_TEXT_FONT_INDEX,
         & _cgmX_TEXT_PREC,
         & _cgmX_CHAR_EXPAN,
         & _cgmX_CHAR_SPACE,
         & _cgmX_TEXT_COLR,
         & _cgmX_CHAR_HEIGHT,
         & _cgmX_CHAR_ORI,
         & _cgmX_TEXT_PATH,
         & _cgmX_TEXT_ALIGN,
         & _cgmX_CHAR_SET_INDEX,
         & _cgmX_ALT_CHAR_SET,
         & _cgmX_FILL_INDEX,
         & _cgmX_INT_STYLE,
         & _cgmX_FILL_COLR,
         & _cgmX_HATCH_INDEX,
         & _cgmX_PAT_INDEX,
         & _cgmX_EDGE_INDEX,
         & _cgmX_EDGE_TYPE,
         & _cgmX_EDGE_WIDTH,
         & _cgmX_EDGE_COLR,
         & _cgmX_EDGE_VIS,
         & _cgmX_FILL_REF_PT,
         & _cgmX_PAT_TABLE,
         & _cgmX_PAT_SIZE,
         & _cgmX_COLR_TABLE,
         & _cgmX_ASF,
         NULL };

static const comando *escape[] = {
         & _cgmX_NULL,
         & _cgmX_ESCAPE,
         & _cgmX_DOMAIN_RING,
         NULL };

static const comando *external[] = {
         & _cgmX_NULL,
         & _cgmX_MESSAGE,
         & _cgmX_APPL_DATA,
         NULL };

static const comando **comandos[] = {
         _elements_list_sets,
         delimiter,
         metafile,
         picture,
         control,
         primitive,
         attributes,
         escape,
         external,
         NULL };

#define unit (cgm->fp)

/************************************************
*                                               *
*         listas de funcoes necessarias         *
*                                               *
************************************************/


static void cgmb_wch ( CGM *, int, int, int );          /* write command header */
static void cgmb_ci  ( CGM *, unsigned long );          /* put colour index at colour index precision */
static void cgmb_cd  ( CGM *, double );                 /* put color direct at colour direct precision */
static void cgmb_rgb ( CGM *, double, double, double ); /* put color direct (rgb) at colour direct precision */
static void cgmb_ix  ( CGM *, long );                   /* put index at index precision */
static void cgmb_e   ( CGM *, int, const char *l[] );     /* put enum ( int*2 ) */
static void cgmb_i   ( CGM *, long );                   /* put int  ( integer precision ) */
static void cgmb_u   ( CGM *, unsigned long );          /* put unsigned int  ( integer precision ) */
static void cgmb_r   ( CGM *, double );                 /* put real ( real precision ) */
static void cgmb_s   ( CGM *, const char * );           /* put string */
static void cgmb_vdc ( CGM *, double );                 /* put VDC at VDC mode and precision */
static void cgmb_p   ( CGM *, double, double );         /* put point at VDC mode and precision */
static void cgmb_co  ( CGM *, const void * );           /* put colour at colour mode and precision */
static void cgmb_sep ( CGM *, const char * );           /* put separator */
static int  cgmb_get_col ( CGM * );                     /* get column position */
static void cgmb_align ( CGM *, int );                  /* align at column number */
static void cgmb_nl    ( CGM * );                       /* new line */
static int  cgmb_term  ( CGM * );                       /* terminate element */

static const CGMF cgmf_binary = {
         cgmb_wch    ,
         cgmb_ci     ,
         cgmb_cd     ,
         cgmb_rgb    ,
         cgmb_ix     ,
         cgmb_e      ,
         cgmb_i      ,
         cgmb_u      ,
         cgmb_r      ,
         cgmb_s      ,
         cgmb_vdc    ,
         cgmb_p      ,
         cgmb_co     ,
         cgmb_sep    ,
         cgmb_get_col,
         cgmb_align  ,
         cgmb_nl     ,
         cgmb_term
         };

static void cgmt_wch ( CGM *, int, int, int );          /* write command header */
static void cgmt_ci  ( CGM *, unsigned long );          /* put colour index at colour index precision */
static void cgmt_cd  ( CGM *, double );                 /* put color direct at colour direct precision */
static void cgmt_rgb ( CGM *, double, double, double ); /* put color direct (rgb) at colour direct precision */
static void cgmt_ix  ( CGM *, long );                   /* put index at index precision */
static void cgmt_e   ( CGM *, int, const char *l[] );     /* put enum ( int*2 ) */
static void cgmt_i   ( CGM *, long );                   /* put int  ( integer precision ) */
static void cgmt_u   ( CGM *, unsigned long );          /* put unsigned int  ( integer precision ) */
static void cgmt_r   ( CGM *, double );                 /* put real ( real precision ) */
static void cgmt_s   ( CGM *, const char * );           /* put string */
static void cgmt_vdc ( CGM *, double );                 /* put VDC at VDC mode and precision */
static void cgmt_p   ( CGM *, double, double );         /* put point at VDC mode and precision */
static void cgmt_co  ( CGM *, const void * );           /* put colour at colour mode and precision */
static void cgmt_sep ( CGM *, const char * );           /* put separator */
static int  cgmt_get_col ( CGM * );                     /* get column position */
static void cgmt_align ( CGM *, int );                  /* align at column number */
static void cgmt_nl    ( CGM * );                       /* new line */
static int  cgmt_term  ( CGM * );                       /* terminate element */

static const CGMF cgmf_clear_text = {
         cgmt_wch    ,
         cgmt_ci     ,
         cgmt_cd     ,
         cgmt_rgb    ,
         cgmt_ix     ,
         cgmt_e      ,
         cgmt_i      ,
         cgmt_u      ,
         cgmt_r      ,
         cgmt_s      ,
         cgmt_vdc    ,
         cgmt_p      ,
         cgmt_co     ,
         cgmt_sep    ,
         cgmt_get_col,
         cgmt_align  ,
         cgmt_nl     ,
         cgmt_term
         };

static void cgmc_wch ( CGM *, int, int, int );          /* write command header */
static void cgmc_ci  ( CGM *, unsigned long );          /* put colour index at colour index precision */
static void cgmc_cd  ( CGM *, double );                 /* put color direct at colour direct precision */
static void cgmc_rgb ( CGM *, double, double, double ); /* put color direct (rgb) at colour direct precision */
static void cgmc_ix  ( CGM *, long );                   /* put index at index precision */
static void cgmc_e   ( CGM *, int, const char *l[] );     /* put enum ( int*2 ) */
static void cgmc_i   ( CGM *, long );                   /* put int  ( integer precision ) */
static void cgmc_u   ( CGM *, unsigned long );          /* put unsigned int  ( integer precision ) */
static void cgmc_r   ( CGM *, double );                 /* put real ( real precision ) */
static void cgmc_s   ( CGM *, const char * );           /* put string */
static void cgmc_vdc ( CGM *, double );                 /* put VDC at VDC mode and precision */
static void cgmc_p   ( CGM *, double, double );         /* put point at VDC mode and precision */
static void cgmc_co  ( CGM *, const void * );           /* put colour at colour mode and precision */
static void cgmc_sep ( CGM *, const char * );           /* put separator */
static int  cgmc_get_col ( CGM * );                     /* get column position */
static void cgmc_align ( CGM *, int );                  /* align at column number */
static void cgmc_nl    ( CGM * );                       /* new line */
static int  cgmc_term  ( CGM * );                       /* terminate element */

static const CGMF cgmf_character = {
         cgmc_wch    ,
         cgmc_ci     ,
         cgmc_cd     ,
         cgmc_rgb    ,
         cgmc_ix     ,
         cgmc_e      ,
         cgmc_i      ,
         cgmc_u      ,
         cgmc_r      ,
         cgmc_s      ,
         cgmc_vdc    ,
         cgmc_p      ,
         cgmc_co     ,
         cgmc_sep    ,
         cgmc_get_col,
         cgmc_align  ,
         cgmc_nl     ,
         cgmc_term
         };

static const CGMF *cgmf[] = { &cgmf_character, &cgmf_binary, &cgmf_clear_text };

/************************************************
*                                               *
*             Funcoes para binario              *
*                                               *
************************************************/

#define cgmb_putw cgmb_putu16
#define cgmb_putb(a,b) cgmb_putc((a),(int)(b))

static void cgmb_putw ( CGM *, unsigned );

static void cgmb_putc ( CGM *cgm, int b )
{

 if ( cgm->op != -1 )
   {
   register int i;
   for ( i=cgm->op; i>=0; i-- )
      {
      if ( cgm->bc[i] == 32766 - 2*i )
         {
         long po = ftell(unit);
         int op  = cgm->op;

         cgm->op = -1;
         fseek(unit, cgm->po[i] , SEEK_SET);
         cgmb_putw ( cgm, (1 << 15) | (cgm->bc[i]) );

         cgm->op = i - 1;
         fseek(unit, po, SEEK_SET);
         cgmb_putw ( cgm, 0 );

         cgm->op    = op;
         cgm->bc[i] = 0;
         cgm->po[i] = po;
         }
      cgm->bc[i] ++;
      }
   }

 fputc ( b, unit );
}


static void cgmb_puti8  ( CGM *cgm, int b )
{
 cgmb_putb ( cgm, b );
}

static void cgmb_puti16 ( CGM *cgm, int b )
{
 cgmb_putb ( cgm, b >> 8 );
 cgmb_putb ( cgm, b      );
}

static void cgmb_puti24 ( CGM *cgm, long b )
{
 cgmb_putb ( cgm, b >> 16 );
 cgmb_putb ( cgm, b >>  8 );
 cgmb_putb ( cgm, b       );
}

static void cgmb_puti32 ( CGM *cgm, long b )
{
 cgmb_putb ( cgm, b >> 24 );
 cgmb_putb ( cgm, b >> 16 );
 cgmb_putb ( cgm, b >>  8 );
 cgmb_putb ( cgm, b       );
}

static void cgmb_putu8  ( CGM *cgm, unsigned int b )
{
 cgmb_putb ( cgm, b );
}

static void cgmb_putu16 ( CGM *cgm, unsigned int b )
{
 cgmb_putb ( cgm, b >> 8 );
 cgmb_putb ( cgm, b      );
}

static void cgmb_putu24 ( CGM *cgm, unsigned long b )
{
 cgmb_putb ( cgm, b >> 16 );
 cgmb_putb ( cgm, b >>  8 );
 cgmb_putb ( cgm, b       );
}

static void cgmb_putu32 ( CGM *cgm, unsigned long b )
{
 cgmb_putb ( cgm, b >> 24 );
 cgmb_putb ( cgm, b >> 16 );
 cgmb_putb ( cgm, b >>  8 );
 cgmb_putb ( cgm, b       );
}

static void cgmb_putfl32 ( CGM *cgm, float b )
{
 union {
  float f;
  long  l;
  } r;
 r.f = b;
 cgmb_putb ( cgm, r.l >> 24 );
 cgmb_putb ( cgm, r.l >> 16 );
 cgmb_putb ( cgm, r.l >>  8 );
 cgmb_putb ( cgm, r.l       );
}

static void cgmb_putfl64 ( CGM *cgm, double b )
{
 union {
  double d;
  long   l[2];
  } r;
 r.d = b;
 cgmb_putb ( cgm, r.l[1] >> 24 );
 cgmb_putb ( cgm, r.l[1] >> 16 );
 cgmb_putb ( cgm, r.l[1] >>  8 );
 cgmb_putb ( cgm, r.l[1]       );
 cgmb_putb ( cgm, r.l[0] >> 24 );
 cgmb_putb ( cgm, r.l[0] >> 16 );
 cgmb_putb ( cgm, r.l[0] >>  8 );
 cgmb_putb ( cgm, r.l[0]       );
}

static void cgmb_putfx32 ( CGM *cgm, float b )
{
          int  si = (          int  ) floor ( b );
 unsigned int  ui = ( unsigned int  ) ( (b - si) * 65536.0 );

 cgmb_puti16 ( cgm, si );
 cgmb_puti16 ( cgm, ui );
}

static void cgmb_putfx64 ( CGM *cgm, double b )
{
          long si = (          long ) floor ( b );
 unsigned long ui = ( unsigned long ) ( (b - si) * 65536.0 * 65536.0 );

 cgmb_puti32 ( cgm, si );
 cgmb_puti32 ( cgm, ui );
}

static void cgmb_wch ( CGM* cgm, int c, int id, int len )
{

/* if ( len & 1 ) len ++; */ /* word aligned */

 if ( len > 30 )
    cgmb_putw ( cgm, (c << 12) | ( id << 5 ) | 31 );
 else
    cgmb_putw ( cgm, (c << 12) | ( id << 5 ) | (int)(len) );


 cgm->op++;

 if ( len > 30 )
    {
    cgm->po[cgm->op] = ftell(unit);
    cgmb_putw ( cgm, 0 );
    }
 else
    cgm->po[cgm->op] = 0L;

 cgm->bc[cgm->op] = 0;

}

static void cgmb_ci ( CGM *cgm, unsigned long ci )
{
 switch ( cgm->cix_prec )
    {
    case 0: cgmb_putu8  ( cgm, (unsigned)ci ); break;
    case 1: cgmb_putu16 ( cgm, (unsigned)ci ); break;
    case 2: cgmb_putu24 ( cgm, ci ); break;
    case 3: cgmb_putu32 ( cgm, ci ); break;
    }
}

static void cgmb_cd ( CGM *cgm, double cd )
{
 unsigned long cv = (unsigned long) (cd * (pow(2.0, (cgm->cd_prec + 1) * 8.0) - 1));
 switch ( cgm->cd_prec )
    {
    case 0: cgmb_putu8  ( cgm, (unsigned)cv ); break;
    case 1: cgmb_putu16 ( cgm, (unsigned)cv ); break;
    case 2: cgmb_putu24 ( cgm, cv ); break;
    case 3: cgmb_putu32 ( cgm, cv ); break;
    }
}

static void cgmb_rgb ( CGM *cgm, double r, double g, double b )
{
 cgmb_cd ( cgm, r );
 cgmb_cd ( cgm, g );
 cgmb_cd ( cgm, b );
}

static void cgmb_ix ( CGM *cgm, long ix )
{
 switch ( cgm->ix_prec )
    {
    case 0: cgmb_puti8  ( cgm, (int)ix ); break;
    case 1: cgmb_puti16 ( cgm, (int)ix ); break;
    case 2: cgmb_puti24 ( cgm, ix ); break;
    case 3: cgmb_puti32 ( cgm, ix ); break;
    }
}

static void cgmb_e ( CGM *cgm, int e, const char *el[] )
{
 cgmb_puti16 ( cgm, e );
}

static void cgmb_i ( CGM *cgm, long i )
{
 switch ( cgm->int_prec )
    {
    case 0: cgmb_puti8  ( cgm, (int)i ); break;
    case 1: cgmb_puti16 ( cgm, (int)i ); break;
    case 2: cgmb_puti24 ( cgm, i ); break;
    case 3: cgmb_puti32 ( cgm, i ); break;
    }
}

static void cgmb_u ( CGM *cgm, unsigned long i )
{
 switch ( cgm->int_prec )
    {
    case 0: cgmb_putu8  ( cgm, (unsigned)i ); break;
    case 1: cgmb_putu16 ( cgm, (unsigned)i ); break;
    case 2: cgmb_putu24 ( cgm, i ); break;
    case 3: cgmb_putu32 ( cgm, i ); break;
    }
}

static void cgmb_r ( CGM *cgm, double f )
{
 switch ( cgm->real_prec )
    {
    case 0: cgmb_putfl32 ( cgm, (float )f ); break;
    case 1: cgmb_putfl64 ( cgm, (double)f ); break;
    case 2: cgmb_putfx32 ( cgm, (float )f ); break;
    case 3: cgmb_putfx64 ( cgm, (double)f ); break;
    }
}

static void cgmb_s ( CGM *cgm, const char *s )
{
 register unsigned i;
 unsigned l = strlen(s);
 int bc;

 if ( l > 254 )
    {
    cgmb_putu8(cgm,255);
    if ( l > 32763 )
       cgmb_putu16 ( cgm, (1<<16) | 32763 );
    else
       cgmb_putu16 ( cgm, l );
    bc = 1;
    }
 else
    cgmb_putu8(cgm,l);

 for ( i=0; s[i]; s++ )
    {
    if ( (i + bc) == 32766 )
       {
       l -= i;
       s += i;
       i  = 0;
       bc = 0;
       if ( l > 32764 )
          cgmb_putu16 ( cgm, (1<<16) | 32764 );
       else
          cgmb_putu16 ( cgm, l );
       }
    cgmb_putc ( cgm, s[i] );
    }
}

static void cgmb_vdc ( CGM *cgm, double vdc)
{
 if ( cgm->vdc_type == 0 )
    switch ( cgm->vdc_int )
       {
       case 0: cgmb_puti8  ( cgm, (int )vdc ); break;
       case 1: 
           /* Evita overflow em ambientes de 32 bits */          
           if (vdc < -32768)     vdc = -32768;
           else if (vdc > 32767) vdc = +32767;
           cgmb_puti16 ( cgm, (int) vdc ); 
           break;
       case 2: cgmb_puti24 ( cgm, (long)vdc ); break;
       case 3:
           /* Evita overflow em ambientes de 32 bits */
           if (vdc < (double)-2147483648.0)     vdc = -2147483648L;
           else if (vdc > (double)2147483647.0) vdc = +2147483647L;
           cgmb_puti32 ( cgm, (long)vdc );
           break;

       }
 else
    switch ( cgm->vdc_real )
       {
       case 0: cgmb_putfl32 ( cgm, (float )vdc ); break;
       case 1: cgmb_putfl64 ( cgm, (double)vdc ); break;
       case 2: cgmb_putfx32 ( cgm, (float )vdc ); break;
       case 3: cgmb_putfx64 ( cgm, (double)vdc ); break;
       }

}

static void cgmb_p ( CGM *cgm, double x, double y)
{
 cgmb_vdc ( cgm, x );
 cgmb_vdc ( cgm, y );
}

static void cgmb_co ( CGM *cgm, const void * co)
{
 if ( cgm->clrsm == 0 ) /* indexed */
    {
    unsigned long ci = *(unsigned long *)co;
    cgmb_ci ( cgm, ci );
    }
 else
    {
    double *cb = (double *) co;
    cgmb_rgb ( cgm, cb[0], cb[1], cb[2] );
    }
}

static void cgmb_sep ( CGM *cgm, const char * sep )
{}

static int  cgmb_get_col ( CGM *cgm )
{
 return 0;
}

static void cgmb_align ( CGM *cgm, int n )
{}

static void cgmb_nl    ( CGM *cgm )
{}

static int cgmb_term ( CGM *cgm )
{
 if ( cgm->op != -1 )
    {
    if ( cgm->bc[cgm->op] & 1 )
       {
       cgmb_putb( cgm, 0 );
       cgm->bc[cgm->op] --;
       }

    if ( cgm->po[cgm->op] != 0L )
       {
       long po = ftell(unit);
       int  op = cgm->op;

       cgm->op = -1;
       fseek ( unit, cgm->po[op], SEEK_SET);
       cgmb_putw ( cgm, cgm->bc[op] );

       fseek ( unit, po, SEEK_SET );
       cgm->op = op;
       }
    cgm->op --;
    }

 return 0;
}

/************************************************
*                                               *
*            Funcoes para clear text            *
*                                               *
************************************************/

static void cgmt_wch ( CGM* cgm, int c, int id, int len )
{
 cgm->cl += fprintf ( unit, "%s", comandos[c+1][id]->ct );
}

static void cgmt_ci ( CGM *cgm, unsigned long ci )
{
 cgm->f->u ( cgm, ci );
}

static void cgmt_cd ( CGM *cgm, double cd )
{
 unsigned long cv = (unsigned long) (cd * (pow(2.0, (cgm->cd_prec + 1) * 8.0) - 1));

 cgm->f->u ( cgm, cv );
}

static void cgmt_rgb ( CGM *cgm, double r, double g, double b )
{
 cgm->f->cd  ( cgm, r );
 cgm->f->cd  ( cgm, g );
 cgm->f->cd  ( cgm, b );
}

static void cgmt_ix ( CGM *cgm, long ix )
{
 cgm->f->i ( cgm, ix );
}

static void cgmt_e ( CGM *cgm, int e, const char *el[] )
{
 cgm->cl += fprintf ( unit, " %s", el[e] );
}

static void cgmt_i ( CGM *cgm, long i )
{
 cgm->cl += fprintf ( unit, " %ld", i );
}

static void cgmt_u ( CGM *cgm, unsigned long i )
{
 cgm->cl += fprintf ( unit, " %lu", i );
}

static void cgmt_r ( CGM *cgm, double f )
{
 cgm->cl += fprintf ( unit, " %g", f );
}

static void cgmt_s ( CGM *cgm, const char *s )
{
 register unsigned i;
 fputc ( 34, unit );

 for ( i=0; s[i]; i++ )
   {
   if ( s[i] == 34 )
      {
      fputc ( 34, unit );
      cgm->cl ++;
      }
   fputc ( s[i], unit );
   }

 fputc ( 34, unit );
 cgm->cl += strlen (s) + 2;
}

static void cgmt_vdc ( CGM *cgm, double vdc)
{
 if ( cgm->vdc_type == 0 )
 {
    /* Evita overflow em ambientes de 32 bits */
    if (vdc < (double)-2147483648.0)     vdc = -2147483648L;
    else if (vdc > (double)2147483647.0) vdc = +2147483647L;

    cgm->f->i ( cgm, (long) vdc );
 }
 else
    cgm->f->r ( cgm,        vdc );
}

static void cgmt_p ( CGM *cgm, double x, double y)
{
 cgm->f->sep ( cgm, "(" );
 cgm->f->vdc ( cgm, x );
 cgm->f->sep ( cgm, "," );
 cgm->f->vdc ( cgm, y );
 cgm->f->sep ( cgm, ")" );
}

static void cgmt_co ( CGM *cgm, const void * co)
{
 if ( cgm->clrsm == 0 ) /* indexed */
    {
    unsigned long ci = *(unsigned *)co;
    cgm->f->ci ( cgm, ci );
    }
 else
    {
    double *cb = (double *) co;
    cgm->f->rgb ( cgm, cb[0], cb[1], cb[2] );
    }
}

static void cgmt_sep ( CGM *cgm, const char * sep )
{
 cgm->cl += fprintf ( unit, " %s", sep );
}

static int  cgmt_get_col ( CGM *cgm )
{
 return cgm->cl;
}

static void cgmt_align ( CGM *cgm, int n )
{
 for ( ; cgm->cl < n ; cgm->cl ++ )
    fputc ( ' ', unit );
}

static void cgmt_nl    ( CGM *cgm )
{
 fputc ( '\n', unit );
 cgm->cl = 1;
}

static int cgmt_term ( CGM *cgm )
{
 fputc ( ';', unit );
 cgm->f->nl(cgm);
 return 0;
}

/************************************************
*                                               *
*            Funcoes para character             *
*                                               *
************************************************/

static void cgmc_wch ( CGM* cgm, int c, int id, int len )
{
 cgm->cl += fprintf ( unit, "%s", comandos[c+1][id]->ct );
}

static void cgmc_ci ( CGM *cgm, unsigned long ci )
{
 cgm->f->u ( cgm, ci );
}

static void cgmc_cd ( CGM *cgm, double cd )
{
 cgm->f->r ( cgm, cd );
}

static void cgmc_rgb ( CGM *cgm, double r, double g, double b )
{
 cgm->f->cd  ( cgm, r );
 cgm->f->sep ( cgm, "," );
 cgm->f->cd  ( cgm, g );
 cgm->f->sep ( cgm, "," );
 cgm->f->cd  ( cgm, b );
}

static void cgmc_ix ( CGM *cgm, long ix )
{
 cgm->f->i ( cgm, ix );
}

static void cgmc_e ( CGM *cgm, int e, const char *el[] )
{
 cgm->cl += fprintf ( unit, " %s", el[e] );
}

static void cgmc_i ( CGM *cgm, long i )
{
 cgm->cl += fprintf ( unit, " %ld", i );
}

static void cgmc_u ( CGM *cgm, unsigned long i )
{
 cgm->cl += fprintf ( unit, " %lu", i );
}

static void cgmc_r ( CGM *cgm, double f )
{
 cgm->cl += fprintf ( unit, " %g", f );
}

static void cgmc_s ( CGM *cgm, const char *s )
{
 register unsigned i;
 fputc ( 34, unit );

 for ( i=0; s[i]; i++ )
   {
   if ( s[i] == 34 )
      {
      fputc ( 34, unit );
      cgm->cl ++;
      }
   fputc ( s[i], unit );
   }

 fputc ( 34, unit );
 cgm->cl += strlen (s) + 2;
}

static void cgmc_vdc ( CGM *cgm, double vdc)
{
 if ( cgm->vdc_type == 0 )
    cgm->f->i ( cgm, (long) vdc );
 else
    cgm->f->r ( cgm,        vdc );
}

static void cgmc_p ( CGM *cgm, double x, double y)
{
 cgm->f->sep ( cgm, "(" );
 cgm->f->vdc ( cgm, x );
 cgm->f->sep ( cgm, "," );
 cgm->f->vdc ( cgm, y );
 cgm->f->sep ( cgm, ")" );
}

static void cgmc_co ( CGM *cgm, const void * co)
{
 if ( cgm->clrsm == 0 ) /* indexed */
    {
    unsigned long ci = *(unsigned long *)co;
    cgm->f->ci ( cgm, ci );
    }
 else
    {
    double *cb = (double *) co;
    cgm->f->rgb ( cgm, cb[0], cb[1], cb[2] );
    }
}

static void cgmc_sep ( CGM *cgm, const char * sep )
{
 cgm->cl += fprintf ( unit, " %s", sep );
}

static int  cgmc_get_col ( CGM *cgm )
{
 return cgm->cl;
}

static void cgmc_align ( CGM *cgm, int n )
{
 for ( ; cgm->cl < n ; cgm->cl ++ )
    fputc ( ' ', unit );
}

static void cgmc_nl    ( CGM *cgm )
{
 fputc ( '\n', unit );
 cgm->cl = 1;
}

static int cgmc_term ( CGM *cgm )
{
 fputc ( ';', unit );
 cgm->f->nl(cgm);
 return 0;
}

/************************************************
*                                               *
*          independente de codificacao          *
*                                               *
************************************************/


/* Definicoes de precisao */

static const long _cgm_int_precs[][2] = {
         {          -128,           127 },      /*  8 */
         {       -32768L,         32767 },      /* 16 */
         { LONG_MIN >> 8, LONG_MAX >> 8 },      /* 24 */
         { LONG_MIN     , LONG_MAX      } };    /* 32 */

static int _cgm_ireal_precs[][4] = {
         { 0,  9, 23, 0 }, /* float*32 */
         { 0, 12, 52, 0 }, /* float*64 */
         { 1, 16, 16,  5 },      /* fixed*32 */
         { 1, 32, 32,  9 } };    /* fixed*64 */

static double _cgm_rreal_precs[][2] = {
         /* float*32 */ { 0, 0 },   /* Em Turbo C, FLT_MAX e DLB_MAX sao */
         /* float*64 */ { 0, 0 },   /* DEFINES para variaveis externas   */
         /* fixed*32 */ { - (32769.0 - 1.0 / 65536.0),
                             32768.0 - 1.0 / 65536.0 },
         /* fixed*64 */ { (double)(LONG_MIN) - ( 1 - 1 / ( 65536.0 * 65536.0 ) ),
                          (double)(LONG_MAX) + ( 1 - 1 / ( 65536.0 * 65536.0 ) ) } };

/* Enumeraveis genericos */

static const char *offon[] =  { "OFF", "ON" };

/*********************
* Delimiter elements *
*********************/

CGM *cgm_begin_metafile ( char *file, int mode, char *header )
{
 CGM *cgm;

 if ( (cgm = (CGM *)malloc ( sizeof (CGM) ) ) == NULL )
       return NULL;

#ifdef __VAXC__

 if ( mode == 2 )
    cgm->fp = fopen ( file , "w"  , "rfm=var", "rat=cr" );
 else
    cgm->fp = fopen ( file , "w+b", "rfm=var", "ctx=stm" );

#else

 if ( mode == 2 )
    cgm->fp = fopen ( file , "w"  );
 else
    cgm->fp = fopen ( file , "w+b" );

#endif

 if ( cgm->fp  == NULL )
    {
     free ( cgm );
     return NULL;
    }

 cgm->mode = mode;
 cgm->f = cgmf[mode];

 cgm->vdc_type = 0;
 cgm->int_prec = 1;
 cgm->real_prec = 2;
 cgm->ix_prec = 1;
 cgm->cd_prec = 0;
 cgm->cix_prec = 0;
 cgm->max_cix = 63;

 cgm->clrsm = 0;
 cgm->lnwsm = 1;
 cgm->mkssm = 1;
 cgm->edwsm = 1;
 cgm->vdc_int = 1;
 cgm->vdc_real = 2;

 cgm->vdc_size = 2;
 cgm->int_size = 2;
 cgm->real_size = 4;
 cgm->ix_size = 3;
 cgm->cd_size = 3;
 cgm->cix_size = 1;
 cgm->clr_size = 1;
 cgm->lnw_size = 4;
 cgm->mks_size = 4;
 cgm->edw_size = 4;

 cgm->cl = 1;

 cgm->op = -1;

 cgm->f->wch  ( cgm, 0, 1, strlen ( header ) + 1 );

 cgm->f->s    ( cgm, header );

 cgm->f->term ( cgm );

 _cgm_ireal_precs[0][3] = FLT_DIG;
 _cgm_ireal_precs[1][3] = DBL_DIG;

 _cgm_rreal_precs[0][0] = - FLT_MAX;
 _cgm_rreal_precs[0][1] =   FLT_MAX;
 _cgm_rreal_precs[1][0] = - DBL_MAX;
 _cgm_rreal_precs[1][1] =   DBL_MAX;

 return cgm;
}

int cgm_end_metafile ( CGM *cgm )
{
 cgm->f->wch  ( cgm, 0, 2, 0 );
 cgm->f->term ( cgm );

 fclose ( cgm->fp );
 cgm->fp = NULL;
 free ( cgm );

 return 0;
}

int cgm_begin_picture (CGM *cgm, const char *s )
{
 cgm->f->wch ( cgm, 0, 3, strlen(s)+1 );
 cgm->f->s   ( cgm, s );
 return cgm->f->term(cgm);
}

int cgm_begin_picture_body ( CGM *cgm )
{
 cgm->f->wch ( cgm, 0, 4, 0 );
 return cgm->f->term(cgm);
}

int cgm_end_picture ( CGM *cgm )
{
 cgm->f->wch ( cgm, 0, 5, 0 );
 return cgm->f->term(cgm);
}

/*******************************
* Metafile Descriptor Elements *
*******************************/

int cgm_metafile_version ( CGM *cgm, long version )
{
 cgm->f->wch ( cgm, 1, 1, cgm->int_size );

 cgm->f->i   ( cgm, version );

 return cgm->f->term(cgm);
}

int cgm_metafile_description ( CGM *cgm, const char *s )
{
 cgm->f->wch ( cgm, 1, 2, 1+strlen(s) );
 cgm->f->s   ( cgm, s );
 return cgm->f->term(cgm);
}

int cgm_vdc_type ( CGM *cgm, int mode )
{
 static const char *vdct[] = { "integer", "real" };
 cgm->f->wch ( cgm, 1, 3, 2 );
 cgm->f->e   ( cgm, mode, vdct );

 cgm->vdc_type = mode;
 if ( cgm->vdc_type == 0 ) /* integer */
    cgm->vdc_size = cgm->vdc_int + 1;
 else
    cgm->vdc_size = ( _cgm_ireal_precs[cgm->vdc_real][1] +
                      _cgm_ireal_precs[cgm->vdc_real][2]) / 8;

 return cgm->f->term(cgm);
}

int cgm_integer_precision ( CGM *cgm, int prec )
{
 cgm->f->wch ( cgm, 1, 4, cgm->int_size );

 switch ( cgm->mode )
    {
    case 0:     /* character */
        break;

    case 1:     /* binary */
        cgm->f->i ( cgm, (long)(prec) );
        break;

    case 2:     /* clear text */
        cgm->f->i ( cgm, _cgm_int_precs[prec/8 - 1][0] );
        cgm->f->sep ( cgm, "," );
        cgm->f->i ( cgm, _cgm_int_precs[prec/8 - 1][1] );
        break;
    }

 cgm->int_prec = prec/8-1;
 cgm->int_size = prec/8;

 return cgm->f->term(cgm);
}

int cgm_real_precision ( CGM *cgm, int mode )
{
 cgm->f->wch ( cgm, 1, 5, 2 + 2*cgm->int_size );
 switch ( cgm->mode )
    {
    case 0:     /* character */
        break;

    case 1:     /* binary */
        cgm->f->e ( cgm,        _cgm_ireal_precs[mode][0] , NULL );
        cgm->f->i ( cgm, (long)(_cgm_ireal_precs[mode][1]) );
        cgm->f->i ( cgm, (long)(_cgm_ireal_precs[mode][2]) );
        break;

    case 2:     /* clear text */
        cgm->f->r   ( cgm,        _cgm_rreal_precs[mode][0] );
        cgm->f->sep ( cgm, "," );
        cgm->f->r   ( cgm,        _cgm_rreal_precs[mode][1] );
        cgm->f->sep ( cgm, "," );
        cgm->f->i   ( cgm, (long)(_cgm_ireal_precs[mode][3]) );
        break;
    }

 cgm->real_prec = mode;
 cgm->real_size = ( _cgm_ireal_precs[mode][1] + _cgm_ireal_precs[mode][2]) / 8;

 /* absolute scaling modes */
 if ( cgm->lnwsm == 1 ) cgm->lnw_size = cgm->real_size;
 if ( cgm->mkssm == 1 ) cgm->mks_size = cgm->real_size;
 if ( cgm->edwsm == 1 ) cgm->edw_size = cgm->real_size;

 return cgm->f->term(cgm);
}

int cgm_index_precision ( CGM *cgm, int prec )
{
 cgm->f->wch ( cgm, 1, 6, cgm->int_size );
 switch ( cgm->mode )
    {
    case 0:     /* character */
        break;

    case 1:     /* binary */
        cgm->f->i ( cgm, (long)(prec) );
        break;

    case 2:     /* clear text */
        cgm->f->i ( cgm, _cgm_int_precs[prec/8 - 1][0] );
        cgm->f->sep ( cgm, "," );
        cgm->f->i ( cgm, _cgm_int_precs[prec/8 - 1][1] );
        break;
    }

 cgm->ix_prec = prec/8-1;
 cgm->ix_size = prec/8;
 return cgm->f->term(cgm);
}

int cgm_colour_precision ( CGM *cgm, int prec )
{
 cgm->f->wch ( cgm, 1, 7, cgm->int_size );
 switch ( cgm->mode )
    {
    case 0:     /* character */
        break;

    case 1:     /* binary */
        cgm->f->i ( cgm, (long)(prec) );
        break;

    case 2:     /* clear text */
        cgm->f->i ( cgm, 1ul+ 2ul * (unsigned long)_cgm_int_precs[prec/8 - 1][1] );
        break;
    }

 cgm->cd_prec = prec/8-1;
 cgm->cd_size = 3*(prec/8);

 if ( cgm->clrsm == 1 ) /* direct */
    cgm->clr_size = cgm->cd_size;

 return cgm->f->term(cgm);
}

int cgm_colour_index_precision ( CGM *cgm, int prec )
{
 cgm->f->wch ( cgm, 1, 8, cgm->int_size );

 switch ( cgm->mode )
    {
    case 0:     /* character */
        break;

    case 1:     /* binary */
        cgm->f->i ( cgm, (long)(prec) );
        break;

    case 2:     /* clear text */
        cgm->f->i ( cgm, 1ul+ 2ul * (unsigned long)_cgm_int_precs[prec/8 - 1][1] );
        break;
    }

 cgm->cix_prec = prec/8-1;
 cgm->cix_size = prec/8;

 if ( cgm->clrsm == 0 ) /* indexed */
    cgm->clr_size = cgm->cix_size;

 return cgm->f->term(cgm);
}

int cgm_maximum_colour_index ( CGM *cgm, unsigned long ci )
{
 cgm->f->wch ( cgm, 1, 9, cgm->cix_size );
 cgm->f->ci  ( cgm, ci );
 return cgm->f->term(cgm);
}

int cgm_colour_value_extent ( CGM *cgm, const double *black,
                                               const double *white)
{
 cgm->f->wch ( cgm, 1, 10, 2 * cgm->cd_size );
 cgm->f->rgb   ( cgm, black[0], black[1], black[2] );
 cgm->f->nl    ( cgm );
 cgm->f->align ( cgm, 15 );
 cgm->f->rgb   ( cgm, white[0], white[1], white[2] );
 return cgm->f->term(cgm);
}

int cgm_metafile_element_list ( CGM *cgm, int n, const int *group, const int *element )
{
 register int i;
 cgm->f->wch ( cgm, 1, 11, 31 );
 cgm->f->sep ( cgm, "\x22" ); /* aspas */
 if ( cgm->mode == 1 ) cgm->f->i ( cgm, n );
 for ( i=0; i<n; i++ )
    {
    if ( cgm->mode == 1 ) /* binario */
       {
       cgm->f->ix ( cgm, group[i] );
       cgm->f->ix ( cgm, element[i] );
       cgm->f->term( cgm );
       }
    else
       {
       cgm->f->wch ( cgm, group[i], element[i], 0 );
       cgm->f->sep ( cgm, "" );
       }
    }
 cgm->f->sep ( cgm, "\x22" ); /* aspas */
 return cgm->f->term(cgm);
}

int cgm_begin_metafile_defaults ( CGM *cgm )
{
 cgm->f->wch ( cgm, 1, 12, 31 );

 /* modo binario - deixa aberto */
 if ( cgm->mode  == 1 ) /* binario */
    return 0;

 return cgm->f->term(cgm);
}

int cgm_end_metafile_defaults ( CGM *cgm )
{
 /* modo binario - ja estava aberto */
 if ( cgm->mode  != 1 ) /* binario */
      cgm->f->wch ( cgm, 1, 0, 0 );

 return cgm->f->term(cgm);
}

int cgm_font_list ( CGM *cgm, const char *fl[] )
{
 register int i;

 cgm->f->wch ( cgm, 1, 13, 31 );

 for ( i=0; fl[i] != NULL; i++ )
    {
    cgm->f->nl ( cgm );
    cgm->f->align ( cgm, 10 );
    cgm->f->s ( cgm, fl[i] );
    }

 return cgm->f->term(cgm);
}

/******************************
* Picture Descriptor Elements *
******************************/

int cgm_scaling_mode ( CGM *cgm, int mode, float metric )
{
 static const char *sm[] = { "abstract", "metric" };
 cgm->f->wch ( cgm, 2, 1, 2 + 4 );
 cgm->f->e   ( cgm, mode, sm );
 if ( cgm->mode == 1 )
  cgmb_putfl32 ( cgm, metric );
 else
  cgm->f->r   ( cgm, metric );
 return cgm->f->term(cgm);
}

int cgm_colour_selection_mode ( CGM *cgm, int mode)
{
 static const char *csm[] = { "indexed", "direct" };
 cgm->f->wch ( cgm, 2, 2, 2 );
 cgm->f->e   ( cgm, mode, csm );

 cgm->clrsm = mode;
 if ( mode == 0 ) /* indexed */
    cgm->clr_size = cgm->cix_size;
 else
    cgm->clr_size = cgm->cd_size;

 return cgm->f->term(cgm);
}

static int _cgm_width_specify_mode ( CGM *cgm, int t, int mode)
{
 static const char *sm[] = { "abstract", "scaled" };
 cgm->f->wch ( cgm, 2, t, 2 );
 cgm->f->e   ( cgm, mode, sm );
 return cgm->f->term(cgm);
}

int cgm_line_width_specify_mode ( CGM *cgm, int mode)
{
 cgm->lnwsm = mode;
 if ( mode == 0 )
  cgm->lnw_size = cgm->vdc_size;
 else
  cgm->lnw_size = cgm->real_size;
 return _cgm_width_specify_mode ( cgm, 3, mode );
}

int cgm_marker_size_specify_mode ( CGM *cgm, int mode)
{
 cgm->mkssm = mode;
 if ( mode == 0 )
  cgm->mks_size = cgm->vdc_size;
 else
  cgm->mks_size = cgm->real_size;
 return _cgm_width_specify_mode ( cgm, 4, mode );
}

int cgm_edge_width_specify_mode ( CGM *cgm, int mode)
{
 cgm->edwsm = mode;
 if ( mode == 0 )
  cgm->edw_size = cgm->vdc_size;
 else
  cgm->edw_size = cgm->real_size;
 return _cgm_width_specify_mode ( cgm, 5, mode );
}

int cgm_vdc_extent ( CGM *cgm, double xmin, double ymin,
                               double xmax, double ymax )
{
 cgm->f->wch ( cgm, 2, 6, 2*2*cgm->vdc_size );
 cgm->f->vdc ( cgm, xmin );
 cgm->f->vdc ( cgm, ymin );
 cgm->f->vdc ( cgm, xmax );
 cgm->f->vdc ( cgm, ymax );
 return cgm->f->term(cgm);
}

int cgm_backgound_colour ( CGM *cgm, const double *cr )
{
 cgm->f->wch ( cgm, 2, 7, cgm->cd_size );
 cgm->f->rgb ( cgm , cr[0], cr[1], cr[2] );
 return cgm->f->term(cgm);
}

/*******************
* Control Elements *
*******************/

int cgm_vdc_integer_precision ( CGM *cgm, int prec )
{
 cgm->f->wch ( cgm, 3, 1, cgm->int_size );
 switch ( cgm->mode )
    {
    case 0:     /* character */
        break;

    case 1:     /* binary */
        cgm->f->i ( cgm, (long)(prec) );
        break;

    case 2:     /* clear text */
        cgm->f->i ( cgm, _cgm_int_precs[prec/8 - 1][0] );
        cgm->f->sep ( cgm, "," );
        cgm->f->i ( cgm, _cgm_int_precs[prec/8 - 1][1] );
        break;
    }

 if ( cgm->vdc_type == 0 )
    cgm->vdc_size = prec/8;

 cgm->vdc_int = prec/8 - 1;

 if ( cgm->lnwsm == 0 && cgm->vdc_type == 0 ) cgm->lnw_size = cgm->vdc_size;
 if ( cgm->mkssm == 0 && cgm->vdc_type == 0 ) cgm->mks_size = cgm->vdc_size;
 if ( cgm->edwsm == 0 && cgm->vdc_type == 0 ) cgm->edw_size = cgm->vdc_size;

 return cgm->f->term(cgm);
}

int cgm_vdc_real_precision ( CGM *cgm, int mode )
{
 cgm->f->wch ( cgm, 3, 2, 2 + 2*cgm->int_size );
 switch ( cgm->mode )
    {
    case 0:     /* character */
        break;

    case 1:     /* binary */
        cgm->f->e ( cgm,        _cgm_ireal_precs[mode][0] , NULL );
        cgm->f->i ( cgm, (long)(_cgm_ireal_precs[mode][1]) );
        cgm->f->i ( cgm, (long)(_cgm_ireal_precs[mode][2]) );
        break;

    case 2:     /* clear text */
        cgm->f->r   ( cgm,        _cgm_rreal_precs[mode][0] );
        cgm->f->sep ( cgm, "," );
        cgm->f->r   ( cgm,        _cgm_rreal_precs[mode][1] );
        cgm->f->sep ( cgm, "," );
        cgm->f->i   ( cgm, (long)(_cgm_ireal_precs[mode][3]) );
        break;
    }

 if ( cgm->vdc_type == 1 )
    cgm->vdc_size = ( _cgm_ireal_precs[mode][1] + _cgm_ireal_precs[mode][2]) / 8;

 cgm->vdc_real = mode;

 if ( cgm->lnwsm == 0 && cgm->vdc_type == 1 ) cgm->lnw_size = cgm->vdc_size;
 if ( cgm->mkssm == 0 && cgm->vdc_type == 1 ) cgm->mks_size = cgm->vdc_size;
 if ( cgm->edwsm == 0 && cgm->vdc_type == 1 ) cgm->edw_size = cgm->vdc_size;

 return cgm->f->term(cgm);
}

int cgm_auxiliary_colour ( CGM *cgm, const void *c )
{
 cgm->f->wch ( cgm, 3, 3, cgm->clr_size );

 cgm->f->co  ( cgm, c ) ;

 return cgm->f->term(cgm);
}

int cgm_transparency ( CGM *cgm, int mode )
{
 cgm->f->wch ( cgm, 3, 4, 2 );

 cgm->f->e   ( cgm, mode, offon );

 return cgm->f->term(cgm);
}

int cgm_clip_rectangle ( CGM *cgm, double xmin, double ymin,
                                   double xmax, double ymax )
{
 cgm->f->wch ( cgm, 3, 5, 4 * cgm->vdc_size );

 cgm->f->vdc ( cgm, xmin );
 cgm->f->vdc ( cgm, ymin );
 cgm->f->vdc ( cgm, xmax );
 cgm->f->vdc ( cgm, ymax );

 return cgm->f->term(cgm);
}

int cgm_clip_indicator ( CGM *cgm, int mode )
{
 cgm->f->wch ( cgm, 3, 6, 2 );

 cgm->f->e   ( cgm, mode, offon );

 return cgm->f->term(cgm);
}

/*******************************
* Graphical Primitive Elements *
*******************************/

static int _cgm_point ( CGM *cgm, double x, double y )
{
 cgm->f->p   ( cgm, x, y );
 return 0;
}

static int _cgm_point_list ( CGM *cgm, int element, int n, const float *p)
{
 register int i;
 cgm->f->wch ( cgm, 4, element, 2*n*cgm->vdc_size );

 for ( i=0; i < 2*n; i+=2 )
    {
    cgm->f->nl    ( cgm );
    cgm->f->align ( cgm, 8 );
    _cgm_point    ( cgm, p[i], p[i+1] );
    }
 return cgm->f->term(cgm);
}

int cgm_polyline ( CGM *cgm, int n, const float *p )
{
 return _cgm_point_list ( cgm, 1, n, p );
}

int cgm_polymarker ( CGM *cgm, int n, const float *p )
{
 return _cgm_point_list ( cgm, 3, n, p );
}

static int _cgm_text_piece ( CGM *cgm, int t, const char *s)
{
 static const char *tt[] = { "NOT_FINAL", "   FINAL" };
 cgm->f->e ( cgm, t, tt );
 cgm->f->s ( cgm, s );
 return cgm->f->term(cgm);
}

int cgm_text ( CGM *cgm, int tt, double x, double y, const char *s )
{
 cgm->f->wch   ( cgm, 4, 4, 2 * cgm->vdc_size + strlen(s) + 3 );
 cgm->f->p     ( cgm, x, y );
 cgm->f->nl    ( cgm );
 cgm->f->align ( cgm, 10 );

 if ( cgm->mode == 2 ) /* clear text */
    {
    while ( strlen (s) > 50 )
       {
       char s1[51];

       strncpy ( s1, s, 50 );
       s1[50] = 0;

       _cgm_text_piece ( cgm, 0, s1 );

       s += 50;
       cgm->f->wch ( cgm, 4, 6, 2 * cgm->vdc_size + strlen(s) + 1 );
       }

    }

 return _cgm_text_piece ( cgm, tt, s );
}

int cgm_polygon ( CGM *cgm, int n, const float *p )
{
 return _cgm_point_list ( cgm, 7, n, p );
}

static int _cgm_cell_rows ( CGM *cgm, long sx, long sy, int prec, const void *c )
{
 register long i,j, brk;

 cgm->f->nl ( cgm );
 cgm->f->sep ( cgm, "(" );

 if ( cgm->clrsm ) 
  brk = 5;
 else
  brk = 12;

 for ( i=0; i < sy; i++ )
  {
   if ( i )
      {
      cgm->f->nl ( cgm );
      cgm->f->align ( cgm, 3 );
      }

   for ( j=0; j < sx; j++)
    {
     if ( j && ( j % brk == 0 ) )
        {
        cgm->f->nl ( cgm );
        cgm->f->align ( cgm, 3 );
        }

     cgm->f->co ( cgm, c );
     c = (void*)((char*)c+ (cgm->clrsm ? (3*sizeof(double)) : sizeof(int)));

     if ( i<sy-1 || j<sx-1 ) cgm->f->sep ( cgm, "," );
    }
  }

 cgm->f->sep ( cgm, ")" );

 return 0;
}

int cgm_cell_array ( CGM *cgm, const float *p, long sx, long sy, int prec, const void *c )
{
 register int i;
 static const char *repmode[] = { "run lenght", "packed" };

 cgm->f->wch ( cgm, 4, 9, 31 );

 for ( i=0; i<3*2; i+=2 )
    _cgm_point ( cgm, p[i], p[i+1] );

 cgm->f->nl (cgm );

 cgm->f->i ( cgm, sx );
 cgm->f->i ( cgm, sy );

 if ( prec == 0 )
    cgm->f->i ( cgm, (long)(prec) );
 else
    {
    switch ( cgm->mode )
       {
       case 0:     /* character */
           break;

       case 1:     /* binary */
           cgm->f->i ( cgm, (long)(prec) );
           break;

       case 2:     /* clear text */
           cgm->f->i ( cgm, 2 * ( _cgm_int_precs[prec/8 - 1][1] + 1) );
           break;
       }
    }

 if ( cgm->mode==1 ) cgm->f->e ( cgm, 1, repmode );

 _cgm_cell_rows( cgm, sx, sy, prec, c );

 return cgm->f->term(cgm);
}

int cgm_rectangle ( CGM *cgm, const float *p )
{
 return _cgm_point_list ( cgm, 11, 2, p );
}

static int _cgm_ellipse_CDP ( CGM *cgm, const float *c, const float *p1,
                              const float *p2 )
{
 _cgm_point ( cgm, c[0], c[1] );
 _cgm_point ( cgm, p1[0], p1[1] );
 _cgm_point ( cgm, p2[0], p2[1] );

 return 0;
}

static int _cgm_ellipse_vectors ( CGM *cgm, double dxs, double dys, double dxe,
                                  double dye )
{
 cgm->f->vdc ( cgm, dxs );
 cgm->f->vdc ( cgm, dys );
 cgm->f->vdc ( cgm, dxe );
 cgm->f->vdc ( cgm, dye );

 return 0;
}

int cgm_elliptical_arc ( CGM *cgm, const float *c, const float *p1,
                         const float *p2, double dxs, double dys, double dxe,
                         double dye )
{
 cgm->f->wch ( cgm, 4, 18, 10*cgm->vdc_size );

 _cgm_ellipse_CDP ( cgm, c, p1, p2 );

 _cgm_ellipse_vectors ( cgm, dxs, dys, dxe, dye );

 return cgm->f->term(cgm);
}

int cgm_elliptical_arc_close ( CGM *cgm, const float *c, const float *p1,
                               const float *p2, double dxs, double dys,
                               double dxe, double dye, int type )
{
 static const char *ct[] = { "pie", "chord" };

 cgm->f->wch ( cgm, 4, 19, 10*cgm->vdc_size + 2 );

 _cgm_ellipse_CDP ( cgm, c, p1, p2 );

 _cgm_ellipse_vectors ( cgm, dxs, dys, dxe, dye );

 cgm->f->e ( cgm, type, ct );

 return cgm->f->term(cgm);
}

/*********************
* Attribute Elements *
*********************/

int cgm_line_bundle_index( CGM *cgm, long li)
{
 cgm->f->wch ( cgm, 5, 1, cgm->ix_size );
 cgm->f->ix  ( cgm, li );
 return cgm->f->term(cgm);
}

int cgm_line_type( CGM *cgm, long lt)
{
 cgm->f->wch ( cgm, 5, 2, cgm->ix_size );
 cgm->f->ix  ( cgm, lt );
 return cgm->f->term(cgm);
}

int cgm_line_width( CGM *cgm, double lw)
{
 cgm->f->wch ( cgm, 5, 3, cgm->lnw_size );

 if ( cgm->lnwsm == 0 ) /* absolute */
      cgm->f->vdc ( cgm, lw );
 else
      cgm->f->r   ( cgm, lw );

 return cgm->f->term(cgm);
}

int cgm_line_colour( CGM *cgm, const void *lc)
{
 cgm->f->wch ( cgm, 5, 4, cgm->clr_size );
 cgm->f->co  ( cgm, lc );
 return cgm->f->term(cgm);
}

int cgm_marker_bundle_index( CGM *cgm, long mi)
{
 cgm->f->wch ( cgm, 5, 5, cgm->ix_size );
 cgm->f->ix  ( cgm, mi );
 return cgm->f->term(cgm);
}

int cgm_marker_type( CGM *cgm, long mt)
{
 cgm->f->wch ( cgm, 5, 6, cgm->ix_size );
 cgm->f->ix  ( cgm, mt );
 return cgm->f->term(cgm);
}

int cgm_marker_size( CGM *cgm, double ms)
{
 cgm->f->wch ( cgm, 5, 7, cgm->mks_size );

 if ( cgm->mkssm == 0 ) /* absolute */
      cgm->f->vdc ( cgm, ms );
 else
      cgm->f->r   ( cgm, ms );

 return cgm->f->term(cgm);
}

int cgm_marker_colour( CGM *cgm, const void *mc)
{
 cgm->f->wch ( cgm, 5, 8, cgm->clr_size );
 cgm->f->co  ( cgm, mc );
 return cgm->f->term(cgm);
}

int cgm_text_bundle_index( CGM *cgm, long ti)
{
 cgm->f->wch ( cgm, 5, 9, cgm->ix_size );
 cgm->f->ix  ( cgm, ti );
 return cgm->f->term(cgm);
}

int cgm_text_font_index( CGM *cgm, long fi)
{
 cgm->f->wch ( cgm, 5, 10, cgm->ix_size );
 cgm->f->ix  ( cgm, fi );
 return cgm->f->term(cgm);
}

int cgm_text_precision( CGM *cgm, int tp)
{
 static const char *txprec[] = { "STRING", "CHAR", "STROKE" };
 cgm->f->wch ( cgm, 5, 11, 2 );
 cgm->f->e   ( cgm, tp, txprec );
 return cgm->f->term(cgm);
}

int cgm_char_expansion_factor ( CGM *cgm, double expan )
{
 cgm->f->wch ( cgm, 5, 12, cgm->real_size );
 cgm->f->r   ( cgm, expan );
 return cgm->f->term(cgm);
}

int cgm_char_spacing ( CGM *cgm, double spacing )
{
 cgm->f->wch ( cgm, 5, 13, cgm->real_size );
 cgm->f->r   ( cgm, spacing );
 return cgm->f->term(cgm);
}

int cgm_text_colour( CGM *cgm, const void *tc)
{
 cgm->f->wch ( cgm, 5, 14, cgm->clr_size );
 cgm->f->co  ( cgm, tc );
 return cgm->f->term(cgm);
}

int cgm_char_height ( CGM *cgm, double height )
{
 cgm->f->wch ( cgm, 5, 15, cgm->vdc_size );
 cgm->f->vdc ( cgm, height );
 return cgm->f->term(cgm);
}

int cgm_char_orientation ( CGM *cgm, double chupx, double chupy,
                                     double chbsx, double chbsy )
{
 cgm->f->wch ( cgm, 5, 16, 4*cgm->vdc_size );
 cgm->f->sep ( cgm, "% char up   %" );
 cgm->f->vdc ( cgm, chupx );
 cgm->f->sep ( cgm, "," );
 cgm->f->vdc ( cgm, chupy );
 cgm->f->nl  ( cgm );
 cgm->f->align ( cgm, 8 );
 cgm->f->sep ( cgm, "% char base %" );
 cgm->f->vdc ( cgm, chbsx );
 cgm->f->sep ( cgm, "," );
 cgm->f->vdc ( cgm, chbsy );
 return cgm->f->term(cgm);
}

int cgm_text_path ( CGM *cgm, int tp)
{
 static const char *txpath[] = { "RIGHT", "LEFT", "UP", "DOWN" };
 cgm->f->wch ( cgm, 5, 17, 2 );
 cgm->f->e   ( cgm, tp, txpath );
 return cgm->f->term(cgm);
}

int cgm_text_alignment ( CGM *cgm, int hor, int ver , double ch, double cv)
{
 static const char *txhor[] = { "NORMHORIZ", "LEFT", "CTR", "RIGHT", "CONTHORIZ" };
 static const char *txver[] = { "NORMVERT", "TOP", "CAP", "HALF", "BASE",
                            "BOTTOM", "CONTHORIZ" };

 cgm->f->wch ( cgm, 5, 18, 2*2 + 2*cgm->real_size );
 cgm->f->e   ( cgm, hor, txhor );
 cgm->f->e   ( cgm, ver, txver );
 cgm->f->r   ( cgm, ch );
 cgm->f->r   ( cgm, cv );
 return cgm->f->term(cgm);
}

int cgm_fill_bundle_index( CGM *cgm, long fi)
{
 cgm->f->wch ( cgm, 5, 21, cgm->ix_size );
 cgm->f->ix  ( cgm, fi );
 return cgm->f->term(cgm);
}

int cgm_interior_style( CGM *cgm, int is)
{
 static const char *style[]= { "HOLLOW", "SOLID", "PAT", "HATCH", "EMPTY" };
 cgm->f->wch ( cgm, 5, 22, 2 );
 cgm->f->e   ( cgm, is, style );
 return cgm->f->term(cgm);
}

int cgm_fill_colour( CGM *cgm, const void *fc)
{
 cgm->f->wch ( cgm, 5, 23, cgm->clr_size );
 cgm->f->co  ( cgm, fc );
 return cgm->f->term(cgm);
}

int cgm_hatch_index( CGM *cgm, long hi)
{
 cgm->f->wch ( cgm, 5, 24, cgm->ix_size );
 cgm->f->ix  ( cgm, hi );
 return cgm->f->term(cgm);
}

int cgm_pattern_index( CGM *cgm, long pi)
{
 cgm->f->wch ( cgm, 5, 25, cgm->ix_size );
 cgm->f->ix  ( cgm, pi );
 return cgm->f->term(cgm);
}

int cgm_edge_width( CGM *cgm, double ew)
{
 cgm->f->wch ( cgm, 5, 28, cgm->edw_size );

 if ( cgm->lnwsm == 0 ) /* absolute */
      cgm->f->vdc ( cgm, ew );
 else
      cgm->f->r   ( cgm, ew );

 return cgm->f->term(cgm);
}

int cgm_edge_colour( CGM *cgm, const void *ec)
{
 cgm->f->wch ( cgm, 5, 29, cgm->clr_size );
 cgm->f->co  ( cgm, ec );
 return cgm->f->term(cgm);
}

int cgm_edge_visibility ( CGM *cgm, int mode )
{
 cgm->f->wch ( cgm, 5, 30, 2 );
 cgm->f->e   ( cgm, mode, offon );
 return cgm->f->term(cgm);
}

int cgm_fill_reference_point ( CGM *cgm, double rpx, double rpy )
{
 cgm->f->wch ( cgm, 5, 31, 2*cgm->vdc_size );
 _cgm_point  ( cgm, rpx, rpy );
 return cgm->f->term(cgm);
}

int cgm_pattern_table ( CGM *cgm, long pi, long sx, long sy, int prec, const void *c)
{
 cgm->f->wch ( cgm, 5, 32, 31 );
 cgm->f->ix  ( cgm, pi );

 cgm->f->i ( cgm, sx );
 cgm->f->i ( cgm, sy );

 if ( prec == 0 )
    cgm->f->i ( cgm, (long)(prec) );
 else
    {
    switch ( cgm->mode )
       {
       case 0:     /* character */
           break;

       case 1:     /* binary */
           cgm->f->i ( cgm, (long)(prec) );
           break;

       case 2:     /* clear text */
           cgm->f->i ( cgm, 2 * ( _cgm_int_precs[prec/8 - 1][1] + 1) );
           break;
       }
    }

 _cgm_cell_rows( cgm, sx, sy, prec, c );

 return cgm->f->term(cgm);
}

int cgm_pattern_size ( CGM *cgm, double hx, double hy, double wx, double wy )
{
 cgm->f->wch ( cgm, 5, 33, 4*cgm->vdc_size );
 cgm->f->sep ( cgm, "% height %" );
 cgm->f->vdc ( cgm, hx );
 cgm->f->sep ( cgm, "," );
 cgm->f->vdc ( cgm, hy );
 cgm->f->nl  ( cgm );
 cgm->f->align ( cgm, 8 );
 cgm->f->sep ( cgm, "% width  %" );
 cgm->f->vdc ( cgm, wx );
 cgm->f->sep ( cgm, "," );
 cgm->f->vdc ( cgm, wy );
 return cgm->f->term(cgm);
}

int cgm_colour_table ( CGM *cgm, long ci, long n, const double *cb )
{
 register long i=n;

 if ( n > 31 ) n = 31;

 cgm->f->wch ( cgm, 5, 34, cgm->int_size+n*cgm->cd_size );
 cgm->f->i   ( cgm, ci );

 n = i;

 for ( i=0; i<n; i++ )
    {
    if ( i )
       {
       cgm->f->nl ( cgm );
       cgm->f->align ( cgm, 18 );
       }
    cgm->f->rgb ( cgm, cb[(int)i], cb[(int)i+1], cb[(int)i+2] );
    }

 return cgm->f->term(cgm);
}

static void _cgm_asf_pair ( CGM *cgm, int asft, int asfv )
{
 static const char *asfvl[] = { "INDIV", "BUNDLED" };
 static const char *asftl[] = {
        "LINE_TYPE",       "LINE_WIDTH",  "LINE_COLR",
        "MARKER_TYPE",     "MARKER_SIZE", "MARKER_COLR",
        "TEXT_FONT_INDEX", "TEXT_PREC",   "CHAR_EXP",    "CHAR_SPACE", "TEXT_COLR",
        "INT_STYLE",       "FILL_COLR",   "HATCH_INDEX", "PAT_INDEX",
        "EDGE_TYPE",       "EDGE_WIDTH",  "EDGE_COLR",
        "ALL", "ALL_LINE", "ALL_MARKER",  "ALL_TEXT", "ALL_FILL", "ALL_EDGE"
        };

 cgm->f->nl    ( cgm );
 cgm->f->align ( cgm, 4 );
 cgm->f->e     ( cgm, asft, asftl );
 cgm->f->e     ( cgm, asfv, asfvl );
}

int cgm_asfs ( CGM *cgm, int n, const int *asfts, const int* asfvs )
{
 register int i;
 cgm->f->wch ( cgm, 5, 35, 2*n* 2 );

 for ( i=0; i<n; i++ )
   _cgm_asf_pair ( cgm, asfts[i], asfvs[i] );

 return cgm->f->term(cgm);
}

/*****************
* Escape Element *
*****************/

/********************
* External elements *
********************/

int cgm_message ( CGM *cgm, int action, const char *s)
{
 static const char *ac[] = { "NOACTION", "ACTION" };
 cgm->f->wch ( cgm, 7, 2, 2 + strlen(s)+1 );
 cgm->f->e   ( cgm, action, ac );
 cgm->f->s   ( cgm, s );
 return cgm->f->term(cgm);
}
