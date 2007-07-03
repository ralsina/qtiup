/** \file
 * \brief iuptree control
 * Functions used to find nodes in the tree.
 *
 * See Copyright Notice in iup.h
 * $Id: itfind.h,v 1.5 2003/06/16 18:56:37 scuri Exp $
 */
 
#ifndef __ITFIND_H 
#define __ITFIND_H

#include "iuptree.h"

#ifdef __cplusplus
extern "C" {
#endif

int   treefindNodeIdFromXY(Ihandle *h, int xmouse, int ymouse);
void  treefindStartEndSelection(Ihandle *h, int *begin, int *end);
Node  treefindNodeFromId(Ihandle *h, int id);
Node  treefindNodeFromString(Ihandle *h, char* id_string);
Node  treefindMarked( Ihandle *h, char* id_string);
Node  treefindPrevious(Node temp, Node node);
Node  treefindParent(Ihandle* h, char* id_string);
int   treefindMaxDepth(Node node);
int   treefindNumNodes(Ihandle *h);
float treefindNumNodesInCanvas(Ihandle *h);
int   treefindNodeId(Ihandle *h, Node node);
int   treefindTotNumNodes(Ihandle *h);

#ifdef __cplusplus
}
#endif

#endif
