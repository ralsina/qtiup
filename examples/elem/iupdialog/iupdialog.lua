--IupDialog Example in IupLua
--Creates a simple dialog.

vbox = iup.vbox { iup.label {title="Label"}, iup.button { title="Test" } }
dlg = iup.dialog{vbox; title="Dialog"}
dlg:show()
