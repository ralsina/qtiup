-- Example IupGLCanvas in Lua 
-- Creates a OpenGL canvas and draws a line in it. 
-- This example uses gllua binding of OpenGL to Lua.
 
cv = iup.glcanvas{buffer="DOUBLE", rastersize = "300x300"}

function cv:action(x, y)
  iup.GLMakeCurrent(self)
  --glClearColor(1.0, 1.0, 1.0, 1.0)
  --glClear(GL_COLOR_BUFFER_BIT)
  --glClear(GL_DEPTH_BUFFER_BIT)
  --glMatrixMode( GL_PROJECTION )
  --glViewport(0, 0, 300, 300)
  --glLoadIdentity()
  --glBegin( GL_LINES ) 
  --glColor(1.0, 0.0, 0.0)
  --glVertex(0.0, 0.0)
  --glVertex(10.0, 10.0)
  --glEnd()
  iup.GLSwapBuffers(self)
  return iup.DEFAULT
end

dg = iup.dialog{cv; title="IupGLCanvas Example"}

function cv:k_any(c)
  if c == iup.K_q then
    return iup.CLOSE
  else
    return iup.DEFAULT
  end
end


dg:show()
