#ifndef _SIM_
#define _SIM_

#include "cd.h"
#include "cdprivat.h"
#include "truetype.h"


typedef struct _cdPrivateSimulation 
{
  struct {
    int n;                  /* Numero de pontos correntes                   */
    int *pntx;              /* Coordenadas X dos vertices                   */
    int *pnty;              /* Coordenadas Y dos vertices                   */
    int *sgmx;              /* Coordenadas dos segmentos de linhas a serem_ */
    int *sgmy;              /* _clipadas no modulo line                     */
  } clippoly;              

  cdTT_Text* tt_text;          /* Estrutura do rasterizador de fontes TrueType */

  int context;

  cdPrivateCanvas *cnv;

  void (*FillLine)(int xmin, int y, int xmax, int _);
  void (*PatternLine)(int xmin, int xmax, int y, int pw, long *pattern);
  void (*StippleLine)(int xmin, int xmax, int y, int pw, unsigned char *pattern);
  void (*HatchLine)(int xmin, int xmax, int y, unsigned char pattern);
} cdPrivateSimulation;


extern cdPrivateSimulation* active_sim;

enum{
  CLIP_LINES,
  CLIP_REGION
};

#define sRotateL(x)					            \
		 do												          \
		 {												          \
			 if((x) & 0x8000)	        		    \
				 (x) = ((x) << 1)|(0x0001);		  \
			 else										          \
				 (x) = ((x) << 1);			        \
		 }while(0)

#define RotateL(x)  ((x) = ((unsigned)(x) << 1) | ((unsigned)(x) >> 7))

#define RotatenL(x,n) ((x) = ((unsigned)(x) << (n)) | ((unsigned)(x) >> (8-(n))))

void simDoFillLine(int _flxmin,int _flymin,int _flxmax,int _flymax);


#endif

