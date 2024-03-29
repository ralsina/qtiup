/*IupColorBrowser Example in C 
Crates a IupColorBrowser element and updates, via callbacks, the values of texts representing the R, G and B colors selected. */

/*****************************************************************************
 * IupColorBrowser example                                                   *
 * Author: Mark Stroetzel Glasberg                                           *
 * e-mail: mark@tecgraf.puc-rio.br                                           *
 * November 22, 2000                                                         *
 *****************************************************************************/
 
/* ANSI C libraries include */
#include <stdlib.h>
#include <stdio.h>

/* IUP libraries include */
#include "iup.h"
#include "iupcontrols.h"

/* Global IUP handles */
Ihandle *text_red, *text_green, *text_blue ;
Ihandle *lbl_state ;

/******************************************************************************
 * Function:                                                                  *
 * ColorBrowser drag callback                                                 *
 *                                                                            *
 * Description:                                                               *
 * This callback is associated to an intermediate color change                *
 *                                                                            *
 * Parameters received:                                                       *
 * self - Element identifier                                                  *
 * r    - red color component                                                 * 
 * g    - green color component                                               *
 * b    - blue color component                                                *
 *                                                                            *
 * Value returned:                                                            *
 * IUP_DEFAULT                                                                *
 ******************************************************************************/
static int clrbrwsr_drag_cb(Ihandle *self, unsigned char r, unsigned char g, unsigned char b)
{
  char buf[10];

  sprintf(buf, "%d", (int) r);
  IupSetAttribute(text_red, IUP_VALUE, buf);
  sprintf(buf, "%d", (int) g);
  IupSetAttribute(text_green, IUP_VALUE, buf);
  sprintf(buf, "%d", (int) b);
  IupSetAttribute(text_blue, IUP_VALUE, buf);

  /* Gives IUP a time to call the OS and update the text field. */
  IupLoopStep();

  return IUP_DEFAULT;
}

/******************************************************************************
 * Function:                                                                  *
 * ColorBrowser drag callback                                                 *
 *                                                                            *
 * Description:                                                               *
 * This callback is associated to a definitive color change                   *
 *                                                                            *
 * Parameters received:                                                       *
 * self - Element identifier                                                  *
 * r    - red color component                                                 * 
 * g    - green color component                                               *
 * b    - blue color component                                                *
 *                                                                            *
 * Value returned:                                                            *
 * IUP_DEFAULT                                                                *
 ******************************************************************************/
static int clrbrwsr_change_cb(Ihandle *self, unsigned char r, unsigned char g, unsigned char b)
{
  clrbrwsr_drag_cb(self, r, g, b);
  
  return IUP_DEFAULT;
}

/* Main program */
int main (void)
{
  /* IUP handles */
  Ihandle *dlg ;
  Ihandle *hbox_final ;
  Ihandle *vbox ;
  Ihandle *clrbrwsr ;
  
  /* Initializes IUP */
  IupOpen () ;
  
  /* Initializes IUP controls */
  IupControlsOpen () ;
  
  /* Program begin */

  /* Creates three texts */
  text_red = IupText (NULL) ;
  text_green = IupText (NULL) ;
  text_blue = IupText (NULL) ;

  /* Creates a color browser */
  clrbrwsr = IupColorBrowser () ;
  
  /* Registers color browser callbacks */
  IupSetCallback(clrbrwsr, IUP_CHANGE_CB, (Icallback) clrbrwsr_change_cb) ;
  IupSetCallback(clrbrwsr, IUP_DRAG_CB, (Icallback) clrbrwsr_drag_cb) ;

  /* Creates a vbox */
  vbox = IupVbox
         (
           IupFill(),
           text_red,
           IupFill(),
           text_green,
           IupFill(),
           text_blue,
           IupFill(),
           NULL
         ) ;

  /* Creates an hbox */
  hbox_final = IupHbox (clrbrwsr, IupFill(), vbox, NULL) ;

  /* Creates a dialog */
  dlg = IupDialog (hbox_final) ;
  
  /* Sets title of the dialog */
  IupSetAttribute (dlg, IUP_TITLE, "IupColorBrowser") ;
  
  /* Shows dialog in the center of the screen */
  IupShowXY (dlg, IUP_CENTER, IUP_CENTER) ;
  
  /* Initializes IUP main loop */
  IupMainLoop () ;

  IupDestroy(dlg);

  /* Finishes IUP */
  IupControlsClose() ;
  IupClose () ;  

  /* Program finished sucessfully */
  return 0 ;
}