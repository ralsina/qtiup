/** \file
 * \brief IUP Core pre-defined dialogs.
 *
 * See Copyright Notice in iup.h
 * $Id: ipredial.h,v 1.5 2006/01/19 22:03:20 scuri Exp $
 */

#ifndef __IPREDIAL_H
#define __IPREDIAL_H

#ifdef __cplusplus
extern "C"
{
#endif

/* Used by the IupScanf implementation */
  int iupDataEntry (int maxlin, int maxcol[], int maxscr[], char *title,
                    char *text[], char *data[]);

/* Other functions declared in <iup.h> and implemented here. 
int  IupListDialog(int tipo, char *titulo, int tam, char *lista[], int opcao, int max_col, int max_lin, int marcas[]);
int  IupAlarm(char *title, char *m, char *b1, char *b2, char *b3);
void IupMessagef(char *title, char *f, ...)
int IupGetFile( char* file )
int IupGetText(char* title, char* text)
*/

#ifdef __cplusplus
}
#endif

#endif
