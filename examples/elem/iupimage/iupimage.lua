-- IupImage Example in IupLua 
-- Creates a button, a label, a toggle and a radio using an image. 
-- Uses an image for the cursor as well.

-- Defines an "X" image 
img_x = iup.image{
  { 1,2,3,3,3,3,3,3,3,2,1 }, 
  { 2,1,2,3,3,3,3,3,2,1,2 }, 
  { 3,2,1,2,3,3,3,2,1,2,3 }, 
  { 3,3,2,1,2,3,2,1,2,3,3 }, 
  { 3,3,3,2,1,2,1,2,3,3,3 }, 
  { 3,3,3,3,2,1,2,3,3,3,3 },  
  { 3,3,3,2,1,2,1,2,3,3,3 }, 
  { 3,3,2,1,2,3,2,1,2,3,3 }, 
  { 3,2,1,2,3,3,3,2,1,2,3 }, 
  { 2,1,2,3,3,3,3,3,2,1,2 }, 
  { 1,2,3,3,3,3,3,3,3,2,1 }
  -- Sets "X" image colors
  ; colors = { "0 1 0", "255 0 0", "255 255 0" }
}

-- Defines a cursor image
img_cursor = iup.image{
  { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
  { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
  { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
  { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
  { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
  { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
  { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
  { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
  { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
  { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,2,2,2,2,2,2,0,0,0,0,0,0,0,0,0,0,0 },
  { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,1,1,1,1,2,0,0,0,0,0,0,0,0,0,0,0 },
  { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,1,1,1,1,2,0,0,0,0,0,0,0,0,0,0,0 },
  { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,1,1,1,1,1,2,0,0,0,0,0,0,0,0,0,0,0 },
  { 0,0,0,0,0,0,0,0,0,0,0,0,0,2,1,1,1,1,1,1,2,0,0,0,0,0,0,0,0,0,0,0 },
  { 0,0,0,0,0,0,0,0,0,0,0,0,2,1,1,1,1,1,2,2,2,0,0,0,0,0,0,0,0,0,0,0 },
  { 0,0,0,0,0,0,0,0,0,0,0,2,1,1,1,1,1,2,0,0,2,0,0,0,0,0,0,0,0,0,0,0 },
  { 0,0,0,0,0,0,0,0,0,0,0,0,2,1,1,1,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
  { 0,0,0,0,0,0,0,0,0,0,0,0,0,2,2,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
  { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
  { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
  { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
  { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
  { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
  { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
  { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
  { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
  { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
  { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
  { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
  { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
  { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
  { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 }
  -- Sets cursor image colors
  ; colors = { "255 0 0", "128 0 0" }, hotspot = "21:10" 
}

-- Creates a button entitled "Dummy" and associates image img_x to it
btn = iup.button{ title = "", image = img_x }

-- Creates a label entitled "Dummy" and associates image img_x to it
lbl = iup.label{ title = "", image = img_x }

-- Creates toggle entitled "Dummy" and associates image img_x to it
tgl = iup.toggle{ title = "", image = img_x }

-- Creates two toggles entitled "Dummy" and associates image img_x to them
tgl_radio_1 = iup.toggle{ title = "", image = img_x }
tgl_radio_2 = iup.toggle{ title = "", image = img_x }

-- Creates label showing image size
lbl_size = iup.label{ title = '"X" image width = '..img_x.width..'; "X" image height = '..img_x.height } 
  
-- Creates frames around the elements 
frm_btn = iup.frame{btn; title="button", size="EIGHTHxEIGHTH"}
frm_lbl = iup.frame{lbl; title="label" , size="EIGHTHxEIGHTH"}
frm_tgl = iup.frame{tgl; title="toggle", size="EIGHTHxEIGHTH"}

frm_tgl_radio = iup.frame{ 
                            iup.radio{ 
                               iup.vbox
                               { 
                                   tgl_radio_1, 
                                   tgl_radio_2 
                               } 
                            }; 
                            title="radio", size="EIGHTHxEIGHTH"
                          }

-- Creates dialog dlg with an hbox containing a button, a label, and a toggle 
dlg = iup.dialog
      {
        iup.vbox
        {
          iup.hbox{frm_btn, frm_lbl, frm_tgl, frm_tgl_radio},
          iup.fill{},
          iup.hbox{iup.fill{}, lbl_size, iup.fill{}} 
        }; title = "IupImage Example", size = "HALFxQUARTER", 
        cursor = img_cursor
      } 

-- Shows dialog in the center of the screen 
dlg:showxy(iup.CENTER, iup.CENTER)