#include <stdio.h>
#include "iup.h"

int main(void)
{
  char *error=NULL;
  Ihandle *dlg = NULL; 
  IupOpen();
  
  /* Loads LED file */
  if(error = IupLoad("iupfiledlg.led"))
  {
    IupMessage("LED error", error);
    return 1 ;
  }

  dlg = IupGetHandle("dlg");  
  IupPopup(dlg, IUP_ANYWHERE, IUP_ANYWHERE); 

  switch(IupGetInt(dlg, IUP_STATUS))
  {
    case 1: 
      IupMessage("New file",IupGetAttribute(dlg, IUP_VALUE));	    
    break ;	    
    
    case 0 : 
      IupMessage("File already exists.",IupGetAttribute(dlg, IUP_VALUE));
    break ;	    
    
    case -1 : 
      IupMessage("IupFileDlg","Operation Canceled");
      return 1;
    break ;	    
  }

  IupDestroy(dlg);
  IupClose();

  return 0;
}