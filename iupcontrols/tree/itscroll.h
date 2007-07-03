/** \file
 * \brief iuptree control
 * Functions used to scroll the tree.
 *
 * See Copyright Notice in iup.h
 * $Id: itscroll.h,v 1.3 2005/07/11 15:51:40 uid20021 Exp $
 */
 
#ifndef __ITSCROLL_H 
#define __ITSCROLL_H

#ifdef __cplusplus
extern "C" {
#endif

int treeScrollDown(Ihandle *h);
int treeScrollUp(Ihandle *h);
int treeScrollPgUp(Ihandle *h);
int treeScrollPgDn(Ihandle *h);
int treeScrollEnd(Ihandle *h);
int treeScrollBegin(Ihandle *h);
int treeScrollShow(Ihandle *h);

#ifdef __cplusplus
}
#endif

#endif
