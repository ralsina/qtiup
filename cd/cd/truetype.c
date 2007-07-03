/*
____________________________________________________________________

%M Subdriver de Fontes TrueType do CDSimulate [Canvas Draw] 
%a Erick de Moura Ferreira
   Antonio Scuri
%d 2/9/98
%p Sistemas Graficos
%v Versao 1.2
_____________________________________________________________________

*/

#include <string.h>
#include <stdlib.h>
#include <memory.h>
#include <stdio.h>

#include "truetype.h"

/*******************************************
        Inicializa o Rasterizador
********************************************/
static char *getCdDir(void)
{
  static char *env = NULL;
  if (env) return env;
  env = getenv("CDDIR");
  if (!env) env = ".";
  return env;
}

#ifdef WIN32
#include <windows.h>
static int ReadStringKey(HKEY base_key, char* key_name, char* value_name, char* value)
{
	HKEY key;
	DWORD max_size = 512;

	if (RegOpenKeyEx(base_key, key_name, 0, KEY_READ, &key) != ERROR_SUCCESS)
		return 0;

  if (RegQueryValueEx(key, value_name, NULL, NULL, (LPBYTE)value, &max_size) != ERROR_SUCCESS)
  {
    RegCloseKey(key);
		return 0;
  }

	RegCloseKey(key);
	return 1;
}

char* GetFontDir(void)
{
  static char font_dir[512];
  if (!ReadStringKey(HKEY_CURRENT_USER, "Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Shell Folders", "Fonts", font_dir))
    return "";
  else
  {
    int i, size = (int)strlen(font_dir);
    for(i = 0; i < size; i++)
    {
      if (font_dir[i] == '\\')
        font_dir[i] = '/';
    }
    return font_dir;
  }
}
#endif

void cdTT_load(cdTT_Text * tt_text, char *font, int size, double xres, double yres)
{
  char filename[512];
  FILE *file;  /* usado apenas para procurar pelo arquivo */
  FT_Error error;
  FT_Face face;          

  /* abre arq. no dir. corrente */
  strcpy(filename, font);
  file = fopen(filename, "r");

  if (file)
    fclose(file);
  else
  {
    /* se nao conseguiu, abre arq. no dir. do cd, */
    sprintf(filename, "%s/%s", getCdDir(), font);
    file = fopen(filename, "r");

    if (file)
      fclose(file);
    else
    {
#ifdef WIN32
      /* no caso do Windows procura no seu diretorio de fontes. */
      sprintf(filename, "%s/%s", GetFontDir(), font);
      file = fopen(filename, "r");

      if (file)
        fclose(file);
      else
        return;
#else
      return;
#endif
    }
  }

  error = FT_New_Face(tt_text->library, filename, 0, &face );
  if (error) 
    return;

                                    /* char_height in 1/64th of points */
  error = FT_Set_Char_Size(face, 0, size*64, (int)(xres*25.4), (int)(yres*25.4));  
  if (error) 
  {
    FT_Done_Face(face);
    return;
  }

  if (tt_text->face && tt_text->face != face)
    FT_Done_Face(tt_text->face);

  tt_text->face = face;

  tt_text->ascent     =  face->size->metrics.ascender >> 6;
  tt_text->descent    =  abs(face->size->metrics.descender >> 6);
  tt_text->max_height =  face->size->metrics.height >> 6;
  tt_text->max_width  =  face->size->metrics.max_advance >> 6;

  if (!face->charmap)
    FT_Set_Charmap(face, face->charmaps[0]);
}

/*******************************************
              Inicializaccao 
********************************************/
cdTT_Text* cdTT_create(void)
{
  cdTT_Text * tt_text = malloc(sizeof(cdTT_Text));
  memset(tt_text, 0, sizeof(cdTT_Text));
  
  FT_Init_FreeType(&tt_text->library);

  return tt_text;
}

/*******************************************
            Desaloca Recursos
********************************************/
void cdTT_free(cdTT_Text * tt_text)
{
  if (tt_text->rgba_data)
    free(tt_text->rgba_data);

  if (tt_text->face)
    FT_Done_Face(tt_text->face);

  FT_Done_FreeType(tt_text->library);

  free(tt_text);
}

#ifdef SunOS_OLD
void *memmove( void *dest, const void *src, size_t count )
{
  return memcpy(dest, src, count);
}
#endif
