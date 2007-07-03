#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <memory.h>

#include "sim.h"
#include "cdfontex.h"


#define CalcYPat(y, h) (active_sim->cnv->invert_yaxis? h-1-(y%h): y%h)
#define CalcYHatch(y, h) (active_sim->cnv->invert_yaxis? h-(y&h): y&h)
#define f_Round_i(_x) (int)(floor(_x+0.5))
#define truth(x) ((x)?1:0)         
#define mask(x)         (1 << ((~(x))&7))
#define PX 1
#define MX 2
#define PY 4
#define MY 8

#ifndef PI
#define PI 3.14159265358979323846
#endif

static unsigned char SimHatchBits[6][8] = {            /* [style][y] (8x8) */
     {0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0x00}, /* CD_HORIZONTAL */
     {0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10}, /* CD_VERTICAL   */
     {0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80}, /* CD_BDIAGONAL  */
     {0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01}, /* CD_FDIAGONAL  */
     {0x10, 0x10, 0x10, 0xFF, 0x10, 0x10, 0x10, 0x10}, /* CD_CROSS      */
     {0x81, 0x42, 0x24, 0x18, 0x18, 0x24, 0x42, 0x81}};/* CD_DIAGCROSS  */

/* macros do cdSimArc */
#define arcPixel(_x, _y) \
  {\
	  if(active_sim->cnv->clip_mode == CD_CLIPOFF)                                      \
      active_sim->cnv->Pixel(_x, _y, secolor);                                      \
	  else if(active_sim->cnv->clip_mode == CD_CLIPAREA && cdSimClipPointInBox(_x, _y) == 0) \
      active_sim->cnv->Pixel(_x, _y, secolor);                                      \
	  else if((active_sim->cnv->clip_mode == CD_CLIPPOLYGON) && cdSimClipPointInPoly(_x, _y) == 1) \
      active_sim->cnv->Pixel(_x, _y, secolor);                                      \
  }

#define in(a,b) ((big ^ (a > 0 && b < 0)) || (!a && !b))

/* plota os quatro pontos simetricos */  
#define Four() \
  if (in(ta1,ta2)) arcPixel(px1, py1); \
  if (in(tc1,tc2)) arcPixel(px2, py2); \
  if (in(tb1,tb2)) arcPixel(px2, py1); \
  if (in(td1,td2)) arcPixel(px1, py2);
#define Two() \
  if (in(ta1,ta2)) arcPixel(px1, py1); \
  if (in(tc1,tc2)) arcPixel(px2, py2); 
#define One() \
  if (in(ta1,ta2)) arcPixel(px1, py1);
#define reduce(a)                              \
  {double t=a/360;                           \
  a=(t>=0)?a-((int)t)*360:a+((int)t+1)*360;}


/* Macros usadas pela funcao cdSimSector.*/
#define LinesX()                                                                        \
  if (big)                                                                              \
  {                                                                                     \
    double t;                                                                           \
    t = r1x;                                                                            \
    r1x = r2x;                                                                          \
    r2x = t;                                                                            \
    t = r1y;                                                                            \
    r1y = r2y;                                                                          \
    r2y = t;                                                                            \
  }                                                                                     \
  if (in(ta1,ta2)&&in(tb1,tb2)) {                  /* Os dois dentro           */       \
    if ((int)r1y != (int)y || (int)r2y != (int)y)  /* Nao intersecta retas     */       \
      simFillLine(px1, py1, px2, py1);                                                  \
    else {                                         /* Intersecta as duas retas */       \
      simFillLine(px1, py1, (int)r2x+xc, py1);                                          \
      simFillLine(px2, py1, (int)r1x+xc, py1);                                          \
    }                                                                                   \
  }                                                                                     \
  else if (in(ta1,ta2))                                                                 \
    simFillLine(px1, py1, (int)r2x+xc, py1);                                            \
  else if (in(tb1,tb2))                                                                 \
    simFillLine(px2, py1, (int)r1x+xc, py1);                                            \
  else if ((int)r1y == (int)y && (int)r2y == (int)y)/* Intersecta as duas retas*/       \
    simFillLine((int)r1x+xc, py1, (int)r2x+xc, py1);/* agora a de baixo        */       \
  if (in(tc1,tc2)&&in(td1,td2)) {                   /* Os dois dentro          */       \
    if ((int)r1y != (int)-y || (int)r2y != (int)-y) /* Nao intersecta retas    */       \
      simFillLine(px1, py2, px2, py2);                                                  \
    else {                                          /* Intersecta as duas retas */      \
      simFillLine(px1, py2, (int)r2x+xc, py2);                                          \
      simFillLine(px2, py2, (int)r1x+xc, py2);                                          \
    }                                                                                   \
  }                                                                                     \
  else if (in(td1,td2))                                                                 \
    simFillLine(px1, py2, (int)r1x+xc, py2);                                            \
  else if (in(tc1,tc2))                                                                 \
    simFillLine(px2, py2, (int)r2x+xc, py2);                                            \
  else if ((int)r1y == (int)-y && (int)r2y == (int)-y)/*Intersecta as duas retas */     \
    simFillLine((int)r1x+xc, py2, (int)r2x+xc, py2);

#define LineX()                                                                         \
  if (in(ta1,ta2))                                                                      \
    simFillLine(xc, yc, px1, yc);                                                       \
  if (in(tb1,tb2))                                                                      \
    simFillLine(xc, yc, px2, yc);  

/* para estilos de linha usando rotacao de bits */
static const int line_styles[5]=
{ 
  0xffff, /* CD_CONTINUOUS  */
  0x0707, /* CD_DASHED      */
  0x1111, /* CD_DOTTED      */
  0x0c3f, /* CD_DASH_DOT    */
  0x111f, /* CD_DASH_DOT_DOT*/
};


/*************************************************************************************/
/*                          CONTROLE                                                 */
/*************************************************************************************/

static void simPatternLine(int xmin, int xmax, int y, int pw, long *pattern);
static void simStippleLine(int xmin, int xmax, int y, int pw, unsigned char *pattern);
static void simHatchLine(int xmin, int xmax, int y, unsigned char pattern);

cdPrivateSimulation* active_sim = NULL;

void* cdCreateSimulation(cdPrivateCanvas* cnv)
{
  cdPrivateSimulation* sim;

  sim = (cdPrivateSimulation*)malloc(sizeof(cdPrivateSimulation));
  memset(sim, 0, sizeof(cdPrivateSimulation));
  sim->cnv = cnv;

  sim->FillLine    = cdSimLine;             
  sim->PatternLine = simPatternLine; 
  sim->StippleLine = simStippleLine; 
  sim->HatchLine   = simHatchLine;   

  return sim;
}

void cdSimInitText(void* sim_data)
{
  cdPrivateSimulation* sim = (cdPrivateSimulation*)sim_data;
  if (!sim->tt_text)
    sim->tt_text = cdTT_create();
}

void cdKillSimulation(void* sim_data)
{
  cdPrivateSimulation* sim = (cdPrivateSimulation*)sim_data;
  if (sim->tt_text) cdTT_free(sim->tt_text);
  free(sim);
}

void cdSimUpdateFillLine(void)
{
  active_sim->FillLine = active_sim->cnv->Line;             
}

void cdActivateSim(cdPrivateCanvas* cnv)
{
  if (cnv)
    active_sim = (cdPrivateSimulation*)cnv->sim_data;
  else
    active_sim = NULL;
}

void cdSimMark(int x, int y)
{
  int oldinteriorstyle = active_sim->cnv->interior_style;
  int oldlinestyle = active_sim->cnv->line_style;
  int oldlinewidth = active_sim->cnv->line_width;
  int size = active_sim->cnv->mark_size;
  int half_size = size/2;
  int bottom = y-half_size;
  int top    = y+half_size;
  int left   = x-half_size;
  int right  = x+half_size;

  if (active_sim->cnv->interior_style != CD_SOLID &&
      (active_sim->cnv->mark_type == CD_CIRCLE ||
       active_sim->cnv->mark_type == CD_BOX ||
       active_sim->cnv->mark_type == CD_DIAMOND))
    cdInteriorStyle(CD_SOLID);

  if (active_sim->cnv->line_style != CD_CONTINUOUS &&
      (active_sim->cnv->mark_type == CD_STAR ||
       active_sim->cnv->mark_type == CD_PLUS ||
       active_sim->cnv->mark_type == CD_X ||
       active_sim->cnv->mark_type == CD_HOLLOW_BOX ||
       active_sim->cnv->mark_type == CD_HOLLOW_CIRCLE ||
       active_sim->cnv->mark_type == CD_HOLLOW_DIAMOND))
    cdLineStyle(CD_CONTINUOUS);

  if (active_sim->cnv->line_width != 1 &&
      (active_sim->cnv->mark_type == CD_STAR ||
       active_sim->cnv->mark_type == CD_PLUS ||
       active_sim->cnv->mark_type == CD_X ||
       active_sim->cnv->mark_type == CD_HOLLOW_BOX ||
       active_sim->cnv->mark_type == CD_HOLLOW_CIRCLE ||
       active_sim->cnv->mark_type == CD_HOLLOW_DIAMOND))
    cdLineWidth(1);

  switch (active_sim->cnv->mark_type)
  {
  case CD_STAR:
    active_sim->cnv->Line(left, bottom, right, top);
    active_sim->cnv->Line(left, top, right, bottom);
    /* continue */
  case CD_PLUS:
    active_sim->cnv->Line(left, y, right, y);
    active_sim->cnv->Line(x, bottom, x, top);
    break;
  case CD_HOLLOW_CIRCLE:
    active_sim->cnv->Arc(x, y, size, size, 0, 360);
    break;
  case CD_HOLLOW_BOX:
    active_sim->cnv->Rect(left, right, bottom, top);
    break;
  case CD_HOLLOW_DIAMOND:
    active_sim->cnv->Line(left, y, x, top);
    active_sim->cnv->Line(x, top, right, y);
    active_sim->cnv->Line(right, y, x, bottom);
    active_sim->cnv->Line(x, bottom, left, y);
    break;
  case CD_X:
    active_sim->cnv->Line(left, bottom, right, top);
    active_sim->cnv->Line(left, top, right, bottom);
    break;
  case CD_CIRCLE:
    active_sim->cnv->Sector(x, y, size, size, 0, 360);
    break;
  case CD_BOX:
    active_sim->cnv->Box(left, right, bottom, top);
    break;
  case CD_DIAMOND:
    {
      cdPoint poly[5];
      poly[0].x = left;
      poly[0].y = y;
      poly[1].x = x;
      poly[1].y = top;
      poly[2].x = right;
      poly[2].y = y;
      poly[3].x = x;
      poly[3].y = bottom;
      active_sim->cnv->Poly(CD_FILL, poly, 4);
    }
    break;
  }

  if (active_sim->cnv->interior_style != oldinteriorstyle &&
      (active_sim->cnv->mark_type == CD_CIRCLE ||
       active_sim->cnv->mark_type == CD_BOX ||
       active_sim->cnv->mark_type == CD_DIAMOND))
    cdInteriorStyle(oldinteriorstyle);

  if (active_sim->cnv->line_style != oldlinestyle &&
      (active_sim->cnv->mark_type == CD_STAR ||
       active_sim->cnv->mark_type == CD_PLUS ||
       active_sim->cnv->mark_type == CD_X ||
       active_sim->cnv->mark_type == CD_HOLLOW_BOX ||
       active_sim->cnv->mark_type == CD_HOLLOW_CIRCLE ||
       active_sim->cnv->mark_type == CD_HOLLOW_DIAMOND))
    cdLineStyle(oldlinestyle);

  if (active_sim->cnv->line_width != oldlinewidth &&
      (active_sim->cnv->mark_type == CD_STAR ||
       active_sim->cnv->mark_type == CD_PLUS ||
       active_sim->cnv->mark_type == CD_X ||
       active_sim->cnv->mark_type == CD_HOLLOW_BOX ||
       active_sim->cnv->mark_type == CD_HOLLOW_CIRCLE ||
       active_sim->cnv->mark_type == CD_HOLLOW_DIAMOND))
    cdLineWidth(oldlinewidth);
}

void cdSimRect(int xmin, int xmax, int ymin, int ymax)
{
	active_sim->cnv->Line(xmin, ymin, xmin, ymax);
	active_sim->cnv->Line(xmin, ymax, xmax, ymax);
	active_sim->cnv->Line(xmax, ymax, xmax, ymin);
	active_sim->cnv->Line(xmax, ymin, xmin, ymin);
}

void cdSimFontDimEx(int *max_width, int *height, int *ascent, int *descent)
{
  cdFontEx(active_sim->cnv->font_type_face, active_sim->cnv->font_style, active_sim->cnv->font_size);
  cdFontDimEx(max_width, height, ascent, descent);
}

void cdSimTextSizeEx(char *s, int *width, int *height)
{
  cdFontEx(active_sim->cnv->font_type_face, active_sim->cnv->font_style, active_sim->cnv->font_size);
  cdTextSizeEx(s, width, height);
}

/*************************************************************************************/
/*                          PRIMITIVAS (private)                                     */
/*************************************************************************************/

static void ordena(int *xx,int left, int right)
{
  int i,j;
  int a;
  
  i = left;
  j = right;
  a = xx[(left + right)/2];
  do
  {
    while (xx[i] < a && i < right) i++;
    while (a < xx[j] && j > left)  j--;
    if (i<=j)
    {
      int b = xx[i];
      xx[i] = xx[j];
      xx[j] = b;
      i++;j--;
    }
  } while (i<=j);
  if (left < j)  ordena(xx,left,j);
  if (i < right) ordena(xx,i,right);
}

static void simFillLine(int _flxmin,int _fly,int _flxmax,int _)
{                                                                                      
  if(_flxmin>_flxmax)                                                                    
  {                                                                                    
    int t;                                                                             
    t=_flxmin;                                                                          
    _flxmin=_flxmax;                                                                     
    _flxmax=t;                                                                          
  }

  switch(active_sim->cnv->clip_mode)
  {
  case CD_CLIPPOLYGON:
    {
      cdSimClipLineInPoly(_flxmin,_fly,_flxmax,_fly);
      break;
    }
  case CD_CLIPAREA:
    {
      if(_flxmin> (active_sim->cnv->clip_xmax)) 
        return;
      if(_flxmax< (active_sim->cnv->clip_xmin)) 
        return;
      if(_fly < (active_sim->cnv->clip_ymin)) 
        return;
      if(_fly > (active_sim->cnv->clip_ymax)) 
        return;
      
      _flxmin = (_flxmin< (active_sim->cnv->clip_xmin)) ? active_sim->cnv->clip_xmin : _flxmin; 
      _flxmax = (_flxmax> (active_sim->cnv->clip_xmax)) ? active_sim->cnv->clip_xmax : _flxmax; 
      /*continua*/
    }
  default:
    {
      int clip_mode = cdClip(CD_CLIPOFF);
      simDoFillLine(_flxmin, _fly, _flxmax,_fly);
      cdClip(clip_mode);
    }
  }
}
                   
void simDoFillLine(int _flxmin,int _flymin,int _flxmax,int _flymax)
{
  switch(active_sim->cnv->interior_style)                                             
  {                                                                                    
  case CD_SOLID:                                                                     
    active_sim->FillLine(_flxmin,_flymin,_flxmax,_flymax);  
    break;
  case CD_PATTERN:                                      
    
    active_sim->PatternLine(_flxmin,_flxmax,_flymin,active_sim->cnv->pattern_w,
                            active_sim->cnv->pattern +                                              
                            active_sim->cnv->pattern_w*CalcYPat(_flymin, active_sim->cnv->pattern_h));
    break;                                                                           
  case CD_HATCH:                                                                     
    active_sim->HatchLine(_flxmin,_flxmax,_flymin,SimHatchBits[active_sim->cnv->hatch_style][CalcYHatch(_flymin, 7)]);
    break;                                                                           
  case CD_STIPPLE:                                                                   
    active_sim->StippleLine(_flxmin, _flxmax, _flymin,                                                               
                            active_sim->cnv->stipple_w,                                        
                            active_sim->cnv->stipple +                                    
                            active_sim->cnv->stipple_w*CalcYPat(_flymin, active_sim->cnv->stipple_h));
  }
}

static void simPatternLine(int xmin, int xmax, int y, int pw, long *pattern)
{
  int x,i;
  int xb;
  long curColor, old_color;
  
  i = xmin % pw;
  
  old_color = cdForeground(CD_QUERY);
  
  for (x = xmin; x <= xmax;)
  {
    if (i == pw) 
      i = 0;
    
    curColor=pattern[i];
    xb=x;
    
    while(pattern[i]==curColor && (x <= xmax))
    {
      i++;
      if (i == pw) 
        i = 0;
      x++;
    }
    
    if(xb==x-1)
      active_sim->cnv->Pixel(xb,y,curColor);
    else
    {
      cdForeground(curColor);
      active_sim->FillLine(xb,y,x-1,y);  
    }
  }
  
  cdForeground(old_color);
}

static void simStippleLine(int xmin, int xmax, int y, int pw, unsigned char *pattern)
{
  int x,xb,i;
  long fgColor,bgColor;
  int opacity;
  int curCase;
  
  fgColor=cdForeground(CD_QUERY);
  opacity=cdBackOpacity(CD_QUERY);
  
  if(opacity==CD_OPAQUE)
  {
    bgColor=cdBackground(CD_QUERY);
    cdForeground(fgColor);
    for (x = xmin, i=xmin%pw ; x <= xmax;)
    {
      if(i==pw) 
        i=0;
      xb=x;
      curCase=pattern[i];
      while (pattern[i]==curCase && (x<=xmax))
      {
        x++;
        i++;
        if(i==pw) 
          i=0;
      }
      if (curCase) 
      {
        if(xb==x-1)
          active_sim->cnv->Pixel(xb,y,fgColor);
        else
          active_sim->FillLine(xb,y,x-1,y);  
      }
    }
    cdForeground(bgColor);
    for (x = xmin, i=xmin%pw ; x <= xmax;)
    {
      if(i==pw) 
        i=0;
      xb=x;
      curCase=pattern[i];
      while (pattern[i]==curCase && (x<=xmax))
      {
        x++;
        i++;
        if(i==pw) 
          i=0;
      }
      if (!curCase)
      {
        if(xb==x-1)
          active_sim->cnv->Pixel(xb,y,bgColor);
        else
          active_sim->FillLine(xb,y,x-1,y);  
      }
    }
  } 
  else
  {
    cdForeground(fgColor);
    for (x = xmin,i=xmin%pw; x <= xmax;)
    {        
      xb=x;
	     curCase=pattern[i];
       while (pattern[i]==curCase && (x<=xmax))
       {
         i++;
         x++;
         if(i==pw) 
           i=0;
       }
       if(curCase)
       {
         if(xb==x-1)
           active_sim->cnv->Pixel(xb,y,fgColor);
         else
           active_sim->FillLine(xb,y,x-1,y);  
       }                                             
    }
  }
  cdForeground(fgColor);
}

static void simHatchLine(int xmin, int xmax, int y, unsigned char pattern)
{
  int x,xb;
  int opacity, mask;
  unsigned char startp;
  long curColor, fgColor, bgColor;
  
  RotatenL(pattern,xmin&7);
  fgColor=cdForeground(CD_QUERY);
  opacity=cdBackOpacity(CD_QUERY);
  
  if(opacity==CD_OPAQUE)
  {
    bgColor=cdBackground(CD_QUERY);
    for (x = xmin; x <= xmax; x++)
    {
      curColor=(pattern&0x80)?fgColor:bgColor;
      
      xb=x;
      startp = pattern&0x80? 1: 0;
      RotateL(pattern);
      while (startp == (pattern&0x80? 1: 0) && x <= xmax)
      {
        x++;
        RotateL(pattern);
      }

      if(xb==x)
        active_sim->cnv->Pixel(xb,y,curColor);
      else
      {
        cdForeground(curColor);
        active_sim->FillLine(xb,y,x,y);  
      }
    }
  }
  else
  {
    cdForeground(fgColor);
    for (x = xmin; x <= xmax; x++)
    {
      mask=(pattern&0x80)?1:0;
      
      xb=x;
      startp = pattern&0x80? 1: 0;
      RotateL(pattern);
      while (startp == (pattern&0x80? 1: 0) && x <= xmax)
      {
        x++;
        RotateL(pattern);
      }
      
      if(mask)
      {
        if(xb==x)
          active_sim->cnv->Pixel(xb,y,fgColor);
        else
          active_sim->FillLine(xb,y,x,y);  
      }
    }
  }
  
  cdForeground(fgColor);
}

static void simFillPoly(int np, int *x, int *y) 
{
  struct block
  {
    int y_max;
    int y_min;
    float xs;
    float delta;
  };
  
  static struct block *aresta=NULL;
  static int *xx=NULL;
  static int NP=0;
  
  int Ymaximo;
  int Yminimo;
  int i,ys;
  int contador,indice;
  int width,height;
  
  active_sim->context = CLIP_REGION;
  
  cdGetCanvasSize(&width,&height,NULL,NULL);
  
  if (np > NP)
  {
    NP=np;
    if (xx) free (xx);
    if (aresta) free (aresta);
    xx=(int *) malloc ((NP+1)*sizeof(int));
    aresta=(struct block *) malloc ((NP+1)*sizeof(struct block));
  }
  
  Ymaximo = y[0];
  Yminimo = y[0];
  indice = 0;
  for(i=0;i<np;i++)
  {
    int i1=(i+1)%np;
    if (y[i] != y[i1])
    {
      
      aresta[indice].y_max = (y[i] > y[i1]) ? y[i] : y[i1];
      aresta[indice].y_min = (y[i] < y[i1]) ? y[i] : y[i1];
      
      aresta[indice].delta = ((float)(x[i1] - x[i])/(float)(y[i1] - y[i]));
      
      if (aresta[indice].y_max == y[i])
        aresta[indice].xs = (float)x[i];
      else
        aresta[indice].xs = (float)x[i1];
      
      if (aresta[indice].y_max > Ymaximo)
        Ymaximo = aresta[indice].y_max;
      if (aresta[indice].y_min < Yminimo)
        Yminimo = aresta[indice].y_min;
      indice++;
    }
  }
  
  
  if (Yminimo < 0)  /* faz o clipping com a parte de baixo da janela de clip */
    Yminimo = 0;
  
  for(ys=Ymaximo;ys>=Yminimo;ys--)
  {
    contador = 0;

    for(i=0;i<indice;i++)
    {
      if (aresta[i].y_min > ys)
      {
        aresta[i] = aresta[indice-1];
        indice--;
      }

      if ((ys >= aresta[i].y_min) && (ys < aresta[i].y_max))
      {
        aresta[i].xs  -= aresta[i].delta;
        xx[contador++] = (int)(floor(aresta[i].xs + 0.5));
      }
      else if (ys == Ymaximo && aresta[i].y_max == Ymaximo)
        xx[contador++] = (int)aresta[i].xs;
    }
    
    if (ys > height) /* faz o cliping com a parte de cima da janela de clip */
      continue;
    
    ordena(xx,0,contador-1);
    
    for(i=0;i<contador;i+=2)
    {
      if (xx[i]+1 <= xx[i+1])
        simFillLine(xx[i],ys,xx[i+1],0);
    }
  }
  
  active_sim->context = CLIP_LINES;
}


/*************************************************************************************/
/*                          PRIMITIVAS (public)                                      */
/*************************************************************************************/


void cdSimLine(int x1, int y1, int x2, int y2)
{
  if(active_sim->cnv->line_width>1)
  {
    const int interior = active_sim->cnv->interior_style;
    const int width = active_sim->cnv->line_width;
    const int style = active_sim->cnv->line_style;
    
    const int dx = x2-x1;
    const int dy = y2-y1;
    
    const double len = hypot(dx,dy);
    
    const double dnx = dx/len;
    const double dny = dy/len;
    
    const int w1 = (int)width/2;
    const int w2 = width-w1;
    
    const int n1x = f_Round_i( w1*dny);
    const int n1y = f_Round_i(-w1*dnx);
    
    const int n2x = f_Round_i(-w2*dny);
    const int n2y = f_Round_i( w2*dnx);
    
    const int p1x = x1 + n1x;
    const int p1y = y1 + n1y;
    const int p2x = x1 + n2x;
    const int p2y = y1 + n2y;
    const int p3x = p2x + dx;
    const int p3y = p2y + dy;
    const int p4x = p1x + dx;
    const int p4y = p1y + dy;
    
    int pntx[4];
    int pnty[4];
    
    active_sim->cnv->line_width = 1;
    active_sim->cnv->interior_style = CD_SOLID;
    active_sim->cnv->line_style = CD_CONTINUOUS;
    
    pntx[0] = p1x;
    pnty[0] = p1y;
    pntx[1] = p2x;
    pnty[1] = p2y;
    pntx[2] = p3x;
    pnty[2] = p3y;
    pntx[3] = p4x;
    pnty[3] = p4y;
    
    simFillPoly(4,pntx,pnty);
    
    active_sim->cnv->interior_style = interior;
    active_sim->cnv->line_width = width;
    active_sim->cnv->line_style = style;
  }
  else 
  {
    short int ls=line_styles[active_sim->cnv->line_style];
    long fgcolor=cdForeground(CD_QUERY);
    long bgcolor=cdBackground(CD_QUERY);
    unsigned char sm;
    int dx,dy,sx,sy,e,n;
    
    if (active_sim->cnv->clip_mode == CD_CLIPAREA && 
        cdSimClipLineInBox(&x1,&y1,&x2,&y2) == 0)
      return;
	  else if(active_sim->cnv->sim_mode & CD_SIM_CLIPPOLY && 
            active_sim->cnv->clip_mode == CD_CLIPPOLYGON) 
    {
      cdSimClipLineInPoly(x1,y1,x2,y2);
      return;
    }
    
    dx=x2-x1;
    if (dx<0)
    {
      dx=(-dx);
      sx=MX;
    }
    else if (dx>0)
      sx=PX;
    else
      sx=0;
    
    dy=y2-y1;
    if (dy<0)
    {
      dy=(-dy);
      sy=MY;
    }
    else if (dy>0)
      sy=PY;
    else
      sy=0;
    
    sm=(char)mask(x1);
    
    if (ls & 1)
      active_sim->cnv->Pixel(x1,y1,fgcolor);
    else if (active_sim->cnv->back_opacity == CD_OPAQUE)
      active_sim->cnv->Pixel(x1,y1,bgcolor);
    
    if (dx > dy)
    {
      n = dx;
      e = dx >> 1;
      while (n-->0)
      {
        if (sx & PX)
        {
          sm>>=1; ++x1;
          if (sm==0)
          {
            sm=(unsigned char)0x80;
          }
        }
        else if (sx & MX)
        {
          sm<<=1; --x1;
          if (sm==0)
          {
            sm=(unsigned char)0x01;
          }
        }
        e += dy;
        if (e > dx)
        {
          if (sy & PY)
            ++y1; 
          else if (sy & MY)
            --y1; 
          e -= dx;
        }
        sRotateL(ls);
        if (ls & 1)
          active_sim->cnv->Pixel(x1,y1,fgcolor);
        else if (active_sim->cnv->back_opacity == CD_OPAQUE)
          active_sim->cnv->Pixel(x1,y1,bgcolor);
      }
    }
    else
    {
      n = dy;
      e = (dy >> 1) + (dy & 1);
      while (n-->0)
      {
        if (sy & PY)
          ++y1;
        else if (sy & MY)
          --y1; 
        e += dx;
        if (e > dy)
        {
          if (sx & PX)
          {
            sm>>=1;  ++x1;
            if (sm==0)
              sm=(unsigned char)0x80;
          }
          else if (sx & MX)
          {
            sm<<=1; --x1;
            if (sm==0)
              sm=(unsigned char)0x01;
          }
          e -= dy;
        }
        sRotateL(ls);
        if (ls & 1)
          active_sim->cnv->Pixel(x1,y1,fgcolor);
        else if (active_sim->cnv->back_opacity == CD_OPAQUE)
          active_sim->cnv->Pixel(x1,y1,bgcolor);
      }
    }
  }
}

void cdSimBox(int xmin, int xmax, int ymin, int ymax)
{
  int y, old_line_style, old_line_width;
  
  active_sim->context = CLIP_REGION;
  old_line_style = cdLineStyle(CD_CONTINUOUS);
  old_line_width = cdLineWidth(1);

  for(y=ymin;y<=ymax;y++)
    simFillLine(xmin, y, xmax, 0);
  
  cdLineStyle(old_line_style);
  cdLineWidth(old_line_width);
  active_sim->context = CLIP_LINES;
}

void cdSimArc(int xc, int yc, int width, int height, double angle1, double angle2)
{
  long int a=width/2;              /* semi-eixo horizontal */
  long int b=height/2;             /* semi-eixo vertical */
  long int x;                      /* (x, y) percorre um quadrante da... */
  long int y;                      /* ...elipse, os outros sao simetricos */
  long int px1;                    /* coordenadas dos pontos simetricos */
  long int px2;                    /* coordenadas dos pontos simetricos */
  long int py1;                    /* coordenadas dos pontos simetricos */
  long int py2;                    /* coordenadas dos pontos simetricos */
  long int F;                      /* valor do ponto na funccao */
  long int DFx, DFy;               /* incremento da funccao */
  long int gx, gy;                 /* gradiente */
  int big = 0;                     /* Maior que 180o ? */
  
  /* constantes precalculadas */
  const long int _a2 = a*a;
  const long int _b2 = b*b;
  const long int _2a2 = 2*_a2;
  const long int _2b2 = 2*_b2;
  const long int _4a2 = 2*_2a2;
  const long int _4b2 = 2*_2b2;
  const long int _8a2 = 2*_4a2;
  const long int _8b2 = 2*_4b2;
  
  /* variaveis para o criterio de pertinencia */
  double x1, y1, x2, y2;
  double ta1, tb1, tc1, td1, ta2, tb2, tc2, td2;
  /* ta1 = y1 * x - x1 * y; ta2 = - y2 * x + x2 * y */
  double angled; /* Variacao entre o angulo 2 e o 1 */
  
  long int secolor = cdForeground(CD_QUERY);

  if (active_sim->cnv->clip_mode != CD_CLIPOFF)
    return;

  reduce(angle1);
  reduce(angle2);
  angled = angle2 - angle1;
  reduce(angled);
  
  if (angled > 180 || angle2 == angle1)
  {
    double anglet = angle1;
    angle1 = angle2;
    angle2 = anglet;
    big = 1;
  }
  
  /* inicializa o ponto final e o inicial */
  x1 = a*cos(angle1/CD_RAD2DEG);
  x2 = a*cos(angle2/CD_RAD2DEG);

  if (active_sim->cnv->invert_yaxis)
  {
    double t;
    y1 = b*sin(-angle1/CD_RAD2DEG);
    y2 = b*sin(-angle2/CD_RAD2DEG);

    t = y1; y1 = y2; y2 = t;
    t = x1; x1 = x2; x2 = t;
  }
  else
  {
    y1 = b*sin(angle1/CD_RAD2DEG);
    y2 = b*sin(angle2/CD_RAD2DEG);
  }
  
  /* checa quem eh o maior semi-eixo */
  if (b>a) 
  {
    x = 0;
    px1 = xc;
    px2 = xc;
    y = b;
    py1 = yc + b;
    py2 = yc - b;
    
    ta1 = tb1 = x1 * y;
    tc1 = td1 = - ta1;
    ta2 = tb2 = x2 * y;
    tc2 = td2 = - ta2;
    
    /* inicializa a funcao de erro */
    /* F(x, y) = _a2*y2 + _b2*x2 - _a2*_b2, em (1, b - 0.5) */
    F = _4b2 - _4a2*_b2 + _a2*(2*b-1)*(2*b-1);
    
    /* inicializa o gradiente */
    gx = _2b2;
    gy = _a2*(2*b-1);
    
    /* inicializa o incremento da funccao implicita */    
    DFx = _4b2*(2*x-1) + _4b2;
    DFy = _4a2 - _4a2*(2*y+1);
    
    /* coloca os primeiros pontos */
    Two();
    
    /* desenha o primeiro octante */
    while (gx<gy) {
      /* se o midpoint estiver fora... */
      if (F>0) {
        y--;
        py1--;
        py2++;
        gy -= _2a2;
        DFy += _8a2;
        F += DFy;
        ta1 -= x1; tb1 -= x1; tc1 += x1; td1 += x1;
        ta2 -= x2; tb2 -= x2; tc2 += x2; td2 += x2;
      }
      x++;
      px1++;
      px2--;
      gx += _2b2;
      DFx += _8b2;
      F += DFx;
      ta1 -= y1; tb1 += y1; tc1 += y1; td1 -= y1;
      ta2 -= y2; tb2 += y2; tc2 += y2; td2 -= y2;
      Four();
    }
    
    /* ajusta F para o segundo octante */
    F = _b2*(2*x+1)*(2*x+1) + _4a2*(y-1)*(y-1) - _4a2*_b2;
    DFx = _4b2*(2*x-1) + _4b2;
    DFy =  _4a2 - _8a2*y;
    
    /* desenha o segundo octante */
    while (y>1) {
      if (F<0) {
        x++;
        px1++;
        px2--;
        DFx += _8b2;
        F += DFx;
        ta1 -= y1; tb1 += y1; tc1 += y1; td1 -= y1;
        ta2 -= y2; tb2 += y2; tc2 += y2; td2 -= y2;
      }
      y--;
      py1--;
      py2++;
      DFy += _8a2;
      F += DFy;
      ta1 -= x1; tb1 -= x1; tc1 += x1; td1 += x1;
      ta2 -= x2; tb2 -= x2; tc2 += x2; td2 += x2;
      if (x>0) {
        Four();
      }
      else {
        Two();
      }
    }
    
    /* desenha o ultimo ponto */
    if (F<0) {
      px1++;
      px2--;
      ta1 -= y1; tb1 += y1; tc1 += y1; td1 -= y1;
      ta2 -= y2; tb2 += y2; tc2 += y2; td2 -= y2;
    }
    py1--;
    py2++;
    ta1 -= x1; tb1 -= x1; tc1 += x1; td1 += x1;
    ta2 -= x2; tb2 -= x2; tc2 += x2; td2 += x2;
    if (px1 != px2) {
      Two();
    }
    else {
      One();
    }
  }
  
  else {
    x = a;
    px1 = xc + a;
    px2 = xc - a;
    y = 0;
    py1 = yc;
    py2 = yc;
    
    ta1 = td1 = - y1 * x;
    tb1 = tc1 = - ta1;
    ta2 = td2 = - y2 * x;
    tb2 = tc2 = - ta2;
    
    /* inicializa a funcao de erro */
    /* F(x, y) = _a2*y2 + _b2*x2 - _a2*_b2, em (a - 0.5, 1) */
    F = _4a2 - _4b2*_a2 + _b2*(2*a-1)*(2*a-1);
    
    /* inicializa o gradiente */
    gy = _2a2;
    gx = _b2*(2*a-1);
    
    /* coloca os primeiros pontos */
    Two();
    
    /* desenha o primeiro octante */
    while (gy<gx) {
      /* se o midpoint estiver fora... */
      if (F>0) {
        x--;
        px1--;
        px2++;
        gx -= _2b2;
        F += _4b2 - _4b2*(2*x+1);
        ta1 += y1; tb1 -= y1; tc1 -= y1; td1 += y1;
        ta2 += y2; tb2 -= y2; tc2 -= y2; td2 += y2;
      }
      y++;
      py1++;
      py2--;
      gy += _2a2;
      F += _8a2*y + _4a2;
      ta1 += x1; tb1 += x1; tc1 -= x1; td1 -= x1;
      ta2 += x2; tb2 += x2; tc2 -= x2; td2 -= x2;
      Four();
    }
    
    /* ajusta F para o segundo octante */
    F = _a2*(2*y+1)*(2*y+1) + _4b2*(x-1)*(x-1) - _4a2*_b2;
    
    /* desenha o segundo octante */
    while (x>1) {
      if (F<0) {
        y++;
        py1++;
        py2--;
        F += _4a2*(2*y-1) + _4a2;
        ta1 += x1; tb1 += x1; tc1 -= x1; td1 -= x1;
        ta2 += x2; tb2 += x2; tc2 -= x2; td2 -= x2;
      }
      x--;
      px1--;
      px2++;
      F += _4b2 - _8b2*x;
      ta1 += y1; tb1 -= y1; tc1 -= y1; td1 += y1;
      ta2 += y2; tb2 -= y2; tc2 -= y2; td2 += y2;
      if (y>0) {
        Four();
      }
      else {
        Two();
      }
    }
    
    /* desenha o ultimo ponto */
    if (F<0) {
      py1++;
      py2--;
      ta1 += x1; tb1 += x1; tc1 -= x1; td1 -= x1;
      ta2 += x2; tb2 += x2; tc2 -= x2; td2 -= x2;
    }
    px1--;
    px2++;
    ta1 += y1; tb1 -= y1; tc1 -= y1; td1 += y1;
    ta2 += y2; tb2 -= y2; tc2 -= y2; td2 += y2;
    if (py1 != py2) {
      Two();
    }
    else {
      One();
    }
  }
}

void cdSimSector(int xc, int yc, int width, int height, double angle1, double angle2)
{
  long int a=width/2;              /* semi-eixo horizontal */
  long int b=height/2;             /* semi-eixo vertical */
  long int x;                      /* (x, y) percorre um quadrante da... */
  long int y;                      /* ...elipse, os outros sao simetricos */
  long int px1;                    /* coordenadas dos pontos simetricos */
  long int px2;                    /* coordenadas dos pontos simetricos */
  long int py1;                    /* coordenadas dos pontos simetricos */
  long int py2;                    /* coordenadas dos pontos simetricos */
  long int F;                      /* valor do ponto na funccao */
  long int DFx, DFy;               /* incremento da funccao */
  long int gx, gy;                 /* gradiente */
  int big = 0;                     /* Maior que 180o ? */
  int old_line_style, old_line_width;
  
  /* constantes precalculadas */
  const long int _a2 = a*a;
  const long int _b2 = b*b;
  const long int _2a2 = 2*_a2;
  const long int _2b2 = 2*_b2;
  const long int _4a2 = 2*_2a2;
  const long int _4b2 = 2*_2b2;
  const long int _8a2 = 2*_4a2;
  const long int _8b2 = 2*_4b2;
  
  /* variaveis para o criterio de pertinencia */
  double x1, y1, x2, y2;
  double ta1, tb1, tc1, td1, ta2, tb2, tc2, td2;
  /* ta1 = y1 * x - x1 * y; ta2 = - y2 * x + x2 * y */
  double angled; /* Variacao entre o angulo 2 e o 1 */
  
  /* variaveis pras retas */
  double r1x, r1y, r2x, r2y;            /* posicao x e y nas retas 1 e 2 */
  double r1k, r2k;
  
  const long int secolor=cdForeground(CD_QUERY);

  old_line_style = cdLineStyle(CD_CONTINUOUS);
  old_line_width = cdLineWidth(1);
  
  active_sim->context = CLIP_REGION;
  
  reduce(angle1);
  reduce(angle2);
  angled = angle2 - angle1;
  reduce(angled);
  
  if (angled > 180 || angle2 == angle1)
  {
    double anglet = angle1;
    angle1 = angle2;
    angle2 = anglet;
    big = 1;
  }
  
  /* inicializa o ponto final e o inicial */
  x1 = a*cos(angle1/CD_RAD2DEG);
  x2 = a*cos(angle2/CD_RAD2DEG);

  if (active_sim->cnv->invert_yaxis)
  {
    double t;
    y1 = b*sin(-angle1/CD_RAD2DEG);
    y2 = b*sin(-angle2/CD_RAD2DEG);

    t = y1; y1 = y2; y2 = t;
    t = x1; x1 = x2; x2 = t;
  }
  else
  {
    y1 = b*sin(angle1/CD_RAD2DEG);
    y2 = b*sin(angle2/CD_RAD2DEG);
  }
  
  x = 0;
  px1 = xc;
  px2 = xc;
  y = b;
  py1 = yc + b;
  py2 = yc - b;
  
  ta1 = tb1 = x1 * y;
  tc1 = td1 = - ta1;
  ta2 = tb2 = x2 * y;
  tc2 = td2 = - ta2;
  
  /* inicializa a funcao de erro */
  /* F(x, y) = _a2*y2 + _b2*x2 - _a2*_b2, em (1, b - 0.5) */
  F = _4b2 - _4a2*_b2 + _a2*(2*b-1)*(2*b-1);
  /* inicializa o gradiente */
  gx = _2b2;
  gy = _a2*(2*b-1);
  
  /* inicializa o incremento da funccao implicita */    
  DFx = _4b2*(2*x-1) + _4b2;
  DFy = _4a2 - _4a2*(2*y+1);
  
  r1k = x1 / y1;
  r2k = x2 / y2;
  
  /* Interseccao da linha horizontal atual com as retas */
  if (y1 != 0)
  {
    r1y = (y1 > 0) ? y : -y;
    r1x = r1y * r1k;      
    if ((int)r1x + xc >= px1 || (int)r1x + xc <= px2) /* nao intersectou */
      r1y++;
  }
  if (y2 != 0)
  {
    r2y = (y2 > 0) ? y : -y;
    r2x = r2y * r2k;      
    if ((int)r2x + xc >= px1 || (int)r2x + xc <= px2) /* nao intersectou */
      r2y++;
  }
  
  /* coloca os primeiros pontos */
  Two();
  
  /* desenha o primeiro octante */
  while (gx<gy) {
    /* se o midpoint estiver fora... */
    if (F>0) {
      LinesX();
      y--;
      py1--;
      py2++;
      gy -= _2a2;
      DFy += _8a2;
      F += DFy;
      ta1 -= x1; tb1 -= x1; tc1 += x1; td1 += x1;
      ta2 -= x2; tb2 -= x2; tc2 += x2; td2 += x2;
      if (y1 != 0)
      {
        r1y = (y1 > 0) ? y : -y;
        r1x = r1y * r1k;      
        if ((int)r1x + xc >= px1 || (int)r1x + xc <= px2) /* nao intersectou */
          r1y++;
      }
      if (y2 != 0)
      {
        r2y = (y2 > 0) ? y : -y;
        r2x = r2y * r2k;      
        if ((int)r2x + xc >= px1 || (int)r2x + xc <= px2) /* nao intersectou */
          r2y++;
      }
    }
    x++;
    px1++;
    px2--;
    gx += _2b2;
    DFx += _8b2;
    F += DFx;
    ta1 -= y1; tb1 += y1; tc1 += y1; td1 -= y1;
    ta2 -= y2; tb2 += y2; tc2 += y2; td2 -= y2;
  }
  
  LinesX();
  /* ajusta F para o segundo octante */
  F = _b2*(2*x+1)*(2*x+1) + _4a2*(y-1)*(y-1) - _4a2*_b2;
  DFx = _4b2*(2*x-1) + _4b2;
  DFy =  _4a2 - _8a2*y;
  
  /* desenha o segundo octante */
  while (y>1) {
    if (F<0) {
      x++;
      px1++;
      px2--;
      DFx += _8b2;
      F += DFx;
      ta1 -= y1; tb1 += y1; tc1 += y1; td1 -= y1;
      ta2 -= y2; tb2 += y2; tc2 += y2; td2 -= y2;
    }
    y--;
    py1--;
    py2++;
    DFy += _8a2;
    F += DFy;
    ta1 -= x1; tb1 -= x1; tc1 += x1; td1 += x1;
    ta2 -= x2; tb2 -= x2; tc2 += x2; td2 += x2;
    if (y1 != 0)
    {
      r1y = (y1 > 0) ? y : -y;
      r1x = r1y * r1k;      
      if ((int)r1x + xc >= px1 || (int)r1x + xc <= px2) /* nao intersectou */
        r1y++;
    }
    if (y2 != 0)
    {
      r2y = (y2 > 0) ? y : -y;
      r2x = r2y * r2k;      
      if ((int)r2x + xc >= px1 || (int)r2x + xc <= px2) /* nao intersectou */
        r2y++;
    }
    LinesX();
  }
  
  /* desenha o ultimo ponto */
  if (F<0) {
    px1++;
    px2--;
    ta1 -= y1; tb1 += y1; tc1 += y1; td1 -= y1;
    ta2 -= y2; tb2 += y2; tc2 += y2; td2 -= y2;
  }
  py1--;
  py2++;
  ta1 -= x1; tb1 -= x1; tc1 += x1; td1 += x1;
  ta2 -= x2; tb2 -= x2; tc2 += x2; td2 += x2;
  LinesX();
  
  /* desenha o ultimo ponto */
  if (F<0) {
    py1++;
    py2--;
    ta1 += x1; tb1 += x1; tc1 -= x1; td1 -= x1;
    ta2 += x2; tb2 += x2; tc2 -= x2; td2 -= x2;
  }
  px1--;
  px2++;
  ta1 += y1; tb1 -= y1; tc1 -= y1; td1 += y1;
  ta2 += y2; tb2 -= y2; tc2 -= y2; td2 += y2;
  LineX();
  
  cdLineStyle(old_line_style);
  cdLineWidth(old_line_width);
  active_sim->context = CLIP_LINES;
}

void cdSimChord(int xc, int yc, int width, int height, double angle1, double angle2)
{
}

void cdSimNativeFont(char* font_data)
{
  /*  Formato de font_data:
     "fontname size" */
  char font[512];
  int size = 0;

  /* this is correct: [^,] = all characters except the coma */
  sscanf(font_data, "%[^,], %d", font, &size);
  if(font[0] == '\0' || size == 0 ) return;

  if (size < 0)
  {
    double size_mm;
    cdPixel2MM(-size, 0, &size_mm, NULL);
    size = (int)(size_mm * CD_MM2PT);
  }

  cdTT_load(active_sim->tt_text, font, size, active_sim->cnv->xres, active_sim->cnv->yres);
}

static char * cd_ttf_font_type_face[4] = {
  "cour",
  "cour",
  "times",
  "arial"};

static char * cd_ttf_font_style[4] = {
  "",
  "bd",
  "i",
  "bi"};

void cdSimFont(int* type_face, int* style, int* size)
{
  char font[50];
  sprintf(font, "%s%s.ttf", cd_ttf_font_type_face[*type_face], cd_ttf_font_style[*style]);

  if (*size < 0)
  {
    double size_mm;
    cdPixel2MM(-*size, 0, &size_mm, NULL);
    *size = (int)(size_mm * CD_MM2PT);
  }

  cdTT_load(active_sim->tt_text, font, *size, active_sim->cnv->xres, active_sim->cnv->yres);

  if (!active_sim->tt_text->face)
  {
    *type_face = active_sim->cnv->font_type_face;
    *style = active_sim->cnv->font_style;
    *size = active_sim->cnv->font_size;
  }
}
            
void cdSimFontDim(int *max_width, int *height, int *ascent, int *descent)
{
  if (!active_sim->tt_text->face)
    return;

  if(ascent) *ascent = active_sim->tt_text->ascent;
  if(descent) *descent= -active_sim->tt_text->descent; /* descent aqui e' negativo */
  if(max_width) *max_width= active_sim->tt_text->max_width;
  if(height) *height= active_sim->tt_text->max_height;
}

void cdSimTextSize(char *s, int *width, int *height)
{
  int w=0;
  FT_Face       face;
  FT_GlyphSlot  slot;
  FT_Error      error;

  if (!active_sim->tt_text->face)
    return;

  face = active_sim->tt_text->face;
  slot = face->glyph;

  /* set transformation */
  FT_Set_Transform( face, NULL, NULL );

  while(*s)
  {
    /* load glyph image into the slot (erase previous one) */
    error = FT_Load_Char( face, *(unsigned char*)s, FT_LOAD_DEFAULT );
    if (error) {s++; continue;}  /* ignore errors */

    w += slot->advance.x; 

    s++;
  }

  if (height) *height = active_sim->tt_text->max_height;
  if (width)  *width  = w >> 6;
}

static void simDrawTextBitmap(FT_Bitmap* bitmap, int x, int y)
{
  unsigned char *red, *green, *blue, *alpha, *bitmap_data;
  int width = bitmap->width;
  int height = bitmap->rows;
  int size = width*height;
  int rgba_data_size = size*4;

  /* avoid spaces */
  if (width == 0 || height == 0)
    return;

  if (!active_sim->tt_text->rgba_data)
    active_sim->tt_text->rgba_data = malloc(rgba_data_size);
  else if (rgba_data_size > active_sim->tt_text->rgba_data_size)
  {
    active_sim->tt_text->rgba_data = realloc(active_sim->tt_text->rgba_data, rgba_data_size);
    active_sim->tt_text->rgba_data_size = rgba_data_size;
  }

  bitmap_data = bitmap->buffer + (height-1)*width;  /* bitmap is top down. */
  red   = active_sim->tt_text->rgba_data;
  green = red   + size;
  blue  = green + size;
  alpha = blue  + size;

  if (active_sim->cnv->PutImageRectRGBA == cdSimPutImageRectRGBA && !active_sim->cnv->GetImage)
  {
    int i, j;
    unsigned char bg_red, bg_green, bg_blue, fg_red, fg_green, fg_blue;
    long int c;
    c = cdBackground(CD_QUERY);
    bg_red   = cdRed(c);
    bg_green = cdGreen(c);
    bg_blue  = cdBlue(c);
    c = cdForeground(CD_QUERY);
    fg_red   = cdRed(c);
    fg_green = cdGreen(c);
    fg_blue  = cdBlue(c);

    for (i = 0; i < height; i++)
    {
      for (j = 0; j < width; j++)
      {
        if (*bitmap_data == 255)
        {
          *red++ = fg_red;
          *green++ = fg_green;
          *blue++ = fg_blue;
        }
        else
        {
          *red++ = bg_red;     
          *green++ = bg_green;
          *blue++ = bg_blue;
        }
        bitmap_data++;
      }

      bitmap_data -= 2*width;
    }

    red   = active_sim->tt_text->rgba_data;
    green = red   + size;
    blue  = green + size;
    active_sim->cnv->PutImageRectRGB(width,height,red,green,blue,x,y,width,height,0,width-1,0,height-1);
  }
  else
  {
    int i;
    long int fg = cdForeground(CD_QUERY);
    memset(red,   cdRed(fg), size);
    memset(green, cdGreen(fg), size);
    memset(blue,  cdBlue(fg), size);
    for (i = 0; i < height; i++)
    {
      memcpy(alpha,  bitmap_data, width);
      alpha += width;
      bitmap_data -= width;
    }

    alpha = blue + size;
    active_sim->cnv->PutImageRectRGBA(width,height,red,green,blue,alpha,x,y,width,height,0,width-1,0,height-1);
  }
}

/* move the reference point to the baseline-left */
static void simGetPenPos(int *pen_x, int *pen_y, int x, int y, int w)
{
  const int h  = active_sim->tt_text->max_height;
  int baseline = active_sim->tt_text->max_height - active_sim->tt_text->ascent;

  /* move to bottom left */
  cdTextTranslatePoint(*pen_x, *pen_y, w, h, baseline, pen_x, pen_y);

  /* move to the base line */
  if (active_sim->cnv->invert_yaxis)
    *pen_y -= baseline;
  else                                    
    *pen_y += baseline;

  if (active_sim->cnv->text_orientation)
  {
    double cos_teta = cos(active_sim->cnv->text_orientation/CD_RAD2DEG);
    double sin_teta = sin(active_sim->cnv->text_orientation/CD_RAD2DEG);
    cdTextRotatePoint(*pen_x, *pen_y, x, y, pen_x, pen_y, sin_teta, cos_teta);
  }
}

void cdSimText(int x, int y, char * s)
{
  FT_Face       face;
  FT_GlyphSlot  slot;
  FT_Matrix     matrix;                 /* transformation matrix */
  FT_Vector     pen;                    /* untransformed origin  */
  FT_Error      error;

  if (!active_sim->tt_text->face)
    return;

  face = active_sim->tt_text->face;
  slot = face->glyph;

  /* set up matrix */
  if (active_sim->cnv->text_orientation)
  {
    double cos_theta = cos(active_sim->cnv->text_orientation/CD_RAD2DEG) * 0x10000L;
    double sin_theta = sin(active_sim->cnv->text_orientation/CD_RAD2DEG) * 0x10000L;
    matrix.xx = (FT_Fixed)( cos_theta);
    matrix.xy = (FT_Fixed)(-sin_theta);
    matrix.yx = (FT_Fixed)( sin_theta);
    matrix.yy = (FT_Fixed)( cos_theta);
  }
  else
    FT_Set_Transform( face, NULL, NULL );

  {
    int uw;
    cdSimTextSize(s, &uw, NULL);
    simGetPenPos(&x, &y, x, y, uw);

    /* the pen position in 26.6 cartesian space coordinates; */
    pen.x = x * 64;
    pen.y = y * 64;
  }

  while(*s)
  {
    /* set transformation */
    if (active_sim->cnv->text_orientation)
      FT_Set_Transform( face, &matrix, NULL );

    /* load glyph image into the slot (erase previous one) */
    error = FT_Load_Char( face, *(unsigned char*)s, FT_LOAD_RENDER );
    if (error) {s++; continue;}  /* ignore errors */

    x = (pen.x>>6)+slot->bitmap_left;
    if (active_sim->cnv->invert_yaxis)
      y = (pen.y>>6)+(slot->bitmap.rows-slot->bitmap_top);
    else
      y = (pen.y>>6)-(slot->bitmap.rows-slot->bitmap_top);

    /* now, draw to our target surface (convert position) */
    simDrawTextBitmap(&slot->bitmap, x, y);

    /* increment pen position */
    pen.x += slot->advance.x;
    if (active_sim->cnv->invert_yaxis)
      pen.y -= slot->advance.y;
    else
      pen.y += slot->advance.y;

    s++;
  }
}

/* Setup Bezier coefficient array once for each control polygon. */
static void BezierForm(int NumCtlPoints, wdPoint* p, wdPoint* c)
{
  int k; long n, choose;
  n= NumCtlPoints-1;
  for(k = 0; k <= n; k++) 
  {
    if (k == 0) choose = 1;
    else if (k == 1) choose = n;
    else choose = choose *(n-k+1)/k;
    c[k].x = p[k].x *choose;
    c[k].y = p[k].y *choose;
  }
}

/*  Return Point pt(t), t <= 0 <= 1 from C, given the number
of Points in control polygon. BezierForm must be called
once for any given control polygon. */
static void BezierCurve(int NumCtlPoints, wdPoint* c, wdPoint *pt, double t)
{   
  int k, n;
  double t1, tt, u;
  wdPoint b[4];

  n = NumCtlPoints -1;  
  u = t;

  b[0].x = c[0].x;
  b[0].y = c[0].y;
  for(k =1; k <=n; k++) 
  {
    b[k].x = c[k].x *u;
    b[k].y = c[k].y *u;
    u =u*t;
  }

  (*pt).x = b[n].x;  
  (*pt).y = b[n].y;
  t1 = 1-t;          
  tt = t1;
  for(k =n-1; k >=0; k--) 
  {
    (*pt).x += b[k].x *tt;
    (*pt).y += b[k].y *tt;
    tt =tt*t1;
  }
}

/* Quick and Simple Bezier Curve Drawing --- Robert D. Miller
 * This 2-D planar Bezier curve drawing software is 3-D compliant ---
 * redefine Point and change the commented lines as indicated.
 * GEMS V
 */
void cdSimPolyBezier(cdPoint* points, int n)
{
  int i = 0, k;
  wdPoint pt1, pt2;
  wdPoint bezier[4];
  wdPoint bezier_control[4];

  while (n > 3)
  {
    bezier[0].x = points[i].x;      /* first point          */
    bezier[0].y = points[i].y;
    bezier[1].x = points[i+1].x;    /* first control point  */
    bezier[1].y = points[i+1].y;
    bezier[2].x = points[i+2].x;    /* second control point */
    bezier[2].y = points[i+2].y;
    bezier[3].x = points[i+3].x;    /* last point           */
    bezier[3].y = points[i+3].y;

    BezierForm(4, bezier, bezier_control);

    for(k =0; k <=10; k++) 
    {
      BezierCurve(4, bezier_control, &pt1, k/10.0);
      if (k != 0) 
      {
        int x1 = (int)pt1.x;
        int y1 = (int)pt1.y;
        int x2 = (int)pt2.x;
        int y2 = (int)pt2.y;

	      if (active_sim->cnv->clip_mode == CD_CLIPAREA && 
            cdSimClipLineInBox(&x1, &y1, &x2, &y2) == 0)
          return;
	      else if(active_sim->cnv->clip_mode == CD_CLIPPOLYGON) 
        {
          cdSimClipLineInPoly(x1, y1, x2, y2);
          return;
        }

        active_sim->cnv->Line(x1, y1, x2, y2);
      }

      pt2 = pt1;
    }

    n -= 3; i += 3;
  }
}

void cdSimPoly(int mode, cdPoint* poly, int n)
{
  int i;
  int *pntx, *pnty, x1, y1, x2, y2;

  pntx = (int*)malloc(sizeof(int)*(n+1));
  pnty = (int*)malloc(sizeof(int)*(n+1));

  for (i = 0; i < n; i++)
  {
    pntx[i] = poly[i].x;
    pnty[i] = poly[i].y;
  }

  switch(mode) 
  {
  case CD_CLOSED_LINES:
    pntx[n] = pntx[0];
    pnty[n] = pnty[0];
    n++;
    /* continua */
  case CD_OPEN_LINES:
    for (i = 0; i< n - 1; i++)
    {
      x1 = pntx[i];
      y1 = pnty[i];
      x2 = pntx[i+1];
      y2 = pnty[i+1];

	    if (active_sim->cnv->clip_mode == CD_CLIPAREA && 
         cdSimClipLineInBox(&x1, &y1, &x2, &y2) == 0)
        continue;
	    else if(active_sim->cnv->clip_mode == CD_CLIPPOLYGON) 
      {
        cdSimClipLineInPoly(x1, y1, x2, y2);
        continue;
      }

      active_sim->cnv->Line(x1, y1, x2, y2);
    }
    break;
  case CD_BEZIER:
    cdSimPolyBezier(poly, n);
    break;
  case CD_CLIP:
    pntx[n] = pntx[0];
    pnty[n] = pnty[0];
    n++;
    
    if(active_sim->clippoly.pntx) free(active_sim->clippoly.pntx);
    if(active_sim->clippoly.pnty) free(active_sim->clippoly.pnty);
    if(active_sim->clippoly.sgmx) free(active_sim->clippoly.sgmx);
    if(active_sim->clippoly.sgmy) free(active_sim->clippoly.sgmy);

    active_sim->clippoly.n = n;
    active_sim->clippoly.pntx = (int*)malloc(sizeof(int)*(active_sim->clippoly.n));
    active_sim->clippoly.pnty = (int*)malloc(sizeof(int)*(active_sim->clippoly.n));
    active_sim->clippoly.sgmx = (int*)malloc(sizeof(int)*(active_sim->clippoly.n));
    active_sim->clippoly.sgmy = (int*)malloc(sizeof(int)*(active_sim->clippoly.n));

    for (i = 0; i< n; i++)
    {
      active_sim->clippoly.pntx[i] = pntx[i];
      active_sim->clippoly.pnty[i] = pnty[i];
    }
    break;
	case CD_FILL:
    {
      int oldwidth=cdLineWidth(1);
		  int oldstyle=cdLineStyle(CD_CONTINUOUS);               							

      simFillPoly(n, pntx, pnty);
  											                                							
		  cdLineStyle(oldstyle);                 													
      cdLineWidth(oldwidth);
    }
		break;
  }

  free(pntx);
  free(pnty);
}

void cdSimPutImageRectRGBA(int iw, int ih, unsigned char *r, unsigned char *g, unsigned char *b, unsigned char *a, int x, int y, int w, int h, int xmin, int xmax, int ymin, int ymax)
{
  if (active_sim->cnv->GetImageRGB && a)
  {
    int size, i, j, dst, src, *fx, *fy, rw, rh;
    unsigned char *ar, *ag, *ab, al;

    size = w * h;
    ar = (unsigned char*)malloc(size*3);
    if (!ar) return;
    ag = ar + size;
    ab = ag + size;

    active_sim->cnv->GetImageRGB(ar, ag, ab, x, y, w, h);

    rw = xmax-xmin+1;
    rh = ymax-ymin+1;

    fx = (int *)malloc(rw*sizeof(int));
    fy = (int *)malloc(rh*sizeof(int));

    if (w==rw && h==rh)
    {
      for (i = 0; i < rw; i++)
        fx[i] = i;

      for (i = 0; i < rh; i++)
        fy[i] = i;
    }
    else
    {
      cdMakeStretchTable(w, rw, fx);
      cdMakeStretchTable(h, rh, fy);
    }
  
    for (i = 0; i < rw; i++)
      fx[i] += xmin;

    for (i = 0; i < rh; i++)
      fy[i] += ymin;
  
    for (j = 0; j < h; j++)
    {
      for (i = 0; i < w; i++)
      {
        dst = j * w + i;
        src = fy[j] * iw + fx[i];
        al = a[src];
        ar[dst] = CD_ALPHA_BLEND(r[src], ar[dst], al);
        ag[dst] = CD_ALPHA_BLEND(g[src], ag[dst], al);
        ab[dst] = CD_ALPHA_BLEND(b[src], ab[dst], al);
      }
    }

    active_sim->cnv->PutImageRectRGB(w, h, ar, ag, ab, x, y, w, h, 0, 0, 0, 0);

    free(ar);

    free(fx);
    free(fy);
  }
  else
    active_sim->cnv->PutImageRectRGB(iw, ih, r, g, b, x, y, w, h, xmin, xmax, ymin, ymax);
}
