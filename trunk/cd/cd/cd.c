char *cd_c = "$Id: cd.c,v 1.14 2005/12/09 16:33:30 scuri Exp $";
/*
* cd.c
* common code for cd
*/
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <assert.h>
#include <memory.h>
#include <math.h>
#include <stdarg.h>


#include "cd.h"
#include "wd.h"
#include "cdprivat.h"
#include "cdirgb.h"


static cdPrivateCanvas *active_private_canvas = NULL;
static cdCanvas *active_canvas = NULL;

#define _cdInvertYAxis(_) (active_private_canvas->h - (_) - 1)

/* troca os valores de duas variaveis inteiras */
#define SWAPINT(a,b) {int c=a;a=b;b=c;}

/* Arredonda um valor real para inteiro mais proximo */
#define ROUND(_) (int) floor((_)+0.5)

/***********/
/* default */
/***********/

#define unus(_) ((void)(_))
static int  cdDefUpdate(void) {return CD_OK;}
static void cdDefFlush(void) {}
static void cdDefClear(void) {}
static void cdDefPixel(int x, int y, long int color) {unus(color); unus(x); unus(y);}
static void cdDefLine(int x1, int y1, int x2, int y2) {unus(x1); unus(y1);unus(x2); unus(y2);}
static void cdDefPoly(int mode, cdPoint* points, int n) {unus(mode); unus(points); unus(n);}
static void cdDefBox(int xmin, int xmax, int ymin, int ymax) {unus(xmin); unus(ymin);unus(xmax); unus(ymax);}
static void cdDefArc(int xc, int yc, int w, int h, double angle1, double angle2) {unus(w);unus(h);unus(xc); unus(yc);unus(angle1);unus(angle2);}
static void cdDefSector(int xc, int yc, int w, int h, double angle1, double angle2) {unus(w);unus(h);unus(xc); unus(yc);unus(angle1);unus(angle2);}
static void cdDefChord(int xc, int yc, int w, int h, double angle1, double angle2) {unus(w);unus(h);unus(xc); unus(yc);unus(angle1);unus(angle2);}
static void cdDefText(int x, int y, char *s) {unus(x); unus(y);unus(s);}
static void cdDefPutImageRectRGB(int iw, int ih, unsigned char *r, unsigned char *g, unsigned char *b, int x, int y, int w, int h, int xmin, int xmax, int ymin, int ymax) {unus(x); unus(y);unus(xmin); unus(ymin);unus(xmax); unus(ymax);unus(b);unus(g); unus(r);unus(w);unus(h);unus(iw);unus(ih);}
static void cdDefPutImageRectMap(int iw, int ih, unsigned char *index, long int *colors, int x, int y, int w, int h, int xmin, int xmax, int ymin, int ymax) {unus(x); unus(y);unus(xmin); unus(ymin);unus(xmax); unus(ymax);unus(w);unus(h);unus(iw);unus(ih);unus(index);unus(colors);}
static void cdDefScrollArea(int xmin, int xmax, int ymin, int ymax, int dx, int dy) {unus(xmin); unus(ymin);unus(xmax); unus(ymax);unus(dy);unus(dx);}
static void cdDefClipArea(int* xmin, int* xmax, int* ymin, int* ymax) {unus(xmin); unus(ymin);unus(xmax); unus(ymax);}
static void* cdDefCreateImage(int w, int h) {unus(w);unus(h);return NULL;}
static void cdDefGetImage(void* image, int x, int y) {unus(x); unus(y);unus(image);}
static void cdDefPutImageRect(void* image, int x, int y, int xmin, int xmax, int ymin, int ymax) {unus(x); unus(y);unus(xmin); unus(ymin);unus(xmax); unus(ymax);unus(image);}
static void cdDefKillImage(void* image) {unus(image);}
static void cdDefwdCall(int usage) {unus(usage);}
static void cdDefNativeFont(char* font) {unus(font);}
static void cdDefPalette(int n, long int *palette, int mode) {unus(mode);unus(n);unus(palette);}
static void cdDefNewRegion(void) {}
static int  cdDefPointInRegion(int x, int y) {unus(x);unus(y); return 0;}
static void cdDefOffsetRegion(int x, int y) {unus(x);unus(y);}
static void cdDefRegionBox(int *xmin, int *xmax, int *ymin, int *ymax) {unus(xmin);unus(xmax);unus(ymin);unus(ymax);}

static int cdDefClip(int mode) {return mode;}
static int cdDefBackOpacity(int opacity) {return opacity;}
static int cdDefWriteMode(int mode) {return mode;}
static int cdDefMarkType(int type) {return type;}
static int cdDefMarkSize(int size) {return size;}
static int cdDefLineStyle(int style) {return style;}
static int cdDefLineWidth(int width) {return width;}
static int cdDefLineCap(int cap) {return cap;}
static int cdDefLineJoin(int join) {return join;}
static int cdDefInteriorStyle(int style) {return style;}
static int cdDefHatch(int style) {return style;}
static void cdDefStipple(int w, int h, unsigned char *stipple) {unus(w); unus(h);unus(stipple);}
static void cdDefPattern(int w, int h, long int *pattern) {unus(w); unus(h);unus(pattern);}
static int cdDefTextAlignment(int alignment) {return alignment;}
static double cdDefTextOrientation(double angle) {return angle;}
static long int cdDefBackground(long int color) {return color;}
static long int cdDefForeground(long int color) {return color;}
static void cdDefFont(int *type_face, int *style, int *size) {unus(type_face);unus(style);unus(size);}
#undef unus

static void set_default_func(cdPrivateCanvas* private_canvas)
{
  /* set default functions */

  private_canvas->Update = cdDefUpdate;
  private_canvas->wdCall = cdDefwdCall;

  private_canvas->Flush = cdDefFlush;
  private_canvas->Clear = cdDefClear;
  private_canvas->Pixel = cdDefPixel;
  private_canvas->Line = cdDefLine;
  private_canvas->Poly = cdDefPoly;
  private_canvas->Box = cdDefBox;
  private_canvas->Arc = cdDefArc;
  private_canvas->Sector = cdDefSector;
  private_canvas->Chord = cdDefChord;
  private_canvas->Text = cdDefText;
  private_canvas->PutImageRectRGB = cdDefPutImageRectRGB;
  private_canvas->PutImageRectMap = cdDefPutImageRectMap;
  private_canvas->ScrollArea = cdDefScrollArea;
  private_canvas->Clip = cdDefClip;
  private_canvas->ClipArea = cdDefClipArea;
  private_canvas->BackOpacity = cdDefBackOpacity;
  private_canvas->WriteMode = cdDefWriteMode;
  private_canvas->MarkType = cdDefMarkType;
  private_canvas->MarkSize = cdDefMarkSize;
  private_canvas->LineStyle = cdDefLineStyle;
  private_canvas->LineWidth = cdDefLineWidth;
  private_canvas->LineCap = cdDefLineCap;
  private_canvas->LineJoin = cdDefLineJoin;
  private_canvas->InteriorStyle = cdDefInteriorStyle;
  private_canvas->Hatch = cdDefHatch;
  private_canvas->Stipple = cdDefStipple;
  private_canvas->Pattern = cdDefPattern;
  private_canvas->Font = cdDefFont;
  private_canvas->NativeFont = cdDefNativeFont;
  private_canvas->TextAlignment = cdDefTextAlignment;
  private_canvas->TextOrientation = cdDefTextOrientation;
  private_canvas->Palette = cdDefPalette;
  private_canvas->Background = cdDefBackground;
  private_canvas->Foreground = cdDefForeground;
  private_canvas->CreateImage = cdDefCreateImage;
  private_canvas->GetImage = cdDefGetImage;
  private_canvas->PutImageRect = cdDefPutImageRect;
  private_canvas->KillImage = cdDefKillImage;
  private_canvas->NewRegion = cdDefNewRegion;
  private_canvas->PointInRegion = cdDefPointInRegion;
  private_canvas->OffsetRegion = cdDefOffsetRegion;
  private_canvas->RegionBox = cdDefRegionBox;

  /* default simulation functions */
  private_canvas->TextSize = cdSimTextSizeEx;
  private_canvas->FontDim = cdSimFontDimEx;
  private_canvas->Mark = cdSimMark;
  private_canvas->Rect = cdSimRect;
  private_canvas->PutImageRectRGBA = cdSimPutImageRectRGBA;


/*
  -- NULL functions to indicate unsupported features
  GetImageRGB

  -- NULL functions to use vector text simulation
  VectorText
  MultiLineVectorText
  VectorFont
  VectorTextDirection
  VectorTextTransform
  VectorTextSize
  VectorCharSize

  -- NULL functions to use WC simulation
  Window
  Viewport
  wMark
  wLine
  wPoly
  wRect
  wBox
  wArc
  wSector
  wText
  wVectorText
  wMultiLineVectorText
  wClipArea
  wVectorTextDirection
  wVectorTextSize
  wVectorCharSize

  -- can NOT be NULL 
  KillCanvas
  Activate
  Deactivate
*/
}

static void set_default_attrib(cdPrivateCanvas* private_canvas)
{
  /* clipping attributes */
  private_canvas->clip_mode = CD_CLIPOFF;

  /* color attributes */
  private_canvas->foreground = CD_BLACK;
  private_canvas->background = CD_WHITE;

  private_canvas->back_opacity = CD_TRANSPARENT;
  private_canvas->write_mode = CD_REPLACE;

  /* primitive attributes */
  private_canvas->mark_type = CD_STAR;
  private_canvas->mark_size = 10;

  private_canvas->line_width = 1;
  private_canvas->line_style = CD_CONTINUOUS;
  private_canvas->line_cap = CD_CAPFLAT;
  private_canvas->line_join = CD_MITER;

  private_canvas->hatch_style = CD_HORIZONTAL;
  private_canvas->interior_style = CD_SOLID;
  private_canvas->fill_mode = CD_EVENODD;

  private_canvas->font_type_face = CD_SYSTEM;
  private_canvas->font_style     = CD_PLAIN;
  private_canvas->font_size      = CD_STANDARD;

  private_canvas->text_alignment = CD_BASE_LEFT;

  /* o resto recebeu zero no memset */
}

static void update_attrib(cdCanvas* canvas, cdPrivateCanvas* private_canvas)
{
  cdCanvas* old_active_canvas = cdActiveCanvas();
  
  cdActivate(canvas);

  private_canvas->Background(private_canvas->background);
  private_canvas->Foreground(private_canvas->foreground);
  private_canvas->BackOpacity(private_canvas->back_opacity);
  private_canvas->WriteMode(private_canvas->write_mode);
  private_canvas->LineStyle(private_canvas->line_style);
  private_canvas->LineWidth(private_canvas->line_width);
  private_canvas->LineCap(private_canvas->line_cap);
  private_canvas->LineJoin(private_canvas->line_join);
  private_canvas->Hatch(private_canvas->hatch_style);
  private_canvas->InteriorStyle(private_canvas->interior_style);
  private_canvas->Font(&private_canvas->font_type_face, &private_canvas->font_style, &private_canvas->font_size);
  private_canvas->TextAlignment(private_canvas->text_alignment);
  private_canvas->TextOrientation(private_canvas->text_orientation);
  private_canvas->MarkType(private_canvas->mark_type);
  private_canvas->MarkSize(private_canvas->mark_size);

  cdActivate(old_active_canvas);
}

static int check_box(int xmin, int xmax, int ymin, int ymax)
{
  if (xmin > xmax) SWAPINT(xmin, xmax);
  if (ymin > ymax) SWAPINT(ymin, ymax);

  if ((xmax-xmin+1) <= 0)
    return 0;

  if ((ymax-ymin+1) <= 0)
    return 0;

  return 1;
}

/***********/
/* control */
/***********/

const char cd_ident[] =
  "$CD: " CD_VERSION " " CD_COPYRIGHT " $\n"
  "$URL: www.tecgraf.puc-rio.br/cd $\n";

static char *tecver = "TECVERID.str:CD:LIB:"CD_VERSION;

char* cdVersion(void)
{              
  (void)cd_ident;
	(void)tecver;
  return CD_VERSION;
}

char* cdVersionDate(void)
{
  return CD_VERSION_DATE;
}
 
int cdVersionNumber(void)
{
  return CD_VERSION_NUMBER;
}

cdCanvas* cdCreateCanvasf(cdContext *context, char* format, ...)
{
  char data[1024];
  va_list arglist;
  va_start(arglist, format);
  vsprintf(data, format, arglist);

  return cdCreateCanvas(context, data);
}

cdCanvas *cdCreateCanvas(cdContext* context, void *data_str)
{
  cdCanvas *canvas;
  cdPrivateCanvas *private_canvas;
  cdPrivateContext* ctx;

  /* usefull for NULL drivers, that do nothing and exist only for portability */
  if (!context)
    return NULL;

  {
    static int first = 1;
    char* env = getenv("CD_QUIET");
    if (first && env && strcmp(env, "NO")==0)
    {
      printf("CD  "CD_VERSION" "CD_COPYRIGHT"\n");
      first = 0;
    }
  }

  /* alocates and initialize everything with 0s */
  canvas = (cdCanvas*)malloc(sizeof(cdCanvas));

  canvas->cnv = malloc(sizeof(cdPrivateCanvas));
  memset(canvas->cnv, 0, sizeof(cdPrivateCanvas));
  private_canvas = (cdPrivateCanvas*)canvas->cnv;

  private_canvas->vector_font_data = cdCreateVectorFont();
  private_canvas->sim_data = cdCreateSimulation(private_canvas);

  private_canvas->canvas = canvas;
  private_canvas->context = context;
  ctx = (cdPrivateContext*)context->ctx;

  /* initialize default attributes, must be before creating the canvas */
  set_default_attrib(private_canvas);

  ctx->CreateCanvas(private_canvas, data_str);
  if (!private_canvas->context_canvas)
  {
    free(private_canvas->vector_font_data);
    free(private_canvas);
    free(canvas);
    return NULL;
  }

  /* functions that can do nothing, must be before InitTable */
  set_default_func(private_canvas);

  /* initialize canvas table */
  ctx->InitTable(private_canvas);

  /* update the default atributes, must be after InitTable */
  update_attrib(canvas, private_canvas);

  /* must be after creating the canvas, so that we know canvas width and height */
  private_canvas->clip_xmax = private_canvas->w-1;
  private_canvas->clip_ymax = private_canvas->h-1;
  wdSetDefaults(private_canvas);

  return canvas;
}

void cdKillCanvas(cdCanvas *canvas)
{
  cdPrivateCanvas* private_canvas;

  assert(canvas);
  assert(canvas->cnv);

  private_canvas = (cdPrivateCanvas*)canvas->cnv;

  if (canvas == active_canvas)
  {
    active_canvas = NULL;
    active_private_canvas = NULL;
  }
  
  private_canvas->KillCanvas(private_canvas->context_canvas);

  if (private_canvas->pattern) free(private_canvas->pattern);
  if (private_canvas->stipple) free(private_canvas->stipple);
  if (private_canvas->poly) free(private_canvas->poly);
  if (private_canvas->clip_poly) free(private_canvas->clip_poly);
  if (private_canvas->wpoly) free(private_canvas->wpoly);
  if (private_canvas->clip_wpoly) free(private_canvas->clip_wpoly);
  if (private_canvas->line_dashes) free(private_canvas->line_dashes);

  cdKillVectorFont(private_canvas->vector_font_data);

  cdKillSimulation(private_canvas->sim_data);

  free(private_canvas);
  free(canvas);
}

cdContext* cdGetContext(cdCanvas *canvas)
{
  cdPrivateCanvas* private_canvas;

  assert(canvas);
  assert(canvas->cnv);

  private_canvas = (cdPrivateCanvas*)canvas->cnv;

  return private_canvas->context;
}

unsigned long cdContextCaps(cdContext *context)
{
  cdPrivateContext* ctx;

  if (!context)
    return CD_CAP_NONE;

  ctx = (cdPrivateContext*)context->ctx;

  return ctx->caps;
}

int cdActivate(cdCanvas *canvas)
{
  /* if is the active canvas, just update canvas state */
  if (active_canvas && canvas == active_canvas)
  {
    if (active_private_canvas->Update() == CD_ERROR)
    {
      active_canvas = NULL;
      active_private_canvas = NULL;
      wdUpdateActive(NULL);
      cdActivateVectorFont(NULL);
      cdActivateSim(NULL);
      return CD_ERROR;
    }

    return CD_OK;
  }

  /* if exists an active canvas, deactivate it */
  if (active_private_canvas) 
    active_private_canvas->Deactivate();
  
  /* allow to active a NULL canvas, the user may restore a previous canvas that was NULL */
  if (canvas == NULL)
  {
    active_canvas = NULL;
    active_private_canvas = NULL;
    wdUpdateActive(NULL);
    cdActivateVectorFont(NULL);
    cdActivateSim(NULL);
    return CD_ERROR;
  }

  /* do the activation */
  active_canvas = canvas;
  active_private_canvas = (cdPrivateCanvas*)canvas->cnv;
  wdUpdateActive(active_private_canvas);
  cdActivateVectorFont(active_private_canvas);
  cdActivateSim(active_private_canvas);
  
  if (active_private_canvas->Activate(active_private_canvas->context_canvas) == CD_ERROR)
  {
    active_canvas = NULL;
    active_private_canvas = NULL;
    wdUpdateActive(NULL);
    cdActivateVectorFont(NULL);
    cdActivateSim(NULL);
    return CD_ERROR;
  }
  
  return CD_OK;
}

cdCanvas* cdActiveCanvas(void)
{
  return active_canvas;
}

int cdSimulate(int mode)
{
  int sim_mode;
  cdPrivateContext* ctx;

  assert(active_private_canvas);

  ctx = (cdPrivateContext*)active_private_canvas->context->ctx;

  sim_mode = active_private_canvas->sim_mode;
  if (mode == CD_QUERY || cdGetContext(active_canvas) == CD_IMAGERGB)
    return sim_mode;

  active_private_canvas->sim_mode = mode;
  if (mode == CD_SIM_NONE)
    return sim_mode;

  set_default_func(active_private_canvas);
  ctx->InitTable(active_private_canvas);

  if (mode & CD_SIM_LINE)
  {
    active_private_canvas->Line = cdSimLine;
    active_private_canvas->wLine = NULL;
  }

  if (mode & CD_SIM_MARK)
  {
    active_private_canvas->Mark = cdSimMark;
    active_private_canvas->wMark = NULL;
  }

  if (mode & CD_SIM_RECT)
  {
    active_private_canvas->Rect = cdSimRect;
    active_private_canvas->wRect = NULL;
  }

  if (mode & CD_SIM_BOX)
  {
    active_private_canvas->Box = cdSimBox;
    active_private_canvas->wBox = NULL;
  }

  if (mode & CD_SIM_ARC)
  {
    active_private_canvas->Arc = cdSimArc;
    active_private_canvas->wArc = NULL;
  }

  if (mode & CD_SIM_SECTOR)
  {
    active_private_canvas->Sector = cdSimSector;
    active_private_canvas->wSector = NULL;
  }

  if (mode & CD_SIM_CHORD)
  {
    active_private_canvas->Chord = cdSimChord;
    active_private_canvas->wChord = NULL;
  }

  if (mode & CD_SIM_TEXT)
  {
    active_private_canvas->Text = cdSimText;
    active_private_canvas->wText = NULL;
    active_private_canvas->NativeFont = cdSimNativeFont;
    active_private_canvas->Font = cdSimFont;
    active_private_canvas->FontDim = cdSimFontDim;
    active_private_canvas->TextSize = cdSimTextSize;
    active_private_canvas->TextOrientation = cdDefTextOrientation;

    cdSimInitText(active_private_canvas->sim_data);
    cdSimFont(&active_private_canvas->font_type_face, &active_private_canvas->font_style, &active_private_canvas->font_size);
  }
  else
    cdFont(active_private_canvas->font_type_face, active_private_canvas->font_style, active_private_canvas->font_size);

  if (mode & CD_SIM_POLYLINE || mode & CD_SIM_POLYGON || mode & CD_SIM_CLIPPOLY)
    active_private_canvas->wPoly = NULL;

  if (mode & CD_SIM_CLIP)
  {
    active_private_canvas->Clip(CD_CLIPOFF);
    active_private_canvas->Clip = cdDefClip;
    active_private_canvas->ClipArea = cdDefClipArea;
    active_private_canvas->wClipArea = NULL;

    if (active_private_canvas->clip_poly)
      cdSimPoly(CD_CLIP, active_private_canvas->clip_poly, active_private_canvas->clip_poly_n);
  }
  else
  {
    if (active_private_canvas->clip_poly)
      active_private_canvas->Poly(CD_CLIP, active_private_canvas->clip_poly, active_private_canvas->clip_poly_n);

    active_private_canvas->Clip(active_private_canvas->clip_mode);
  }

  if (mode & CD_SIM_WD)
  {
    active_private_canvas->wRect = NULL;
    active_private_canvas->wLine = NULL;
    active_private_canvas->wMark = NULL;
    active_private_canvas->wBox = NULL;
    active_private_canvas->wArc = NULL;
    active_private_canvas->wSector = NULL;
    active_private_canvas->wChord = NULL;
    active_private_canvas->wPoly = NULL;
    active_private_canvas->wText = NULL;
    active_private_canvas->wVectorText = NULL;
    active_private_canvas->wMultiLineVectorText = NULL;
    active_private_canvas->wVectorTextDirection = NULL;
    active_private_canvas->wVectorTextSize = NULL;
    active_private_canvas->wVectorCharSize = NULL;
    active_private_canvas->Window = NULL;
    active_private_canvas->Viewport = NULL;
    active_private_canvas->wClipArea = NULL;
  }

  if (mode & CD_SIM_VECTORTEXT)
  {
    active_private_canvas->VectorText = NULL;
    active_private_canvas->MultiLineVectorText = NULL;
    active_private_canvas->VectorFont = NULL;
    active_private_canvas->VectorTextTransform = NULL;
    active_private_canvas->VectorTextDirection = NULL;
    active_private_canvas->VectorTextSize = NULL;
    active_private_canvas->VectorCharSize = NULL;
    active_private_canvas->wVectorText = NULL;
    active_private_canvas->wMultiLineVectorText = NULL;
    active_private_canvas->wVectorTextDirection = NULL;
    active_private_canvas->wVectorTextSize = NULL;
    active_private_canvas->wVectorCharSize = NULL;
  }

  cdSimUpdateFillLine();

  return sim_mode;
}

cdState* cdSaveState(void)
{
  cdPrivateCanvas *state_canvas;
  cdState* state;

  state = (cdState*)malloc(sizeof(cdState));
  state->ste = (cdPrivateCanvas*)malloc(sizeof(cdPrivateCanvas));
  memcpy(state->ste, active_private_canvas, sizeof(cdPrivateCanvas));
  state_canvas = (cdPrivateCanvas*)state->ste;

  if (state_canvas->pattern) 
  {
    int size = state_canvas->pattern_w*state_canvas->pattern_h*sizeof(long int);
    state_canvas->pattern = (long int*)malloc(size);
    memcpy(state_canvas->pattern, active_private_canvas->pattern, size);
  }

  if (state_canvas->stipple) 
  {
    int size = state_canvas->stipple_w*state_canvas->stipple_h;
    state_canvas->stipple = (unsigned char*)malloc(size);
    memcpy(state_canvas->stipple, active_private_canvas->stipple, size);
  }

  if (state_canvas->clip_poly) 
  {
    int size = state_canvas->clip_poly_n*sizeof(cdPoint);
    state_canvas->clip_poly = (cdPoint*)malloc(size);
    memcpy(state_canvas->clip_poly, active_private_canvas->clip_poly, size);
  }

  if (state_canvas->clip_wpoly) 
  {
    int size = state_canvas->clip_wpoly_n*sizeof(wdPoint);
    state_canvas->clip_wpoly = (wdPoint*)malloc(size);
    memcpy(state_canvas->clip_wpoly, active_private_canvas->clip_wpoly, size);
  }

  if (state_canvas->line_dashes) 
  {
    int size = state_canvas->line_dashes_count*sizeof(int);
    state_canvas->line_dashes = (int*)malloc(size);
    memcpy(state_canvas->line_dashes, active_private_canvas->line_dashes, size);
  }

  return state;
}

void cdReleaseState(cdState* state)
{
  cdPrivateCanvas *state_canvas = (cdPrivateCanvas*)state->ste;

  if (state_canvas->stipple) 
    free(state_canvas->stipple);

  if (state_canvas->pattern) 
    free(state_canvas->pattern);

  if (state_canvas->clip_poly) 
    free(state_canvas->clip_poly);

  if (state_canvas->clip_wpoly) 
    free(state_canvas->clip_wpoly);

  if (state_canvas->line_dashes) 
    free(state_canvas->line_dashes);

  free(state_canvas);
  free(state);
}

void cdRestoreState(cdState* state)
{
  cdPrivateCanvas *state_canvas = (cdPrivateCanvas*)state->ste;

  active_private_canvas->clip_poly_n = state_canvas->clip_poly_n;
  if (active_private_canvas->clip_poly) 
  {
    free(active_private_canvas->clip_poly);
    active_private_canvas->clip_poly = NULL;
  }

  active_private_canvas->clip_wpoly_n = state_canvas->clip_wpoly_n;
  if (active_private_canvas->clip_wpoly) 
  {
    free(active_private_canvas->clip_wpoly);
    active_private_canvas->clip_wpoly = NULL;
  }

  if (state_canvas->line_dashes) 
  {
    free(active_private_canvas->line_dashes);
    active_private_canvas->line_dashes = NULL;
  }

  if (state_canvas->clip_poly) 
  {
    int size = state_canvas->clip_poly_n*sizeof(cdPoint);
    active_private_canvas->clip_poly = (cdPoint*)malloc(size);
    memcpy(active_private_canvas->clip_poly, state_canvas->clip_poly, size);
  }

  if (state_canvas->clip_wpoly) 
  {
    int size = state_canvas->clip_wpoly_n*sizeof(wdPoint);
    active_private_canvas->clip_wpoly = (wdPoint*)malloc(size);
    memcpy(active_private_canvas->clip_wpoly, state_canvas->clip_wpoly, size);
  }

  if (state_canvas->line_dashes) 
  {
    int size = state_canvas->line_dashes_count*sizeof(int);
    active_private_canvas->line_dashes = (int*)malloc(size);
    memcpy(active_private_canvas->line_dashes, state_canvas->line_dashes, size);
  }

  cdClip(CD_CLIPOFF);
  if (active_private_canvas->clip_poly)
    active_private_canvas->Poly(CD_CLIP, active_private_canvas->clip_poly, active_private_canvas->clip_poly_n);
  if (active_private_canvas->clip_wpoly && active_private_canvas->wPoly)
    active_private_canvas->wPoly(CD_CLIP, active_private_canvas->clip_wpoly, active_private_canvas->clip_wpoly_n);
  cdClipArea(state_canvas->clip_xmin, state_canvas->clip_xmax, state_canvas->clip_ymin, state_canvas->clip_ymax);
  cdClip(state_canvas->clip_mode);
  cdBackground(state_canvas->background);
  cdForeground(state_canvas->foreground);
  cdBackOpacity(state_canvas->back_opacity);
  cdWriteMode(state_canvas->write_mode);
  cdLineStyle(state_canvas->line_style);
  cdLineWidth(state_canvas->line_width);
  cdLineCap(state_canvas->line_cap);
  cdLineJoin(state_canvas->line_join);
  cdFillMode(state_canvas->fill_mode);
  cdHatch(state_canvas->hatch_style);
  if (state_canvas->stipple) cdStipple(state_canvas->stipple_w, state_canvas->stipple_h, state_canvas->stipple);
  if (state_canvas->pattern) cdPattern(state_canvas->pattern_w, state_canvas->pattern_h, state_canvas->pattern);
  cdInteriorStyle(state_canvas->interior_style);
  cdFont(state_canvas->font_type_face, state_canvas->font_style, state_canvas->font_size);
  cdNativeFont(state_canvas->native_font);
  cdTextAlignment(state_canvas->text_alignment);
  cdTextOrientation(state_canvas->text_orientation);
  cdMarkType(state_canvas->mark_type);
  cdMarkSize(state_canvas->mark_size);
  cdOrigin(state_canvas->origin_x, state_canvas->origin_y);
  wdWindow(state_canvas->win_xmin, state_canvas->win_xmax, state_canvas->win_ymin, state_canvas->win_ymax);
  wdViewport(state_canvas->view_xmin, state_canvas->view_xmax, state_canvas->view_ymin, state_canvas->view_ymax);
  cdSimulate(state_canvas->sim_mode);
}

void cdFlush(void)
{
  assert(active_private_canvas);
  active_private_canvas->Flush();
}

void cdClear(void)
{
  assert(active_private_canvas);
  active_private_canvas->Clear();
}

static cdAttribute* FindAttrib(cdPrivateCanvas *private_canvas, char* name, int *a)
{
  int i;

  for (i=0; i < private_canvas->attrib_n; i++)
  {
    if (strcmp(name, private_canvas->attrib_list[i]->name) == 0)
    {
      if (a) *a = i;
      return private_canvas->attrib_list[i];
    }
  }

  return NULL;
}

void cdRegisterAttribute(cdPrivateCanvas *private_canvas, cdAttribute* attrib)
{
  cdAttribute* old_attrib;
  int a;

  assert(private_canvas);

  old_attrib = FindAttrib(private_canvas, attrib->name, &a);

  if (old_attrib)
    private_canvas->attrib_list[a] = attrib;
  else
  {
    private_canvas->attrib_list[private_canvas->attrib_n] = attrib;
    private_canvas->attrib_n++;
  }
}

void cdSetAttribute(char* name, char *data)
{
  cdAttribute* attrib;

  assert(active_private_canvas);

  attrib = FindAttrib(active_private_canvas, name, NULL);
  if (attrib && attrib->set)
    attrib->set(data);
}

void cdSetfAttribute(char* name, char* format, ...)
{
  char data[1024];
  va_list arglist;
  va_start(arglist, format);
  vsprintf(data, format, arglist);

  cdSetAttribute(name, data);
}

char* cdGetAttribute(char* name)
{
  cdAttribute* attrib;

  assert(active_private_canvas);

  attrib = FindAttrib(active_private_canvas, name, NULL);
  if (attrib && attrib->get)
    return attrib->get();

  return NULL;
}

int cdInvertYAxis(int y)
{                         
  assert(active_private_canvas);
  return active_private_canvas->h - y - 1;
}

/******************/
/* interpretation */
/******************/

int cdPlay(cdContext* context, int xmin, int xmax, int ymin, int ymax, void *data)
{
  cdPrivateContext* ctx;

  assert(context);
  assert(active_private_canvas);
  
  ctx = (cdPrivateContext*)context->ctx;

  if (!ctx->Play)
    return CD_ERROR;

  if (!check_box(xmin, xmax, ymin, ymax))
    return CD_ERROR;

  return ctx->Play(xmin, xmax, ymin, ymax, data);
}

int cdRegisterCallback(cdContext *context, int cb, cdCallback func)
{
  cdPrivateContext* ctx;

  assert(context);
  
  ctx = (cdPrivateContext*)context->ctx;

  if (!ctx->RegisterCallback)
    return CD_ERROR;

  return ctx->RegisterCallback(cb, func);
}

/************/
/* clipping */
/************/

int cdClip(int mode)
{
  int clip_mode;

  assert(active_private_canvas);
  assert(mode==CD_QUERY || (mode>=CD_CLIPOFF && mode<=CD_CLIPREGION));

  clip_mode = active_private_canvas->clip_mode;

  if ( mode == CD_QUERY || 
       mode == clip_mode || 
      (mode == CD_CLIPPOLYGON && !active_private_canvas->clip_poly) )
    return clip_mode;

  wdCall(0);

  active_private_canvas->clip_mode = active_private_canvas->Clip(mode);

  return clip_mode;
}

void cdClipArea(int xmin, int xmax, int ymin, int ymax)
{
  assert(active_private_canvas);

  if (!check_box(xmin, xmax, ymin, ymax))
    return;

  if (active_private_canvas->origin)
  {
    xmin += active_private_canvas->origin_x;
    xmax += active_private_canvas->origin_x;
    ymin += active_private_canvas->origin_y;
    ymax += active_private_canvas->origin_y;
  }

  if (active_private_canvas->invert_yaxis)
  {
    ymin = _cdInvertYAxis(ymin);
    ymax = _cdInvertYAxis(ymax);
    SWAPINT(ymin, ymax);
  }

  if (xmin == active_private_canvas->clip_xmin && 
      xmax == active_private_canvas->clip_xmax && 
      ymin == active_private_canvas->clip_ymin && 
      ymax == active_private_canvas->clip_ymax)
    return;

  wdCall(0);
  active_private_canvas->ClipArea(&xmin, &xmax, &ymin, &ymax);

  active_private_canvas->clip_xmin = xmin;
  active_private_canvas->clip_xmax = xmax;
  active_private_canvas->clip_ymin = ymin;
  active_private_canvas->clip_ymax = ymax;
}

int cdGetClipArea(int *xmin, int *xmax, int *ymin, int *ymax)
{
  int _xmin, _xmax, _ymin, _ymax;
  assert(active_private_canvas);

  _xmin = active_private_canvas->clip_xmin;
  _xmax = active_private_canvas->clip_xmax;
  _ymin = active_private_canvas->clip_ymin;
  _ymax = active_private_canvas->clip_ymax;

  if (active_private_canvas->invert_yaxis)
  {
    _ymin = _cdInvertYAxis(_ymin);
    _ymax = _cdInvertYAxis(_ymax);
    SWAPINT(_ymin, _ymax);
  }

  if (active_private_canvas->origin)
  {
    _xmin -= active_private_canvas->origin_x;
    _xmax -= active_private_canvas->origin_x;
    _ymin -= active_private_canvas->origin_y;
    _ymax -= active_private_canvas->origin_y;
  }

  if (xmin) *xmin = _xmin;
  if (xmax) *xmax = _xmax;
  if (ymin) *ymin = _ymin;
  if (ymax) *ymax = _ymax;

  return active_private_canvas->clip_mode;
}

int * cdGetClipPoly(int *n)
{
  static int poly[2*_CD_POLY_BLOCK];
  if (n) *n = active_private_canvas->clip_poly_n;
  if (active_private_canvas->clip_poly && active_private_canvas->clip_poly_n < 2*_CD_POLY_BLOCK)
  {
    int i;

    memcpy(poly, active_private_canvas->clip_poly, active_private_canvas->clip_poly_n*sizeof(cdPoint));

    for (i=0; i < active_private_canvas->clip_poly_n; i++)
    {
      if (active_private_canvas->invert_yaxis)
      {
        active_private_canvas->clip_poly[i].y = _cdInvertYAxis(active_private_canvas->clip_poly[i].y);
      }

      if (active_private_canvas->origin)
      {
        active_private_canvas->clip_poly[i].x -= active_private_canvas->origin_x;
        active_private_canvas->clip_poly[i].y -= active_private_canvas->origin_y;
      }
    }

    return poly;
  }
  else
    return NULL;
}

int cdPointInRegion(int x, int y)
{
  assert(active_private_canvas);

  if (active_private_canvas->origin)
  {
    x += active_private_canvas->origin_x;
    y += active_private_canvas->origin_y;
  }

  if (active_private_canvas->invert_yaxis)
    y = _cdInvertYAxis(y);

  wdCall(0);
  return active_private_canvas->PointInRegion(x, y);
}

void cdOffsetRegion(int x, int y)
{
  assert(active_private_canvas);

  if (active_private_canvas->invert_yaxis)
    y = -y;

  wdCall(0);
  active_private_canvas->OffsetRegion(x, y);
}

void cdRegionBox(int *xmin, int *xmax, int *ymin, int *ymax)
{
  int _xmin, _xmax, _ymin, _ymax;
  assert(active_private_canvas);

  active_private_canvas->RegionBox(&_xmin, &_xmax, &_ymin, &_ymax);

  if (active_private_canvas->invert_yaxis)
  {
    _ymin = _cdInvertYAxis(_ymin);
    _ymax = _cdInvertYAxis(_ymax);
    SWAPINT(_ymin, _ymax);
  }

  if (active_private_canvas->origin)
  {
    _xmin -= active_private_canvas->origin_x;
    _xmax -= active_private_canvas->origin_x;
    _ymin -= active_private_canvas->origin_y;
    _ymax -= active_private_canvas->origin_y;
  }

  if (xmin) *xmin = _xmin;
  if (xmax) *xmax = _xmax;
  if (ymin) *ymin = _ymin;
  if (ymax) *ymax = _ymax;
}

/***************/
/* coordinates */
/***************/

void cdGetCanvasSize(int *width, int *height, double *width_mm, double *height_mm)
{
  assert(active_private_canvas);

  if (width) *width = active_private_canvas->w;
  if (height) *height = active_private_canvas->h;
  if (width_mm) *width_mm = active_private_canvas->w_mm;
  if (height_mm) *height_mm = active_private_canvas->h_mm;
}

void cdMM2Pixel (double mm_dx, double mm_dy, int *dx, int *dy)
{
  assert(active_private_canvas);

  if (dx) *dx = (int)(mm_dx*active_private_canvas->xres);
  if (dy) *dy = (int)(mm_dy*active_private_canvas->yres);
}

void cdPixel2MM (int dx, int dy, double *mm_dx, double *mm_dy)
{
  assert(active_private_canvas);

  if (mm_dx) *mm_dx = ((double)dx)/active_private_canvas->xres;
  if (mm_dy) *mm_dy = ((double)dy)/active_private_canvas->yres;
}

void cdOrigin(int x, int y)
{
  assert(active_private_canvas);

  active_private_canvas->origin_x = x;
  active_private_canvas->origin_y = y;

  if (active_private_canvas->origin_x == 0 && active_private_canvas->origin_y == 0)
    active_private_canvas->origin = 0;
  else
    active_private_canvas->origin = 1;
}

void cdUpdateYAxis(int* y)
{
  assert(active_private_canvas);
  assert(y);

  if(active_private_canvas->invert_yaxis)
  {
    *y = active_private_canvas->h - *y - 1;

    if (active_private_canvas->origin)
      *y -= 2*active_private_canvas->origin_y;
  }
}

/**************/
/* primitives */
/**************/

void cdPixel(int x, int y, long int color)
{
  assert(active_private_canvas);

  if (active_private_canvas->origin)
  {
    x += active_private_canvas->origin_x;
    y += active_private_canvas->origin_y;
  }

  if (active_private_canvas->invert_yaxis)
    y = _cdInvertYAxis(y);

  if (active_private_canvas->sim_mode & CD_SIM_CLIP)
  {
	  if(active_private_canvas->sim_mode & CD_SIM_CLIPAREA && 
       active_private_canvas->clip_mode == CD_CLIPAREA && 
       cdSimClipPointInBox(x,y) == 0)
      return;
	  else if(active_private_canvas->sim_mode & CD_SIM_CLIPPOLY && 
            active_private_canvas->clip_mode == CD_CLIPPOLYGON && 
            cdSimClipPointInPoly(x,y) == 0) 
      return;
  }

  wdCall(0);
  active_private_canvas->Pixel(x, y, color);
}

void cdMark(int x, int y)
{
  assert(active_private_canvas);

  if (active_private_canvas->mark_size == 1)
  {
    cdPixel(x, y, active_private_canvas->foreground);
    return;
  }

  if (active_private_canvas->origin)
  {
    x += active_private_canvas->origin_x;
    y += active_private_canvas->origin_y;
  }

  if (active_private_canvas->invert_yaxis)
    y = _cdInvertYAxis(y);

  if (active_private_canvas->sim_mode & CD_SIM_CLIP)
  {
    /* simulacao de clipping de marca so' e' feito aqui */

	  if(active_private_canvas->sim_mode & CD_SIM_CLIPAREA && 
      active_private_canvas->clip_mode == CD_CLIPAREA && 
      cdSimClipPointInBox(x,y) == 0)
      return;
	  else if(active_private_canvas->sim_mode & CD_SIM_CLIPPOLY && 
            active_private_canvas->clip_mode == CD_CLIPPOLYGON && 
            cdSimClipPointInPoly(x,y) == 0) 
      return;
  }

  wdCall(0);
  active_private_canvas->Mark(x, y);
}

void cdLine(int x1, int y1, int x2, int y2)
{
  assert(active_private_canvas);

  if (x1 == x2 && y1 == y2)
  {
    cdPixel(x1, y1, active_private_canvas->foreground);
    return;
  }
    
  if (active_private_canvas->origin)
  {
    x1 += active_private_canvas->origin_x;
    y1 += active_private_canvas->origin_y;
    x2 += active_private_canvas->origin_x;
    y2 += active_private_canvas->origin_y;
  }

  if (active_private_canvas->invert_yaxis)
  {
    y1 = _cdInvertYAxis(y1);
    y2 = _cdInvertYAxis(y2);
  }

  wdCall(0);

  if (active_private_canvas->sim_mode & CD_SIM_CLIP && !(active_private_canvas->sim_mode & CD_SIM_LINE))
  {
	  if(active_private_canvas->sim_mode & CD_SIM_CLIPAREA && 
       active_private_canvas->clip_mode == CD_CLIPAREA && 
       cdSimClipLineInBox(&x1,&y1,&x2,&y2) == 0)
      return;
	  else if(active_private_canvas->sim_mode & CD_SIM_CLIPPOLY && 
            active_private_canvas->clip_mode == CD_CLIPPOLYGON) 
    {
      cdSimClipLineInPoly(x1,y1,x2,y2);
      return;
    }
  }

  active_private_canvas->Line(x1, y1, x2, y2);
}

void cdBegin(int mode)
{
  assert(active_private_canvas);
  assert(mode>=CD_FILL);

  if (mode == CD_REGION)
  {
    active_private_canvas->new_region = 1;
    active_private_canvas->poly_n = 0;
    active_private_canvas->NewRegion();
    return;
  }

  active_private_canvas->sim_poly = 0;

  if (active_private_canvas->interior_style == CD_HOLLOW && mode == CD_FILL)
    mode = CD_CLOSED_LINES;

  /* simulacao de linhas */
  if ((mode == CD_CLOSED_LINES || mode == CD_OPEN_LINES || mode == CD_BEZIER) && 
      active_private_canvas->sim_mode & CD_SIM_POLYLINE)
    active_private_canvas->sim_poly = 1;

  /* simulacao de poligonos preenchidos */
  if (mode == CD_FILL && active_private_canvas->sim_mode & CD_SIM_POLYGON)
    active_private_canvas->sim_poly = 1;

  /* simulacao de clipping com poligono */
  if (active_private_canvas->sim_mode & CD_SIM_CLIP)
    active_private_canvas->sim_poly = 1;

  if (!active_private_canvas->poly)
  {
    active_private_canvas->poly = (cdPoint*)malloc(sizeof(cdPoint)*(_CD_POLY_BLOCK+1));
    active_private_canvas->poly_size = _CD_POLY_BLOCK;
    active_private_canvas->wpoly = (wdPoint*)malloc(sizeof(wdPoint)*(_CD_POLY_BLOCK+1));
    active_private_canvas->wpoly_size = _CD_POLY_BLOCK;
  }

  active_private_canvas->poly_n = 0;
  active_private_canvas->poly_mode = mode;
}

void cdVertex(int x, int y)
{
  assert(active_private_canvas);

  active_private_canvas->wd_poly = 0;

  if (active_private_canvas->origin)
  {
    x += active_private_canvas->origin_x;
    y += active_private_canvas->origin_y;
  }

  if (active_private_canvas->invert_yaxis)
    y = _cdInvertYAxis(y);

  if (active_private_canvas->poly_n == active_private_canvas->poly_size)
  {
    active_private_canvas->poly_size += _CD_POLY_BLOCK;
    active_private_canvas->poly = (cdPoint*)realloc(active_private_canvas->poly, sizeof(cdPoint) * (active_private_canvas->poly_size+1));
  }

  active_private_canvas->poly[active_private_canvas->poly_n].x = x;
  active_private_canvas->poly[active_private_canvas->poly_n].y = y;
  active_private_canvas->poly_n++;
}

void cdEnd(void)
{
  assert(active_private_canvas);

  if (active_private_canvas->new_region && active_private_canvas->poly_n == 0)
  {
    active_private_canvas->new_region = 0;
    if (active_private_canvas->clip_mode == CD_CLIPREGION) cdClip(CD_CLIPREGION);
    return;
  }

  if (active_private_canvas->poly_mode==CD_OPEN_LINES && active_private_canvas->poly_n < 2)
  {
    active_private_canvas->poly_n = 0;
    return;
  }

  if (active_private_canvas->poly_mode==CD_BEZIER && active_private_canvas->poly_n < 4)
  {
    active_private_canvas->poly_n = 0;
    return;
  }

  if ((active_private_canvas->poly_mode == CD_CLOSED_LINES ||
       active_private_canvas->poly_mode == CD_FILL ||
       active_private_canvas->poly_mode == CD_CLIP) && active_private_canvas->poly_n < 3)
  {
    active_private_canvas->poly_n = 0;
    return;
  }

  if (active_private_canvas->sim_poly)
  {
    wdCall(0);
    cdSimPoly(active_private_canvas->poly_mode, active_private_canvas->poly, active_private_canvas->poly_n);
  }
  else
  {
    if (active_private_canvas->wd_poly)
    {
      wdCall(1);
      active_private_canvas->wPoly(active_private_canvas->poly_mode, active_private_canvas->wpoly, active_private_canvas->poly_n);
    }
    else
    {
      wdCall(0);
      active_private_canvas->Poly(active_private_canvas->poly_mode, active_private_canvas->poly, active_private_canvas->poly_n);
    }
  }

  if (active_private_canvas->poly_mode == CD_CLIP)
  {
    if (active_private_canvas->clip_wpoly) 
    {
      free(active_private_canvas->clip_wpoly);
      active_private_canvas->clip_wpoly = NULL;
      active_private_canvas->clip_wpoly_n = 0;
    }

    if (active_private_canvas->clip_poly) 
    {
      free(active_private_canvas->clip_poly);
      active_private_canvas->clip_poly = NULL;
      active_private_canvas->clip_poly_n = 0;
    }

    if (active_private_canvas->wd_poly)
    {
      active_private_canvas->clip_wpoly_n = active_private_canvas->poly_n;
      active_private_canvas->clip_wpoly = (wdPoint*)malloc((active_private_canvas->poly_n+1) * sizeof(wdPoint));
      memcpy(active_private_canvas->clip_wpoly, active_private_canvas->wpoly, active_private_canvas->poly_n * sizeof(wdPoint));
    }
    else
    {
      active_private_canvas->clip_poly_n = active_private_canvas->poly_n;
      active_private_canvas->clip_poly = (cdPoint*)malloc((active_private_canvas->poly_n+1) * sizeof(cdPoint));
      memcpy(active_private_canvas->clip_poly, active_private_canvas->poly, active_private_canvas->poly_n * sizeof(cdPoint));
    }
  }

  active_private_canvas->poly_n = 0;
}

void cdRect(int xmin, int xmax, int ymin, int ymax)
{
  assert(active_private_canvas);

  if (!check_box(xmin, xmax, ymin, ymax))
    return;

  if (active_private_canvas->origin)
  {
    xmin += active_private_canvas->origin_x;
    xmax += active_private_canvas->origin_x;
    ymin += active_private_canvas->origin_y;
    ymax += active_private_canvas->origin_y;
  }

  if (active_private_canvas->invert_yaxis)
  {
    ymin = _cdInvertYAxis(ymin);
    ymax = _cdInvertYAxis(ymax);
    SWAPINT(ymin, ymax);
  }

  wdCall(0);

  if (active_private_canvas->sim_mode & CD_SIM_CLIP)
  {
    /* simulacao de clipping de rect so' e' feito aqui */

	  if(active_private_canvas->sim_mode & CD_SIM_CLIPAREA && 
       active_private_canvas->clip_mode == CD_CLIPAREA)
    {
      int x1, y1, x2, y2;

      cdClip(CD_CLIPOFF);

	    x1=xmin; y1=ymin; x2=xmin; y2=ymax;
      if (cdSimClipLineInBox(&x1,&y1,&x2,&y2) == 1)
        active_private_canvas->Line(x1,y1,x2,y2);

	    x1=xmin; y1=ymax; x2=xmax; y2=ymax;
      if (cdSimClipLineInBox(&x1,&y1,&x2,&y2) == 1)
        active_private_canvas->Line(x1,y1,x2,y2);

	    x1=xmax; y1=ymax; x2=xmax; y2=ymin;
      if (cdSimClipLineInBox(&x1,&y1,&x2,&y2) == 1)
        active_private_canvas->Line(x1,y1,x2,y2);

	    x1=xmax; y1=ymin; x2=xmin; y2=ymin;
      if (cdSimClipLineInBox(&x1,&y1,&x2,&y2) == 1)
        active_private_canvas->Line(x1,y1,x2,y2);

      cdClip(CD_CLIPAREA);

      return;
    }
	  else if(active_private_canvas->sim_mode & CD_SIM_CLIPPOLY && 
            active_private_canvas->clip_mode == CD_CLIPPOLYGON) 
    {
	    cdSimClipLineInPoly(xmin, ymin, xmin, ymax);
	    cdSimClipLineInPoly(xmin, ymax, xmax, ymax);
	    cdSimClipLineInPoly(xmax, ymax, xmax, ymin);
	    cdSimClipLineInPoly(xmax, ymin, xmin, ymin);
      return;
    }
  }

  active_private_canvas->Rect(xmin, xmax, ymin, ymax);
}

void cdBox(int xmin, int xmax, int ymin, int ymax)
{
  assert(active_private_canvas);

  if (!check_box(xmin, xmax, ymin, ymax))
    return;

  if (active_private_canvas->interior_style == CD_HOLLOW)
  {
    cdRect(xmin, xmax, ymin, ymax);
    return;
  }

  if (active_private_canvas->origin)
  {
    xmin += active_private_canvas->origin_x;
    xmax += active_private_canvas->origin_x;
    ymin += active_private_canvas->origin_y;
    ymax += active_private_canvas->origin_y;
  }

  if (active_private_canvas->invert_yaxis)
  {
    ymin = _cdInvertYAxis(ymin);
    ymax = _cdInvertYAxis(ymax);
    SWAPINT(ymin, ymax);
  }

  wdCall(0);                                          

  if (active_private_canvas->sim_mode & CD_SIM_CLIP && !(active_private_canvas->sim_mode & CD_SIM_BOX))
  {
	  if(active_private_canvas->sim_mode & CD_SIM_CLIPAREA && 
       active_private_canvas->clip_mode == CD_CLIPAREA && 
       cdSimClipBoxInBox(&xmin,&xmax,&ymin,&ymax) == 0)
      return;
	  else if(active_private_canvas->sim_mode & CD_SIM_CLIPPOLY && 
            active_private_canvas->clip_mode == CD_CLIPPOLYGON) 
    {
      cdSimBox(xmin, xmax, ymin, ymax);
      return;
    }
  }

  active_private_canvas->Box(xmin, xmax, ymin, ymax);
}

void cdArc(int xc, int yc, int w, int h, double angle1, double angle2)
{
  assert(active_private_canvas);

  if (angle1 == angle2 || w == 0 || h == 0)
    return;

  angle1 = fmod(angle1,360);
  angle2 = fmod(angle2,360);
  if (angle2 <= angle1) angle2 += 360;

  if (active_private_canvas->origin)
  {
    xc += active_private_canvas->origin_x;
    yc += active_private_canvas->origin_y;
  }

  if (active_private_canvas->invert_yaxis)
    yc = _cdInvertYAxis(yc);

  wdCall(0);

  if (active_private_canvas->sim_mode & CD_SIM_CLIP && !(active_private_canvas->sim_mode & CD_SIM_ARC))
  {
	  if(active_private_canvas->clip_mode != CD_CLIPOFF)
    {
      cdSimArc(xc, yc, w, h, angle1, angle2);
      return;
    }
  }

  active_private_canvas->Arc(xc, yc, w, h, angle1, angle2);
}

void cdSector(int xc, int yc, int w, int h, double angle1, double angle2)
{
  assert(active_private_canvas);

  if (angle1 == angle2 || w == 0 || h == 0)
    return;

  if (active_private_canvas->interior_style == CD_HOLLOW)
  {
    int xi,yi,xf,yf;
    
    xi = xc + ROUND(w*cos(CD_DEG2RAD*angle1)/2.0);
    yi = yc + ROUND(h*sin(CD_DEG2RAD*angle1)/2.0);
    
    xf = xc + ROUND(w*cos(CD_DEG2RAD*angle2)/2.0);
    yf = yc + ROUND(h*sin(CD_DEG2RAD*angle2)/2.0);

    cdArc(xc, yc, w, h, angle1, angle2);
    if (angle1 != 0 || angle2 != 360)
    {
      cdLine(xi, yi, xc, yc);
      cdLine(xc, yc, xf, yf);
    }
    return;
  }

  angle1 = fmod(angle1,360);
  angle2 = fmod(angle2,360);
  if (angle2 <= angle1) angle2 += 360;

  if (active_private_canvas->origin)
  {
    xc += active_private_canvas->origin_x;
    yc += active_private_canvas->origin_y;
  }

  if (active_private_canvas->invert_yaxis)
    yc = _cdInvertYAxis(yc);

  wdCall(0);

  if (active_private_canvas->sim_mode & CD_SIM_CLIP && !(active_private_canvas->sim_mode & CD_SIM_SECTOR))
  {
	  if(active_private_canvas->clip_mode != CD_CLIPOFF)
    {
      cdSimSector(xc, yc, w, h, angle1, angle2);
      return;
    }
  }

  active_private_canvas->Sector(xc, yc, w, h, angle1, angle2);
}

void cdChord(int xc, int yc, int w, int h, double angle1, double angle2)
{
  assert(active_private_canvas);

  if (angle1 == angle2 || w == 0 || h == 0)
    return;

  if (active_private_canvas->interior_style == CD_HOLLOW)
  {
    int xi,yi,xf,yf;
    
    xi = xc + ROUND(w*cos(CD_DEG2RAD*angle1)/2.0);
    yi = yc + ROUND(h*sin(CD_DEG2RAD*angle1)/2.0);
    
    xf = xc + ROUND(w*cos(CD_DEG2RAD*angle2)/2.0);
    yf = yc + ROUND(h*sin(CD_DEG2RAD*angle2)/2.0);

    cdArc(xc, yc, w, h, angle1, angle2);
    cdLine(xi, yi, xf, yf);
    return;
  }

  angle1 = fmod(angle1,360);
  angle2 = fmod(angle2,360);
  if (angle2 <= angle1) angle2 += 360;

  if (active_private_canvas->origin)
  {
    xc += active_private_canvas->origin_x;
    yc += active_private_canvas->origin_y;
  }

  if (active_private_canvas->invert_yaxis)
    yc = _cdInvertYAxis(yc);

  wdCall(0);

  if (active_private_canvas->sim_mode & CD_SIM_CLIP && !(active_private_canvas->sim_mode & CD_SIM_SECTOR))
  {
	  if(active_private_canvas->clip_mode != CD_CLIPOFF)
    {
      cdSimChord(xc, yc, w, h, angle1, angle2);
      return;
    }
  }

  active_private_canvas->Chord(xc, yc, w, h, angle1, angle2);
}

void cdText(int x, int y, char *s)
{
  assert(active_private_canvas);
  assert(s);

  if (s[0] == 0)
    return;

  if (active_private_canvas->origin)
  {
    x += active_private_canvas->origin_x;
    y += active_private_canvas->origin_y;
  }

  if (active_private_canvas->invert_yaxis)
    y = _cdInvertYAxis(y);

  wdCall(0);

  if (active_private_canvas->sim_mode & CD_SIM_CLIPAREA)
  {
    /* simulacao de clipping de texto so' e' feito aqui */

	  if(active_private_canvas->sim_mode & CD_SIM_CLIPAREA && 
       active_private_canvas->clip_mode == CD_CLIPAREA && 
       cdSimClipTextInBox(x, y, s) == 0)
      return;
  }

  active_private_canvas->Text(x, y, s);
}

/**************/
/* attributes */
/**************/

int cdBackOpacity(int opacity)
{
  int back_opacity;

  assert(active_private_canvas);
  assert(opacity==CD_QUERY || (opacity>=CD_OPAQUE && opacity<=CD_TRANSPARENT));

  back_opacity = active_private_canvas->back_opacity;

  if (opacity == CD_QUERY || opacity == back_opacity)
    return back_opacity;

  active_private_canvas->back_opacity = active_private_canvas->BackOpacity(opacity);

  return back_opacity;
}

int cdWriteMode(int mode)
{
  int write_mode;

  assert(active_private_canvas);
  assert(mode==CD_QUERY || (mode>=CD_REPLACE && mode<=CD_NOT_XOR));

  write_mode = active_private_canvas->write_mode;

  if (mode == CD_QUERY || mode == write_mode)
    return write_mode;

  active_private_canvas->write_mode = active_private_canvas->WriteMode(mode);

  return write_mode;
}

int cdLineStyle(int style)
{
  int line_style;

  assert(active_private_canvas);
  assert(style==CD_QUERY || (style>=CD_CONTINUOUS && style<=CD_CUSTOM));

  line_style = active_private_canvas->line_style;

  if (style == CD_QUERY || style == line_style)
    return line_style;

  if (style == CD_CUSTOM && !active_private_canvas->line_dashes_count)
    return line_style;

  active_private_canvas->line_style = active_private_canvas->LineStyle(style);

  return line_style;
}

int cdLineJoin(int join)
{
  int line_join;

  assert(active_private_canvas);
  assert(join==CD_QUERY || (join>=CD_MITER && join<=CD_ROUND));

  line_join = active_private_canvas->line_join;

  if (join == CD_QUERY || join == line_join)
    return line_join;

  active_private_canvas->line_join = active_private_canvas->LineJoin(join);

  return line_join;
}

int cdLineCap(int cap)
{
  int line_cap;

  assert(active_private_canvas);
  assert(cap==CD_QUERY || (cap>=CD_CAPFLAT && cap<=CD_CAPROUND));

  line_cap = active_private_canvas->line_cap;

  if (cap == CD_QUERY || cap == line_cap)
    return line_cap;

  active_private_canvas->line_cap = active_private_canvas->LineCap(cap);

  return line_cap;
}

int cdRegionCombineMode(int mode)
{
  int combine_mode;

  assert(active_private_canvas);
  assert(mode==CD_QUERY || (mode>=CD_UNION && mode<=CD_NOTINTERSECT));

  combine_mode = active_private_canvas->combine_mode;

  if (mode == CD_QUERY || mode == combine_mode)
    return combine_mode;

  active_private_canvas->combine_mode = mode;

  return combine_mode;
}

void cdLineStyleDashes(int* dashes, int count)
{
  assert(active_private_canvas);

  if (active_private_canvas->line_dashes)
    free(active_private_canvas->line_dashes);

  active_private_canvas->line_dashes = malloc(count*sizeof(int));
  active_private_canvas->line_dashes_count = count;
  memcpy(active_private_canvas->line_dashes, dashes, count*sizeof(int));
}

int cdLineWidth(int width)
{
  int line_width;

  assert(active_private_canvas);
  assert(width==CD_QUERY || width>0);

  line_width = active_private_canvas->line_width;

  if (width == CD_QUERY || width == line_width)
    return line_width;

  active_private_canvas->line_width = active_private_canvas->LineWidth(width);

  return line_width;
}

int  cdFillMode(int mode)
{
  int fill_mode;

  assert(active_private_canvas);
  assert(mode==CD_QUERY || (mode>=CD_EVENODD && mode<=CD_WINDING));
                                        
  fill_mode = active_private_canvas->fill_mode;

  if (mode == CD_QUERY || mode == fill_mode)
    return fill_mode;

  active_private_canvas->fill_mode = mode;

  return fill_mode;
}

int cdInteriorStyle (int style)
{
  int interior_style;

  assert(active_private_canvas);
  assert(style==CD_QUERY || (style>=CD_SOLID && style<=CD_HOLLOW));

  interior_style = active_private_canvas->interior_style;

  if ( style == CD_QUERY || style == interior_style)
    return interior_style;
    
  if ((style == CD_PATTERN && !active_private_canvas->pattern_size) || 
	    (style == CD_STIPPLE && !active_private_canvas->stipple_size))
  return interior_style;

  if (style == CD_HOLLOW)
  {
    active_private_canvas->interior_style = CD_HOLLOW;
    return interior_style;
  }

  active_private_canvas->interior_style = active_private_canvas->InteriorStyle(style);

  return interior_style;
}

int cdHatch(int style)
{
  int hatch_style;

  assert(active_private_canvas);
  assert(style==CD_QUERY || (style>=CD_HORIZONTAL && style<=CD_DIAGCROSS));

  hatch_style = active_private_canvas->hatch_style;

  if (style == CD_QUERY)
    return hatch_style;

  active_private_canvas->hatch_style = active_private_canvas->Hatch(style);
  active_private_canvas->interior_style = CD_HATCH;

  return hatch_style;
}

void cdStipple(int w, int h, unsigned char *stipple)
{
  assert(active_private_canvas);
  assert(stipple);
  assert(w>0);
  assert(h>0);

  if (w <= 0 || h <= 0)
    return;

  active_private_canvas->Stipple(w, h, stipple);

  if (w*h >= active_private_canvas->stipple_size)       /* realoca array dos pontos */
  {
    int newsize = w*h;

    if (active_private_canvas->stipple) free(active_private_canvas->stipple);
    active_private_canvas->stipple = (unsigned char*)malloc(newsize);
    active_private_canvas->stipple_size = newsize;

    if (!active_private_canvas->stipple) 
    {
      active_private_canvas->stipple_size = 0;
      return;
    }
  }

  memcpy(active_private_canvas->stipple, stipple, w*h);

  active_private_canvas->interior_style = CD_STIPPLE;
  active_private_canvas->stipple_w = w;
  active_private_canvas->stipple_h = h;
}

unsigned char * cdGetStipple(int* w, int* h)
{
  if (!active_private_canvas->stipple_size)
    return NULL;

  if (w) *w = active_private_canvas->stipple_w;
  if (h) *h = active_private_canvas->stipple_h;

  return active_private_canvas->stipple;
}

void cdPattern(int w, int h, long int *pattern)
{
  assert(active_private_canvas);
  assert(pattern);
  assert(w>0);
  assert(h>0);

  if (w <= 0 || h <= 0)
    return;

  active_private_canvas->Pattern(w, h, pattern);

  if (w*h >= active_private_canvas->pattern_size)       /* realoca array dos pontos */
  {
    int newsize = w*h;

    if (active_private_canvas->pattern) free(active_private_canvas->pattern);
    active_private_canvas->pattern = (long int*)malloc(newsize*sizeof(long int));
    active_private_canvas->pattern_size = newsize;

    if (!active_private_canvas->pattern) 
    {
      active_private_canvas->pattern_size = 0;
      return;
    }
  }

  memcpy(active_private_canvas->pattern, pattern, w*h*sizeof(long int));

  active_private_canvas->interior_style = CD_PATTERN;
  active_private_canvas->pattern_w = w;
  active_private_canvas->pattern_h = h;
}

long int * cdGetPattern(int* w, int* h)
{
  if (!active_private_canvas->pattern_size)
    return NULL;

  if (w) *w = active_private_canvas->pattern_w;
  if (h) *h = active_private_canvas->pattern_h;

  return active_private_canvas->pattern;
}

void cdFont(int type_face, int style, int size)
{
  assert(active_private_canvas);
  assert(type_face>=CD_SYSTEM && type_face<=CD_NATIVE);
  assert(style>=CD_PLAIN && style<=CD_BOLD_ITALIC);

  if (type_face == active_private_canvas->font_type_face && 
      style == active_private_canvas->font_style && 
      size == active_private_canvas->font_size)
    return;

  if (type_face == CD_NATIVE)
  {
    active_private_canvas->font_type_face = CD_NATIVE;
    active_private_canvas->NativeFont(active_private_canvas->native_font);
  }
  else
  {
    active_private_canvas->Font(&type_face, &style, &size);
    active_private_canvas->font_type_face = type_face;
    active_private_canvas->font_style     = style;
    active_private_canvas->font_size      = size;
  }

  active_private_canvas->native_font[0] = 0;
}

void cdGetFont(int *type_face, int *style, int *size)
{
  assert(active_private_canvas);

  if (type_face) *type_face = active_private_canvas->font_type_face;
  if (style) *style = active_private_canvas->font_style;
  if (size) *size = active_private_canvas->font_size;
}

char* cdNativeFont(char* font)
{
  static char native_font[512] = "";

  assert(active_private_canvas);

  strcpy(native_font, active_private_canvas->native_font);

  if (!font || font[0] == 0)
    return native_font;

  strcpy(active_private_canvas->native_font, font);

  active_private_canvas->font_type_face = CD_NATIVE;
  active_private_canvas->NativeFont(active_private_canvas->native_font);

  return native_font;
}

int cdTextAlignment(int alignment)
{
  int text_alignment;

  assert(active_private_canvas);
  assert(alignment==CD_QUERY || (alignment>=CD_NORTH && alignment<=CD_BASE_RIGHT));

  text_alignment = active_private_canvas->text_alignment;

  if (alignment == CD_QUERY || alignment == text_alignment)
    return text_alignment;

  active_private_canvas->text_alignment = active_private_canvas->TextAlignment(alignment);

  return text_alignment;
}

double cdTextOrientation(double angle)
{
  double text_orientation;

  assert(active_private_canvas);

  text_orientation = active_private_canvas->text_orientation;

  if (angle == CD_QUERY || angle == text_orientation)
    return text_orientation;

  active_private_canvas->text_orientation = active_private_canvas->TextOrientation(angle);

  return text_orientation;
}

int cdMarkType(int type)
{
  int mark_type;

  assert(active_private_canvas);
  assert(type==CD_QUERY || (type>=CD_PLUS && type<=CD_HOLLOW_DIAMOND));

  mark_type = active_private_canvas->mark_type;

  if (type == CD_QUERY || type == mark_type)
    return mark_type;

  active_private_canvas->mark_type = active_private_canvas->MarkType(type);

  return mark_type;
}

int cdMarkSize(int size)
{
  int mark_size;

  assert(active_private_canvas);
  assert(size == CD_QUERY || size>0);

  mark_size = active_private_canvas->mark_size;

  if (size == CD_QUERY || size == mark_size)
    return mark_size;

  active_private_canvas->mark_size = active_private_canvas->MarkSize(size);

  return mark_size;
}

long int cdBackground(long int color)
{
  long int background;

  assert(active_private_canvas);

  background = active_private_canvas->background;

  if (color == CD_QUERY || color == background)
    return background;

  active_private_canvas->background = active_private_canvas->Background(color);

  return background;
}

long int cdForeground(long int color)
{
  long int foreground;

  assert(active_private_canvas);

  foreground = active_private_canvas->foreground;

  if (color == CD_QUERY || color == foreground)
    return foreground;

  active_private_canvas->foreground = active_private_canvas->Foreground(color);

  return foreground;
}

/**************/
/* properties */
/**************/

void cdFontDim(int *max_width, int *height, int *ascent, int *descent)
{
  assert(active_private_canvas);
  active_private_canvas->FontDim(max_width, height, ascent, descent);
}

void cdTextSize(char *s, int *width, int *height)
{
  assert(active_private_canvas);
  assert(s);
  active_private_canvas->TextSize(s, width, height);
}

void cdTextRotatePoint(int x, int y, int cx, int cy, int *rx, int *ry, double sin_theta, double cos_theta)
{
  /* translate to (cx,cy) */
  x = x - cx;
  y = y - cy;

  /* rotate */
  if (active_private_canvas->invert_yaxis)
  {
    *rx = (int)( (x * cos_theta) + (y * sin_theta)); 
    *ry = (int)(-(x * sin_theta) + (y * cos_theta));
  }
  else
  {
    *rx = (int)((x * cos_theta) - (y * sin_theta)); 
    *ry = (int)((x * sin_theta) + (y * cos_theta));
  }

  /* translate back */
  *rx = *rx + cx;
  *ry = *ry + cy;
}

void cdTextTranslatePoint(int x, int y, int w, int h, int baseline, int *rx, int *ry)
{
  /* move to left */
  switch (active_private_canvas->text_alignment)
  {
  case CD_BASE_RIGHT:
  case CD_NORTH_EAST:
  case CD_EAST:
  case CD_SOUTH_EAST:
    *rx = x - w;    
    break;
  case CD_BASE_CENTER:
  case CD_CENTER:
  case CD_NORTH:
  case CD_SOUTH:
    *rx = x - w/2;  
    break;
  case CD_BASE_LEFT:
  case CD_NORTH_WEST:
  case CD_WEST:
  case CD_SOUTH_WEST:
    *rx = x;         
    break;
  }

  /* move to bottom */
  switch (active_private_canvas->text_alignment)
  {
  case CD_BASE_LEFT:
  case CD_BASE_CENTER:
  case CD_BASE_RIGHT:
    if (active_private_canvas->invert_yaxis)
      *ry = y + baseline;
    else
      *ry = y - baseline;
    break;
  case CD_SOUTH_EAST:
  case CD_SOUTH_WEST:
  case CD_SOUTH:
    *ry = y;
    break;
  case CD_NORTH_EAST:
  case CD_NORTH:
  case CD_NORTH_WEST:
    if (active_private_canvas->invert_yaxis)
      *ry = y + h;
    else
      *ry = y - h;
    break;
  case CD_CENTER:
  case CD_EAST:
  case CD_WEST:
    if (active_private_canvas->invert_yaxis)
      *ry = y + h/2;
    else
      *ry = y - h/2;
    break;
  }
}

void cdTextBounds(int x, int y, char *s, int *rect)
{
  int w, h, ascent, height, baseline;
  int xmin, xmax, ymin, ymax;
  int old_invert_yaxis = active_private_canvas->invert_yaxis;
  
  cdTextSize (s, &w, &h);
  cdFontDim(NULL, &height, &ascent, NULL);
  baseline = height - ascent;

  /* in this case we are always upwards */
  active_private_canvas->invert_yaxis = 0;

  /* move to bottom-left */
  cdTextTranslatePoint(x, y, w, h, baseline, &xmin, &ymin);

  xmax = xmin + w-1;
  ymax = ymin + h-1;

  if (active_private_canvas->text_orientation)
  {
    double cos_theta = cos(active_private_canvas->text_orientation/CD_RAD2DEG);
    double sin_theta = sin(active_private_canvas->text_orientation/CD_RAD2DEG);

    cdTextRotatePoint(xmin, ymin, x, y, &rect[0], &rect[1], sin_theta, cos_theta);
    cdTextRotatePoint(xmax, ymin, x, y, &rect[2], &rect[3], sin_theta, cos_theta);
    cdTextRotatePoint(xmax, ymax, x, y, &rect[4], &rect[5], sin_theta, cos_theta);
    cdTextRotatePoint(xmin, ymax, x, y, &rect[6], &rect[7], sin_theta, cos_theta);
  }
  else
  {
    rect[0] = xmin; rect[1] = ymin;
    rect[2] = xmax; rect[3] = ymin;
    rect[4] = xmax; rect[5] = ymax;
    rect[6] = xmin; rect[7] = ymax;
  }

  active_private_canvas->invert_yaxis = old_invert_yaxis;
}

void cdTextBox(int x, int y, char *s, int *xmin, int *xmax, int *ymin, int *ymax)
{
  int rect[8];
  int _xmin, _xmax, _ymin, _ymax;

  cdTextBounds(x, y, s, rect);

  _xmin = rect[0];
  _ymin = rect[1];
  _xmax = rect[0];
  _ymax = rect[1];

  if(rect[2] < _xmin) _xmin = rect[2];
  if(rect[4] < _xmin) _xmin = rect[4];
  if(rect[6] < _xmin) _xmin = rect[6];

  if(rect[3] < _ymin) _ymin = rect[3];
  if(rect[5] < _ymin) _ymin = rect[5];
  if(rect[7] < _ymin) _ymin = rect[7];

  if(rect[2] > _xmax) _xmax = rect[2];
  if(rect[4] > _xmax) _xmax = rect[4];
  if(rect[6] > _xmax) _xmax = rect[6];

  if(rect[3] > _ymax) _ymax = rect[3];
  if(rect[5] > _ymax) _ymax = rect[5];
  if(rect[7] > _ymax) _ymax = rect[7];

  if (xmin) *xmin = _xmin;
  if (xmax) *xmax = _xmax;
  if (ymin) *ymin = _ymin;
  if (ymax) *ymax = _ymax;
}

/****************/
/* color coding */
/****************/

long int cdEncodeColor(unsigned char r, unsigned char g, unsigned char b)
{
  return (((unsigned long)r) << 16) |
         (((unsigned long)g) <<  8) |
         (((unsigned long)b) <<  0);
}

void cdDecodeColor(long int color, unsigned char *r, unsigned char *g, unsigned char *b)
{
  *r = cdRed(color);
  *g = cdGreen(color);
  *b = cdBlue(color);
}

unsigned char cdDecodeAlpha(long int color)
{
  unsigned char alpha = cdReserved(color);
  return ~alpha;
}

long int cdEncodeAlpha(long int color, unsigned char alpha)
{
  alpha = ~alpha;
  return (((unsigned long)alpha) << 24) | (color & 0xFFFFFF);
}

int cdGetColorPlanes(void)
{
  assert(active_private_canvas);
  return active_private_canvas->bpp;
}

void cdPalette(int n, long int *palette, int mode)
{
  assert(active_private_canvas);
  assert(n>0);
  assert(palette);
  assert(mode>=CD_POLITE && mode<=CD_FORCE);

  if (n <= 0 || active_private_canvas->bpp > 8) 
    return;

  active_private_canvas->Palette(n, palette, mode);
}

/*****************/
/* client images */
/*****************/

static void norm_limits(int w, int h, int *xmin, int *xmax, int *ymin, int *ymax)
{
  *xmin = *xmin < 0? 0: *xmin < w? *xmin: (w - 1);
  *ymin = *ymin < 0? 0: *ymin < h? *ymin: (h - 1);
  *xmax = *xmax < 0? 0: *xmax < w? *xmax: (w - 1);
  *ymax = *ymax < 0? 0: *ymax < h? *ymax: (h - 1);
}

void cdGetImageRGB(unsigned char *r, unsigned char *g, unsigned char *b, int x, int y, int w, int h)
{
  assert(active_private_canvas);
  assert(r);
  assert(g);
  assert(b);
  assert(w>0);
  assert(h>0);

  if (active_private_canvas->origin)
  {
    x += active_private_canvas->origin_x;
    y += active_private_canvas->origin_y;
  }

  if (active_private_canvas->invert_yaxis)
    y = _cdInvertYAxis(y);

  if (active_private_canvas->GetImageRGB)
    active_private_canvas->GetImageRGB(r, g, b, x, y, w, h);
}

void cdPutImageRectRGB(int iw, int ih, unsigned char *r, unsigned char *g, unsigned char *b, int x, int y, int w, int h, int xmin, int xmax, int ymin, int ymax)
{
  assert(active_private_canvas);
  assert(iw>0);
  assert(ih>0);
  assert(r);
  assert(g);
  assert(b);

  if (w == 0) w = iw;
  if (h == 0) h = ih;
  if (xmax == 0) xmax = iw - 1;
  if (ymax == 0) ymax = ih - 1;

  if (!check_box(xmin, xmax, ymin, ymax))
    return;

  norm_limits(iw, ih, &xmin, &xmax, &ymin, &ymax);

  if (active_private_canvas->origin)
  {
    x += active_private_canvas->origin_x;
    y += active_private_canvas->origin_y;
  }

  if (active_private_canvas->invert_yaxis)
  {
    y = _cdInvertYAxis(y);
  }

  wdCall(0);

  if (active_private_canvas->bpp <= 8)
  {
    int height = ymax-ymin+1;
    unsigned char* map = (unsigned char*)malloc(iw* height);
    int pal_size = 1L << active_private_canvas->bpp;
    long *colors = (long *)malloc(pal_size*4);

    if (!map || !colors)
    {
      if (map) free(map);
      if (colors) free(colors);
      return;
    }

    cdRGB2Map(iw, height, r+ymin*iw, g+ymin*iw, b+ymin*iw, map, pal_size, colors);
    active_private_canvas->PutImageRectMap(iw, height, map, colors, x, y, w, h, xmin, xmax, 0, height-1);

    free(map);
    free(colors);
  }
  else
    active_private_canvas->PutImageRectRGB(iw, ih, r, g, b, x, y, w, h, xmin, xmax, ymin, ymax);
}

void cdPutImageRectRGBA(int iw, int ih, unsigned char *r, unsigned char *g, unsigned char *b, unsigned char *a, int x, int y, int w, int h, int xmin, int xmax, int ymin, int ymax)
{
  assert(active_private_canvas);
  assert(iw>0);
  assert(ih>0);
  assert(r);
  assert(g);
  assert(b);

  if (w == 0) w = iw;
  if (h == 0) h = ih;
  if (xmax == 0) xmax = iw - 1;
  if (ymax == 0) ymax = ih - 1;

  if (!check_box(xmin, xmax, ymin, ymax))
    return;

  norm_limits(iw, ih, &xmin, &xmax, &ymin, &ymax);

  if (active_private_canvas->origin)
  {
    x += active_private_canvas->origin_x;
    y += active_private_canvas->origin_y;
  }

  if (active_private_canvas->invert_yaxis)
  {
    y = _cdInvertYAxis(y);
  }

  wdCall(0);
  active_private_canvas->PutImageRectRGBA(iw, ih, r, g, b, a, x, y, w, h, xmin, xmax, ymin, ymax);
}

static long* GrayColorMap(void)
{
  static long color_map[256] = {1};

  if (color_map[0])
  {
    int c;
    for (c = 0; c < 256; c++)
      color_map[c] = cdEncodeColor((unsigned char)c, (unsigned char)c, (unsigned char)c);
  }

  return color_map;
}

void cdPutImageRectMap(int iw, int ih, unsigned char *index, long int *colors, int x, int y, int w, int h, int xmin, int xmax, int ymin, int ymax)
{
  assert(active_private_canvas);
  assert(index);
  assert(iw>0);
  assert(ih>0);

  if (w == 0) w = iw;
  if (h == 0) h = ih;
  if (xmax == 0) xmax = iw - 1;
  if (ymax == 0) ymax = ih - 1;

  if (!check_box(xmin, xmax, ymin, ymax))
    return;

  norm_limits(iw, ih, &xmin, &xmax, &ymin, &ymax);

  if (active_private_canvas->origin)
  {
    x += active_private_canvas->origin_x;
    y += active_private_canvas->origin_y;
  }

  if (active_private_canvas->invert_yaxis)
  {
    y = _cdInvertYAxis(y);
  }

  if (colors == NULL)
    colors = GrayColorMap();

  wdCall(0);
  active_private_canvas->PutImageRectMap(iw, ih, index, colors, x, y, w, h, xmin, xmax, ymin, ymax);
}

/*****************/
/* server images */
/*****************/

void* cdCreateImage(int w, int h)
{
  cdServerImage* server_image;
  void *image;

  assert(active_private_canvas);
  assert(w>0);
  assert(h>0);

  image = active_private_canvas->CreateImage(w, h);
  if (!image)
    return NULL;

  server_image = (cdServerImage*)malloc(sizeof(cdServerImage));

  server_image->GetImage = active_private_canvas->GetImage;
  server_image->PutImageRect = active_private_canvas->PutImageRect;
  server_image->KillImage = active_private_canvas->KillImage;

  server_image->w = w;
  server_image->h = h;

  server_image->img = image;

  return server_image;
}

void cdGetImage(void* image, int x, int y)
{
  cdServerImage* server_image;

  assert(image);
  assert(active_private_canvas);

  server_image = (cdServerImage*)image;

  if (active_private_canvas->origin)
  {
    x += active_private_canvas->origin_x;
    y += active_private_canvas->origin_y;
  }

  if (active_private_canvas->invert_yaxis)
    y = _cdInvertYAxis(y);

  server_image->GetImage(server_image->img, x, y);
}

void cdPutImageRect(void* image, int x, int y, int xmin, int xmax, int ymin, int ymax)
{
  cdServerImage* server_image;

  assert(image);
  assert(active_private_canvas);

  server_image = (cdServerImage*)image;

  if (xmax == 0) xmax = server_image->w - 1;
  if (ymax == 0) ymax = server_image->h - 1;

  if (!check_box(xmin, xmax, ymin, ymax))
    return;

  norm_limits(server_image->w, server_image->h, &xmin, &xmax, &ymin, &ymax);

  if (active_private_canvas->origin)
  {
    x += active_private_canvas->origin_x;
    y += active_private_canvas->origin_y;
  }

  if (active_private_canvas->invert_yaxis)
  {
    y = _cdInvertYAxis(y);
  }

  wdCall(0);
  server_image->PutImageRect(server_image->img, x, y, xmin, xmax, ymin, ymax);
}

void cdKillImage(void* image)
{
  cdServerImage* server_image;

  assert(image);

  server_image = (cdServerImage*)image;
  server_image->KillImage(server_image->img);
  free(server_image);
}

void cdScrollArea(int xmin, int xmax, int ymin, int ymax, int dx, int dy)
{
  assert(active_private_canvas);

  if (!check_box(xmin, xmax, ymin, ymax))
    return;

  if (dx == 0 && dy == 0)
    return;

  if (active_private_canvas->origin)
  {
    xmin += active_private_canvas->origin_x;
    xmax += active_private_canvas->origin_x;
    ymin += active_private_canvas->origin_y;
    ymax += active_private_canvas->origin_y;
  }

  if (active_private_canvas->invert_yaxis)
  {
    dy = -dy;
    ymin = _cdInvertYAxis(ymin);
    ymax = _cdInvertYAxis(ymax);
    SWAPINT(ymin, ymax);
  }

  wdCall(0);
  active_private_canvas->ScrollArea(xmin, xmax, ymin, ymax, dx, dy);
}

/*******************/
/* extended images */
/*******************/

typedef struct _cdBitmapData 
{
  unsigned char *index;
  long int *colors;

  unsigned char *r;
  unsigned char *g;
  unsigned char *b;
  
  unsigned char *a;

  int free_data;

  int xmin, xmax, ymin, ymax;
} cdBitmapData;


cdBitmap* cdCreateBitmap(int w, int h, int type)
{
  int size = w * h;
  cdBitmap* image = (cdBitmap*)malloc(sizeof(cdBitmap));
  cdBitmapData* data = (cdBitmapData*)malloc(sizeof(cdBitmapData));
  memset(data, 0, sizeof(cdBitmapData));

  assert(w>0);
  assert(h>0);

  image->w = w;
  image->h = h;
  image->type = type;
  image->data = data;
  data->free_data = 1;

  if (type == CD_RGB || type == CD_RGBA)
  {
    data->r = (unsigned char*)malloc(size);
    data->g = (unsigned char*)malloc(size);
    data->b = (unsigned char*)malloc(size);

    if (!data->r || !data->g || !data->b)
    {
      if (data->r) free(data->r);
      if (data->g) free(data->g);
      if (data->b) free(data->b);
      free(data);
      data = NULL;
    }

    if (type == CD_RGBA)
    {
      data->a = (unsigned char*)malloc(size);
      if (!data->a)
      {
        free(data->r);
        free(data->g);
        free(data->b);
        free(data);
        data = NULL;
      }
    }
  }
  else
  {
    image->type = CD_MAP;
    data->index = (unsigned char*)malloc(size);
    data->colors = (long int*)malloc(256*sizeof(long int));

    if (!data->index || !data->colors)
    {
      if (data->index) free(data->index);
      if (data->colors) free(data->colors);
      free(data);
      data = NULL;
    }
  }

  data->xmin = 0;
  data->ymin = 0;
  data->xmax = image->w-1;
  data->ymax = image->h-1;

  return image;
}

cdBitmap* cdInitBitmap(int w, int h, int type, ...)
{
  va_list arglist;
  cdBitmap* image = (cdBitmap*)malloc(sizeof(cdBitmap));
  cdBitmapData* data = (cdBitmapData*)malloc(sizeof(cdBitmapData));
  memset(data, 0, sizeof(cdBitmapData));

  assert(w>0);
  assert(h>0);

  image->w = w;
  image->h = h;
  image->type = type;
  image->data = data;
  data->free_data = 0;

  va_start(arglist, type);

  if (type == CD_RGB || type == CD_RGBA)
  {
    data->r = va_arg(arglist, unsigned char*);
    data->g = va_arg(arglist, unsigned char*);
    data->b = va_arg(arglist, unsigned char*);

    assert(data->r);
    assert(data->g);
    assert(data->b);

    if (type == CD_RGBA)
    {
      data->a = va_arg(arglist, unsigned char*);
      assert(data->a);
    }
  }
  else
  {
    image->type = CD_MAP;
    data->index = va_arg(arglist, unsigned char*);
    data->colors = va_arg(arglist, long int*);

    assert(data->index);
    assert(data->colors);

  }

  data->xmin = 0;
  data->ymin = 0;
  data->xmax = image->w-1;
  data->ymax = image->h-1;

  return image;
}

void cdKillBitmap(cdBitmap* image)
{
  cdBitmapData* data;

  assert(image);
  assert(image->data);

  data = (cdBitmapData*)image->data;

  if (data->free_data == 0)
  {
    free(data);
    free(image);
    return;
  }

  if (image->type == CD_RGB || image->type == CD_RGBA)
  {
    free(data->r);
    free(data->g);
    free(data->b);

    if (image->type == CD_RGBA)
    {
      free(data->a);
    }
  }
  else
  {
    free(data->index);
    free(data->colors);
  }

  free(data);
  free(image);
}

void cdGetBitmap(cdBitmap* image, int x, int y)
{
  cdBitmapData* data;

  assert(image);
  assert(image->data);

  data = (cdBitmapData*)image->data;

  if (image->type == CD_RGB || image->type == CD_RGBA)
  {
    cdGetImageRGB(data->r, data->g, data->b, x, y, image->w, image->h);
    return;
  }
}

void cdBitmapRGB2Map(cdBitmap* image_rgb, cdBitmap* image_map)
{
  cdBitmapData* data_rgb;
  cdBitmapData* data_map;

  assert(image_rgb);
  assert(image_rgb->data);
  assert(image_map);
  assert(image_map->data);

  data_rgb = (cdBitmapData*)image_rgb->data;
  data_map = (cdBitmapData*)image_map->data;

  if ((image_rgb->type != CD_RGB && image_rgb->type != CD_RGBA) || (image_map->type <= 0))
    return;

  cdRGB2Map(image_rgb->w, image_rgb->h, data_rgb->r, data_rgb->g, data_rgb->b, data_map->index, image_map->type, data_map->colors);
}

unsigned char* cdBitmapGetData(cdBitmap* image, int dataptr)
{
  cdBitmapData* data;

  assert(image);
  assert(image->data);

  data = (cdBitmapData*)image->data;

  switch(dataptr)
  {
  case CD_IRED:
    return data->r;
  case CD_IGREEN:
    return data->g;
  case CD_IBLUE:
    return data->b;
  case CD_IALPHA:
    return data->a;
  case CD_INDEX:
    return data->index;
  case CD_COLORS:
    return (unsigned char*)data->colors;
  }

  return NULL;
}

void cdBitmapSetRect(cdBitmap* image, int xmin, int xmax, int ymin, int ymax)
{
  cdBitmapData* data;

  assert(image);
  assert(image->data);

  data = (cdBitmapData*)image->data;

  data->xmin = xmin;
  data->xmax = xmax;
  data->ymin = ymin;
  data->ymax = ymax;
}

void cdPutBitmap(cdBitmap* image, int x, int y, int w, int h)
{
  cdBitmapData* data;

  assert(image);
  assert(image->data);

  data = (cdBitmapData*)image->data;

  switch(image->type)
  {
  case CD_RGB:
    cdPutImageRectRGB(image->w, image->h, data->r, data->g, data->b, x, y, w, h, data->xmin, data->xmax, data->ymin, data->ymax);
    return;
  case CD_RGBA:
    cdPutImageRectRGBA(image->w, image->h, data->r, data->g, data->b, data->a, x, y, w, h, data->xmin, data->xmax, data->ymin, data->ymax);
    return;
  default:
    cdPutImageRectMap(image->w, image->h, data->index, data->colors, x, y, w, h, data->xmin, data->xmax, data->ymin, data->ymax);
    return;
  }
}

/****************************************************************************/

/*
%F Cria um vetor x com dx entradas, tal que x[k] eh o numero de ordem do
   intervalo ao qual k pertence quando se divide um espaco de largura dx
   em p intervalos.

   Por: Diego Fernandes Nehab
   Em:  01/04/96
*/
void cdMakeStretchTable(int dx, int p, int *x) 
{

  int i;                                /* contador dos intervalos */
  int l;                                /* largura do intervalo */
  int k;                                /* offset no vetor x */
  int e;                                /* erro da aproximacao inteira */
  int _2p;                              /* dobro do numero de intervalos */
  int q;                                /* quociente da divisao inteira */
  int _2r;                              /* dobro do resto da divisao inteira */

  /* inicializa as variaveis */
  k = 0;                                /* offset no vetor x */
  e = 0;                                /* inicializa o erro */
  _2p = p << 1;                         /* dobro do numero de intervalos */
  _2r = (dx % p) << 1;                  /* dobro do resto da divisao inteira */
  q = dx / p;                           /* quociente da divisao inteira */

  /* gera o vetor de mapeamento */
  for (i=0; i<p; i++)                   /* para p intervalos */
  {
    e += _2r;                           /* incrementa o erro */

    if (e >= p)                         /* estourou? */
    {
      e -= _2p;                         /* ajusta o novo limite */
      l = q + 1;                        /* arredonda para cima */
    }
    else 
    {
      l = q;                            /* arredonda para baixo */
    }

    while (l) 
    {
      x[k++] = i;                       /* preenche o intervalo com... */
      l--;                              /* ...seu numero de ordem */
    }
  }
}

/* funcao usada para calcular os retangulos efetivos de zoom 
   de imagens clientes. Pode ser usada para os eixos X e Y.

   canvas_size - tamanho do canvas (canvas->w, canvas->h)
   cnv_rect_pos - posicao no canvas onde a regiao sera´ desenhada (x, y)
   cnv_rect_size - tamanho da regiao no canvas com zoom (w, h)
   img_rect_pos - posicao na imagem da regiao a ser desenhada (min)
   img_rect_size - tamanho da regiao na imagem (max-min+1)

   calcula o melhor tamanho a ser usado
   retorna 0 se o retangulo resultante e´ nulo
*/
int cdCalcZoom(int canvas_size,
               int cnv_rect_pos, int cnv_rect_size, 
               int *new_cnv_rect_pos, int *new_cnv_rect_size, 
               int img_rect_pos, int img_rect_size, 
               int *new_img_rect_pos, int *new_img_rect_size, 
               int is_horizontal)
{
  int offset;
  float zoom_factor = (float)img_rect_size / (float)cnv_rect_size;

  /* valores default sem otimizacao */
  *new_cnv_rect_size = cnv_rect_size, *new_cnv_rect_pos = cnv_rect_pos;    
  *new_img_rect_size = img_rect_size, *new_img_rect_pos = img_rect_pos;

  if (cnv_rect_size > 0)
  {
    /* se posicao no canvas > tamanho do canvas, fora da janela, nao desenha nada */
    if (cnv_rect_pos >= canvas_size) 
      return 0;

    /* se posicao no canvas + tamanho da regiao no canvas < 0, fora da janela, nao desenha nada */
    if (cnv_rect_pos+cnv_rect_size < 0) 
      return 0;

    /* se posicao no canvas < 0, entao comeca fora do canvas melhor posicao no canvas e' 0
                                 E o tamanho e' reduzido do valor negativo */
    if (cnv_rect_pos < 0) 
    {
      /* valores ajustados para cair numa vizinhanca de um pixel da imagem */
      offset = (int)ceil(cnv_rect_pos*zoom_factor);   /* offset is <0 */
      offset = (int)ceil(offset/zoom_factor);
      *new_cnv_rect_pos -= offset;  
      *new_cnv_rect_size += offset; 
    }

    /* se posicao no canvas + tamanho da regiao no canvas > tamanho do canvas, 
       termina fora do canvas entao 
       o tamanho da regiao no canvas e' o tamanho do canvas reduzido da posicao */
    if (*new_cnv_rect_pos+*new_cnv_rect_size > canvas_size) 
    {
      offset = (int)((*new_cnv_rect_pos+*new_cnv_rect_size - canvas_size)*zoom_factor);
      *new_cnv_rect_size -= (int)(offset/zoom_factor);  /* offset is >0 */
    }
  }
  else
  {
    /* cnv_rect_size tamanho negativo, significa imagem top down */
    /* calculos adicionados pela Paula */

    /* se posicao no canvas + tamanho no canvas (xmin+1) >= tamanho do canvas, fora da janela, nao desenha nada */
    if (cnv_rect_pos+cnv_rect_size >= canvas_size) 
      return 0;

    /* se posicao da imagem com zoom (xmax) < 0, fora da janela, nao desenha nada */
    if (cnv_rect_pos < 0) 
      return 0;

    /* se posicao com zoom (xmax) >= tamanho do canvas, posicao da imagem com zoom e' o tamanho do canvas menos um
       tambem o tamanho e' reduzido do valor negativo */
    if (cnv_rect_pos >= canvas_size) 
    {
      *new_cnv_rect_pos = canvas_size-1; 
      *new_cnv_rect_size = cnv_rect_size + (cnv_rect_pos - *new_cnv_rect_pos);
    }

    /* se posicao + tamanho com zoom (xmin+1) < 0, 
       entao o tamanho com zoom e' a posição + 1 */
    if (cnv_rect_pos+cnv_rect_size < 0) 
      *new_cnv_rect_size = -(*new_cnv_rect_pos + 1);
  }

  /* agora ja' tenho tamanho e posicao da regiao no canvas,
     tenho que obter tamanho e posicao dentro da imagem original,
     baseado nos novos valores */

  /* tamanho da regiao na imagem e' a conversao de zoom para real do tamanho no canvas */
  *new_img_rect_size = (int)(*new_cnv_rect_size * zoom_factor + 0.5);

  if (is_horizontal)
  {
    /* em X, o offset dentro da imagem so' existe se houver diferenca entre a posicao inicial da
       imagem e a posicao otimizada (ambas da imagem com zoom) */
    if (*new_cnv_rect_pos != cnv_rect_pos)
    {
      offset = *new_cnv_rect_pos - cnv_rect_pos; /* offset is >0 */
      *new_img_rect_pos += (int)(offset*zoom_factor);
    }
  }
  else
  {
    /* em Y, o offset dentro da imagem so' existe se houver diferenca entre a posicao 
       final (posição inicial + tamanho) da imagem e a posicao otimizada (ambas da 
       imagem com zoom) */
    if ((cnv_rect_pos + cnv_rect_size) != (*new_cnv_rect_pos + *new_cnv_rect_size))
    {
      /* offset is >0, because Y axis is from top to bottom */
      offset = (cnv_rect_pos + cnv_rect_size) - (*new_cnv_rect_pos + *new_cnv_rect_size);
      *new_img_rect_pos += (int)(offset*zoom_factor);
    }
  }

  return 1;
}

int cdGetFileName(const char* strdata, char* filename)
{
  const char* start = strdata;
  if (!strdata || strdata[0] == 0) return 0;
  
  if (strdata[0] == '\"')
  {   
    strdata++; /* the first " */
    while(*strdata && *strdata != '\"')
      *filename++ = *strdata++;
    strdata++; /* the last " */
  }
  else
  {
    while(*strdata && *strdata != ' ')
      *filename++ = *strdata++;
  }

  if (*strdata == ' ')
    strdata++;

  *filename = 0;
  return (int)(strdata - start);
}

