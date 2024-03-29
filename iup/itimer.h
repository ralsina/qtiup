/** \file
 * \brief Timer Control.
 *
 * See Copyright Notice in iup.h
 * $Id: itimer.h,v 1.2 2003/06/16 18:56:36 scuri Exp $
 */

#ifndef __ITIMER_H
#define __ITIMER_H

#ifdef __cplusplus
extern "C"
{
#endif

  void IupTimerOpen (void);
  void IupTimerClose (void);

/* declared in <iup.h>
Ihandle *IupTimer(void);
*/

#ifdef __cplusplus
}
#endif

#endif
