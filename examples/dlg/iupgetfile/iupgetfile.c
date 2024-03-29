/*IupGetFile Example in C 
Shows a typical file-selection dialog. */

#include <stdio.h>
#include "iup.h"

int main(void)
{
  char file[256];
  IupOpen();
  IupSetLanguage(IUP_ENGLISH);

  strcpy(file, "*.txt");

  switch(IupGetFile(file))
  {
    case 1: 
      IupMessage("New file",file);	    
    break ;	    
    
    case 0 : 
      IupMessage("File already exists",file);
    break ;	    
    
    case -1 : 
      IupMessage("IupFileDlg","Operation canceled");
      return 1;
      
    case -2 : 
      IupMessage("IupFileDlg","Allocation error");
      return 1;
      
    case -3 : 
      IupMessage("IupFileDlg","Invalid parameter");
      return 1;
    break ;	    
  }

  IupClose();

  return 0;
}