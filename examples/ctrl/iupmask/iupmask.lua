-- IupMask Example in Lua
-- Creates an IupText that accepts only numbers.

txt = iup.text{}
iup.maskSet(txt, "/d*", 0, 1) ;
dg = iup.dialog{txt}
dg:show()