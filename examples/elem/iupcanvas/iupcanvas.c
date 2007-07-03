/*
 * IupCanvas example
 * Description : Creates a IUP canvas and uses CD to draw on it
 *      Remark : IUP must be linked to the CD library                      
 */

#include <stdio.h>
#include <stdlib.h>

#include "iup.h"
#include "cd.h"
#include "cdiup.h"

static cdCanvas *cdcanvas = NULL ;

int repaint_cb( Ihandle *self, float x, float y )
{
  cdCanvas *oldCanvas = cdActiveCanvas() ;
  cdActivate( cdcanvas ) ;
  cdForeground( CD_BLUE ) ;
  cdBox(0, 100, 0, 100) ;
  cdActivate(oldCanvas) ;
  return IUP_DEFAULT ;
}

int main(void)
{
  Ihandle *dlg = NULL ;
  Ihandle *cnvs = NULL ;
  IupOpen() ;

  cnvs = IupCanvas( NULL ) ;
  IupSetCallback(cnvs, "ACTION",( Icallback )repaint_cb ) ;
  IupSetAttributes( cnvs, "SIZE=300x100, XMIN=0, XMAX=99, POSX=0, DX=10" ) ;
  
  dlg = IupDialog( IupFrame( cnvs ) ) ;
  IupSetAttribute( dlg, IUP_TITLE, "IupCanvas & Canvas Draw" ) ;
  IupMap( dlg ) ;
  
  cdcanvas = cdCreateCanvas( CD_IUP, cnvs ) ;
  
  IupShowXY( dlg, IUP_CENTER, IUP_CENTER ) ;
  IupMainLoop() ;
  IupDestroy(dlg);
  IupClose() ;  
  return 0 ;
}