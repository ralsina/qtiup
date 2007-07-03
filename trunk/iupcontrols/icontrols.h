/** \file
 * \brief controls initialization.
 *
 * See Copyright Notice in iup.h
 * $Id: icontrols.h,v 1.2 2005/09/22 01:19:31 scuri Exp $
 */
 
#ifndef __ICONTROLS_H 
#define __ICONTROLS_H

#ifdef __cplusplus
extern "C" {
#endif

void IupMatrixOpen(void);
void IupMatrixClose(void);

void IupTreeOpen( void );
void IupTreeClose(void);

void IupSpinOpen(void);
void IupSpinClose(void);

void IupCboxOpen(void);
void IupValOpen(void);
void IupColorBrowserOpen(void);
void IupColorbarOpen(void);
void IupDialOpen(void);
void IupGaugeOpen(void);
void IupGetColorOpen(void);
void IupSboxOpen(void);
void IupTabsOpen(void);
void IupCellsOpen(void);

#ifdef __cplusplus
}
#endif

#endif
