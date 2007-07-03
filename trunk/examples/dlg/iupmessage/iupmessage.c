/* IupMessage Example */

/* IUP libraries include */
#include "iup.h"

/* Main program */
int main (void)
{
  /* Initializes IUP */
  IupOpen() ;
  
  /* Executes IupMessage */
  IupMessage("IupMessage Example", "Press the button") ;

  /* Finishes IUP */
  IupClose () ;

  /* Program finished sucessfully */
  return 0 ;
}