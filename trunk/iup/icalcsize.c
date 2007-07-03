/** \file
 * \brief set size & position of interface elements
 *
 * See Copyright Notice in iup.h
 * $Id: icalcsize.c,v 1.34 2006/03/30 22:42:44 scuri Exp $
 */

#include <stdio.h>		/* NULL */
#include <string.h>

#include "iglobal.h"
#include "idrv.h"

#define IUP_MARGIN  "MARGIN"
#define IUP_GAP     "GAP"

#define HasTitle(_)     (iupTableGet(env(_),IUP_TITLE)!=NULL)

#define H0	0x10
#define H1	0x20
#define W1	0x40
#define W0	0x80

#define isW1(n)	(usersize(n) & W1)
#define isW0(n)	(usersize(n) & W0)
#define isW(n)	(isW1(n) || isW0(n))
#define isH1(n)	(usersize(n) & H1)
#define isH0(n)	(usersize(n) & H0)
#define isH(n)	(isH1(n) || isH0(n))

static int shrink = 0;

static int  CountH1 (Ihandle *n);
static int  CountH0 (Ihandle *n);
static int  CountW1 (Ihandle *n);
static int  CountW0 (Ihandle *n);
static int  margemX (Ihandle *n);
static int  margemY (Ihandle *n);
static int  gap (Ihandle *n);

static void  iupdrvMultiLineSize  (Ihandle* e, int *w, int *h);
static void  iupdrvListSize       (Ihandle* e, int *w, int *h);
static void  iupdrvButtonSize     (Ihandle* e, char *s, int *w, int *h);
static void  iupdrvFillSize       (Ihandle* e, int *w, int *h);
static void  iupdrvFrameSize      (Ihandle* e, int *w, int *h);
static void  iupdrvHboxSize       (Ihandle* e, int *w, int *h);
static void  iupdrvLabelSize      (Ihandle* e, char *s, int *w, int *h);
static void  iupdrvTextSize       (Ihandle* e, int *w, int *h);
static void  iupdrvToggleSize     (Ihandle* e, char *s, int *w, int *h);
static void  iupdrvVboxSize       (Ihandle* e, int *w, int *h);
static void  iupdrvZboxSize       (Ihandle* e, int *w, int *h);

static int   iupdrvBoxSize        (Ihandle* e, int *w, int *h);

static int DiffWidth(Ihandle *n)
{
  int w=0;
  Ihandle *c;
  int number_of_children=0;
  foreachchild (c,n)
  {
   number_of_children++;
   w+=naturalwidth(c);
  }
  w+=(number_of_children-1)*gap(n) + 2*margemX(n);
  return currentwidth(n)-w;

}

static int DiffHeight(Ihandle *n)
{
  int h=0;
  Ihandle *c;
  int number_of_children=0;
  foreachchild (c,n)
  {
   number_of_children++;
   h+=naturalheight(c);
  }
  h+=(number_of_children-1)*gap(n) + 2*margemY(n);
  return currentheight(n)-h;

}

#define WIDTH2RASTER(w, cw) ((w * cw)/4)
#define HEIGHT2RASTER(h, ch) ((h * ch)/8)

int iupSetSize (Ihandle *root)
{
 if (root == NULL)
  return IUP_ERROR;
 if (type(root) == DIALOG_)
 {
  int w, h;
  int flag;
  char *str;

  iupSetNaturalSize (root);

  flag = iupGetSize( root, &w, &h );
  if (flag&1) w = naturalwidth(root);
  if (flag&2) h = naturalheight(root);

  str = IupGetAttribute(root, IUP_SHRINK );
  shrink = str && iupStrEqual(str, IUP_YES);
  iupSetCurrentSize (root, w, h );
  shrink = 0;

  iupSetPosition (root, 0, 0);

#if _TEST_CODE_
  /* zeramos os tamanhos para que alteracoes no tamanho feitas apos o
     mapeamento sejam consideradas */
  iupSetEnv(root, IUP_SIZE, NULL);
  iupSetEnv(root, IUP_RASTERSIZE, NULL);
#endif
 }
 else
 {
  int w=currentwidth(root);
  int h=currentheight(root);
  int x=posx(root);
  int y=posy(root);
  iupSetNaturalSize (root);
  iupSetCurrentSize (root, w, h);
  iupSetPosition (root, x, y);
 }
 return IUP_NOERROR;
}

void IupRefresh(Ihandle *self)
{
  Ihandle* dlg = IupGetDialog(self);
  if (dlg)
  {
    iupSetSize(dlg);
    iupdrvResizeObjects(dlg);
  }
}

/*                                                                  */
/* esta funcao retorna o tamanho definido no elemento de interface. */
/* valor de retorno e' "bit fields":                                */
/*            0: tamanho nao definido                               */
/*            1: tamanho X definido em pixels                       */
/*            2: tamanho Y definido em pixels                       */
int iupGetSize (Ihandle* e, int* w, int *h)
{
 int rc=0;
 char *v=NULL;
 int charunit = 0;   /* = 0, size defined in raster unit  */
                     /* = 1, size defined in character unit */
 if (e == NULL) return rc;

 /* Check with real dialog (the user may have changed it) */
 if (type(e) == DIALOG_)
   v=iupCpiGetAttribute (e,IUP_RASTERSIZE); 

 /* check RASTERSIZE attribute */
 if (v == NULL)
   v=iupTableGet (env(e), IUP_RASTERSIZE);

 /* check SIZE attribute */
 if (v == NULL)
 {
   charunit=1;
   v=iupTableGet (env(e), IUP_SIZE);
 }

 *w = *h = 0;
 if (v)
 {
   char *sh,sw[40];
   int charwidth,charheight;
   iupdrvGetCharSize(e,&charwidth, &charheight);
   strcpy(sw,v);
   sh=strchr(sw,'x');
   if (sh == NULL)
     sh="";
   else
   {
     *sh='\0';
     ++sh;
   }
  
   if (iupStrEqualNoCase(sw, IUP_FULL))          { *w=iupdrvWindowSizeX(e,1); rc|=1;}
   else if (iupStrEqualNoCase(sw, IUP_HALF))     { *w=iupdrvWindowSizeX(e,2); rc|=1;}
   else if (iupStrEqualNoCase(sw, IUP_THIRD))    { *w=iupdrvWindowSizeX(e,3); rc|=1;}
   else if (iupStrEqualNoCase(sw, IUP_QUARTER))  { *w=iupdrvWindowSizeX(e,4); rc|=1;}
   else if (iupStrEqualNoCase(sw, IUP_EIGHTH))   { *w=iupdrvWindowSizeX(e,8); rc|=1;}
   else if (charunit==0)                   { if (sscanf(sw,"%d",w)==1)  rc|=1;}
   else if (sscanf(sw,"%d",w) == 1) { *w = WIDTH2RASTER(*w, charwidth); rc|=1;}
      
   if (iupStrEqualNoCase(sh, IUP_FULL))          { *h=iupdrvWindowSizeY(e,1); rc|=2;}
   else if (iupStrEqualNoCase(sh, IUP_HALF))     { *h=iupdrvWindowSizeY(e,2); rc|=2;}
   else if (iupStrEqualNoCase(sh, IUP_THIRD))    { *h=iupdrvWindowSizeY(e,3); rc|=2;}
   else if (iupStrEqualNoCase(sh, IUP_QUARTER))  { *h=iupdrvWindowSizeY(e,4); rc|=2;}
   else if (iupStrEqualNoCase(sh, IUP_EIGHTH))   { *h=iupdrvWindowSizeY(e,8); rc|=2;}
   else if (charunit==0)                   { if (sscanf(sh,"%d",h)==1)  rc|=2;}
   else if (sscanf(sh,"%d",h)==1)  { *h = HEIGHT2RASTER(*h, charheight); rc|=2;}
 }
 return rc;
}

static int iupGetFillSize(Ihandle* e, int *w, int *h)
{
 int charunit = 0;   /* = 0, size defined in raster unit  */
                     /* = 1, size defined in character unit */
 /* pega o tamanho definido pelo usuario, em raster */
 char *s=iupTableGet (env(e), IUP_RASTERSIZE);
 /* se o usuario nao definiu o tamanho em raster, */
 /* pega o tamanho definido pelo usuario no IUP_SIZE */
 if (s == NULL)
 {
  charunit=1;
  s=iupTableGet (env(e), IUP_SIZE);
 }
 *w=*h=0;
 if (s != NULL)
 {
  Ihandle *p;
  int charwidth=0,charheight=0;
  iupdrvGetCharSize(e,&charwidth, &charheight);
  for (p=parent(e); p; p=parent(p))
   if (type(p)==HBOX_)
   {
    sscanf (s, "%d", w);
    if (charunit)  *w = WIDTH2RASTER(*w, charwidth);
    return 1;
   }
   else if (type(p)==VBOX_)
   {
    sscanf (s, "%d", h);
    if (charunit) *h = HEIGHT2RASTER(*h, charheight);
    return 2;
   }
 }
 return 0;
}

static void setusersize(Ihandle *n)
{
 char *s=IupGetAttribute (n,"EXPAND");
 if (s == NULL)
   usersize(n) = W1 | W0 | H1 | H0;
 else if (iupStrEqual(s,IUP_YES))
   usersize(n) = W1 | W0 | H1 | H0;
 else if (iupStrEqual(s,IUP_NO))
   usersize(n) = 0;
 else if (iupStrEqual(s,IUP_HORIZONTAL))
   usersize(n) = W1 | W0;
 else
   usersize(n) = H1 | H0;
}

int iupSetNaturalSize (Ihandle *n)
{
  Ihandle *c;

  setusersize(n);
  if (hclass(n))
  {
    iupCpiSetNaturalSize(n);
    return usersize(n);
  }

  if(type(n) == DIALOG_)
  {
    int which = iupdrvBoxSize (n, &naturalwidth(n), &naturalheight(n));
    if (child(n) != NULL)
    {
      usersize(n) |= iupSetNaturalSize(child(n));
      if (which == 0)
      { 
        /* o usuario setou o tamanho para NULL */
        naturalwidth(n) = naturalwidth(child(n));
        naturalheight(n) = naturalheight(child(n));
      }
      else
      {
        if (!(which & 1))
          naturalwidth(n) = MAX(currentwidth(n), naturalwidth(child(n)));

        if (!(which & 2))
	{
          naturalheight(n) = MAX(currentheight(n), naturalheight(child(n)));
	}
      }
    }
  }
  else if(type(n) == HBOX_)
  {
    int w,h;
    int number_of_children=0;
    int expand = 0;			/* direcao que os elem. filhos podem se expandir */
    iupdrvHboxSize (n, &w,&h);
    naturalwidth(n) = naturalheight(n) = 0;
    foreachchild (c,n)
    {
      number_of_children++;
      expand|=iupSetNaturalSize (c);
      naturalwidth(n) += naturalwidth(c);
      naturalheight(n) = MAX(naturalheight(n),naturalheight(c));
    }

    /* se a caixa nao pode expandir, nao adianta seus filhos serem expansiveis */
    usersize(n) &= expand;

    naturalwidth(n) += (number_of_children-1)*gap(n) + 2*margemX(n);
    naturalheight(n)+= 2*margemY(n);

    naturalwidth(n)  = MAX(w, naturalwidth(n));
    naturalheight(n) = MAX(h, naturalheight(n));
  }
  else if(type(n) == VBOX_)
  {
    int w,h;
    int expand = 0;			/* direcao que os elem. filhos podem se expandir */
    int number_of_children=0;
    iupdrvVboxSize (n, &w,&h);
    naturalwidth(n) = naturalheight(n) = 0;
    foreachchild (c,n)
    {
      number_of_children++;
      expand|=iupSetNaturalSize (c);
      naturalwidth(n)   = MAX(naturalwidth(n),naturalwidth(c));
      naturalheight(n) += naturalheight(c);
    }

    /* se a caixa nao pode expandir, nao adianta seus filhos serem expansiveis */
    usersize(n) &= expand;

    naturalwidth(n) += 2*margemX(n);
    naturalheight(n)+=(number_of_children-1)*gap(n) + 2*margemY(n);
    naturalwidth(n)  = MAX(w, naturalwidth(n));
    naturalheight(n) = MAX(h, naturalheight(n));
  }
  else if(type(n) == ZBOX_)
  {
    int expand = 0;			/* direcao que os elem. filhos podem se expandir */
    iupdrvZboxSize (n, &naturalwidth(n), &naturalheight(n));
    foreachchild (c,n)
    {
      expand|=iupSetNaturalSize (c);
      naturalwidth(n)  = MAX(naturalwidth(n),naturalwidth(c));
      naturalheight(n) = MAX(naturalheight(n),naturalheight(c));
    }
    /* se a caixa nao pode expandir, nao adianta seus filhos serem expansiveis */
    usersize(n) &= expand;
  }
  else if(type(n) == FILL_)
  {
    iupdrvFillSize (n, &naturalwidth(n), &naturalheight(n));
    usersize(n)= 0;
    if (parent(n))
    {
      if ((type(parent(n))==HBOX_) && (naturalwidth(n)<=0))  usersize(n)|=W0;
      if ((type(parent(n))==VBOX_) && (naturalheight(n)<=0)) usersize(n)|=H0;
    }
  }
  else if(type(n) == FRAME_)
  {
    int cw=0, ch=0;
    iupdrvGetCharSize(n,&cw,&ch);
    iupdrvFrameSize (n, &naturalwidth(n), &naturalheight(n));
    usersize(n)&=iupSetNaturalSize (child(n));
    naturalwidth(n) =MAX(naturalwidth(n),2*FRAME_MARGEM_X+naturalwidth(child(n)));
    naturalheight(n)=MAX(naturalheight(n),HasTitle(n)*ch+2*FRAME_MARGEM_Y+naturalheight(child(n)));
  }
  else if(type(n) == RADIO_)
  {
    usersize(n)&=iupSetNaturalSize (child(n));
    naturalwidth(n) =naturalwidth(child(n));
    naturalheight(n)=naturalheight(child(n));
  }
  else if(type(n) == BUTTON_)
    iupdrvButtonSize (n, IupGetAttribute(n,IUP_TITLE), &naturalwidth(n), &naturalheight(n));
  else if(type(n) == CANVAS_)
    iupdrvCanvasSize (n, &naturalwidth(n), &naturalheight(n));
  else if(type(n) == LABEL_)
    iupdrvLabelSize (n, IupGetAttribute(n,IUP_TITLE), &naturalwidth(n), &naturalheight(n));
  else if(type(n) == LIST_)
  {
    iupdrvListSize (n, &naturalwidth(n), &naturalheight(n));
  }
  else if(type(n) == MULTILINE_)
    iupdrvMultiLineSize (n, &naturalwidth(n), &naturalheight(n));
  else if(type(n) == TEXT_)
    iupdrvTextSize (n, &naturalwidth(n), &naturalheight(n));
  else if(type(n) == TOGGLE_)
    iupdrvToggleSize (n, IupGetAttribute(n,IUP_TITLE), &naturalwidth(n), &naturalheight(n));

  return usersize(n);
}

void iupSetCurrentSize (Ihandle *n, int w, int h)
{
  Ihandle *c;

  if (!n)
    return;

  if(hclass(n))
  {
    iupCpiSetCurrentSize(n,w,h);
    return;
  }

  if(type(n) == DIALOG_)
  {
    if (!shrink)
    {
      currentwidth(n)  = MAX (w, naturalwidth(n));
      currentheight(n) = MAX (h, naturalheight(n));
    }
    else
    {
      currentwidth(n)  = w;
      currentheight(n) = h;
    }
    iupSetCurrentSize (child(n), currentwidth(n), currentheight(n));
  }
  else if(type(n) == HBOX_)
  {
    if (!shrink)
    {
      if (!isH(n)) h = 0;
      if (!isW(n)) w = 0;
      currentwidth(n)  = MAX (naturalwidth(n),w);
      currentheight(n) = MAX (naturalheight(n),h);
    }
    else
    {
      currentwidth(n)  = isW(n) ? w : naturalwidth(n);
      currentheight(n) = isH(n) ? h : naturalheight(n);
    }
    {
    int d1 = isW1(n)              ?(DiffWidth(n))/CountW1(n) : 0;
    int d0 = (!isW1(n) && isW0(n))?(DiffWidth(n))/CountW0(n) : 0;
    foreachchild (c,n)
    {
      int d=isW1(c)?d1:(isW0(c)?d0:0);
      iupSetCurrentSize (c, naturalwidth(c)+d, currentheight(n)-2*margemY(n));
    }
    }
  }
  else if(type(n) == VBOX_)
  {
    if (!shrink)
    {
      if (!isH(n)) h = 0;
      if (!isW(n)) w = 0;
      currentwidth(n)  = MAX (naturalwidth(n),w);
      currentheight(n) = MAX (naturalheight(n),h);
    }
    else
    {
      currentwidth(n)  = isW(n) ? w : naturalwidth(n);
      currentheight(n) = isH(n) ? h : naturalheight(n);
    }
    {
    int d1 = isH1(n)              ?(DiffHeight(n))/CountH1(n) : 0;
    int d0 = (!isH1(n) && isH0(n))?(DiffHeight(n))/CountH0(n) : 0;
    foreachchild (c,n)
    {
      int d=isH1(c)?d1:(isH0(c)?d0:0);
      iupSetCurrentSize (c, currentwidth(n)-2*margemX(n), naturalheight(c)+d);
    }
    }
  }
  else if(type(n) == ZBOX_)
  {
    if (!shrink)
    {
      if (!isH(n)) h = 0;
      if (!isW(n)) w = 0;
      currentwidth(n)  = MAX (naturalwidth(n),w);
      currentheight(n) = MAX (naturalheight(n),h);
    }
    else
    {
      currentwidth(n)  = isW(n) ? w : naturalwidth(n);
      currentheight(n) = isH(n) ? h : naturalheight(n);
    }
    foreachchild (c,n)
    iupSetCurrentSize (c, currentwidth(n), currentheight(n));
  }
  else if(type(n) == FILL_ || type(n) == CANVAS_)
  {
    currentwidth(n)= isW(n)  ? w : naturalwidth(n);
    currentheight(n)= isH(n) ? h : naturalheight(n);
  }
  else if(type(n) == FRAME_)
  {
    int cw=0, ch=0;
    iupdrvGetCharSize(n,&cw,&ch);
    iupSetCurrentSize (child(n), w-2*FRAME_MARGEM_X, h-2*FRAME_MARGEM_Y-HasTitle(n)*ch);
    if (!shrink)
    {
      currentwidth(n) =MAX(naturalwidth(n), 2*FRAME_MARGEM_X+currentwidth(child(n)));
      currentheight(n)=MAX(naturalheight(n),HasTitle(n)*ch+2*FRAME_MARGEM_Y+currentheight(child(n)));
    }
    else
    {
      currentwidth(n) = isW(n) ? 
        2*FRAME_MARGEM_X+currentwidth(child(n)) :
        naturalwidth(n);
      currentheight(n)= isW(n) ?
        HasTitle(n)*ch+2*FRAME_MARGEM_Y+currentheight(child(n)) :
        naturalheight(n);
    }
  }
  else if(type(n) == RADIO_)
  {
    iupSetCurrentSize (child(n), w, h);
    currentwidth(n) =currentwidth(child(n));
    currentheight(n)=currentheight(child(n));
  }
  else if(type(n) == LIST_   || type(n) == BUTTON_   ||
          type(n) == TOGGLE_ || type(n) == LABEL_    ||
          type(n) == TEXT_   || type(n) == MULTILINE_ )
  {
  /*
    currentwidth(n)=naturalwidth(n);
    currentheight(n)=naturalheight(n);
    break;
  */
    currentwidth(n)= isW(n)  ? w : naturalwidth(n);
    currentheight(n)= isH(n) ? h : naturalheight(n);
  }
}

void iupSetPosition (Ihandle *n, int x, int y)
{
  Ihandle *c;

  if (n == NULL)
    return;

  posx(n) = x;
  posy(n) = y;

  if(hclass(n))
  {
    iupCpiSetPosition(n,x,y);
    return;
  }

  if(type(n) == DIALOG_)
  {
    iupSetPosition (child(n), x, y);
  }
  else if(type(n) == FRAME_)
  {
    int cw=0, ch=0;
    iupdrvGetCharSize(n,&cw,&ch);
    iupSetPosition (child(n), x+FRAME_MARGEM_X, y+FRAME_MARGEM_Y+HasTitle(n)*ch);
  }
  else if(type(n) == RADIO_)
  {
    iupSetPosition (child(n), x, y);
  }
  else if(type(n) == HBOX_)
  {
    int g=gap(n);
    int justify=0;
    char *j=IupGetAttribute(n,IUP_ALIGNMENT);
    if (j != NULL)
    {
      if (iupStrEqual(j,IUP_ACENTER))
      {
        justify=1;
      }
      else if (iupStrEqual(j,IUP_SOUTH) ||
        iupStrEqual(j,IUP_ABOTTOM))
      {
        justify=2;
      }
    }
    x+=margemX(n);
    y+=margemY(n);
    foreachchild(c,n)
    {
      int dy;
      if (justify==1)     /* center */
        dy=(currentheight(n)-currentheight(c)-2*margemY(n))/2;
      else if(justify==2) /* down */
        dy=currentheight(n)-currentheight(c)-2*margemY(n);
      else                 /* up */
        dy=0;
      iupSetPosition (c, x, y+dy);
      x+=currentwidth(c)+g;
    }
  }
  else if(type(n) == VBOX_)
  {
    int g=gap(n);
    int justify=0;
    char *j=IupGetAttribute(n,IUP_ALIGNMENT);
    if (j != NULL)
    {
      if (iupStrEqual(j,IUP_ACENTER))
        justify=1;
      else if ((iupStrEqual(j,IUP_EAST)) ||
        (iupStrEqual(j,IUP_ARIGHT)))
        justify=2;
    }
    x+=margemX(n);
    y+=margemY(n);
    foreachchild(c,n)
    {
      int dx;
      if (justify==1)     /* center */
        dx=(currentwidth(n)-currentwidth(c)-2*margemX(n))/2;
      else if (justify==2) /* right */
        dx=currentwidth(n)-currentwidth(c)-2*margemX(n);
      else                 /* left */
        dx=0;
      iupSetPosition (c, x+dx, y);
      y+=currentheight(c)+g;
    }
  }
  else if(type(n) == ZBOX_)
  {

#define      CT 0
#define      N  1
#define      S  2
#define      W  3
#define      E  4
#define      NE 5
#define      SE 6
#define      NW 7
#define      SW 8

    int justify=0;
    char *j=IupGetAttribute(n,IUP_ALIGNMENT);
    if (j != NULL)
    {
      if (iupStrEqual(j,IUP_ACENTER))                             justify=CT;
      else if (iupStrEqual(j,IUP_NORTH) || iupStrEqual(j,IUP_ATOP))    justify=N;
      else if (iupStrEqual(j,IUP_SOUTH) || iupStrEqual(j,IUP_ABOTTOM)) justify=S;
      else if (iupStrEqual(j,IUP_WEST) || iupStrEqual(j,IUP_ALEFT))    justify=W;
      else if (iupStrEqual(j,IUP_EAST) || iupStrEqual(j,IUP_ARIGHT))   justify=E;
      else if (iupStrEqual(j,IUP_NE))                             justify=NE;
      else if (iupStrEqual(j,IUP_SE))                             justify=SE;
      else if (iupStrEqual(j,IUP_NW))                             justify=NW;
      else if (iupStrEqual(j,IUP_SW))                             justify=SW;
    }
    foreachchild(c,n)
    {
      int dx=0,dy=0;
      switch (justify)
      {
      case CT:
        dx=(currentwidth(n)-currentwidth(c))/2;
        dy=(currentheight(n)-currentheight(c))/2;
        break;
      case N:
        dx=(currentwidth(n)-currentwidth(c))/2;
        dy=0;
        break;
      case S:
        dx=(currentwidth(n)-currentwidth(c))/2;
        dy=currentheight(n)-currentheight(c);
        break;
      case W:
        dx=0;
        dy=(currentheight(n)-currentheight(c))/2;
        break;
      case E:
        dx=currentwidth(n)-currentwidth(c);
        dy=(currentheight(n)-currentheight(c))/2;
        break;
      case NE:
        dx=currentwidth(n)-currentwidth(c);
        dy=0;
        break;
      case SE:
        dx=currentwidth(n)-currentwidth(c);
        dy=currentheight(n)-currentheight(c);
        break;
      case NW:
        dx=0;
        dy=0;
        break;
      case SW:
        dx=0;
        dy=currentheight(n)-currentheight(c);
        break;
      }
      iupSetPosition (c, x+dx, y+dy);
    }

#undef      CT
#undef      N
#undef      S
#undef      W
#undef      E
#undef      NE
#undef      SE
#undef      NW
#undef      SW

  }
}

static int CountH1 (Ihandle *n)
{
 Ihandle *c;
 int i=0;
 foreachchild (c,n)
  if (isH1(c)) i++;
 return i;
}

static int CountH0 (Ihandle *n)
{
 Ihandle *c;
 int i=0;
 foreachchild (c,n)
  if (isH0(c)) i++;
 return i;
}

static int CountW1 (Ihandle *n)
{
 Ihandle *c;
 int i=0;
 foreachchild (c,n)
  if (isW1(c)) i++;
 return i;
}

static int CountW0 (Ihandle *n)
{
 Ihandle *c;
 int i=0;
 foreachchild (c,n)
  if (isW0(c)) i++;
 return i;
}

static int margemX (Ihandle *n)
{
  return IupGetInt(n,IUP_MARGIN);
}

static int margemY (Ihandle *n)
{
  return IupGetInt2(n,IUP_MARGIN);
}

static int gap (Ihandle *n)
{
  return IupGetInt(n,IUP_GAP);
}

static int iupdrvBoxSize(Ihandle* e, int *w, int *h)
{
  int rc = 0;
  type(e) = USER_;
  rc = iupGetSize(e, w, h);
  type(e) = DIALOG_;
  return rc;
}

static void iupdrvFillSize(Ihandle* e, int *w, int *h)
{
 iupGetFillSize(e,w,h);
}

static void iupdrvFrameSize(Ihandle* e, int *w, int *h)
{
 iupGetSize(e, w, h);
}

static void iupdrvHboxSize(Ihandle* e, int *w, int *h)
{
 iupGetSize(e, w, h);
 *h = 0;
}

static void iupdrvVboxSize(Ihandle* e, int* w, int* h)
{
 iupGetSize(e, h, w);
 *w = 0;
}

static void iupdrvZboxSize(Ihandle* e, int* w, int* h)
{
 iupGetSize(e, h, w);
}

static void iupdrvLabelSize(Ihandle* e, char *s, int *w, int *h)
{
  iupGetSize(e, w, h);

  if (*w <= 0 || *h <= 0)
  {
    int native_w = 0, native_h = 0;
    Ihandle *image = IupGetHandle(IupGetAttribute(e, IUP_IMAGE));
    if (image)
    {
      native_w = IupGetInt(image,IUP_WIDTH);
      native_h = IupGetInt(image,IUP_HEIGHT);
    }
    else
    {
      if (!IupGetAttribute(e, IUP_SEPARATOR))
        iupdrvGetTextLineBreakSize(e, s, &native_w, &native_h);
    }

    if (*w <= 0) *w = native_w;
    if (*h <= 0) *h = native_h;
  }
}

int iupdrvCanvasSize (Ihandle* e, int *w, int *h)
{
  int ret=0; /* no resize */
  int cw, ch;
  iupdrvGetCharSize(e, &cw, &ch); 
  iupGetSize(e, w, h);

  /* minimum size is 2 characters if not defined */

  if (*w <= 0)
  {
    ret |= 1;  /* resize in W */
    *w = cw*2;
  }

  if (*h <= 0)
  {
    ret |= 2;  /* resize in H */
    *h = ch*2;
  }

  return ret;
}

static void iupdrvButtonSize (Ihandle* e, char *s, int *w, int *h)
{
  iupGetSize(e, w, h);

  if (*w <= 0 || *h <= 0)
  {
    int native_w = 0, native_h = 0;
    Ihandle *image = IupGetHandle(IupGetAttribute(e, IUP_IMAGE));
    if (image)
    {
      Ihandle *impress = IupGetHandle(IupGetAttribute(e, IUP_IMPRESS));

      native_w = IupGetInt(image,IUP_WIDTH);
      native_h = IupGetInt(image,IUP_HEIGHT);

      if(!impress)
      {
        int border = iupdrvGetBorderSize(e);
        /* add room for borders */
        native_w += 2*border;  
        native_h += 2*border;  
      }
    }
    else
    {
      int cw, ch;
      int border = iupdrvGetBorderSize(e);
      iupdrvGetCharSize(e, &cw, &ch);
      iupdrvStringSize(e, s, &native_w, &native_h);

      /* add room around the text */
      native_w += 2*cw; 
      native_h += ch/2; 

      /* add room for borders */
      native_w += 2*border;  
      native_h += 2*border;  
   }

    if (*w <= 0) *w = native_w;
    if (*h <= 0) *h = native_h;
  }
}

static void iupdrvToggleSize (Ihandle* e, char *s, int *w, int *h)
{
  iupGetSize(e, w, h);

  if (*w <= 0 || *h <= 0)
  {
    int native_w = 0, native_h = 0;
    Ihandle *image = IupGetHandle(IupGetAttribute(e, IUP_IMAGE));
    if (image)
    {
      native_w = IupGetInt(image,IUP_WIDTH);
      native_h = IupGetInt(image,IUP_HEIGHT);

      {
        /* toggle always have borders */
        int border = iupdrvGetBorderSize(e);
        /* add room for borders */
        native_w += 2*border;  
        native_h += 2*border;  
      }
    }
    else
    {
      iupdrvStringSize(e, s, &native_w, &native_h);

      /* add room for check box */
      native_w += 16+4; /* square width + space before text */
      native_h = MAX(native_h, 16);
    }

    if (*w <= 0) *w = native_w;
    if (*h <= 0) *h = native_h;
  }
}

static void iupdrvTextSize(Ihandle* e, int *w, int *h)
{
  iupGetSize(e, w, h);

  if (*w <= 0 || *h <= 0)
  {
    int native_w = 0, native_h = 0;
    char* s = IupGetAttribute(e, "VALUE");
    if (!s || !(*s)) s = "WWWWW"; /* default 5 chars */

    iupdrvStringSize(e, s, &native_w, &native_h);

    /* add room for borders */
    native_w += 2*(3+4);
    native_h += 2*(3+1);  

    if (*w <= 0) *w = native_w;
    if (*h <= 0) *h = native_h;
  }
}

static void iupdrvMultiLineSize(Ihandle* e, int *w, int *h)
{
  iupGetSize(e, w, h);

  if (*w <= 0 || *h <= 0)
  {
    int native_w = 0, native_h = 0;
    int sb = iupdrvGetScrollSize();
    char* s = IupGetAttribute(e, "VALUE");
    if (!s || !(*s)) s = "WWWWW"; /* default 5 chars */

    iupdrvGetTextLineBreakSize(e, s, &native_w, &native_h);

    /* add room for borders and scrollbars */
    native_w += 2*(3+5)+sb;
    native_h += 2*(3+1)+sb;  

    if (*w <= 0) *w = native_w;
    if (*h <= 0) *h = native_h;
  }
}

static void getListItemSize(Ihandle *e, int *w, int *h, int isdropdown)
{
   char *value;
   int i, iw, ih;
   char index[20];

   *w = 0;
   *h = 0;

   i = 1;
   sprintf(index, "%d", i);
   while ((value = IupGetAttribute(e, index)) != NULL)
   {
      iupdrvStringSize(e, value, &iw, &ih);
      if (iw > *w)
         *w = iw;
      if (isdropdown)
      {
        if (ih > *h) 
          *h = ih;
      }
      else
        (*h) += ih;

      i++; 
      sprintf(index, "%d", i);
   }

   if (*w == 0 || *h == 0) /* default is 5 characters in 1 item */
     iupdrvStringSize(e, "WWWWW", w, h);
}

static void iupdrvListSize(Ihandle* e, int *w, int *h)
{
  iupGetSize(e, w, h);

  if (*w <= 0 || *h <= 0)
  {
    int native_w = 0, native_h = 0;
    int sb = iupdrvGetScrollSize();
    int isdropdown = iupCheck(e, IUP_DROPDOWN)==YES? 1: 0;
    int haseditbox = iupCheck(e, "EDITBOX")==YES? 1: 0;

    getListItemSize (e, &native_w, &native_h, isdropdown);

    /* add room for borders */
    native_w += 2*(3+5);
    native_h += 2*(3+2);  

    /* add room for scrollbar or dropdown box */
    native_w += sb;

    if (isdropdown && haseditbox)
    {
      /* adds an extra border */
      native_h += 6;  
    }

    if (haseditbox && !isdropdown) 
    {
      /* adds the text box size */
      int text_w, text_h;
      char* s = IupGetAttribute(e, "VALUE");
      if (!s || !(*s)) s = "WWWWW"; /* default 5 chars */

      iupdrvStringSize(e, s, &text_w, &text_h);
      native_h += 5; /* internal border */
      if (text_w > native_w)
        native_w = text_w;
      native_h += text_h;
    }

    if (isdropdown)
    {
      if (native_h < sb)
        native_h = sb;
    }

    if (*w <= 0) *w = native_w;
    if (*h <= 0) *h = native_h;
  }
}
