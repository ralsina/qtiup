/*Example IupGLCanvas in C 
	Creates a OpenGL canvas and draws a line in it.*/

/********************************************************************
** Tested on Windows, please change windows include.
**
** by Mark Stroetzel Glasberg, March 2001
** mark@tecgraf.puc-rio.br
********************************************************************/

#include <stdio.h>
#include "iup.h"          
#include "iupgl.h"
#include "GL/gl.h" 

int k_any(Ihandle *self, int c)
{
  if(c == K_q)
    return IUP_CLOSE;
  else
    return IUP_DEFAULT;
}

int redraw(Ihandle *self, float x, float y)
{
  int w, h;
  char *size = IupGetAttribute(self, IUP_RASTERSIZE);
  sscanf(size, "%dx%d", &w, &h);

  IupGLMakeCurrent(self);
  glViewport(0, 0, w, h);
  glClearColor(1.0, 1.0, 1.0, 1.f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glColor3f(1.0,0.0,0.0);
  glBegin(GL_QUADS); 
  glVertex2f(0.9f,0.9f); 
  glVertex2f(0.9f,-0.9f); 
  glVertex2f(-0.9f,-0.9f); 
  glVertex2f(-0.9f,0.9f); 
  glEnd();

  IupGLSwapBuffers(self); 

  return IUP_DEFAULT;
}

int main(void)
{
  Ihandle *canvas, *finale, *dg;

  IupOpen();
  IupGLCanvasOpen();

  canvas = IupGLCanvas(NULL);
  IupSetCallback(canvas, "ACTION", (Icallback) redraw);
  IupSetAttribute(canvas, IUP_BUFFER, IUP_DOUBLE);
  IupSetAttribute(canvas, IUP_RASTERSIZE, "123x200");

  finale = IupHbox(IupFill(), 
                   canvas, 
                   IupFill(), 
                   NULL);

  dg = IupDialog(finale);
  IupSetAttribute(dg, IUP_TITLE, "IupGLCanvas");

  IupShow(dg);
  IupMainLoop();
  IupDestroy(dg);
  IupClose();

  return 1;
}
