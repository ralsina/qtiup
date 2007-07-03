/** \file
 * \brief initializes iupdial, iupgauge, iuptabs, iupcb, iupgc and iupval controls.
 *
 * See Copyright Notice in iup.h
 * $Id: iupcontrols.c,v 1.17 2006/01/19 22:03:21 scuri Exp $
 */


#include <iup.h>
#include <iupcontrols.h>
#include "icontrols.h"

#include <assert.h>

static int opened = 0;

int IupControlsOpen(void)
{
  if (opened)
    return IUP_OPENED;
  opened = 1;

  IupValOpen();
  IupDialOpen();
  IupGaugeOpen();
  IupTabsOpen();
  IupColorBrowserOpen();
  IupGetColorOpen(); /* this must be after mask and colorbrowser open */
  IupMatrixOpen();
  IupTreeOpen();
  IupColorbarOpen();
  IupSboxOpen();
  IupSpinOpen();
  IupCboxOpen();
  IupCellsOpen();

  return IUP_NOERROR;
}

void IupControlsClose(void)
{
  if (!opened)
    return;
  opened = 0;

  IupMatrixClose();
  IupTreeClose();
  IupSpinClose();
}
