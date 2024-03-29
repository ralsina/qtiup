/*IupFill Example in C 
Uses the Fill element to horizontally centralize a button and to justify it to the left and right.*/

/* IupFill example */
#include <stdio.h>
#include "iup.h"

int main( void )
{
  /* IUP handles */
  Ihandle *frame_left ;
  Ihandle *frame_right ;
  Ihandle *frame_center ;
  Ihandle *dialog ; 

  /* Initializes IUP */
  IupOpen() ;
  
  /* Creates frame with left aligned button */
  frame_left = IupFrame
  (
    IupHbox
    (
      IupButton( "Ok" , "" ),
      IupFill(),
      NULL
    )
  ) ;

  /* Sets frame's title */
  IupSetAttribute( frame_left, IUP_TITLE, "Left aligned" ) ;

  /* Creates frame with centered button */
  frame_center = IupFrame
  (
    IupHbox
    (
      IupFill (),
      IupButton ( "Ok", "" ),
      IupFill (),
      NULL
    )
  ) ;

  /* Sets frame's title */
  IupSetAttribute( frame_center, IUP_TITLE, "Centered" ) ;

  /* Creates frame with right aligned button */
  frame_right = IupFrame
  (
    IupHbox
    (
      IupFill (),
      IupButton ( "Ok", "" ),
      NULL
    )
  ) ;

  /* Sets frame's title */
  IupSetAttribute( frame_right, IUP_TITLE, "Right aligned" ) ;

  /* Creates dialog with these three frames */
  dialog = IupDialog
  (
    IupVbox
    (
      frame_left,
      frame_center,
      frame_right,
      NULL
    )
  ) ;

  /* Sets dialog's size and title */
  IupSetAttributes( dialog, "SIZE=120, TITLE=IupFill") ;

  IupShow( dialog ) ;  /* Shows dialog in the center of the screen */
  IupMainLoop() ;      /* Initializes IUP main loop */
  IupDestroy(dialog);
  IupClose() ;         /* Finishes IUP */

  return 0;
}