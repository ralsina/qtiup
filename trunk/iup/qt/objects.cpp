#include <QApplication>
#include <QDialog>
#include <QPushButton>
#include <QTextEdit>
#include <QResizeEvent>
#include <QPainter>
#include <QMenuBar>
#include <QX11Info>

extern int qapp_exec;


QObject *
topLevel (QObject * w)
{
  if (w->parent () == 0)
    return w;
  else
    return topLevel (w->parent ());
}

//God I hate CPP's inescrutability. If I put these above toplevel
//it doesn't build
#include "objects.h"
#include <iglobal.h>

// Stuff from qt.cpp
extern "C"
{
  void iupdrvGetCharSize (Ihandle * n, int *w, int *h);
  char *iupdrvGetAttribute (Ihandle * n, char *a);
  void iupdrvResizeObjects (Ihandle * n);
}
extern IObject *iupmot_parent_widget;
void iupmotSetAttrs (Ihandle * n);

void
createMenu (Ihandle * n)
{
  printf ("CreateMenu\n");
  IQMenu *w =
    new IQMenu (dynamic_cast < QWidget * >(iupmot_parent_widget), n);
  n->handle = static_cast < void *>(static_cast < IObject * >(w));
  iupmotSetAttrs (n);
}


void
createMultiline (Ihandle * n)
{
  printf ("CreateMultiline\n");
  IQMultiline *w =
    new IQMultiline (dynamic_cast < QWidget * >(iupmot_parent_widget), n);
  n->handle = static_cast < void *>(static_cast < IObject * >(w));
  iupmotSetAttrs (n);
}

void
createList (Ihandle * n)
{
  int isdropdown = iupCheck (n,
                             IUP_DROPDOWN) == YES ? 1 : 0;
  int haseditbox = iupCheck (n,
                             "EDITBOX") == YES ? 1 : 0;
  int ismulti = iupCheck (n,
                          "MULTIPLE") == YES ? 1 : 0;
  printf ("CreateList %d %d %d\n", isdropdown, haseditbox, ismulti);
  if (isdropdown)
    {
      IQCombo *w =
        new IQCombo (dynamic_cast < QWidget * >(iupmot_parent_widget), n);
      w->setEditable (haseditbox);
      n->handle = static_cast < void *>(static_cast < IObject * >(w));
    }
  else
    {
      IQList *w =
        new IQList (dynamic_cast < QWidget * >(iupmot_parent_widget), n);
      n->handle = static_cast < void *>(static_cast < IObject * >(w));
      if (ismulti)
        {
          w->setSelectionMode (QAbstractItemView::MultiSelection);
        }
      if (haseditbox)
        {
          //FIXME this doesn't work for some reason.
          w->
            connect
            (w,
             SIGNAL
             (itemClicked
              (QListWidgetItem *)), w, SLOT (editItem (QListWidgetItem *)));
        }
    }
  iupmotSetAttrs (n);
}

void
createItem (Ihandle * n)
{
  printf ("CreateItem\n");
  IQItem *i =
    new IQItem (dynamic_cast < QWidget * >(iupmot_parent_widget), n);
  n->handle = static_cast < void *>(static_cast < IObject * >(i));
  iupmotSetAttrs (n);
}

void
createCanvas (Ihandle * n)
{
  printf ("CreateCanvas\n");
  IQCanvas *w =
    new IQCanvas (dynamic_cast < QWidget * >(iupmot_parent_widget), n);
  n->handle = static_cast < void *>(static_cast < IObject * >(w));
  w->widget = w;
  iupmotSetAttrs (n);

  //Call MAP_CB
  IFn cb = (IFn) (IupGetCallback (n, "MAP_CB"));
  if (cb)
    {
      int result = cb (n);
      if (result == IUP_CLOSE)
        {
          QWidget *w = dynamic_cast < QWidget * >(topLevel (w));
          if (w)
            w->close ();
          if (qapp_exec)
            qApp->exit ();
        }
    }
}


void
createButton (Ihandle * n)
{
  printf ("CreateButton\n");
  IQButton *w =
    new IQButton (dynamic_cast < QWidget * >(iupmot_parent_widget), n);;
  n->handle = static_cast < void *>(static_cast < IObject * >(w));
  w->widget = w;
  iupmotSetAttrs (n);
}

void
createText (Ihandle * n)
{
  printf ("CreateText\n");
  IQText *w =
    new IQText (dynamic_cast < QWidget * >(iupmot_parent_widget), n);
  n->handle = static_cast < void *>(static_cast < IObject * >(w));
  w->widget = w;
  iupmotSetAttrs (n);
}

void
createLabel (Ihandle * n)
{
  printf ("CreateLabel\n");
  IQLabel *w =
    new IQLabel (dynamic_cast < QWidget * >(iupmot_parent_widget), n);
  n->handle = static_cast < void *>(static_cast < IObject * >(w));
  w->widget = w;
  iupmotSetAttrs (n);
}

void
createFrame (Ihandle * n)
{
  printf ("CreateFrame\n");
  IQFrame *w =
    new IQFrame (dynamic_cast < QWidget * >(iupmot_parent_widget), n);
  n->handle = static_cast < void *>(static_cast < IObject * >(w));
  w->widget = w;
  iupmotSetAttrs (n);
}

void
createDialog (Ihandle * n)
{
  printf ("CreateDialog\n");
  IQDialog *w = new IQDialog (n);
  n->handle = static_cast < void *>(static_cast < IObject * >(w));
  w->widget = w;
  iupmotSetAttrs (n);
  //Call MAP_CB
  IFn cb = (IFn) (IupGetCallback (n, "MAP_CB"));
  if (cb)
    {
      int result = cb (n);
      if (result == IUP_CLOSE)
        {
          QWidget *w = dynamic_cast < QWidget * >(topLevel (w));
          if (w)
            w->close ();
          if (qapp_exec)
            qApp->exit ();
        }
    }
}

void
createImage (Ihandle * n, Ihandle * prnt, int inactive)
{
  int width = IupGetInt (n, IUP_WIDTH);
  int height = IupGetInt (n, IUP_HEIGHT);
  unsigned char *data = (unsigned char *) (image_data (n));
  unsigned int red, green, blue;
  QColor bgcolor;
  QColor table[256];
  char attr[6];

  static struct
  {
    unsigned int r, g, b;
  } default_colors[] =
  {
    {
    0, 0, 0},
    {
    128, 0, 0},
    {
    0, 128, 0},
    {
    128, 128, 0},
    {
    0, 0, 128},
    {
    128, 0, 128},
    {
    0, 128, 128},
    {
    192, 192, 192},
    {
    128, 128, 128},
    {
    255, 0, 0},
    {
    0, 255, 0},
    {
    255, 255, 0},
    {
    0, 0, 255},
    {
    255, 0, 255},
    {
    0, 255, 255},
    {
  255, 255, 255}};

  char *value = IupGetAttribute (prnt ? prnt : n, IUP_BGCOLOR);
  if (value)
    {
      if (iupGetRGB (value, &red, &green, &blue))
        bgcolor = QColor (red, green, blue);
    }

  int i, j;

  for (i = 0; i < 16; i++)
    {
      sprintf (attr, "%d", i);
      value = IupGetAttribute (n, attr);

      if (value)
        {
          if (iupStrEqual (value, "BGCOLOR"))
            table[i] = bgcolor;
          else
            {
              if (!iupGetRGB (value, &red, &green, &blue))
                table[i] =
                  QColor (default_colors[i].r, default_colors[i].g,
                          default_colors[i].b);
              else
                table[i] = QColor (red, green, blue);
            }
        }
      else
        {
          table[i] =
            QColor (default_colors[i].r, default_colors[i].g,
                    default_colors[i].b);
        }
    }

  for (; i < 256; i++)
    {
      sprintf (attr, "%d", i);
      value = IupGetAttribute (n, attr);
      if (!value)
        break;

      iupGetRGB (value, &red, &green, &blue);
      table[i] = QColor (red, green, blue);
    }

  //FIXME: implement inactive (probably using Qt)

//   if (inactive)
//   {
//     Pixmap fginactive;
//     unsigned char bgred, bggreen, bgblue,
//                   red, green, blue;
//     int total_colors = i;

  /* replace the non transparent colors by a darker version of the background color. */
  /* replace near white by the background color */
  /* replace similar colors to the background color by itself */

//     iupmot_GetRGB(bgcolor, &bgred, &bggreen, &bgblue);
//     fginactive = iupmot_GetPixel(bgred/2, bggreen/2, bgblue/2);

//     for (i=0;i<total_colors;i++)
//     {
//       iupmot_GetRGB(table[i], &red, &green, &blue);
// 
//       if (((red + green + blue)/3 > 250) ||
//           ((abs(red-bgred) + abs(green-bggreen) + abs(blue-bgblue))) < 9 )
//       {
//         table[i] = bgcolor;
//       }
//       else
//       {
//         table[i] = fginactive;
//       }
//     }
//   }

  QPixmap *pxm = new QPixmap (width, height);
  QPainter p (pxm);
  for (i = 0; i < height; i++)
    {
      for (j = 0; j < width; j++)
        {
          p.setPen (table[data[i * width + j]]);
          p.drawPoint (j, i);
        }
    }

  if (inactive)
    iupSetEnv (n, "_IUPMOT_IMINACTIVE",
               static_cast < char *>(static_cast < void *>(pxm)));
  else
    handle (n) = static_cast < void *>(pxm);
}

char *
mouseEventToStatus (QMouseEvent * event)
{
  char *status = new char[6];
  strcpy (status, "     ");
  if (event->buttons () | Qt::LeftButton)
    {
      status[2] = IUP_BUTTON1;
    }
  else if (event->buttons () | Qt::MidButton)
    {
      status[3] = IUP_BUTTON2;
    }
  else if (event->buttons () | Qt::RightButton)
    {
      status[4] = IUP_BUTTON3;
    };

  Qt::KeyboardModifiers mods = event->modifiers ();
  if (mods & Qt::ShiftModifier)
    status[0] = 'S';
  if (mods & Qt::ControlModifier)
    status[1] = 'C';
  status[5] = ' ';
  return status;
}



IObject::IObject ()
{
  tag = IUP_YES;
}

char *
IObject::get_active ()
{
  QWidget *w = dynamic_cast < QWidget * >(this);
  if (!w)
    return IUP_YES;
  if (w->isEnabled ())
    return IUP_YES;
  return IUP_NO;
}

char *
IObject::get_alignment ()
{
  printf ("Calling get_alignment Stub\n");
  return NULL;
}

char *
IObject::get_bgcolor ()
{
  QString color;
  QWidget *w = dynamic_cast < QWidget * >(this);
  QColor bg = w->palette ().window ().color ();
  color.sprintf ("%d %d %d", bg.red (), bg.green (), bg.blue ());
  return qstrdup (color.toAscii ().data ());
}

char *
IObject::get_caret ()
{
  printf ("Calling get_caret Stub\n");
  return NULL;
}

char *
IObject::get_conid ()
{
  printf ("Calling get_conid Stub\n");
  return NULL;
}

char *
IObject::get_fgcolor ()
{
  QString color;
  QColor fg =
    (dynamic_cast < QWidget * >(this))->palette ().windowText ().color ();
  color.sprintf ("%d %d %d", fg.red (), fg.green (), fg.blue ());
  return qstrdup (color.toAscii ().data ());
}

char *
IObject::get_font ()
{
  QString rn = ((QWidget *) (this))->font ().toString ();
  return qstrdup (rn.toAscii ().data ());
}

char *
IObject::get_multiple ()
{
  printf ("Calling get_multiple Stub\n");
  return NULL;
}

char *
IObject::get_nc ()
{
  return qstrdup ("999999999");
}

char *
IObject::get_posx ()
{
  printf ("Calling get_posx Stub\n");
  return NULL;
}

char *
IObject::get_posy ()
{
  printf ("Calling get_posy Stub\n");
  return NULL;
}

char *
IObject::get_rastersize ()
{
  QWidget *w = dynamic_cast < QWidget * >(this);
  QString s;
  s.sprintf ("%dx%d", w->width (), w->height ());
  printf ("SIZE=%s\n", s.toAscii ().data ());
  return qstrdup (s.toAscii ().data ());
}

char *
IObject::get_readonly ()
{
  printf ("Calling get_readonly Stub\n");
  return NULL;
}

char *
IObject::get_selectcolor ()
{
  printf ("Calling get_selectcolor Stub\n");
  return NULL;
}

char *
IObject::get_selectedtext ()
{
  printf ("Calling get_selectedtext Stub\n");
  return NULL;
}

char *
IObject::get_selection ()
{
  printf ("Calling get_selection Stub\n");
  return NULL;
}

char *
IObject::get_size ()
{
  QWidget *w = dynamic_cast < QWidget * >(this);
  QString s;
  int charWidth, charHeight;
  iupdrvGetCharSize (n, &charWidth, &charHeight);
  s.sprintf ("%dx%d", w->width () * 4 / charWidth,
             w->height () * 4 / charHeight);
  printf ("SIZE=%s\n", s.toAscii ().data ());
  return qstrdup (s.toAscii ().data ());
}

char *
IObject::get_truecolorcnv ()
{
  printf ("Calling get_truecolorcnv Stub\n");
  return NULL;
}

char *
IObject::get_value ()
{
  printf ("Calling get_value Stub\n");
  return NULL;
}

char *
IObject::get_visible ()
{
  QWidget *w = dynamic_cast < QWidget * >(this);
  if (w && w->isVisible ())
    return IUP_YES;
  return IUP_NO;
}

char *
IObject::get_wid ()
{
  QWidget *w = dynamic_cast < QWidget * >(this);
  if (w)
    {
      QString s;
      s.sprintf ("%d", w->winId ());
      return qstrdup (s.toAscii ().data ());
    }
  return NULL;
}

char *
IObject::get_x ()
{
  printf ("Calling get_x Stub\n");
  return NULL;
}

char *
IObject::get_display ()
{
  return (char *) (QX11Info::display ());
}

char *
IObject::get_screen ()
{
  int *i = new int;
  QWidget *w = dynamic_cast < QWidget * >(this);
  *i = w->x11Info ().screen ();
  return (char *) (i);
}


char *
IObject::get_window ()
{
  QWidget *w = dynamic_cast < QWidget * >(this);
  return (char *) (w->winId ());
}

char *
IObject::get_y ()
{
  printf ("Calling get_window Stub\n");
  return NULL;
}

char *
IObject::get_margin ()
{
  QWidget *_w = dynamic_cast < QWidget * >(this);
  int w, h, z;
  _w->getContentsMargins (&w, &h, &z, &z);
  QString v;
  v.sprintf ("%dx%d", w, h);
  return qstrdup (v.toAscii ().data ());
}

void
IObject::set_active (char *v)
{
  int value;
  if (!v)
    value = TRUE;
  else if (iupStrEqualNoCase (v, IUP_YES))
    value = TRUE;
  else if (iupStrEqualNoCase (v, IUP_NO))
    value = FALSE;
  else
    return;
  QWidget *w = dynamic_cast < QWidget * >(this);
  if (w)
  	w->setEnabled (value);
}

void
IObject::set_alignment (char *v)
{
  printf ("Calling set_alignment Stub\n");
}

void
IObject::set_append (char *v)
{
  printf ("Calling set_append Stub\n");
}

void
IObject::set_bgcolor (char *v)
{
  if (!v)
    return;
  QWidget *w = dynamic_cast < QWidget * >(this);
  int r, g, b;
  sscanf (v, "%d %d %d", &r, &g, &b);
  QPalette p = w->palette ();
  p.setColor (QPalette::Window, QColor (r, g, b));
  p.setColor (QPalette::Base, QColor (r, g, b));
  p.setColor (QPalette::Button, QColor (r, g, b));
  w->setPalette (p);
  w->setBackgroundRole (QPalette::Window);
}

void
IObject::set_border (char *v)
{
  printf ("Calling set_border Stub\n");
}

void
IObject::set_caret (char *v)
{
  printf ("Calling set_caret Stub\n");
}

void
IObject::set_cursor (char *v)
{
  printf ("Calling set_cursor Stub\n");
}

void
IObject::set_defaultenter (char *v)
{
  printf ("Calling set_defaultenter Stub\n");
}

void
IObject::set_dx (char *v)
{
  set_posx (iupdrvGetAttribute (n, IUP_POSX));
}

void
IObject::set_dy (char *v)
{
  set_posy (iupdrvGetAttribute (n, IUP_POSY));
}

void
IObject::set_fgcolor (char *v)
{
  QWidget *w = dynamic_cast < QWidget * >(this);
  int r, g, b;
  sscanf (v, "%d %d %d", &r, &g, &b);
  QPalette p = w->palette ();
  p.setColor (QPalette::WindowText, QColor (r, g, b));
  p.setColor (QPalette::ButtonText, QColor (r, g, b));
  p.setColor (QPalette::Text, QColor (r, g, b));
  w->setPalette (p);
}

void
IObject::set_font (char *v)
{
  QWidget *w = dynamic_cast < QWidget * >(this);
  char *f = IupMapFont (v);
  if (f == NULL)
    f = v;
  QFont qf;
  qf.fromString (f);
  w->setFont (qf);
}

void
IObject::set_icon (char *v)
{
  printf ("Calling set_icon Stub\n");
}

void
IObject::set_image (char *v)
{
  printf ("Calling set_image Stub\n");
}

void
IObject::set_iminactive (char *v)
{
  printf ("Calling set_iminactive Stub\n");
}

void
IObject::set_impress (char *v)
{
  printf ("Calling set_impress Stub\n");
}

void
IObject::set_insert (char *v)
{
  printf ("Calling set_insert Stub\n");
}

void
IObject::set_key (char *v)
{
  printf ("Calling set_key Stub\n");
}

void
IObject::set_mask (char *v)
{
  printf ("Calling set_mask Stub\n");
}

void
IObject::set_menu (char *v)
{
  printf ("Calling set_menu Stub\n");
}

void
IObject::set_multiple (char *v)
{
  printf ("Calling set_multiple Stub\n");
}

void
IObject::set_nc (char *v)
{
  printf ("Calling set_nc Stub\n");
}

void
IObject::set_parentdialog (char *v)
{
  printf ("Calling set_parentdialog Stub\n");
}

void
IObject::set_posx (char *v)
{
  printf ("Calling set_posx Stub\n");
}

void
IObject::set_posy (char *v)
{
  printf ("Calling set_posy Stub\n");
}

void
IObject::set_rastersize (char *v)
{
  printf ("Calling set_rastersize Stub\n");
}

void
IObject::set_readonly (char *v)
{
  printf ("Calling set_readonly Stub\n");
}

void
IObject::set_selectcolor (char *v)
{
  printf ("Calling set_selectcolor Stub\n");
}

void
IObject::set_selection (char *v)
{
  printf ("Calling set_selection Stub\n");
}

void
IObject::set_selectedtext (char *v)
{
  printf ("Calling set_selectedtext Stub\n");
}

void
IObject::set_size (char *v)
{
  printf ("Calling set_size Stub\n");
}

void
IObject::set_title (char *v)
{
  printf ("Calling set_title Stub\n");
}

void
IObject::set_value (char *v)
{
  printf ("Calling set_value Stub\n");
}

void
IObject::set_visible (char *v)
{
  QWidget *w = dynamic_cast < QWidget * >(this);
  bool vis = iupCheck (n, IUP_VISIBLE) == YES ? 1 : 0;
  w->setVisible (vis);
}

void
IObject::set_fullscreen (char *v)
{
  printf ("Calling set_fullscreen Stub\n");
}

void
IObject::set_separator (char *v)
{
  printf ("Calling set_separator Stub\n");
}

void
IObject::set_showdropdown (char *v)
{
  printf ("Calling set_showdropdown Stub\n");
}

void
IObject::set_password (char *v)
{
  printf ("Calling set_password Stub\n");
}

void
IObject::set_zorder (char *v)
{
  printf ("Calling set_zorder Stub\n");
}

void
IObject::set_visibleitems (char *v)
{
  printf ("Calling set_visibleitems Stub\n");
}

void
IObject::set_margin (char *v)
{
  QWidget *_w = dynamic_cast < QWidget * >(this);
  int w, h;
  sscanf (v, "%dx%d", &w, &h);
  _w->setContentsMargins (w, h, w, h);
}

/////////////////////////////////////////////////

IQItem::IQItem (QWidget * parent, Ihandle * h):
IObject (), QAction (parent)
{
  parent->addAction (this);
  n = h;
}

void
IQItem::set_title (char *v)
{
}


IQTimer::IQTimer (QObject * parent, Ihandle * h):
IObject (), QTimer (parent)
{
  n = h;
  period = -1;
}

void
IQTimer::set_time (char *v)
{
  period = atoi (IupGetAttribute (n, "TIME"));
}

void
IQTimer::set_run (char *v)
{
  if (strcmp (v, "YES") == 0)
    {
      connect (this, SIGNAL (timeout ()), this, SLOT (doCallback ()));
      start (period);
    }
  else
    {
      disconnect (this, SIGNAL (timeout ()), this, SLOT (doCallback ()));
    }

}
void
IQTimer::doCallback ()
{
  IFn cb = (IFn) (IupGetCallback (n, IUP_ACTION));
  if (cb)
    {
      int result = cb (n);
      if (result == IUP_CLOSE)
        {
          QWidget *w = dynamic_cast < QWidget * >(topLevel (this));
          if (w)
            w->close ();
          if (qapp_exec)
            qApp->exit ();
        }
      else
        start (period);
    }
}

/////////////////////////////////////////////////


IQCanvas::IQCanvas (QWidget * parent, Ihandle * h):
IObject (), QFrame (parent)
{
  n = h;
  pressed = FALSE;
  setAttribute (Qt::WA_PaintOutsidePaintEvent);
}

void
IQCanvas::paintEvent (QPaintEvent * event)
{
  IFnff cb = (IFnff) IupGetCallback (n, "ACTION");
  if (cb)
    {
      printf ("canvas paintevent\n");
      int result = cb (n, 0, 0);
      if (result == IUP_CLOSE)
        {
          ((QWidget *) topLevel (this))->close ();
          if (qapp_exec)
            qApp->exit ();
        }
    }
}

void
IQCanvas::resizeEvent (QResizeEvent * event)
{
  printf ("canvaresizecallback1\n");
  IFnii cb = (IFnii) (IupGetCallback (n, IUP_RESIZE_CB));
  if (cb)
    {
      printf ("canvaresizecallback2\n");
      int result = cb (n, event->size ().width (), event->size ().height ());
      if (result == IUP_CLOSE)
        {
          ((QWidget *) topLevel (this))->close ();
          if (qapp_exec)
            qApp->exit ();
        }
    }
}

void
IQCanvas::mouseMoveEvent (QMouseEvent * event)
{
  IFniis cb = (IFniis) (IupGetCallback (n, IUP_MOTION_CB));
  if (cb)
    {
      printf ("canvasmovecallback\n");
      char *status = mouseEventToStatus (event);
      printf ("status=%d\n", status[2]);
      int x = event->x ();
      int y = event->y ();
      int result = cb (n, x, y, status);
      if (result == IUP_CLOSE)
        {
          ((QWidget *) topLevel (this))->close ();
          if (qapp_exec)
            qApp->exit ();
        }
    }
}


void
IQCanvas::mousePressEvent (QMouseEvent * event)
{
  pressed = TRUE;


  IFniiiis cb = (IFniiiis) (IupGetCallback (n, IUP_BUTTON_CB));
  if (cb)
    {
      printf ("canvascallback\n");
      char *status = mouseEventToStatus (event);
      int button;
      if (event->button () == Qt::LeftButton)
        {
          button = IUP_BUTTON1;
        }
      else if (event->button () == Qt::MidButton)
        {
          button = IUP_BUTTON2;
        }
      else if (event->button () == Qt::RightButton)
        {
          button = IUP_BUTTON3;
        };

      Qt::KeyboardModifiers mods = event->modifiers ();

      int x = event->x ();
      int y = height () - event->y ();
      int result = cb (n, button, 1, x, y, status);
      if (result == IUP_CLOSE)
        {
          ((QWidget *) topLevel (this))->close ();
          if (qapp_exec)
            qApp->exit ();
        }
    }







  QWidget::mousePressEvent (event);
}

void
IQCanvas::mouseReleaseEvent (QMouseEvent * event)
{
  if (pressed)
    {
      pressed = FALSE;
      IFniiiis cb = (IFniiiis) (IupGetCallback (n, IUP_BUTTON_CB));
      if (cb)
        {
          printf ("canvascallback\n");
          char *status = new char[6];
          int button;
          if (event->button () == Qt::LeftButton)
            {
              button = IUP_BUTTON1;
              status[2] = IUP_BUTTON1;
            }
          else if (event->button () == Qt::MidButton)
            {
              button = IUP_BUTTON2;
              status[3] = IUP_BUTTON2;
            }
          else if (event->button () == Qt::RightButton)
            {
              button = IUP_BUTTON3;
              status[4] = IUP_BUTTON3;
            };

          Qt::KeyboardModifiers mods = event->modifiers ();
          if (mods & Qt::ShiftModifier)
            status[0] = 'S';
          if (mods & Qt::ControlModifier)
            status[1] = 'C';

          status[5] = ' ';

          int x = event->x ();
          int y = height () - event->y ();
          int result = cb (n, button, 0, x, y, status);
          if (result == IUP_CLOSE)
            {
              ((QWidget *) topLevel (this))->close ();
              if (qapp_exec)
                qApp->exit ();
            }
        }
      QWidget::mouseReleaseEvent (event);
    }
}


/////////////////////////////////////////////////



IQLabel::IQLabel (QWidget * parent, Ihandle * h):
IObject (), QLabel (parent)
{
  n = h;
}

char *
IQLabel::get_alignment ()
{
  if (alignment () == Qt::AlignLeft)
    return IUP_ALEFT;
  else if (alignment () == Qt::AlignRight)
    return IUP_ARIGHT;
  else if (alignment () == Qt::AlignCenter)
    return IUP_ACENTER;
}

void
IQLabel::set_alignment (char *v)
{
  if (0 == strcmp (v, IUP_ALEFT))
    setAlignment (Qt::AlignLeft);
  else if (0 == strcmp (v, IUP_ARIGHT))
    setAlignment (Qt::AlignRight);
  else if (0 == strcmp (v, IUP_ACENTER))
    setAlignment (Qt::AlignCenter);
}

void
IQLabel::set_title (char *v)
{
  //If we have a IMAGE attrib, we should not set the text
  char *img = IupGetAttribute (n, "IMAGE");
  if (!img)
    setText (v);
}

void
IQLabel::set_image (char *v)
{
  if (!v)
    return;
  Ihandle *img = IupGetHandle (v);
  if (!img)
    return;
  if (handle (img) == NULL)
    createImage (img, n, 0);

  QPixmap *p = static_cast < QPixmap * >(handle (img));
  setPixmap (*p);
}





















IAbstractList::IAbstractList ():IObject ()
{
}
void
IAbstractList::set_option (Ihandle * n, int op, char *v)
{
  printf ("Calling set_option Stub\n");
}




IQList::IQList (QWidget * parent, Ihandle * h):
QListWidget (parent), IObject ()
{
  n = h;
}

char *
IQList::get_multiple ()
{
  if (selectionMode () == QAbstractItemView::MultiSelection)
    return IUP_YES;
  return IUP_NO;
}

char *
IQList::get_value ()
{
  if (selectionMode () == QAbstractItemView::MultiSelection)
    {
      QString result;
      for (int i = 0; i < count (); i++)
        {
          if (isItemSelected (item (i)))
            result += "+";
          else
            result += "-";
        }
      return qstrdup (result.toAscii ().data ());
    }
  else
    {
      int i = currentRow () + 1;
      if (i < 1)
        i = 1;
      QString v;
      v.sprintf ("%d", i);
      return qstrdup (v.toAscii ().data ());
    }
}

void
IQList::set_value (char *v)
{
  int ismulti = iupCheck (n, "MULTIPLE") == YES ? 1 : 0;
  if (ismulti)
    {
      //TODO implement set_value for multiselection lists
    }
  else
    setItemSelected (item (atoi (v)), TRUE);
  return;
}

void
IQList::set_option (Ihandle * n, int op, char *v)
{
  static char nstr[10];
  char *val;
  int i = op;
  while (1)
    {
      sprintf (nstr, "%d", i);
      val = iupGetEnv (n, nstr);
      if (!val)
        break;
      while (count () < i)
        {
          addItem ("");
        }
      printf ("setting option #%d to %s\n", i - 1, val);
      item (i - 1)->setText (QString (val));
      i++;
    }
}

void
IQList::editCurrentItem (QListWidgetItem * i)
{
  printf ("editcurrentitem");
  i->setFlags (Qt::ItemIsEditable);
  editItem (i);
}

void
IQList::doActionCallback (int i)
{
}

IQCombo::IQCombo (QWidget * parent, Ihandle * h):
QComboBox (parent), IAbstractList ()
{
  n = h;
  connect (this, SIGNAL (editTextChanged (char *)), this,
           SLOT (doEditCallback (char *)));
  connect (this, SIGNAL (activated (int)), this,
           SLOT (doActionCallback (int)));
}

char *
IQCombo::get_caret ()
{
  QString val;
  val.sprintf ("%d", lineEdit ()->cursorPosition ());
  return qstrdup (val.toAscii ().data ());
}

char *
IQCombo::get_readonly ()
{
  if (isEditable ())
    return IUP_YES;
  else
    return IUP_NO;
}

char *
IQCombo::get_selectedtext ()
{
  if (isEditable ())
    return qstrdup (lineEdit ()->selectedText ().toAscii ().data ());
  return NULL;
}

char *
IQCombo::get_selection ()
{
  if (isEditable ())
    {
      QString val;
      int b, e, _b;
      b = 1;
      e = 1;
      _b = lineEdit ()->selectionStart ();
      if (_b != -1)
        {
          b = _b;
          e = b + lineEdit ()->selectedText ().length ();
        }
      val.sprintf ("%d:%d", b, e);
      return qstrdup (val.toAscii ().data ());
    }
  return NULL;
}

char *
IQCombo::get_value ()
{
  if (isEditable ())
    return qstrdup (lineEdit ()->text ().toAscii ().data ());
  else
    {
      int i = currentIndex () + 1;
      if (i < 1)
        i = 1;
      QString v;
      v.sprintf ("%d", i);
      return qstrdup (v.toAscii ().data ());
    }
}

void
IQCombo::set_caret (char *v)
{
  if (isEditable ())
    {
      char val[10];
      int pos;
      sscanf (val, "%d", pos);
      lineEdit ()->setCursorPosition (pos);
      return;
    }
}

void
IQCombo::set_readonly (char *v)
{
  bool ro = iupCheck (n, "READONLY") == YES ? TRUE : FALSE;
  setEditable (!ro);
}

void
IQCombo::set_selection (char *v)
{
  if (isEditable ())
    {
      int b, e;
      sscanf (v, "%d:%d", b, e);
      lineEdit ()->setSelection (b, e - b);
    }
}

void
IQCombo::set_value (char *v)
{
  setCurrentIndex (atoi (v));
}

void
IQCombo::set_showdropdown (char *v)
{
  bool show = iupCheck (n, "SHOWDROPDOWN") == YES ? 1 : 0;
  if (show)
    showPopup ();
  else
    hidePopup ();
}


void
IQCombo::doActionCallback (int i)
{
  static int olditem = -1;
  QString t;
  IFnsii cb = (IFnsii) IupGetCallback (n, IUP_ACTION);
  if (cb)
    {
      t = itemText (olditem);
      printf ("Doing Action Callback\n");
      if (olditem != -1)
        {
          //call the deselection of the old item.
          int result = cb (n, qstrdup (t.toAscii ().data ()),
                           olditem, 0);
          if (result == IUP_CLOSE)
            {
              dynamic_cast < QWidget * >(topLevel (this))->close ();
              if (qapp_exec)
                qApp->exit ();
              //Ok, if someone makes that deselecting an
              //item exits the app, then the selection
              //callback is not called. But this is nuts.
              return;
            }
        }
      t = itemText (i);
      //and call the selection of the new one.
      char *v = qstrdup (t.toAscii ().data ());
      int result = cb (n, v, i, 1);
      if (result == IUP_CLOSE)
        {
          dynamic_cast < QWidget * >(topLevel (this))->close ();
          if (qapp_exec)
            qApp->exit ();
        }
      olditem = i;
    }
}

void
IQCombo::set_option (Ihandle * n, int op, char *v)
{
  static char nstr[10];
  char *val;
  int i = op;
  while (1)
    {
      sprintf (nstr, "%d", i);
      val = iupGetEnv (n, nstr);
      if (!val)
        break;
      while (count () < i)
        {
          addItem ("");
        }
      printf ("setting option #%d to %s\n", i - 1, val);
      setItemText (i - 1, val);
      i++;
    }
}

void
IQCombo::doEditCallback (char *t)
{
  setItemText (currentIndex (), t);
  IFnis cb = (IFnis) IupGetCallback (n, "EDIT_CB");
  if (cb)
    {
      char *after = qstrdup (t);
      printf ("Doing Edit Callback -> %s\n", after);
      int result = cb (n, currentIndex (), after);
      if (result == IUP_CLOSE)
        {
          ((QWidget *) topLevel (this))->close ();
          if (qapp_exec)
            qApp->exit ();
        }
    }
}


IQButton::IQButton (QWidget * parent, Ihandle * h):QPushButton (parent)
{
  pressed = FALSE;
  n = h;
  connect (this, SIGNAL (clicked ()), this, SLOT (doActionCallback ()));
  //Set properties
  move (n->posx, n->posy);
  resize (n->current.width, n->current.height);
  char *
    i = IupGetAttribute (n, "IMAGE");
  char *
    t = IupGetAttribute (n, "TITLE");
  if (t)
    setText (t);
}

void
IQButton::set_title (char *v)
{
  //If we have a IMAGE attrib, we should not set the text
  char *img = IupGetAttribute (n, "IMAGE");
  if (!img)
    setText (v);
}

void
IQButton::set_image (char *v)
{
  if (!v)
    return;
  Ihandle *img = IupGetHandle (v);
  if (!img)
    return;
  if (handle (img) == NULL)
    createImage (img, n, 0);

  QPixmap *p = static_cast < QPixmap * >(handle (img));
  setIcon (*p);
  setText ("");
}

void
IQButton::doActionCallback ()
{
  printf ("Doing Button Callback\n");
  Icallback cb = IupGetCallback (n, IUP_ACTION);
  if (cb)
    {
      int result = cb (n);
      if (result == IUP_CLOSE)
        {
          ((QWidget *) topLevel (this))->close ();
          if (qapp_exec)
            qApp->exit ();
        }
    }
}

void
IQButton::enterEvent (QEvent * ev)
{
  printf ("Doing Enter Callback\n");
  Icallback cb = IupGetCallback (n, IUP_ENTERWINDOW_CB);
  if (cb)
    {
      int result = cb (n);
      if (result == IUP_CLOSE)
        {
          ((QWidget *) topLevel (this))->close ();
          if (qapp_exec)
            qApp->exit ();
        }
    }
  QPushButton::enterEvent (ev);
}

void
IQButton::leaveEvent (QEvent * ev)
{
  printf ("Doing Leave Callback\n");
  Icallback cb = IupGetCallback (n, IUP_LEAVEWINDOW_CB);
  if (cb)
    {
      int result = cb (n);
      if (result == IUP_CLOSE)
        {
          ((QWidget *) topLevel (this))->close ();
          if (qapp_exec)
            qApp->exit ();
        }
    }
  QPushButton::leaveEvent (ev);
}

void
IQButton::mousePressEvent (QMouseEvent * event)
{
  pressed = TRUE;
  QPushButton::mousePressEvent (event);
}

void
IQButton::mouseReleaseEvent (QMouseEvent * event)
{
  if (pressed)
    {
      pressed = FALSE;
      printf ("Doing Leave Callback\n");
      Icallback cb = IupGetCallback (n, IUP_BUTTON_CB);
      if (cb)
        {
          int result = cb (n);
          if (result == IUP_CLOSE)
            {
              ((QWidget *) topLevel (this))->close ();
              if (qapp_exec)
                qApp->exit ();
            }
        }
      QPushButton::mouseReleaseEvent (event);
    }
}



IQDialog::IQDialog (Ihandle * h):QDialog (), IObject ()
{
  n = h;
  //Set properties
  if (h)
    {
      move (h->posx, h->posy);
      resize (h->current.width, h->current.height);
      char *
        t = IupGetAttribute (h, "TITLE");
      if (t)
        {
          setWindowTitle (t);
          printf ("Dialog title is %s\n", t);
        }
      else
        printf ("No title on dialog creation\n");
    }

}

void
IQDialog::set_title (char *v)
{
  setWindowTitle (v);
}

void
IQDialog::resizeEvent (QResizeEvent * ev)
{
  printf ("resizing dialog\n");
  static bool inside = FALSE;
  if (inside == FALSE)
    {
      inside = TRUE;
      QString s;
      s.sprintf ("%dx%d", ev->size ().width (), ev->size ().height ());
      iupStoreEnv (n, IUP_RASTERSIZE, s.toAscii ().data ());
      iupSetEnv (n, IUP_SIZE, NULL);
      if (iupSetSize (n) != IUP_ERROR)
        {
          iupdrvResizeObjects (child (n));
        }
    }
  inside = FALSE;
  QDialog::resizeEvent (ev);
}

IQMenu::IQMenu (QWidget * p, Ihandle * h):QMenuBar (p), IObject ()
{
  n = h;
}


IQFrame::IQFrame (QWidget * p, Ihandle * h):QGroupBox (p), IObject ()
{
  n = h;
  connect (this, SIGNAL (clicked ()), this, SLOT (doCallback ()));
  //Set properties
  move (h->posx, h->posy);
  resize (h->current.width, h->current.height);
  char *
    t = IupGetAttribute (h, "TITLE");
  if (t)
    setTitle (t);
}

void
IQFrame::set_title (char *v)
{
  setTitle (v);
}


IQMultiline::IQMultiline (QWidget * parent, Ihandle * h):QTextEdit (parent),
IObject ()
{
  n = h;
}

char *
IQMultiline::get_value ()
{
  return qstrdup (document ()->toPlainText ().toAscii ().data ());
}

char *
IQMultiline::get_readonly ()
{
  if (isReadOnly ())
    return IUP_YES;
  else
    return IUP_NO;
}

void
IQMultiline::set_insert (char *v)
{
  insertPlainText (v);
}

void
IQMultiline::set_append (char *v)
{
  append (v);
}

void
IQMultiline::set_readonly (char *v)
{
  bool ro = iupCheck (n, "READONLY") == YES ? TRUE : FALSE;
  setReadOnly (!ro);
}


IQText::IQText (QWidget * parent, Ihandle * h):QLineEdit (parent), IObject ()
{
  n = h;
  connect (this, SIGNAL (textChanged (char *)), this,
           SLOT (doActionCallback (char *)));
}

char *
IQText::get_value ()
{
  return qstrdup (text ().toAscii ().data ());
}

void
IQText::set_readonly (char *v)
{
  bool ro = iupCheck (n, "READONLY") == YES ? TRUE : FALSE;
  setReadOnly (ro);
}

void
IQText::set_selection (char *v)
{
  int b, e;
  sscanf (v, "%d:%d", b, e);
  setSelection (b, e - b);
}

void
IQText::set_value (char *v)
{
  setText (v);
}

void
IQText::set_password (char *v)
{
  bool pwd = iupCheck (n, "PASSWORD") == YES ? 1 : 0;
  if (pwd)
    setEchoMode (QLineEdit::Password);
  else
    setEchoMode (QLineEdit::Normal);
}


void
IQText::keyPressEvent (QKeyEvent * ev)
{
  printf ("Doing IQText ACTION Callback\n");
  char *after;
  IFnis cb = (IFnis) IupGetCallback (n, IUP_ACTION);
  if (cb)
    {
      //FIXME the key tables in IUP and Qt are completely
      //different :-P
      int result = cb (n, ev->key (), after);
      if (result == IUP_CLOSE)
        {
          ((QWidget *) topLevel (this))->close ();
          if (qapp_exec)
            qApp->exit ();
        }
      else if (result == IUP_IGNORE)
        {
          ev->ignore ();
          return;
        }
      else
        {
          setText (after);
          ev->accept ();
        }
    }
  QLineEdit::keyPressEvent (ev);
}
