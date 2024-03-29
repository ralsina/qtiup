/*IupListDialog Example in C 
Shows a color-selection dialog. */

#include <stdio.h>
#include <string.h>
#include "iup.h"

int main(void)
{
  int error; 	
  int size = 8 ;
  int marks[8] = { 0,0,0,0,1,1,0,0 };
  char *options[] = {
    "Blue"   ,
    "Red"    ,
    "Green"  ,
    "Yellow" ,
    "Black"  ,
    "White"  ,
    "Gray"   ,
    "Brown"  } ;
	  
  IupOpen();
  IupSetLanguage(IUP_ENGLISH);
  
  error = IupListDialog(2,"Color Selection",size,options,0,16,5,marks);

  if(error == -1)
  {
    IupMessage("IupListDialog","Operation canceled");
  }
  else
  {
    int i;
    char selection[80] = "";
	  
    for(i = 0 ; i < size ; i++)
    {
      if(marks[i])
      {
        char temp[10];	      
        sprintf(temp,"%s\n",options[i]);      
        strcat(selection,temp);
      }
    }
    
    if(strcmp(selection,""))
      IupMessage("Options selected",selection);	  
    else
      IupMessage("IupListDialog","No option selected");
  }
  
  IupClose();

  return 0;
}