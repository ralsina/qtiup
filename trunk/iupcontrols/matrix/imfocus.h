/** \file
 * \brief iupmatrix focus control.
 *
 * See Copyright Notice in iup.h
 * $Id: imfocus.h,v 1.10 2006/01/19 22:03:21 scuri Exp $
 */
 
#ifndef __IMFOCUS_H 
#define __IMFOCUS_H

#ifdef __cplusplus
extern "C" {
#endif

int iupmatFocusCb(Ihandle *hm, int focus);

void iupmatShowFocus  (Ihandle *h);
void iupmatHideFocus  (Ihandle *h);
void iupmatSetFocusPos(Ihandle *h, int lin, int col);

void iupmatHideSetShowFocus(Ihandle *h, int lin, int col);
void iupmatSetShowFocus    (Ihandle *h, int lin, int col);

int iupmatIsFocusVisible(Ihandle *h);

#ifdef __cplusplus
}
#endif

#endif
