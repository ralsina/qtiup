/*IupVbox Example in C 
Creates a dialog with buttons placed one above the other, showing the organization possibilities of the elements inside a vbox. The IUP_ALIGNMENT attribute is explored in all its possibilities to obtain the effects. The attributes IUP_GAP, IUP_MARGIN and IUP_SIZE are also tested. */

/* IupVbox Example */	

/* ANSI C libraries include */
#include <stdio.h>

/* IUP libraries include */
#include <iup.h>

/* Main program */
int main (void)
{
  /* IUP identifiers */
  Ihandle *dlg ;
  Ihandle *frm_1, *frm_2, *frm_3 ;
  Ihandle *btn_11, *btn_12, *btn_13 ;
  Ihandle *btn_21, *btn_22, *btn_23 ;
  Ihandle *btn_31, *btn_32, *btn_33 ;
  Ihandle *vbox_1, *vbox_2, *vbox_3 ;

  /* Initializes IUP */
  IupOpen();

  /* Program begin */

  /* Creates frame 1 */
  frm_1 = IupFrame
  (
    IupHbox
    (
      IupFill(),
      vbox_1 = IupVbox
      (
        btn_11=IupButton ("1", ""),
        btn_12=IupButton ("2", ""),
        btn_13=IupButton ("3", ""),
        NULL
      ),
      IupFill(),
      NULL
    )
  ) ;

  /* Sets title of frame 1 */
  IupSetAttribute (frm_1, IUP_TITLE, "ALIGNMENT = ALEFT, GAP = 10") ;

  /* Sets attributes of buttons inside frame 1 */
  IupSetAttribute (btn_11, IUP_SIZE, "20x30") ;
  IupSetAttribute (btn_12, IUP_SIZE, "30x30") ;
  IupSetAttribute (btn_13, IUP_SIZE, "40x30") ;

  /* Sets attributes of vbox 1 */
  IupSetAttributes (vbox_1, "ALIGNMENT = ALEFT, GAP = 10") ;

  /* Creates frame 2 */
  frm_2 = IupFrame
  (
    IupHbox
    (
      IupFill(),
      vbox_2 = IupVbox
      (
        btn_21 = IupButton ("1", ""),
        btn_22 = IupButton ("2", ""),
        btn_23 = IupButton ("3", ""),
        NULL
      ),
      IupFill (),
      NULL
    )
  );

  /* Sets title of frame 1 */
  IupSetAttribute (frm_2, IUP_TITLE, "ALIGNMENT = ACENTER, MARGIN = 15") ;

  /* Sets attributes of buttons inside frame 2 */
  IupSetAttribute (btn_21, IUP_SIZE, "20x30") ;
  IupSetAttribute (btn_22, IUP_SIZE, "30x30") ;
  IupSetAttribute (btn_23, IUP_SIZE, "40x30") ;

  /* Sets attributes of vbox 2 */
  IupSetAttributes (vbox_2, "ALIGNMENT = ACENTER, MARGIN = 15") ;

  /* Creates frame 3 */
  frm_3 = IupFrame
  (
    IupHbox
    (
      IupFill (),
      vbox_3 = IupVbox
      (
        btn_31=IupButton ("1", ""),
        btn_32=IupButton ("2", ""),
        btn_33=IupButton ("3", ""),
        NULL
      ),
      IupFill (),
      NULL
    )
  ) ;

  /* Sets title of frame 3 */
  IupSetAttribute (frm_3, IUP_TITLE, "ALIGNMENT = ARIGHT, SIZE = 20") ;

  /* Sets attributes of buttons inside frame 3 */
  IupSetAttribute (btn_31, IUP_SIZE, "20x30") ;
  IupSetAttribute (btn_32, IUP_SIZE, "30x30") ;
  IupSetAttribute (btn_33, IUP_SIZE, "40x30") ;

  /* Sets attributes of vbox 3 */
  IupSetAttributes (vbox_3, "ALIGNMENT = ARIGHT, SIZE = 20") ;

  /* Creates dialog with three frames inside a vbo_ */
  dlg = IupDialog
  (
    IupVbox
    (
      frm_1,
      frm_2,
      frm_3,
      NULL
    )
  ) ;

  /* Sets title and size of the dialog */
  IupSetAttributes (dlg, "TITLE = \"IupVbox Example\", SIZE = QUARTER");

  /* Shows dialog in the center of the screen */
  IupShowXY (dlg, IUP_CENTER, IUP_CENTER) ;

  /* Initializes IUP main loop */
  IupMainLoop () ;

  IupDestroy(dlg);

  /* Finishes IUP */
  IupClose () ;  

  /* Program finished sucessfully */
  return 0 ;
}