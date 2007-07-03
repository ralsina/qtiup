char *cddgn_c = "$Id: cddgn.c,v 1.3 2005/06/23 20:23:20 scuri Exp $";
/*                                  
   %M Canvas Draw - CD_DGN MicroStation design file
   %a Vinicius Almendra
   %d 2 2 96
   %p Sistemas Graficos
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <assert.h>

#include "cd.h"
#include "cdprivat.h"
#include "cddgn.h"

/** verificar esse codigo **/

#if 0
#define RX 32767
#define RY 32767
#else
#define RX 16383
#define RY 16383
#endif

#define DX ((RX*0.0254)/1600)
#define DY ((RY*0.0254)/1600)

/* defines */

#define MAX_NUM_VERTEX 101
#define MAX_NUM_VERTEX_PER_POLYLINE 15000

#ifndef PI
#define PI 3.14159265358979323846
#endif

#define RAD(x) ((x)*(PI/180.))

#define END_OF_DGN_FILE ((short)0xffff)
#define DGN_FILE_BLOCK  512

#define NOFILL 0    /* tipos de fill que o driver faz */
#define CONVEX 1
#define NORMAL 2 

/* macros */

#define SIZE_LINE_STRING(x)  (19+4*(x))
#define SIZE_FILLED_SHAPE(x) (27+4*(x))
#define SIZE_ARC             40
#define SIZE_LINE            26

#define IGNORE(x) ((void) x)


/* prototipos de funcao */
static void startComplexShape(unsigned short,short,unsigned short,
                            unsigned long,unsigned long,
                            unsigned long,unsigned long);
static void endComplexElement(void);

/* estruturas similares as que o MicroStation usa */

typedef struct
{
  union 
  {
    struct 
    {  
      unsigned level:6;
      unsigned :1;
      unsigned complex:1;
      unsigned type:7;
      unsigned :1;
    } flags;
    short type_as_word;
  } type;

  unsigned short words;
  unsigned long  xmin;
  unsigned long  ymin;
  unsigned long  xmax;
  unsigned long  ymax;
} Elm_hdr; 

typedef struct
{
  short attindx;
  union
  {
    short s;
    struct
    {
      unsigned /*class*/ :4;
      unsigned /*res*/   :4;
      unsigned /*l*/     :1;
      unsigned /*n*/     :1;
      unsigned /*m*/     :1;
      unsigned attributes:1;
      unsigned /*r*/     :1;
      unsigned /*p*/     :1;
      unsigned /*s*/     :1;
      unsigned /*hole*/  :1;
    } flags;
  } props;

  union
  {
    short s;
    struct
    {
      unsigned style:3;
      unsigned weight:5;
      unsigned color:8;
    } b;
  } symb;

} Disp_hdr;

/* define um ponto */

typedef struct
{
  int x, y;
}
point;

/* tipo de poligono/setor */

enum
{
  FILLED,
  OPEN 
};

/* grupos de tamanhos de caracter 
   (usado por gettextwidth e cdfontdim) */

static long fontsizes[4][8]=
{
  {1,2,4,5,6,7,0},
  {5,3,2,1,0},
  {8,6,4,3,2,1,0},
  {5,3,2,1,0}
};


/**********************
 * contexto do driver *
 **********************/

typedef struct
{
  cdPrivateCanvas* cnv;
  FILE *arq_ptr;                             /* arquivo dgn */
  long int bytes;                            /* tamanho do arquivo */
  char level;
  
  short color, style;                 

  short alignment;
  short typeface;
  short symbology;
  long tl;
  short is_base;                               /* setado se texto e' do tipo CD_BASE_...   */

  short fill_type;                   /* como o driver faz fill:
                                           NOFILL -> nao faz fill
                                           CONVEX -> so faz fill de poligonos convexos
                                           NORMAL -> faz fill normalmente */

  long colortable[256];              /* palette */
  short num_colors;

  short is_complex;
} tDGN_GC;

static tDGN_GC *gc;  /* contexto */

/******************************
 *                            *
 * funcoes internas do driver *
 *                            *
 ******************************/

/*********************************************************
 * Obtem o descent do texto (para letras como q,p,g etc) * 
 *********************************************************/

static long get_descent(char *text)
{
 char *descent="jgyqp";
 long  a=0; 
 long length = strlen(text);

 while(a < length)
 {
   if(strchr(descent, text[a]))
     return gc->cnv->font_size/2;

   a++;
 }
 return 0;
}

/***********************************************
 * Calcula a largura da string no MicroStation *
 ***********************************************/

static long gettextwidth(char *s)
{
  long a=0,
       width=0,
       length = strlen(s);

  short font=0,default_size=0;

  static char *fontchars[4][8] =
  {
    {         /* CD_SYSTEM */
      "Ww",
      "jshyut#*-=<>",
      "iIl[]",
      ";:.,'|!()`{}",
      "","","",""
    }, 

    { /* CD_COURIER */
      "Iflrit!();.'",
      "1|[]\"/`",
      "BCDEKPRSUVXYbdgkpq&-_", 
      "w#%", 
      "Wm^+=<>~",
      "@","",""
    }, 

    { /* CD_TIMES_ROMAN */
      "m",
      "HMUWw",
      "CSTZLbhknpuvxy23567890e",
      "fstz1#$-=<>", 
      "Iijl*[]", 
      ";:.,'|!()`{}",
      "",""
    },
    
    { /* CD_HELVETICA */
      "Ww",
      "jshyut#*-=<>",
      "iIl[]", 
      ";:.,'|!()`{}",
      "","","",""
    }  
  };

  switch(gc->cnv->font_type_face)
  {
  case CD_SYSTEM:
    font=0;
    default_size=4;
    break;

  case CD_COURIER:
    font=1;
    default_size=2;
    break;
    
  case CD_TIMES_ROMAN:
    font=2;
    default_size=5;
    break;

  case CD_HELVETICA:
    font=3;
    default_size=4;
    break;
  }


  for(a=0,width=0;a < length; a++)
  { 
    static short size_number;
    static char letter;
   
    if(s[a] == ' ')
      letter = s[a-1];
    else
      letter = s[a];
      
    for(size_number=0;size_number < 8;size_number++)
    {
      if(strchr(fontchars[font][size_number], letter))
      {
        width+=(gc->tl*fontsizes[font][size_number])/6;
        break;
      }
    }
    
    if(size_number == 8)
      width+=(gc->tl*default_size)/6;

    width+=gc->tl/3;
  }
  
  width-=gc->tl/3;

  if((gc->cnv->font_style == CD_ITALIC) || (gc->cnv->font_style == CD_BOLD_ITALIC))
    width+= (long) ((double) gc->cnv->font_size*tan(4*atan(1)/8)); /* angulo de 15 graus */

  return width;
}

/****************************
 * Salva um byte no arquivo *
 ****************************/

static void put_byte(unsigned char byte)
{
  fputc(byte, gc->arq_ptr);
}

/************************************
 * Salva um sequencia de caracteres *
 ************************************/

static void writec (const char *t, short tam )
{
 short i;

 gc->bytes += tam;
 for ( i = 0; i < tam; i++ )
     fputc ( t[i], gc->arq_ptr );
}

/******************
 * Salva uma word *
 ******************/

static void put_word (unsigned short w)
{
  char c;

  c = (char) (w & 0xff);
  fputc (c, gc->arq_ptr);
  c = (char) ((w >> 8) & 0xff);
  fputc (c, gc->arq_ptr);
  gc->bytes += 2;
}

/****************************
 * Salva um long no arquivo *
 ****************************/

static void put_long (unsigned long i)
{
  put_word ((short) (i >> 16));
  put_word ((short) i);
}

/*******************
 * Salva um double *
 *******************/

static void put_as_double(long i)
{ 
  float dfloat=(float) 4*i;
 
  put_long(*((long *) &dfloat)); 
  put_long(0);

  gc->bytes+=sizeof(float);
}

/****************************
 * Salva uma UOR no arquivo *
 ****************************/

static void put_uor (long i)
{
  int s = (short) (1 << 15);

  put_word ((unsigned short)((i >> 16) ^ s)); /* troca o bit 31 
                                             para transformar em uor */
  put_word ((unsigned short)i);
}

/*************************************** 
 * Salva a bounding box de um elemento *
 ***************************************/

static void put_bound (long xmin, long xmax, long ymin, long ymax)
{
  put_uor (xmin);
  put_uor (ymin);
  put_uor (0L);
  put_uor (xmax);
  put_uor (ymax);
  put_uor (0L);
}

/******************************************
 * Calcula a bounding box de uma polyline *
 ******************************************/

static void line_string_bound(point *buffer, short num_vertex,
                     unsigned long *xmin, unsigned long *ymin, 
                     unsigned long *xmax, unsigned long *ymax)
{
  short i;
  unsigned long v;
  
  *xmin = *xmax = buffer[0].x;
  *ymin = *ymax = buffer[0].y;

  for (i = 1; i < num_vertex; i++)
  {
	  v = buffer[i].x;
    if (v < *xmin)
      *xmin = v;
    else if (v > *xmax)
      *xmax = v;

   v = (long) buffer[i].y;
	 if (v < *ymin)
      *ymin = v;
    else if (v > *ymax)
      *ymax = v;
  }
}

/************************************
 * Retorna symbology de um elemento *
 ************************************/

static short symbology(void)
{
  return ((gc->color << 8) | (gc->cnv->line_width << 3) | gc->style);
} 

/*****************************************
 * Salva um Element Header no arquivo *
 *****************************************/

static void putElementHeader(Elm_hdr *ehdr)
{
  ehdr->type.flags.complex = gc->is_complex;
  
  put_word((short)(ehdr->type.flags.type << 8 |
    ehdr->type.flags.complex << 7 | ehdr->type.flags.level));
  

  put_word (ehdr->words);  
  put_bound(ehdr->xmin, ehdr->xmax, ehdr->ymin, ehdr->ymax);
}

/**************************************
 * Salva um display header no arquivo *
 **************************************/

static void putDisplayHeader(Disp_hdr *dhdr)
{
  put_word(0);                      /* graphics group */
  put_word(dhdr->attindx);          /* index to attributes */
  put_word(dhdr->props.flags.attributes << 11); /* properties */
  put_word(dhdr->symb.s);           /* display symbology */
}


/***************************************
 * completa o arquivo com zeros para   *
 * que o numero de bytes seja multiplo *
 * de 512                              *
 ***************************************/

static void complete_file (void)
{
  long resto, i;

  put_word (END_OF_DGN_FILE);

  resto = DGN_FILE_BLOCK - gc->bytes % DGN_FILE_BLOCK;

  /* checa validade do tamanho do arquivo */
  assert(resto%2 == 0);

  for (i = 0; i < resto; i+=2)
    put_word(0);
}

/*************************************
 * Salva um elemento arco no arquivo *
 *************************************/

static void arc (long xc, long yc, long w, long h, double a1, double a2)
{
  Elm_hdr ehdr;
  Disp_hdr dhdr; 

    /* raster header element */
  ehdr.type.flags.level=gc->level;
  ehdr.type.flags.type=16;
  ehdr.words=SIZE_ARC-2;
  ehdr.xmin=xc - w/2;
  ehdr.xmax=xc + w/2;
  ehdr.ymin=yc - h/2;
  ehdr.ymax=yc + h/2;

  putElementHeader(&ehdr);

    /* Display Header */
  dhdr.attindx = ehdr.words - 14;
  dhdr.props.flags.attributes = 0;
  dhdr.symb.s = symbology();
  putDisplayHeader(&dhdr);

  put_long((long) a1*360000);       /* start angle */
  put_long((long) (a2-a1)*360000);  /* sweep angle */
  put_as_double(w/2);           /* primary axis */
  put_as_double(h/2);           /* secondary axis */
  put_long(0);                      /* rotation angle (sempre 0) */
  put_as_double(xc);          /* x origin */
  put_as_double(yc);          /* y origin */
}

/***************************************
 * Salva um elemento elipse no arquivo *
 ***************************************/

static void ellipse(long xc, long yc, long w, long h, short type)
{
  Elm_hdr ehdr;
  Disp_hdr dhdr; 

    /* raster header element */
  ehdr.type.flags.level=gc->level;
  ehdr.type.flags.type=15;
  ehdr.words=34+((type==FILLED) ? 8 : 0);
  ehdr.xmin=xc - w/2;
  ehdr.xmax=xc + w/2;
  ehdr.ymin=yc - h/2;
  ehdr.ymax=yc + h/2;

  putElementHeader(&ehdr);

    /* Display Header */
  dhdr.attindx=20;
  dhdr.props.flags.attributes=(type == FILLED) ? 1 : 0;
  dhdr.symb.s=symbology();
  putDisplayHeader(&dhdr);

  put_as_double(w/2);           /* primary axis */
  put_as_double(h/2);           /* secondary axis */
  put_long(50);               /* rotation angle (sempre 0) */
  put_as_double(xc);          /* x origin */
  put_as_double(yc);          /* y origin */
  
    /* salva atributo de fill */
  if(type == FILLED)
  {
    put_word(0x1007);
    put_word(65);
    put_word(0x802);
    put_word(0x0001);
    put_word(gc->color);
    put_word(0);
    put_word(0);
    put_word(0);
  }
}

static short getclosestColor(long color)
{
  short count=0, closest=0;
  long diff=0;
  unsigned char r = cdRed(color),
                g = cdGreen(color),
                b = cdBlue(color);
  short rd, gd, bd;
  long newdiff;

  /* procura a cor mais proxima */

  diff = 3*65536;  /* inicializa com maior diferenca possivel */

  for(count=0; count < gc->num_colors; count++)
  {
    rd = r - cdRed(gc->colortable[count]);
    gd = g - cdGreen(gc->colortable[count]);
    bd = b - cdBlue(gc->colortable[count]);

    newdiff = rd*rd + gd*gd + bd*bd;

    if(newdiff <= diff)
    {
      /* verifica se encontrou a cor */
      if(newdiff == 0)
        return count-1;

      diff = newdiff;
      closest=count-1;
    }
  }

  /* nao encontrou a cor, tenta adicionar na palette, ou retorna a mais proxima */
  if(gc->num_colors < 254)
  {
    gc->colortable[gc->num_colors+1] = color;
      return gc->num_colors++;
  }
  else
    return closest;
}

static void saveColorTable(void)
{
  unsigned char r,g,b;
  short i;
  
  put_word ((0x05 << 8) | 1);  /* colortable */
  put_word (434);

  put_long(0);
  put_long(0);
  put_long(0);
  put_long(0xffffffff);
  put_long(0xffffffff);
  put_long(0xffffffff);

  put_word(0);
  put_word(420);
  put_word(0x0400);
  put_word (0x100);
  put_word(0);

  for(i=0;i<256;i++)
  {
	 cdDecodeColor(gc->colortable[i], &r, &g, &b);
	 put_byte(r);
	 put_byte(g);
	 put_byte(b);
  }

  put_word(25);
  for(i=0;i<32;i++)
	 put_word(0x2020);
}


/*****************************
 * Le uma word de um arquivo *
 *****************************/

static short file_get_word(FILE *fp)
{
  short word=0;

  word =  fgetc(fp);
  word |= ((short)fgetc(fp) << 8) & 0xff00;    

  return word;
}

/********************************
 * Salva uma word em um arquivo *
 ********************************/

static void file_put_word (short word, FILE *fp)
{
  fputc ((char) (word & 0xff), fp);
  fputc ((char) ((word >> 8) & 0xff), fp);
}

/*******************************************
 * Le elementos de um arquivo DGN e os     *
 * coloca no inicio do arquivo aberto pelo *
 * driver                                  *
 *******************************************/

static void dgn_copy (FILE *dgn, tDGN_GC *fgc)
{
  short word=0;

  while ((word = file_get_word(dgn)) != END_OF_DGN_FILE) 
  {
    file_put_word(word, fgc->arq_ptr); /* type e level do elemento */
    fgc->bytes+=2;

    word = file_get_word(dgn); /* words to follow */
    file_put_word(word, fgc->arq_ptr);
    fgc->bytes+=2;

    while (word)       /* copia resto do elemento */
    {
      file_put_word(file_get_word(dgn), fgc->arq_ptr);
      word--;
      fgc->bytes+=2;
    }
  }
}


/*
 *  Funcoes do driver
 */

static void cdkillcanvas(void *cnv)
{
  tDGN_GC *old_gc = gc;
  gc = (tDGN_GC *)cnv;

  saveColorTable();
  complete_file ();

  fclose (gc->arq_ptr);
  free(gc);

  gc = old_gc;
}

static int cdactivate(void *new_gc)
{
  gc = (tDGN_GC *) new_gc;
  return CD_OK;
}

static void cddeactivate (void)
{
  fflush(gc->arq_ptr);
  gc = NULL;
}

static void cdflush (void)
{
  assert(gc->arq_ptr);
  fflush(gc->arq_ptr);
}


/******************************************************/
/* primitives                                         */
/******************************************************/

static void cdline (int x1, int y1, int x2, int y2)
{
  Elm_hdr ehdr;
  Disp_hdr dhdr;
  point buffer[2];

  buffer[0].x=x1;
  buffer[0].y=y1;
  buffer[1].x=x2;
  buffer[1].y=y2;
  
  ehdr.type.flags.level=gc->level;
  ehdr.type.flags.type=3;
  
  ehdr.words=SIZE_LINE-2;

  line_string_bound(buffer, 2, &ehdr.xmin,
                    &ehdr.ymin,&ehdr.xmax,&ehdr.ymax);
  
  putElementHeader(&ehdr);

    /* Display Header */
  dhdr.attindx = ehdr.words - 14;
  dhdr.props.flags.attributes = 0;
  dhdr.symb.s=symbology();

  putDisplayHeader(&dhdr);

    /* pontos inicial e final da linha */

  put_long ((long) x1);     
  put_long ((long) y1);
  put_long ((long) x2);
  put_long ((long) y2);
}

static void cdbox (int xmin, int xmax, int ymin, int ymax)
{
  Elm_hdr ehdr;
  Disp_hdr dhdr; 

    /* raster header element */
  ehdr.type.flags.level=gc->level;
  ehdr.type.flags.type=6;
  ehdr.words=17+4*5+8;
  ehdr.xmin=xmin;
  ehdr.xmax=xmax;
  ehdr.ymin=ymin;
  ehdr.ymax=ymax;

  putElementHeader(&ehdr);

    /* Display Header */
  dhdr.attindx=3+4*5;
  dhdr.props.flags.attributes=1;
  dhdr.symb.s=symbology();
  putDisplayHeader(&dhdr);

  put_word (5);                       /* numero de vertices */

   /* vertices */
  put_long ((long) xmin);
  put_long ((long) ymin);
  
  put_long ((long) xmax);
  put_long ((long) ymin);
 
  put_long ((long) xmax);
  put_long ((long) ymax);

  put_long ((long) xmin);
  put_long ((long) ymax);

  put_long ((long) xmin);
  put_long ((long) ymin);
 
    /* atributos de fill */

  put_word(0x1007);
  put_word(65);
  put_word(0x802);
  put_word(0x0001);
  put_word(gc->color);
  put_word(0);
  put_word(0);
  put_word(0);
}

static void cdarc (int xc, int yc, int w, int h, double a1, double a2)
{
  if (a2 == a1 + 360)
    ellipse(xc,yc,w,h,OPEN);
  else
    arc(xc, yc, w, h, a1, a2);
}

static void cdsector (int xc, int yc, int w, int h, double a1, double a2)
{
  if (a2 == a1 + 360)
  {
    ellipse(xc,yc,w,h,FILLED);
    return;
  }

  startComplexShape(3, 1, SIZE_ARC+SIZE_LINE*2,
    (unsigned long) xc-w/2, (unsigned long) yc-h/2,
    (unsigned long) xc+h/2, (unsigned long) yc+h/2);

  arc(xc, yc, w, h, a1, a2);
  
  cdline(xc, yc, (int)
    (((double)w*cos(RAD(a1))/2.+.5)), (int) (((double)h*sin(RAD(a1)))/2.+.5));
  cdline(xc, yc, (int) 
    (((double)w*cos(RAD(a2))/2.+.5)), (int) (((double)h*sin(RAD(a2)))/2.+.5));

  endComplexElement();
}

static void cdtext (int x, int y, char *s)
{
  long n=0;
  long descent=0;
  short w=0;
  long hc=0,wc=0;
  short italic = (gc->cnv->font_style == CD_BOLD_ITALIC) || 
               (gc->cnv->font_style == CD_ITALIC);
 
  Elm_hdr ehdr;
  Disp_hdr dhdr;

  n = strlen(s);

  if(n > 255)
    n=255;

  w = (short)((n/2)+(n%2));
  descent=get_descent(s);
  hc = gc->cnv->font_size+descent;
  wc = gettextwidth(s);
 
  y+=descent;

  switch (gc->alignment)
  {
  case 12: x = x;                   y = y;                   break;
  case 13: x = x;                   y = y - (int) (hc/2.0);  break;
  case 14: x = x;                   y = y - (int) hc;        break;
  case  6: x = x - (int) (wc/2.0);  y = y;                   break;
  case  7: x = x - (int) (wc/2.0);  y = y - (int) (hc/2.0);  break;
  case  8: x = x - (int) (wc/2.0);  y = y - (int) hc;        break;
  case  0: x = x - (int) wc;        y = y;                   break;
  case  1: x = x - (int) wc;        y = y - (int) (hc/2.0);  break;
  case  2: x = x - (int) wc;        y = y - (int) hc;        break;
  }

  if(gc->is_base)
    y -= (int) (hc/4.0);

   /* raster header element */
  ehdr.type.flags.level=gc->level;
  ehdr.type.flags.type=17;
  ehdr.words=28+w+((italic) ? 8 : 0);
  ehdr.xmin=x;
  ehdr.xmax=x+wc;
  ehdr.ymin=y-descent;
  ehdr.ymax=y+hc;

  putElementHeader(&ehdr);

    /* Display Header */
  dhdr.attindx=14+w;
  dhdr.props.flags.attributes=(italic) ? 1 : 0;

  switch(gc->cnv->font_style)
  {
  case CD_BOLD_ITALIC:
  case CD_BOLD:
    dhdr.symb.s=gc->color << 8 | (3 << 3);
    break;

  default:
    dhdr.symb.s=gc->color << 8;
    break;
  }

  putDisplayHeader(&dhdr);

  put_word ((gc->alignment << 8) | gc->typeface);

  put_long ((long)(1000 * gc->tl / 6) | (1 << 7));
  put_long ((long)(1000 * gc->cnv->font_size / 6) | (1 << 7));
 
  put_long(0);
  put_long(x);
  put_long(y);
  put_word((unsigned short) n);
  writec(s, n+(n%2)); /* deve escrever sempre um numero par de caracteres */

  if(italic)
  {
    put_word(0x1007);  /* atributos e words to follow */
    put_word(0x80d4);  /* tipo de atributo */
    put_long(0x000865c0);
    put_long(0x00520000);
    put_long(0x00000000);
  }
}

static void startComplexShape(unsigned short num_elements,
                            short is_fill,
                            unsigned short size,
                            unsigned long xmin,
                            unsigned long ymin,
                            unsigned long xmax,
                            unsigned long ymax)
{
  Elm_hdr ehdr;
  Disp_hdr dhdr;

  ehdr.type.flags.level=gc->level;
  ehdr.type.flags.type=14;
  ehdr.words=22 + ((is_fill) ? 8 : 0);
  ehdr.xmax = xmax; 
  ehdr.xmin = xmin; 
  ehdr.ymax = ymax; 
  ehdr.ymin = ymin; 

  putElementHeader(&ehdr);

    /* Display Header */
  dhdr.attindx=4;
  dhdr.props.flags.attributes = (is_fill) ? 1 : 0;
  dhdr.symb.s=symbology();

  putDisplayHeader(&dhdr);

  put_word (size + 5 + ((is_fill) ? 8 : 0));
  put_word (num_elements);

  put_long(0);  /* atributo nulo */
  put_long(0);
 
    /* salva atributo de fill */
  if(is_fill)
  {
    put_word(0x1007);
    put_word(65);
    put_word(0x802);
    put_word(0x0001);
    put_word(gc->color);
    put_word(0);
    put_word(0);
    put_word(0);
  }

  /* marca inicio de elemento complexo */

  gc->is_complex = 1;
}


static void startComplexChain(unsigned short num_elements,
                            unsigned short size,
                            unsigned long xmin,
                            unsigned long ymin,
                            unsigned long xmax,
                            unsigned long ymax)
                            
{
  Elm_hdr ehdr;
  Disp_hdr dhdr;

  ehdr.type.flags.level=gc->level;
  ehdr.type.flags.type=12;

  ehdr.words=22;
  ehdr.xmax = xmax; 
  ehdr.xmin = xmin; 
  ehdr.ymax = ymax; 
  ehdr.ymin = ymin; 


  putElementHeader(&ehdr);

    /* Display Header */
  dhdr.attindx=4;
  dhdr.props.flags.attributes = 1;
  dhdr.symb.s=symbology();

  putDisplayHeader(&dhdr);

  put_word (size+5);
  put_word (num_elements);

  put_long(0);  /* atributo nulo */
  put_long(0);

  
  /* marca inicio de elemento complexo */

  gc->is_complex = 1;
}

static void endComplexElement(void)
{
  gc->is_complex = 0;
}

static void putLineString(point *buffer, short num_vertex)
{
  Elm_hdr ehdr;
  Disp_hdr dhdr;
  short i=0;

  ehdr.type.flags.level=gc->level;

  ehdr.type.flags.type=4;
  
  ehdr.words=SIZE_LINE_STRING(num_vertex)-2;

  line_string_bound(buffer, num_vertex, &ehdr.xmin,
                    &ehdr.ymin,&ehdr.xmax,&ehdr.ymax);
  
  putElementHeader(&ehdr);

    /* Display Header */
  dhdr.attindx=ehdr.words-14;
  dhdr.props.flags.attributes = 0;
  dhdr.symb.s=symbology();

  putDisplayHeader(&dhdr);

  put_word (num_vertex);

  for (i = 0; i < num_vertex; i++)
  {
    put_long ((long) buffer[i].x);
    put_long ((long) buffer[i].y);
  }
}

static void putShape(point *buffer, short num_vertex)
{
  Elm_hdr ehdr;
  Disp_hdr dhdr;
  short i=0;

  ehdr.type.flags.level=gc->level;
  ehdr.type.flags.type=6;

  ehdr.words=SIZE_FILLED_SHAPE(num_vertex)-2;

  line_string_bound(buffer, num_vertex, &ehdr.xmin,
                    &ehdr.ymin,&ehdr.xmax,&ehdr.ymax);
  

  putElementHeader(&ehdr);

    /* Display Header */
  dhdr.attindx=ehdr.words - 14 - 8; /* 8 -> size of attributes */
  dhdr.props.flags.attributes = 1;
  dhdr.symb.s=symbology();

  putDisplayHeader(&dhdr);

  put_word (num_vertex);

  for (i = 0; i < num_vertex; i++)
  {
    put_long ((long) buffer[i].x);
    put_long ((long) buffer[i].y);
  }

  put_word(0x1007);
  put_word(65);
  put_word(0x802);
  put_word(0x0001);
  put_word(gc->color);
  put_word(0);
  put_word(0);
  put_word(0);
}

static void cdpoly(int mode, cdPoint* poly, int n)
{
  short is_fill=0;

  if(mode == CD_FILL && gc->fill_type == NOFILL)
    mode = CD_CLOSED_LINES;

  if(n > MAX_NUM_VERTEX_PER_POLYLINE)
    n = MAX_NUM_VERTEX_PER_POLYLINE;

  /* acerta buffer de pontos */
  if(mode == CD_FILL || mode == CD_CLOSED_LINES)
  {
    poly[n].x = poly[0].x;
    poly[n].y = poly[0].y;
    n++;
  }

  /* se fill_type for CONVEX, testa se poligono e' convexo ou concavo */
  if((gc->fill_type == CONVEX) && (n > 3) && (mode == CD_FILL))
  {
    short signal=0;
    short count=0;
    long vect=0;

    /* calcula sinal do vetorial entre o primeiro lado e o segundo */
    vect = (poly[1].x - poly[0].x) * 
           (poly[2].y - poly[1].y) -
           (poly[1].y - poly[0].y) * 
           (poly[2].x - poly[1].x);

    if(vect == 0)
      mode = CD_CLOSED_LINES;  /* ver se precisa mudar */
    else
    {
      signal = vect/abs(vect);

      for(count=1 ; count< (n-2); count++)
      { 
        vect = (poly[count+1].x - poly[count].x) * 
               (poly[count+2].y - poly[count+1].y) -
               (poly[count+1].y - poly[count].y) * 
               (poly[count+2].x - poly[count+1].x);
      
        if(vect == 0)
        {
          mode=CD_CLOSED_LINES;
          break;
        }

        if((vect/abs(vect)) != signal)
        {
          mode=CD_CLOSED_LINES;
          break;
        }
      }
    }
  }

  /* se tiver fill */

  if(mode == CD_FILL)
    is_fill=1;

  if(n > MAX_NUM_VERTEX)  /* tem que usar complex shape ou chain */
  {
    short count=0;
    short num_whole_elements = n / MAX_NUM_VERTEX;
    short num_whole_vertex = num_whole_elements * MAX_NUM_VERTEX;
    short rest = n % MAX_NUM_VERTEX;
    short is_there_rest = (rest > 0) ? 1 : 0;
    unsigned long xmax, xmin, ymax, ymin;
    unsigned short size =
         SIZE_LINE_STRING(MAX_NUM_VERTEX)*num_whole_elements+ 
         SIZE_LINE_STRING(rest)*is_there_rest;

    line_string_bound((point*)poly, n, &xmin, &ymin, &xmax, &ymax);

    if(mode == CD_OPEN_LINES)
      startComplexChain((unsigned short) (num_whole_elements+((rest > 0) ? 1 : 0)),
                         size, xmin, ymin, xmax, ymax);
    else
      startComplexShape((unsigned short) (num_whole_elements+((rest > 0) ? 1 : 0)),
                        is_fill, size, xmin, ymin, xmax, ymax);

    for(count=0;count < num_whole_vertex; count+=MAX_NUM_VERTEX, n-=MAX_NUM_VERTEX)
      putLineString(&((point*)poly)[count], MAX_NUM_VERTEX);

    if(rest)
      putLineString(&((point*)poly)[num_whole_vertex],n);

    endComplexElement();
  }
  else
  {
    if(is_fill)
      putShape((point*)poly, n);
    else
      putLineString((point*)poly, n);
  }
}

/**************
 * attributes *
 **************/

static int cdlinestyle (int style)
{
  switch(style)
  {
  case CD_CONTINUOUS:
    gc->style = 0;
    break;

  case CD_DASHED:
    gc->style = 3;
    break;

  case CD_DOTTED:
    gc->style = 1;
    break;

  case CD_DASH_DOT:
    gc->style = 4;
    break;

  case CD_DASH_DOT_DOT:
    gc->style = 6;
    break;
  }

  return style;
}

static int cdlinewidth (int width)
{
  width = width & 31;
  return width;
}

static void cdfont (int *type_face, int *style, int *size)
{
  IGNORE(style);             

  if (*size < 0)
  {
    double size_mm;
    cdPixel2MM(-*size, 0, &size_mm, NULL);
    *size = (int)(size_mm * CD_MM2PT + 0.5);
  }

  gc->tl = (long) (*size/4)*3;

  switch(*type_face)
  {
    case CD_COURIER:
      gc->typeface=43; /* fonte ? */
      break;

    case CD_TIMES_ROMAN:
      gc->typeface=2; /* fonte fancy */
      break;

    case CD_HELVETICA:
    case CD_SYSTEM:
      gc->typeface=0; /* fonte standard */
      break;
  }
}

static void cdnativefont(char *font)
{
  gc->typeface = atoi(font);
}

static void cdfontdim (int *max_width, int *height, int *ascent, int *descent)
{
  if(max_width)
  {
    int a=0;
    *max_width=0;

    while(fontsizes[gc->typeface][a])
    {
      if(fontsizes[gc->typeface][a] > *max_width)
        *max_width = fontsizes[gc->typeface][a];
      a++;
    }

    assert(*max_width != 0);
  }

  if(height)  *height=gc->cnv->font_size*3/2;
  if(ascent)  *ascent=gc->cnv->font_size;
  if(descent) *descent=gc->cnv->font_size/2;
}

static void cdtextsize (char *s, int *width, int *height)
{
 if(height) *height = gc->cnv->font_size + get_descent(s);
 if(width)  *width  = gettextwidth(s);
}

static int cdtextalignment (int alignment)
{
  gc->is_base = 0;

  /* DGN guarda posicao do texto em relacao ao ponto */
    
  switch(alignment)
  {
  case CD_NORTH:
    gc->alignment = 8; /* center-bottom */
    break;

  case CD_SOUTH:
    gc->alignment = 6; /* center-top */
    break;

  case CD_EAST:
    gc->alignment = 1; /* left-center */
    break;

  case CD_WEST:
    gc->alignment = 13; /* right-center */
    break;

  case CD_NORTH_EAST:
    gc->alignment = 2; /* left-bottom */
    break;

  case CD_NORTH_WEST:
    gc->alignment = 14; /* right-bottom */
    break;

  case CD_SOUTH_EAST:
    gc->alignment = 0; /* left-top */
    break;

  case CD_SOUTH_WEST:
    gc->alignment = 12; /* right-top */
    break;

  case CD_CENTER:
    gc->alignment = 7; /* center-center */
    break;

  case CD_BASE_LEFT:
    gc->alignment = 13; /* right-center */
    gc->is_base=1;
    break;

  case CD_BASE_CENTER:
    gc->alignment = 7; /* center-center */
    gc->is_base=1;
    break;

  case CD_BASE_RIGHT:
    gc->alignment = 1; /* left-center */
    gc->is_base=1;
    break;
  }

  return alignment;
}

/******************************************************/
/* color                                              */
/******************************************************/

static void cdpalette (int n, long int *palette, int mode)
{
  int c=0;

  IGNORE(mode);
  
  for(c=0; c < n; c++)
    gc->colortable[c] = *palette++;

  gc->num_colors = n;
}

static long int cdforeground (long int color)
{
  gc->color = getclosestColor(color);
  return color;
}

/******************************************************/
/* client images                                      */
/******************************************************/

static void cdputimagerectmap(int iw, int ih, unsigned char *index,
			 long int *colors, int x, int y, int w, int h, int xmin, int xmax, int ymin, int ymax)
{
  int i=0,j=0, remainder=iw%2, total_colors;
  int *ix=NULL,*iy=NULL;
  Elm_hdr ehdr;
  Disp_hdr dhdr; 
  unsigned char map_colors[256];

  /* aloca memoria (tem que ser aqui, senao uma falha pode deixar
     um elemento pela metade) */

  ix = (int *) malloc(w*sizeof(int));
  iy = (int *) malloc(h*sizeof(int)); 

    /* raster header element */
  ehdr.type.flags.level=gc->level;
  ehdr.type.flags.type=87;
  ehdr.words=39;
  ehdr.xmin=x;
  ehdr.xmax=x+w;
  ehdr.ymin=y;
  ehdr.ymax=y+h;

  putElementHeader(&ehdr);

    /* Display Header */
  dhdr.attindx=25;
  dhdr.symb.s=0;

  putDisplayHeader(&dhdr);

    /* description of the element */
  put_long(21+(23+w/2+w%2)*h);  /* total length of cell */
  put_word(0x0714);         /* raster flags */
  put_word(0x0100);         /* background e foreground colors
                               (nao usados) */
  put_word(w);             /* largura da imagem em pixels */
  put_word(h);             /* altura da imagem em pixel */ 
  put_long(0);              /* resevado */
  put_as_double(0);            /* resolution (nao usado) */

  put_word(0x4080);         /* scale */
  put_long(0);
  put_word(0);

  put_long(x);              /* origem */ 
  put_long(y+h);
  put_long(0);

  put_word(0);              /* no de vertices */ 

  gc->is_complex = 1; /* elemento complexo */

  /* obtem o maior indice usado na imagem */

  total_colors = 0;
  for (i = 0; i < iw*ih; i++)
  {        
    if (index[i] > total_colors)
      total_colors = index[i];
  }
  total_colors++;

  /* cria tabela para acelerar match de cor na palette */

  for (i = 0; i < total_colors; i++)
  {        
    map_colors[i] = (unsigned char)getclosestColor(colors[i]);
  }

  /** salva dados da imagem **/

  /* calcula stretch */

  cdMakeStretchTable(w, xmax-xmin+1, ix);
  cdMakeStretchTable(h, ymax-ymin+1, iy);
 
  for(i=h-1; i >= 0; i--)
  {
      /* raster header element */
    ehdr.type.flags.level=gc->level;
    ehdr.type.flags.type=88;
    ehdr.words=21+w/2+remainder;

    putElementHeader(&ehdr);

      /* Display Header */
    dhdr.attindx=7+w/2+remainder;
    dhdr.symb.s=0;
    putDisplayHeader(&dhdr);

    put_word(0x0714);   /* raster flags */
    put_word(0x0100);   /* background e foreground 
                           colors (nao usados) */

    put_word(0);    /* x offset da origem */
    put_word(i);    /* y offset */
    put_word(w);    /* numero de pixels neste elemento */
    
    for(j=0; j < w; j++)   
      put_byte(map_colors[index[(iy[i]+ymin)*iw+ix[j]+xmin]]);

    if(remainder) put_byte(0);
  }

  gc->is_complex = 0;

  free(ix);  /* libera memoria alocada */
  free(iy);
}

/******************************************************/
/* server images                                      */
/******************************************************/

static void cdpixel (int x, int y, long int color)
{
  long  old_color = cdforeground(color);
  int old_linestyle = cdlinestyle(CD_CONTINUOUS);
  int old_linewidth = cdlinewidth(1);

  cdline(x,y,x,y);

  cdforeground(old_color);
  cdlinestyle(old_linestyle);
  cdlinewidth(old_linewidth);
}

static void cdcreatecanvas(cdPrivateCanvas* cnv, void *data)
{
  tDGN_GC *new_dgn;
  char* strdata = (char*)data;
  char words[4][256];
  char filename[10240] = "";
  char seedfile[10240] = "";
  int count  = 0;
  double res = 0;
  int maxint = (sizeof(int)>2) ? 0x7fffffff : 0x7fff; /* maior inteiro */
    
  /* checa para ver se ponteiro e' NULL */
  assert(data);

  /* separa palavras da expressao, que e' na forma
     "filename [mm_wxmm_h] [res] [-f] [-sseedfile]" */

  strdata += cdGetFileName(strdata, filename);
  if (filename[0] == 0)
    return;

  sscanf(strdata, "%s %s %s %s", words[0], words[1], words[2], words[3]);
  
  if(!strlen(filename)) /* se nao pegou filename */
    return;
   
  new_dgn = (tDGN_GC *) malloc(sizeof(tDGN_GC));

  /* tenta criar arquivo DGN */

  if((new_dgn->arq_ptr=fopen (filename, "wb"))==NULL)
  {
    free(new_dgn);
    return;
  }

  /* verifica se foi passado tamanho do canvas em mm. Se foi,
     extrai-o da string */

  if(sscanf(words[0], "%lgx%lg",
     &cnv->w_mm, &cnv->h_mm) == 2)
  {
    count++; /* incrementa contador de palavras */

    if(cnv->w_mm == 0 || cnv->h_mm == 0)
    { 
      fclose(new_dgn->arq_ptr);
      free(new_dgn);
      return;
    }
  }
  else
    cnv->w_mm = cnv->h_mm = 0;
  
  /* Verifica se foi passada resolucao */

  if(sscanf(words[count], "%lg", &res) == 1)
  {
    count++; /* incrementa contador de palavras */

    if(res <= 0)  /* verifica validade da razao */
    { 
      fclose(new_dgn->arq_ptr);
      free(new_dgn);
      return;
    }
  }
  else
    res = 3.78;

  /* se tamanho em milimetros nao tiver sido inicializado,
     usa como default o tamanho maximo em pixels para fazer as
     contas
   */

  if(cnv->w_mm == 0)
  {
    cnv->w = maxint;
    cnv->h = maxint;

    cnv->w_mm = cnv->w / res;
    cnv->h_mm = cnv->h / res;
  }
  else
  {
    cnv->w = (long) (cnv->w_mm * res);
    cnv->h = (long) (cnv->h_mm * res);
  }

  cnv->xres = res;
  cnv->yres = res;
  cnv->bpp = 8;
  
  /* verifica se usuario que alterar metodo de fill */

  if(!strcmp(words[count], "-f"))
  {
    new_dgn->fill_type = CONVEX;
    count++;
  }
  else
    new_dgn->fill_type = NORMAL;
  
  /* se tiver passado seedfile como argumento */
  if(sscanf(words[count], "-s%s", seedfile) == 1)
  {
    FILE *seed=NULL;
    char *cd_dir = getenv("CDDIR");
    static char newfilename[512];

    if(cd_dir == NULL)
      cd_dir = ".";

    sprintf(newfilename, "%s/%s", cd_dir, seedfile);
        
    count++;

    /* testa concatenando com variavel de ambiente */

    if((seed = fopen (newfilename, "rb"))==NULL)
    { 
      /* tenta abrir usando string passada pelo usuario
         diretamente */

      if((seed = fopen (seedfile, "rb"))==NULL)
      { 
        fclose(new_dgn->arq_ptr);
        free(new_dgn);
        return;
      }
    }

    /* concatena seed */  

    fseek(seed, 0, SEEK_SET);
    fseek(new_dgn->arq_ptr, 0, SEEK_SET);

    new_dgn->bytes=0;
    dgn_copy(seed, new_dgn);
    fclose(seed);
  }
  
  new_dgn->cnv = cnv;
  cnv->context_canvas = new_dgn;

  /* config */

  new_dgn->level = 1;

  /** valores default do contexto sao setados **/

  /* texto */

  new_dgn->alignment = 12; 
  new_dgn->is_base = 1;
  new_dgn->typeface = 0;
  new_dgn->tl=12;

  /* cores */

  memset(new_dgn->colortable, 0, 1024); 
  new_dgn->colortable[0] = CD_BLACK;
  new_dgn->num_colors = 1;

  /* atributos */

  new_dgn->color = 1;
  new_dgn->style = 0;

  /* DGN */

  new_dgn->is_complex=0;
}

static void cdinittable(cdPrivateCanvas* cnv)
{
  cnv->Flush = cdflush;
  cnv->Pixel = cdpixel;
  cnv->Line = cdline;
  cnv->Poly = cdpoly;
  cnv->Box = cdbox;
  cnv->Arc = cdarc;
  cnv->Sector = cdsector;
  cnv->Text = cdtext;
  cnv->FontDim = cdfontdim;
  cnv->TextSize = cdtextsize;
  cnv->PutImageRectMap = cdputimagerectmap;

  cnv->LineStyle = cdlinestyle;
  cnv->LineWidth = cdlinewidth;
  cnv->Font = cdfont;
  cnv->NativeFont = cdnativefont;
  cnv->TextAlignment = cdtextalignment;
  cnv->Palette = cdpalette;
  cnv->Foreground = cdforeground;

  cnv->KillCanvas = cdkillcanvas;
  cnv->Activate = cdactivate;
  cnv->Deactivate = cddeactivate;
}

/******************************************************/

static cdPrivateContext cdDGNContextTable =
{
  CD_CAP_ALL & ~(CD_CAP_CLEAR | CD_CAP_PLAY | 
                 CD_CAP_IMAGERGBA | CD_CAP_GETIMAGERGB | 
                 CD_CAP_CLIPAREA | CD_CAP_CLIPPOLY | 
                 CD_CAP_MARK | CD_CAP_RECT | 
                 CD_CAP_LINECAP | CD_CAP_LINEJOIN | CD_CAP_REGION | CD_CAP_CHORD |
                 CD_CAP_IMAGERGB | CD_CAP_IMAGESRV | 
                 CD_CAP_BACKGROUND | CD_CAP_BACKOPACITY | CD_CAP_WRITEMODE | 
                 CD_CAP_HATCH | CD_CAP_STIPPLE | CD_CAP_PATTERN | 
                 CD_CAP_IMAGERGBA | CD_CAP_GETIMAGERGB | 
                 CD_CAP_WD | CD_CAP_VECTORTEXT | CD_CAP_TEXTORIENTATION),
  cdcreatecanvas,
  cdinittable,
  NULL,
  NULL
};


static cdContext cdDGN =
{
  &cdDGNContextTable
};


cdContext* cdContextDGN(void)
{
  return &cdDGN;
}


