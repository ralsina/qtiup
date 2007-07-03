/** \file
 * \brief iuptree control
 * Functions used to verify limits between specified canvas regions
 *
 * See Copyright Notice in iup.h
 * $Id: itlimits.c,v 1.4 2005/07/11 15:51:40 uid20021 Exp $
 */

#include "itlimits.h"
#include <cd.h>

/*

%F Determines if a pixel is inside a region
%i x      : pixel horizontal coordinate
   y      : pixel vertical coordinate
   x0     : region left border
   y0     : region top border
   w      : region width
   h      : region height
%o returns 1 on sucess and 0 otherwise */
int treeInsideRegion(int x, int y, int x0, int y0, int w, int h )
{
  cdUpdateYAxis(&y);
  y++;
	
  return (x >= x0 && x <= x0 + w && y >= y0 && y <= y0 + h);
}
