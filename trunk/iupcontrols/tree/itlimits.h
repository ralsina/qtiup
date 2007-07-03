/** \file
 * \brief iuptree control
 * Functions used to verify limits between specified canvas regions.
 *
 * See Copyright Notice in iup.h
 * $Id: itlimits.h,v 1.3 2005/07/11 15:51:40 uid20021 Exp $
 */
 
#ifndef __ITLIMITS_H 
#define __ITLIMITS_H

#ifdef __cplusplus
extern "C" {
#endif

int treeInsideRegion(int x, int y, int x0, int y0, int w, int h );

#ifdef __cplusplus
}
#endif

#endif
