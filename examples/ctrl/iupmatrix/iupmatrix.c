#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "iup.h"
#include "iupcontrols.h"

Ihandle *create_mat(void)
{
  Ihandle *mat = IupMatrix(NULL); 
  
  IupSetAttribute(mat,IUP_NUMCOL,"2"); 
  IupSetAttribute(mat,IUP_NUMLIN,"3"); 
  
  IupSetAttribute(mat,IUP_NUMCOL_VISIBLE,"2") ;
  IupSetAttribute(mat,IUP_NUMLIN_VISIBLE,"3") ;
  
  IupSetAttribute(mat,"0:0","Inflation");
  IupSetAttribute(mat,"1:0","Medicine");
  IupSetAttribute(mat,"2:0","Food"); 
  IupSetAttribute(mat,"3:0","Energy"); 
  IupSetAttribute(mat,"0:1","January 2000"); 
  IupSetAttribute(mat,"0:2","February 2000"); 
  IupSetAttribute(mat,"1:1","5.6");
  IupSetAttribute(mat,"2:1","2.2");
  IupSetAttribute(mat,"3:1","7.2");
  IupSetAttribute(mat,"1:2","4.5");
  IupSetAttribute(mat,"2:2","8.1");
  IupSetAttribute(mat,"3:2","3.4");
  IupSetAttribute(mat,"WIDTHDEF","34");

  return mat;
}

/* Main program */
int main (void)
{
  Ihandle *dlg;
  IupOpen();       
  IupControlsOpen () ;

  dlg = IupDialog(create_mat()) ;
  IupShowXY (dlg,IUP_CENTER,IUP_CENTER) ;
  IupMainLoop () ;
  IupDestroy(dlg);
  IupControlsClose() ;
  IupClose () ;  
  return 0 ;
}
