/*
 Canvas Draw - CD_IRGB Driver
*/

#include <stdlib.h> 
#include <memory.h> 
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include <cd.h>
#include "cdprivat.h"
#include "cdirgb.h"
#include "sim.h"

/********************/
/* Local Structures */

typedef struct _cdImageRGB 
{
  int w, h;
  unsigned char* red;     /* red color buffer */
  unsigned char* green;   /* green color buffer */
  unsigned char* blue;    /* blue color buffer */
} cdImageRGB;


typedef struct cdCanvasIRGB 
{
  cdPrivateCanvas* cnv;

  int xmin, ymin;         /* Define a regiao de clip, pode ser o clipping ou o canvas todo */
  int xmax, ymax;         

  int user_image;         /* indica se a imagem deve ser liberada ou nao */

  unsigned char* red;     /* red color buffer */
  unsigned char* green;   /* green color buffer */
  unsigned char* blue;    /* blue color buffer */
} cdCanvasIRGB;

cdCanvasIRGB *irgbActiveCanvas = NULL;



/*******************/
/* Local functions */

static int sRound(double x)
{
	return (int)(x + 0.5);
}

static void sCombineRGB(unsigned char *dr, unsigned char *dg, unsigned char *db, unsigned char sr, unsigned char sg, unsigned char sb)
{
  switch (irgbActiveCanvas->cnv->write_mode)
  {
  case CD_REPLACE:
    *dr = sr;
    *dg = sg;
    *db = sb;
    break;
  case CD_XOR:
    *dr ^= sr;
    *dg ^= sg;
    *db ^= sb;
    break;
  case CD_NOT_XOR:
    *dr = ~(sr ^ *dr);
    *dg = ~(sg ^ *dg);
    *db = ~(sb ^ *db);
    break;
  }
}

static void sCombineRGBA(unsigned char *dr, unsigned char *dg, unsigned char *db, unsigned char sr, unsigned char sg, unsigned char sb, unsigned char sa)
{
  switch (irgbActiveCanvas->cnv->write_mode)
  {
  case CD_REPLACE:
    *dr = CD_ALPHA_BLEND(sr, *dr, sa);
    *dg = CD_ALPHA_BLEND(sg, *dg, sa);
    *db = CD_ALPHA_BLEND(sb, *db, sa);
    break;
  case CD_XOR:
    *dr ^= CD_ALPHA_BLEND(sr, *dr, sa);
    *dg ^= CD_ALPHA_BLEND(sg, *dg, sa);
    *db ^= CD_ALPHA_BLEND(sb, *db, sa);
    break;
  case CD_NOT_XOR:
    *dr = ~(CD_ALPHA_BLEND(sr, *dr, sa) ^ *dr);
    *dg = ~(CD_ALPHA_BLEND(sg, *dg, sa) ^ *dg);
    *db = ~(CD_ALPHA_BLEND(sb, *db, sa) ^ *db);
    break;
  }
}
  
static void sCombineRGBLine(unsigned char *dr, unsigned char *dg, unsigned char *db, unsigned char *sr, unsigned char *sg, unsigned char *sb, int size)
{
  int c;

  if (size > 0)
  {
    switch (irgbActiveCanvas->cnv->write_mode)
    {
    case CD_REPLACE:
      for (c = 0; c < size; c++)
      {
        *dr++ = *sr++;
        *dg++ = *sg++;
        *db++ = *sb++;
      }
      break;
    case CD_XOR:
      for (c = 0; c < size; c++)
      {
        *dr++ ^= *sr++;
        *dg++ ^= *sg++;
        *db++ ^= *sb++;
      }
      break;
    case CD_NOT_XOR:
      for (c = 0; c < size; c++)
      {
        *dr++ = ~(*sr++ ^ *dr);
        *dg++ = ~(*sg++ ^ *dg);
        *db++ = ~(*sb++ ^ *db);
      }
      break;
    }
  }
  else
  {
    size *= -1;
    switch (irgbActiveCanvas->cnv->write_mode)
    {
    case CD_REPLACE:
      for (c = 0; c < size; c++)
      {
        *dr-- = *sr--;
        *dg-- = *sg--;
        *db-- = *sb--;
      }
      break;
    case CD_XOR:
      for (c = 0; c < size; c++)
      {
        *dr-- ^= *sr--;
        *dg-- ^= *sg--;
        *db-- ^= *sb--;
      }
      break;
    case CD_NOT_XOR:
      for (c = 0; c < size; c++)
      {
        *dr-- = ~(*sr-- ^ *dr);
        *dg-- = ~(*sg-- ^ *dg);
        *db-- = ~(*sb-- ^ *db);
      }
      break;
    }
  }
}

static void sCombineRGBALine(unsigned char *dr, unsigned char *dg, unsigned char *db, unsigned char *sr, unsigned char *sg, unsigned char *sb, unsigned char *sa, int size)
{
  int c;

  if (size > 0)
  {
    switch (irgbActiveCanvas->cnv->write_mode)
    {
    case CD_REPLACE:
      for (c = 0; c < size; c++)
      {
        *(dr++) = CD_ALPHA_BLEND(*sr, *dr, *sa);
        *(dg++) = CD_ALPHA_BLEND(*sg, *dg, *sa);
        *(db++) = CD_ALPHA_BLEND(*sb, *db, *sa);
        sa++; sr++; sg++; sb++;
      }
      break;
    case CD_XOR:
      for (c = 0; c < size; c++)
      {
        *(dr++) ^= CD_ALPHA_BLEND(*sr, *dr, *sa);
        *(dg++) ^= CD_ALPHA_BLEND(*sg, *dg, *sa);
        *(db++) ^= CD_ALPHA_BLEND(*sb, *db, *sa);
        sa++; sr++; sg++; sb++;
      }
      break;
    case CD_NOT_XOR:
      for (c = 0; c < size; c++)
      {
        *(dr++) = ~(CD_ALPHA_BLEND(*sr, *dr, *sa) ^ *dr);
        *(dg++) = ~(CD_ALPHA_BLEND(*sg, *dg, *sa) ^ *dg);
        *(db++) = ~(CD_ALPHA_BLEND(*sb, *db, *sa) ^ *db);
        sa++; sr++; sg++; sb++;
      }
      break;
    }
  }
  else
  {
    size *= -1;
    switch (irgbActiveCanvas->cnv->write_mode)
    {
    case CD_REPLACE:
      for (c = 0; c < size; c++)
      {
        *(dr--) = CD_ALPHA_BLEND(*sr, *dr, *sa);
        *(dg--) = CD_ALPHA_BLEND(*sg, *dg, *sa);
        *(db--) = CD_ALPHA_BLEND(*sb, *db, *sa);
        sa--; sr--; sg--; sb--;
      }
      break;
    case CD_XOR:
      for (c = 0; c < size; c++)
      {
        *(dr--) ^= CD_ALPHA_BLEND(*sr, *dr, *sa);
        *(dg--) ^= CD_ALPHA_BLEND(*sg, *dg, *sa);
        *(db--) ^= CD_ALPHA_BLEND(*sb, *db, *sa);
        sa--; sr--; sg--; sb--;
      }
      break;
    case CD_NOT_XOR:
      for (c = 0; c < size; c++)
      {
        *(dr--) = ~(CD_ALPHA_BLEND(*sr, *dr, *sa) ^ *dr);
        *(dg--) = ~(CD_ALPHA_BLEND(*sg, *dg, *sa) ^ *dg);
        *(db--) = ~(CD_ALPHA_BLEND(*sb, *db, *sa) ^ *db);
        sa--; sr--; sg--; sb--;
      }
      break;
    }
  }
}

/* Verifica se esta dentro do canvas na horizontal */
static int sNormX(int x)
{
  return x < 0? 0: x < irgbActiveCanvas->cnv->w? x: irgbActiveCanvas->cnv->w - 1;
}

/* Verifica se esta dentro do canvas na vertical */
static int sNormY(int y)
{
  return y < 0? 0: y < irgbActiveCanvas->cnv->h? y: irgbActiveCanvas->cnv->h - 1;
}

static void irgbFillLine(int xmin, int y, int xmax, int _unused)
{
  int x;
  unsigned long offset = y * irgbActiveCanvas->cnv->w;
  unsigned char fg_red, fg_green, fg_blue, *r, *g, *b;
  (void)_unused;

  fg_red = cdRed(irgbActiveCanvas->cnv->foreground);
  fg_green = cdGreen(irgbActiveCanvas->cnv->foreground);
  fg_blue = cdBlue(irgbActiveCanvas->cnv->foreground); 

  r = irgbActiveCanvas->red + offset;
  g = irgbActiveCanvas->green + offset;
  b = irgbActiveCanvas->blue + offset;

  if (y < irgbActiveCanvas->ymin)
    return;

  if (y > irgbActiveCanvas->ymax)
    return;

  if (xmin < irgbActiveCanvas->xmin)  /* Arruma limites de acordo com o retangulo de clip */
    xmin = irgbActiveCanvas->xmin;    /* so clipa em x                                    */
  if (xmax > irgbActiveCanvas->xmax)
    xmax = irgbActiveCanvas->xmax;

  for (x = xmin; x <= xmax; x++)
    sCombineRGB(r + x, g + x, b + x, fg_red, fg_green, fg_blue);
}

static void irgbPatternLine(int xmin, int xmax, int y, int pw, long *pattern)
{
  int x,i;
  unsigned long offset = y * irgbActiveCanvas->cnv->w;
  unsigned char *r, *g, *b;

  r = irgbActiveCanvas->red + offset;
  g = irgbActiveCanvas->green + offset;
  b = irgbActiveCanvas->blue + offset;

  if (y < irgbActiveCanvas->ymin || y > irgbActiveCanvas->ymax)
    return;
  
  if (xmin < irgbActiveCanvas->xmin)  /* Arruma limites de acordo com o retangulo de clip */
    xmin = irgbActiveCanvas->xmin;    /* so clipa em x                                    */
  if (xmax > irgbActiveCanvas->xmax)
    xmax = irgbActiveCanvas->xmax;

  i = xmin % pw;

  for (x = xmin; x <= xmax; x++,i++)
  {
    if (i == pw) 
      i = 0;
    sCombineRGB(r + x, g + x, b + x, cdRed(pattern[i]), cdGreen(pattern[i]), cdBlue (pattern[i]));
  }
}

static void irgbStippleLine(int xmin, int xmax, int y, int pw, unsigned char *pattern)
{
  int x,i;
  unsigned long offset = y * irgbActiveCanvas->cnv->w;
  unsigned char fg_red, fg_green, fg_blue, *r, *g, *b;
  unsigned char bg_red, bg_green, bg_blue;

  fg_red = cdRed(irgbActiveCanvas->cnv->foreground);
  fg_green = cdGreen(irgbActiveCanvas->cnv->foreground);
  fg_blue = cdBlue(irgbActiveCanvas->cnv->foreground); 
  bg_red = cdRed(irgbActiveCanvas->cnv->background);
  bg_green = cdGreen(irgbActiveCanvas->cnv->background);
  bg_blue = cdBlue(irgbActiveCanvas->cnv->background); 

  r = irgbActiveCanvas->red + offset;
  g = irgbActiveCanvas->green + offset;
  b = irgbActiveCanvas->blue + offset;

  if (y < irgbActiveCanvas->ymin || y > irgbActiveCanvas->ymax)
    return;

  if (xmin < irgbActiveCanvas->xmin)  /* Arruma limites de acordo com o retangulo de clip */
    xmin = irgbActiveCanvas->xmin;    /* so clipa em x                                    */
  if (xmax > irgbActiveCanvas->xmax)
    xmax = irgbActiveCanvas->xmax;

  i = xmin % pw;

  for (x = xmin; x <= xmax; x++,i++)
  {
    if (i == pw) 
      i = 0;
    if(pattern[i])
      sCombineRGB(r + x, g + x, b + x, fg_red, fg_green, fg_blue);
    else if (irgbActiveCanvas->cnv->back_opacity == CD_OPAQUE)
      sCombineRGB(r + x, g + x, b + x, bg_red, bg_green, bg_blue);
  }
}

static void irgbHatchLine(int xmin, int xmax, int y, unsigned char pattern)
{
  int x;
  unsigned long offset = y * irgbActiveCanvas->cnv->w;
  unsigned char fg_red, fg_green, fg_blue, *r, *g, *b;
  unsigned char bg_red, bg_green, bg_blue;

  fg_red = cdRed(irgbActiveCanvas->cnv->foreground);
  fg_green = cdGreen(irgbActiveCanvas->cnv->foreground);
  fg_blue = cdBlue(irgbActiveCanvas->cnv->foreground); 
  bg_red = cdRed(irgbActiveCanvas->cnv->background);
  bg_green = cdGreen(irgbActiveCanvas->cnv->background);
  bg_blue = cdBlue(irgbActiveCanvas->cnv->background); 

  r = irgbActiveCanvas->red + offset;
  g = irgbActiveCanvas->green + offset;
  b = irgbActiveCanvas->blue + offset;
  
  if (y < irgbActiveCanvas->ymin || y > irgbActiveCanvas->ymax)
    return;

  if (xmin < irgbActiveCanvas->xmin)  /* Arruma limites de acordo com o retangulo de clip */
    xmin = irgbActiveCanvas->xmin;    /* so clipa em x                                    */
  if (xmax > irgbActiveCanvas->xmax)
    xmax = irgbActiveCanvas->xmax;

  RotatenL(pattern,xmin&7);

  for (x = xmin; x <= xmax; x++)
  {
    if (pattern & 0x80)
      sCombineRGB(r + x, g + x, b + x, fg_red, fg_green, fg_blue);
    else if (irgbActiveCanvas->cnv->back_opacity == CD_OPAQUE)
      sCombineRGB(r + x, g + x, b + x, bg_red, bg_green, bg_blue);

    RotateL(pattern);
  }
}

/****************/
/* CD functions */

static void cdkillcanvas(void *canvas)
{
  cdCanvasIRGB *cnv = (cdCanvasIRGB *)canvas;

  if (!cnv->user_image)
  {
    free(cnv->red);
    free(cnv->green);
    free(cnv->blue);
  }

  free(cnv);
}

static int cdactivate(void *data)
{
  irgbActiveCanvas = (cdCanvasIRGB*)data;
  return CD_OK;
}

static void cddeactivate(void)
{
  irgbActiveCanvas = NULL;
}

unsigned char* cdRedImage(cdCanvas* canvas)
{
  cdPrivateCanvas* private_canvas;

  assert(canvas);
  assert(canvas->cnv);

  private_canvas = (cdPrivateCanvas*)canvas->cnv;
  irgbActiveCanvas = (cdCanvasIRGB*)private_canvas->context_canvas;

  return irgbActiveCanvas->red;
}

unsigned char* cdGreenImage(cdCanvas* canvas)
{
  cdPrivateCanvas* private_canvas;

  assert(canvas);
  assert(canvas->cnv);

  private_canvas = (cdPrivateCanvas*)canvas->cnv;
  irgbActiveCanvas = (cdCanvasIRGB*)private_canvas->context_canvas;

  return irgbActiveCanvas->green;
}

unsigned char* cdBlueImage(cdCanvas* canvas)
{
  cdPrivateCanvas* private_canvas;

  assert(canvas);
  assert(canvas->cnv);

  private_canvas = (cdPrivateCanvas*)canvas->cnv;
  irgbActiveCanvas = (cdCanvasIRGB*)private_canvas->context_canvas;

  return irgbActiveCanvas->blue;
}

static void cdclear(void)
{
  int size = irgbActiveCanvas->cnv->w * irgbActiveCanvas->cnv->h; 
  memset(irgbActiveCanvas->red, cdRed(irgbActiveCanvas->cnv->background), size);
  memset(irgbActiveCanvas->green, cdGreen(irgbActiveCanvas->cnv->background), size);
  memset(irgbActiveCanvas->blue, cdBlue(irgbActiveCanvas->cnv->background), size);
}

static int cdclip(int mode)
{
  if (mode == CD_CLIPAREA) 
  {
    irgbActiveCanvas->xmin = irgbActiveCanvas->cnv->clip_xmin;
    irgbActiveCanvas->ymin = irgbActiveCanvas->cnv->clip_ymin;
    irgbActiveCanvas->xmax = irgbActiveCanvas->cnv->clip_xmax;
    irgbActiveCanvas->ymax = irgbActiveCanvas->cnv->clip_ymax;
  }
  else
  {
    irgbActiveCanvas->xmin = 0;
    irgbActiveCanvas->ymin = 0;
    irgbActiveCanvas->xmax = irgbActiveCanvas->cnv->w - 1;
    irgbActiveCanvas->ymax = irgbActiveCanvas->cnv->h - 1;
  }

  return mode;
}

static void cdcliparea(int *xmin, int *xmax, int *ymin, int *ymax)
{                
  *xmin = sNormX(*xmin);
  *ymin = sNormY(*ymin);
  *xmax = sNormX(*xmax);
  *ymax = sNormY(*ymax);

  if (irgbActiveCanvas->cnv->clip_mode == CD_CLIPAREA)
  {
    irgbActiveCanvas->xmin = *xmin;
    irgbActiveCanvas->ymin = *ymin;
    irgbActiveCanvas->xmax = *xmax;
    irgbActiveCanvas->ymax = *ymax;
  }
}

static void cdgetimagergb(unsigned char *r, unsigned char *g, unsigned char *b, int x, int y, int w, int h)
{
  int dst_offset, src_offset, l, xsize, ysize, xpos, ypos;
  unsigned char *src_red, *src_green, *src_blue;

  if (x >= irgbActiveCanvas->cnv->w || y >= irgbActiveCanvas->cnv->h || 
      x + w < 0 || y + h < 0)
    return;

  /* ajusta parametros de entrada */
  xpos = sNormX(x);
  ypos = sNormY(y);

  xsize = w < (irgbActiveCanvas->cnv->w - xpos)? w: irgbActiveCanvas->cnv->w - xpos;
  ysize = h < (irgbActiveCanvas->cnv->h - ypos)? h: irgbActiveCanvas->cnv->h - ypos;

  /* ajusta posicao inicial em source */
  src_offset = xpos + ypos * irgbActiveCanvas->cnv->w;
  src_red = irgbActiveCanvas->red + src_offset;
  src_green = irgbActiveCanvas->green + src_offset;
  src_blue = irgbActiveCanvas->blue + src_offset;

  /* offset para source */
  src_offset = irgbActiveCanvas->cnv->w;

  /* ajusta posicao inicial em destine */
  dst_offset = (xpos - x) + (ypos - y) * w;
	r += dst_offset;
	g += dst_offset;
	b += dst_offset;

  for (l = 0; l < ysize; l++)
  {
    memcpy(r, src_red, xsize);
    memcpy(g, src_green, xsize);
    memcpy(b, src_blue, xsize);

    src_red += src_offset;
		src_green += src_offset;
		src_blue += src_offset;

		r += w;
		g += w;
		b += w;
  }
}

static void cdputimagerectrgb(int iw, int ih, unsigned char *r, unsigned char *g, unsigned char *b, int x, int y, int w, int h, int xmin, int xmax, int ymin, int ymax)
{
  int l, c, xsize, ysize, xpos, ypos, src_offset, dst_offset, rh, rw, img_topdown = 0;
  unsigned char *dst_red, *dst_green, *dst_blue, *src_red, *src_green, *src_blue;

  if (h < 0)
  {
    h = -h;
    y -= (h - 1);    /* y is at top-left, move it to bottom-left */
    img_topdown = 1; /* image pointer will start at top-left     */
  }

  /* verifica se esta dentro da area de desenho */
  if (x > irgbActiveCanvas->xmax || y > irgbActiveCanvas->ymax || 
      (x+w) < irgbActiveCanvas->xmin || (y+h) < irgbActiveCanvas->ymin)
    return;

  xpos = x < irgbActiveCanvas->xmin? irgbActiveCanvas->xmin: x;
  ypos = y < irgbActiveCanvas->ymin? irgbActiveCanvas->ymin: y;

  xsize = (x+w) < irgbActiveCanvas->xmax+1? (x+w) - xpos: irgbActiveCanvas->xmax - xpos + 1;
  ysize = (y+h) < irgbActiveCanvas->ymax+1? (y+h) - ypos: irgbActiveCanvas->ymax - ypos + 1;

  rw = xmax-xmin+1;
  rh = ymax-ymin+1;

  /* testa se tem que fazer zoom */
  if (rw != w || rh != h)
  {
    double factor;
	  int* XTab = (int*)malloc(w*sizeof(int));
	  int* YTab = (int*)malloc(h*sizeof(int));

    /* cria tabelas de conversao de colunas e linhas para acelerar o processo de zoom */
	  factor = (double)iw / (double)w;
	  for(c = 0; c < w; c++)
		  XTab[c] = sRound(factor * c) + xmin;

	  factor = (double)ih / (double)h;
	  for(l = 0; l < h; l++)
		  YTab[l] = sRound(factor * l) + ymin;

    /* ajusta posicao inicial em source */
    if (img_topdown)
      src_offset = YTab[(ih - 1) - (ypos - y)] * iw;
    else
      src_offset = YTab[ypos - y] * iw;

		src_red = r + src_offset;
		src_green = g + src_offset;
		src_blue = b + src_offset;

    /* ajusta posicao inicial em destine */
    dst_offset = xpos + ypos * irgbActiveCanvas->cnv->w;
    dst_red = irgbActiveCanvas->red + dst_offset;
    dst_green = irgbActiveCanvas->green + dst_offset;
    dst_blue = irgbActiveCanvas->blue + dst_offset;

    /* offset para destine */
    dst_offset = irgbActiveCanvas->cnv->w;
    
	  for(l = 0; l < ysize; l++)
	  {
  	  for(c = 0; c < xsize; c++)
      {
        src_offset = XTab[c + (xpos - x)];
        sCombineRGB(&dst_red[c], &dst_green[c], &dst_blue[c], src_red[src_offset], src_green[src_offset], src_blue[src_offset]);
      }

		  dst_red += dst_offset;
		  dst_green += dst_offset;
		  dst_blue += dst_offset;

      if (img_topdown)
        src_offset = YTab[(ih - 1) - (l + (ypos - y))] * iw;
      else
        src_offset = YTab[l + (ypos - y)] * iw;

		  src_red = r + src_offset;
		  src_green = g + src_offset;
		  src_blue = b + src_offset;
	  }

	  free(XTab);
	  free(YTab);
  }
  else
  {
    /* ajusta posicao inicial em destine */
    dst_offset = xpos + ypos * irgbActiveCanvas->cnv->w;
    dst_red = irgbActiveCanvas->red + dst_offset;
    dst_green = irgbActiveCanvas->green + dst_offset;
    dst_blue = irgbActiveCanvas->blue + dst_offset;

    /* ajusta posicao inicial em source */
    if (img_topdown)
      src_offset = (xpos - x + xmin) + ((ih - 1) - (ypos - y + ymin)) * iw;
    else
      src_offset = (xpos - x + xmin) + (ypos - y + ymin) * iw;

	  r += src_offset;
	  g += src_offset;
	  b += src_offset;

    for (l = 0; l < ysize; l++)
    {
      sCombineRGBLine(dst_red, dst_green, dst_blue, r, g, b, xsize);

      dst_red += irgbActiveCanvas->cnv->w;
		  dst_green += irgbActiveCanvas->cnv->w;
		  dst_blue += irgbActiveCanvas->cnv->w;

      if (img_topdown)
      {
		    r -= iw;
		    g -= iw;
		    b -= iw;
      }
      else
      {
		    r += iw;
		    g += iw;
		    b += iw;
      }
    }
  }
}

static void cdputimagerectrgba(int iw, int ih, unsigned char *r, unsigned char *g, unsigned char *b, unsigned char *a, int x, int y, int w, int h, int xmin, int xmax, int ymin, int ymax)
{
  int l, c, xsize, ysize, xpos, ypos, src_offset, dst_offset, rw, rh, img_topdown = 0;
  unsigned char *dst_red, *dst_green, *dst_blue, *src_red, *src_green, *src_blue, *src_alpha;

  if (h < 0)
  {
    h = -h;
    y -= (h - 1);    /* y is at top-left, move it to bottom-left */
    img_topdown = 1; /* image pointer will start at top-left     */
  }

  /* verifica se esta dentro da area de desenho */
  if (x > irgbActiveCanvas->xmax || y > irgbActiveCanvas->ymax || 
       (x+w) < irgbActiveCanvas->xmin || (y+h) < irgbActiveCanvas->ymin)
    return;

  xpos = x < irgbActiveCanvas->xmin? irgbActiveCanvas->xmin: x;
  ypos = y < irgbActiveCanvas->ymin? irgbActiveCanvas->ymin: y;

  xsize = (x+w) < irgbActiveCanvas->xmax+1? (x+w) - xpos: irgbActiveCanvas->xmax - xpos + 1;
  ysize = (y+h) < irgbActiveCanvas->ymax+1? (y+h) - ypos: irgbActiveCanvas->ymax - ypos + 1;

  rw = xmax-xmin+1;
  rh = ymax-ymin+1;

  /* testa se tem que fazer zoom */
  if (rw != w || rh != h)
  {
    double factor;
	  int* XTab = (int*)malloc(w*sizeof(int));
	  int* YTab = (int*)malloc(h*sizeof(int));

    /* cria tabelas de conversao de colunas e linhas para acelerar o processo de zoom */
	  factor = (double)iw / (double)w;
	  for(c = 0; c < w; c++)
		  XTab[c] = sRound(factor * c) + xmin;

	  factor = (double)ih / (double)h;
	  for(l = 0; l < h; l++)
		  YTab[l] = sRound(factor * l) + ymin;

    /* ajusta posicao inicial em source */
    if (img_topdown)
      src_offset = YTab[(ih - 1) - (ypos - y)] * iw;
    else
      src_offset = YTab[ypos - y] * iw;

	  src_red = r + src_offset;
		src_green = g + src_offset;
		src_blue = b + src_offset;
		src_alpha = a + src_offset;

    /* ajusta posicao inicial em destine */
    dst_offset = xpos + ypos * irgbActiveCanvas->cnv->w;
    dst_red = irgbActiveCanvas->red + dst_offset;
    dst_green = irgbActiveCanvas->green + dst_offset;
    dst_blue = irgbActiveCanvas->blue + dst_offset;

	  for(l = 0; l < ysize; l++)
	  {
  	  for(c = 0; c < xsize; c++)
      {
        src_offset = XTab[c + (xpos - x)];
        sCombineRGBA(&dst_red[c], &dst_green[c], &dst_blue[c], src_red[src_offset], src_green[src_offset], src_blue[src_offset], src_alpha[src_offset]);
      }

		  dst_red += irgbActiveCanvas->cnv->w;
		  dst_green += irgbActiveCanvas->cnv->w;
		  dst_blue += irgbActiveCanvas->cnv->w;

      if (img_topdown)
        src_offset = YTab[(ih - 1) - (l + (ypos - y))] * iw;
      else
        src_offset = YTab[l + (ypos - y)] * iw;

		  src_red = r + src_offset;
		  src_green = g + src_offset;
		  src_blue = b + src_offset;
		  src_alpha = a + src_offset;
	  }

	  free(XTab);
	  free(YTab);
  }
  else
  {
    /* ajusta posicao inicial em destine */
    dst_offset = xpos + ypos * irgbActiveCanvas->cnv->w;
    dst_red = irgbActiveCanvas->red + dst_offset;
    dst_green = irgbActiveCanvas->green + dst_offset;
    dst_blue = irgbActiveCanvas->blue + dst_offset;

    /* ajusta posicao inicial em source */
    if (img_topdown)
      src_offset = (xpos - x + xmin) + ((ih - 1) - (ypos - y + ymin)) * iw;
    else
      src_offset = (xpos - x + xmin) + (ypos - y + ymin) * iw;

	  r += src_offset;
	  g += src_offset;
	  b += src_offset;
	  a += src_offset;

    for (l = 0; l < ysize; l++)
    {
      sCombineRGBALine(dst_red, dst_green, dst_blue, r, g, b, a, xsize);

      dst_red += irgbActiveCanvas->cnv->w;
		  dst_green += irgbActiveCanvas->cnv->w;
		  dst_blue += irgbActiveCanvas->cnv->w;

      if (img_topdown)
      {
		    r -= iw;
		    g -= iw;
		    b -= iw;
		    a -= iw;
      }
      else
      {
		    r += iw;
		    g += iw;
		    b += iw;
		    a += iw;
      }
    }
  }
}

static void cdputimagerectmap(int iw, int ih, unsigned char *index, long int *colors, int x, int y, int w, int h, int xmin, int xmax, int ymin, int ymax)
{
  int l, c, xsize, ysize, xpos, ypos, src_offset, dst_offset, rw, rh, pal_size, idx, img_topdown = 0;
  unsigned char *dst_red, *dst_green, *dst_blue, *src_index, red_map[256], green_map[256], blue_map[256];

  if (h < 0)
  {
    h = -h;
    y -= (h - 1);    /* y is at top-left, move it to bottom-left */
    img_topdown = 1; /* image pointer will start at top-left     */
  }

  /* verifica se esta dentro da area de desenho */
  if (x > irgbActiveCanvas->xmax || y > irgbActiveCanvas->ymax || 
       (x+w) < irgbActiveCanvas->xmin || (y+h) < irgbActiveCanvas->ymin)
    return;

  xpos = x < irgbActiveCanvas->xmin? irgbActiveCanvas->xmin: x;
  ypos = y < irgbActiveCanvas->ymin? irgbActiveCanvas->ymin: y;

  xsize = (x+w) < irgbActiveCanvas->xmax+1? (x+w) - xpos: irgbActiveCanvas->xmax - xpos + 1;
  ysize = (y+h) < irgbActiveCanvas->ymax+1? (y+h) - ypos: irgbActiveCanvas->ymax - ypos + 1;

  rw = xmax-xmin+1;
  rh = ymax-ymin+1;

  /* Como nao sabemos o tamanho da palette a priori, 
  teremos que ver qual o maior indice usado na imagem. */
  pal_size = 0;
  
  for (l=0; l<ih; l++) 
  {
    for (c=0; c<iw; c++) 
    {
      idx = index[l*iw + c];
      if (idx > pal_size)
        pal_size = idx;
    }
  }
  
  pal_size++;

  for (l = 0; l < pal_size; l++)
  {
    red_map[l] = cdRed(colors[l]);
    green_map[l] = cdGreen(colors[l]);
    blue_map[l] = cdBlue(colors[l]);
  }

  /* testa se tem que fazer zoom */
  if (rw != w || rh != h)
  {
    double factor;
	  int* XTab = (int*)malloc(w*sizeof(int));
	  int* YTab = (int*)malloc(h*sizeof(int));

    /* cria tabelas de conversao de colunas e linhas para acelerar o processo de zoom */
	  factor = (double)rw / (double)w;
	  for(c = 0; c < w; c++)
		  XTab[c] = sRound(factor * c) + xmin;

	  factor = (double)rh / (double)h;
	  for(l = 0; l < h; l++)
		  YTab[l] = sRound(factor * l) + ymin;

    /* ajusta posicao inicial em source */
    if (img_topdown)
      src_offset = YTab[(ih - 1) - (ypos - y)] * iw;
    else
      src_offset = YTab[ypos - y] * iw;

		src_index = index + src_offset;

    /* ajusta posicao inicial em destine */
    dst_offset = xpos + ypos * irgbActiveCanvas->cnv->w;
    dst_red = irgbActiveCanvas->red + dst_offset;
    dst_green = irgbActiveCanvas->green + dst_offset;
    dst_blue = irgbActiveCanvas->blue + dst_offset;

	  for(l = 0; l < ysize; l++)
	  {
  	  for(c = 0; c < xsize; c++)
      {
        src_offset = XTab[c + (xpos - x)];
        idx = src_index[src_offset];
        sCombineRGB(&dst_red[c], &dst_green[c], &dst_blue[c], red_map[idx], green_map[idx], blue_map[idx]);
      }

		  dst_red += irgbActiveCanvas->cnv->w;
		  dst_green += irgbActiveCanvas->cnv->w;
		  dst_blue += irgbActiveCanvas->cnv->w;

      if (img_topdown)
        src_offset = YTab[(ih - 1) - (l + (ypos - y))] * iw;
      else
        src_offset = YTab[l + (ypos - y)] * iw;

		  src_index = index + src_offset;
	  }

	  free(XTab);
	  free(YTab);
  }
  else
  {
    /* ajusta posicao inicial em destine */
    dst_offset = xpos + ypos * irgbActiveCanvas->cnv->w;
    dst_red = irgbActiveCanvas->red + dst_offset;
    dst_green = irgbActiveCanvas->green + dst_offset;
    dst_blue = irgbActiveCanvas->blue + dst_offset;

    /* ajusta posicao inicial em source */
    if (img_topdown)
      src_offset = (xpos - x + xmin) + ((ih - 1) - (ypos - y + ymin)) * iw;
    else
      src_offset = (xpos - x + xmin) + (ypos - y + ymin) * iw;

	  index += src_offset;

    for (l = 0; l < ysize; l++)
    {
  	  for(c = 0; c < xsize; c++)
      {
        idx = index[c];
        sCombineRGB(&dst_red[c], &dst_green[c], &dst_blue[c], red_map[idx], green_map[idx], blue_map[idx]);
      }

      dst_red += irgbActiveCanvas->cnv->w;
		  dst_green += irgbActiveCanvas->cnv->w;
		  dst_blue += irgbActiveCanvas->cnv->w;

      if (img_topdown)
		    index -= iw;
      else
		    index += iw;
    }
  }
}

static void cdpixel(int x, int y, long int color)
{
  int offset = irgbActiveCanvas->cnv->w * y + x;

  /* verifica se esta dentro da area de desenho */
  if (x < irgbActiveCanvas->xmin ||
      x > irgbActiveCanvas->xmax ||
      y < irgbActiveCanvas->ymin ||
      y > irgbActiveCanvas->ymax)
  return;

  sCombineRGB(irgbActiveCanvas->red + offset, irgbActiveCanvas->green + offset, irgbActiveCanvas->blue + offset, cdRed(color), cdGreen(color), cdBlue(color));
}

static void* cdcreateimage(int w, int h)
{
  cdImageRGB* Image;
  int t = w * h;

  Image = (cdImageRGB*) malloc(sizeof(cdImageRGB));

  Image->w = w;
  Image->h = h;

  Image->red = (unsigned char*) malloc(t);
  if (!Image->red)
  {
    free(Image);
    return NULL;
  }

  Image->green = (unsigned char*) malloc(t);
  if (!Image->green)
  {
    free(Image->red);
    free(Image);
    return NULL;
  }

  Image->blue = (unsigned char*) malloc(t);
  if (!Image->blue)
  {
    free(Image->red);
    free(Image->green);
    free(Image);
    return NULL;
  }

  memset(Image->red, 0xFF, t);
  memset(Image->green, 0xFF, t);
  memset(Image->blue, 0xFF, t);

  return Image;
}

static void cdgetimage(void* image, int x, int y)
{
  cdImageRGB* Image = (cdImageRGB*)image;
  cdGetImageRGB(Image->red, Image->green, Image->blue, x, y, Image->w, Image->h);
}

static void cdputimagerect(void* image, int x, int y, int xmin, int xmax, int ymin, int ymax)
{
  int iw, ih, w, h;
  unsigned char *r, *g, *b;
  int l, xsize, ysize, xpos, ypos, src_offset, dst_offset;
  unsigned char *dst_red, *dst_green, *dst_blue;

  cdImageRGB* Image = (cdImageRGB*)image;

  iw = Image->w;
  ih = Image->h;

  r = Image->red;
  g = Image->green;
  b = Image->blue;

  w = xmax-xmin+1;
  h = ymax-ymin+1;

  /* verifica se esta dentro da area de desenho */
  if (x > irgbActiveCanvas->xmax || y > irgbActiveCanvas->ymax || 
      x + w < irgbActiveCanvas->xmin || y + h < irgbActiveCanvas->ymin)
    return;

  xpos = x;
  ypos = y;

  if (ypos < irgbActiveCanvas->ymin) ypos = irgbActiveCanvas->ymin;
  if (xpos < irgbActiveCanvas->xmin) xpos = irgbActiveCanvas->xmin;

  xsize = w < (irgbActiveCanvas->xmax+1 - xpos)? w: (irgbActiveCanvas->xmax+1 - xpos);
  ysize = h < (irgbActiveCanvas->ymax+1 - ypos)? h: (irgbActiveCanvas->ymax+1 - ypos);

  /* ajusta posicao inicial em destine */
  dst_offset = xpos + ypos * irgbActiveCanvas->cnv->w;
  dst_red = irgbActiveCanvas->red + dst_offset;
  dst_green = irgbActiveCanvas->green + dst_offset;
  dst_blue = irgbActiveCanvas->blue + dst_offset;

  /* ajusta posicao inicial em source */
  src_offset = ((xpos - x) + xmin) + ((ypos - y) + ymin) * iw;
	r += src_offset;
	g += src_offset;
	b += src_offset;

  for (l = 0; l < ysize; l++)
  {
    sCombineRGBLine(dst_red, dst_green, dst_blue, r, g, b, xsize);

    dst_red += irgbActiveCanvas->cnv->w;
		dst_green += irgbActiveCanvas->cnv->w;
		dst_blue += irgbActiveCanvas->cnv->w;

		r += iw;
		g += iw;
		b += iw;
  }
}

static void cdkillimage(void* image)
{
  cdImageRGB* Image = (cdImageRGB*)image;
  free(Image->red);
  free(Image->green);
  free(Image->blue);
  free(Image);
}

static void cdscrollarea(int xmin, int xmax, int ymin, int ymax, int dx, int dy)
{
  int l;
  long src_offset, dst_offset;
  int incx,incy, xsize, ysize;
  int dst_xmin, dst_xmax, dst_ymin, dst_ymax;

  /* corrige valores de entrada */

  xmin = sNormX(xmin);
  ymin = sNormY(ymin);
  xmax = sNormX(xmax);
  ymax = sNormY(ymax);

  dst_xmin = xmin + dx;
  dst_ymin = ymin + dy;
  dst_xmax = xmax + dx;
  dst_ymax = ymax + dy;

  /* verifica se esta dentro da area de desenho */
  if (dst_xmin > irgbActiveCanvas->xmax || dst_ymin > irgbActiveCanvas->ymax || 
      dst_xmax < irgbActiveCanvas->xmin || dst_ymax < irgbActiveCanvas->ymin)
    return;

  if (dst_ymin < irgbActiveCanvas->ymin) dst_ymin = irgbActiveCanvas->ymin;
  if (dst_xmin < irgbActiveCanvas->xmin) dst_xmin = irgbActiveCanvas->xmin;

  if (dst_ymax > irgbActiveCanvas->ymax) dst_ymax = irgbActiveCanvas->ymax;
  if (dst_xmax > irgbActiveCanvas->xmax) dst_xmax = irgbActiveCanvas->xmax;

  if (dst_xmin > dst_xmax || dst_ymin > dst_ymax)
    return;

  /* Decide de onde vai comecar a copiar, isto e' necessario pois pode haver 
     uma intersecao entre a imagem original e a nova imagem, assim devo 
     garantir que nao estou colocando um ponto, em cima de um ponto ainda nao
     lido da imagem antiga. */

  xsize = dst_xmax - dst_xmin + 1;
  ysize = dst_ymax - dst_ymin + 1;

  /* sentido de copia da direita para a esquerda ou ao contrario. */
  if (dx < 0)
  {
    incx = 1;
    dst_offset = dst_xmin;
    src_offset = xmin;
  }
  else
  {
    incx = -1;
    dst_offset = dst_xmax;
    src_offset = xmax;
  }

  /* sentido de copia de cima para baixo ou ao contrario. */
  if (dy < 0)
  {
    incy = irgbActiveCanvas->cnv->w;
    dst_offset += dst_ymin * irgbActiveCanvas->cnv->w;
    src_offset += ymin * irgbActiveCanvas->cnv->w;
  }
  else
  {
    incy = -(irgbActiveCanvas->cnv->w);
    dst_offset += dst_ymax * irgbActiveCanvas->cnv->w;
    src_offset += ymax * irgbActiveCanvas->cnv->w;
  }

  xsize *= incx;

  for (l = 0; l < ysize; l++)
  {
    sCombineRGBLine(irgbActiveCanvas->red + dst_offset, irgbActiveCanvas->green + dst_offset, irgbActiveCanvas->blue + dst_offset, irgbActiveCanvas->red + src_offset, irgbActiveCanvas->green + src_offset, irgbActiveCanvas->blue + src_offset, xsize);
    dst_offset += incy;
    src_offset += incy;
  }
}

static char* get_green_attrib(void)
{
  return (char*)irgbActiveCanvas->green;
}

static cdAttribute green_attrib =
{
  "GREENIMAGE",
  NULL,
  get_green_attrib
}; 

static char* get_blue_attrib(void)
{
  return (char*)irgbActiveCanvas->blue;
}

static cdAttribute blue_attrib =
{
  "BLUEIMAGE",
  NULL,
  get_blue_attrib
}; 

static char* get_red_attrib(void)
{
  return (char*)irgbActiveCanvas->red;
}

static cdAttribute red_attrib =
{
  "REDIMAGE",
  NULL,
  get_red_attrib
}; 

static void cdcreatecanvas(cdPrivateCanvas* cnv, void *data)
{
  cdCanvasIRGB* new_irgb;
  int w = 0, h = 0;
  float res = (float)3.78;
  unsigned char *r = NULL, *g = NULL, *b = NULL;
  char* str_data = (char*)data;

  /* resolution and rgb */
#ifdef SunOS_OLD
    sscanf(str_data, "%dx%d %d %d %d", &w, &h, &r, &g, &b);
#else
    sscanf(str_data, "%dx%d %p %p %p", &w, &h, &r, &g, &b);
#endif

  if (w == 0 || h == 0)
    return;

  {
    char *str = strstr(str_data, "-r");
    if (str != NULL)
    {
      str += 2;
      sscanf(str, "%g", &res);
    }
  }

  new_irgb = (cdCanvasIRGB *)malloc(sizeof(cdCanvasIRGB));

  cnv->w = w;
  cnv->h = h;
  cnv->yres = res;
  cnv->xres = res;
  cnv->w_mm = ((double)w) / res;
  cnv->h_mm = ((double)h) / res;
  cnv->bpp = 24;

  if (r && g && b)
  {
    new_irgb->user_image = 1;

    new_irgb->red = r;
    new_irgb->green = g;
    new_irgb->blue = b;
  }
  else
  {
    int size = w * h;

    new_irgb->user_image = 0;

    new_irgb->red = (unsigned char*)malloc(size);
    new_irgb->green = (unsigned char*)malloc(size);
    new_irgb->blue = (unsigned char*)malloc(size);

    if (!new_irgb->red || !new_irgb->green || !new_irgb->blue)
    {
      if (!new_irgb->red) free(new_irgb->red);
      if (!new_irgb->green) free(new_irgb->green);
      if (!new_irgb->blue) free(new_irgb->blue);
      free(new_irgb);
      return;
    }

    memset(new_irgb->red, 0xFF, size);
    memset(new_irgb->green, 0xFF, size);
    memset(new_irgb->blue, 0xFF, size);
  }

  new_irgb->xmin = 0;
  new_irgb->ymin = 0;
  new_irgb->xmax = cnv->w - 1;
  new_irgb->ymax = cnv->h - 1;

  cnv->context_canvas = new_irgb;
  new_irgb->cnv = cnv;

  cdSimInitText(cnv->sim_data); 
  /* nao preciso inicializar a fonte,
     pois isso sera' faito na inicializacao dos atributos default do driver */

  cdRegisterAttribute(cnv, &red_attrib);
  cdRegisterAttribute(cnv, &green_attrib);
  cdRegisterAttribute(cnv, &blue_attrib);
}

static void cdinittable(cdPrivateCanvas* cnv)
{
  cdPrivateSimulation* sim;

  /* initialize function table*/
  cnv->Clear = cdclear;
  cnv->Pixel = cdpixel;
  cnv->PutImageRectRGB = cdputimagerectrgb;
  cnv->PutImageRectRGBA = cdputimagerectrgba;
  cnv->PutImageRectMap = cdputimagerectmap;
  cnv->ScrollArea = cdscrollarea;

  cnv->Clip = cdclip;
  cnv->ClipArea = cdcliparea;
  cnv->GetImageRGB = cdgetimagergb;
  cnv->CreateImage = cdcreateimage;
  cnv->GetImage = cdgetimage; 
  cnv->PutImageRect = cdputimagerect;
  cnv->KillImage = cdkillimage; 

  cnv->KillCanvas = cdkillcanvas;
  cnv->Activate = cdactivate;
  cnv->Deactivate = cddeactivate;

  /* force simulation */
  cnv->Line = cdSimLine;
  cnv->Poly = cdSimPoly;
  cnv->Box = cdSimBox;
  cnv->Arc = cdSimArc;
  cnv->Sector = cdSimSector;
  cnv->Chord = cdSimChord;
  cnv->Text = cdSimText;

  cnv->NativeFont = cdSimNativeFont;
  cnv->Font = cdSimFont;
  cnv->FontDim = cdSimFontDim;
  cnv->TextSize = cdSimTextSize;

  sim = (cdPrivateSimulation*)cnv->sim_data;

  sim->FillLine    = irgbFillLine;
  sim->PatternLine = irgbPatternLine; 
  sim->StippleLine = irgbStippleLine; 
  sim->HatchLine   = irgbHatchLine;   
}

static cdPrivateContext cdImageRGBContextTable =
{
  CD_CAP_ALL & ~(CD_CAP_FLUSH | CD_CAP_PLAY | 
                 CD_CAP_LINECAP | CD_CAP_LINEJOIN | CD_CAP_REGION |
                 CD_CAP_PALETTE | CD_CAP_TEXTORIENTATION),
  cdcreatecanvas,
  cdinittable,
  NULL,
  NULL
};
 
static cdContext cdIRGB =
{
  &cdImageRGBContextTable
};

cdContext* cdContextImageRGB(void)
{
  return &cdIRGB;
}

