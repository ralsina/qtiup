/** \file
 * \brief iupgauge control
 *
 * See Copyright Notice in iup.h
 * $Id: iupgauge.c,v 1.48 2006/03/15 02:51:55 scuri Exp $
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#include <iup.h>
#include <cd.h>
#include <cdiup.h>
#include <cdimage.h>
#include <iupcpi.h>
#include "istrutil.h"

#include <iupcompat.h>
#include "cdiuputil.h"
#include "iupgauge.h"
#include "icontrols.h"

#ifdef WIN32
#define FONT_ATTR "WIN_FONT"          /* OLD STUFF */
#else
#define FONT_ATTR "MOTIF_FONT"
#endif

#define GDEFAULTCOLOR "64 96 192"
#define GDEFAULTSIZE "180x20"

#define GAP 3
#define BLOCKS 20

static Ihandle *gaugecreate(Iclass *ic);
static void gaugesetattr(Ihandle *h, char *attr, char *value);
static void gaugedestroy(Ihandle *h);
static char *gaugegetattr(Ihandle *h, char *attr);

typedef struct _Tgauge
{
  int w,h;
  void *im;
  cdCanvas *cdim;
  cdCanvas *cdcanvas;
  long bgcolor,
       fgcolor,
       light_shadow,
       mid_shadow,
       dark_shadow;
  double value,  /* value is min<value<max */
         vmin,
         vmax;
  char *text;
  char *typeface;
  int show_text,
      dashed,
      margin;
} Tgauge;

static void beginimage(Tgauge *g)
{
  cdActivate(g->cdim);
  cdBackground(g->bgcolor);
  cdClear();
}

static void putimage(Tgauge *g)
{
  cdActivate(g->cdcanvas);
  cdPutImage(g->im,0,0);
}

#define round(_) ((int)((_)+.5))

static void drawtext(Tgauge *g, int xmid)
{
  int x, y, xmin, xmax, ymin, ymax;
  char* text = g->text;
  if (g->typeface) cdNativeFont(g->typeface);
  cdTextAlignment(CD_CENTER);
  cdBackOpacity(CD_TRANSPARENT);

  x = (int)(0.5*g->w);
  y = (int)(0.5*g->h);

  if (text == NULL)
  {
    static char m[30];
    sprintf(m,"%.1f%%",100*(g->value-g->vmin)/(g->vmax-g->vmin));
    text = m;
  }

  cdTextBox(x, y, text, &xmin, &xmax, &ymin, &ymax);

  if (xmid < xmin)
  {
    cdForeground(g->fgcolor);
    cdText(x,y,text);
  }
  else if (xmid > xmax)
  {
    cdForeground(g->bgcolor);
    cdText(x,y,text);
  }
  else
  {
    cdClip(CD_CLIPAREA);
    cdClipArea(xmin, xmid, ymin, ymax);
    cdForeground(g->bgcolor);
    cdText(x,y,text);

    cdClipArea(xmid, xmax, ymin, ymax);
    cdForeground(g->fgcolor);
    cdText(x,y,text);
    cdClip(CD_CLIPOFF);
  }
}

static void drawgauge(Tgauge *g)
{
  int margem = g->margin+2; /* inclui o pixel usado para desenhar a borda 3D */
  int ystart = margem;
  int yend = g->h-1 - margem;
  int xstart = margem;
  int xend = g->w-1 - margem;

  cdIupDrawSunkenRect(0, 0, g->w-1, g->h-1, g->light_shadow, g->mid_shadow, g->dark_shadow);

  cdForeground(g->fgcolor);

  if (g->dashed)
  {
    float step = (xend-xstart+1)/(float)BLOCKS;
    float boxw = step-GAP;
    float vx = (float)((xend-xstart+1) * (g->value-g->vmin)/(g->vmax-g->vmin));
    int intvx = (int)(100*vx);
    float i=0;

    if (g->value == g->vmin) return;

    while ( round(100*(i + boxw)) <= intvx )
    {
      cdBox( xstart + round(i),
             xstart + round(i+boxw) - 1, ystart, yend );
      i += step;
    }
  }
  else
  {
    int xmid = xstart + round((xend-xstart+1) * (g->value-g->vmin)/(g->vmax-g->vmin));

    if (g->value != g->vmin)
      cdBox( xstart, xmid, ystart, yend );

    if(g->show_text)
      drawtext(g, xmid);
  }
}

static int resize(Ihandle *h)
{
  Tgauge *g=(Tgauge *)iupGetEnv(h,"_IUPGAUGE_DATA");
  cdCanvas *old_one;

  if (g == NULL || !g->cdcanvas)
    return IUP_DEFAULT;

  old_one = cdActiveCanvas();
  cdActivate( g->cdcanvas );
  cdGetCanvasSize(&g->w,&g->h,NULL,NULL);

  if (g->im!=NULL)
  {
    cdKillImage(g->im);
    cdKillCanvas(g->cdim);
  }
  g->im=cdCreateImage(g->w,g->h);
  g->cdim=cdCreateCanvas(CD_IMAGE,g->im);

  cdActivate( old_one );
  return IUP_DEFAULT;
}

static int repaint(Ihandle *h)
{
  Tgauge *g=(Tgauge *)iupGetEnv(h,"_IUPGAUGE_DATA");
  cdCanvas *old_one;

  if (!g || g->cdcanvas == NULL)
    return IUP_DEFAULT;

  if (g->im == NULL) 
    resize(h);

  old_one = cdActiveCanvas();

  g->bgcolor = cdIupConvertColor(cdIupGetBgColor(h));
  cdIupCalcShadows(g->bgcolor, &g->light_shadow, &g->mid_shadow, &g->dark_shadow);

  g->fgcolor = cdIupConvertColor(IupGetAttribute(h, IUP_FGCOLOR));

  beginimage(g);
  drawgauge(g);
  putimage(g);

  if(old_one)
    cdActivate(old_one);

  return IUP_DEFAULT;
}

static int map_cb(Ihandle *h)
{
  Tgauge *g=(Tgauge *)iupGetEnv(h,"_IUPGAUGE_DATA");
  cdCanvas *old_one = cdActiveCanvas();

  if (g->cdcanvas==NULL)
  {
    g->cdcanvas=cdCreateCanvas(CD_IUP,h);

    cdActivate(g->cdcanvas);
    cdGetCanvasSize(&g->w,&g->h,NULL,NULL);

    resize(h);

    if(old_one)
      cdActivate(old_one);
  }

  return IUP_DEFAULT;
}

static void gaugedestroy(Ihandle *h)
{
  Tgauge *g=(Tgauge *)iupGetEnv(h,"_IUPGAUGE_DATA");
  if (g==NULL) return;
  if (!g->cdcanvas) return;

  if (g->im!=NULL) cdKillImage(g->im);
  if (g->cdim!=NULL) cdKillCanvas(g->cdim);
  if (g->cdcanvas!=NULL) cdKillCanvas(g->cdcanvas);

  iupSetEnv(h, "_IUPGAUGE_DATA", NULL);

  free(g);
}

Ihandle *IupGauge(void)
{
  return IupCreate("gauge");
}

static Ihandle *gaugecreate(Iclass *ic)
{
  Tgauge *g=(Tgauge*)malloc(sizeof(Tgauge));
  Ihandle *self = IupCanvas(NULL);

  iupSetEnv(self,"_IUPGAUGE_DATA",(char*)g); /* hangs valuator data in canvas */
  IupSetCallback(self,IUP_RESIZE_CB,(Icallback)resize);
  IupSetCallback(self,IUP_MAP_CB,(Icallback)map_cb);
  IupSetCallback(self,IUP_ACTION,(Icallback)repaint);
  iupSetEnv(self,IUP_EXPAND, IUP_NO);
  iupSetEnv(self,IUP_BORDER, IUP_NO);
  iupSetEnv(self,IUP_SCROLLBAR, IUP_NO);
  iupSetEnv(self,IUP_FGCOLOR, GDEFAULTCOLOR);
  iupSetEnv(self,IUP_SIZE, GDEFAULTSIZE);


  memset(g, 0, sizeof(Tgauge));

  g->fgcolor = cdIupConvertColor(GDEFAULTCOLOR);

  g->vmax = 1;
  g->bgcolor = CD_GRAY;
  g->light_shadow = CD_WHITE;
  g->mid_shadow = CD_GRAY;
  g->dark_shadow = CD_DARK_GRAY;

  g->show_text = 1;
  g->dashed    = 0;
  g->margin    = 1;

  return self;
}

static void crop_value(Tgauge *g)
{
  if (g->value>g->vmax) g->value=g->vmax;
  else if (g->value<g->vmin) g->value=g->vmin;
}

static void gaugesetattr(Ihandle *h, char *attr, char *value)
{
  Tgauge *g=(Tgauge *)iupGetEnv(h,"_IUPGAUGE_DATA");

  if(g == NULL)
  {
    iupCpiDefaultSetAttr(h, attr, value); /* chama metodo default */
    return;
  }

  if(iupStrEqual(attr,IUP_VALUE))  /* faz update no caso de setar o valor */
  {
    if(value == NULL)
      g->value = 0;
    else
      g->value = atof(value);
    crop_value(g);
  }
  else if(iupStrEqual(attr, IUP_MIN)) 
  {
    if(value==NULL) /* reseta para default */
      g->vmin = 0;
    else
      g->vmin = atof(value);
    crop_value(g);
  }
  else if(iupStrEqual(attr, IUP_MAX))
  {
    if(value==NULL) /* reseta para default */
      g->vmax = 1;
    else
      g->vmax = atof(value);
    crop_value(g);
  }
  else if(iupStrEqual(attr,ICTL_SHOW_TEXT))
  {
    if(iupStrEqual(value, IUP_YES))
      g->show_text = 1;
    else if(iupStrEqual(value, IUP_NO))
      g->show_text = 0;
  }
  else if(iupStrEqual(attr,ICTL_MARGIN))
  {
    if ( value == NULL ) g->margin = 1;
    else g->margin = atoi( value );
  }
  else if(iupStrEqual(attr,ICTL_DASHED))
  {
    if(iupStrEqual(value, IUP_YES))
      g->dashed = 1;
    else if(iupStrEqual(value, IUP_NO))
      g->dashed = 0;
  }
  else if(iupStrEqual(attr,ICTL_TEXT))
  {
    char *tmp ;

    if (value == NULL)
    {
      if(g->text != NULL)
        free(g->text);
      g->text = NULL;
    }
    else
    {
      tmp = (char*)iupStrDup(value);

      if(g->text != NULL)
        free(g->text);

      if(tmp != NULL)
        g->text = tmp;
    }
  }
  else if(iupStrEqual(attr, IUP_FONT) || iupStrEqual(attr, FONT_ATTR))
  {
    char *native = IupMapFont(value);
    char *tmp;

    if (!native) native = value;

    tmp = (char*)iupStrDup(native);

    if(g->typeface != NULL)
      free(g->typeface);

    if(tmp != NULL)
      g->typeface = tmp;
  }
  else
  {
    iupCpiDefaultSetAttr(h, attr, value); /* chama metodo default */
    return;
  }

  /* redesenha controle caso ele ja' tenha sido mapeado */
  if(g->cdcanvas != NULL)  
    repaint(h);
  else if(IupGetAttribute(h, IUP_WID) != NULL) 
    resize(h);
}

static char *gaugegetattr(Ihandle *h, char *attr)
{
  Tgauge *g = (Tgauge *)iupGetEnv(h,"_IUPGAUGE_DATA");

  if(g == NULL)
    return iupCpiDefaultGetAttr(h, attr); /* chama metodo default */

  if(iupStrEqual(attr, IUP_VALUE))
  {
    static char value[30];
    sprintf(value, "%g", g->value);
    return value;
  }
  else if(iupStrEqual(attr, ICTL_TEXT))
    return g->text;
  else if(iupStrEqual(attr, ICTL_SHOW_TEXT))
  {
    if(g->show_text)
      return IUP_YES;
    else
      return IUP_NO;
  }
  else if(iupStrEqual(attr, ICTL_MARGIN))
  { 
    static char buffer[ 10 ];
    sprintf( buffer, "%d", g->margin );
    return buffer;
  }
  else if(iupStrEqual(attr, ICTL_DASHED))
  {
    if(g->dashed)
      return IUP_YES;
    else
      return IUP_NO;
  }
  else
    return iupCpiDefaultGetAttr(h, attr); /* chama metodo default */
}

void IupGaugeOpen(void)
{
  Iclass* ICGauge = iupCpiCreateNewClass("gauge", NULL);

  iupCpiSetClassMethod(ICGauge, ICPI_CREATE, (Imethod) gaugecreate);
  iupCpiSetClassMethod(ICGauge, ICPI_DESTROY, (Imethod) gaugedestroy);
  iupCpiSetClassMethod(ICGauge, ICPI_SETATTR, (Imethod) gaugesetattr);
  iupCpiSetClassMethod(ICGauge, ICPI_GETATTR, (Imethod) gaugegetattr);
}
