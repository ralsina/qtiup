/** \file
 * \brief iupmatrix control
 * mouse events.
 *
 * See Copyright Notice in iup.h
 * $Id: immouse.h,v 1.3 2003/06/16 18:56:37 scuri Exp $
 */
 
#ifndef __IMMOUSE_H 
#define __IMMOUSE_H

#ifdef __cplusplus
extern "C" {
#endif

int iupmatMouseButtonCb (Ihandle *hm, int b, int press, int x, int y, char *r);
int iupmatMouseMoveCb   (Ihandle *hm, int x, int y);

#ifdef __cplusplus
}
#endif

#endif
