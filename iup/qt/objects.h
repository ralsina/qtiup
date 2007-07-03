#ifndef __OBJECTS_H
#define __OBJECTS_H

#include <QPushButton>
#include <QLabel>
#include <QLineEdit>
#include <QDialog>
#include <QGroupBox>
#include <QTextEdit>
#include <QComboBox>
#include <QListWidget>
#include <QMenuBar>
#include <QTimer>

#include <iup.h>

class QResizeEvent;
class QKeyEvent;

QObject *topLevel (QObject * w);

class IObject
{
public:
  IObject ();
  Ihandle *n;
  QWidget *widget;
  char *tag;
  virtual char *get_active ();
  virtual char *get_alignment ();
  virtual char *get_bgcolor ();
  virtual char *get_caret ();
  virtual char *get_conid ();
  virtual char *get_fgcolor ();
  virtual char *get_font ();
  virtual char *get_multiple ();
  virtual char *get_nc ();
  virtual char *get_posx ();
  virtual char *get_posy ();
  virtual char *get_rastersize ();
  virtual char *get_readonly ();
  virtual char *get_selectcolor ();
  virtual char *get_selectedtext ();
  virtual char *get_selection ();
  virtual char *get_size ();
  virtual char *get_truecolorcnv ();
  virtual char *get_value ();
  virtual char *get_visible ();
  virtual char *get_wid ();
  virtual char *get_x ();
  virtual char *get_display ();
  virtual char *get_screen ();
  virtual char *get_window ();
  virtual char *get_y ();
  virtual char *get_margin ();

  virtual void set_active (char *v);
  virtual void set_alignment (char *v);
  virtual void set_append (char *v);
  virtual void set_bgcolor (char *v);
  virtual void set_border (char *v);
  virtual void set_caret (char *v);
  virtual void set_cursor (char *v);
  virtual void set_defaultenter (char *v);
  virtual void set_dx (char *v);
  virtual void set_dy (char *v);
  virtual void set_fgcolor (char *v);
  virtual void set_font (char *v);
  virtual void set_icon (char *v);
  virtual void set_image (char *v);
  virtual void set_iminactive (char *v);
  virtual void set_impress (char *v);
  virtual void set_insert (char *v);
  virtual void set_key (char *v);
  virtual void set_mask (char *v);
  virtual void set_menu (char *v);
  virtual void set_multiple (char *v);
  virtual void set_nc (char *v);
  virtual void set_parentdialog (char *v);
  virtual void set_posx (char *v);
  virtual void set_posy (char *v);
  virtual void set_rastersize (char *v);
  virtual void set_readonly (char *v);
  virtual void set_selectcolor (char *v);
  virtual void set_selection (char *v);
  virtual void set_selectedtext (char *v);
  virtual void set_size (char *v);
  virtual void set_title (char *v);
  virtual void set_value (char *v);
  virtual void set_visible (char *v);
  virtual void set_fullscreen (char *v);
  virtual void set_separator (char *v);
  virtual void set_showdropdown (char *v);
  virtual void set_password (char *v);
  virtual void set_zorder (char *v);
  virtual void set_visibleitems (char *v);
  virtual void set_margin (char *v);
};

class IQMenu:public QMenuBar, public IObject
{
Q_OBJECT public:
	IQMenu (QWidget *parent, Ihandle *_handle);
};

class IQItem: public QAction, public IObject
{
Q_OBJECT public:
	IQItem (QWidget *parent, Ihandle *_handle);
        virtual void set_title (char *v);
};

class IQTimer:public QTimer, public IObject
{
Q_OBJECT public:
    IQTimer (QObject * parent, Ihandle * _handle);
  virtual void set_time (char *v);
  virtual void set_run (char *v);
  int period;
  public slots:void doCallback ();
};

class IQCanvas:public QFrame, public IObject
{
Q_OBJECT public:
    IQCanvas (QWidget * parent, Ihandle * _handle);
  bool pressed;
protected:
  void paintEvent (QPaintEvent * event);
  void mousePressEvent (QMouseEvent * event);
  void mouseReleaseEvent (QMouseEvent * event);
  void mouseMoveEvent (QMouseEvent * event);
  void resizeEvent (QResizeEvent * event);
};


class IQLabel:public QLabel, public IObject
{
Q_OBJECT public:
    IQLabel (QWidget * parent, Ihandle * _handle);
  virtual char *get_alignment ();

  virtual void set_alignment (char *v);
  virtual void set_title (char *);
  virtual void set_image (char *);
};

class IAbstractList:public IObject
{
public:
  IAbstractList ();
  virtual void set_option (Ihandle * n, int op, char *v);
};

class IQCombo:public QComboBox, public IAbstractList
{
Q_OBJECT public:
    IQCombo (QWidget * parent, Ihandle * _handle);
  virtual char *get_caret ();
  virtual char *get_readonly ();
  virtual char *get_selectedtext ();
  virtual char *get_selection ();
  virtual char *get_value ();

  virtual void set_caret (char *v);
  virtual void set_readonly (char *v);
  virtual void set_selection (char *v);
  virtual void set_value (char *v);
  virtual void set_showdropdown (char *v);

  virtual void set_option (Ihandle * n, int op, char *v);

  public Q_SLOTS:void doEditCallback (char *);
  void doActionCallback (int);
};

class IQList:public QListWidget, public IObject
{
Q_OBJECT public:
    IQList (QWidget * parent, Ihandle * _handle);
  virtual char *get_multiple ();
  virtual char *get_value ();
  virtual void set_value (char *v);

  virtual void set_option (Ihandle * n, int op, char *v);

  public Q_SLOTS:void editCurrentItem (QListWidgetItem *);
  void doActionCallback (int);
  //TODO: Lots of things. Check everything.
};


class IQButton:public QPushButton, public IObject
{
Q_OBJECT public:
    IQButton (QWidget * parent, Ihandle * _handle);

  void set_title (char *);
  void set_image (char *);

  bool pressed;
  void enterEvent (QEvent * event);
  void leaveEvent (QEvent * event);
  void mousePressEvent (QMouseEvent * event);
  void mouseReleaseEvent (QMouseEvent * event);
  public Q_SLOTS:void doActionCallback ();
};

class IQDialog:public QDialog, public IObject
{
Q_OBJECT public:
    IQDialog (Ihandle * _handle);

  virtual void set_title (char *);

  void resizeEvent (QResizeEvent * event);
};

class IQFrame:public QGroupBox, public IObject
{
Q_OBJECT public:
    IQFrame (QWidget * parent, Ihandle * _handle);
  virtual void set_title (char *);
};

class IQMultiline:public QTextEdit, public IObject
{
Q_OBJECT public:
    IQMultiline (QWidget * parent, Ihandle * _handle);
  virtual char *get_value ();
  virtual char *get_readonly ();
  virtual void set_append (char *v);
  virtual void set_insert (char *v);
  virtual void set_readonly (char *v);
//TODO: Lots of things. Check everything.
};

class IQText:public QLineEdit, public IObject
{
Q_OBJECT public:
    IQText (QWidget * parent, Ihandle * _handle);
  virtual char *get_value ();
  virtual void set_readonly (char *v);
  virtual void set_value (char *v);
  virtual void set_password (char *v);
  virtual void set_selection (char *v);
  void keyPressEvent (QKeyEvent * event);
//TODO: CARET_CB
};

//Factories
void createDialog (Ihandle * n);
void createCanvas (Ihandle * n);
void createMultiline (Ihandle * n);
void createList (Ihandle * n);
void createItem (Ihandle * n);
void createButton (Ihandle * n);
void createText (Ihandle * n);
void createLabel (Ihandle * n);
void createFrame (Ihandle * n);
void createDialog (Ihandle * n);
void createMenu (Ihandle * n);
void createImage (Ihandle * n, Ihandle * prnt, int inactive);

#endif
