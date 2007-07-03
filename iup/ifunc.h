/** \file
 * \brief Global Function table.
 *
 * See Copyright Notice in iup.h
 * $Id: ifunc.h,v 1.6 2006/01/19 22:03:20 scuri Exp $
 */

#ifndef __IFUNC_H
#define __IFUNC_H

#ifdef __cplusplus
extern "C"
{
#endif

  void iupFuncInit (void);
  void iupFuncFinish (void);

/* Other functions declared in <iup.h> and implemented here. 
char *IupGetActionName (void);
Icallback IupGetFunction (const char *name);
Icallback IupSetFunction (const char *name, Icallback func);
*/

#ifdef __cplusplus
}
#endif

#endif
