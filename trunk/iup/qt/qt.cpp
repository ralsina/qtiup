/** \file
 * \brief Qt Driver Core and Initialization
 *
 * See Copyright Notice in iup.h
 * $Id: motif.c,v 1.153 2006/03/21 01:03:28 scuri Exp $
 */


#if defined(__STDC__) && defined(ULTRIX)
#define X$GBLS(x)  #x
#endif

#include <stdio.h>              /* scanf */
#include <string.h>             /* strlen */
#include <assert.h>             /* strlen */
#include <sys/utsname.h>
#include <unistd.h>


#include "objects.h"


#include <QApplication>
#include <QObject>
#include <QX11Info>
#include <QWidget>
#include <QMenu>
#include <QLineEdit>
#include <QFontMetrics>
#include <QLabel>
#include <QStyle>
#include <QMessageBox>

#include <X11/Xlib.h>
#include <iupdef.h>
#include <iup.h>
#include <istrutil.h>
#include <inames.h>
#include <ifunc.h>
#include <iupcpi.h>
#include <ilex.h>
#include <iglobalenv.h>
#include <imacros.h>
#include <iglobal.h>
#include <itimer.h>
#include <idrv.h>

// #include "ifunc.h"
// #include "iglobalenv.h"
// #include "ifiledlg.h"
// #include "itimer.h"
// #include "idrv.h"

int qapp_exec = FALSE;

typedef unsigned short Dimension;


typedef char *(*get_fn) (Ihandle *);
typedef void (*set_fn) (Ihandle * n, char *v);

Itable *attrEnv_get = 0;
Itable *attrEnv_set = 0;
Itable *attrDefEnv = 0;
#define NSTORE 31

static struct
{
  char *attr;
  char *type;
} store[NSTORE];

static const char *ALL_ = "ALL";
static const char *QUERY_ = "QUERY";
static const char *NOTDIALOG_ = "NOTDIALOG";
static const char *WIDGET_ = "WIDGET";

static const char *progname = "Iup";

static int iup_opened = 0;

void iupmotGetInitAttTable (void);
void iupmotSetInitAttTable (void);

IObject *iupmot_parent_widget = NULL;
Ihandle *iupmot_parent = NULL;

///////////////////////////Start of getters
static char *
get_active (Ihandle * n)
{
  if (n == NULL || n->handle == NULL)
    return NULL;
  return (static_cast < IObject * >(n->handle))->get_active ();
}

static char *
get_alignment (Ihandle * n)
{
  if (n == NULL || n->handle == NULL)
    return NULL;
  return (static_cast < IObject * >(n->handle))->get_alignment ();
}

static char *
get_bgcolor (Ihandle * n)
{
  if (n == NULL || n->handle == NULL)
    return NULL;
  return (static_cast < IObject * >(n->handle))->get_bgcolor ();
}

static char *
get_caret (Ihandle * n)
{
  if (n == NULL || n->handle == NULL)
    return NULL;
  return (static_cast < IObject * >(n->handle))->get_caret ();
}

static char *
get_conid (Ihandle * n)
{
  if (n == NULL || n->handle == NULL)
    return NULL;
  return (static_cast < IObject * >(n->handle))->get_conid ();
}


static char *
get_fgcolor (Ihandle * n)
{
  if (n == NULL || n->handle == NULL)
    return NULL;
  return (static_cast < IObject * >(n->handle))->get_fgcolor ();
}

static char *
get_font (Ihandle * n)
{
  if (n == NULL || n->handle == NULL)
    return NULL;
  return (static_cast < IObject * >(n->handle))->get_font ();
}

static char *
get_multiple (Ihandle * n)
{
  if (n == NULL || n->handle == NULL)
    return NULL;
  return (static_cast < IObject * >(n->handle))->get_multiple ();
}

static char *
get_nc (Ihandle * n)
{
  if (n == NULL || n->handle == NULL)
    return NULL;
  return (static_cast < IObject * >(n->handle))->get_nc ();
}

static char *
get_posx (Ihandle * n)
{
  if (n == NULL || n->handle == NULL)
    return NULL;
  return (static_cast < IObject * >(n->handle))->get_posx ();
}

static char *
get_posy (Ihandle * n)
{
  if (n == NULL || n->handle == NULL)
    return NULL;
  return (static_cast < IObject * >(n->handle))->get_posy ();
}

static char *
get_rastersize (Ihandle * n)
{
  if (n == NULL || n->handle == NULL)
    return NULL;
  return (static_cast < IObject * >(n->handle))->get_rastersize ();
}

static char *
get_readonly (Ihandle * n)
{
  if (n == NULL || n->handle == NULL)
    return NULL;
  return (static_cast < IObject * >(n->handle))->get_readonly ();
}

static char *
get_selectcolor (Ihandle * n)
{
  if (n == NULL || n->handle == NULL)
    return NULL;
  return (static_cast < IObject * >(n->handle))->get_selectcolor ();
}

static char *
get_selectedtext (Ihandle * n)
{
  if (n == NULL || n->handle == NULL)
    return NULL;
  return (static_cast < IObject * >(n->handle))->get_selectedtext ();
}

static char *
get_selection (Ihandle * n)
{
  if (n == NULL || n->handle == NULL)
    return NULL;
  return (static_cast < IObject * >(n->handle))->get_selection ();
}

static char *
get_size (Ihandle * n)
{
  printf ("Getting size\n");
  if (n == NULL || n->handle == NULL)
    return NULL;
  return (static_cast < IObject * >(n->handle))->get_size ();
}

static char *
get_truecolorcnv (Ihandle * n)
{
  if (n == NULL || n->handle == NULL)
    return NULL;
  return (static_cast < IObject * >(n->handle))->get_truecolorcnv ();
}

static char *
get_value (Ihandle * n)
{
  if (n == NULL || n->handle == NULL)
    return NULL;
  return (static_cast < IObject * >(n->handle))->get_value ();
}

static char *
get_visible (Ihandle * n)
{
  if (n == NULL || n->handle == NULL)
    return NULL;
  return (static_cast < IObject * >(n->handle))->get_visible ();
}

static char *
get_wid (Ihandle * n)
{
  if (n == NULL || n->handle == NULL)
    return NULL;
  return (static_cast < IObject * >(n->handle))->get_wid ();
}

static char *
get_x (Ihandle * n)
{
  if (n == NULL || n->handle == NULL)
    return NULL;
  return (static_cast < IObject * >(n->handle))->get_x ();
}

static char *
get_y (Ihandle * n)
{
  if (n == NULL || n->handle == NULL)
    return NULL;
  return (static_cast < IObject * >(n->handle))->get_y ();
}

static char *
get_margin (Ihandle * n)
{
  if (n == NULL || n->handle == NULL)
    return NULL;
  return (static_cast < IObject * >(n->handle))->get_margin ();
}

///////////////////////////End of getters


///////////////////////////Start of setters


static void
set_active (Ihandle * n, char *v)
{
  if (n == NULL || n->handle == NULL)
    return;
  (static_cast < IObject * >(n->handle))->set_active (v);
}

static void
set_alignment (Ihandle * n, char *v)
{
  if (n == NULL || n->handle == NULL)
    return;
  (static_cast < IObject * >(n->handle))->set_alignment (v);
}


static void
set_append (Ihandle * n, char *v)
{
  if (n == NULL || n->handle == NULL)
    return;
  (static_cast < IObject * >(n->handle))->set_append (v);
}



static void
set_bgcolor (Ihandle * n, char *v)
{
  if (n == NULL || n->handle == NULL)
    return;
  (static_cast < IObject * >(n->handle))->set_bgcolor (v);
}

static void
set_border (Ihandle * n, char *v)
{
  if (n == NULL || n->handle == NULL)
    return;
  (static_cast < IObject * >(n->handle))->set_border (v);
}

static void
set_caret (Ihandle * n, char *v)
{
  if (n == NULL || n->handle == NULL)
    return;
  (static_cast < IObject * >(n->handle))->set_caret (v);
}

static void
set_cursor (Ihandle * n, char *v)
{
  if (n == NULL || n->handle == NULL)
    return;
  (static_cast < IObject * >(n->handle))->set_cursor (v);
}

static void
set_defaultenter (Ihandle * n, char *v)
{
  if (n == NULL || n->handle == NULL)
    return;
  (static_cast < IObject * >(n->handle))->set_defaultenter (v);
}

static void
set_dx (Ihandle * n, char *v)
{
  if (n == NULL || n->handle == NULL)
    return;
  (static_cast < IObject * >(n->handle))->set_dx (v);
}

static void
set_dy (Ihandle * n, char *v)
{
  if (n == NULL || n->handle == NULL)
    return;
  (static_cast < IObject * >(n->handle))->set_dy (v);
}


static void
set_fgcolor (Ihandle * n, char *v)
{
  printf ("Setting fgcolor to %s\n", v);
  if (n == NULL || n->handle == NULL)
    return;
  (static_cast < IObject * >(n->handle))->set_fgcolor (v);
}

static void
set_font (Ihandle * n, char *v)
{
  if (n == NULL || n->handle == NULL)
    return;
  (static_cast < IObject * >(n->handle))->set_font (v);
}

static void
set_icon (Ihandle * n, char *v)
{
  if (n == NULL || n->handle == NULL)
    return;
  (static_cast < IObject * >(n->handle))->set_icon (v);
}

static void
set_image (Ihandle * n, char *v)
{
  if (n == NULL || n->handle == NULL)
    return;
  (static_cast < IObject * >(n->handle))->set_image (v);
}

static void
set_iminactive (Ihandle * n, char *v)
{
  if (n == NULL || n->handle == NULL)
    return;
  (static_cast < IObject * >(n->handle))->set_icon (v);
}

static void
set_impress (Ihandle * n, char *v)
{
  if (n == NULL || n->handle == NULL)
    return;
  (static_cast < IObject * >(n->handle))->set_icon (v);
}

static void
set_insert (Ihandle * n, char *v)
{
  if (n == NULL || n->handle == NULL)
    return;
  (static_cast < IObject * >(n->handle))->set_insert (v);
}

static void
set_key (Ihandle * n, char *v)
{
  if (n == NULL || n->handle == NULL)
    return;
  (static_cast < IObject * >(n->handle))->set_key (v);
}

static void
set_mask (Ihandle * n, char *v)
{
  if (n == NULL || n->handle == NULL)
    return;
  (static_cast < IObject * >(n->handle))->set_mask (v);
}

static void
set_menu (Ihandle * n, char *v)
{
  if (n == NULL || n->handle == NULL)
    return;
  (static_cast < IObject * >(n->handle))->set_menu (v);
}

static void
set_multiple (Ihandle * n, char *v)
{
  if (n == NULL || n->handle == NULL)
    return;
  (static_cast < IObject * >(n->handle))->set_multiple (v);
}

static void
set_nc (Ihandle * n, char *v)
{
  if (n == NULL || n->handle == NULL)
    return;
  (static_cast < IObject * >(n->handle))->set_nc (v);
}

static void
set_parentdialog (Ihandle * n, char *v)
{
  if (n == NULL || n->handle == NULL)
    return;
  (static_cast < IObject * >(n->handle))->set_parentdialog (v);
}

static void
set_posx (Ihandle * n, char *v)
{
  if (n == NULL || n->handle == NULL)
    return;
  (static_cast < IObject * >(n->handle))->set_posx (v);
}

static void
set_posy (Ihandle * n, char *v)
{
  if (n == NULL || n->handle == NULL)
    return;
  (static_cast < IObject * >(n->handle))->set_posy (v);
}

static void
set_rastersize (Ihandle * n, char *v)
{
  if (n == NULL || n->handle == NULL)
    return;
  (static_cast < IObject * >(n->handle))->set_rastersize (v);
}

static void
set_readonly (Ihandle * n, char *v)
{
  if (n == NULL || n->handle == NULL)
    return;
  (static_cast < IObject * >(n->handle))->set_readonly (v);
}

static void
set_selectcolor (Ihandle * n, char *v)
{
  if (n == NULL || n->handle == NULL)
    return;
  (static_cast < IObject * >(n->handle))->set_selectcolor (v);
}

static void
set_selection (Ihandle * n, char *v)
{
  if (n == NULL || n->handle == NULL)
    return;
  (static_cast < IObject * >(n->handle))->set_selection (v);
}

static void
set_selectedtext (Ihandle * n, char *v)
{
  if (n == NULL || n->handle == NULL)
    return;
  (static_cast < IObject * >(n->handle))->set_selectedtext (v);
}

static void
set_size (Ihandle * n, char *v)
{
  if (n == NULL || n->handle == NULL)
    return;
  printf ("set_size: %s %d %d\n", type (n), n,
          (static_cast < IObject * >(n->handle))->n);
  (static_cast < IObject * >(n->handle))->set_size (v);
}

static void
set_title (Ihandle * n, char *v)
{
  if (n == NULL || n->handle == NULL)
    return;
  (static_cast < IObject * >(n->handle))->set_title (v);
}

static void
set_value (Ihandle * n, char *v)
{
  if (n == NULL || n->handle == NULL)
    return;
  (static_cast < IObject * >(n->handle))->set_value (v);
}

static void
set_visible (Ihandle * n, char *v)
{
  if (n == NULL || n->handle == NULL)
    return;
  (static_cast < IObject * >(n->handle))->set_visible (v);
}

static void
set_fullscreen (Ihandle * n, char *v)
{
  if (n == NULL || n->handle == NULL)
    return;
  (static_cast < IObject * >(n->handle))->set_fullscreen (v);
}

static void
set_separator (Ihandle * n, char *v)
{
  if (n == NULL || n->handle == NULL)
    return;
  (static_cast < IObject * >(n->handle))->set_separator (v);
}

static void
set_showdropdown (Ihandle * n, char *v)
{
  if (n == NULL || n->handle == NULL)
    return;
  (static_cast < IObject * >(n->handle))->set_showdropdown (v);
}

static void
set_password (Ihandle * n, char *v)
{
  if (n == NULL || n->handle == NULL)
    return;
  (static_cast < IObject * >(n->handle))->set_password (v);
}

static void
set_zorder (Ihandle * n, char *v)
{
  if (n == NULL || n->handle == NULL)
    return;
  (static_cast < IObject * >(n->handle))->set_zorder (v);
}

static void
set_visibleitems (Ihandle * n, char *v)
{
  if (n == NULL || n->handle == NULL)
    return;
  (static_cast < IObject * >(n->handle))->set_visibleitems (v);
}

static void
set_margin (Ihandle * n, char *v)
{
  if (n == NULL || n->handle == NULL)
    return;
  (static_cast < IObject * >(n->handle))->set_margin (v);
  printf ("Setting margin to %s\n", v);
  QWidget *_w =
    dynamic_cast < QWidget * >(static_cast < IObject * >(n->handle));
  int w, h;
  sscanf (v, "%dx%d", &w, &h);
  _w->setContentsMargins (w, h, w, h);
}

static void
set_time (Ihandle * n, char *v)
{
  printf ("set_time\n");
  if (n == NULL || n->handle == NULL)
    return;
  IQTimer *t =
    dynamic_cast < IQTimer * >(static_cast < IObject * >(n->handle));
  if (t)
    t->set_time (v);
}

static void
set_run (Ihandle * n, char *v)
{
  printf ("set_run\n");
  if (n == NULL || n->handle == NULL)
    return;
  IQTimer *t =
    dynamic_cast < IQTimer * >(static_cast < IObject * >(n->handle));
  if (t)
    {
      t->set_run (v);
    }
}















static struct table
{
  char *name;
  set_fn set;
}

attributes_set[] =
{
  {
  IUP_ACTIVE, set_active},
  {
  IUP_ALIGNMENT, set_alignment},
  {
  IUP_APPEND, set_append},
  {
  IUP_BGCOLOR, set_bgcolor},
  {
  IUP_BORDER, set_border},
  {
  IUP_CARET, set_caret},
    //{
    //IUP_CURSOR, set_cursor},
    //{
    //IUP_DEFAULTENTER, set_defaultenter},
  {
  IUP_DX, set_dx},
  {
  IUP_DY, set_dy},
  {
  IUP_FGCOLOR, set_fgcolor},
  {
  IUP_FONT, set_font},
  {
  IUP_ICON, set_icon},
  {
  IUP_IMAGE, set_image},
  {
  IUP_IMINACTIVE, set_iminactive},
  {
  IUP_IMPRESS, set_impress},
  {
  IUP_INSERT, set_insert},
  {
  IUP_KEY, set_key},
  {
  "MASK", set_mask},
  {
  IUP_MENU, set_menu},
//    {"MOTIF_FONT",      set_font},
  {
  IUP_MULTIPLE, set_multiple},  /* multiple tem que ficar antes de value */
  {
  IUP_NC, set_nc},
  {
  IUP_PARENTDIALOG, set_parentdialog},
  {
  IUP_POSX, set_posx},
  {
  IUP_POSY, set_posy},
  {
  IUP_RASTERSIZE, set_rastersize},
  {
  "READONLY", set_readonly},
  {
  "SELECTCOLOR", set_selectcolor},
  {
  IUP_SELECTION, set_selection},
  {
  IUP_SELECTEDTEXT, set_selectedtext},
  {
  IUP_SIZE, set_size},
  {
  IUP_TITLE, set_title},
  {
  IUP_VALUE, set_value},
  {
  IUP_VISIBLE, set_visible},
  {
  IUP_FULLSCREEN, set_fullscreen},
  {
  IUP_SEPARATOR, set_separator},
  {
  "SHOWDROPDOWN", set_showdropdown},
  {
  "PASSWORD", set_password},
  {
  "ZORDER", set_zorder},
  {
  IUP_VISIBLE_ITEMS, set_visibleitems},
  {
  "TIME", set_time},
  {
  "RUN", set_run},
  {
  IUP_MARGIN, set_margin}
};
static struct
{
  char *name;
  get_fn get;
  char *def;
}

attributes_get[] =
{
  {
  IUP_ACTIVE, get_active, IUP_YES},
  {
  IUP_ALIGNMENT, get_alignment, NULL},
  {
  IUP_BGCOLOR, get_bgcolor, NULL},
  {
  IUP_BORDER, NULL, IUP_YES},
  {
  IUP_CARET, get_caret, NULL},
  {
  IUP_CONID, get_conid, NULL},
  {
  IUP_CURSOR, NULL, IUP_ARROW},
  {
  IUP_DX, NULL, "0.1"},
  {
  IUP_DY, NULL, "0.1"},
  {
  IUP_FGCOLOR, get_fgcolor, NULL},
  {
  IUP_FONT, get_font, NULL},
  {
  IUP_MAX, NULL, "1.0"},
  {
  IUP_MIN, NULL, "0.0"},
  {
  IUP_MULTIPLE, get_multiple, IUP_NO},
  {
  IUP_NC, get_nc, "2048"},
  {
  IUP_POSX, get_posx, "0.0"},
  {
  IUP_POSY, get_posy, "0.0"},
  {
  "DRAWSIZE", get_rastersize, NULL},
  {
  IUP_RASTERSIZE, get_rastersize, NULL},
  {
  "READONLY", get_readonly, IUP_NO},
  {
  IUP_SCROLLBAR, NULL, IUP_NO},
  {
  "SELECTCOLOR", get_selectcolor, NULL},
  {
  IUP_SELECTEDTEXT, get_selectedtext, NULL},
  {
  IUP_SELECTION, get_selection, NULL},
  {
  IUP_SIZE, get_size, NULL},
  {
  "TRUECOLORCANVAS", get_truecolorcnv, NULL},
  {
  IUP_VALUE, get_value, NULL},
  {
  IUP_VISIBLE, get_visible, NULL},
  {
  IUP_WID, get_wid, NULL},
  {
  IUP_X, get_x, NULL},
  {
  IUP_XMAX, NULL, "1.0"},
  {
  IUP_XMIN, NULL, "0.0"},
  {
  IUP_Y, get_y, NULL},
  {
  IUP_YMAX, NULL, "1.0"},
  {
  IUP_YMIN, NULL, "0.0"},
  {
  IUP_RESIZE, NULL, "YES"},
  {
  IUP_MARGIN, get_margin, "0"},
  {
  IUP_INSERT, NULL, ""}
};
int
IupOpen (void)
{
  if (iup_opened)
    return IUP_OPENED;
  iup_opened = 1;
  iupNamesInit ();
  iupFuncInit ();
  iupLexInit ();
  iupCpiInit ();
  iupGlobalEnvInit ();
  iupmotSetInitAttTable ();
  iupmotGetInitAttTable ();
  char **argv = NULL;
  int argc;
  QApplication *app = new QApplication (argc, argv);
//   {
//     static char ss[15];
//     sprintf( ss, "%dx%d", DisplayWidth(iupmot_display,iupmot_screen),
//       DisplayHeight(iupmot_display,iupmot_screen) );
//     IupSetGlobal( IUP_SCREENSIZE, ss );
//   }

  {
    static char sd[10];
    sprintf (sd, "%d", QX11Info::appDepth ());
    IupSetGlobal ("SCREENDEPTH", sd);
  }
/*  iupmotMessageDlgOpen();
  IupFileDlgOpen();*/
  return IUP_NOERROR;
}

int
IupMainLoop (void)
{
  qapp_exec = TRUE;
  QApplication::exec ();
  qapp_exec = FALSE;
  return IUP_NOERROR;
}

void
IupFlush (void)
{
  qApp->processEvents ();
  return;
}

static void
set_option (Ihandle * n, int op, char *v)
{
  printf ("set_option %d %s %d %s\n", op, v, handle (n), type (n));
  if (handle (n) && type (n) == LIST_ && op > 0)
    {
      IAbstractList *l =
        static_cast < IAbstractList * >(static_cast < IObject * >(n->handle));
      l->set_option (n, op, v);
    }
}

void
iupmotSetAttrs (Ihandle * n)
{
  printf ("Setting attributes\n");
  int i;
  /* procura opcoes da lista */
  if (type (n) == LIST_ && env (n))
    {
      char *val;
      val = iupGetEnv (n, "1");
      if (val)
        set_option (n, 1, val);
    }
  int NATTRIBUTES = sizeof (attributes_set) / sizeof (attributes_set[0]);
  for (i = 0; i < NATTRIBUTES; i++)
    {
      char *a = attributes_set[i].name;
      printf ("setting %s\n", a);
      char *val;
      if ((iupStrEqualNoCase (a, IUP_SIZE)) ||  /* estes atributos nao sao herdados */
          (iupStrEqualNoCase (a, IUP_VALUE)) ||
          (iupStrEqualNoCase (a, IUP_TITLE)))
        {
          if (!env (n))
            continue;
          val = (char *) iupTableGet (env (n), a);
        }
      else
        {
          val = iupGetEnv (n, a);
        }
      if (val)
        (*attributes_set[i].set) (n, val);
    }
}

/************************************************************************
*                       Show/Hide/Popup Management
************************************************************************/


int
IupHide (Ihandle * n)
{
  if ((n == NULL) || (handle (n) == NULL))
    return IUP_ERROR;
  IupSetAttribute (n, IUP_VISIBLE, IUP_NO);
  IupFlush ();
  return IUP_NOERROR;
}

int
IupShowXY (Ihandle * n, int x, int y)
{
  if (!n || IupMap (n) == IUP_ERROR)
    return IUP_ERROR;
  QWidget *w =
    dynamic_cast < QWidget * >(static_cast < IObject * >(handle (n)));
  IupSetAttribute (n, IUP_VISIBLE, IUP_YES);
  w->move (x, y);
  w->show ();
  IupFlush ();
  return IUP_NOERROR;
}

int
IupShow (Ihandle * n)
{
  return IupShowXY (n, IupGetInt (n, IUP_X), IupGetInt (n, IUP_Y));
}

int
IupPopup (Ihandle * n, int x, int y)
{
  if (n == NULL || handle (n) == 0)
    {
      return IUP_ERROR;
    }

  if (type (n) != DIALOG_ && type (n) != MENU_)
    return IUP_ERROR;
  IQDialog *d =
    dynamic_cast < IQDialog * >(static_cast < IObject * >(n->handle));
  if (d)
    d->exec ();
  else
    {
      QWidget *w =
        dynamic_cast < QWidget * >(static_cast < IObject * >(n->handle));
      w->show ();
    }
  return IUP_NOERROR;
}


/************************************************************************
*                       Open/Close/Map and others
************************************************************************/
void
iupmotSetParent (IObject * w, Ihandle * h)
{
  iupmot_parent_widget = w;
  iupmot_parent = h;
}

int
IupMap (Ihandle * n)
{
  printf ("Mapping\n");
  int createdialog = 0;
  if (n == NULL)
    {
      printf ("null mapping\n");
      return IUP_ERROR;
    }

  if (type (n) == DIALOG_)
    {
      if (!handle (n))
        createdialog = 1;
      if (iupSetSize (n) == IUP_ERROR)
        return IUP_ERROR;
    }
  else if (type (n) == MENU_)
    {
      if (!parent (n))
        iupmotSetParent (NULL, NULL);
      else if (!handle (parent (n)))
        return IUP_ERROR;
      else
        iupmotSetParent (static_cast < IObject * >(handle (parent (n))),
                         parent (n));
    }
  else if (type (n) == SUBMENU_ || type (n) == ITEM_)
    {
      if (!parent (n) || !handle (parent (n)))
        return IUP_ERROR;
      iupmotSetParent (static_cast < IObject * >(handle (parent (n))),
                       parent (n));
    }
  else
    {
      Ihandle *dial;
      dial = IupGetDialog (n);
      if (dial == NULL || handle (dial) == NULL)
        return IUP_ERROR;
      iupmotSetParent (static_cast < IObject * >(handle (dial)), dial);
    }
  iupdrvCreateObjects (n);
  iupmotSetParent (NULL, NULL);
  if (createdialog)
    {
      Icallback cb = IupGetCallback (n, IUP_MAP_CB);
      iupdrvSetAttribute (n, IUP_CURSOR, IupGetAttribute (n, IUP_CURSOR));
      iupdrvSetAttribute
        (n, IUP_DEFAULTENTER, IupGetAttribute (n, IUP_DEFAULTENTER));
      if (cb)
        {
          if (cb (n) == IUP_CLOSE)
            {

              dynamic_cast <
                QWidget *
                >(topLevel
                  (dynamic_cast <
                   QWidget * >(static_cast <
                               IObject * >((n->handle)))))->close ();
              if (qapp_exec)
                qApp->exit ();
            }
        }
    }

  if (type (n) == DIALOG_)
    iupdrvSetAttribute
      (n, "PARENTDIALOG", IupGetAttribute (n, "PARENTDIALOG"));
  IupFlush ();
  return IUP_NOERROR;
}


Ihandle *
IupSetFocus (Ihandle * n)
{
  return 0;
}

Ihandle *
IupGetFocus (void)
{
  return 0;
}

void
IupClose (void)
{
  if (!iup_opened)
    return;
  iup_opened = 0;
  qApp->quit ();
  iupStrGetMemory (-1);         /* Frees internal buffer */
  IupTimerClose ();
  iupGlobalEnvFinish ();
  iupNamesFinish ();
  iupLexFinish ();
  iupCpiFinish ();
  iupFuncFinish ();
}


void
iupdrvGetCharSize (Ihandle * n, int *w, int *h)
{
  if (n == NULL or n->handle == NULL)
    {
      printf ("iupdrvGetCharSize with null handle\n");
      QFontMetrics fm = qApp->fontMetrics ();
      *w = fm.maxWidth ();
      *h = fm.height ();
      return;
    }
  else
    {
      QWidget *widg =
        dynamic_cast < QWidget * >(static_cast < IObject * >((n->handle)));
      QFontMetrics fm = widg->fontMetrics ();
      *w = fm.maxWidth ();
      *h = fm.height ();
    }
}


void
IupTimerClose (void)
{
}

Ihandle *
IupTimer (void)
{
  printf ("CreateTimer\n");
  Ihandle *timer = IupUser ();
  IQTimer *t =
    new IQTimer (dynamic_cast < QWidget * >(iupmot_parent_widget), timer);
  timer->handle = static_cast < void *>(static_cast < IObject * >(t));
  iupmotSetAttrs (timer);
  return timer;
}

static void
qtResizeBasicElement (Ihandle * n, int x, int y)
{
  QWidget *widg =
    dynamic_cast < QWidget * >(static_cast < IObject * >(n->handle));
  if (widg)
    {
      if (n && handle (n))
        {
          printf
            ("resizing %d to %d, %d\n",
             (int) n, currentwidth (n), currentheight (n));
          widg->move (x, y);
          widg->resize (currentwidth (n), currentheight (n));
        }
    }
  else
    {
      printf ("widg error\n");
    }
}


void
iupdrvResizeObjects (Ihandle * n)
{
  Ihandle *c = NULL;
  assert (n);
  if (n == NULL)
    return;
  if (type (n) == DIALOG_)
    {
      qtResizeBasicElement (n, posx (n), posy (n));
      iupdrvResizeObjects (child (n));
    }
  else if (type (n) == ZBOX_ || type (n) == HBOX_ || type (n) == VBOX_)
    {
      foreachchild (c, n) iupdrvResizeObjects (c);
    }
  else if (type (n) == FRAME_)
    {
      qtResizeBasicElement (n, posx (n), posy (n));
      iupdrvResizeObjects (child (n));
    }
  else if (type (n) == RADIO_)
    {
      iupdrvResizeObjects (child (n));
    }
  else
    if
    (type
     (n)
     ==
     CANVAS_
     ||
     type
     (n)
     ==
     LIST_
     ||
     type
     (n)
     ==
     TOGGLE_
     ||
     type
     (n)
     ==
     BUTTON_
     || type (n) == LABEL_ || type (n) == TEXT_ || type (n) == MULTILINE_)
    {
      qtResizeBasicElement (n, posx (n), posy (n));
    }
}


int
iupdrvWindowSizeX (Ihandle * n, int scale)
{
  //FIXME: figure out what this hsould do, and make it do it
  printf ("iupdrvWindowSizeX is broken\n");
  int sw = 1024;
  int decorx = 0;
  return sw / scale - decorx;
}

int
iupdrvWindowSizeY (Ihandle * n, int scale)
{
  //FIXME: figure out what this hsould do, and make it do it
  printf ("iupdrvWindowSizeY is broken\n");
  int sh = 768;
  int decorx = 0;
  return sh / scale - decorx;
}

int
iupdrvGetScrollSize (void)
{
  return qApp->style ()->pixelMetric (QStyle::PM_ScrollBarSliderMin);
}



char *
iupdrvGetAttribute (Ihandle * n, char *a)
{
  get_fn getfunc;
  if (n == NULL || handle (n) == NULL || a == NULL)
    return NULL;
  getfunc = (get_fn) iupTableGet (attrEnv_get, a);
  if (getfunc != NULL)
    return (getfunc) (n);
  return NULL;
}

char *
iupdrvGetDefault (Ihandle * n, char *a)
{
  char *def;
  if (n == NULL || a == NULL)
    return NULL;
  if (iupStrEqual (a, IUP_FONT))
    {
      return QApplication::font ().toString ().toLatin1 ().data ();
    }
  else if (iupStrEqual (a, "EXPAND"))
    {
      if (type
          (n)
          ==
          FILL_
          ||
          type
          (n)
          ==
          CANVAS_
          ||
          type
          (n)
          ==
          FRAME_
          ||
          type
          (n)
          ==
          HBOX_
          || type (n) == RADIO_ || type (n) == VBOX_ || type (n) == ZBOX_)
        return IUP_YES;
      else
        return IUP_NO;
    }

  def = (char *) iupTableGet (attrDefEnv, a);
  return def;
}


int
iupdrvStoreAttribute (Ihandle * n, char *a)
{
  int i;
  if (n == NULL || a == NULL || hclass (n))
    return 1;
  i = -1;
  for (i = 0; i < NSTORE; i++)
    {
      char *type = store[i].type;
      if (type == ALL_ || type == QUERY_)
        {
          if (iupStrEqual (a, store[i].attr))
            return 0;
          if (type (n) != DIALOG_ && iupStrEqual (a, store[i].attr))
            return 0;
          if ((type (n)
               ==
               MULTILINE_
               ||
               type (n)
               ==
               CANVAS_
               ||
               type (n)
               ==
               BUTTON_
               ||
               type (n)
               == ITEM_
               ||
               type (n)
               ==
               LABEL_
               ||
               type (n)
               == TEXT_
               ||
               type (n)
               ==
               TOGGLE_
               ||
               type (n)
               ==
               SEPARATOR_
               ||
               type (n)
               == MENU_
               || type (n) == SUBMENU_) && iupStrEqual (a, store[i].attr))
            return 0;
        }
      else if (type == NOTDIALOG_)
        {
          if (type (n) != DIALOG_ && iupStrEqual (a, store[i].attr))
            return 0;
          if ((type (n)
               ==
               MULTILINE_
               ||
               type (n)
               ==
               CANVAS_
               ||
               type (n)
               ==
               BUTTON_
               ||
               type (n)
               == ITEM_
               ||
               type (n)
               ==
               LABEL_
               ||
               type (n)
               == TEXT_
               ||
               type (n)
               ==
               TOGGLE_
               ||
               type (n)
               ==
               SEPARATOR_
               ||
               type (n)
               == MENU_
               || type (n) == SUBMENU_) && iupStrEqual (a, store[i].attr))
            return 0;
        }
      else if (type == WIDGET_)
        {
          if ((type (n)
               ==
               MULTILINE_
               ||
               type (n)
               ==
               CANVAS_
               ||
               type (n)
               ==
               BUTTON_
               ||
               type (n)
               == ITEM_
               ||
               type (n)
               ==
               LABEL_
               ||
               type (n)
               == TEXT_
               ||
               type (n)
               ==
               TOGGLE_
               ||
               type (n)
               ==
               SEPARATOR_
               ||
               type (n)
               == MENU_
               || type (n) == SUBMENU_) && iupStrEqual (a, store[i].attr))
            return 0;
        }
      else
        {
          if (store[i].type == type (n) && iupStrEqual (a, store[i].attr))
            return 0;
        }
    }
  return 1;
}

int
iupdrvGetBorderSize (Ihandle * n)
{
  return 2;                     /* border+focus area */
}


int
iupdrvSetGlobal (const char *name, const char *value)
{
  if (iupStrEqual (name, "AUTOREPEAT"))
    {                           //NO idea how to do this with Qt
      XKeyboardControl values;
      if (iupStrEqual (value, "YES"))
        values.auto_repeat_mode = 1;
      else
        values.auto_repeat_mode = 0;
      XChangeKeyboardControl
        (QX11Info::display (), KBAutoRepeatMode, &values);
      return 1;
    }
  else if (iupStrEqual (name, "CURSORPOS"))
    {
      int x, y;
      if (sscanf (value, "%dx%d", &x, &y) == 2)
        {
          QCursor::setPos (x, y);
        }
      return 1;
    }
  return 0;
}


void
iupdrvCreateObject (Ihandle * self, Ihandle * owner)
{
  IObject *old_widget = iupmot_parent_widget;
  Ihandle *old_parent = iupmot_parent;
  iupmot_parent = owner;
  if (owner)
    {
      iupmot_parent_widget = static_cast < IObject * >(handle (owner));
    }
  iupdrvCreateObjects (self);
  iupmot_parent_widget = old_widget;
  iupmot_parent = old_parent;
}

void
iupdrvCreateObjects (Ihandle * n)
{
  int flag = 0;
  if (type
      (n)
      !=
      DIALOG_
      &&
      iupmot_parent_widget
      ==
      NULL && type (n) != COLOR_ && type (n) != IMAGE_ && type (n) != MENU_)
    {
      flag = 1;
      iupmot_parent = IupGetDialog (n);
      iupmot_parent_widget =
        static_cast < IObject * >(handle (iupmot_parent));
    }

  if (hclass (n))
    iupCpiMap (n, iupmot_parent);
  else
    {
      if (!handle (n))
        iupdrvCreateNativeObject (n);
    }

  if (flag)
    {
      flag = 0;
      iupmot_parent_widget = NULL;
      iupmot_parent = NULL;
    }
}


void
iupdrvCreateNativeObject (Ihandle * n)
{
  Ihandle *c;
  printf ("Creating a %s\n", type (n));
  if (type (n) == BUTTON_)
    {
      createButton (n);
    }
  else if (type (n) == CANVAS_)
    {
      createCanvas (n);
    }
  else if (type (n) == FILL_)
    {
      handle (n) = static_cast < void *>(iupmot_parent_widget);
    }
  else if (type (n) == FRAME_)
    {
      createFrame (n);
      iupdrvCreateObjects (child (n));
    }
  else if (type (n) == ITEM_)
    {
      createItem (n);
    }
  else if (type (n) == LABEL_)
    {
      createLabel (n);
    }
  else if (type (n) == LIST_)
    {
      createList (n);
    }                           /*
                                   else if(type(n) == RADIO_)
                                   {
                                   createRadio (n);
                                   } */
  else if (type (n) == TEXT_)
    {
      createText (n);
    }
  else if (type (n) == MULTILINE_)
    {
      createMultiline (n);
    }                           /*
                                   else if(type(n) == TOGGLE_)
                                   {
                                   createToggle (n);
                                   }
                                   else if(type(n) == SEPARATOR_)
                                   {
                                   createSeparator (n);
                                   }
                                   else if(type(n) == SUBMENU_)
                                   {
                                   createSubmenu (n);

                                   {
                                   Widget oldparent = iupmot_parent_widget;
                                   iupmot_parent_widget = (Widget)handle(n);

                                   iupdrvCreateObjects (child(n));

                                   iupmot_parent_widget = oldparent;
                                   }
                                   } */
  else if (type (n) == MENU_)
    {
      createMenu (n);

      {
        IObject *oldparent = iupmot_parent_widget;
        iupmot_parent_widget = static_cast <IObject *> (handle (n));

        foreachchild (c, n) iupdrvCreateObjects (c);

        iupmot_parent_widget = oldparent;
      }
    }
  else if (type (n) == VBOX_ || type (n) == HBOX_)
    {
      handle (n) = static_cast < void *>(iupmot_parent_widget);
      foreachchild (c, n) iupdrvCreateObjects (c);
    }                           /*
                                   else if(type(n) == ZBOX_)
                                   {
                                   handle(n) = iupmot_parent_widget;

                                   int v = iupmotGetZboxValue( n );
                                   foreachchild(c,n)
                                   {
                                   iupdrvCreateObjects(c);
                                   if (c!=v) IupSetAttribute(c,IUP_VISIBLE,IUP_NO);
                                   }
                                   IupSetAttribute(n, "_IUPMOT_PREVIOUS_ZBOX_VALUE", IupGetName(v));

                                   } */
  else if (type (n) == DIALOG_)
    {
      createDialog (n);
      /* the parent of the controls will be the dialog area */
      iupmot_parent_widget = static_cast < IObject * >(handle (n));
      iupmot_parent = n;
      iupdrvCreateObjects (child (n));
      iupmot_parent_widget = NULL;
      iupmot_parent = NULL;
    }
  else if (type (n) == IMAGE_)
    {
      createImage (n, NULL, 0);
    }
}

void
iupdrvStringSize (Ihandle * n, char *s, int *w, int *h)
{
if (w)
  *w = qApp->fontMetrics ().width (s);
if (h)
  *h = qApp->fontMetrics ().height ();
}


Ihandle *
IupFileDlg (void)
{
  return IupCreate ("filedlg");
}

// void IupFileDlgOpen(void)
// {
//   Iclass *ICFileDlg = iupCpiCreateNewClass("filedlg", NULL);
// 
//   iupCpiSetClassMethod(ICFileDlg, ICPI_SETATTR, (Imethod) motFileDlgSet);
//   iupCpiSetClassMethod(ICFileDlg, ICPI_MAP,     (Imethod) motFileDlgMap);
//   iupCpiSetClassMethod(ICFileDlg, ICPI_CREATE,  (Imethod) motFileDlgCreate);
//   iupCpiSetClassMethod(ICFileDlg, ICPI_GETSIZE, (Imethod) motFileDlgGetSize);
// }

void
iupdrvSetAttribute (Ihandle * n, char *a, char *v)
{
  Ihandle *c;
  set_fn setfunc;
  printf ("iupdrvSetAttribute %s, %s\n", a, v);
  if (n == NULL || handle (n) == NULL)
    return;
  setfunc = (set_fn) iupTableGet (attrEnv_set, a);
  if (setfunc == NULL)
    {
      int op;
      if (sscanf (a, "%d", &op) != 1)
        return;
      set_option (n, op, v);    /* Sets the value of a List element (1,2,etc) */
      return;
    }
  else
    (setfunc) (n, v);           /* Calls apropriate function to set atribute */
  /* Non-inherited attributes. */
  if (!iupIsInheritable (a) || iupStrEqualNoCase (a, IUP_VISIBLE))      /*Visibility is dealt in set_visible */
    return;
  foreachchild (c, n) if (iupTableGet (env (c), a) == NULL)
    iupdrvSetAttribute (c, a, v);
}

void
iupdrvDestroyHandle (Ihandle * h)
{
  QWidget *w =
    dynamic_cast < QWidget * >(static_cast < IObject * >(handle (h)));
  if (w)
    {
      w->hide ();
      w->deleteLater ();
    }
  h->handle = NULL;
}

Ihandle *idleTimer = 0;

void
iupdrvSetIdleFunction (Icallback f)
{
  if (!idleTimer)
    {
      idleTimer = IupTimer ();
    }
  IupSetCallback (idleTimer, "ACTION_CB", f);
  IupSetAttribute (idleTimer, "TIME", "0");
  if (f)
    IupSetAttribute (idleTimer, "RUN", IUP_YES);
  else
    IupSetAttribute (idleTimer, "RUN", IUP_NO);
}

void
iupdrvGetTextLineBreakSize (Ihandle * n, char *text, int *w, int *h)
{
  const char *curstr;
  const char *nextstr;
  int len, num_lin, lw;
  QRect r =
    qApp->fontMetrics ().boundingRect (0, 0, 100000, 1000000, Qt::AlignLeft,
                                       text);
  *w = r.width ();
  *h = r.height ();
}


char *
iupdrvGetGlobal (const char *name)
{
  if (iupStrEqual (name, "CURSORPOS"))
    {
      QPoint p = QCursor::pos ();
      char *curpos = iupStrGetMemory (50);
      sprintf (curpos, "%dx%d", p.x (), p.y ());
      return curpos;
    }
  return NULL;
}


void
IupMessage (char *title, char *msg)
{
  printf ("Displaying Message\n");
  QMessageBox::information (0, title, msg, QMessageBox::Ok);
}


void
iupmotGetInitAttTable (void)
{
  int i;
  if (attrEnv_get)
    return;
  store[0].attr = IUP_ACTIVE;
  store[0].type = WIDGET_;
  store[1].attr = IUP_FGCOLOR;
  store[1].type = WIDGET_;
  store[2].attr = IUP_BGCOLOR;
  store[2].type = WIDGET_;
  store[3].attr = IUP_WID;
  store[3].type = QUERY_;
  store[4].attr = IUP_X;
  store[4].type = QUERY_;
  store[5].attr = IUP_Y;
  store[5].type = QUERY_;
  store[6].attr = "XSCREEN";
  store[6].type = QUERY_;
  store[7].attr = "XWINDOW";
  store[7].type = QUERY_;
  store[8].attr = "XDISPLAY";
  store[8].type = QUERY_;
  store[9].attr = "SELECTCOLOR";
  store[9].type = TOGGLE_;
  store[10].attr = IUP_VALUE;
  store[10].type = TOGGLE_;
  store[11].attr = IUP_VALUE;
  store[11].type = MULTILINE_;
  store[12].attr = IUP_VALUE;
  store[12].type = TEXT_;
  store[13].attr = IUP_VALUE;
  store[13].type = ITEM_;
  store[14].attr = IUP_VALUE;
  store[14].type = LIST_;
  store[15].attr = IUP_VALUE;
  store[15].type = RADIO_;
  store[16].type = IUP_NC;
  store[16].attr = TEXT_;
  store[17].type = IUP_CARET;
  store[17].attr = TEXT_;
  store[18].type = IUP_CARET;
  store[18].attr = MULTILINE_;
  store[19].type = IUP_SELECTION;
  store[19].attr = TEXT_;
  store[20].type = IUP_SELECTION;
  store[20].attr = MULTILINE_;
  store[21].type = IUP_SELECTEDTEXT;
  store[21].attr = TEXT_;
  store[22].type = IUP_SELECTEDTEXT;
  store[22].attr = MULTILINE_;
  store[23].type = IUP_POSX;
  store[23].attr = CANVAS_;
  store[24].type = IUP_POSY;
  store[24].attr = CANVAS_;
  store[25].type = IUP_CONID;
  store[25].attr = CANVAS_;
  store[26].type = IUP_ALIGNMENT;
  store[26].attr = BUTTON_;
  store[27].type = IUP_MULTIPLE;
  store[27].attr = LIST_;
  store[28].type = IUP_ALIGNMENT;
  store[28].attr = LABEL_;
  store[29].type = IUP_ALIGNMENT;
  store[29].attr = TEXT_;
  store[30].type = IUP_ALIGNMENT;
  store[30].attr = MULTILINE_;
  attrEnv_get = iupTableCreate (IUPTABLE_STRINGINDEXED);
  attrDefEnv = iupTableCreate (IUPTABLE_STRINGINDEXED);
  int ll = sizeof (attributes_get) / sizeof (attributes_get[0]);
  for (i = 0; i < ll; i++)
    {
      iupTableSet
        (attrEnv_get,
         attributes_get[i].name, (void *) attributes_get[i].get, IUP_POINTER);
      iupTableSet
        (attrDefEnv,
         attributes_get[i].name, attributes_get[i].def, IUP_POINTER);
    }
}

void
iupmotSetInitAttTable (void)
{
  int i;
  if (attrEnv_set)
    return;
  attrEnv_set = iupTableCreate (IUPTABLE_STRINGINDEXED);
  int ll = sizeof (attributes_set) / sizeof (attributes_set[0]);
  for (i = 0; i < ll; i++)
    {
      iupTableSet
        (attrEnv_set,
         attributes_set[i].name, (void *) attributes_set[i].set, IUP_POINTER);
    }
};

#define IUP_NUM_FONT 36
static struct
{
  char Iup_name[50];
  char font[70];
}
IupFonts[IUP_NUM_FONT] =
{
  {
  IUP_HELVETICA_NORMAL_8, "helvetica,8,-1,5,50,0,0,0,0,0"},
  {
  IUP_HELVETICA_ITALIC_8, "helvetica,8,-1,5,50,1,0,0,0,0"},
  {
  IUP_HELVETICA_BOLD_8, "helvetica,8,-1,5,75,0,0,0,0,0"},
  {
  IUP_HELVETICA_NORMAL_10, "helvetica,10,-1,5,50,0,0,0,0,0"},
  {
  IUP_HELVETICA_ITALIC_10, "helvetica,10,-1,5,50,1,0,0,0,0"},
  {
  IUP_HELVETICA_BOLD_10, "helvetica,10,-1,5,75,0,0,0,0,0"},
  {
  IUP_HELVETICA_NORMAL_12, "helvetica,12,-1,5,50,0,0,0,0,0"},
  {
  IUP_HELVETICA_ITALIC_12, "helvetica,12,-1,5,50,1,0,0,0,0"},
  {
  IUP_HELVETICA_BOLD_12, "helvetica,12,-1,5,75,0,0,0,0,0"},
  {
  IUP_HELVETICA_NORMAL_14, "helvetica,14,-1,5,50,0,0,0,0,0"},
  {
  IUP_HELVETICA_ITALIC_14, "helvetica,14,-1,5,50,1,0,0,0,0"},
  {
  IUP_HELVETICA_BOLD_14, "helvetica,14,-1,5,75,0,0,0,0,0"},
  {
  IUP_COURIER_NORMAL_8, "courier,8,-1,5,50,0,0,0,0,0"},
  {
  IUP_COURIER_ITALIC_8, "courier,8,-1,5,50,1,0,0,0,0"},
  {
  IUP_COURIER_BOLD_8, "courier,8,-1,5,75,0,0,0,0,0"},
  {
  IUP_COURIER_NORMAL_10, "courier,10,-1,5,50,0,0,0,0,0"},
  {
  IUP_COURIER_ITALIC_10, "courier,10,-1,5,50,1,0,0,0,0"},
  {
  IUP_COURIER_BOLD_10, "courier,10,-1,5,75,0,0,0,0,0"},
  {
  IUP_COURIER_NORMAL_12, "courier,12,-1,5,50,0,0,0,0,0"},
  {
  IUP_COURIER_ITALIC_12, "courier,12,-1,5,50,1,0,0,0,0"},
  {
  IUP_COURIER_BOLD_12, "courier,12,-1,5,75,0,0,0,0,0"},
  {
  IUP_COURIER_NORMAL_14, "courier,14,-1,5,50,0,0,0,0,0"},
  {
  IUP_COURIER_ITALIC_14, "courier,14,-1,5,50,1,0,0,0,0"},
  {
  IUP_COURIER_BOLD_14, "courier,14,-1,5,75,0,0,0,0,0"},
  {
  IUP_TIMES_NORMAL_8, "times,8,-1,5,50,0,0,0,0,0"},
  {
  IUP_TIMES_ITALIC_8, "times,8,-1,5,50,1,0,0,0,0"},
  {
  IUP_TIMES_BOLD_8, "times,8,-1,5,75,0,0,0,0,0"},
  {
  IUP_TIMES_NORMAL_10, "times,10,-1,5,50,0,0,0,0,0"},
  {
  IUP_TIMES_ITALIC_10, "times,10,-1,5,50,1,0,0,0,0"},
  {
  IUP_TIMES_BOLD_10, "times,10,-1,5,75,0,0,0,0,0"},
  {
  IUP_TIMES_NORMAL_12, "times,12,-1,5,50,0,0,0,0,0"},
  {
  IUP_TIMES_ITALIC_12, "times,12,-1,5,50,1,0,0,0,0"},
  {
  IUP_TIMES_BOLD_12, "times,12,-1,5,75,0,0,0,0,0"},
  {
  IUP_TIMES_NORMAL_14, "times,14,-1,5,50,0,0,0,0,0"},
  {
  IUP_TIMES_ITALIC_14, "times,14,-1,5,50,1,0,0,0,0"},
  {
IUP_TIMES_BOLD_14, "times,14,-1,5,75,0,0,0,0,0"},};
char *
IupMapFont (char *iupfont)
{
  int c = 0;
  if (!iupfont)
    return NULL;
  for (c = 0; c < IUP_NUM_FONT; c++)
    {
      if (iupStrEqual (iupfont, IupFonts[c].Iup_name))
        return IupFonts[c].font;
    }

  return NULL;                  /* font was not an IUP font */
}

char *
IupUnMapFont (char *driverfont)
{
  int c = 0;
  for (c = 0; c < IUP_NUM_FONT; c++)
    if (iupStrEqual (driverfont, IupFonts[c].font))
      return IupFonts[c].Iup_name;
  return NULL;                  /* font was not an IUP font */
}


void
iupdrvUpdate (Ihandle * n)
{
}

int
IupLoopStep (void)
{
  qApp->processEvents ();
  return IUP_DEFAULT;
}
