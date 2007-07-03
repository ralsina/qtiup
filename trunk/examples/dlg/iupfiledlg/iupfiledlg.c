/*IupFileDlg Example in C 
Shows a typical file-saving dialog. */
#include <stdio.h>
#include <iup.h>

int main (void)
{
  Ihandle *filedlg; 
  IupOpen();
  IupSetLanguage(IUP_ENGLISH);
  
  filedlg = IupFileDlg(); 
 
  IupSetAttributes(filedlg, "DIALOGTYPE = SAVE, TITLE = \"File Save\"");
  IupSetAttributes(filedlg, "FILTER = \"*.bmp\", FILTERINFO = \"Bitmap Files\"");

  IupPopup(filedlg, IUP_ANYWHERE, IUP_ANYWHERE); 

  switch(IupGetInt(filedlg, IUP_STATUS))
  {
    case 1: 
      IupMessage("New file",IupGetAttribute(filedlg, IUP_VALUE)); 
      break; 

    case 0 : 
      IupMessage("File already exists",IupGetAttribute(filedlg, IUP_VALUE));
      break; 

    case -1 : 
      IupMessage("IupFileDlg","Operation Canceled");
      return 1;
      break; 
  }

  IupDestroy(filedlg);
  IupClose(); 
  return 0;
}