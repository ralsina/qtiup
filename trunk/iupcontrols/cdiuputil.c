/** \file
 * \brief cdiuputil. CD and IUP utilities for the IupControls
 *
 * See Copyright Notice in iup.h
 * $Id: cdiuputil.c,v 1.12 2006/03/22 22:16:59 scuri Exp $
 */


#include <stdlib.h>
#include <stdarg.h>

#include <iup.h>
#include <cd.h>
#include <iupcompat.h>
#include "cdiuputil.h"

Ihandle** iupGetParamList(Ihandle* first, va_list arglist)
{
  const int INITIAL_NUMBER = 50;
  Ihandle **params = NULL;
  Ihandle *elem = NULL;
  int max_elements = 0, num_elements = 0;

  params = (Ihandle **) malloc (sizeof (Ihandle *) * INITIAL_NUMBER);

  max_elements = INITIAL_NUMBER;

  elem = first;

  while (elem != NULL)
  {
    params[num_elements] = elem;
    num_elements++;

    /* verifica se precisa realocar memoria */
    if (num_elements >= max_elements)
    {
      Ihandle **new_params = NULL;

      max_elements += INITIAL_NUMBER;

      new_params = (Ihandle **) realloc (params, sizeof (Ihandle *) * max_elements);

      params = new_params;
    }

    elem = va_arg (arglist, Ihandle*);
  }
  params[num_elements] = NULL;

  return params;
}

long cdIupConvertColor(char *color)
{
  unsigned int ri, gi, bi;
  iupGetRGB(color, &ri, &gi, &bi);
  return cdEncodeColor((unsigned char)ri,(unsigned char)gi,(unsigned char)bi);
}

char *cdIupGetParentBgColor (Ihandle* self)
{
  char *color =
    IupGetAttribute (IupGetParent (self), IUP_BGCOLOR);

  if (color == NULL)
    color = IupGetAttribute (IupGetDialog (self), IUP_BGCOLOR);

  if (color == NULL)
    color = "128 128 128";

  return color;
}

char *cdIupGetBgColor(Ihandle* self)
{
  char *color = iupGetEnv(self, IUP_BGCOLOR);

  if(color == NULL)
    color = cdIupGetParentBgColor(self);

  return color;
}

void cdIupCalcShadows(long bgcolor, long *light_shadow, long *mid_shadow, long *dark_shadow)
{
  int r, bg_r = cdRed(bgcolor);
  int g, bg_g = cdGreen(bgcolor);
  int b, bg_b = cdBlue(bgcolor);

  /* light_shadow */

  int max = bg_r;
  if (bg_g > max) max = bg_g;
  if (bg_b > max) max = bg_b;

  if (255-max < 64)
  {
    r = 255;
    g = 255;
    b = 255;
  }
  else
  {
    /* preserve some color information */
    if (bg_r == max) r = 255;
    else             r = bg_r + (255-max);
    if (bg_g == max) g = 255;
    else             g = bg_g + (255-max);
    if (bg_b == max) b = 255;
    else             b = bg_b + (255-max);
  }

  if (light_shadow) *light_shadow = cdEncodeColor(r, g, b); 

  /* dark_shadow */

  r = bg_r - 128;
  g = bg_g - 128;
  b = bg_b - 128;
  if (r < 0) r = 0;
  if (g < 0) g = 0;
  if (b < 0) b = 0;

  if (dark_shadow) *dark_shadow = cdEncodeColor(r, g, b); 

  /* mid_shadow = (dark_shadow+bgcolor)/2*/

  if (mid_shadow) *mid_shadow = cdEncodeColor((bg_r+r)/2, (bg_g+g)/2, (bg_b+b)/2);
}

void cdIupDrawSunkenRect(int x1, int y1, int x2, int y2, long light_shadow, long mid_shadow, long dark_shadow)
{
  cdForeground( mid_shadow );
  cdLine( x1, y1+1,   x1, y2 );
  cdLine( x1,  y2, x2-1, y2 );

  cdForeground( dark_shadow );
  cdLine( x1+1, y1+2, x1+1, y2-1 );
  cdLine( x1+1, y2-1, x2-2, y2-1 );

  cdForeground( light_shadow );
  cdLine( x1, y1, x2, y1 );
  cdLine( x2, y1, x2, y2 );
}

void cdIupDrawRaisenRect(int x1, int y1, int x2, int y2, long light_shadow, long mid_shadow, long dark_shadow)
{
  cdForeground( light_shadow );
  cdLine( x1, y1+1,   x1, y2 );
  cdLine( x1,  y2, x2-1, y2 );

  cdForeground( dark_shadow );
  cdLine( x1, y1, x2, y1 );
  cdLine( x2, y1, x2, y2 );

  cdForeground( mid_shadow );
  cdLine( x1+1, y1+1, x2-1, y1+1 );
  cdLine( x2-1, y1+2, x2-1, y2-1 );
}

void cdIupDrawVertSunkenMark(int x, int y1, int y2, long light_shadow, long dark_shadow)
{
  cdForeground( dark_shadow );
  cdLine( x-1, y1, x-1, y2 );
  cdForeground( light_shadow );
  cdLine(   x, y1,   x, y2 );
}

void cdIupDrawHorizSunkenMark(int x1, int x2, int y, long light_shadow, long dark_shadow)
{
  cdForeground( dark_shadow );
  cdLine( x1, y+1, x2, y+1 );
  cdForeground( light_shadow );
  cdLine( x1, y, x2, y );
}

#ifdef WIN32
#include <windows.h>
#else
#endif

void cdIupDrawFocusRect(Ihandle* self, int x1, int y1, int x2, int y2)
{
  int y, x, w, h;

  cdUpdateYAxis(&y1);
  cdUpdateYAxis(&y2);
  y = y1;
  if (y2<y1) y = y2;
  x = x1;
  if (x2<x1) x = x2;

  w = abs(x2-x1)+1;
  h = abs(y2-y1)+1;

#ifdef WIN32
  {
    HDC hDC = (HDC)cdGetAttribute("HDC");
    RECT rect;
    (void)self;

    rect.left = x;  
    rect.top = y;  
    rect.right = x+w;  
    rect.bottom = y+h;

    DrawFocusRect(hDC, &rect);
  }
#else
  //FIXME implement this stuff for Qt
#endif
}

char* cdIupSetFontBold(char* font, int bold)
{
  static char new_font[200];
  int c;
  if (!bold)
    return font;

#ifdef WIN32  
  c = (int)strcspn(font, ":")+1;      /* extract typeface */

  new_font[0] = 0;
  strncpy(new_font, font, c);
  new_font[c] = 0;
  font += c;  /* jump typeface and separator */

  strcat(new_font, "BOLD");

  c = (int)strcspn(font, ":");      /* extract attrib */
  font += c;
  strcat(new_font, font);
#else
  //FIXME implement this stuff for Qt
#endif

  return new_font;
}

