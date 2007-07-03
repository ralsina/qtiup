#ifndef _CD_MFPRIV_
#define _CD_MFPRIV_

#ifdef __cplusplus
extern "C" {
#endif

typedef struct cdCanvasMF 
{
  char filename[10240];   /* nome do arquivo, precisa ser o primeiro dessa estrutura, pois e' usado por outros drivers */
  void* data;             /* tambem usado por outros drivers */  
  int last_line_style;
  int last_fill_mode;
  cdPrivateCanvas* cnv;
  FILE* Stream;
} cdCanvasMF;

void cdMFCreateCanvas(cdPrivateCanvas *cnv, void *data);
void cdMFInitTable(cdPrivateCanvas* cnv);
void cdMFKillCanvas(void *canvas);

#ifdef __cplusplus
}
#endif

#endif 


