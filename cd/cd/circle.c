#include <stdio.h>
#include <math.h>

#include <cd.h>

#include "list.h"
#include "types.h"
#include "intcgm.h"
#include "intcgm6.h"

#include "circle.h"

#define DIM 360

#ifndef PI
#define PI 3.14159265358979323846
#endif

#define PI180 PI/180.
#define TWOPI 2*PI
#define VARCS TWOPI/32.

int poly_circle ( double xc, double yc, double radius, double angi, double angf, int fechado )
{
 double coseno, seno;
 double xs, ys;
 
 coseno = cos (VARCS);
 seno   = sin (VARCS);

 xs = radius * cos(angi);
 ys = radius * sin(angi);

 cdBegin ( setintstyle(fill_att.int_style) );

 if ( fechado==CLOSED_PIE ) cdVertex ( vdcx2canvas(xc), vdcy2canvas(yc) );

 cdVertex ( vdcx2canvas(xc+xs), vdcy2canvas(yc+ys) );

 while ( (angi+VARCS) < angf )
  {
   double xe = xs;
   xs = xs * coseno - ys * seno;
   ys = ys * coseno + xe * seno;
   cdVertex ( vdcx2canvas(xc+xs), vdcy2canvas(yc+ys) );
   angi += VARCS;
  }

 cdVertex ( vdcx2canvas(xc+radius*cos(angf)), vdcy2canvas(yc+radius*sin(angf)) );

 cdEnd();

 return 0;
}

int line_circle ( double xc, double yc, double radius, double angi, double angf, int fechado )
{
 double coseno, seno;
 double xant, yant, firstx, firsty;
 double xs, ys;

 /* GERA O DESENHO DO CIRCULO/ARCO */

 coseno = cos (VARCS);
 seno   = sin (VARCS);

 xs = radius * cos(angi);
 ys = radius * sin(angi);

 if ( fechado==CLOSED_PIE )
  cdLine ( vdcx2canvas(xc), vdcy2canvas(yc), vdcx2canvas(xc+xs), vdcy2canvas(yc+ys) );

 xant = firstx = xc+xs;
 yant = firsty = yc+ys;

 while ( (angi+VARCS) < angf )
  {
   double xe = xs;
   xs = xs * coseno - ys * seno;
   ys = ys * coseno + xe * seno;
   cdLine ( vdcx2canvas(xant), vdcy2canvas(yant), vdcx2canvas(xc+xs), vdcy2canvas(yc+ys) );
   xant = xc+xs;
   yant = yc+ys;
   angi += VARCS;
  }

 cdLine ( vdcx2canvas(xant), vdcy2canvas(yant),
          vdcx2canvas(xc+radius*cos(angf)), vdcy2canvas(yc+radius*sin(angf)) );

 xant = xc+radius*cos(angf);
 yant = yc+radius*sin(angf);

 if ( fechado==CLOSED_PIE )
  cdLine ( vdcx2canvas(xant), vdcy2canvas(yant), vdcx2canvas(xc), vdcy2canvas(yc) );
 else if ( fechado==CLOSED_CHORD )
  cdLine ( vdcx2canvas(xant), vdcy2canvas(yant), vdcx2canvas(firstx), vdcy2canvas(firsty) );

 return 0;
}

