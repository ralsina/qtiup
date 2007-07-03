/** \file
 * \brief iuptree control
 * Functions used to handle the mouse.
 *
 * See Copyright Notice in iup.h
 * $Id: itmouse.h,v 1.4 2005/07/11 15:51:40 uid20021 Exp $
 */
 
#ifndef __ITMOUSE_H 
#define __ITMOUSE_H

#ifdef __cplusplus
extern "C" {
#endif

int treeMouseMotionCb (Ihandle *ht, int x, int y, char *r);
int treeMouseButtonCb (Ihandle *ht, int b, int press, int x, int y, char *r);
int treeCollapseButton(Ihandle *h, int xmouse, int ymouse, int x, int y);
int treeExpandButton(Ihandle* h, int xmouse, int ymouse, int x, int y);
int treeCollapsedBranch(Ihandle* h, int xmouse, int ymouse, int x, int y);
int treeLeaf(Ihandle* h, int xmouse, int ymouse, int x, int y);
int treeText(Ihandle* h, Node node, int text_x, int xmouse, int ymouse, int x, int y);

#ifdef __cplusplus
}
#endif

#endif
