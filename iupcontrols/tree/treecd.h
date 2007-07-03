/** \file
 * \brief CD canvas utilities.
 * Header file that contains a function to activate the canvas
 *
 * See Copyright Notice in iup.h
 * $Id: treecd.h,v 1.2 2003/06/16 18:56:37 scuri Exp $
 */
 
#ifndef __TREECD_H 
#define __TREECD_H

#include <cd.h>
#include <cdiup.h>

#ifdef __cplusplus
extern "C" {
#endif

#define CdActivate(tree,err)  do{if((tree)->cdh) \
                                  err = cdActivate((tree)->cdh); \
                                else                            \
                                  err = CD_ERROR;               \
                               }while(0)

#ifdef __cplusplus
}
#endif

#endif
