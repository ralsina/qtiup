#include "cdqt.h"

#include <QPaintDevice>
#include <QPainter>
#include <QColor>


cdCanvasQt *_cdcnvQt = 0;

QColor color(unsigned long rgb)
{
  unsigned long pixel;
  int r,g,b;
  r = (unsigned short)((cdRed(rgb)*USHRT_MAX)/UCHAR_MAX);
  g = (unsigned short)((cdGreen(rgb)*USHRT_MAX)/UCHAR_MAX);
  b = (unsigned short)((cdBlue(rgb)*USHRT_MAX)/UCHAR_MAX);
  printf ("r=%d,g=%d,b=%d\n",r/256,g/256,b/256);
  return QColor (r/256,g/256,b/256);
}

static void cdQtpixel(int x, int y, long int color)
{
  printf ("cdQpixel\n");
  QPaintDevice *w=_cdcnvQt->widget;
  y=w->height()-y;
  QPainter p(w);
  p.drawPoint(x,y);
}

static void cdQtline(int x1, int y1, int x2, int y2)
{
  QPaintDevice *w=_cdcnvQt->widget;
  y1=w->height()-y1;
  y2=w->height()-y2;
  QPainter p(w);
  p.drawLine(x1,y1,x2,y2);
}


static void cdQtrect(int xmin, int xmax, int ymin, int ymax)
{
  printf ("cdQtrect\n");
  QPaintDevice *w=_cdcnvQt->widget;
  ymin=w->height()-ymin;
  ymax=w->height()-ymax;
  QPainter p(w);
  p.drawRect(xmin,ymin,xmax-xmin,ymax-ymin);
}


static void cdQtbox(int xmin, int xmax, int ymin, int ymax)
{
  printf ("cdQtbox\n");
  QPaintDevice *w=_cdcnvQt->widget;
  ymin=w->height()-ymin;
  ymax=w->height()-ymax;
  QPainter p(w);
  p.fillRect(xmin,ymin,xmax-xmin,ymax-ymin,_cdcnvQt->fg);
}

static long int cdQtbackground(long int c)
{
  printf ("cdQtbackground\n");
  QColor prev=_cdcnvQt->bg;
  if (c!=CD_QUERY)
  {
    _cdcnvQt->bg=color(c);
  }
  return cdEncodeColor(prev.red(),prev.blue(),prev.green());
}
static long int cdQtforeground(long int c)
{
  printf ("cdQtforeground %d\n",c);
  _cdcnvQt->fg=color(c);
  return c;
}

static void cdQtpoly(int mode, cdPoint* poly, int n)
{
  printf ("cdQtpoly\n");
  QPolygon pol (n);
  
  QPaintDevice *w=_cdcnvQt->widget;
  QPainter p(w);
  
  int h=w->height();
  
  for (int i = 0; i < n; i++)
  {
    pol.setPoint(i,poly[i].x,h-poly[i].y);
  }
  
  
  switch( mode )
  {
  case CD_FILL:
    //FIXME: see what the fillrule should be
    p.setBrush(_cdcnvQt->fg);
    p.drawPolygon(pol);
    break;
  //The following mades no sense. It mdified pnt which was freed immediately.
  //Unless mode could be CD_CLOSED and CD_OPEN at the same time, I guess.
/*    case CD_CLOSED_LINES: 
      pnt[n].x = pnt[0].x;
      pnt[n].y = pnt[0].y;
      n++;*/
      /* continua */
    case CD_OPEN_LINES:
    {
      p.drawPolyline(pol);
      break;
    }
    case CD_CLIP:
      //FIXME work on clipping
      break;
    case CD_BEZIER:
      //FIXME maybe should use Qt's cubic beziers.
      cdSimPolyBezier(poly, n);
      break;
  }

  
  
}

static void cdQtflush(void)
{
  printf ("Called cdStub cdQtflush\n");
}

static void cdQtarc(int xc, int yc, int w, int h, double a1, double a2)
{
  printf ("drawing arc, %f, %f\n",a1,a2);
  QPaintDevice *_w=_cdcnvQt->widget;
  QPainter p(_w);
  
  int left=xc-w/2;
  int top=_w->height()-(yc-h/2);
  
  p.drawArc(left,top,w,h,a1,a2);
}

static void cdQtsector(int xc, int yc, int w, int h, double a1, double a2)
{
  printf ("drawing sector, %f, %f\n",a1,a2);
  QPaintDevice *_w=_cdcnvQt->widget;
  QPainter p(_w);
  p.setBrush(_cdcnvQt->fg);
  int left=xc-w/2;
  int top=_w->height()-(yc-h/2);
  
  p.drawPie(left,top,w,h,a1,a2);
}

static void cdQtchord(int xc, int yc, int w, int h, double a1, double a2)
{
  printf ("Called cdStub cdQtchord\n");
}

static void cdQttext(int x, int y, char *s)
{
  printf ("drawing text %s\n",s);
  QPaintDevice *w=_cdcnvQt->widget;
  QPainter p(w);
  p.drawText(x,y,s);
}

static void cdQtfontdim(int *max_width, int *height, int *ascent, int *descent)
{
  printf ("Called cdStub cdQtfontdim\n");
}

static void cdQttextsize(char *s, int *width, int *height)
{
  printf ("Called cdStub cdQttextsize\n");
}

static void cdQtputimagerectrgb(int iw, int ih, unsigned char *r, unsigned char *g, unsigned char *b, int x, int y, int w, int h, int xmin, int xmax, int ymin, int ymax)
{
  if (xmin==xmax==ymin==ymax==0)
  {
    xmax=iw-1;
    ymax=ih-1;
  }
  
  if (w==h==0)
  {
    w=iw;
    h=ih;
  }

  QPixmap px(w,h);
  QPainter p(&px);

  for (int i=0;i<w;i++)
    for (int j=0;j<h;j++)
    {
      p.setPen(QColor(r[j*iw+1],g[j*iw+1],b[j*iw+1]));
      p.drawPoint(i,j);
    }
    
  p.drawPixmap(QRect(x,y,w,h),px,QRect(xmin,ymin,xmax-ymin,ymax-ymin));
}

static void cdQtputimagerectmap(int iw, int ih, unsigned char *index, long int *colors, int x, int y, int w, int h, int xmin, int xmax, int ymin, int ymax)
{
  printf ("Called cdStub cdQtputimagerectmap\n");
}

static void cdQtscrollarea (int xmin, int xmax, int ymin, int ymax, int dx, int dy)
{
  printf ("Called cdStub cdQtscrollarea\n");
}

static void cdnewregion(void)
{
  printf ("Called cdStub cdnewregion\n");
}

static int cdpointinregion(int x, int y)
{
  printf ("Called cdStub cdpointinregion\n");
}

static void cdoffsetregion(int x, int y)
{
  printf ("Called cdStub cdoffsetregion\n");
}

static void cdregionbox(int *xmin, int *xmax, int *ymin, int *ymax)
{
  printf ("Called cdStub cdregionbox\n");
}

static int cdQtclip(int clip_mode)
{
  printf ("Called cdStub cdQtclip\n");
}

static void cdQtcliparea(int *xmin, int *xmax, int *ymin, int *ymax)
{
  printf ("Called cdStub cdQtcliparea\n");
}

static int cdQtwritemode(int write_mode)
{
  printf ("Called cdStub cdQtwritemode\n");
}

static int cdQtlinestyle(int style)
{
  printf ("Called cdStub cdQtlinestyle\n");
}

static int cdQtlinewidth(int width)
{
  printf ("Called cdStub cdQtlinewidth\n");
}

static int cdQtlinecap(int cap)
{
  printf ("Called cdStub cdQtlinecap\n");
}

static int cdQtlinejoin(int join)
{
  printf ("Called cdStub cdQtlinejoin\n");
}

static int cdQtbackopacity(int opaque)
{
  printf ("Called cdStub cdQtbackopacity\n");
}

static int cdQtinteriorstyle(int style)
{
  printf ("Called cdStub cdQtinteriorstyle\n");
}

static int cdQthatch(int hatch_style)
{
  printf ("Called cdStub cdQthatch\n");
}

static void cdQtstipple(int w, int h, unsigned char *data)
{
  printf ("Called cdStub cdQtstipple\n");
}

static void cdQtpattern(int w, int h, long int *colors)
{
  printf ("Called cdStub cdQtpattern\n");
}

static void cdQtfont(int *type_face, int *style, int *size)
{
  printf ("Called cdStub cdQtfont\n");
}

static void cdQtnativefont(char* font_name)
{
  printf ("Called cdStub cdQtnativefont\n");
}

static void cdQtpalette(int n, long int *palette, int mode)
{
  printf ("Called cdStub cdQtpalette\n");
}

static void cdQtgetimagergb(unsigned char *r, unsigned char *g, unsigned char *b, int x, int y, int w, int h)
{
  printf ("Called cdStub cdQtgetimagergb\n");
}

static void *cdQtcreateimage (int w, int h)
{
  cdxImage *xi=new cdxImage;
  xi->img=new QPixmap (w,h);
  return static_cast <void *> (xi);
}

static void cdQtgetimage (void *image, int x, int y)
{
  printf ("Called cdStub cdQtgetimage\n");
}

static void cdQtputimagerect (void *image, int x, int y, int xmin, int xmax, int ymin, int ymax)
{
  printf ("cdQtputimagerect\n");
  cdxImage *xi=((cdxImage *)image);
  QPixmap *pix = xi->img;
  if (!pix)
  {
    printf ("empty pixmap\n");
    return;
  }
  QPaintDevice *w=_cdcnvQt->widget;
  QPainter p(w);
  
  p.drawPixmap (x,y,*pix,xmin,ymin,xmax-xmin,ymax-ymin);
  
}

static void cdQtkillimage (void *image)
{
  printf ("Called cdStub cdQtkillimage\n");
}

static void cdQtputimagerectrgba(int iw, int ih, unsigned char *r, unsigned char *g, unsigned char *b, unsigned char *a, int x, int y, int w, int h, int xmin
, int xmax, int ymin, int ymax)
{
  printf ("Called cdStub cdQtputimagerectrgba\n");
}



void cdQtinittable(cdPrivateCanvas* cnv)
{
  cnv->Flush = cdQtflush;
  cnv->Pixel = cdQtpixel;
  cnv->Line = cdQtline;
  cnv->Poly = cdQtpoly;
  cnv->Rect = cdQtrect;
  cnv->Box = cdQtbox;
  cnv->Arc = cdQtarc;
  cnv->Sector = cdQtsector;
  cnv->Chord = cdQtchord;
  cnv->Text = cdQttext;
  cnv->FontDim = cdQtfontdim;
  cnv->TextSize = cdQttextsize;
  cnv->PutImageRectRGB = cdQtputimagerectrgb;
  cnv->PutImageRectMap = cdQtputimagerectmap;
  cnv->ScrollArea = cdQtscrollarea;
  cnv->NewRegion = cdnewregion;
  cnv->PointInRegion = cdpointinregion;
  cnv->OffsetRegion = cdoffsetregion;
  cnv->RegionBox = cdregionbox;

  cnv->Clip = cdQtclip;
  cnv->ClipArea = cdQtcliparea;
  cnv->WriteMode = cdQtwritemode;
  cnv->LineStyle = cdQtlinestyle;
  cnv->LineWidth = cdQtlinewidth;
  cnv->LineCap = cdQtlinecap;
  cnv->LineJoin = cdQtlinejoin;
  cnv->BackOpacity = cdQtbackopacity;
  cnv->InteriorStyle = cdQtinteriorstyle;
  cnv->Hatch = cdQthatch;
  cnv->Stipple = cdQtstipple;
  cnv->Pattern = cdQtpattern;
  cnv->Font = cdQtfont;
  cnv->NativeFont = cdQtnativefont;
  cnv->Palette = cdQtpalette;
  cnv->Background = cdQtbackground;
  cnv->Foreground = cdQtforeground;
  cnv->GetImageRGB = cdQtgetimagergb;
  cnv->CreateImage = cdQtcreateimage;
  cnv->GetImage = cdQtgetimage;
  cnv->PutImageRect = cdQtputimagerect;
  cnv->KillImage = cdQtkillimage;

  if (cnv->bpp > 8)
    cnv->PutImageRectRGBA = cdQtputimagerectrgba;
}
