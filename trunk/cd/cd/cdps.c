char *cdps_c = "$Id: cdps.c,v 1.11 2005/12/07 13:03:57 scuri Exp $";

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>

#include "cd.h"
#include "cdprivat.h"
#include "wd.h"
#include "cdps.h"


#define mm2pt(x) (CD_MM2PT*(x))

#ifndef min
#define min(a, b) ((a)<(b)?(a):(b))
#endif
#ifndef max
#define max(a, b) ((a)>(b)?(a):(b))
#endif

#define SWAPI(a,b) {int tempi = a; a = b; b = tempi;}
#define SWAPD(a,b) {double tempd = a; a = b; b = tempd;}

/*
** dada uma cor do CD, obtem uma de suas componentes, na faixa 0-1.
*/
#define get_red(_)   (((double)cdRed(_))/255)
#define get_green(_) (((double)cdGreen(_))/255)
#define get_blue(_)  (((double)cdBlue(_))/255)

/* ATENTION: currentmatrix/setmatrix
   Remeber that there is a tranformation set just after file open, to define margins and pixel scale.
   So use transformations carefully.
*/
   

#define RotateL(x)  ((x) = ((unsigned)(x) << 1) | ((unsigned)(x) >> 7))

static unsigned char HatchBits[6][8] = {            /* [style][y] (8x8) */
     {0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0x00}, /* CD_HORIZONTAL */
     {0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10}, /* CD_VERTICAL   */
     {0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80}, /* CD_BDIAGONAL  */
     {0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01}, /* CD_FDIAGONAL  */
     {0x10, 0x10, 0x10, 0xFF, 0x10, 0x10, 0x10, 0x10}, /* CD_CROSS      */
     {0x81, 0x42, 0x24, 0x18, 0x18, 0x24, 0x42, 0x81}};/* CD_DIAGCROSS  */


typedef struct cdCanvasPS 
{
  FILE *psFile;          /* Arquivo PS */
  int res;               /* Resolucao */
  int pages;             /* Numero total de paginas */
  double width;          /* Largura do papel (points) */
  double height;         /* Altura do papel (points) */
  double xmin, ymin;     /* Definem as margens esquerda e inferior (points) */
  double xmax, ymax;     /* Definem as margens direita e superior (points) */
  double bbxmin, bbymin; /* Definem a bounding box */
  double bbxmax, bbymax; /* Definem a bounding box */
  double bbmargin;       /* Define a margem para a bounding box */
  double scale;          /* Fator de conversao de coordenadas (pixel2points) */
  int eps;               /* Postscrip encapsulado? */
  int level1;            /* if true generates level 1 only function calls */
  int landscape;         /* page orientation */
  int last_fill;         /* indicates the last fill state */
  long last_hfg, last_hbg; /* last foreground and background when hatch and stipple was last updated */
  long last_sfg, last_sbg;
  int debug;             /* print debug strings in the file */
  float  rotate_angle;
  int    rotate_center_x,
         rotate_center_y;

  cdPrivateCanvas* cnv;

  int font[CD_HELVETICA+1][CD_BOLD_ITALIC+1]; /* Registra as fontes usadas */
  char *native_font[100];
  int num_native_font;

  int poly_holes[500];
  int holes;

} cdCanvasPS;

static cdCanvasPS *active_ps = NULL;

/*
%F Ajusta o tamanho do papel em points.
*/
static void setpspapersize(cdCanvasPS *cnv, int size)
{
  static struct
  {
    int width;
    int height;
  } paper[] =
    {
      { 2393, 3391 },   /*   A0   */
      { 1689, 2393 },   /*   A1   */
      { 1192, 1689 },   /*   A2   */
      {  842, 1192 },   /*   A3   */
      {  595,  842 },   /*   A4   */
      {  420,  595 },   /*   A5   */
      {  612,  792 },   /* LETTER */
      {  612, 1008 }    /*  LEGAL */
    };

  if (size<CD_A0 || size>CD_LEGAL) 
    return;

  cnv->width = (double)paper[size].width;
  cnv->height = (double)paper[size].height;
}

/*
%F Registra os valores default para impressao.
*/
static void setpsdefaultvalues(cdCanvasPS *cnv)
{
  /* all the other values are set to 0 */
  setpspapersize(cnv, CD_A4);
  cnv->xmin = 25.4; /* ainda em mm, sera' convertido para points na init_ps */
  cnv->xmax = 25.4;
  cnv->ymin = 25.4;
  cnv->ymax = 25.4;
  cnv->res = 300;
  cnv->last_fill = -1;
}

/*
%F Insere o ponto (x, y) na BoundingBox corrente.
Nao leva em consideracao a espessura das linhas.
*/
static void insertpoint(int x, int y)
{
  double xmin = x*active_ps->scale + active_ps->xmin - active_ps->bbmargin;
  double ymin = y*active_ps->scale + active_ps->ymin - active_ps->bbmargin;

  double xmax = x*active_ps->scale + active_ps->xmin + active_ps->bbmargin;
  double ymax = y*active_ps->scale + active_ps->ymin + active_ps->bbmargin;

  if (!active_ps->bbxmin && !active_ps->bbxmax && !active_ps->bbymin && !active_ps->bbymax)
  {
    active_ps->bbxmin = xmin;
    active_ps->bbymin = ymin;

    active_ps->bbxmax = xmax;
    active_ps->bbymax = ymax;
  }
  else
  {
    if (active_ps->cnv->clip_mode == CD_CLIPAREA)
    {
      active_ps->bbxmin = max(active_ps->cnv->clip_xmin*active_ps->scale + active_ps->xmin, min(active_ps->bbxmin, xmin));
      active_ps->bbymin = max(active_ps->cnv->clip_ymin*active_ps->scale + active_ps->ymin, min(active_ps->bbymin, ymin));

      active_ps->bbxmax = min(active_ps->cnv->clip_xmax*active_ps->scale + active_ps->xmax, max(active_ps->bbxmax, xmax));
      active_ps->bbymax = min(active_ps->cnv->clip_ymax*active_ps->scale + active_ps->ymax, max(active_ps->bbymax, ymax));
    }
    else
    {
      active_ps->bbxmin = max(active_ps->xmin, min(active_ps->bbxmin, xmin));
      active_ps->bbymin = max(active_ps->ymin, min(active_ps->bbymin, ymin));

      active_ps->bbxmax = min(active_ps->xmax, max(active_ps->bbxmax, xmax));
      active_ps->bbymax = min(active_ps->ymax, max(active_ps->bbymax, ymax));
    }
  }
}

/*
%F Ajusta a BoundingBox para conter o ponto (x,y).
Leva em consideracao a espessura das linhas.
*/
static void bbox(int x, int y)
{
  if (active_ps->cnv->line_width > 1)
  {
    insertpoint(x-active_ps->cnv->line_width, y-active_ps->cnv->line_width);
    insertpoint(x+active_ps->cnv->line_width, y+active_ps->cnv->line_width);
  }
  else 
    insertpoint(x, y);
}

static void wbbox(double wx, double wy)
{
  int x, y;
  wdWorld2Canvas(wx, wy, &x, &y);
  if (active_ps->cnv->line_width > 1)
  {
    insertpoint(x-active_ps->cnv->line_width, y-active_ps->cnv->line_width);
    insertpoint(x+active_ps->cnv->line_width, y+active_ps->cnv->line_width);
  }
  else 
    insertpoint(x, y);
}

/*
%F Checa se os valores de tipo e estilo estao corretos e \
os ajusta para acesso direto as estruturas que formam o nome.
*/
static char *findfont(int type_face, int style)
{
  static char font[50];

  static char *family[] = 
  {
    "Courier",      /* CD_FIXED */
    "Courier",      /* CD_COURIER */
    "Times",        /* CD_TIMES_ROMAN */
    "Helvetica"     /* CD_HELVETICA */
  };

  static char *type[] = 
  {
    "",              /* CD_PLAIN */
    "-Bold",         /* CD_BOLD */
    "-Oblique",      /* CD_ITALIC */
    "-BoldOblique",  /* CD_BOLD_ITALIC */
    
    "-Roman",        /* Plain p/ Times */
    "-Bold",         /* Bold p/ Times */
    "-Italic",       /* Italic p/ Times */
    "-BoldItalic"    /* BoldItalic p/ Times */
  };

  if (type_face == CD_TIMES_ROMAN) 
	  style += CD_BOLD_ITALIC+1;

  sprintf(font, "%s%s", family[type_face], type[style]);

  return font;
}

static char new_codes[] = {"\
/newcodes	% foreign character encodings\n\
[\n\
160/space 161/exclamdown 162/cent 163/sterling 164/currency\n\
165/yen 166/brokenbar 167/section  168/dieresis 169/copyright\n\
170/ordfeminine 171/guillemotleft 172/logicalnot 173/hyphen 174/registered\n\
175/macron 176/degree 177/plusminus 178/twosuperior 179/threesuperior\n\
180/acute 181/mu 182/paragraph  183/periodcentered 184/cedilla\n\
185/onesuperior 186/ordmasculine 187/guillemotright 188/onequarter\n\
189/onehalf 190/threequarters 191/questiondown 192/Agrave 193/Aacute\n\
194/Acircumflex 195/Atilde 196/Adieresis 197/Aring 198/AE 199/Ccedilla\n\
200/Egrave 201/Eacute 202/Ecircumflex 203/Edieresis 204/Igrave  205/Iacute\n\
206/Icircumflex 207/Idieresis 208/Eth 209/Ntilde 210/Ograve 211/Oacute\n\
212/Ocircumflex 213/Otilde  214/Odieresis 215/multiply 216/Oslash\n\
217/Ugrave 218/Uacute 219/Ucircumflex 220/Udieresis 221/Yacute 222/Thorn\n\
223/germandbls 224/agrave 225/aacute 226/acircumflex 227/atilde\n\
228/adieresis 229/aring 230/ae 231/ccedilla  232/egrave 233/eacute\n\
234/ecircumflex 235/edieresis 236/igrave 237/iacute 238/icircumflex\n\
239/idieresis 240/eth 241/ntilde 242/ograve 243/oacute 244/ocircumflex\n\
245/otilde 246/odieresis 247/divide 248/oslash 249/ugrave  250/uacute\n\
251/ucircumflex 252/udieresis 253/yacute 254/thorn 255/ydieresis\n\
] def\n\
"};

static char change_font[] = {"\
% change fonts using ISO Latin1 characters\n\
/ChgFnt		% size psname natname  =>  font\n\
{\n\
    dup FontDirectory exch known	% is re-encoded name known?\n\
    { exch pop }			% yes, get rid of long name\n\
    { dup 3 1 roll ReEncode } ifelse	% no, re-encode it\n\
    findfont exch scalefont setfont\n\
} def\n\
"};

static char re_encode[] = {"\
/ReEncode	%\n\
{\n\
  12 dict begin\n\
	/newname exch def\n\
	/basename exch def\n\
	/basedict basename findfont def\n\
	/newfont basedict maxlength dict def\n\
	basedict\n\
	{ exch dup /FID ne\n\
	    { dup /Encoding eq\n\
		    { exch dup length array copy newfont 3 1 roll put }\n\
		    { exch newfont 3 1 roll put } ifelse\n\
	    }\n\
	    { pop pop } ifelse\n\
	} forall\n\
	newfont /FontName newname put\n\
	newcodes aload pop newcodes length 2 idiv\n\
	{ newfont /Encoding get 3 1 roll put } repeat\n\
	newname newfont definefont pop\n\
    end\n\
} def\n\
"};

static void setcliprect(cdCanvasPS* ps, double xmin, double ymin, double xmax, double ymax)
{
  fprintf(ps->psFile, "initclip\n");

  /* cliping geral para a margem */
  if (ps->level1)
  {
    fprintf(ps->psFile, "N\n");
    fprintf(ps->psFile, "%g %g M\n", xmin, ymin);
    fprintf(ps->psFile, "%g %g L\n", xmin, ymax);
    fprintf(ps->psFile, "%g %g L\n", xmax, ymax);
    fprintf(ps->psFile, "%g %g L\n", xmax, ymin);
    fprintf(ps->psFile, "C\n");
    fprintf(ps->psFile, "clip\n");
    fprintf(ps->psFile, "N\n");
  }
  else
    fprintf(ps->psFile, "%g %g %g %g rectclip\n", xmin, ymin, xmax-xmin, ymax-ymin);
}

/*
%F Inicializa o arquivo PS.
*/
static void init_ps(cdCanvasPS *new_ps)
{
  int type_face, style;
  double w, h;

  time_t now = time(NULL);

  new_ps->scale = 72.0/new_ps->res;
  new_ps->xmin = mm2pt(new_ps->xmin);
  new_ps->xmax = new_ps->width - mm2pt(new_ps->xmax);
  new_ps->ymin = mm2pt(new_ps->ymin);
  new_ps->ymax = new_ps->height - mm2pt(new_ps->ymax);
  new_ps->bbmargin = mm2pt(new_ps->bbmargin);

  fprintf(new_ps->psFile, "%%!PS-Adobe-3.0 %s\n", new_ps->eps ? "EPSF-3.0":"");
  fprintf(new_ps->psFile, "%%%%Title: CanvasDraw\n");
  fprintf(new_ps->psFile, "%%%%Creator: CanvasDraw\n");
  fprintf(new_ps->psFile, "%%%%CreationDate: %s", asctime(localtime(&now)));
  fprintf(new_ps->psFile, "%%%%DocumentFonts: (atend)\n"); /* attend means at the end of the file, */
  fprintf(new_ps->psFile, "%%%%Pages: (atend)\n");         /* see killcanvas */ 
  fprintf(new_ps->psFile, "%%%%PageOrder: Ascend\n");         
  fprintf(new_ps->psFile, "%%%%LanguageLevel: %d\n", new_ps->level1 ? 1: 2);
  fprintf(new_ps->psFile, "%%%%Orientation: %s\n", new_ps->landscape ? "Landscape": "Portrait");

  if (new_ps->eps)
  {
    fprintf(new_ps->psFile, "%%%%BoundingBox: (atend)\n");
    new_ps->bbxmin = new_ps->bbxmax = new_ps->bbymin = new_ps->bbymax = 0;
    /* BoundingBox==Empty */
  }

  fprintf(new_ps->psFile, "%%%%EndComments\n");
  fprintf(new_ps->psFile, "%%%%BeginProlog\n");
  
  fprintf(new_ps->psFile, "/N {newpath} bind def\n");
  fprintf(new_ps->psFile, "/C {closepath} bind def\n");
  fprintf(new_ps->psFile, "/M {moveto} bind def\n");
  fprintf(new_ps->psFile, "/L {lineto} bind def\n");
  fprintf(new_ps->psFile, "/B {curveto} bind def\n");
  fprintf(new_ps->psFile, "/S {stroke} bind def\n");
  fprintf(new_ps->psFile, "/LL {moveto lineto stroke} bind def\n");
  
  if (!new_ps->level1)
  {
    fprintf(new_ps->psFile, "/RF {rectfill} bind def\n");
    fprintf(new_ps->psFile, "/RS {rectstroke} bind def\n");
  }

  fprintf(new_ps->psFile, "%%%%EndProlog\n");
  fprintf(new_ps->psFile, "%%%%BeginSetup\n");
  
  if (!new_ps->eps && !new_ps->level1)
  {
    fprintf(new_ps->psFile, "%%%%IncludeFeature: *Resolution %d\n", new_ps->res);
    fprintf(new_ps->psFile, "%%%%BeginFeature: *PageSize\n");
    fprintf(new_ps->psFile, "<< /PageSize [%g %g] >> setpagedevice\n", new_ps->width, new_ps->height);
    fprintf(new_ps->psFile, "%%%%EndFeature\n");
  }

  fprintf(new_ps->psFile, "%%%%EndSetup\n");

  for (type_face=CD_SYSTEM; type_face<=CD_HELVETICA; type_face++)
    for (style=CD_PLAIN; style<=CD_BOLD_ITALIC; style++)
      new_ps->font[type_face][style] = 0;

  fputs(new_codes, new_ps->psFile);
  fputs(change_font, new_ps->psFile);
  fputs(re_encode, new_ps->psFile);

  w = new_ps->xmax - new_ps->xmin;
  h = new_ps->ymax - new_ps->ymin;

  new_ps->cnv->w = (int)(w/new_ps->scale + 0.5);   /* Converte p/ unidades do usuario */
  new_ps->cnv->h = (int)(h/new_ps->scale + 0.5); /* Converte p/ unidades do usuario */
  new_ps->cnv->w_mm = w/CD_MM2PT;   /* Converte p/ milimetros */
  new_ps->cnv->h_mm = h/CD_MM2PT; /* Converte p/ milimetros */
  new_ps->cnv->bpp = 24;
  new_ps->cnv->xres = new_ps->cnv->w / new_ps->cnv->w_mm;
  new_ps->cnv->yres = new_ps->cnv->h / new_ps->cnv->h_mm;

  fprintf(new_ps->psFile, "%%%%Page: 1 1\n");
  new_ps->pages = 1;

  if (new_ps->debug) fprintf(new_ps->psFile, "\n%%cdCreateCanvas: Margin Begin\n");

  /* shift das coordenadas por causa da margem */
  fprintf(new_ps->psFile, "%g %g translate\n", new_ps->xmin, new_ps->ymin);

  /* escala para coordenadas serem em pixels */
  fprintf(new_ps->psFile, "%g %g scale\n", new_ps->scale, new_ps->scale);

  /* cliping geral para a margem */
  setcliprect(new_ps, 0, 0, new_ps->cnv->w, new_ps->cnv->h);

  if (new_ps->debug) fprintf(new_ps->psFile, "%%cdCreateCanvas: MarginEnd\n");
}


static void cdkillcanvas(void *canvas)
{
  cdCanvasPS *cnv = (cdCanvasPS *)canvas;
  int type_face, style, i;

  if (cnv->debug) fprintf(cnv->psFile, "\n%%cdKillCanvas\n");

  fprintf(cnv->psFile, "showpage\n");
  fprintf(cnv->psFile, "%%%%Trailer\n");
  fprintf(cnv->psFile, "%%%%Pages: %d 1\n", cnv->pages);

  if (cnv->eps)
  {
    int xmin = (int)cnv->bbxmin;
    int xmax = (int)cnv->bbxmax;
    int ymin = (int)cnv->bbymin;
    int ymax = (int)cnv->bbymax;
    if (xmax < cnv->bbxmax) xmax++;
    if (ymax < cnv->bbymax) ymax++;
    fprintf(cnv->psFile,"%%%%BoundingBox: %5d %5d %5d %5d\n",xmin,ymin,xmax,ymax);
  }

  fprintf(cnv->psFile, "%%%%DocumentFonts:");

  for (type_face=CD_SYSTEM; type_face<=CD_HELVETICA; type_face++)
  {
    for (style=CD_PLAIN; style<=CD_BOLD_ITALIC; style++)
    {
      if (cnv->font[type_face][style])
      {
        char *font = findfont(type_face, style);
        fprintf(cnv->psFile, " %s", font);
      }
    }
  }

  for (i = 0; i < cnv->num_native_font; i++)
  {
    fprintf(cnv->psFile, " %s", cnv->native_font[i]);
    free(cnv->native_font[i]);
  }

  putc('\n', cnv->psFile);
  fprintf(cnv->psFile,"%%%%EOF");

  fclose(cnv->psFile);

  free(cnv);
}

static int cdlinestyle(int style);
static int cdlinewidth(int style);
static int cdhatch(int style);
static void cdstipple(int n, int m, unsigned char *stipple);

static int cdactivate(void *cnv)
{
  active_ps = (cdCanvasPS *)cnv;
  return CD_OK;
}

static void cddeactivate(void)
{
  fflush(active_ps->psFile);
  active_ps = NULL;
}

static void wdcall(int usage)
{
  if (active_ps->debug) fprintf(active_ps->psFile, "\n%%wdCall %d Begin\n", usage);

  fprintf(active_ps->psFile, "[0 0 0 0 0 0] defaultmatrix\n");  /* reset to default */
  fprintf(active_ps->psFile, "setmatrix\n");  

  /* margin and scale */
  fprintf(active_ps->psFile, "%g %g translate\n", active_ps->xmin, active_ps->ymin);
  fprintf(active_ps->psFile, "%g %g scale\n", active_ps->scale, active_ps->scale);

  if (usage)
  {
    /* WC transformation */
    fprintf(active_ps->psFile, "%g %g translate\n", active_ps->cnv->tx, active_ps->cnv->ty);
    fprintf(active_ps->psFile, "%g %g scale\n", active_ps->cnv->sx, active_ps->cnv->sy);

    if (active_ps->rotate_angle)
    {
      double wx, wy;
      wdCanvas2World((int)active_ps->rotate_center_x, (int)active_ps->rotate_center_y, &wx, &wy);
      fprintf(active_ps->psFile, "%g %g translate\n", wx, wy);
      fprintf(active_ps->psFile, "%g rotate\n", (double)active_ps->rotate_angle);
      fprintf(active_ps->psFile, "%g %g translate\n", -wx, -wy);
    }
  }
  else
  {
    if (active_ps->rotate_angle)
    {
      /* rotation = translate to point + rotation + translate back */
      fprintf(active_ps->psFile, "%d %d translate\n", active_ps->rotate_center_x, active_ps->rotate_center_y);
      fprintf(active_ps->psFile, "%g rotate\n", (double)active_ps->rotate_angle);
      fprintf(active_ps->psFile, "%d %d translate\n", -active_ps->rotate_center_x, -active_ps->rotate_center_y);
    }
  }

  cdlinewidth(active_ps->cnv->line_width);
  cdlinestyle(active_ps->cnv->line_style);

  if (active_ps->debug) fprintf(active_ps->psFile, "%%wdCall %dEnd\n", usage);
}

static void update_fill(int fill)
{
  if (fill == 0)
  {
    /* called before a NON filled primitive */

    if (active_ps->last_fill != CD_SOLID)
    {
      if (active_ps->debug) fprintf(active_ps->psFile, "\n%%cdPsUpdateFill %d Begin\n", fill);

      fprintf(active_ps->psFile, "%g %g %g setrgbcolor\n", get_red(active_ps->cnv->foreground), 
                                                           get_green(active_ps->cnv->foreground), 
                                                           get_blue(active_ps->cnv->foreground));
      active_ps->last_fill = CD_SOLID;

      if (active_ps->debug) fprintf(active_ps->psFile, "%%cdPsUpdateFill %dEnd\n", fill);
    }
  }
  else
  {
    /* called before a filled primitive */

    if (active_ps->last_fill != active_ps->cnv->interior_style)
    {
      if (active_ps->debug) fprintf(active_ps->psFile, "\n%%cdPsUpdateFill %d Begin\n", fill);

      if (active_ps->cnv->interior_style == CD_SOLID)
      {
        fprintf(active_ps->psFile, "%g %g %g setrgbcolor\n", get_red(active_ps->cnv->foreground), 
                                                             get_green(active_ps->cnv->foreground), 
                                                             get_blue(active_ps->cnv->foreground));
      }
      else if (!active_ps->level1)
      {
        if (active_ps->cnv->interior_style == CD_HATCH && (active_ps->last_hfg != active_ps->cnv->foreground || 
                                                           active_ps->last_hbg != active_ps->cnv->background))
        {
          cdhatch(active_ps->cnv->hatch_style);
        }

        if (active_ps->cnv->interior_style == CD_STIPPLE && (active_ps->last_sfg != active_ps->cnv->foreground || 
                                                             active_ps->last_sbg != active_ps->cnv->background))
        {
          cdstipple(active_ps->cnv->stipple_w, active_ps->cnv->stipple_h, active_ps->cnv->stipple);
        }

        fprintf(active_ps->psFile, "cd_pattern\n");
        fprintf(active_ps->psFile, "setpattern\n");
      }

      active_ps->last_fill = active_ps->cnv->interior_style;

      if (active_ps->debug) fprintf(active_ps->psFile, "%%cdPsUpdateFill %dEnd\n", fill);
    }
  }
}

/*
%F Comeca uma nova pagina.
*/
static void cdflush(void)
{
  if (active_ps->debug) fprintf(active_ps->psFile, "\n%%cdFlush Begin\n");

  if (active_ps->eps)
    fprintf(stderr, "%%CanvasDraw: flush not allowed in EPS mode... ignoring\n");
  else
  {
    fprintf(active_ps->psFile, "gsave\n");

    fprintf(active_ps->psFile, "showpage\n");
    active_ps->pages++;
    fprintf(active_ps->psFile, "%%%%Page: %d %d\n", active_ps->pages, active_ps->pages);

    fprintf(active_ps->psFile, "grestore\n");
  }

  if (active_ps->debug) fprintf(active_ps->psFile, "%%cdFlushEnd\n");
}


/******************************************************/
/* coordinate transformation                          */
/******************************************************/

static void cdcliparea(int* xmin, int* xmax, int* ymin, int* ymax)
{
  if (active_ps->cnv->clip_mode != CD_CLIPAREA)
    return;

  if (active_ps->debug) fprintf(active_ps->psFile, "\n%%cdClipArea Begin\n");

  setcliprect(active_ps, (double)*xmin, (double)*ymin, (double)*xmax, (double)*ymax);

  if (active_ps->debug) fprintf(active_ps->psFile, "%%cdClipAreaEnd\n");
}

static int cdclip(int mode)
{
  if (active_ps->debug) fprintf(active_ps->psFile, "\n%%cdClip %d Begin\n", mode);

  if (mode == CD_CLIPAREA)
  {
    active_ps->cnv->clip_mode = CD_CLIPAREA;

    setcliprect(active_ps, (double)active_ps->cnv->clip_xmin, 
                           (double)active_ps->cnv->clip_ymin, 
                           (double)active_ps->cnv->clip_xmax, 
                           (double)active_ps->cnv->clip_ymax);
  }
  else if (mode == CD_CLIPPOLYGON)
  {
    fprintf(active_ps->psFile, "clip_polygon\n");
  }
  else
  {
    /* margin clipping only */
    setcliprect(active_ps, 0, 0, active_ps->cnv->w, active_ps->cnv->h);
  }

  if (active_ps->debug) fprintf(active_ps->psFile, "%%cdClip %dEnd\n", mode);

  return mode;
}

/******************************************************/
/* primitives                                         */
/******************************************************/

static void cdline(int x1, int y1, int x2, int y2)
{
  update_fill(0);

  fprintf(active_ps->psFile, "N %d %d %d %d LL\n", x1, y1, x2, y2);

  if (active_ps->eps)
  {
    bbox(x1, y1);
    bbox(x2, y2);
  }
}

static void wdline(double x1, double y1, double x2, double y2)
{
  update_fill(0);

  fprintf(active_ps->psFile, "N %g %g %g %g LL\n", x1, y1, x2, y2);

  if (active_ps->eps)
  {
    wbbox(x1, y1);
    wbbox(x2, y2);
  }
}

static void cdrect(int xmin, int xmax, int ymin, int ymax)
{
  update_fill(0);

  if (active_ps->level1)
  {
    fprintf(active_ps->psFile, "N\n");
    fprintf(active_ps->psFile, "%d %d M\n", xmin, ymin);
    fprintf(active_ps->psFile, "%d %d L\n", xmin, ymax);
    fprintf(active_ps->psFile, "%d %d L\n", xmax, ymax);
    fprintf(active_ps->psFile, "%d %d L\n", xmax, ymin);
    fprintf(active_ps->psFile, "C S\n");
  }
  else
    fprintf(active_ps->psFile, "%d %d %d %d RS\n", xmin, ymin, xmax - xmin, ymax - ymin);

  if (active_ps->eps)
  {
    bbox(xmin, ymin);
    bbox(xmax, ymax);
  }
}

static void wdrect(double xmin, double xmax, double ymin, double ymax)
{
  update_fill(0);

  if (active_ps->level1)
  {
    fprintf(active_ps->psFile, "N\n");
    fprintf(active_ps->psFile, "%g %g M\n", xmin, ymin);
    fprintf(active_ps->psFile, "%g %g L\n", xmin, ymax);
    fprintf(active_ps->psFile, "%g %g L\n", xmax, ymax);
    fprintf(active_ps->psFile, "%g %g L\n", xmax, ymin);
    fprintf(active_ps->psFile, "C S\n");
  }
  else
    fprintf(active_ps->psFile, "%g %g %g %g RS\n", xmin, ymin, xmax - xmin, ymax - ymin);

  if (active_ps->eps)
  {
    wbbox(xmin, ymin);
    wbbox(xmax, ymax);
  }
}

static void cdbox(int xmin, int xmax, int ymin, int ymax)
{
  update_fill(1);

  if (active_ps->level1)
  {
    fprintf(active_ps->psFile, "N\n");
    fprintf(active_ps->psFile, "%d %d M\n", xmin, ymin);
    fprintf(active_ps->psFile, "%d %d L\n", xmin, ymax);
    fprintf(active_ps->psFile, "%d %d L\n", xmax, ymax);
    fprintf(active_ps->psFile, "%d %d L\n", xmax, ymin);
    fprintf(active_ps->psFile, "C fill\n");
  }
  else
    fprintf(active_ps->psFile, "%d %d %d %d RF\n", xmin, ymin, xmax - xmin, ymax - ymin);

  if (active_ps->eps)
  {
    bbox(xmin, ymin);
    bbox(xmax, ymax);
  }
}

static void wdbox(double xmin, double xmax, double ymin, double ymax)
{
  update_fill(1);

  if (active_ps->level1)
  {
    fprintf(active_ps->psFile, "N\n");
    fprintf(active_ps->psFile, "%g %g M\n", xmin, ymin);
    fprintf(active_ps->psFile, "%g %g L\n", xmin, ymax);
    fprintf(active_ps->psFile, "%g %g L\n", xmax, ymax);
    fprintf(active_ps->psFile, "%g %g L\n", xmax, ymin);
    fprintf(active_ps->psFile, "C fill\n");
  }
  else
    fprintf(active_ps->psFile, "%g %g %g %g RF\n", xmin, ymin, xmax - xmin, ymax - ymin);

  if (active_ps->eps)
  {
    wbbox(xmin, ymin);     
    wbbox(xmax, ymax);
  }
}

static void cdarc(int xc, int yc, int w, int h, double a1, double a2)
{
  update_fill(0);

  if (w==h) /* Circulo: PS implementa direto */
  {
    fprintf(active_ps->psFile, "N %d %d %g %g %g arc S\n", xc, yc, 0.5*w, a1, a2);
  }
  else /* Elipse: mudar a escala p/ criar a partir do circulo */
  {
    if (active_ps->debug) fprintf(active_ps->psFile, "\n%%cdArc Ellipse Begin\n");

    fprintf(active_ps->psFile, "[0 0 0 0 0 0] currentmatrix\n"); /* fill new matrix from CTM */
    fprintf(active_ps->psFile, "%d %d translate\n", xc, yc);
    fprintf(active_ps->psFile, "1 %g scale\n", ((double)h)/w);
    fprintf(active_ps->psFile, "N\n");
    fprintf(active_ps->psFile, "0 0 %g %g %g arc\n", 0.5*w, a1, a2);
    fprintf(active_ps->psFile, "S\n");
    fprintf(active_ps->psFile, "setmatrix\n"); /* back to CTM */

    if (active_ps->debug) fprintf(active_ps->psFile, "%%cdArc EllipseEnd\n");
  }

  if (active_ps->eps)
  {
    bbox((int)(xc+w/2*cos(a1/CD_RAD2DEG)), (int)(yc+h/2*sin(a1/CD_RAD2DEG)));
    bbox((int)(xc+w/2*cos(a2/CD_RAD2DEG)), (int)(yc+h/2*sin(a2/CD_RAD2DEG)));
    if (a1 > a2)
      bbox(xc+w/2, yc);
    if ((a1<90 && 90<a2) || (a1>a2 && a2>90) || (a2<a1 && a1<90))
      bbox(xc, yc+h/2);
    if ((a1<180 && 180<a2) || (a1>a2 && a2>180) || (a2<a1 && a1<180))
      bbox(xc-w/2, yc);
    if ((a1<270 && 270<a2) || (a1>a2 && a2>270) || (a2<a1 && a1<270))
      bbox(xc, yc-h/2);
  }
}

static void wdarc(double xc, double yc, double w, double h, double a1, double a2)
{
  update_fill(0);

  if (w==h) /* Circulo: PS implementa direto */
  {
    fprintf(active_ps->psFile, "N %g %g %g %g %g arc S\n", xc, yc, 0.5*w, a1, a2);
  }
  else /* Elipse: mudar a escala p/ criar a partir do circulo */
  {
    if (active_ps->debug) fprintf(active_ps->psFile, "\n%%wdArc Ellipse Begin\n");

    fprintf(active_ps->psFile, "[0 0 0 0 0 0] currentmatrix\n");
    fprintf(active_ps->psFile, "%g %g translate\n", xc, yc);
    fprintf(active_ps->psFile, "1 %g scale\n", h/w);
    fprintf(active_ps->psFile, "N\n");
    fprintf(active_ps->psFile, "0 0 %g %g %g arc\n", 0.5*w, a1, a2);
    fprintf(active_ps->psFile, "S\n");
    fprintf(active_ps->psFile, "setmatrix\n");

    if (active_ps->debug) fprintf(active_ps->psFile, "%%wdArc EllipseEnd\n");
  }

  if (active_ps->eps)
  {
    wbbox(xc+w/2*cos(a1/CD_RAD2DEG), yc+h/2*sin(a1/CD_RAD2DEG));
    wbbox(xc+w/2*cos(a2/CD_RAD2DEG), yc+h/2*sin(a2/CD_RAD2DEG));
    if (a1 > a2)
      wbbox(xc+w/2, yc);
    if ((a1<90 && 90<a2) || (a1>a2 && a2>90) || (a2<a1 && a1<90))
      wbbox(xc, yc+h/2);
    if ((a1<180 && 180<a2) || (a1>a2 && a2>180) || (a2<a1 && a1<180))
      wbbox(xc-w/2, yc);
    if ((a1<270 && 270<a2) || (a1>a2 && a2>270) || (a2<a1 && a1<270))
      wbbox(xc, yc-h/2);
  }
}

static void cdsector(int xc, int yc, int w, int h, double a1, double a2)
{
  update_fill(1);

  if (w==h) /* Circulo: PS implementa direto */
  {
    if (active_ps->debug) fprintf(active_ps->psFile, "\n%%cdSector Circle Begin\n");

    fprintf(active_ps->psFile, "N\n");
    fprintf(active_ps->psFile, "%d %d M\n", xc, yc);
    fprintf(active_ps->psFile, "%d %d %g %g %g arc\n", xc, yc, 0.5*w, a1, a2);
    fprintf(active_ps->psFile, "C fill\n");

    if (active_ps->debug) fprintf(active_ps->psFile, "%%cdSector CircleEnd\n");
  }
  else /* Elipse: mudar a escala p/ criar a partir do circulo */
  {
    if (active_ps->debug) fprintf(active_ps->psFile, "\n%%cdSector Ellipse Begin\n");

    fprintf(active_ps->psFile, "[0 0 0 0 0 0] currentmatrix\n");
    fprintf(active_ps->psFile, "%d %d translate\n", xc, yc);
    fprintf(active_ps->psFile, "1 %g scale\n", ((double)h)/w);
    fprintf(active_ps->psFile, "N\n");
    fprintf(active_ps->psFile, "0 0 M\n");
    fprintf(active_ps->psFile, "0 0 %g %g %g arc\n", 0.5*w, a1, a2);
    fprintf(active_ps->psFile, "C fill\n");
    fprintf(active_ps->psFile, "setmatrix\n");

    if (active_ps->debug) fprintf(active_ps->psFile, "%%cdSector EllipseEnd\n");
  }

  if (active_ps->eps)
  {
    bbox((int)(xc+w/2*cos(a1/CD_RAD2DEG)), (int)(yc+h/2*sin(a1/CD_RAD2DEG)));
    bbox((int)(xc+w/2*cos(a2/CD_RAD2DEG)), (int)(yc+h/2*sin(a2/CD_RAD2DEG)));
    if (a1 > a2)
      bbox(xc+w/2, yc);
    if ((a1<90 && 90<a2) || (a1>a2 && a2>90) || (a2<a1 && a1<90))
      bbox(xc, yc+h/2);
    if ((a1<180 && 180<a2) || (a1>a2 && a2>180) || (a2<a1 && a1<180))
      bbox(xc-w/2, yc);
    if ((a1<270 && 270<a2) || (a1>a2 && a2>270) || (a2<a1 && a1<270))
      bbox(xc, yc-h/2);
  }
}

static void wdsector(double xc, double yc, double w, double h, double a1, double a2)
{
  update_fill(1);

  if (w==h) /* Circulo: PS implementa direto */
  {
    if (active_ps->debug) fprintf(active_ps->psFile, "\n%%wdSector Circle Begin\n");

    fprintf(active_ps->psFile, "N\n");
    fprintf(active_ps->psFile, "%g %g M\n", xc, yc);
    fprintf(active_ps->psFile, "%g %g %g %g %g arc\n", xc, yc, 0.5*w, a1, a2);
    fprintf(active_ps->psFile, "C fill\n");

    if (active_ps->debug) fprintf(active_ps->psFile, "%%wdSector CircleEnd\n");
  }
  else /* Elipse: mudar a escala p/ criar a partir do circulo */
  {
    if (active_ps->debug) fprintf(active_ps->psFile, "\n%%wdSector Ellipse Begin\n");

    fprintf(active_ps->psFile, "[0 0 0 0 0 0] currentmatrix\n");
    fprintf(active_ps->psFile, "%g %g translate\n", xc, yc);
    fprintf(active_ps->psFile, "1 %g scale\n", h/w);
    fprintf(active_ps->psFile, "N\n");
    fprintf(active_ps->psFile, "0 0 M\n");
    fprintf(active_ps->psFile, "0 0 %g %g %g arc\n", 0.5*w, a1, a2);
    fprintf(active_ps->psFile, "C fill\n");
    fprintf(active_ps->psFile, "setmatrix\n");

    if (active_ps->debug) fprintf(active_ps->psFile, "%%wdSector EllipseEnd\n");
  }

  if (active_ps->eps)
  {
    wbbox(xc+w/2*cos(a1/CD_RAD2DEG), yc+h/2*sin(a1/CD_RAD2DEG));
    wbbox(xc+w/2*cos(a2/CD_RAD2DEG), yc+h/2*sin(a2/CD_RAD2DEG));
    if (a1 > a2)
      wbbox(xc+w/2, yc);
    if ((a1<90 && 90<a2) || (a1>a2 && a2>90) || (a2<a1 && a1<90))
      wbbox(xc, yc+h/2);
    if ((a1<180 && 180<a2) || (a1>a2 && a2>180) || (a2<a1 && a1<180))
      wbbox(xc-w/2, yc);
    if ((a1<270 && 270<a2) || (a1>a2 && a2>270) || (a2<a1 && a1<270))
      wbbox(xc, yc-h/2);
  }
}

static void cdchord(int xc, int yc, int w, int h, double a1, double a2)
{
  update_fill(1);

  if (w==h) /* Circulo: PS implementa direto */
  {
    if (active_ps->debug) fprintf(active_ps->psFile, "\n%%cdChord Circle Begin\n");

    fprintf(active_ps->psFile, "N\n");
    fprintf(active_ps->psFile, "%d %d %g %g %g arc\n", xc, yc, 0.5*w, a1, a2);
    fprintf(active_ps->psFile, "C fill\n");

    if (active_ps->debug) fprintf(active_ps->psFile, "%%cdChord CircleEnd\n");
  }
  else /* Elipse: mudar a escala p/ criar a partir do circulo */
  {
    if (active_ps->debug) fprintf(active_ps->psFile, "\n%%cdChord Ellipse Begin\n");

    fprintf(active_ps->psFile, "[0 0 0 0 0 0] currentmatrix\n");
    fprintf(active_ps->psFile, "%d %d translate\n", xc, yc);
    fprintf(active_ps->psFile, "1 %g scale\n", ((double)h)/w);
    fprintf(active_ps->psFile, "N\n");
    fprintf(active_ps->psFile, "0 0 %g %g %g arc\n", 0.5*w, a1, a2);
    fprintf(active_ps->psFile, "C fill\n");
    fprintf(active_ps->psFile, "setmatrix\n");

    if (active_ps->debug) fprintf(active_ps->psFile, "%%cdChord EllipseEnd\n");
  }

  if (active_ps->eps)
  {
    bbox((int)(xc+w/2*cos(a1/CD_RAD2DEG)), (int)(yc+h/2*sin(a1/CD_RAD2DEG)));
    bbox((int)(xc+w/2*cos(a2/CD_RAD2DEG)), (int)(yc+h/2*sin(a2/CD_RAD2DEG)));
    if (a1 > a2)
      bbox(xc+w/2, yc);
    if ((a1<90 && 90<a2) || (a1>a2 && a2>90) || (a2<a1 && a1<90))
      bbox(xc, yc+h/2);
    if ((a1<180 && 180<a2) || (a1>a2 && a2>180) || (a2<a1 && a1<180))
      bbox(xc-w/2, yc);
    if ((a1<270 && 270<a2) || (a1>a2 && a2>270) || (a2<a1 && a1<270))
      bbox(xc, yc-h/2);
  }
}

static void wdchord(double xc, double yc, double w, double h, double a1, double a2)
{
  update_fill(1);

  if (w==h) /* Circulo: PS implementa direto */
  {
    if (active_ps->debug) fprintf(active_ps->psFile, "\n%%wdChord Circle Begin\n");

    fprintf(active_ps->psFile, "N\n");
    fprintf(active_ps->psFile, "%g %g %g %g %g arc\n", xc, yc, 0.5*w, a1, a2);
    fprintf(active_ps->psFile, "C fill\n");

    if (active_ps->debug) fprintf(active_ps->psFile, "%%wdChord CircleEnd\n");
  }
  else /* Elipse: mudar a escala p/ criar a partir do circulo */
  {
    if (active_ps->debug) fprintf(active_ps->psFile, "\n%%wdChord Ellipse Begin\n");

    fprintf(active_ps->psFile, "[0 0 0 0 0 0] currentmatrix\n");
    fprintf(active_ps->psFile, "%g %g translate\n", xc, yc);
    fprintf(active_ps->psFile, "1 %g scale\n", h/w);
    fprintf(active_ps->psFile, "N\n");
    fprintf(active_ps->psFile, "0 0 %g %g %g arc\n", 0.5*w, a1, a2);
    fprintf(active_ps->psFile, "C fill\n");
    fprintf(active_ps->psFile, "setmatrix\n");

    if (active_ps->debug) fprintf(active_ps->psFile, "%%wdChord EllipseEnd\n");
  }

  if (active_ps->eps)
  {
    wbbox(xc+w/2*cos(a1/CD_RAD2DEG), yc+h/2*sin(a1/CD_RAD2DEG));
    wbbox(xc+w/2*cos(a2/CD_RAD2DEG), yc+h/2*sin(a2/CD_RAD2DEG));
    if (a1 > a2)
      wbbox(xc+w/2, yc);
    if ((a1<90 && 90<a2) || (a1>a2 && a2>90) || (a2<a1 && a1<90))
      wbbox(xc, yc+h/2);
    if ((a1<180 && 180<a2) || (a1>a2 && a2>180) || (a2<a1 && a1<180))
      wbbox(xc-w/2, yc);
    if ((a1<270 && 270<a2) || (a1>a2 && a2>270) || (a2<a1 && a1<270))
      wbbox(xc, yc-h/2);
  }
}

static void cdtext(int x, int y, char *s)
{
  int i, length;
  int px = x, py = y;
  int ascent, height, baseline;
  
  update_fill(0);

  cdFontDim(NULL, &height, &ascent, NULL);
  baseline = height - ascent;

  if (active_ps->debug) fprintf(active_ps->psFile, "\n%%cdText Begin\n");

  fprintf(active_ps->psFile, "N 0 0 M\n");
  putc('(', active_ps->psFile);

  for (length = (int)strlen(s), i=0; i<length; i++)
  {
    if (s[i]=='(' || s[i]==')')
      putc('\\', active_ps->psFile);
    putc(s[i], active_ps->psFile);
  }

  fprintf(active_ps->psFile, ")\n");
  fprintf(active_ps->psFile, "dup true charpath\n");
  fprintf(active_ps->psFile, "flattenpath\n");
  fprintf(active_ps->psFile, "pathbbox\n");    /* bbox na pilha: llx lly urx ury */
  fprintf(active_ps->psFile, "exch\n");        /* troca o topo: llx lly ury urx */
  fprintf(active_ps->psFile, "4 1 roll\n");    /* roda: urx llx lly ury */
  fprintf(active_ps->psFile, "exch\n");        /* troca o topo: urx llx ury lly */
  fprintf(active_ps->psFile, "sub\n");         /* subtrai: urx llx h */
  fprintf(active_ps->psFile, "3 1 roll\n");    /* roda: h urx llx */
  fprintf(active_ps->psFile, "sub\n");         /* subtrai: h w */
  fprintf(active_ps->psFile, "0 0\n");         /* empilha: h w 0 0 */
  fprintf(active_ps->psFile, "4 -1 roll\n");   /* roda: w 0 0 h */

  fprintf(active_ps->psFile, "gsave\n");
  fprintf(active_ps->psFile, "%d %d translate\n", x, y);

  if (active_ps->cnv->text_orientation != 0)
    fprintf(active_ps->psFile, "%g rotate\n", active_ps->cnv->text_orientation);

  switch (active_ps->cnv->text_alignment) /* Operacao em Y, topo da pilha: 0 h */
  {
  case CD_NORTH:
  case CD_NORTH_EAST:
  case CD_NORTH_WEST:
    fprintf(active_ps->psFile, "sub\n");       /* subtrai: w x y-h */
    py -= active_ps->cnv->font_size;
    break;
  case CD_EAST:
  case CD_WEST:
  case CD_CENTER:
    fprintf(active_ps->psFile, "2 div sub\n"); /* subtrai: w x y-h/2 */
    py -= active_ps->cnv->font_size/2;
    break;
  case CD_SOUTH_EAST:
  case CD_SOUTH:
  case CD_SOUTH_WEST:
    fprintf(active_ps->psFile, "pop %d add\n", baseline); /* subtrai: w x y+baseline */
    py += baseline;
    break;
  case CD_BASE_RIGHT:
  case CD_BASE_CENTER:
  case CD_BASE_LEFT:
    fprintf(active_ps->psFile, "pop\n");       /* desempilha: w x y */
    break;
  }

  fprintf(active_ps->psFile, "3 1 roll\n");    /* roda: y' w x */
  fprintf(active_ps->psFile, "exch\n");        /* inverte: y' x w */

  switch (active_ps->cnv->text_alignment) /* Operacao em X, topo da pilha: x w */
  {
  case CD_NORTH:
  case CD_SOUTH:
  case CD_CENTER:
  case CD_BASE_CENTER:
    fprintf(active_ps->psFile, "2 div sub\n");  /* subtrai: y' x-w/2 */
    px -= (active_ps->cnv->font_size/2*(int)strlen(s))/2;
    break;
  case CD_NORTH_EAST:
  case CD_EAST:
  case CD_SOUTH_EAST:
  case CD_BASE_RIGHT:
    fprintf(active_ps->psFile, "sub\n");        /* subtrai: y' x-w */
    px -= active_ps->cnv->font_size/2*(int)strlen(s);
    break;
  case CD_SOUTH_WEST:
  case CD_WEST:
  case CD_NORTH_WEST:
  case CD_BASE_LEFT:
    fprintf(active_ps->psFile, "pop\n");        /* desempilha: y' x */
    break;
  }

  fprintf(active_ps->psFile, "exch\n");         /* inverte: x' y' */
  fprintf(active_ps->psFile, "M\n");            /* moveto */

  fprintf(active_ps->psFile, "show\n");

  if (active_ps->eps)
  {
    bbox(px, py);
    bbox(px+active_ps->cnv->font_size/2*(int)strlen(s), py+active_ps->cnv->font_size);
  }

  fprintf(active_ps->psFile, "grestore\n");

  if (active_ps->debug) fprintf(active_ps->psFile, "%%cdTextEnd\n");
}

static void cdpoly(int mode, cdPoint* poly, int n)
{
  int i;

  if (mode == CD_CLIP)
  {
    if (active_ps->debug) fprintf(active_ps->psFile, "\n%%cdPoly %d Begin\n", mode);

    fprintf(active_ps->psFile, "/clip_polygon {\n");
    fprintf(active_ps->psFile, "initclip\n");
  }
  else
  {
    if (mode == CD_FILL)
      update_fill(1);
    else
      update_fill(0);

    if (active_ps->debug) fprintf(active_ps->psFile, "\n%%cdPoly %d Begin\n", mode);
  }

  fprintf(active_ps->psFile, "N\n");
  fprintf(active_ps->psFile, "%d %d M\n", poly[0].x, poly[0].y);

  if (active_ps->eps) 
    bbox(poly[0].x, poly[0].y);

  if (mode == CD_BEZIER)
  {
    for (i=1; i<n; i+=3)
    {
      fprintf(active_ps->psFile, "%d %d %d %d %d %d B\n", poly[i].x,   poly[i].y, 
                                                          poly[i+1].x, poly[i+1].y, 
                                                          poly[i+2].x, poly[i+2].y);

      if (active_ps->eps) 
      {
        bbox(poly[i].x,   poly[i].y);
        bbox(poly[i+2].x, poly[i+2].y);
        bbox(poly[i+3].x, poly[i+3].y);
      }
    }
  }
  else
  {
    int hole_index = 0;

    for (i=1; i<n; i++)
    {
      if (active_ps->holes && i == active_ps->poly_holes[hole_index])
      {
        fprintf(active_ps->psFile, "%d %d M\n", poly[i].x, poly[i].y);
        hole_index++;
      }
      else
        fprintf(active_ps->psFile, "%d %d L\n", poly[i].x, poly[i].y);

      if (active_ps->eps) 
        bbox(poly[i].x, poly[i].y);
    }
  }

  switch (mode)
  {
  case CD_CLOSED_LINES :
    fprintf(active_ps->psFile, "C S\n");
    break;
  case CD_OPEN_LINES :
    fprintf(active_ps->psFile, "S\n");
    break;
  case CD_BEZIER :
    fprintf(active_ps->psFile, "S\n");
    break;
  case CD_FILL :
    if (active_ps->holes || active_ps->cnv->fill_mode==CD_EVENODD)
      fprintf(active_ps->psFile, "eofill\n");
    else
      fprintf(active_ps->psFile, "fill\n");
    break;
  case CD_CLIP :
    if (active_ps->cnv->fill_mode==CD_EVENODD)
      fprintf(active_ps->psFile, "C eoclip\n");
    else
      fprintf(active_ps->psFile, "C clip\n");
    fprintf(active_ps->psFile, "N\n");
    fprintf(active_ps->psFile, "} bind def\n");
    if (active_ps->cnv->clip_mode == CD_CLIPPOLYGON) 
      fprintf(active_ps->psFile, "clip_polygon\n");
    break;
  }

  if (active_ps->debug) fprintf(active_ps->psFile, "%%cdPoly %dEnd\n", mode);
}

static void wdpoly(int mode, wdPoint* poly, int n)
{
  int i, hole_index = 0;

  if (mode == CD_CLIP)
  {
    if (active_ps->debug) fprintf(active_ps->psFile, "\n%%wdPoly %d Begin\n", mode);

    fprintf(active_ps->psFile, "/clip_polygon {\n");
    fprintf(active_ps->psFile, "initclip\n");
  }
  else
  {
    if (mode == CD_FILL)
      update_fill(1);
    else
      update_fill(0);

    if (active_ps->debug) fprintf(active_ps->psFile, "\n%%wdPoly %d Begin\n", mode);
  }

  fprintf(active_ps->psFile, "N\n");
  fprintf(active_ps->psFile, "%g %g M\n", poly[0].x, poly[0].y);

  if (active_ps->eps) 
    wbbox(poly[0].x, poly[0].y);

  for (i=1; i<n; i++)
  {
    if (active_ps->holes && i == active_ps->poly_holes[hole_index])
    {
      fprintf(active_ps->psFile, "%g %g M\n", poly[i].x, poly[i].y);
      hole_index++;
    }
    else
      fprintf(active_ps->psFile, "%g %g L\n", poly[i].x, poly[i].y);

    if (active_ps->eps) 
      wbbox(poly[i].x, poly[i].y);
  }

  switch (mode)
  {
  case CD_CLOSED_LINES :
    fprintf(active_ps->psFile, "C S\n");
    break;
  case CD_OPEN_LINES :
    fprintf(active_ps->psFile, "S\n");
    break;
  case CD_FILL :
    if (active_ps->holes || active_ps->cnv->fill_mode==CD_EVENODD)
      fprintf(active_ps->psFile, "eofill\n");
    else
      fprintf(active_ps->psFile, "fill\n");
    break;
  case CD_CLIP :
    if (active_ps->cnv->fill_mode==CD_EVENODD)
      fprintf(active_ps->psFile, "C eoclip\n");
    else
      fprintf(active_ps->psFile, "C clip\n");
    fprintf(active_ps->psFile, "N\n");
    fprintf(active_ps->psFile, "} bind def\n");
    if (active_ps->cnv->clip_mode == CD_CLIPPOLYGON) 
      fprintf(active_ps->psFile, "clip_polygon\n");
    break;
  }

  if (active_ps->debug) fprintf(active_ps->psFile, "%%wdPoly %dEnd\n", mode);
}


/******************************************************/
/* attributes                                         */
/******************************************************/

static int cdlinestyle(int style)
{
  double mm = (72.0/25.4) / active_ps->scale;

  if (active_ps->cnv->using_wd)
    mm /=  active_ps->cnv->s;

  if (active_ps->debug) fprintf(active_ps->psFile, "\n%%cdLineStyle %d Begin\n", style);

  fprintf(active_ps->psFile, "[");

  switch (style)
  {
  case CD_CONTINUOUS : /* empty dash */
    fprintf(active_ps->psFile, " ");
    break;
  case CD_DASHED :
    fprintf(active_ps->psFile, "%g %g", 3*mm, mm);
    break;
  case CD_DOTTED :
    fprintf(active_ps->psFile, "%g %g", mm, mm);
    break;
  case CD_DASH_DOT :
    fprintf(active_ps->psFile, "%g %g %g %g", 3*mm, mm, mm, mm);
    break;
  case CD_DASH_DOT_DOT :
    fprintf(active_ps->psFile, "%g %g %g %g %g %g", 3*mm, mm, mm, mm, mm, mm);
    break;
  case CD_CUSTOM :
    {
      int i;
      for (i = 0; i < active_ps->cnv->line_dashes_count; i++)
        fprintf(active_ps->psFile, "%g ", active_ps->cnv->line_dashes[i]*mm);
    }
    break;
  }

  fprintf(active_ps->psFile, "] 0 setdash\n");

  if (active_ps->debug) fprintf(active_ps->psFile, "%%cdLineStyle %dEnd\n", style);

  return style;
}

static int cdlinewidth(int width)
{
  if (active_ps->cnv->using_wd)
    fprintf(active_ps->psFile, "%g setlinewidth\n", ((double)width) / active_ps->cnv->s);
  else
    fprintf(active_ps->psFile, "%d setlinewidth\n", width);

  return width;
}

static int cdlinejoin(int join)
{
  int cd2ps_join[] = {0, 2, 1};
  fprintf(active_ps->psFile, "%d setlinejoin\n", cd2ps_join[join]);
  return join;
}

static int cdlinecap(int cap)
{
  int cd2ps_cap[] =  {0, 2, 1};
  fprintf(active_ps->psFile, "%d setlinecap\n", cd2ps_cap[cap]);
  return cap;
}

static void make_pattern(int n, int m, void* data, void (*data2rgb)(int n, int i, int j, void* data, unsigned char*r, unsigned char*g, unsigned char*b))
{
  int i, j;
  unsigned char r, g, b;

  if (active_ps->debug) fprintf(active_ps->psFile, "\n%%cdPsMakePattern Begin\n");

  fprintf(active_ps->psFile, "/cd_pattern\n");
  fprintf(active_ps->psFile, "currentfile %d string readhexstring\n", n*m*3);

  for (j=0; j<m; j++)
  {
    for (i=0; i<n; i++)
    {
      data2rgb(n, i, j, data, &r, &g, &b);
      fprintf(active_ps->psFile, "%02x%02x%02x", (int)r, (int)g, (int)b);
    }

    fprintf(active_ps->psFile, "\n");
  }

  fprintf(active_ps->psFile, "pop\n");
  fprintf(active_ps->psFile, "/Pat exch def\n");
  fprintf(active_ps->psFile, "<<\n");
  fprintf(active_ps->psFile, "  /PatternType 1\n");
  fprintf(active_ps->psFile, "  /PaintType 1\n");
  fprintf(active_ps->psFile, "  /TilingType 1\n");
  fprintf(active_ps->psFile, "  /BBox [0 0 %d %d]\n", n, m);
  fprintf(active_ps->psFile, "  /XStep %d /YStep %d\n", n, m);
  fprintf(active_ps->psFile, "  /PaintProc {\n");
  fprintf(active_ps->psFile, "              pop\n");
  fprintf(active_ps->psFile, "              %d %d 8\n", n, m);
  fprintf(active_ps->psFile, "              matrix\n");
  fprintf(active_ps->psFile, "              Pat\n");
  fprintf(active_ps->psFile, "              false 3\n");
  fprintf(active_ps->psFile, "              colorimage\n");
  fprintf(active_ps->psFile, "             }\n");
  fprintf(active_ps->psFile, ">>\n");
  fprintf(active_ps->psFile, "matrix\n");
  fprintf(active_ps->psFile, "makepattern\n");
  fprintf(active_ps->psFile, "def\n");

  if (active_ps->debug) fprintf(active_ps->psFile, "%%cdPsMakePatternEnd\n");
}

static void long2rgb(int n, int i, int j, void* data, unsigned char*r, unsigned char*g, unsigned char*b)
{
  long* long_data = (long*)data;
  cdDecodeColor(long_data[j*n+i], r, g, b);
}

static void cdpattern(int n, int m, long int *pattern)
{
  if (active_ps->level1)
    return;

  make_pattern(n, m, pattern, long2rgb);
  active_ps->last_fill = -1;
}

static void uchar2rgb(int n, int i, int j, void* data, unsigned char*r, unsigned char*g, unsigned char*b)
{
  unsigned char* uchar_data = (unsigned char*)data;
  if (uchar_data[j*n+i])
    cdDecodeColor(active_ps->cnv->foreground, r, g, b);
  else
    cdDecodeColor(active_ps->cnv->background, r, g, b);
}

static void cdstipple(int n, int m, unsigned char *stipple)
{
  if (active_ps->level1)
    return;

  active_ps->last_sfg = active_ps->cnv->foreground;
  active_ps->last_sbg = active_ps->cnv->background;

  make_pattern(n, m, stipple, uchar2rgb);
  active_ps->last_fill = -1;
}

static void ucharh2rgb(int n, int i, int j, void* data, unsigned char*r, unsigned char*g, unsigned char*b)
{
  unsigned char* uchar_data = (unsigned char*)data;
  static unsigned char hatch;
  if (i == 0) hatch = uchar_data[j];
  if (hatch & 0x80)
    cdDecodeColor(active_ps->cnv->foreground, r, g, b);
  else
    cdDecodeColor(active_ps->cnv->background, r, g, b);
  RotateL(hatch);
}

static int cdhatch(int style)
{
  if (active_ps->level1)
    return active_ps->cnv->hatch_style;

  active_ps->last_hfg = active_ps->cnv->foreground;
  active_ps->last_hbg = active_ps->cnv->background;

  make_pattern(8, 8, HatchBits[style], ucharh2rgb);
  active_ps->last_fill = -1;

  return style;
}

static void cdfont(int* type_face, int* style, int* size)
{
  char *font = findfont(*type_face, *style);
  int size_pixel;

  if (*size < 0)
  {
    double size_mm;
    size_pixel = -*size;
    cdPixel2MM(size_pixel, 0, &size_mm, NULL);
    *size = (int)(size_mm * CD_MM2PT);
  }
  else
    size_pixel = (int)(*size / active_ps->scale);

  fprintf(active_ps->psFile, "%d /%s /%s-Latin1 ChgFnt\n", size_pixel, font, font);

  active_ps->font[*type_face][*style] = 1;
}

static void cdnativefont(char *font)
{
  char name[256];
  int size;
  sscanf(font, "%s %d", name, &size);
  if (size < 0)
    fprintf(active_ps->psFile, "%d ", (int)(-size));
  else
    fprintf(active_ps->psFile, "%d ", (int)(size / active_ps->scale));
  active_ps->cnv->font_size = size;
  fprintf(active_ps->psFile, "/%s ", name);
  fprintf(active_ps->psFile, "/%s-Latin1 ", name);
  fprintf(active_ps->psFile, "ChgFnt\n");
  active_ps->cnv->font_size = size;
  active_ps->native_font[active_ps->num_native_font] = (char*)malloc(strlen(name)+1);
  memcpy(active_ps->native_font[active_ps->num_native_font], name, strlen(name)+1);
  active_ps->num_native_font++;
}


/******************************************************/
/* color                                              */
/******************************************************/

static long int cdforeground(long int color)
{
  active_ps->last_fill = -1;
  return color;
}


/******************************************************/
/* client images                                      */
/******************************************************/

static void cdputimagerectrgb(int iw, int ih, unsigned char *r, unsigned char *g, unsigned char *b, int x, int y, int w, int h, int xmin, int xmax, int ymin, int ymax)
{
  int i, j, rw, rh;
  rw = xmax-xmin+1;
  rh = ymax-ymin+1;
  (void)ih;

  if (active_ps->level1)
    return;

  if (active_ps->debug) fprintf(active_ps->psFile, "%%cdPutImageRectRGB Start\n");

  fprintf(active_ps->psFile, "[0 0 0 0 0 0] currentmatrix\n");
  fprintf(active_ps->psFile, "%d %d translate\n", x, y);
  fprintf(active_ps->psFile, "%d %d scale\n", w, h);

  fprintf(active_ps->psFile, "%d %d 8\n", rw, rh);
  fprintf(active_ps->psFile, "[%d 0 0 %d 0 0]\n", rw, rh);
  fprintf(active_ps->psFile, "{currentfile %d string readhexstring pop}\n", rw);
  fprintf(active_ps->psFile, "false 3\n");
  fprintf(active_ps->psFile, "colorimage\n");

  for (j=ymin; j<=ymax; j++)
  {
    for (i=xmin; i<=xmax; i++)
    {
      int pos = j*iw+i;
      fprintf(active_ps->psFile, "%02x%02x%02x", (int)r[pos], (int)g[pos], (int)b[pos]);
    }

    fprintf(active_ps->psFile, "\n");
  }

  fprintf(active_ps->psFile, "setmatrix\n");

  if (active_ps->debug) fprintf(active_ps->psFile, "%%cdPutImageRectRGBEnd\n");
}

static int isgray(int size, unsigned char *index, long int *colors)
{
  int i, pal_size = 0;
  unsigned char r, g, b;

  for (i = 0; i < size; i++)
  {
    if (index[i] > pal_size)
      pal_size = index[i];
  }

  pal_size++;

  for (i = 0; i < pal_size; i++)
  {
    cdDecodeColor(colors[i], &r, &g, &b);

    if (i != r || r != g || g != b)
      return 0;
  }

  return 1;
}

static void cdputimagerectmap(int iw, int ih, unsigned char *index, long int *colors, int x, int y, int w, int h, int xmin, int xmax, int ymin, int ymax)
{
  int i, j, rw, rh, is_gray;
  rw = xmax-xmin+1;
  rh = ymax-ymin+1;
  (void)ih;

  is_gray = isgray(iw*ih, index, colors);

  if (!is_gray && active_ps->level1)
    return;

  if (active_ps->debug) fprintf(active_ps->psFile, "%%cdPutImageRectMap Start\n");

  fprintf(active_ps->psFile, "[0 0 0 0 0 0] currentmatrix\n");
  fprintf(active_ps->psFile, "%d %d translate\n", x, y);
  fprintf(active_ps->psFile, "%d %d scale\n", w, h);

  fprintf(active_ps->psFile, "%d %d 8\n", rw, rh);
  fprintf(active_ps->psFile, "[%d 0 0 %d 0 0]\n", rw, rh);
  fprintf(active_ps->psFile, "{currentfile %d string readhexstring pop}\n", rw);

  if (is_gray)
  {
    fprintf(active_ps->psFile, "image\n");

    for (j=ymin; j<=ymax; j++)
    {
      for (i=xmin; i<=xmax; i++)
      {
        int pos = j*iw+i;
        fprintf(active_ps->psFile, "%02x", (int)index[pos]);
      }

      fprintf(active_ps->psFile, "\n");
    }
  }
  else
  {
    fprintf(active_ps->psFile, "false 3\n");
    fprintf(active_ps->psFile, "colorimage\n");

    for (j=ymin; j<=ymax; j++)
    {
      for (i=xmin; i<=xmax; i++)
      {
        int pos = j*iw+i;
        unsigned char r, g, b;
        cdDecodeColor(colors[index[pos]], &r, &g, &b);
        fprintf(active_ps->psFile, "%02x%02x%02x", (int)r, (int)g, (int)b);
      }

      fprintf(active_ps->psFile, "\n");
    }
  }

  fprintf(active_ps->psFile, "setmatrix\n");

  if (active_ps->debug) fprintf(active_ps->psFile, "%%cdPutImageRectMapEnd\n");
}

/******************************************************/
/* server images                                      */
/******************************************************/

static void cdpixel(int x, int y, long int color)
{
  if (active_ps->debug) fprintf(active_ps->psFile, "%%cdPixel Start\n");

  fprintf(active_ps->psFile, "%g %g %g setrgbcolor\n",
          get_red(color), get_green(color), get_blue(color));

  if (active_ps->level1)
  {
    fprintf(active_ps->psFile, "N\n");
    fprintf(active_ps->psFile, "%d %d 1 0 360 arc\n", x, y);
    fprintf(active_ps->psFile, "C fill\n");
  }
  else
    fprintf(active_ps->psFile, "%d %d 1 1 RF\n", x, y);

  active_ps->last_fill = -1;

  if (active_ps->eps) 
    bbox(x, y);

  if (active_ps->debug) fprintf(active_ps->psFile, "%%cdPixelEnd\n");
}

/******************************************************/
/* custom attributes                                  */
/******************************************************/

static void set_rotate_attrib(char* data)
{
  if (data)
  {
    sscanf(data, "%g %d %d", &active_ps->rotate_angle,
                             &active_ps->rotate_center_x,
                             &active_ps->rotate_center_y);
  }
  else
  {
    active_ps->rotate_angle = 0;
    active_ps->rotate_center_x = 0;
    active_ps->rotate_center_y = 0;
  }

  wdcall(active_ps->cnv->using_wd);
}

static char* get_rotate_attrib(void)
{
  static char data[100];

  if (!active_ps->rotate_angle)
    return NULL;

  sprintf(data, "%g %d %d", (double)active_ps->rotate_angle,
                            active_ps->rotate_center_x,
                            active_ps->rotate_center_y);

  return data;
}

static cdAttribute rotate_attrib =
{
  "ROTATE",
  set_rotate_attrib,
  get_rotate_attrib
}; 

static void set_cmd_attrib(char* data)
{
  fprintf(active_ps->psFile, data);
}

static cdAttribute cmd_attrib =
{
  "CMD",
  set_cmd_attrib,
  NULL
}; 

static void set_poly_attrib(char* data)
{
  int hole;

  if (data == NULL)
  {
    active_ps->holes = 0;
    return;
  }

  sscanf(data, "%d", &hole);
  active_ps->poly_holes[active_ps->holes] = hole;
  active_ps->holes++;
}

static char* get_poly_attrib(void)
{
  static char holes[10];
  sprintf(holes, "%d", active_ps->holes);
  return holes;
}

static cdAttribute poly_attrib =
{
  "POLYHOLE",
  set_poly_attrib,
  get_poly_attrib
}; 

/*
%F Cria um novo canvas PS
Parametros passados em data:
nome       nome do arquivo de saida <= 255 caracteres
-p[num]    tamanho do papel (A0-5, LETTER, LEGAL)
-w[num]    largura do papel em milimetros
-h[num]    altura do papel em milimetros
-l[num]    margem esquerda em milimetros
-r[num]    margem direita em milimetros
-b[num]    margem inferior em milimetros
-t[num]    margem superior em milimetros
-s[num]    resolucao em dpi
-e         encapsulated postscript
-1         level 1 operators only
-d[num]    margem da bbox em milimetros para eps
*/
static void cdcreatecanvas(cdPrivateCanvas* cnv, void *data)
{
  char *line = (char *)data;
  cdCanvasPS *new_ps;
  char filename[10240] = "";

  new_ps = (cdCanvasPS *)malloc(sizeof(cdCanvasPS));
  memset(new_ps, 0, sizeof(cdCanvasPS));

  line += cdGetFileName(line, filename);
  if (filename[0] == 0)
    return;

  if ((new_ps->psFile = fopen(filename, "w")) == NULL)
  {
    free(new_ps);
    return;
  }

  new_ps->holes = 0;
  cdRegisterAttribute(cnv, &poly_attrib);
  cdRegisterAttribute(cnv, &cmd_attrib);
  cdRegisterAttribute(cnv, &rotate_attrib);

  setpsdefaultvalues(new_ps);

  while (*line != '\0')
  {
    while (*line != '\0' && *line != '-') 
      line++;

    if (*line != '\0')
    {
      float num;
      line++;
      switch (*line++)
      {
      case 'p':
        {
          int paper;
          sscanf(line, "%d", &paper);
          setpspapersize(new_ps, paper);
          break;
        }
      case 'w':
        sscanf(line, "%g", &num);
        new_ps->width = mm2pt(num);
        break;
      case 'h':
        sscanf(line, "%g", &num);
        new_ps->height = mm2pt(num);
        break;
      case 'l':
        sscanf(line, "%g", &num);
        new_ps->xmin = num;
        break;
      case 'r':
        sscanf(line, "%g", &num);
        new_ps->xmax = num;   /* right margin, must be converted to xmax */
        break;
      case 'b':
        sscanf(line, "%g", &num);
        new_ps->ymin = num;  
        break;
      case 't':
        sscanf(line, "%g", &num);
        new_ps->ymax = num;  /* top margin, must be converted to ymax */
        break;
      case 's':
        sscanf(line, "%d", &(new_ps->res));
        break;
      case 'e':
        new_ps->eps = 1;
        break;
      case 'o':
        new_ps->landscape = 1;
        break;
      case '1':
        new_ps->level1 = 1;
        break;
      case 'g':
        new_ps->debug = 1;
        break;
      case 'd':
        sscanf(line, "%g", &num);
        new_ps->bbmargin = num;
        break;
      }
    }

    while (*line != '\0' && *line != ' ') 
      line++;
  }

  /* store the base canvas */
  new_ps->cnv = cnv;

  /* update canvas context */
  cnv->context_canvas = new_ps;

  if (new_ps->landscape == 1)
  {
    SWAPD(new_ps->width, new_ps->height);
    SWAPD(new_ps->xmin, new_ps->ymin);
    SWAPD(new_ps->xmax, new_ps->ymax);
  }

  init_ps(new_ps);
}

static void cdinittable(cdPrivateCanvas* cnv)
{
  cnv->Flush = cdflush;
  cnv->Pixel = cdpixel;
  cnv->Line = cdline;
  cnv->Poly = cdpoly;
  cnv->Rect = cdrect;
  cnv->Box = cdbox;
  cnv->Arc = cdarc;
  cnv->Sector = cdsector;
  cnv->Chord = cdchord;
  cnv->Text = cdtext;
  cnv->PutImageRectRGB = cdputimagerectrgb;
  cnv->PutImageRectMap = cdputimagerectmap;
  cnv->wLine = wdline;
  cnv->wPoly = wdpoly;
  cnv->wRect = wdrect;
  cnv->wBox = wdbox;
  cnv->wArc = wdarc;
  cnv->wSector = wdsector;
  cnv->wChord = wdchord;

  cnv->Clip = cdclip;
  cnv->ClipArea = cdcliparea;
  cnv->LineStyle = cdlinestyle;
  cnv->LineWidth = cdlinewidth;
  cnv->LineCap = cdlinecap;
  cnv->LineJoin = cdlinejoin;
  cnv->Pattern = cdpattern;
  cnv->Stipple = cdstipple;
  cnv->Hatch = cdhatch;
  cnv->Font = cdfont;
  cnv->NativeFont = cdnativefont;
  cnv->Foreground = cdforeground;

  cnv->KillCanvas = cdkillcanvas;
  cnv->Activate = cdactivate;
  cnv->Deactivate = cddeactivate;
  cnv->wdCall = wdcall;
}

static cdPrivateContext cdPSContextTable =
{
  CD_CAP_ALL & ~(CD_CAP_CLEAR | CD_CAP_PLAY | CD_CAP_PALETTE | 
                 CD_CAP_MARK | CD_CAP_REGION |
                 CD_CAP_IMAGESRV | 
                 CD_CAP_BACKGROUND | CD_CAP_BACKOPACITY | CD_CAP_WRITEMODE | 
                 CD_CAP_FONTDIM | CD_CAP_TEXTSIZE | 
                 CD_CAP_IMAGERGBA | CD_CAP_GETIMAGERGB | 
                 CD_CAP_VECTORTEXT),
  cdcreatecanvas,
  cdinittable,
  NULL,
  NULL
};

static cdContext cdPS =
{
  &cdPSContextTable
};


cdContext* cdContextPS(void)
{
  return &cdPS;
}


