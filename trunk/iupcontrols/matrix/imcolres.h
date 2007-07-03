/** \file
 * \brief iupmatrix column resize.
 *
 * See Copyright Notice in iup.h
 * $Id: imcolres.h,v 1.3 2003/06/16 18:56:37 scuri Exp $
 */
 
#ifndef __IMCOLRES_H 
#define __IMCOLRES_H

#ifdef __cplusplus
extern "C" {
#endif

int  iupmatColresTry         (Ihandle *h, int x, int y);
void iupmatColresFinish      (Ihandle *h, int x);
void iupmatColresMove        (Ihandle *h, int x);
void iupmatColresChangeCursor(Ihandle *h, int x, int y);
int  iupmatColresResizing    (void);

void  iupmatColresSet (Ihandle *h, int col, int m);
char *iupmatColresGet (Ihandle *h, int col, int m);

#ifdef __cplusplus
}
#endif

#endif
