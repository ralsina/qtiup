/*
____________________________________________________________________

%M Subdriver de Fontes TrueType do cdSimulate [Canvas Draw] 
%a Erick de Moura Ferreira
   Antonio Scuri
%d 2/6/98
%p Sistemas Graficos
%v Versao 1.1
_____________________________________________________________________
*/

#ifndef _CD_TRUETYPE_
#define _CD_TRUETYPE_

#ifdef __cplusplus
extern "C" {
#endif

#include "ft2build.h"
#include FT_FREETYPE_H

/*
   In CD version 4.4 we start to use FreeType 2.
   Current tested version FreeType 2.1.10.
   Only TrueType font support is enabled.
   Files ftmodule.h and ftoption.h were changed to configure the library.
*/

typedef struct _cdTT_Text
{
  FT_Library library;
  FT_Face face;          

  unsigned char* rgba_data;
  int rgba_data_size;

  int max_height;
  int max_width;
  int descent;
  int ascent;

}cdTT_Text;

cdTT_Text* cdTT_create(void);
void cdTT_free(cdTT_Text * tt_text);
void cdTT_load(cdTT_Text * tt_text, char *font,int size, double xres, double yres);

#ifdef __cplusplus
}
#endif

#endif  /* ifndef _CD_TRUETYPE_ */

