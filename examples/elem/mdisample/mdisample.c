#if 1
/* To check for memory leaks */
#define VLD_MAX_DATA_DUMP 80
#include <vld.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <iup.h>
#include <iupcontrols.h>

static char img_bits1[] = 
{
 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,1
,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,1,1
,1,1,1,1,1,1,1,1,1,1,1,1,1,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,1,1,1
,1,1,1,1,1,1,1,1,1,1,1,1,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,1,1,1,1
,1,1,1,1,1,1,1,1,1,1,1,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,1,1,1,1,1
,1,1,1,1,1,1,1,1,1,1,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,1,1,1,1,1,1
,1,1,1,1,1,1,1,1,1,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,1,1,1,1,1,1,1
,1,1,1,1,1,1,1,1,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,1,1,1,1,1,1,1,1
,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2
,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2
,2,2,2,0,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2
,2,2,2,0,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2
,2,2,2,0,2,0,2,0,2,2,0,2,2,2,0,0,0,2,2,2,0,0,2,0,2,2,0,0,0,2,2,2
,2,2,2,0,2,0,0,2,0,0,2,0,2,0,2,2,2,0,2,0,2,2,0,0,2,0,2,2,2,0,2,2
,2,2,2,0,2,0,2,2,0,2,2,0,2,2,2,2,2,0,2,0,2,2,2,0,2,0,2,2,2,0,2,2
,2,2,2,0,2,0,2,2,0,2,2,0,2,2,0,0,0,0,2,0,2,2,2,0,2,0,0,0,0,0,2,2
,2,2,2,0,2,0,2,2,0,2,2,0,2,0,2,2,2,0,2,0,2,2,2,0,2,0,2,2,2,2,2,2
,2,2,2,0,2,0,2,2,0,2,2,0,2,0,2,2,2,0,2,0,2,2,0,0,2,0,2,2,2,0,2,2
,2,2,2,0,2,0,2,2,0,2,2,0,2,2,0,0,0,0,2,2,0,0,2,0,2,2,0,0,0,2,2,2
,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,0,2,2,2,2,2,2,2,2
,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,0,2,2,2,0,2,2,2,2,2,2,2,2
,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,0,0,0,2,2,2,2,2,2,2,2,2
,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2
,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2
,1,1,1,1,1,1,1,2,2,2,2,2,2,2,2,2,2,2,2,2,2,1,1,1,1,1,1,1,1,1,1,1
,1,1,1,1,1,1,2,2,2,2,2,2,2,2,2,2,2,2,2,2,1,1,1,1,1,1,1,1,1,1,1,1
,1,1,1,1,1,2,2,2,2,2,2,2,2,2,2,2,2,2,2,1,1,1,1,1,1,1,1,1,1,1,1,1
,1,1,1,1,2,2,2,2,2,2,2,2,2,2,2,2,2,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1
,1,1,1,2,2,2,2,2,2,2,2,2,2,2,2,2,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1
,1,1,2,2,2,2,2,2,2,2,2,2,2,2,2,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1
,1,2,2,2,2,2,2,2,2,2,2,2,2,2,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1
,2,2,2,2,2,2,2,2,2,2,2,2,2,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1
};

static char img_bits2[] = 
{
 2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,2
,2,2,2,2,2,2,2,2,2,2,2,2,2,2,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,2,2
,2,2,2,2,2,2,2,2,2,2,2,2,2,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,2,2,2
,2,2,2,2,2,2,2,2,2,2,2,2,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,2,2,2,2
,2,2,2,2,2,2,2,2,2,2,2,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,2,2,2,2,2
,2,2,2,2,2,2,2,2,2,2,3,3,3,3,1,1,3,3,3,3,3,3,3,3,3,3,2,2,2,2,2,2
,2,2,2,2,2,2,2,2,2,3,3,3,3,3,1,1,3,3,3,3,3,3,3,3,3,2,2,2,2,2,2,2
,2,2,2,2,2,2,2,2,3,3,3,3,3,3,1,1,3,3,3,3,3,3,3,3,2,2,2,2,2,2,2,2
,3,3,3,3,3,3,3,3,3,3,3,3,3,3,1,1,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3
,3,3,3,3,3,3,3,3,3,3,3,3,3,3,1,1,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3
,3,3,3,0,3,3,3,3,3,3,3,3,3,3,1,1,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3
,3,3,3,0,3,3,3,3,3,3,3,3,3,3,1,1,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3
,3,3,3,0,3,0,3,0,3,3,0,3,3,3,1,1,0,3,3,3,0,0,3,0,3,3,0,0,0,3,3,3
,3,3,3,0,3,0,0,3,0,0,3,0,3,0,1,1,3,0,3,0,3,3,0,0,3,0,3,3,3,0,3,3
,3,3,3,0,3,0,3,3,0,3,3,0,3,3,1,1,3,0,3,0,3,3,3,0,3,0,3,3,3,0,3,3
,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1
,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1
,3,3,3,0,3,0,3,3,0,3,3,0,3,0,1,1,3,0,3,0,3,3,0,0,3,0,3,3,3,0,3,3
,3,3,3,0,3,0,3,3,0,3,3,0,3,3,1,1,0,0,3,3,0,0,3,0,3,3,0,0,0,3,3,3
,3,3,3,3,3,3,3,3,3,3,3,3,3,3,1,1,3,3,3,3,3,3,3,0,3,3,3,3,3,3,3,3
,3,3,3,3,3,3,3,3,3,3,3,3,3,3,1,1,3,3,3,0,3,3,3,0,3,3,3,3,3,3,3,3
,3,3,3,3,3,3,3,3,3,3,3,3,3,3,1,1,3,3,3,3,0,0,0,3,3,3,3,3,3,3,3,3
,3,3,3,3,3,3,3,3,3,3,3,3,3,3,1,1,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3
,3,3,3,3,3,3,3,3,3,3,3,3,3,3,1,1,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3
,2,2,2,2,2,2,2,3,3,3,3,3,3,3,1,1,3,3,3,3,3,2,2,2,2,2,2,2,2,2,2,2
,2,2,2,2,2,2,3,3,3,3,3,3,3,3,1,1,3,3,3,3,2,2,2,2,2,2,2,2,2,2,2,2
,2,2,2,2,2,3,3,3,3,3,3,3,3,3,3,3,3,3,3,2,2,2,2,2,2,2,2,2,2,2,2,2
,2,2,2,2,3,3,3,3,3,3,3,3,3,3,3,3,3,3,2,2,2,2,2,2,2,2,2,2,2,2,2,2
,2,2,2,3,3,3,3,3,3,3,3,3,3,3,3,3,3,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2
,2,2,3,3,3,3,3,3,3,3,3,3,3,3,3,3,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2
,2,3,3,3,3,3,3,3,3,3,3,3,3,3,3,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2
,3,3,3,3,3,3,3,3,3,3,3,3,3,3,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2
};

static int line = 0;
static int getfocus_cb(Ihandle *self)
{
  printf("%2d-getfocus(%s#%s)\n", line, IupGetType(self), 
                                        IupGetAttribute(self, "CINDEX"));
  line++;
  return IUP_DEFAULT;
}

static int killfocus_cb(Ihandle *self)
{
  printf("%2d-killfocus(%s#%s)\n", line, IupGetType(self), 
                                         IupGetAttribute(self, "CINDEX"));
  line++;
  return IUP_DEFAULT;
}

static int action(Ihandle* self)
{
  printf("%2d-action(%s#%s) Value=%s\n", line, IupGetType(self), 
                                               IupGetAttribute(self, "CINDEX"), 
                                               IupGetAttribute(self, "VALUE"));
  line++;
  return IUP_DEFAULT;
}

static void createImages (void)
{
  Ihandle *img;

  img = IupImage(32,32, img_bits1);
  IupSetHandle ("img1", img); 
  IupSetAttribute (img, "0", "0 0 0"); 
  IupSetAttribute (img, "1", "BGCOLOR");
  IupSetAttribute (img, "2", "255 0 0");

  img = IupImage(32,32, img_bits2);
  IupSetHandle ("img2", img); 
  IupSetAttribute (img, "0", "0 0 0"); 
  IupSetAttribute (img, "1", "0 255 0");
  IupSetAttribute (img, "2", "BGCOLOR");
  IupSetAttribute (img, "3", "255 0 0");
}

static void set_callbacks(Ihandle* ctrl)  
{               
  Ihandle* child;
  
  IupSetCallback(ctrl, "GETFOCUS_CB", (Icallback)getfocus_cb);
  IupSetCallback(ctrl, "KILLFOCUS_CB", (Icallback)killfocus_cb);
  IupSetCallback(ctrl, "ACTION", (Icallback)action);
  
  child = IupGetNextChild(ctrl, NULL);
  while (child)
  { 
    set_callbacks(child);  
    child = IupGetNextChild(ctrl, child);
  }
}

Ihandle* createDialog (void)
{
  Ihandle *_hbox_1, *_cnv_1, *_vbox_1, *dlg, 
    *_frm_1, *_frm_2, *_frm_3, *_frm_4, *_frm_5,
    *_list_1, *_list_2, *_list_3, *_list_4, *_text_1, *_ml_1;

  _frm_1 = IupFrame(
    IupVbox(
      IupSetAttributes(IupButton("Button Text", NULL), "CINDEX=1"),
      IupSetAttributes(IupButton("", NULL), "CINDEX=2, BGCOLOR=\"255 128 0\", RASTERSIZE=30x30"),
      IupSetAttributes(IupButton("", NULL), "IMAGE=img1,CINDEX=3"),
      IupSetAttributes(IupButton("", NULL), "IMAGE=img1,CINDEX=4, FLAT=YES"),
      IupSetAttributes(IupButton("", NULL), "IMAGE=img1,IMPRESS=img2,CINDEX=5"),
      NULL));
  IupSetAttribute(_frm_1,IUP_TITLE,"IupButton");

  _frm_2 = IupFrame(
    IupVbox(
      IupSetAttributes(IupLabel("Label Text\nLine 2\nLine 3"), "CINDEX=1"),
      IupSetAttributes(IupLabel(""), "SEPARATOR=HORIZONTAL,CINDEX=2"),
      IupSetAttributes(IupLabel(""), "IMAGE=img1,CINDEX=3"),
      NULL));
  IupSetAttribute(_frm_2,IUP_TITLE,"IupLabel");

  _frm_3 = IupFrame(
    IupVbox(
      IupSetAttributes(IupToggle("Toggle Text", NULL), "VALUE=ON,CINDEX=1"),
      IupSetAttributes(IupToggle("3State Text", NULL), "VALUE=NOTDEF,CINDEX=2,3STATE=YES"),
      IupSetAttributes(IupToggle("", NULL), "IMAGE=img1,IMPRESS=img2,CINDEX=3"),
      IupSetAttributes(IupFrame(IupRadio(IupVbox(
        IupSetAttributes(IupToggle("Toggle Text", NULL), "CINDEX=4"),
        IupSetAttributes(IupToggle("Toggle Text", NULL), "CINDEX=5"),
        NULL))), "TITLE=IupRadio"),
      NULL));
  IupSetAttribute(_frm_3,IUP_TITLE,"IupToggle");

  _text_1 = IupText( NULL);
  IupSetAttribute(_text_1,IUP_VALUE,"IupText Text");
//  IupSetAttribute(_text_1,IUP_SIZE,"80x");
  IupSetAttribute(_text_1,"CINDEX","1");

  _ml_1 = IupMultiLine( NULL);
  IupSetAttribute(_ml_1,IUP_VALUE,"IupMultiline Text\nSecond Line\nThird Line");
//  IupSetAttribute(_ml_1,IUP_SIZE,"50x30");
//  IupSetAttribute(_ml_1,IUP_EXPAND,"YES");
//  IupSetAttribute(_ml_1,IUP_SIZE,"80x60");
  IupSetAttribute(_ml_1,"CINDEX","1");

  _frm_4 = IupFrame(IupVbox(
    _text_1,
    _ml_1,
    NULL));
  IupSetAttribute(_frm_4,IUP_TITLE,"IupText/IupMultiline");

  _list_1 = IupList( NULL);
  IupSetAttribute(_list_1,IUP_EXPAND,"YES");
//  IupSetAttribute(_list_1, IUP_SIZE, "50x40");
  IupSetAttribute(_list_1,IUP_VALUE,"1");
  IupSetAttribute(_list_1,"MULTIPLE","YES");  
  IupSetAttribute(_list_1,"1","Item 1 Text");
  IupSetAttribute(_list_1,"2","Item 2 Text");
  IupSetAttribute(_list_1,"3","Item 3 Text Big Item");
  IupSetAttribute(_list_1,"4","Item 4 Text");
  IupSetAttribute(_list_1,"5","Item 5 Text");
  IupSetAttribute(_list_1,"6","Item 6 Text");
  IupSetAttribute(_list_1,"CINDEX","1");        

  _list_2 = IupList( NULL);
  IupSetAttribute(_list_2,IUP_DROPDOWN,"YES");
  IupSetAttribute(_list_2,IUP_EXPAND,"YES");
  IupSetAttribute(_list_2, "VISIBLE_ITEMS", "3");
//  IupSetAttribute(_list_2, IUP_SIZE, "50x");
  IupSetAttribute(_list_2,IUP_VALUE,"2");
  IupSetAttribute(_list_2,"1","Item 1 Text");
  IupSetAttribute(_list_2,"2","Item 2 Text Big Item");
  IupSetAttribute(_list_2,"3","Item 3 Text");
  IupSetAttribute(_list_2,"4","Item 4 Text");
  IupSetAttribute(_list_2,"5","Item 5 Text");
  IupSetAttribute(_list_2,"6","Item 6 Text");
  IupSetAttribute(_list_2,"CINDEX","2");

  _list_3 = IupList( NULL);
  IupSetAttribute(_list_3,"EDITBOX","YES");
  IupSetAttribute(_list_3,IUP_EXPAND,"YES");
//  IupSetAttribute(_list_3, IUP_SIZE, "50x40");
  IupSetAttribute(_list_3,IUP_VALUE,"Test Value");
  IupSetAttribute(_list_3,"1","Item 1 Text");
  IupSetAttribute(_list_3,"2","Item 2 Text Big Item");
  IupSetAttribute(_list_3,"3","Item 3 Text");
  IupSetAttribute(_list_3,"4","Item 4 Text");
  IupSetAttribute(_list_3,"5","Item 5 Text");
  IupSetAttribute(_list_3,"6","Item 6 Text");
  IupSetAttribute(_list_3,"CINDEX","3");

  _list_4 = IupList( NULL);
  IupSetAttribute(_list_4,"EDITBOX","YES");
  IupSetAttribute(_list_4,IUP_DROPDOWN,"YES");
  IupSetAttribute(_list_4,IUP_EXPAND,"YES");
  IupSetAttribute(_list_4, "VISIBLE_ITEMS", "3");
//  IupSetAttribute(_list_4, IUP_SIZE, "50x10");
  IupSetAttribute(_list_4,IUP_VALUE,"Test Value");
  IupSetAttribute(_list_4,"1","Item 1 Text");
  IupSetAttribute(_list_4,"2","Item 2 Text Big Item");
  IupSetAttribute(_list_4,"3","Item 3 Text");
  IupSetAttribute(_list_4,"4","Item 4 Text");
  IupSetAttribute(_list_4,"5","Item 5 Text");
  IupSetAttribute(_list_4,"6","Item 6 Text");
  IupSetAttribute(_list_4,"CINDEX","4");

  _frm_5 =  IupFrame(IupVbox(
      _list_1,
      _list_2,
      _list_3,
      _list_4,
      NULL));
  IupSetAttribute(_frm_5,IUP_TITLE,"IupList");

  _hbox_1 = IupHbox(
    _frm_1,
    _frm_2,
    _frm_3,
    _frm_4,
    _frm_5,
    NULL);

  _cnv_1 = IupCanvas( "do_nothing");
  IupSetAttribute(_cnv_1,IUP_BGCOLOR,"128 255 0");

  _vbox_1 = IupVbox(
//    IupTabs(_hbox_1, NULL),
    _hbox_1,
    _cnv_1,
    NULL);
  IupSetAttribute(_vbox_1,IUP_MARGIN,"5x5");
  IupSetAttribute(_vbox_1,IUP_ALIGNMENT,"ARIGHT");
  IupSetAttribute(_vbox_1,IUP_GAP,"5");            

  set_callbacks(_vbox_1);  

  dlg = IupDialog(_vbox_1);
  IupSetAttribute(dlg,IUP_TITLE,"IupDialog Title");
//  IupSetAttribute(dlg,"SHRINK","YES");
//  IupSetAttribute(dlg, IUP_SIZE, "500x200");

//  IupSetAttribute(dlg,"BGCOLOR","255 0 255");
//  IupSetAttribute(dlg,"FONT","Times New Roman:ITALIC:10");
//  IupSetAttribute(dlg, IUP_FONT, IUP_TIMES_BOLD_14);
//  IupSetAttribute(_vbox_1,"ACTIVE","NO");

  //IupSetAttribute(dlg,"COMPOSITED", "YES");
  //IupSetAttribute(dlg,"LAYERED", "YES");
  //IupSetAttribute(dlg,"LAYERALPHA", "192");

//  IupMap(dlg);
//  IupSetAttribute(dlg, IUP_SIZE, NULL);

  return dlg;
}

static int mdi_tilehoriz(Ihandle* self)
{
  IupSetAttribute(self, "MDIARRANGE", "TILEHORIZONTAL");
  return IUP_DEFAULT;
}

static int mdi_tilevert(Ihandle* self)
{
  IupSetAttribute(self, "MDIARRANGE", "TILEVERTICAL");
  return IUP_DEFAULT;
}

static int mdi_cascade(Ihandle* self)
{
  IupSetAttribute(self, "MDIARRANGE", "CASCADE");
  return IUP_DEFAULT;
}

static int mdi_icon(Ihandle* self)
{
  IupSetAttribute(self, "MDIARRANGE", "ICON");
  return IUP_DEFAULT;
}

static int mdi_next(Ihandle* self)
{
  IupSetAttribute(self, "MDIACTIVATE", "NEXT");
  return IUP_DEFAULT;
}

static int mdi_previous(Ihandle* self)
{
  IupSetAttribute(self, "MDIACTIVATE", "PREVIOUS");
  return IUP_DEFAULT;
}

static int mdi_closeall(Ihandle* self)
{
  IupSetAttribute(self, "MDICLOSEALL", NULL);
  return IUP_DEFAULT;
}

static int mdi_activate(Ihandle* self)
{
  printf("%2d-mdi_activate(%s)\n", line, IupGetName(self));
  line++;
  return IUP_DEFAULT;
}

static int mdi_new(Ihandle* self)
{
  Ihandle* dlg = createDialog();
  IupSetAttribute(dlg,"MDICHILD","YES");
  IupSetAttribute(dlg,"MDICLIENT","mdiClient");
  IupSetCallback(dlg,"MDIACTIVATE_CB",(Icallback)mdi_activate);
//  IupSetAttribute(dlg, "PLACEMENT", "MAXIMIZED");

  IupShow(dlg);

  return IUP_DEFAULT;
}

static void createMenu(void)
{
  Ihandle* winmenu;
  Ihandle* mnu = IupMenu(
  IupSubmenu("MDI",IupMenu(
      IupItem("New", "mdi_new"), 
      NULL)),
  winmenu = IupSubmenu("Window", IupMenu(
      IupItem("Tile Horizontal", "mdi_tilehoriz"), 
      IupItem("Tile Vertical", "mdi_tilevert"), 
      IupItem("Cascade", "mdi_cascade"), 
      IupItem("Icon Arrange", "mdi_icon"), 
      IupItem("Close All", "mdi_closeall"), 
      IupSeparator(),
      IupItem("Next", "mdi_next"), 
      IupItem("Previous", "mdi_previous"), 
      NULL)),
    NULL);
  IupSetHandle("mnu",mnu);
  IupSetHandle("mdiMenu",winmenu);

  IupSetFunction("mdi_new", (Icallback)mdi_new);
  IupSetFunction("mdi_tilehoriz", (Icallback)mdi_tilehoriz);
  IupSetFunction("mdi_tilevert", (Icallback)mdi_tilevert);
  IupSetFunction("mdi_cascade", (Icallback)mdi_cascade);
  IupSetFunction("mdi_icon", (Icallback)mdi_icon);
  IupSetFunction("mdi_next", (Icallback)mdi_next);
  IupSetFunction("mdi_previous", (Icallback)mdi_previous);
  IupSetFunction("mdi_closeall", (Icallback)mdi_closeall);
}

static Ihandle* createFrame(void)
{
  Ihandle *dlg, *cnv;
  cnv = IupCanvas( "do_nothing");
  IupSetAttribute(cnv,"MDICLIENT","YES");
  IupSetHandle("mdiClient", cnv);

  dlg = IupDialog(cnv);
  IupSetAttribute(dlg,IUP_TITLE,"MDI Frame");
  IupSetAttribute(dlg,"MDIMENU","mdiMenu");
  IupSetAttribute(dlg,"RASTERSIZE","800x600");

  return dlg;
}

int main(void)
{
  Ihandle* dlg;
  IupOpen();      
//  IupControlsOpen();      
  createImages();
  createMenu();

  dlg = createFrame(); //createDialog();
//  IupSetAttribute(dlg, "PLACEMENT", "MAXIMIZED");
  IupSetAttribute(dlg,IUP_MENU,"mnu");
  IupShowXY(dlg,IUP_CENTER,IUP_CENTER);
  IupMainLoop();
  IupDestroy(IupGetHandle("img1"));
  IupDestroy(IupGetHandle("img2"));
  IupDestroy(dlg);
//  IupControlsClose();      
  IupClose();  
  return 0;
}
